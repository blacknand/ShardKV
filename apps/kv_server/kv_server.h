#ifndef KV_TCP_SERVER_H
#define KV_TCP_SERVER_H

#include "~/libs/kv_store/kv_store.h"
#include "~/libs/consistent_hashing/consistent_hash.h"
#include "~/libs/rate_limiter/token_bucket.h"
#include "~/libs/rate_limiter/gossip_rate_limiter.h"

#include <boost/asio.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/executor_work_guard.hpp>

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
#include <string_view>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>


using boost::asio::ip::tcp;

extern std::mutex console_mutex;
class KVTCPConnection;


class KVTCPServer 
{
public:
    KVTCPServer(boost::asio::io_context& io_context, unsigned short port, const std::string &address)
        :   self_address(address), 
            acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), 
            nodes_strand(boost::asio::make_strand(io_context)),
            io_context_(io_context),
            rate_limiter(100.0, 200.0),
            client_rate_limiter(10.0, 50.0),
            gossip_rate_limiter(std::make_shared<GossipRateLimiter>(1000.0, 2000.0, this, io_context))
        {
            start_accept();
            gossip_rate_limiter->start();
        }    

    void run_server(boost::asio::io_context& context, int port, int threads);
    void handle_client(tcp::socket socket);
    void add_node(const std::string_view address);
    void remove_node(const std::string_view address);
    std::string forward_to_node(std::string_view address, std::string_view message);

    std::unordered_set<std::string> active_nodes;       
    std::string self_address;   
    
private:
    friend class KVTCPConnection;     // For _nodes_strand

    void start_accept();
    void handle_accept(std::shared_ptr<KVTCPConnection> new_connection, const boost::system::error_code& error);

    tcp::acceptor acceptor_;
    KVStore store;                 
    std::mutex node_mutex;
    boost::asio::strand<boost::asio::io_context::executor_type> nodes_strand;
    boost::asio::io_context& io_context_;
    TokenBucket rate_limiter, client_rate_limiter;      
    std::shared_ptr<GossipRateLimiter> gossip_rate_limiter;      // Global rate limiter
    size_t rejected_requests = 0;
};


class KVTCPConnection : public std::enable_shared_from_this<KVTCPConnection> 
{
public:
    typedef std::shared_ptr<KVTCPConnection> pointer;
    static pointer create(boost::asio::io_context& io_context, const TokenBucket& client_rate_limit) {
        return std::make_shared<KVTCPConnection>(io_context, client_rate_limit);
    }
    tcp::socket& socket() { return socket_; }
    void start(KVStore& store, KVTCPServer* server);
    void stop() { client_rate_limiter.stop(); }

private:
    friend class std::allocator<KVTCPConnection>;     // Allow std::make_shared access private constructor
    KVTCPConnection(boost::asio::io_context& io_context, const TokenBucket& client_rate_limit) 
        : socket_(io_context), strand_(boost::asio::make_strand(io_context)), 
            client_rate_limiter(client_rate_limit.rate, client_rate_limit.get_capacity()) {}
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);

    tcp::socket socket_;
    std::string message;
    boost::asio::streambuf buffer;
    KVStore *kv_store = nullptr;
    KVTCPServer *server = nullptr;
    ConsistentHash *hash_ring = nullptr;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    TokenBucket client_rate_limiter;
};

#endif  // KV_TCP_SERVER_H