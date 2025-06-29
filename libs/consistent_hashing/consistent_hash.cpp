#include "consistent_hash.h"

// #include "../version_config.h"


void ConsistentHash::add_node(const std::string &node) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < vnode_count; i++) {
        std::string virtual_node = node + "#" + std::to_string(i);
        uint32_t hash = HashUtils::hash_key(virtual_node, hash_seed);
        ring[hash] = node;
    }
}


void ConsistentHash::remove_node(const std::string &node) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < vnode_count; i++) {
        std::string virtual_node = node + "#" + std::to_string(i);
        uint32_t hash = HashUtils::hash_key(virtual_node, hash_seed);
        ring.erase(hash);
    }
}


std::string ConsistentHash::get_node(const std::string &key) 
{
    // geeks4geeks article somewhere
    std::lock_guard<std::mutex> lock(mutex_);
    if (ring.empty()) throw std::runtime_error("Hash ring empty"); 

    uint32_t hash = HashUtils::hash_key(key, hash_seed);
    auto it = ring.lower_bound(hash);

    // Return the address and port of the server
    // that is closest to the hash value, if there is no appropriate value
    // then loop back to the beginning of the hash ring and return the first node
    if (it == ring.end()) it = ring.begin();

    return it->second;
}


std::unordered_set<std::string> ConsistentHash::get_nodes() 
{
    // NOTE: inefficient
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_set<std::string> nodes;
    for (const auto& entry : ring) nodes.insert(entry.second); 

    return nodes;
}

