#include "token_bucket.h"


bool TokenBucket::consume(double tokens)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_updated_).count();
    double elapsed_seconds = elapsed_ms / 1000.0;

    tokens_ = std::min(capacity_, tokens_ + rate_ * elapsed_seconds);
    if (tokens_ >= tokens) {
        tokens_ -= tokens;
        last_updated_ = now;
        std::cerr << "[DEBUG] TokenBucket: consumed " << tokens << ", remaining=" << tokens_ << "\n" << std::flush;
        return true;
    }

    std::cerr << "[DEBUG] TokenBucket: insufficient tokens\n" << std::flush;
    return false;
}