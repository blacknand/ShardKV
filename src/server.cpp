#include "server.h"


void TCPConnection::start(KVStore& store, TCPServer* server, ConsistentHash* hash_ring) {
    kv_store = &store;
    _hash_ring = hash_ring;
    _server = &server;
    _socket.async_read_some(boost::asio::buffer(_buffer),
        boost::bind(&TCPConnection::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}


void TCPConnection::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::string request(_buffer.data(), bytes_transferred);
        std::string command, key, value;
        std::istringstream iss(request);
        iss >> command >> key;

        std::cerr << "Read: " << std::string(_buffer.data(), bytes_transferred) << "\n";

        // std::string responsible_node = _hash_ring->get_node(key);
        // if (responsible_node != _server->self_address) {
        //     std::string response = forward_to_node(responsible_node, request);
        //     _message = response + "\n";
        //     return;
        // }
      
        if (command == "JOIN" && iss >> key) {
            // If client is requesting to join active nodes
            _server->add_node(key);
            _message = "OK\n";
        } else if (command == "LEAVE" && iss >> key) {
            // If client is requesting to leave active nodes
            _server->remove_node(key);
            _message = "OK\n";
        } else if (command == "PUT" && iss >> value) {
            kv_store->put(key, value);
            _message = "OK\n";
        } else if (command == "GET") {
            std::string result = kv_store->get(key);
            if (result.empty()) { 
                _message = "NOT_FOUND\n"; 
            } else {
                _message = result + "\n";
            }
        } else if (command == "DELETE") {
            int result = kv_store->remove(key);
            _message = (result == 0) ? "OK\n" : "NOT_FOUND\n";
        } else if (command == "EXIT") {
            _message = "CLOSING_SOCKET\n";
            boost::asio::async_write(_socket, boost::asio::buffer(_message),
                boost::bind(&TCPConnection::handle_write, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
            _socket.close();
            return;
        } else {
            _message = "ERROR\n";
        }

        // Send respone
        boost::asio::async_write(_socket, boost::asio::buffer(_message),
                                    boost::bind(&TCPConnection::handle_write, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
    } else {
        _socket.close();
    }
}


void TCPConnection::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::cerr << "Wrote: " << _message << std::flush;
        _socket.async_read_some(boost::asio::buffer(_buffer),
            boost::bind(&TCPConnection::handle_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "Write error: " << error.message() << "\n";
        _socket.close();
    }
}


void TCPServer::start_accept() {
    // Static convert the _accept io_context to an io_context pointer
    TCPConnection::pointer new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(_acceptor.get_executor().context()));
    _acceptor.async_accept(new_connection->socket(),
        boost::bind(&TCPServer::handle_accept, this, new_connection,
                    boost::asio::placeholders::error));
}


void TCPServer::handle_accept(TCPConnection::pointer new_connection, const boost::system::error_code& error) {
    if (!error) {
        new_connection->start(_store, this);
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
    start_accept();
}


void TCPServer::handle_client(tcp::socket socket) {
    auto connection = TCPConnection::create(static_cast<boost::asio::io_context&>(socket.get_executor().context()));
    connection->socket() = std::move(socket);
    connection->start(_store);
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


int main() {
    try {
        boost::asio::io_context io_context;
        const int port = 5000;
        const int num_threads = std::thread::hardware_concurrency();

        TCPServer tcp_server(io_context, port);
        tcp_server.run_server(io_context, port, num_threads, "127.0.0.1:5000");
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

