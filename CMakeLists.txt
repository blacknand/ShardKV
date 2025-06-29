cmake_minimum_required(VERSION 4.0.1)
project(ShardKV)

configure_file(version_config.in.h, version_config.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_PREFIX_PATH "$ENV{HOME}/.local")
set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/binary")

file(MAKE_DIRECTORY ${PROTO_GEN_DIR})
set(PROTO_FILE ${PROTO_SRC_DIR}/shardkv.proto)

cmake_policy(SET CMP0135 NEW) #CMake policyt 0135 DOWNLOAD_EXTRACT_TIMESTAMP

# TODO: Enable exceptions for googletest

include(FindProtobuf)
find_package(Protobuf REQUIRED)
find_package(Boost CONFIG REQUIRED COMPONENTS system)    # For Boost.Asio
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

set(SHARED_SOURCES
    src/kv_store.cpp
    src/consistent_hash.cpp
    src/MurmurHash3.cpp
    src/hash_utils.h
 )

# gRPC compile command
set(GRPC_CPP_PLUGIN "$ENV{HOME}/.local/bin/grpc_cpp_plugin")
add_custom_command(
  OUTPUT  ${PROTO_GEN_DIR}/shardkv.grpc.pb.cc 
          ${PROTO_GEN_DIR}/shardkv.grpc.pb.h
          ${PROTO_GEN_DIR}/shardkv.pb.cc
          ${PROTO_GEN_DIR}/shardkv.pb.h
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
  ARGS  --grpc_out=${PROTO_GEN_DIR}
        --cpp_out=${PROTO_GEN_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        -I ${PROTO_SRC_DIR}
        ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
  COMMENT "Generating gRPC sources"
  VERBATIM
)

add_custom_target(generate_grpc_files
  DEPENDS
    ${PROTO_GEN_DIR}/shardkv.pb.cc
    ${PROTO_GEN_DIR}/shardkv.grpc.pb.cc
)

add_library(shardkv_grpc
  ${PROTO_GEN_DIR}/shardkv.grpc.pb.cc
  ${PROTO_GEN_DIR}/shardkv.pb.cc
)

add_dependencies(shardkv_grpc generate_grpc_files)

target_include_directories(shardkv_grpc PUBLIC ${PROTO_GEN_DIR} ${Protobuf_INCLUDE_DIRS})
target_link_libraries(shardkv_grpc PUBLIC gRPC::grpc++ protobuf::libprotobuf)

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

# shardkv_grpc for the generated gRPC library
target_link_libraries(shardkv_server
                        PRIVATE Boost::system 
                        pthread 
                        gRPC::grpc++ 
                        shardkv_grpc
)
target_link_libraries(shardkv_client 
                        PRIVATE 
                        Boost::system 
                        pthread 
                        gRPC::grpc++ 
                        shardkv_grpc
)

# Enable testing
enable_testing()

# Test consistent hashing
add_executable(test_consistent_hashing
                src/consistent_hash.cpp
                src/hash_utils.h
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
add_test(NAME KVStoreTest COMMAND test_kv_store)

# Test gRPC server and client
add_executable(test_grpc
                tests/test_grpc.cpp
                src/kv_store.cpp
                src/grpc_server.cpp
                src/grpc_client.cpp
)

target_include_directories(test_grpc PRIVATE src)
target_link_libraries(test_grpc 
                        gtest gtest_main
                        grpc++ 
                        protobuf::libprotobuf 
                        shardkv_grpc
)
add_test(NAME TestGrpc COMMAND test_grpc)