#include "gossip_rate_limiter.h"
#include "server.h"


void GossipRateLimiter::start()
{
    timer_.expires_after(gossip_interval);
    gossip();
}


void GossipRateLimiter::stop()
{
    boost::system::error_code ec;
    timer_.cancel();
    if (ec)
        std::cerr << "[ERROR] Failed to cancel gossip timer: " << ec.message() << "\n" << std::flush;
}


void GossipRateLimiter::gossip() 
{
    broadcast_state();
    timer_.expires_after(gossip_interval);
    timer_.async_wait([self = shared_from_this()](const boost::system::error_code& error) {
        if (!error) 
            self->gossip();
        else
            std::cerr << "[ERROR] Gossip timer error: " << error.message() << "\n" << std::flush;
    });
}


void GossipRateLimiter::broadcast_state()
{
    std::lock_guard<std::mutex> lock(state_mutex);
    std::string state = local_bucket.serialize();

    // Randomly select k nodes to send the state to
    std::vector<std::string> targets;
    for (const auto& node : server->active_nodes) {
        if (dist(rng) < 0.3 && node != server->self_address)  {
            targets.emplace_back(node);
            if (targets.size() >= 3)
                break;
        }
    }

    for (const auto& target : targets)
        server->forward_to_node(target, "GOSSIP_RATE_LIMIT " + state);
}


void GossipRateLimiter::recieve_state(const std::string& address, const std::string& state)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    try {
        TokenBucket tmp(local_bucket.get_rate(), local_bucket.get_capacity());
        tmp.deserialize(state);
        nodes_states[address] = {tmp.get_tokens(), tmp.get_last_updated()};
    } catch (std::exception& e) {
        std::cerr << "[ERROR] Failed to deserialize gossip state from " << address << ": " << e.what() << "\n" << std::flush;
    }
}


bool GossipRateLimiter::consume(double tokens)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    double global_tokens = local_bucket.get_tokens();
    int count = 1;
    auto now = std::chrono::steady_clock::now();

    for (auto it = nodes_states.begin(); it != nodes_states.end();) {
        auto& [addr, state] = *it;
        auto [node_tokens, last_updated] = state;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_updated).count();
        if (elapsed_ms >= 5000) {
            it = nodes_states.erase(it);
            continue;
        }
        double elapsed_seconds = elapsed_ms / 1000.0;
        global_tokens += std::min(local_bucket.get_capacity(), node_tokens + local_bucket.get_rate() * elapsed_seconds);
        count++;
        ++it;
    }

    global_tokens /= count;
    if (global_tokens >= tokens)
        return local_bucket.consume(tokens);

    return false;
}


double GossipRateLimiter::get_tokens() const 
{
    std::lock_guard<std::mutex> lock(state_mutex);
    double global_tokens = local_bucket.get_tokens();
    int count = 1;
    auto now = std::chrono::steady_clock::now();

    for (const auto& [addr, state] : nodes_states) {
        auto [nodes_tokens, last_updated] = state;
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_updated).count();
        double elapsed_seconds = elapsed_ms / 1000.0;
        global_tokens += std::min(local_bucket.get_capacity(), nodes_tokens + local_bucket.get_rate() * elapsed_seconds);
        count++;
    }

    return global_tokens / count;
}


void GossipRateLimiter::update_rate_limit(double rate, double burst_size)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    local_bucket.update(rate, burst_size);
    nodes_states.clear();
}


