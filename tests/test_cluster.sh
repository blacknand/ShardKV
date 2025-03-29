#! /bin/bash

# Kill background processes on exit
clean_up() {
    echo "Cleaning up..."
    pkill -f "server 5000"
    pkill -f "server 5001"
}

trap clean_up EXIT

log_dir = "../logs"
server5000log = "server.5000.log"
server5001log = "server.5001.log"

# Create logs directory if not exists
if [ ! -d "$log_dir" ]; then
    mkdir -p "$log_dir"
    echo "Created logs directory"
fi

# Create server log files if not exists
if [ ! -f "$server5000log" && ! -f "$server50001log" ]; then
    touch "${log_dir}/${$server5000log}"
    touch "${log_dir}/${$server5001log}"
    echo "Create server logs"
fi

# Start 2 servers on different ports and pipe all output to log files in background
./../build/shardkv_server 5000 "127.0.0.1:5000" > ../logs/server.5000.log 2>&1 &
./../build/shardkv_server 5001 "127.0.0.1:5001" > ../logs/server.5001.log 2>&1 &
sleep 1

# Join the servers together
echo "JOIN 127.0.0.1:5001" | ./../build/shardkv_client 127.0.0.1 5000
echo "JOIN 127.0.0.1:5000" | ./../build/shardkv_client 127.0.0.1 5001

# Insert a key that hashes to server 5000 and retrieve it from server 5001
echo "PUT rocket starship" | ./../build/shardkv_client 127.0.0.1 5000
echo "GET rocket" | ./../build/shardkv_client 127.0.0.1 5001

echo "--- Server 5000 Log ---"
cat server.5000.log
echo "--- Server 5001 Log ---"
cat server.5001.log
