#include "token_bucket.h"

// TODO: Remove all this overhead with locking mutexes

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


double TokenBucket::get_tokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_updated_).count();
    double elapsed_seconds = elapsed_ms / 1000.0;
    return std::min(capacity_, tokens_ + rate_ * elapsed_seconds);
}


std::chrono::steady_clock::time_point TokenBucket::get_last_updated() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_updated_;
}


std::string TokenBucket::serialize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << tokens_ << "," << last_updated_.time_since_epoch().count();
    return oss.str();
}


void TokenBucket::deserialize(const std::string& state) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::istringstream iss(state);
    std::string token_count, timestamp;
    std::getline(iss, token_count, ',');
    std::getline(iss, timestamp, ',');
    tokens_ = std::stod(token_count);
    last_updated_ = std::chrono::steady_clock::time_point(
        std::chrono::steady_clock::duration(std::stol(timestamp)));
}