#include "server.h"


void TCPConnection::start(KVStore& store, TCPServer* server, ConsistentHash* hash_ring) {
    kv_store = &store;
    _hash_ring = hash_ring;
    _server = server;
    _socket.async_read_some(boost::asio::buffer(_buffer),
        boost::bind(&TCPConnection::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}


void TCPConnection::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
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
            // If client is requesting to join active nodes
            if (!key.empty()) {
                std::cerr << "[DEBUG] Joining node: " << key << "\n" << std::flush;
                _server->add_node(key);
                _hash_ring->add_node(key);
                _message = "OK\n";
            } else {
                std::cerr << "[ERROR] JOIN command missing address\n" << std::flush;
                _message = "[ERROR] Missing address for JOIN\n";
            }
        } else if (command == "LEAVE") {
            if (!key.empty()) {
                // If client is requesting to leave active nodes
                std::cerr << "[DEBUG] Removing node: " << key << "\n" << std::flush;
                _server->remove_node(key);
                _hash_ring->remove_node(key);
                _message = "OK\n";
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
                std::string responsible_node = _hash_ring->get_node(key);   // Node that stores the key
                std::cerr << "[DEBUG] Responsible node for key: '" << key << "': " << responsible_node << "\n" << std::flush;

                if (responsible_node != _server->self_address) {
                    std::cerr << "[DEBUG] Forwarding PUT to node: " << responsible_node << "\n" << std::flush;
                    _message = forward_to_node(responsible_node, request) + "\n";
                } else {
                    std::cerr << "[DEBUG] Storing key locally\n" << std::flush;
                    kv_store->put(key, value);
                    _message = "OK\n";
                }
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


void TCPConnection::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::cerr << "[DEBUG] Command sent successfully\n" << std::flush;
        _buffer.clear();    // Clear buffer before reading response
        _socket.async_read_some(boost::asio::buffer(_buffer),
            boost::bind(&TCPConnection::handle_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "[ERROR] Write error: " << error.message() << "\n";
        _socket.close();
    }
}


std::string TCPConnection::forward_to_node(const std::string &address, const std::string &message) {
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
        
        std::cerr << "[DEBUG] Forwarding to host: " << host << ", port: " << port << "\n";
        std::cerr << std::flush;

        // NOTE: This is a blocking call, if we scaled to thousands of clients this
        // will need to be asynchronous

        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);

        std::cerr << "[DEBUG] Raw address: " << address << "\n";
        std::cerr << "[DEBUG] Parsed host: " << host << "\n";
        std::cerr << "[DEBUG] Parsed port: " << port << "\n";
        std::cerr << "[DEBUG] Resolving..." << std::flush << "\n";

        auto endpoints = resolver.resolve(tcp::resolver::query{
            host,
            port,
            boost::asio::ip::resolver_query_base::numeric_service
        });

        std::cerr << "[DEBUG] Resolved endpoints successfully" << std::flush << "\n";

        // Connect the endpoints by creating a socket
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        boost::asio::write(socket, boost::asio::buffer(message + "\n"));
        boost::asio::streambuf response_buf;
        boost::asio::read_until(socket, response_buf, '\n');

        std::istream is(&response_buf);
        std::string response;
        std::getline(is, response);
        return response;
    } catch (std::exception& e) {
        std::cerr << "Forwarding error: " << e.what() << "\n";
        return "ERROR_FORWARDING";
    }
}


void TCPServer::start_accept() {
    // Static convert the _accept io_context to an io_context pointer
    TCPConnection::pointer new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(_acceptor.get_executor().context()));
    _acceptor.async_accept(new_connection->socket(),
        boost::bind(&TCPServer::handle_accept, this, new_connection,
                    boost::asio::placeholders::error));
}


void TCPServer::handle_accept(boost::shared_ptr<TCPConnection> new_connection, const boost::system::error_code& error) {
    if (!error) {
        new_connection->start(_store, this, &_hash_ring);
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
    start_accept();
}


void TCPServer::handle_client(tcp::socket socket) {
    auto new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(socket.get_executor().context()));
    new_connection->socket() = std::move(socket);
    // pretty sure _store needs to be a KVStore *
    new_connection->start(_store, this, &_hash_ring);
}


void TCPServer::run_server(boost::asio::io_context& context, int port, int threads) {
    std::vector<std::thread> thread_pool;
    for (int i = 0; i < threads; ++i) {
        thread_pool.emplace_back([&context]() { context.run(); });
    }

    std::cout << "ShardKV server listening on port " << port << " with " << threads << " worker threads." << std::endl;

    // Wait for threads to finish
    for (auto& thread : thread_pool) {
        thread.join();
    }
}


void TCPServer::add_node(const std::string &address) {
    std::lock_guard<std::mutex> lock(node_mutex);
    active_nodes.insert(address);
}


void TCPServer::remove_node(const std::string &address) {
    active_nodes.erase(address);
}


int main(int argc, char **argv) {
    try {   
        int fd = open("debug.log", O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd == -1) {
            perror("open");
            return 1;
        }

        if (dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2");
            return 1;
        }

        close(fd);
        std::cerr << "Message should now appear in debug.log\n";

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

