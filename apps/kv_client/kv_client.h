#ifndef KV_TCP_CLIENT_H
#define KV_TCP_CLIENT_H

#include <boost/asio.hpp>

#include <iostream>
#include <functional>
#include <string>
#include <fcntl.h>
#include <unistd.h>

using boost::asio::ip::tcp;


class KVTCPClient 
{
public:
    KVTCPClient(boost::asio::io_context& io_context, const std::string& host, const std::string& port)
        : _io_context(io_context), _socket(io_context), _resolver(io_context) {
        _resolver.async_resolve(host, port,
            [this](const boost::system::error_code& error, tcp::resolver::results_type results) {
                handle_resolve(error, results);
            });
    }
    void start();

private:
    void handle_resolve(const boost::system::error_code& error, tcp::resolver::results_type endpoint_iterator);
    void handle_connect(const boost::system::error_code& error);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    boost::asio::io_context& _io_context;
    tcp::resolver _resolver;
    tcp::socket _socket;
    boost::asio::streambuf _buffer;     // For dynamic sizing
};


#endif  // KV_TCP_CLIENT_H