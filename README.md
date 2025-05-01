> **Warning:** This project is still in active development. To see the current development process, see `src/TODO.md`
# ShardKV
A fast distributed, self-healing key-value store designed for fault-tolerance and scalability.
## Installation
```bash
git clone git@github.com:blacknand/ShardKV.git
cd ShardKV
mkdir build && cd build
cmake ..
make
```
## Building
```bash
cmake -B build -G Ninja     # Assumes Ninja rather than make. Switch to any other build system if preferred
```
## Requirements
Most of ShardKVs dependencies can be installed simply by using the CMake build commands above, but it is recommended/required to have some of the other dependencies
installed locally.
- CMake (most up to date version)
- gRPC (with buffer protocols)
- GoogleTest
- WSL if on Windows (for the Bash shell scripts)
- Boost (for Boost.Asio and other Boost parsing/formatting libraries)
- `MurmurHash3.cpp` & `MurmurHash3.h` (for the `MurmurHash3_x86_32()` function)
> **Note:** When ShardKV is fully released, I will be creating a docker container for it to prevent anyone from having to install all of these dependencies, apart from obviously needing docker.
## Usage
- To run the ShardKV server application
```bash
# NOTE: ShardKV is not in a usable state. Please do not try to run this command yet.
# NOTE: If you want to try ShardKV in its current state, please see src/README.md
./build/shardkv_server    
```
- To run the ShardKV client application
```bash
./build/shardkv_client    
```

