#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include <thread>
#include <mutex>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>

class TokenBucket
{
public:
    TokenBucket(double rate, double burst_size)
        :   capacity_(burst_size),          // Maximum number of tokens bucket can hold
            rate_(rate),                    // Number of tokens added per second
            tokens_(burst_size),            // Bucket of tokens
            last_updated_(std::chrono::steady_clock::now()) 
    {}

    bool consume(double tokens);
    void stop() {}
    double get_tokens() const;
    std::chrono::steady_clock::time_point get_last_updated() const;
    std::string serialize() const;
    void deserialize(const std::string& state);

private:
    double capacity_, rate_, tokens_;
    std::chrono::steady_clock::time_point last_updated_;
    std::chrono::milliseconds refill_period_;
    mutable std::mutex mutex_;
};

#endif      // TOKEN_BUCKET_H