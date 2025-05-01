#ifndef CONSISTENT_HASH_H
#define CONSISTENT_HASH_H

#include "MurmurHash3.h"
#include "hash_utils.h"

#include <iostream>
#include <string>
#include <map>
#include <unordered_set>
#include <mutex>


class ConsistentHash 
{
public:
    explicit ConsistentHash(int vnodes = 200, uint32_t seed = 42) : vnode_count(vnodes), hash_seed(seed) {}

    void add_node(const std::string &node);
    void remove_node(const std::string &node);
    std::unordered_set<std::string> get_nodes();
    std::string get_node(const std::string &key);
private:
    std::map<uint32_t, std::string> ring;
    std::mutex mutex_;
    int vnode_count;
    uint32_t hash_seed;
};

#endif 