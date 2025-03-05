cmake_minimum_required(VERSION 3.10)
project(ShardKV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files
add_executable(shardkv 
                src/main.cpp 
                src/server.cpp 
                src/client.cpp 
                src/kv_store.cpp 
                src/consistent_hash.cpp 
                src/raft.cpp)

# Include directories
target_include_directories(shardkv PRIVATE src)

# TCP server test
enable_testing()
add_executable(test_tcp_server 
                src/kv_store.cpp
                src/client.cpp 
                src/server.cpp 
                tests/test_tcp_server.cpp)
add_test(NAME TCPServerTest COMMAND test_tcp_server)

# Add GoogleTest for unit testing
enable_testing()
add_executable(test_kvstore tests/test_kv_store.cpp src/kv_store.cpp)
target_link_libraries(test_kvstore gtest gtest_main pthread)
add_test(NAME KVStoreTest COMMAND test_kvstore)