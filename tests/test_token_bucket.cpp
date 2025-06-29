#include "~/apps/kv_server/kv_server.h"

#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <thread>
#include <chrono>
#include <future>

class TokenBucketTest : public ::testing::Test
{
protected:
    TokenBucket bucket{10.0, 20.0, std::chrono::milliseconds(100)};
};

TEST_F(TokenBucketTest, ConsumeTokens)
{
    ASSERT_TRUE(bucket.consume(5.0)) << "Could not consume 5 tokens";
    ASSERT_DOUBLE_EQ(bucket.get_tokens(), 15.0) << "Tokens were not updated correctly";
    ASSERT_FALSE(bucket.consume(16.0)) << "Consumed more tokens than were available";
}

TEST_F(TokenBucketTest, RefillTokens)
{
    bucket.consume(10.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    double tokens = bucket.get_tokens();
    ASSERT_GE(tokens, 12.0) << "Tokens not refilled";
    ASSERT_LE(tokens, 20.0) << "Tokens exceeded capacity: " << tokens;
}

TEST_F(TokenBucketTest, SerializeDeseralize)
{
    bucket.consume(5.0);
    std::string state = bucket.serialize();
    TokenBucket new_bucket(10.0, 20.0);
    new_bucket.deserialize(state);
    ASSERT_DOUBLE_EQ(new_bucket.get_tokens(), 15.0) << "Deserialization failed";
}

TEST_F(TokenBucketTest, UpdateTokens)
{
    bucket.update(20.0, 20.0);
    ASSERT_DOUBLE_EQ(bucket.get_tokens(), 20.0) << "Tokens not updated correctly, expected to have 20 but actually has " << bucket.get_tokens();
}


TEST_F(TokenBucketTest, SimulateOverflow)
{
    double current_tokens = bucket.get_tokens();
    bucket.update(200.0, 100.0);
    ASSERT_DOUBLE_EQ(current_tokens, bucket.get_tokens()) << "Number of tokens exceeded capacity";
}


TEST_F(TokenBucketTest, TestConcurrency)
{
    boost::asio::thread_pool pool(10);
    std::vector<std::future<bool>> futures;
    bucket.update(5.0, 100.0);

    for (int i = 0; i < 10; i++) {
        std::promise<bool> p;
        futures.push_back(p.get_future());
        boost::asio::post(pool, [this, p = std::move(p)]() mutable {
            p.set_value(bucket.consume(1.0));
        });
    }

    int successful_consumes = 0;
    for (auto f& : futures) {
        if (f.get())
            successful_consumes++;
    }

    ASSERT_EQ(successful_consumes, 5);

    pool.join();
}


TEST_F(TokenBucketTest, TestBoundaries)
{
    // Zero consumption
    ASSERT_TRUE(bucket.consume(0));
    int tokens = bucket.get_tokens();
    ASSERT_TRUE(bucket.consume(tokens));

    // Exceed consumption and capcity limit
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    tokens = bucket.get_tokens();
    ASSERT_DOUBLE_LE(tokens, 100.0);

    // Update to lower capacity
    bucket.update(5.0, 20.0);
    tokens = tokens.get_tokens();
    ASSERT_DOUBLE_EQ(tokens, 20.0);
}


TEST_F(TokenBucketTest, ErrorHandlingDeserialization)
{
    bucket.update(5.0, 100);
    // Generate random string and deserialize to check bucket is in sensible state
    std::string random_string( size_t length )
    {
        // https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
        auto randchar = []() -> char
        {
            const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str;
    }

    for (int i = 0; i < 5; i++)
        bucket.deserialize(random_string(i + 10));

    int tokens = bucket.get_tokens();
    ASSERT_DOUBLE_LE(tokens, 100.0);
}