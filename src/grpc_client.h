#ifndef GRPC_CLIENT_H
#define GRPC_CLIENT_H

#pragma once

#include <memory>
#include <string>
#include <functional>
#include <grpcpp/grpcpp.h>

#include "../build/binary/shardkv.grpc.pb.h"
#include "kv_store.h"

class AsyncKVClient 
{
public:
    AsyncKVClient(std::shared_ptr<grpc::Channel> channel);
    void put_async(const std::string& key, const std::string& value, std::function<void(const std::string&)> callback);
    void get_async(const std::string& key, std::function<void(const std::string&)> callback);
    void remove_async(const std::string& key, std::function<void(const std::string&)> callback);
    void shutdown();
private:
    // Internal tag for async state machine
    struct AsyncCall {
        shardkv::KVResponse reply;
        grpc::ClientContext context;
        grpc::Status status;
        std::unique_ptr<grpc::ClientAsyncResponseReader<shardkv::KVResponse>> response_reader;
        std::function<void(const std::string&)> callback;
    };

    void async_complete_rpc();
    std::unique_ptr<shardkv::KVService::Stub> stub_;
    grpc::CompletionQueue cq_;
    std::thread worker_;
    std::atomic<bool> running_ = true;
};

#endif