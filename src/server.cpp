#include "server.h"


void TCPConnection::start(KVStore& store, TCPServer* server) 
{
    kv_store = &store;
    _server = server;
    // Start async reading of data from the client immediately
    boost::asio::post(_strand, [self = shared_from_this()] {
        boost::asio::async_read_until(self->_socket, self->_buffer, "\n",
            boost::asio::bind_executor(self->_strand,
                [self](const boost::system::error_code& error, size_t bytes_transferred) {
                    self->handle_read(error, bytes_transferred);
                }));
    });
}


void TCPConnection::handle_read(const boost::system::error_code& error, size_t bytes_transferred) 
{
    if (!error) {
        if (!_server->rate_limiter_.consume(1.0)) {
            _message = "ERROR: Node rate limit exceeded\n";
            std::cerr << "[ERROR] Node rate limit exceeded\n" << std::flush;
        } else if (!client_rate_limiter.consume(1.0)) {
            _message = "ERROR: Client rate limit exceeded\n";
            std::cerr << "[ERROR] Client rate limit exceeded\n" << std::flush;
        } else {
            // Extract data from boost::asio::const_buffer
            std::istream is(&_buffer);
            std::string request;
            std::getline(is, request);
            request.erase(0, request.find_first_not_of(" \t\r\n"));
            request.erase(request.find_last_not_of(" \t\r\n") + 1);

            if (request.empty()) {
                boost::asio::async_read_until(_socket, _buffer, "\n",
                    boost::asio::bind_executor(_strand,
                        [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                            self->handle_read(error, bytes_transferred);        // If request is empty, carry on reading
                        }));
                return;
            }

            std::cerr << "[DEBUG] Raw request: " << request << "\n" << std::flush;
            std::string command, key, value;
            std::istringstream iss(request);
            iss >> command >> key;

            std::cerr << "[DEBUG] Parsed command: '" << command << ", key: " << key << "'\n" << std::flush;

            if (command == "JOIN") {
                _message = "JOIN command received\n";
            } else if (command == "LEAVE") {
                if (!key.empty()) {
                    // If client is requesting to leave active nodes,
                    // post the lambda expression defined below to the _nodes_strand
                    // to synchronise access to the active_nodes hash map
                    std::cerr << "[DEBUG] Removing node: " << key << "\n" << std::flush;
                    boost::asio::post(_server->_nodes_strand, [self = shared_from_this(), key]() {
                        self->_server->remove_node(key);
                        self->_message = "OK\n";
                        boost::asio::async_write(self->_socket, boost::asio::buffer(self->_message),
                            boost::asio::bind_executor(self->_strand,
                                [self](const boost::system::error_code& error, size_t bytes_transferred) {
                                    self->handle_write(error, bytes_transferred);
                                }));
                    });
                    return;
                } else {
                    std::cerr << "[ERROR] LEAVE command missing address\n" << std::flush;
                    _message = "ERROR: Missing address for LEAVE\n";
                }
            } else if (command == "PUT") {
                // Extract data from boost::asio::const_buffer
                std::string rest_of_line;
                std::getline(iss, rest_of_line);
                value = rest_of_line;
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                if (key.empty() || value.empty()) {
                    std::cerr << "[ERROR] PUT command missing key or value\n" << std::flush;
                    _message = "ERROR: Missing key or value for PUT\n";
                } else {
                    std::cerr << "[DEBUG] PUT key: '" << key << "', value: '" << value << "'\n" << std::flush;
                    std::cerr << "[DEBUG] Storing key locally\n" << std::flush;
                    kv_store->put(key, value);
                    _message = "OK\n";
                }
            } else if (command == "GET") {
                if (!key.empty()) {
                    std::cerr << "[DEBUG] Getting key: '" << key << "'\n" << std::flush;
                    std::string result = kv_store->get(key);
                    if (result.empty()) {
                        std::cerr << "[DEBUG] Key not found\n" << std::flush;
                        _message = "NOT_FOUND\n";
                    } else {
                        std::cerr << "[DEBUG] Found value: '" << result << "'\n" << std::flush;
                        _message = result + "\n";
                    }
                } else {
                    std::cerr << "[ERROR] GET command missing key\n" << std::flush;
                    _message = "ERROR: Missing key for GET\n";
                }
            } else if (command == "DELETE") {
                if (!key.empty()) {
                    std::cerr << "[DEBUG] Deleting key: '" << key << "'\n" << std::flush;
                    int result = kv_store->remove(key);
                    _message = (result == 0) ? "OK\n" : "NOT_FOUND\n";
                } else {
                    std::cerr << "[ERROR] DELETE command missing key\n" << std::flush;
                    _message = "ERROR: Missing key for DELETE\n";
                }
            } else if (command == "EXIT") {
                std::cerr << "[DEBUG] Closing connection per client request\n" << std::flush;
                _message = "CLOSING_SOCKET\n";
                boost::asio::async_write(_socket, boost::asio::buffer(_message),
                    boost::asio::bind_executor(_strand,
                        [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                            self->handle_write(error, bytes_transferred);
                        }));
                _socket.close();
                return;
            } else {
                std::cerr << "[ERROR] Unkown command: " << command << "\n" << std::flush;
                _message = "ERROR: Unkown command\n";
            }
        }

        std::cerr << "[DEBUG] Sending response: " << _message << "\n" << std::flush;
        boost::asio::async_write(_socket, boost::asio::buffer(_message),
            boost::asio::bind_executor(_strand,
                [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                    self->handle_write(error, bytes_transferred);
                }));
    } else {
        std::cerr << "[ERROR] Read error: " << error.message() << "\n" << std::flush;
        stop();
        _socket.close();
    } 
}


