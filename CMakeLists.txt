cmake_minimum_required(VERSION 3.10)
project(ShardKV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Boost with required components (e.g., system for Asio)
find_package(Boost REQUIRED COMPONENTS system)

# Add GoogleTest as a dependency
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Source files for main executable
add_executable(shardkv 
                src/main.cpp 
                # src/server.cpp 
                # src/client.cpp 
                src/kv_store.cpp 
                src/consistent_hash.cpp 
                # Will require MurmurHash3 at some point
                src/MurmurHash3.cpp
                src/raft.cpp)

# Include directories for shardkv (Boost and src)
target_include_directories(shardkv PRIVATE src ${Boost_INCLUDE_DIRS})

# Link Boost libraries to shardkv
target_link_libraries(shardkv PRIVATE ${Boost_LIBRARIES})

# Enable testing
enable_testing()

# TCP server test
# add_executable(test_tcp_server 
#                 src/kv_store.cpp
#                 src/client.cpp 
#                 src/server.cpp 
#                 tests/test_tcp_server.cpp)

# target_include_directories(test_tcp_server PRIVATE src ${Boost_INCLUDE_DIRS})
# target_link_libraries(test_tcp_server PRIVATE ${Boost_LIBRARIES})
# add_test(NAME TCPServerTest COMMAND test_tcp_server)


# Test consistent hashing
add_executable(test_consistent_hashing
                src/consistent_hash.cpp
                src/MurmurHash3.cpp
                tests/test_consistent_hashing.cpp)       

target_include_directories(test_consistent_hashing PRIVATE src)
target_link_libraries(test_consistent_hashing PRIVATE gtest gtest_main)
add_test(NAME TestConsistentHashing COMMAND test_consistent_hashing)


# Add GoogleTest for unit testing
add_executable(test_kvstore 
                tests/test_kv_store.cpp 
                src/kv_store.cpp)

target_include_directories(test_kvstore PRIVATE src)  
target_link_libraries(test_kvstore PRIVATE gtest gtest_main)
add_test(NAME KVStoreTest COMMAND test_kvstore)