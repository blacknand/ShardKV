#include "server.h"


void TCPConnection::start(KVStore& store, TCPServer* server) 
{
    kv_store = &store;
    _server = server;
    // Start async reading of data from the client immediately
    _socket.async_read_some(boost::asio::buffer(_buffer),
        boost::bind(&TCPConnection::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}


void TCPConnection::handle_read(const boost::system::error_code& error, size_t bytes_transferred) 
{
    if (!error) {
        std::string request(_buffer.data(), bytes_transferred);
        request = boost::algorithm::trim_copy(request);

        if (request.empty()) {
            _socket.async_read_some(boost::asio::buffer(_buffer),
                boost::bind(&TCPConnection::handle_read, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            return;
        }

        std::cerr << "[DEBUG] Raw request: " << request << "\n" << std::flush;
        std::string command, key, value;
        std::istringstream iss(request);
        iss >> command >> key;

        std::cerr << "[DEBUG] Parsed command: '" << command << ", key: " << key << "'\n" << std::flush;

        if (command == "JOIN") {
            _message = "JOIN command recieved\n";
        } else if (command == "LEAVE") {
            if (!key.empty()) {
                // If client is requesting to leave active nodes
                std::cerr << "[DEBUG] Removing node: " << key << "\n" << std::flush;
                boost::asio::post(_server->_nodes_strand, [this, key]() {
                    _server->remove_node(key);
                    _message = "OK\n";
                    boost::asio::async_write(_socket, boost::asio::buffer(_message),
                        boost::bind(&TCPConnection::handle_write, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
                });
                return;
            } else {
                std::cerr << "[ERROR] LEAVE command missing address\n" << std::flush;
                _message = "ERROR: Missing address for LEAVE\n";
            }
        } else if (command == "PUT") {
            std::string rest_of_line;
            std::getline(iss, rest_of_line);
            value = boost::algorithm::trim_copy(rest_of_line);

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
                boost::bind(&TCPConnection::handle_write, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            _socket.close();
            return;
        } else {
            std::cerr << "[ERROR] Unkown command: " << command << "\n" << std::flush;
            _message = "ERROR: Unkown command\n";
        }

        // Send respone
        std::cerr << "[DEBUG] Sending response: " << _message << "\n" << std::flush;
        boost::asio::async_write(_socket, boost::asio::buffer(_message),
                                    boost::bind(&TCPConnection::handle_write, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "[ERROR] Read error: " << error.message() << "\n" << std::flush;
        _socket.close();
    }
}


void TCPConnection::handle_write(const boost::system::error_code& error, size_t bytes_transferred) 
{
    if (!error) {
        std::cerr << "[DEBUG] Command sent successfully\n" << std::flush;
        // _buffer.clear();    
        _socket.async_read_some(boost::asio::buffer(_buffer),
            boost::bind(&TCPConnection::handle_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "[ERROR] Write error: " << error.message() << "\n";
        _socket.close();
    }
}


// IGNORE FOR NOW
std::string TCPConnection::forward_to_node(const std::string &address, const std::string &message) 
{
    try {
        size_t colon = address.find(':');
        std::cerr << "[FORWARD] Attempting to resolve: " << address << "\n";
        std::cerr << std::flush;
        std::string host = boost::algorithm::trim_copy(address.substr(0, colon));
        std::string port = boost::algorithm::trim_copy(address.substr(colon + 1));

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
    } catch (std::exception& e) {
        std::cerr << "Forwarding error: " << e.what() << "\n";
        return "ERROR_FORWARDING";
    }
}


void TCPServer::start_accept() 
{
    TCPConnection::pointer new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(_acceptor.get_executor().context()));
    _acceptor.async_accept(new_connection->socket(),
        boost::bind(&TCPServer::handle_accept, this, new_connection,
                    boost::asio::placeholders::error));
}


void TCPServer::handle_accept(boost::shared_ptr<TCPConnection> new_connection, const boost::system::error_code& error) 
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
    std::vector<std::thread> thread_pool;
    for (int i = 0; i < threads; ++i) {
        thread_pool.emplace_back([&context]() { context.run(); });
    }

    std::cout << "ShardKV server listening on port " << port << " with " << threads << " worker threads." << std::endl;

    for (auto& thread : thread_pool) {
        thread.join();
    }
}


void TCPServer::add_node(const std::string &address) 
{
    boost::asio::post(_nodes_strand, [this, address]() {
        active_nodes.insert(address);
    });
}


void TCPServer::remove_node(const std::string &address) 
{
    boost::asio::post(_nodes_strand, [this, address]() {
        active_nodes.erase(address);
    });
}


int main(int argc, char **argv) 
{
    try {   
        boost::asio::io_context io_context;
        const int port = atoi(argv[1]);
        const std::string address = argv[2];
        const int num_threads = std::thread::hardware_concurrency();

        TCPServer tcp_server(io_context, port, address);
        tcp_server.run_server(io_context, port, num_threads);
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

