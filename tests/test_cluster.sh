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
CLIENT_DEBUG_LOG="$PROJECT_ROOT/client_debug.log"

# Make sure logs dir exists at project root
mkdir -p "$LOG_DIR"

# Touch log files if not exist
[ ! -f "$SERVER5001_LOG" ] && touch "$SERVER5001_LOG"
[ ! -f "$SERVER5002_LOG" ] && touch "$SERVER5002_LOG"
[ ! -f "$DEBUG_LOG" ] && touch "$DEBUG_LOG"
[ ! -f "$CLIENT_DEBUG_LOG" ] && touch "$CLIENT_DEBUG_LOG"

# Start servers with more verbose output
echo "Starting server on port 5001..."
"$BUILD_DIR/shardkv_server" 5001 "127.0.0.1:5001" > "$SERVER5001_LOG" 2>&1 &
SERVER5001_PID=$!
sleep 2

echo "Starting server on port 5002..."
"$BUILD_DIR/shardkv_server" 5002 "127.0.0.1:5002" > "$SERVER5002_LOG" 2>&1 &
SERVER5002_PID=$!
sleep 2

echo "Servers started with PIDs: $SERVER5001_PID, $SERVER5002_PID"
echo "Waiting a moment for initialization..."
sleep 2

# Function to run a command using expect
run_command_with_expect() {
    local host=$1
    local port=$2
    local command=$3
    
    echo "Running command on $host:$port: $command"
    
    # Create a temporary expect script
    cat > /tmp/kv_test_expect.exp << EOF
#!/usr/bin/expect -f
set timeout 5
spawn $BUILD_DIR/shardkv_client $host $port
expect "Enter command: "
send "$command\r"
expect "Received: "
expect "Enter command: "
send "EXIT\r"
expect eof
EOF
    
    # Make it executable
    chmod +x /tmp/kv_test_expect.exp
    
    # Run the expect script
    /tmp/kv_test_expect.exp
    
    # Clean up
    rm /tmp/kv_test_expect.exp
    
    echo "Command completed"
    sleep 1
}

# Check if expect is available
if ! command -v expect &> /dev/null; then
    echo "Error: 'expect' utility is not installed. Please install it to run this test."
    echo "On macOS: brew install expect"
    echo "On Ubuntu/Debian: apt-get install expect"
    exit 1
fi

# Test operations
echo "Joining nodes to cluster..."
run_command_with_expect "127.0.0.1" "5001" "JOIN 127.0.0.1:5002"
run_command_with_expect "127.0.0.1" "5002" "JOIN 127.0.0.1:5001"

echo "Testing KV operations..."
run_command_with_expect "127.0.0.1" "5001" "PUT rocket starship"
run_command_with_expect "127.0.0.1" "5002" "GET rocket"

# Print logs with timestamps for better debugging
echo "--- Server 5001 Log ---"
cat "$SERVER5001_LOG"
echo "--- Server 5002 Log ---"
cat "$SERVER5002_LOG"
echo "--- Debug Log ---"
cat "$DEBUG_LOG"
echo "--- Client Debug Log ---"
cat "$CLIENT_DEBUG_LOG"

echo "Test complete."