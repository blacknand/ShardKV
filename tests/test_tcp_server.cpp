#include "../src/server.h"
#include "../src/client.h"

#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>


void run_client() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    client_driver();
}


int main(int argc, char **argv) {
    std::thread server_thread(server_driver);
    std::thread client_thread(run_client);

    client_thread.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (server_thread.joinable()) 
        server_thread.join();

    std::cout << "[INFO] TCP server testing finished" << std::endl;

    return 0;
}