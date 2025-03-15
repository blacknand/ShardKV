#include <boost/asio.hpp>
#include <iostream>


class ThreadPool {
public:
    ThreadPool(size_t num_threads) : pool(num_threads) {}
    void enqueue(std::function<void()> task);
    void wait();

private:
    boost::asio::thread_pool pool;
}