#include "thread_pool.h"


void ThreadPool::enqueue(std::function<void()> task) {
    boost::asio::post(pool, std::move(task));
}


void ThreadPool::wait() {
    pool.join();
}