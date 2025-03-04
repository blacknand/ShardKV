# ShardKV
A fast distributed, self-healing key-value store designed for fault-tolerance and scalability. Instead of being a simple-key value store like Redis, ShardKV is a networked, multi-node system where
- Data is stored across multiple servers
- If a server fails, another server automatically takes over
- Nodes communicate over a network to distribute and recover data
- A leader is elected to maintain consistency
## Features
- **In-Memory Storage**: Fast access to key-value pairs.
- **Networking**: Client-server model using TCP sockets.
- **Consistent Hashing**: Evenly distributes data across multiple nodes.
- **Self-Healing**: Automatically recovers from node failures.
- **Leader Election**: Uses the Raft consensus algorithm for consistency.
## Roadmap
- [x] Implement in-memory key-value store
- [ ] Add networking (TCP server)
- [ ] Implement consistent hashing
- [ ] Add failure detection and self-healing
- [ ] Implement leader election (Raft)
## Installation
```bash
git clone git@github.com:blacknand/ShardKV.git
cd ShardKV
mkdir build && cd build
cmake ..
make
```
## Usage
```bash
./shardkv
```
## Requirements
- `GCC` or `clang`
- `CMake`