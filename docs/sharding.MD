## Partitioning stratergy
A hash function maps each key to a shard. There are 10 shards, and the way that a shard is determined for a key is `hash(key) % 10`
## Data structures
Each shard is a seperate instance of a `std::unordered_map` for key-value pairs. In `kv_store.cpp` there is a `std::vector` of `std::unorderd_map`s where each element in the vector is a shard.
## Synchronisation
Each shard uses a `std::shard_mutex` as its own synchronisation mechanism to handle concurrent access within the shard. This means:


- Multiple `GET` operations (reads) can process concurrently within a shard using a shared lock.
- `PUT` and `REMOVE` operations (writes) use an exclusive lock, seralising writes within the shard.
- Operations on different shards are independentm so they do not block each other.


## Routing
