#include "server.h"

std::mutex console_mutex;


void TCPConnection::start(KVStore& store, TCPServer* server) 
{
    kv_store = &store;
    server = server;
    // Start async reading of data from the client immediately
    boost::asio::post(strand_, [self = shared_from_this()] {
        boost::asio::async_read_until(self->socket_, self->buffer, "\n",
            boost::asio::bind_executor(self->strand_,
                [self](const boost::system::error_code& error, size_t bytes_transferred) {
                    self->handle_read(error, bytes_transferred);
                }));
    });
}


void TCPConnection::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::istream is(&buffer);
        std::string request;
        std::getline(is, request);
        request.erase(0, request.find_first_not_of(" \t\r\n"));
        request.erase(request.find_last_not_of(" \t\r\n") + 1);

        if (request.empty()) {
            boost::asio::async_read_until(socket_, buffer, "\n",
                boost::asio::bind_executor(strand_,
                    [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                        self->handle_read(error, bytes_transferred);
                    }));
            return;
        }

        std::cerr << "[DEBUG] Raw request: " << request << "\n" << std::flush;
        std::string command, key, value;
        std::istringstream iss(request);
        iss >> command >> key;

        if (command == "GOSSIP_RATE_LIMIT") {
            std::string state;
            std::getline(iss, state);
            state.erase(0, state.find_first_not_of(" \t\r\n"));
            server->gossip_rate_limiter.recieve_state(server->self_address, state);
            message = "OK\n";
        } else if (!server->gossip_rate_limiter.consume(1.0)) {
            message = "ERROR: Global rate limit exceeded\n";
            std::cerr << "[ERROR] Global rate limit exceeded\n" << std::flush;
            server->rejected_requests++;
        } else if (!server->rate_limiter.consume(1.0)) {
            message = "ERROR: Node rate limit exceeded\n";
            std::cerr << "[ERROR] Node rate limit exceeded\n" << std::flush;
            server->rejected_requests++;
        } else if (!client_rate_limiter.consume(1.0)) {
            message = "ERROR: Client rate limit exceeded\n";
            std::cerr << "[ERROR] Client rate limit exceeded\n" << std::flush;
           server->rejected_requests++;
        } else {
            if (command == "JOIN") {
                boost::asio::post(server->nodes_strand, [self = shared_from_this(), key]() {
                    self->server->add_node(key);
                    self->message = "OK\n";
                    boost::asio::async_write(self->socket_, boost::asio::buffer(self->message),
                        boost::asio::bind_executor(self->strand_,
                            [self](const boost::system::error_code& error, size_t bytes_transferred) {
                                self->handle_write(error, bytes_transferred);
                            }));
                });
                return;
            } else if (command == "LEAVE") {
                if (!key.empty()) {
                    std::cerr << "[DEBUG] Removing node: " << key << "\n" << std::flush;
                    boost::asio::post(server->nodes_strand, [self = shared_from_this(), key]() {
                        self->server->remove_node(key);
                        self->message = "OK\n";
                        boost::asio::async_write(self->socket_, boost::asio::buffer(self->message),
                            boost::asio::bind_executor(self->strand_,
                                [self](const boost::system::error_code& error, size_t bytes_transferred) {
                                    self->handle_write(error, bytes_transferred);
                                }));
                    });
                    return;
                } else {
                    std::cerr << "[ERROR] LEAVE command missing address\n" << std::flush;
                    message = "ERROR: Missing address for LEAVE\n";
                }
            } else if (command == "PUT") {
                std::string rest_of_line;
                std::getline(iss, rest_of_line);
                value = rest_of_line;
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                if (key.empty() || value.empty()) {
                    std::cerr << "[ERROR] PUT command missing key or value\n" << std::flush;
                    message = "ERROR: Missing key or value for PUT\n";
                } else {
                    std::cerr << "[DEBUG] PUT key: '" << key << "', value: '" << value << "'\n" << std::flush;
                    kv_store->put(key, value);
                    message = "OK\n";
                }
            } else if (command == "GET") {
                if (!key.empty()) {
                    std::cerr << "[DEBUG] Getting key: '" << key << "'\n" << std::flush;
                    std::string result = kv_store->get(key);
                    if (result.empty()) {
                        std::cerr << "[DEBUG] Key not found\n" << std::flush;
                        message = "NOT_FOUND\n";
                    } else {
                        std::cerr << "[DEBUG] Found value: '" << result << "'\n" << std::flush;
                        message = result + "\n";
                    }
                } else {
                    std::cerr << "[ERROR] GET command missing key\n" << std::flush;
                    message = "ERROR: Missing key for GET\n";
                }
            } else if (command == "DELETE") {
                if (!key.empty()) {
                    std::cerr << "[DEBUG] Deleting key: '" << key << "'\n" << std::flush;
                    int result = kv_store->remove(key);
                    message = (result == 0) ? "OK\n" : "NOT_FOUND\n";
                } else {
                    std::cerr << "[ERROR] DELETE command missing key\n" << std::flush;
                    message = "ERROR: Missing key for DELETE\n";
                }
            } else if (command == "SET_RATE_LIMIT") {
                std::string type, rate_str, burst_str;
                iss >> type >> rate_str >> burst_str;
                try {
                    double rate = std::stod(rate_str);
                    double burst_size = std::stod(burst_str);
                    if (type == "client") {
                        client_rate_limiter.update(rate, burst_size);
                        message = "OK\n";
                    } else if (type == "node") {
                        server->rate_limiter.update(rate, burst_size);
                        message = "OK\n";
                    } else if (type == "global") {
                        server->gossip_rate_limiter.update_rate_limit(rate, burst_size);
                        message = "OK\n";
                    } else {
                        message = "ERROR: Invalid rate limit type\n";
                    }
                } catch (std::exception& e) {
                    message = "ERROR: Invalid rate limit parameters\n";
                }
            } else if (command == "STATS") {
                std::ostringstream oss;
                oss << "client_tokens:" << client_rate_limiter.get_tokens() << "\n"
                    << "node_tokens:" << server->rate_limiter.get_tokens() << "\n"
                    << "global_tokens:" << server->gossip_rate_limiter.get_tokens() << "\n"
                    << "rejected_requests:" << server->rejected_requests << "\n";
                message = oss.str();
            } else if (command == "EXIT") {
                std::cerr << "[DEBUG] Closing connection per client request\n" << std::flush;
                message = "CLOSINGsocket_\n";
                boost::asio::async_write(socket_, boost::asio::buffer(message),
                    boost::asio::bind_executor(strand_,
                        [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                            self->handle_write(error, bytes_transferred);
                        }));
                socket_.close();
                return;
            } else {
                std::cerr << "[ERROR] Unknown command: " << command << "\n" << std::flush;
                message = "ERROR: Unknown command\n";
            }
        }

        boost::asio::async_write(socket_, boost::asio::buffer(message),
            boost::asio::bind_executor(strand_,
                [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                    self->handle_write(error, bytes_transferred);
                }));
    } else {
        std::cerr << "[ERROR] Read error: " << error.message() << "\n" << std::flush;
        stop();
        socket_.close();
    }
}


