#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include <thread>
#include <mutex>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
// #include <atomic>    // TODO: Implement atomics to remove overhead

class TokenBucket
{
public:
    TokenBucket(double rate, double burst_size, std::chrono::milliseconds refill_period = std::chrono::milliseconds(100))
        : capacity(burst_size), rate(rate), tokens(burst_size), 
          last_updated(std::chrono::steady_clock::now()), refill_period(refill_period) {}

    bool consume(double tokens);
    void stop() {}
    double get_tokens() const;
    std::chrono::steady_clock::time_point get_last_updated() const;
    double get_rate() const { return rate; } 
    double get_capacity() const { return capacity; }
    std::string serialize() const;
    void deserialize(const std::string& state);
    void update(double new_rate, double new_capacity);

    double capacity, rate, tokens;

private:
    std::chrono::steady_clock::time_point last_updated;
    std::chrono::milliseconds refill_period;
    mutable std::mutex mutex_;
};

#endif      // TOKEN_BUCKET_H