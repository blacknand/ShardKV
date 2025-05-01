#ifndef GRPC_SERVER_H
#define GRPC_SERVER_H

#pragma once

#include <memory>
#include <string>
#include <functional>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <atomic>
#include <mutex>

#include "../build/binary/shardkv.grpc.pb.h"
#include "kv_store.h"

class AsyncKVServer 
{
public:
    AsyncKVServer(KVStore* kv_store);
    ~AsyncKVServer();
    void run(const std::string& server_address);
    void shutdown();
private:
    void handle_rpcs();

    KVStore* kv_store_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    shardkv::KVService::AsyncService service_;
    std::thread server_thread_;
    bool running_ = true;
};

#endif