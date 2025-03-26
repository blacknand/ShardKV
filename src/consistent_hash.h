#ifndef CONSISTENT_HASH_H
#define CONSISTENT_HASH_H

#include "MurmurHash3.h"

#include <iostream>
#include <string>
#include <map>


class ConsistentHash {
public:
    explicit ConsistentHash(int vnodes = 200, uint32_t seed = 42) : vnode_count(vnodes), hash_seed(seed) {}

    uint32_t hash_key(const std::string &key);
    void add_node(const std::string &node);
    void remove_node(const std::string &node);
    std::string get_node(const std::string &key);
private:
    std::map<uint32_t, std::string> ring;
    int vnode_count;
    uint32_t hash_seed;
};

#endif 