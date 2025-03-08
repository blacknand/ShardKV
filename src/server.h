#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;

class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
public:
    typedef boost::shared_ptr<TCPConnection> pointer;
    static pointer create(boost::asio::io_context& io_context) {
        return pointer(new TCPConnection(io_context));
    }
    tcp::socket& socket() { return _socket; }
    void start();

private:
    TCPConnection(boost::asio::io_context& io_context) : _socket(io_context) {}
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    tcp::socket _socket;
    std::string _message;
};

class TCPServer {
public:
    TCPServer(boost::asio::io_context& io_context, unsigned short port)
        : _acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }    

private:
    void start_accept();
    void handle_accept(TCPConnection::pointer new_connection, const boost::system::error_code& error);
    tcp::acceptor _acceptor;
};

#endif  // SERVER_H