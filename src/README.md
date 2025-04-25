### Disclaimer 
ShardKV is currently in a very early testing phase. While CMake is the build system of the project, I have been manually compiling it since I plan to use CMake when building the first usable version. 

### Build
```bash
# For the TCP server
g++ server.cpp kv_store.cpp \                                                                                                 
-I$(brew --prefix boost)/include \
-L$(brew --prefix boost)/lib \
-std=c++17 -Wall -Wextra \
-lboost_system -pthread \
-o server
# For the TCP client
g++ client.cpp \                                                                                                 
-I$(brew --prefix boost)/include \
-L$(brew --prefix boost)/lib \
-std=c++17 -Wall -Wextra \
-lboost_system -pthread \
-o client
```

### Run
```bash
# The server in 1 shell
./server 127.0.0.1:8000
# Client in other shell (run as many clients as you want for testing)
./client 127.0.0.1 8000
```