void TCPConnection::handle_write(const boost::system::error_code& error, size_t /*bytes_transferred*/) 
{
    if (!error) {
        std::cerr << "[DEBUG] Command sent successfully\n" << std::flush;
        boost::asio::async_read_until(_socket, _buffer, "\n",
            boost::asio::bind_executor(_strand,
                [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                    self->handle_read(error, bytes_transferred);
                }));
    } else {
        std::cerr << "[ERROR] Write error: " << error.message() << "\n";
        _socket.close();
    }
}


// NOTE: IGNORE FOR NOW; not implementing soon
std::string TCPConnection::forward_to_node(std::string_view address, std::string_view message) 
{
    try {
        size_t colon = address.find(':');
        std::cerr << "[FORWARD] Attempting to resolve: " << address << "\n" << std::flush;
        std::string host = std::string(address.substr(0, colon));
        host.erase(0, host.find_first_not_of(" \t\r\n")); 
        host.erase(host.find_last_not_of(" \t\r\n") + 1);
        std::string port = std::string(address.substr(colon + 1));
        port.erase(0, port.find_first_not_of(" \t\r\n")); 
        port.erase(port.find_last_not_of(" \t\r\n") + 1);

        if (colon == std::string::npos) {
            std::cerr << "[ERROR] Invalid address format: " << address << "\n";
            std::cerr << std::flush;
            return "ERROR_FORWARDING";
        }        

        if (host.empty() || port.empty()) {
            std::cerr << "[ERROR] Empty host or port extracted from address: " << address << "\n";
            std::cerr << std::flush;
            return "ERROR_FORWARDING";
        }       

        return "NULL";
    } catch (std::exception& e) {
        std::cerr << "Forwarding error: " << e.what() << "\n";
        return "ERROR_FORWARDING";
    }
}


void TCPServer::start_accept() 
{
    TCPConnection::pointer new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(_acceptor.get_executor().context()), client_rate_limit);
    _acceptor.async_accept(new_connection->socket(),
    [this, new_connection](const boost::system::error_code& error) {
            handle_accept(new_connection, error);
        });
}


void TCPServer::handle_accept(std::shared_ptr<TCPConnection> new_connection, const boost::system::error_code& error) 
{
    if (!error) {
        std::cout << "Client connected" << std::endl; 
        new_connection->start(_store, this);
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
    start_accept();
}


void TCPServer::handle_client(tcp::socket socket) 
{
    auto new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(socket.get_executor().context()));
    new_connection->socket() = std::move(socket);
    new_connection->start(_store, this);
}


void TCPServer::run_server(boost::asio::io_context& context, int port, int threads) 
{
    auto work_guard = boost::asio::make_work_guard(context);

    boost::asio::thread_pool pool(threads);
    for (int i = 0; i < threads; ++i) 
        boost::asio::post(pool, [&context](){ context.run(); });

    std::cout << "ShardKV server listening on port " << port << " with " << threads << " worker threads." << std::endl;

    boost::asio::signal_set signals(context, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code& ec, int signo) { 
        if (!ec) {
            std::cout << "Recieved signal: " << ec << " , stopping server" << std::endl;
            context.stop(); 
            if (_acceptor.is_open()) {
                boost::system::error_code ec;
                _acceptor.close();
                if (ec)
                    std::cerr << "Error closing acceptor: " << ec << std::endl;
            }
        } else
            std::cerr << "Signal handler error: " << ec.message() << std::endl;
    });

    pool.join();
}


void TCPServer::add_node(std::string_view address) 
{
    boost::asio::post(_nodes_strand, [this, address = std::string(address)]() {
        active_nodes.insert(address);
    });
}


void TCPServer::remove_node(std::string_view address) 
{
    boost::asio::post(_nodes_strand, [this, address = std::string(address)]() {
        active_nodes.erase(address);
    });
}


int main(int argc, char **argv) 
{
    try {   
        // if (argc < 3) {
        //     std::cerr << "USAGE: " << argv[0] << "<address> <port>\n";
        //     return 1;
        // }

        boost::asio::io_context io_context;
        const int port = atoi(argv[2]);
        const std::string address = argv[1];
        const int num_threads = std::thread::hardware_concurrency();

        TCPServer tcp_server(io_context, port, address);
        tcp_server.run_server(io_context, port, num_threads);
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

