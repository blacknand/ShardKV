#include "consistent_hash.h"


uint32_t ConsistentHash::hash_key(const std::string& key) {
    uint32_t hash;
    MurmurHash3_x86_32(key.c_str(), key.size(), 42, &hash); // 42 = seed
    return hash;
}


void ConsistentHash::add_node(const std::string &node) {
    for (int i = 0; i < vnode_count; i++) {
        std::string node = node + "#" + std::to_string(i);
        uint32_t hash = this->hash_key(node);
        ring[hash] = node;
    }
}


void ConsistentHash::remove_node(const std::string &node) {
    for (int i = 0; i < vnode_count; i++) {
        std::string node = node + "#" + std::to_string(i);
        uint32_t hash = this->hash_key(node);
        ring.erase(hash);
    }
}


std::string ConsistentHash::get_node(const std::string &key) {
    if (ring.empty()) throw std::runtime_error("Hash ring empty");

    uint32_t hash = this->hash_key(key);
    auto it = ring.lower_bound(hash);

    if (it == ring.end())
        it = ring.begin();

    return it->second;
}


int main() {
    ConsistentHash hash;
    std::string key = "test_key";
    std::cout << "Hash: " << hash.hash_key(key) << std::endl;
    return 0;
}