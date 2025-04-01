#include "grpc_server.h"


enum class RpcType { PUT, GET, DELETE }

class CallData {
public:
    CallData(shardkv::KVService::AsyncService* service, grpc::ServerCompletionQueue* cq, RpcType rpc_type) 
    : service_(service), cq_(cq), responder_(&ctx_), rpc_type_(rpc_type), status_(CREATE) {
        proceed();
    }

    void proceed() {
        if (status_ == CREATE) {
            status_ = PROCESS;

            switch (rpc_type_) {
                case RpcType::PUT:
                    service_->RequestPut(&ctx_, &put_req_, &responder_, cq_, cq_, this);
                    break;
                case RpcType::GET:
                    service_->RequestGet(&ctx_, &get_req_, &responder_, cq_, cq_, this);
                    break;
                case RpcType::DELETE:
                    service_->ReequestDelete(&ctx_, &del_req_, &responder_, cq_, cq_, this);
                    break;
            }

            // Request that the server start processing 
        } else if (status_ == PROCESS) {
            new CallData(service_, cq_, rpc_type_);
            // Simulated responses — REPLACE with real KV logic
            switch (rpc_type_) {
                case RpcType::PUT:
                    response_.set_result("OK: Stored [" + put_req_.key() + "] = " + put_req_.value());
                    break;
                case RpcType::GET:
                    response_.set_result("OK: Value for [" + get_req_.key() + "] = mockvalue");
                    break;
                case RpcType::DELETE:
                    response_.set_result("OK: Deleted key [" + del_req_.key() + "]");
                    break;
            }
            status_ = FINISH;
            responder_.Finish(response_, Status::OK, this);
        } else {
            CHECK_EQ(status_, FINISH);
            delete this;
        }
    }

private:
    shardkv::KVService::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    RecType::rpc_type_;

    shardkv::PutRequest put_req_;
    shardkv::GetRequest get_req_;
    shardkv::DeleteRequest del_req_;

    shardkv::KVResponse response_;
    grpc::ServerAsyncResponseWriter<shardkv::KVResponse> responder_;

    enum grpc::CallStatus { CREATE, PROCESS, FINISH };
    grpc::CallStatus _status;
};


// ============ AsyncKVServer impl ============ // 


AsyncKVServer::AsyncKVServer() {}


AsyncKVServer::~AsyncKVServer() {
    shutdown();
}


AsyncKVServer::shutdown() {
    running_ = false;
    if (server_) {
        server_->shutdown();
        cq_->shutdown();
        if (server_thread_.joinable()) server_thread_.join();
    }
}


void AsyncKVServer::run(const std::string& server_address) {
    grpc::ServerBuilder builder;
    builder.addListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();

    std::cout << "ShardKV gRPC server listening on " << server_address << std::endl;

    new CallData(&service_, cq_.get(), RpcType::PUT);
    new CallData(&service_, cq_.get(), RpcType::GET);
    new CallData(&service_, cq_.get(), RpcType::DELETE);

    server_thread_ = std::thread(&AsyncKVServer::handle_rpcs, this);
}


void AsyncKVServer::handle_rpcs() {
    new CallDataPut(&service_, cq.get());       // Start first handler

    void* tag;
    bool ok;
    while (running_ && cq_->Next(&tag, &ok)) {
        if (ok) static_cast<CallDataPut*>(tag)->Proceed();
    }
}
