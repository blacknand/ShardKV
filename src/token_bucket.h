#ifndef TOCKET_BUCKET_H
#define TOCKET_BUCKET_H

#include <thread>
#include <mutex>

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

private:
    double capacity_, rate_, tokens_;
    std::chrono::steady_clock::time_point last_updated_;
    std::mutex mutex_;
};

#endif      // TOCKET_BUCKET_H