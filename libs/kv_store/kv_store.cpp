#include "~/libs/kv_store/kv_store.h"
#include "~/libs/consistent_hashing/consistent_hash.h"
#include "~/libs/utils/hash_utils.h"


KVStore::KVStore() : shards(NUM_SHARDS), shared_mutexes(NUM_SHARDS) {}


size_t KVStore::get_shard_index(const std::string& key) const
{
    uint32_t hash = HashUtils::hash_key(key, HASH_SEED);
    return hash % NUM_SHARDS;
}


void KVStore::put(const std::string &key, const std::string &value) 
{
    size_t shard_index = get_shard_index(key);
    std::unique_lock<std::shared_mutex> lock(shared_mutexes[shard_index]);
    shards[shard_index][key] = value;
}


std::string KVStore::get(const std::string& key) 
{
    size_t shard_index = get_shard_index(key);
    std::shared_lock<std::shared_mutex> lock(shared_mutexes[shard_index]);
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