#ifndef CLIENT_H
#define CLIENT_H

// Boost headers
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind/bind.hpp>

// gRPC headers
// #include "shardkv.ph.h"
// #include "shardkv.grpc.ph.h"
// #include <grpcpp/grpcpp.h>

// C++ std headers
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>

using boost::asio::ip::tcp;


class TCPClient {
public:
    TCPClient(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
        : _io_context(io_context), _socket(io_context), _resolver(io_context) {
        tcp::resolver::query query(host, port);
        _resolver.async_resolve(query,
            boost::bind(&TCPClient::handle_resolve, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::iterator));
    }
    void start();

private:
    void handle_resolve(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
    void handle_connect(const boost::system::error_code& error);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    boost::asio::io_context& _io_context;
    tcp::resolver _resolver;
    tcp::socket _socket;
    boost::asio::streambuf _buffer;     // For dynamic sizing
};


#endif  // CLIENT_H