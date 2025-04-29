#ifndef KV_STORE_H
#define KV_STORE_H

#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>
#include <functional>


class KVStore
{
public:
    KVStore();
    void put(const std::string &key, const std::string &value);
    std::string get(const std::string& key);
    int remove(const std::string& key);

private:
    static constexpr size_t NUM_SHARDS = 16;
    std::vector<std::unordered_map<std::string, std::string>> shards;
    std::vector<std::shared_mutex> shared_mutexes;

    size_t get_shard_index(const std::string& key) const;
};

#endif  // KV_STORE_H