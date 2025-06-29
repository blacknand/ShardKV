#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio.hpp>

#include "~/apps/kv_server/kv_server.h"
#include "~/libs/rate_limiter/gossip_rate_limiter.h"

class MockKVTCPServer : public KVTCPServer
{
    MockKVTCPServer(boost::asio::io_context& io_context) : KVTCPServer(io_context, "127.0.0.1", "8080") {}
    MOCK_METHOD(std::string, forward_to_node, (std::string_view address, std::string_view message), (override));
}