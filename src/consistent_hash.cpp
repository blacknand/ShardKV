#include "consistent_hash.h"


uint32_t ConsistentHash::hash_key(const std::string& key) {
    uint32_t hash;
    MurmurHash3_x86_32(key.c_str(), key.size(), 42, &hash); // 42 = seed
    return hash;
}


int main() {
    ConsistentHash hash;
    std::string key = "test_key";
    std::cout << "Hash: " << hash.hash_key(key) << std::endl;
    return 0;
}