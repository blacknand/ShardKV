#include "kv_store.h"


KVStore::KVStore() {}


void KVStore::put(const std::string &key, const std::string &value) 
{
    std::lock_guard<std::mutex> lock(store_mutex);
    store[key] = value;
}


std::string KVStore::get(const std::string& key) 
{
    std::lock_guard<std::mutex> lock(store_mutex);
    auto it = store.find(key);
    return (it != store.end()) ? it->second : "";
}


int KVStore::remove(const std::string& key) 
{
    std::lock_guard<std::mutex> lock(store_mutex);
    auto it = store.find(key);
    if (it != store.end()) {
        store.erase(key);
        return 0;
    } else
        return -1;
}