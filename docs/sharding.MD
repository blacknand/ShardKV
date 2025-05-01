## What is sharding?
As you may have guessed, the ShardKV is obviously the sharding process discussed here. Sharding is a technique used in distributed systems to partition data into smaller, more managable subsets called shards, which can be processed independently. Each shard contains a portion of the total data, and the shards are typically distributed across multiple nodes. Instead of storing all data in a single data structure (i.e. the `std::unordered_map` used in `KVStore`), it is split into multiple structures (shards) based on a key. Each shard can then be processed independently allowing parallel operations on differernt shards.


A good example of sharding, is to imagine a key-value store with 1 million pairs:


- *Without sharding*: All 1 milllion pairs are stored in a single `std::unordered_map`. Every `PUT`, `GET` or `REMOVE` operation access this map, and a single mutex serialises all operations causing contention under high load.
- *With sharding*: Split the 1 million pairs into 16 shards. A hash function `MurMur3` then maps each key to one of the 16 shards. Now operations on keys in different shards can proceed concurrently, reducing contention.
## Partitioning stratergy
A hash function maps each key to a shard. There are 16 shards, and the way that a shard is determined for a key is `hash(key) % 16`
## Data structures
Each shard is a seperate instance of a `std::unordered_map` for key-value pairs. In `KVStore` there is a `std::vector` of `std::unorderd_map`s where each element in the vector is a shard.
## Synchronisation
Each shard uses a `std::shard_mutex` as its own synchronisation mechanism to handle concurrent access within the shard. This means:


- Multiple `GET` operations (reads) can process concurrently within a shard using a shared lock.
- `PUT` and `REMOVE` operations (writes) use an exclusive lock, seralising writes within the shard.
- Operations on different shards are independentm so they do not block each other.


## Routing
