#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
std::queue<int> queue; // Shared queue
const size_t queueSize = 10; // Maximum queue size
std::mutex mtx;
std::condition_variable cv;

void producer(int id, int itemCount) {
  for (int i = 0; i < itemCount; ++i) {
    std::unique_lock<std::mutex> lock(mtx);
    // Wait if the queue is full
    cv.wait(lock, [] { return queue.size() < queueSize; });
    // Produce an item
    int item = id * 100 + i; // Unique item ID
    queue.push(item);
    std::cout << "Producer " << id << " produced: " << item << "\n";
    // Because this will sleep the unlock and then notify_all order is important
    lock.unlock();
    cv.notify_all(); // Notify consumers
    // Never hold a lock when going to sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
  }
}
void consumer(int id, int itemCount) {
  for (int i = 0; i < itemCount; ++i) {
    std::unique_lock<std::mutex> lock(mtx);
    // Wait if the queue is empty
    cv.wait(lock, [] { return !queue.empty(); });
    // Consume an item
    int item = queue.front();
    queue.pop();
    std::cout << "Consumer " << id << " consumed: " << item << "\n";
    lock.unlock();
    cv.notify_all(); // Notify producers
    std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Simulate work
  }
}
int main() {
  const int producerCount = 2;
  const int consumerCount = 2;
  const int itemsPerProducer = 5;
  std::vector<std::thread> producers, consumers;
  // Create producer threads
  for (int i = 0; i < producerCount; ++i) {
    producers.emplace_back(producer, i + 1, itemsPerProducer);
  }
  // Create consumer threads
  for (int i = 0; i < consumerCount; ++i) {
    consumers.emplace_back(consumer, i + 1, (producerCount * itemsPerProducer) / consumerCount);
  }
  // Join all threads
  for (auto& p : producers) p.join();
  for (auto& c : consumers) c.join();
  return 0;
}

