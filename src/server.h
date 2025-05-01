#ifndef SERVER_H
#define SERVER_H


#include "kv_store.h"
#include "consistent_hash.h"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/algorithm/string.hpp>

// #include "../build/binary/shardkv.grpc.pb.h"
// #include <grpcpp/create_channel.h>
// #include <grpcpp/security/credentials.h>
// #include <grpcpp/grpcpp.h>
// #include "shardkv.grpc.ph.h"
// #include <grpcpp/grpcpp.h>

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>


using boost::asio::ip::tcp;

std::mutex console_mutex;
class TCPConnection;


class TCPServer 
{
public:
    TCPServer(boost::asio::io_context& io_context, unsigned short port, const std::string &address)
        : _acceptor(io_context, 
          tcp::endpoint(tcp::v4(), port)), 
          self_address(address), 
          _nodes_strand(boost::asio::make_strand(io_context))
        {
            start_accept();
        }    

    void run_server(boost::asio::io_context& context, int port, int threads);
    void handle_client(tcp::socket socket);
    void add_node(const std::string &address);
    void remove_node(const std::string &address);
    
    std::string self_address;

private:
    friend class TCPConnection;     // For _nodes_strand

    void start_accept();
    void handle_accept(boost::shared_ptr<TCPConnection> new_connection, const boost::system::error_code& error);

    tcp::acceptor _acceptor;
    KVStore _store;                 
    std::unordered_set<std::string> active_nodes;       
    std::mutex node_mutex;
    boost::asio::strand<boost::asio::io_context::executor_type> _nodes_strand;
};


class TCPConnection : public boost::enable_shared_from_this<TCPConnection> 
{
public:
    typedef boost::shared_ptr<TCPConnection> pointer;
    static pointer create(boost::asio::io_context& io_context) {
        return pointer(new TCPConnection(io_context));
    }
    tcp::socket& socket() { return _socket; }
    void start(KVStore& store, TCPServer* server);

    tcp::socket _socket;
    std::string _message;
    std::vector<char> _buffer;
    KVStore *kv_store = nullptr;
    TCPServer *_server = nullptr;
    ConsistentHash *_hash_ring = nullptr;

private:
    TCPConnection(boost::asio::io_context& io_context) 
        : _socket(io_context), _buffer(1024) {}
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    std::string forward_to_node(const std::string &address, const std::string &message);
};

#endif  // SERVER_H