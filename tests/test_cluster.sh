#!/bin/bash

# Kill background processes on exit
clean_up() {
    echo "Cleaning up..."
    pkill -f "shardkv_server 5000"
    pkill -f "shardkv_server 5001"
}
trap clean_up EXIT

# Resolve paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
LOG_DIR="$PROJECT_ROOT/logs"

SERVER5000_LOG="$LOG_DIR/server.5000.log"
SERVER5001_LOG="$LOG_DIR/server.5001.log"

# Make sure logs dir exists at project root
mkdir -p "$LOG_DIR"

# Touch log files if not exist
[ ! -f "$SERVER5000_LOG" ] && touch "$SERVER5000_LOG"
[ ! -f "$SERVER5001_LOG" ] && touch "$SERVER5001_LOG"

# Start servers (FROM ANYWHERE, using absolute paths)
"$BUILD_DIR/shardkv_server" 5000 "127.0.0.1:5000" > "$SERVER5000_LOG" 2>&1 &
"$BUILD_DIR/shardkv_server" 5001 "127.0.0.1:5001" > "$SERVER5001_LOG" 2>&1 &
sleep 1

# Join cluster
echo "JOIN 127.0.0.1:5001" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5000
echo "JOIN 127.0.0.1:5000" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5001

# Test KV operations
echo "PUT rocket starship" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5000
echo "GET rocket" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5001

# Print logs from root logs directory
echo "--- Server 5000 Log ---"
cat "$SERVER5000_LOG"
echo "--- Server 5001 Log ---"
cat "$SERVER5001_LOG"
