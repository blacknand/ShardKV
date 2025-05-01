#include "grpc_server.h"
#include "kv_store.h"


enum class RpcType { PUT, GET, DELETE };

class CallData 
{
public:
    CallData(shardkv::KVService::AsyncService* service, grpc::ServerCompletionQueue* cq, RpcType rpc_type, KVStore* kv_store) 
    : service_(service), cq_(cq), responder_(&ctx_), rpc_type_(rpc_type), kv_store_(kv_store), status_(CREATE) {
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
                    service_->RequestDelete(&ctx_, &del_req_, &responder_, cq_, cq_, this);
                    break;
            }

        // Request that the server start processing 
        } else if (status_ == PROCESS) {
            new CallData(service_, cq_, rpc_type_, kv_store_);
            switch (rpc_type_) {
                case RpcType::PUT:
                    response_.set_result(kv_store_->put(put_req_.key(), put_req_.value()) ? "OK" : "FAIL");
                    break;
                case RpcType::GET:
                    response_.set_result(kv_store_->get(get_req_.key()) ? "OK" : "FAIL");
                    break;
                case RpcType::DELETE:
                    response_.set_result(kv_store_->remove(del_req_.key()) ? "OK" : "FAIL");
                    break;
            }
            status_ = FINISH;
            responder_.Finish(response_, Status::OK, this);
        } else {
            delete this;
        }
    }

private:
    shardkv::KVService::AsyncService* service_;
    grpc::ServerCompletionQueue* cq_;
    grpc::ServerContext ctx_;
    RpcType rpc_type_;

    shardkv::PutRequest put_req_;
    shardkv::GetRequest get_req_;
    shardkv::DeleteRequest del_req_;

    KVStore* kv_store_;

    shardkv::KVResponse response_;
    grpc::ServerAsyncResponseWriter<shardkv::KVResponse> responder_;

    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
};


// ============ AsyncKVServer impl ============ // 


AsyncKVServer::AsyncKVServer(KVStore* kv_store) : kv_store_(kv_store) {}


AsyncKVServer::~AsyncKVServer() 
{
    shutdown();
}


void AsyncKVServer::shutdown() 
{
    running_ = false;
    if (server_) {
        server_->shutdown();
        cq_->shutdown();
        if (server_thread_.joinable()) server_thread_.join();
    }
}


void AsyncKVServer::run(const std::string& server_address) 
{
    grpc::ServerBuilder builder;
    builder.addListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();

    std::cout << "ShardKV gRPC server listening on " << server_address << std::endl;

    new CallData(&service_, cq_.get(), RpcType::PUT, kv_store_);
    new CallData(&service_, cq_.get(), RpcType::GET, kv_store_);
    new CallData(&service_, cq_.get(), RpcType::DELETE, kv_store_);

    server_thread_ = std::thread(&AsyncKVServer::handle_rpcs, this);
}


void AsyncKVServer::handle_rpcs() 
{
    new CallData(&service_, cq_.get(), RpcType::PUT, kv_store_);
    new CallData(&service_, cq_.get(), RpcType::GET, kv_store_);
    new CallData(&service_, cq_.get(), RpcType::DELETE, kv_store_);

    void* tag;
    bool ok;
    while (running_ && cq_->Next(&tag, &ok)) {
        if (ok) static_cast<CallData*>(tag)->Proceed();
    }
}
