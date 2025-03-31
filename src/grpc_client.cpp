#include "grpc_client.h"


AsyncKVClient::AsyncKVClient(std::shared_ptr<grpc::Channel> channel) :
    stub_(shardkv::KVService::NewStub(channel)) {
    worker_ = std::thread(&AsyncKVClient::AsyncCompleteRec, this);
}


void AsyncKVClient::put_async(const std::string& key, const std::string& value, std::function<void(const std::string&)> callback) {
    auto* call = new AsyncCall;
    call->callback = callback;
    
    shardkv::PutRequest request;
    request.set_key(key);
    request.set_value(value);

    call->response_reader = stub_->AsyncPut(&call->context, request, &cq_);
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
}


void AsyncKVClient::get_async(const std::string& key, std::function<void(const std::string&)> callback) {
    auto* call = new AsyncCall;
    call->callback = callback;
    
    shardkv::GetRequest request;
    request.set_key(key);

    call->response_reader = stub_->AsyncPut(&call->context, request, &cq_);
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
}


void AsyncKVClient::remove_async(const std::string& key, std::function<void(const std::string&)> callback) {
    auto* call = new AsyncCall;
    call->callback = callback;
    
    shardkv::DeleteRequest request;
    request.set_key(key);

    call->response_reader = stub_->AsyncPut(&call->context, request, &cq_);
    call->response_reader->Finish(&call->reply, &call->status, (void*)call);
}


void AsyncKVClient::async_complete_rpc() {

}


void AsyncKVClient::shutdown() {
    running_ = false;
    cq_.shutdown();
    if (worker_.joinable()) worker_.join();
}