#ifndef CONSISTENT_HASH_H
#define CONSISTENT_HASH_H

#include "MurmurHash3.h"

#include <iostream>
#include <string>

class ConsistentHash {
public:
    uint32_t hash_key(const std::string &key);
};

#endif 