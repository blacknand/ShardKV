#include "../src/server.h"
#include "../src/client.h"

#include <thread>
#include <mutex>
#include <chrono>


void run_client() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client_driver();
}


int main(int argc, char **argv) {
    std::thread server_thread(server_driver);
    std::thread client_thread(client_driver);

    server_thread.join();
    client_thread.join();

    return 0;
}