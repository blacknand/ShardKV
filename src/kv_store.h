#ifndef KV_STORE_H
#define KV_STORE_H

#include <unordered_map>
#include <mutex>
#include <string>

class KVStore {
public:
    KVStore();
    void put(const std::string &key, const std::string &value);
    std::string get(const std::string& key);
    int remove(const std::string& key);

private:
    std::unordered_map<std::string, std::string> store;
    std::mutex store_mutex;
};

#endif  // KV_STORE_H