void TCPConnection::handle_write(const boost::system::error_code& error, size_t bytes_transferred) 
{
    if (!error) {
        std::cerr << "[DEBUG] Command sent successfully\n" << std::flush;
        boost::asio::async_read_until(socket_, buffer, "\n",
            boost::asio::bind_executor(strand_,
                [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                    self->handle_read(error, bytes_transferred);
                }));
    } else {
        std::cerr << "[ERROR] Write error: " << error.message() << "\n";
        socket_.close();
    }
}


std::string TCPServer::forward_to_node(std::string_view address, std::string_view message) 
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
    TCPConnection::pointer new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(acceptor_.get_executor().context()), client_rate_limiter);
    acceptor_.async_accept(new_connection->socket(),
    [this, new_connection](const boost::system::error_code& error) {
            handle_accept(new_connection, error);
        });
}


void TCPServer::handle_accept(std::shared_ptr<TCPConnection> new_connection, const boost::system::error_code& error) 
{
    if (!error) {
        std::cout << "Client connected" << std::endl; 
        new_connection->start(store, this);
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
    start_accept();
}


void TCPServer::handle_client(tcp::socket socket) 
{
    auto new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(socket.get_executor().context()), client_rate_limiter);
    new_connection->socket() = std::move(socket);
    new_connection->start(store, this);
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
            if (acceptor_.is_open()) {
                boost::system::error_code ec;
                acceptor_.close();
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
    boost::asio::post(nodes_strand, [this, address = std::string(address)]() {
        active_nodes.insert(address);
    });
}


void TCPServer::remove_node(std::string_view address) 
{
    boost::asio::post(nodes_strand, [this, address = std::string(address)]() {
        active_nodes.erase(address);
    });
}


int main(int argc, char **argv) 
{
    try {   
        // if (argc < 3) {
        //     std::cerr << "USAGE: " << argv[0] << "<address> <port>\n";
        //     return 1;
        // 

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

