cmake_minimum_required(VERSION 3.10)
project(ShardKV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost with required components (e.g., system for Asio)
find_package(Boost REQUIRED COMPONENTS system)

# Source files for main executable
add_executable(shardkv 
                src/main.cpp 
                src/server.cpp 
                src/client.cpp 
                src/kv_store.cpp 
                src/consistent_hash.cpp 
                src/raft.cpp)

# Include directories for shardkv (Boost and src)
target_include_directories(shardkv PRIVATE src ${Boost_INCLUDE_DIRS})

# Link Boost libraries to shardkv
target_link_libraries(shardkv PRIVATE ${Boost_LIBRARIES})

# Enable testing
enable_testing()

# TCP server test
add_executable(test_tcp_server 
                src/kv_store.cpp
                src/client.cpp 
                src/server.cpp 
                tests/test_tcp_server.cpp)
target_include_directories(test_tcp_server PRIVATE src ${Boost_INCLUDE_DIRS})
target_link_libraries(test_tcp_server PRIVATE ${Boost_LIBRARIES})
add_test(NAME TCPServerTest COMMAND test_tcp_server)

# Add GoogleTest for unit testing
add_executable(test_kvstore 
                tests/test_kv_store.cpp 
                src/kv_store.cpp)
target_include_directories(test_kvstore PRIVATE src)  # No Boost needed here unless kv_store.cpp uses it
target_link_libraries(test_kvstore PRIVATE gtest gtest_main)
add_test(NAME KVStoreTest COMMAND test_kvstore)