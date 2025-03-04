cmake_minimum_required(VERSION 3.10)
project(ShardKV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Source files
add_executable(shardkv src/main.cpp src/server.cpp src/kv_store.cpp src/consistent_hash.cpp src/raft.cpp)

# Include directories
target_include_directories(shardkv PRIVATE src)

