#include "server.h"
#include "kv_store.h"


void TCPConnection::start() {
    _message = "test message";
    boost::asio::async_write(_socket, boost::asio::buffer(_message),
        boost::bind(&TCPConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}


void TCPConnection::handle_write(const boost::system::error_code& error, size_t /*bytes_transferred*/) {
    if (!error) {
        std::cout << "Message sent to client\n";
    } else {
        std::cerr << "Write error: " << error.message() << "\n";
    }
    _socket.close();
}


void TCPServer::start_accept() {
    TCPConnection::pointer new_connection = TCPConnection::create(static_cast<boost::asio::io_context&>(_acceptor.get_executor().context()));
    _acceptor.async_accept(new_connection->socket(),
        boost::bind(&TCPServer::handle_accept, this, new_connection,
                    boost::asio::placeholders::error));
}


void TCPServer::handle_accept(TCPConnection::pointer new_connection, const boost::system::error_code& error) {
    if (!error) {
        new_connection->start();
    } else {
        std::cerr << "Accept error: " << error.message() << "\n";
    }
    start_accept();
}


int main() {
    try {
        boost::asio::io_context io_context;
        TCPServer server(io_context, 8080);
        std::cout << "Server running on port 8080\n";
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}