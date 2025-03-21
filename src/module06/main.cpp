#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <vector>

template <typename T>
class ThreadSafeQueue {
public:
    void enqueue(T item) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(std::move(item));
        cv.notify_one();
    }

    T dequeue() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !q.empty(); });
        T item = std::move(q.front());
        q.pop();
        return item;
    }

private:
    std::queue<T> q;
    std::mutex mtx;
    std::condition_variable cv;
};

std::mutex mut;

void producer(ThreadSafeQueue<int>& queue, int id) {
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        queue.enqueue(i + id * 100);
        {
            std::lock_guard<std::mutex> lock(mut);
            std::cout << "Producer " << id << " enqueued " << i + id * 100 << std::endl;
        }
    }
}

void consumer(ThreadSafeQueue<int>& queue, int id) {
    for (int i = 0; i < 10; ++i) {
        int item = queue.dequeue();
        {
            std::lock_guard<std::mutex> lock(mut);
            std::cout << "Consumer " << id << " dequeued " << item << std::endl;
        }
    }
}

int main() {
    ThreadSafeQueue<int> queue;

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    const int number_of_threads{8};
    for (int i = 0; i < number_of_threads; ++i) {
        producers.emplace_back(producer, std::ref(queue), i);
        consumers.emplace_back(consumer, std::ref(queue), i);
    }

    for (auto& producer : producers) {
        producer.join();
    }

    for (auto& consumer : consumers) {
        consumer.join();
    }

    return 0;
}