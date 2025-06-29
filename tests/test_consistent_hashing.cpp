#include "~/libs/consistent_hashing/consistent_hash.h"
#include "~/libs/utils/hash_utils.h"

// #include "../version_config.h"

#include <gtest/gtest.h>

#define SEED 42

// TODO: must fix test for updated consistent_hash
class ConsistentHashTest : public ::testing::Test 
{
protected:
    ConsistentHash hash;

    void SetUp() {
        hash.add_node("node1");
        hash.add_node("node2");
        hash.add_node("node3");
    }
};


TEST_F(ConsistentHashTest, ReturnConsistentHash) 
{
    std::string key = "test_key";
    uint32_t hash1 = HashUtils::hash_key(key, SEED);
    uint32_t hash2 = HashUtils::hash_key(key, SEED);

    EXPECT_EQ(hash1, hash2) << "Hash function should be deterministic" << std::endl;
}


TEST_F(ConsistentHashTest, NodeIsAssigned) 
{
    std::string key = "test_key";
    std::string node = hash.get_node(key);

    EXPECT_FALSE(node.empty()) << "get_node() should return a vald node" << std::endl;
}


TEST_F(ConsistentHashTest, NodesAddedCorrectly) 
{
    hash.add_node("test_node");

    // Check if we can get nodes from the hash ring
    std::string key = "new_test_key";
    std::string node = hash.get_node(key);

    EXPECT_FALSE(node.empty()) << "get_node() should return a valid node after adding nodes" << std::endl;
}


TEST_F(ConsistentHashTest, NodesRemovedCorrectly) 
{
    hash.remove_node("node2");

    std::string key = "test_key";
    std::string node = hash.get_node(key);

    EXPECT_NE(node, "node2") << "After removal, NodeB should not be assigned to any key.";
}


TEST_F(ConsistentHashTest, LoadDistributionAfterRemoval) 
{
    std::string key = "sample_key";
    std::string old_node = hash.get_node(key);

    hash.remove_node(old_node);
    std::string new_node = hash.get_node(key);

    EXPECT_NE(old_node, new_node) << "Key should be reassigned to a different node" << std::endl;
}


int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}