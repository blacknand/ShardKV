# TODO List: Improvements for ShardKV TCP Server & Client

## **Phase 1: Immediate Fixes (Before Moving to Raft & gRPC)**
### **1️⃣ Multi-threaded Event Loop**
- [x] Replace single-threaded `io_context.run()` with `boost::asio::thread_pool`
- [x] Use `std::thread::hardware_concurrency()` to scale with CPU cores
- [x] Ensure all async handlers are executed in worker threads

### **2️⃣ Better Error Handling**
- [ ] Handle unexpected disconnections (`connection_reset`)
- [ ] Implement graceful shutdown (`SIGINT` handling)
- [ ] Improve reconnect logic for clients (`async_reconnect`)

### **3️⃣ Memory Optimization**
- [ ] Replace `std::vector<char>` with `boost::asio::streambuf`
- [ ] Use `async_read_until(_buffer, "\n")` for zero-copy operations

### **4️⃣ Backpressure Control (Prevent Overload)**
- [ ] Implement connection limit (`MAX_CONNECTIONS`)
- [ ] Use `std::atomic<int>` to track active connections
- [ ] Modify `_acceptor.listen(10)` to limit backlog queue

---

## **Phase 2: Distributed Consensus & RPC (Raft, gRPC)**
### **1️⃣ Implement gRPC for Node-to-Node Communication**
- [ ] Install gRPC & Protobuf
- [ ] Define Protobuf messages for **Leader Election**, **Log Replication**
- [ ] Implement `grpc::async_service` for Raft communication

### **2️⃣ Implement Raft Algorithm**
- [ ] Implement leader election (`RequestVote RPC`)
- [ ] Implement log replication (`AppendEntries RPC`)
- [ ] Implement failover recovery (followers sync with leader)

---

## **Phase 3: Security & Observability**
### **1️⃣ TLS Encryption**
- [ ] Install Boost.Asio SSL (`boost::asio::ssl`)
- [ ] Configure TLS 1.2 (`use_certificate_chain_file`)
- [ ] Encrypt all TCP traffic

### **2️⃣ Structured Logging & Monitoring**
- [ ] Install `spdlog` for structured logging
- [ ] Replace `std::cerr` with `spdlog::info()`
- [ ] Integrate Prometheus for tracking **latency, requests/sec, errors**

### **3️⃣ Performance Benchmarking**
- [ ] Use **wrk** to simulate **high load**
- [ ] Measure **latency & throughput**
- [ ] Profile with **Valgrind for memory leaks**

---

## **Phase 4: Advanced Scaling & Self-Healing**
### **1️⃣ Dynamic Node Scaling**
- [ ] Implement node auto-discovery
- [ ] Allow new nodes to **join & sync data**
- [ ] Automatically rebalance shards

### **2️⃣ Persistent Storage**
- [ ] Integrate **RocksDB** for durable storage
- [ ] Implement **Write-Ahead Logging (WAL)**
- [ ] Enable **snapshot backups** to recover from crashes

---

# **🔹 Next Steps**
🚀 **Implement all Phase 1 tasks first** (before Raft).  
🔜 Move to Phase 2 (gRPC, Raft) after networking is solid.  
