cmake_minimum_required(VERSION 3.10)
project(ShardKV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED COMPONENTS system)    # For Boost.Asio
find_package(gRPC CONFIG REQUIRED)                # Find gRPC canonical way

# Add GoogleTest as a dependency
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings when dealing with GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add gRPC as a dependency
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        1.71.0   # Latest as of Mon 31st March 2025
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

set(SHARED_SOURCES
    src/kv_store.cpp
    src/consistent_hash.cpp
    src/MurmurHash3.cpp
    # TODO: when implementing the raft protocol, add raft.cpp here
 )

# ShardKV server target
add_executable(shardkv_server
                src/server.cpp
                ${SHARED_SOURCES}
)

# ShardKV client executable
add_executable(shardkv_client
                src/client.cpp
                ${SHARED_SOURCES}
)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(shardkv_server PRIVATE Boost::system pthread grpc++)
target_link_libraries(shardkv_client PRIVATE Boost::system pthread)


# Enable testing
enable_testing()

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