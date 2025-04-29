#include "kv_store.h"
#include "consistent_hash.h"


KVStore::KVStore() : shards(NUM_SHARD), shard_mutexes(NUM_SHARD) {}


size_t KVStore::get_shard_index(const std::string& key) 
{
    uint32_t hash = ConsistentHash::hash_key(key);
    return hash % NUM_SHARDS;
}


void KVStore::put(const std::string &key, const std::string &value) 
{
    size_t shard_index = get_shard_index(key);
    std::unique_lock<std::shared_mutex> lock(shard_mutexes[shard_index]);
    shards[shard_index][key] = value;
}


std::string KVStore::get(const std::string& key) 
{
    size_t shard_index = get_shard_index(key);
    std::shared_lock<std::shared_mutex> lock(shard_mutexes[shard_index]);
    auto it = shards[shard_index].find(key);
    return (it != shards[shard_index].end()) ? it->second : "";
}


int KVStore::remove(const std::string& key) 
{
    size_t shard_index = get_shard_index(key);
    std::unique_lock<std::shared_mutex> lock(shared_mutexes[shard_index]);
    auto it = shards[shard_index].find(key);
    if (it != shards[shard_index].end()) {
        shards[shard_index].erase(key);
        return 0;
    }

    return -1;
}