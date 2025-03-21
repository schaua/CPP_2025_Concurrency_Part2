#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <vector>
#include<oneapi/tbb/concurrent_queue.h>

template <typename T>
class ThreadSafeQueue
{
public:
    void pop()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue_.empty())
            return;
        queue_.pop();
    }
    void push(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        try
        {
            queue_.push(value);
            cv.notify_one();
        }
        catch (std::string message)
        {
            return;
        }
    }
    T &front()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue_.empty())
            emptyElement;
        return queue_.front();
    }
    T &back()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue_.empty())
            emptyElement;
        return queue_.back();
    }

    T& dequeue()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]
                { return !queue_.empty(); });
        T& item = queue_.front();
        queue_.pop(); 
        return item;
    }
    
    bool empty()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;
    std::mutex mtx;
    std::condition_variable cv;
    T emptyElement;
};

std::mutex mut;

void producer(oneapi::tbb::concurrent_queue<int> &q, int id)
{
    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        q.push(i + id * 100);
        {
            std::lock_guard<std::mutex> lock(mut);
            std::cout << "Producer " << id << " enqueued " << i + id * 100 << std::endl;
        }
    }
}

void consumer(oneapi::tbb::concurrent_queue<int> &q, int id)
{
    for (int i = 0; i < 10; ++i)
    {
        // Two thread safe operations
        // in sequence are not
        // threadsafe
        // int item = q.front();
        // q.pop();
        // our custom queue
        //int& item = q.dequeue();
        // tbb queue
        int item;
        while (!q.try_pop(item)){}

        {
            std::lock_guard<std::mutex> lock(mut);
            std::cout << "Consumer " << id << " dequeued " << item << std::endl;
        }
    }
}

int main()
{
    // ThreadSafeQueue<int> localqueue;
    oneapi::tbb::concurrent_queue<int> localqueue;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    const int number_of_threads{8};
    for (int i = 0; i < number_of_threads; ++i)
    {
        producers.emplace_back(producer, std::ref(localqueue), i);
        consumers.emplace_back(consumer, std::ref(localqueue), i);
    }

    for (auto &producer : producers)
    {
        producer.join();
    }

    for (auto &consumer : consumers)
    {
        consumer.join();
    }

    return 0;
}
