#ifndef SERVER_H
#define SERVER_H


#include "kv_store.h"
#include "consistent_hash.h"

#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>

// #include "../build/binary/shardkv.grpc.pb.h"
// #include <grpcpp/create_channel.h>
// #include <grpcpp/security/credentials.h>
// #include <grpcpp/grpcpp.h>
// #include "shardkv.grpc.ph.h"
// #include <grpcpp/grpcpp.h>

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <memory>
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
    // ~TCPServer();
    TCPServer(boost::asio::io_context& io_context, unsigned short port, const std::string &address)
        :   self_address(address), 
            _acceptor(io_context, tcp::endpoint(tcp::v4(), port)), 
            _nodes_strand(boost::asio::make_strand(io_context)),
            _io_context(io_context)
        {
            start_accept();
        }    

    void run_server(boost::asio::io_context& context, int port, int threads);
    void handle_client(tcp::socket socket);
    void add_node(const std::string_view address);
    void remove_node(const std::string_view address);
    
private:
    friend class TCPConnection;     // For _nodes_strand

    void start_accept();
    void handle_accept(std::shared_ptr<TCPConnection> new_connection, const boost::system::error_code& error);

    std::string self_address;   
    tcp::acceptor _acceptor;
    KVStore _store;                 
    std::unordered_set<std::string> active_nodes;       
    std::mutex node_mutex;
    boost::asio::strand<boost::asio::io_context::executor_type> _nodes_strand;
    boost::asio::io_context& _io_context;
};


class TCPConnection : public std::enable_shared_from_this<TCPConnection> 
{
public:
    // ~TCPConnection();
    typedef std::shared_ptr<TCPConnection> pointer;
    static pointer create(boost::asio::io_context& io_context) {
        return std::make_shared<TCPConnection>(io_context);
    }
    tcp::socket& socket() { return _socket; }
    void start(KVStore& store, TCPServer* server);

private:
    friend class std::allocator<TCPConnection>;     // Allow std::make_shared access private constructor
    TCPConnection(boost::asio::io_context& io_context) 
        : _socket(io_context), _strand(boost::asio::make_strand(io_context)) {}
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    std::string forward_to_node(std::string_view address, std::string_view message);

    tcp::socket _socket;
    std::string _message;
    boost::asio::streambuf _buffer;
    KVStore *kv_store = nullptr;
    TCPServer *_server = nullptr;
    ConsistentHash *_hash_ring = nullptr;
    boost::asio::strand<boost::asio::io_context::executor_type> _strand;
};

#endif  // SERVER_H