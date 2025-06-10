#ifndef TOCKET_BUCKET_H
#define TOCKET_BUCKET_H

#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <mutex>

class TokenBucket
{
public:
    TokenBucket(boost::asio::io_context& io_context, double rate, double burst_size)
        :   rate_(rate),                    // Number of tokens added per second
            capacity_(burst_size),          // Maximum number of tokens bucket can hold
            tokens_(burst_size),            // Bucket of tokens
            timer_(io_context),
            interval_(static_cast<int>(100.0 / rate * 10.0)) 
    {
        start_timer();
    }

    bool consume(double tokens);
    void stop();
private:
    void start_timer();
    void add_tokens();
    double capacity_, rate_, tokens_;
    int interval_;
    std::mutex mutex_;
    boost::asio::steady_timer timer_;
};

#endif      // TOCKET_BUCKET_H