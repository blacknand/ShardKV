This project uses GoogleTest for all testing. You can find the mock tests inside of `tests`. To test yourself, first create the build directory if you have not done so already:


```bash
mkdir build && cd build
```

You only have to create the build directory once, and build KVShard once to be able to run any of the tests:
```bash
cmake ..
make
```


- Test `KVStore`
```bash
./test_kvstore
```

- Test `ConsistentHashing`
```bash
./test_consistent_hashing
```