ShardKV is currently in a very early testing phase. While CMake is the build system of the project, I have been manually compiling it since I plan to use CMake when building the first usable version. For the mean time, please use the following compile commands (requires you to have C++ Boost installed):


```bash
g++ server.cpp kv_shard.cpp -o server -lboost_system
g++ client.cpp -o client -lboost_system
```