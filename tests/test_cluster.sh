#!/bin/bash

# Kill background processes on exit
clean_up() {
    echo "Cleaning up..."
    pkill -f shardkv_server
}
trap clean_up EXIT

# macOS Control Center listens to port 5000 & 7000

echo "Cleaning up old servers before starting new ones..."
pkill -f shardkv_server
sleep 1

# Resolve paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
LOG_DIR="$PROJECT_ROOT/logs"

SERVER5001_LOG="$LOG_DIR/server.5001.log"
SERVER5002_LOG="$LOG_DIR/server.5002.log"
DEBUG_LOG="$PROJECT_ROOT/debug.log"

# Make sure logs dir exists at project root
mkdir -p "$LOG_DIR"

# Touch log files if not exist
[ ! -f "$SERVER5001_LOG" ] && touch "$SERVER5001_LOG"
[ ! -f "$SERVER5002_LOG" ] && touch "$SERVER5002_LOG"
[ ! -f "$DEBUG_LOG" ] && touch "$DEBUG_LOG"

# Start servers (FROM ANYWHERE, using absolute paths)
echo "Starting server on port 5001..."
"$BUILD_DIR/shardkv_server" 5001 "127.0.0.1:5001" > "$SERVER5001_LOG" 2>&1 &
sleep 1

echo "Starting server on port 5002..."
"$BUILD_DIR/shardkv_server" 5002 "127.0.0.1:5002" > "$SERVER5002_LOG" 2>&1 &
sleep 1

echo "Servers started. Waiting a moment for initialization..."
sleep 2

echo "Joining nodes to cluster..."
echo -e "JOIN 127.0.0.1:5002\n" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5001
echo -e "JOIN 127.0.0.1:5001\n" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5002

echo "Testing KV operations..."
echo -e "PUT rocket starship\n" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5001
echo -e "GET rocket\n" | "$BUILD_DIR/shardkv_client" 127.0.0.1 5002

# Print logs from root logs directory
echo "--- Server 5001 Log ---"
cat "$SERVER5001_LOG"
echo "--- Server 5002 Log ---"
cat "$SERVER5002_LOG"
echo "--- Debug Log ---"
cat "$DEBUG_LOG"