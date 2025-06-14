#!/bin/bash

SERVER_HOST="127.0.0.1"
SERVER_PORT="8080"
REQUESTS=250
CONCURRENT_CLIENTS=2

../src/./server $SERVER_HOST $SERVER_PORT & SERVER_PID=$!
sleep 1

send_requests() {
    for ((i=1; i<=$REQUESTS; i++)); do
        # Send TCP request and wait 1 second to close connection
        response=$(echo "GET key" | nc -w 1 $SERVER_HOST $SERVER_PORT)
        echo "Request $i: $response"
        if [[ "$response" == "ERROR: Rate limit exceeded" ]]; then
            echo "Rate limit hit at request $i"
        fi
    done
}

for ((c=1; c<=CONCURRENT_CLIENTS; c++)); do
    send_requests &
done

wait

kill $SERVER_PID