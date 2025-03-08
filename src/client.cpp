#include "client.h"

void TCPClient::start() {
    _io_context.run();
}

void TCPClient::handle_resolve(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator) {
    if (!error) {
        boost::asio::async_connect(_socket, endpoint_iterator,
            boost::bind(&TCPClient::handle_connect, this,
                        boost::asio::placeholders::error));
    } else {
        std::cerr << "Resolve error: " << error.message() << "\n";
    }
}

void TCPClient::handle_connect(const boost::system::error_code& error) {
    if (!error) {
        boost::asio::async_read(_socket, boost::asio::buffer(_buffer),
            boost::bind(&TCPClient::handle_read, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "Connect error: " << error.message() << "\n";
    }
}

void TCPClient::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error || error == boost::asio::error::eof) {
        std::cout << "Received: ";
        std::cout.write(_buffer.data(), bytes_transferred);
        std::cout << "\n";
        _socket.close();
    } else if (error != boost::asio::error::eof) {
        std::cerr << "Read error: " << error.message() << "\n";
    } else {
        std::cout << "Server closed connection\n";
        _socket.close();
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        TCPClient client(io_context, "127.0.0.1", "8080");
        client.start();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}