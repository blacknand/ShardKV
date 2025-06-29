#ifndef GOSSIP_RATE_LIMITER_H
#define GOSSIP_RATE_LIMITER_H

#include "token_bucket.h"

#include <boost/asio.hpp>
#include <random>
#include <map>
#include <mutex>
#include <memory>

class KVTCPServer;
class GossipRateLimiter : public std::enable_shared_from_this<GossipRateLimiter>
{
public:
    GossipRateLimiter(double rate, double burst_size, KVTCPServer* server,
                        boost::asio::io_context& io_context,
                        std::chrono::milliseconds gossip_interval = std::chrono::milliseconds(100))
    :   local_bucket(rate, burst_size), server(server), gossip_interval(gossip_interval),
        timer_(io_context), rng(std::random_device{}()), dist(0, 1.0) {}

    bool consume(double tokens);
    void start();
    void stop();
    void recive_state(const std::string& address, const std::string& state);
    void update_rate_limit(double rate, double burst_size);
    // void update(double new_rate, double new_capacity);
    double get_tokens() const;

private:
    void gossip();
    void broadcast_state();

    TokenBucket local_bucket;
    KVTCPServer* server;
    std::chrono::milliseconds gossip_interval;
    boost::asio::steady_timer timer_;
    std::map<std::string, std::pair<double, std::chrono::steady_clock::time_point>> nodes_states;
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> dist;
    mutable std::mutex state_mutex;
};

#endif  // GOSSIP_RATE_LIMITER_H
