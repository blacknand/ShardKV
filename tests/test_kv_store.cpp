#include "../src/kv_store.h"

#include <gtest/gtest.h>
#include <thread>


TEST(KVStoreTest, BasicOperations) {
    KVStore store;

    store.put("key1", "value1");
    EXPECT_EQ(store.get("key1"), "value1");

    store.put("key1", "new_value");
    EXPECT_EQ(store.get("key1"), "new_value");

    store.remove("key1");
    EXPECT_EQ(store.get("key1"), "");
}


TEST(KVStoreTest, ThreadSafety) {
    KVStore store;

    std::thread t1([&]() { store.put("key2", "value2"); });
    std::thread t2([&]() { store.put("key3", "value3"); });

    t1.join();
    t2.join();

    EXPECT_EQ(store.get("key2"), "value2");
    EXPECT_EQ(store.get("key3"), "value3");
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}