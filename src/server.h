#ifndef SERVER_H
#define SERVER_H


#include "kv_store.h"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <mutex>
#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>


using boost::asio::ip::tcp;

std::mutex console_mutex;


class TCPConnection : public boost::enable_shared_from_this<TCPConnection> {
public:
    typedef boost::shared_ptr<TCPConnection> pointer;
    static pointer create(boost::asio::io_context& io_context) {
        return pointer(new TCPConnection(io_context));
    }
    tcp::socket& socket() { return _socket; }
    void start(KVStore& store);

private:
    TCPConnection(boost::asio::io_context& io_context) 
        : _socket(io_context), _buffer(1024) {}
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

    tcp::socket _socket;
    std::string _message;
    std::vector<char> _buffer;
    KVStore *kv_store;
};


class TCPServer {
public:
    // Initialise exceptor with io_context for main event loop and IP address with port number
    TCPServer(boost::asio::io_context& io_context, unsigned short port)
        : _acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }    

    void run_server(boost::asio::io_context& context, int port, int threads);
    void handle_client(tcp::socket socket);

private:
    void start_accept();
    void handle_accept(TCPConnection::pointer new_connection, const boost::system::error_code& error);

    tcp::acceptor _acceptor;
    KVStore _store;                 // Server owns KVStore object which is passed to TCPConnection
};


#endif  // SERVER_H