#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include "MurmurHash3.h"
#include <string>
#include <cstdint>

namespace HashUtils {
    inline uint32_t hash_key(const std::string& key, uint32_t seed) {
        uint32_t hash;
        MurmurHash3_x86_32(key.c_str(), key.size(), seed, &hash);
        return hash;
    }
}

#endif