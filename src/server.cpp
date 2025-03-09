#include "server.h"


void TCPConnection::start(KVStore& store) {
    kv_store = &store;
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
        
        if (command == "PUT" && iss >> value) {
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
        new_connection->start(_store);
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
    start_accept();
}


int main() {
    try {
        // Create the Boost.asio event loop and pass to the server
        boost::asio::io_context io_context;
        TCPServer server(io_context, 8080);
        std::cout << "Server running on port 8080\n";
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}