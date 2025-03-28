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
        _socket.set_option(boost::asio::ip::tcp::no_delay(true));  // Disable Nagle’s
        std::string command;
        std::cout << "Enter command: ";
        std::getline(std::cin, command);
        boost::asio::async_write(_socket, boost::asio::buffer(command + "\n"),
            boost::bind(&TCPClient::handle_write, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } else {
        std::cerr << "Connect error: " << error.message() << "\n";
    }
}


void TCPClient::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        std::istream is(&_buffer);
            std::string response;
            std::getline(is, response); // Read until \n
            std::cout << "Received: " << response << "\n";

            if (response == "CLOSING_SOCKET") {
                _socket.close();            // Stop the client socket
                _io_context.stop();         // Stop the clients io_context
                return;
            }

            // Prompt for next command
            std::string command;
            std::cout << "Enter command: ";
            std::getline(std::cin, command);
            boost::asio::async_write(_socket, boost::asio::buffer(command + "\n"),
                boost::bind(&TCPClient::handle_write, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    } else if (error == boost::asio::error::eof) {
        std::cout << "Server closed connection\n";
        _socket.close();
        _io_context.stop();
    } else {
        std::cerr << "Read error: " << error.message() << "\n";
        _socket.close();
        _io_context.stop();
    }
}


void TCPClient::handle_write(const boost::system::error_code& error, size_t /*bytes_transferred*/) {
    if (!error) {
        boost::asio::async_read_until(_socket, _buffer, "\n",
                    boost::bind(&TCPClient::handle_read, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));

        std::cerr << "Wrote command\n";
    } else {
        std::cerr << "Write error: " << error.message() << "\n";
        _socket.close();
    }
}


int main(int argc, char **argv) {
    try {
        boost::asio::io_context io_context;
        TCPClient client(io_context, argv[0], argv[1]);
        client.start();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}