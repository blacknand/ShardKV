#include "token_bucket.h"


bool TokenBucket::consume(double tokens)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        return true;
    }
    return false;
}


void TokenBucket::start_timer()
{
    timer_.expires_after(std::chrono::milliseconds(interval_));
    timer_.async_wait([this](const boost::system::error_code& ec) {
        if (!ec) {
            add_tokens();
            start_timer();
        }
    });
}


void TokenBucket::stop() 
{
    boost::system::error_code ec;
    timer_.cancel();
}


void TokenBucket::add_tokens() 
{
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = std::min(capacity_, tokens_ + (rate_ / (1000.0 / interval_)));
}