#! /bin/bash

# Kill background processes on exit
clean_up() {
    echo "Cleaning up..."
    pkill -f "server 5000"
    pkill -f "server 5001"
}

trap clean_up EXIT

# Start 2 servers on different ports and pipe all output to log files in background
./build/server 5000 "127.0.0.1:5000" > server.5000.log 2>&1 &
./build/server 5001 "127.0.0.1:5001" > server.5001.log 2>&1 &
sleep 1

# Join the servers together
cd /
echo "JOIN 127.0.0.1:5001" | ./build/client 127.0.0.1 5000
echo "JOIN 127.0.0.1:5000" | ./build/client 127.0.0.1 5001

# Insert a key that hashes to server 5000 and retrieve it from server 5001
echo "PUT rocket starship" | ./build/client 127.0.0.1 5000
echo "GET rocket" | ./build/client 127.0.0.1 5001

echo "--- Server 5000 Log ---"
cat server.5000.log
echo "--- Server 5001 Log ---"
cat server.5001.log
