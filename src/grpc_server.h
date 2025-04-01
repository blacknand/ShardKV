#ifndef GRPC_SERVER_H
#define GRPC_SERVER_H

#pragma once

#include <memory>
#include <string>
#include <functional>
#include <grpcpp/grcpcpp.h>

#include "../build/binary/shardkv.grpc.pb.h"

class AsyncKVServer {
public:
    AsyncKVServer();
    ~AsyncKVServer();
    void run(const std::string& server_address);
    void shutdown();
private:
    void handle_rpcs();

    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    shardkv::KVService::AsyncService service_;
    std::thread server_thread_;
    bool running_ = true;
};

#endif