#include "token_bucket.h"

// TODO: Remove all this overhead with locking mutexes

bool TokenBucket::consume(double tokens)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_updated).count();
    double elapsed_seconds = elapsed_ms / 1000.0;

    tokens = std::min(capacity, tokens + rate * elapsed_seconds);
    if (tokens >= tokens) {
        tokens -= tokens;
        last_updated = now;
        std::cerr << "[DEBUG] TokenBucket: consumed " << tokens << ", remaining=" << tokens << "\n" << std::flush;
        return true;
    }

    std::cerr << "[DEBUG] TokenBucket: insufficient tokens\n" << std::flush;
    return false;
}


double TokenBucket::get_tokens() const 
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_updated).count();
    double elapsed_seconds = elapsed_ms / 1000.0;
    return std::min(capacity, tokens + rate * elapsed_seconds);
}


std::chrono::steady_clock::time_point TokenBucket::get_last_updated() const 
{
    std::lock_guard<std::mutex> lock(mutex_);
    return last_updated;
}


std::string TokenBucket::serialize() const 
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << tokens << "," << last_updated.time_since_epoch().count() << "," << rate << "," << capacity;
    return oss.str();
}


void TokenBucket::deserialize(const std::string& state) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::istringstream iss(state);
    std::string token_count, timestamp, rate_str, capacity_str;
    std::getline(iss, token_count, ',');
    std::getline(iss, timestamp, ',');
    std::getline(iss, rate_str, ',');
    std::getline(iss, capacity_str, ',');
    try {
        tokens = std::stod(token_count);
        last_updated = std::chrono::steady_clock::time_point(
            std::chrono::steady_clock::duration(std::stol(timestamp)));
        rate = std::stod(rate_str);
        capacity = std::stod(capacity_str);
    } catch (std::exception& e) {
        std::cerr << "[ERROR] TokenBucket deserialize failed: " << e.what() << "\n" << std::flush;
        // Reset to defaults on error
        tokens = capacity;
        last_updated = std::chrono::steady_clock::now();
    }
}


void TokenBucket::update(double new_rate, double new_capacity)
{
    std::lock_guard<std::mutex> lock(mutex_);
    rate = new_rate;
    capacity = new_capacity;
    tokens = std::min(tokens, capacity);
    last_updated = std::chrono::steady_clock::now();
}