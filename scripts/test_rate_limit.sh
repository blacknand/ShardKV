#!/bin/bash

SERVER_HOST="127.0.0.1"
SERVER_PORT="8080"
REQUESTS=250
CONCURRENT_CLIENTS=2
SERVER_LOG="../server.log"


if [ -f ../server.log ]; then
    current_datetime=$(date +"%Y-%m-%d %H:%M:%S")
    cat /dev/null > ../server.log
    echo $current_datetime > ../server.log
else 
    echo "Error: server.log does not exist"
fi

../src/./server $SERVER_HOST $SERVER_PORT >> $SERVER_LOG 2>&1 & 
SERVER_PID=$!
sleep 2

send_requests() {
    local client_id=$1
    for ((i=1; i<=$REQUESTS; i++)); do
        # Send TCP request and wait 1 second to close connection
        response=$(echo "GET key" | nc -w 1 $SERVER_HOST $SERVER_PORT)
        if [ -z $"response" ]; then
            echo "Client $client_id Request $i: [Connection failed]"
        else
            echo "Request $i: $response"
            if [[ "$response" == "ERROR: Rate limit exceeded" ]]; then
                echo "Rate limit hit at request $i"
            fi
        fi
    done
}

# Per client TokenBucket instance -- incorrect
# send_requests() {
#     local client_id=$1
#     # Send all requests over a single connection
#     {
#         for ((i=1; i<=$REQUESTS; i++)); do
#             echo -e "GET key\n"  # Add newline for proper request termination
#         done
#     } | nc $SERVER_HOST $SERVER_PORT > responses.txt
#     cat responses.txt | while read -r response; do
#         echo "Client $client_id Request: $response"
#         if [[ "$response" == "ERROR: Rate limit exceeded" ]]; then
#             echo "Rate limit hit"
#         fi
#     done
# }

for ((c=1; c<=CONCURRENT_CLIENTS; c++)); do
    send_requests &
done

wait

kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
echo "Server logs:"
cat $SERVER_LOG