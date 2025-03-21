#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <glog/logging.h>

std::mutex mut;
void log(const std::string message, std::string level)
{
    std::lock_guard<std::mutex> lock_clog(mut);
    std::clog << level << ": " << message << std::endl;
    LOG(INFO) << message << std::endl;
}

class TaskLogic
{
public:
    // void increment(const int loops, std::atomic<int> &counter)
    void increment(const int loops, int &counter)
    {
        for (int i{0}; i < loops; ++i)
        {
            ++counter;
        }
        log( "Current counter is "   + std::to_string(counter), "INFO");
    }
};

// Create the shared value
// One solution to a shared counter is 
// to make it atomic
// std::atomic<int> shared_counter = 0;
int shared_counter = 0;

void test_increment_single_thread(const int count)
{
    std::cout << "Starting test_increment_single_thread" << std::endl;
    TaskLogic task;
    task.increment(count, shared_counter);
}

void test_increment_multiple_threads(const int count, const int threadCount)
{
    std::cout << "Starting test_increment_multiple_threads" << std::endl;
    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i)
    {
        threads.push_back(std::thread([&]()
                                      {
            TaskLogic task;
            task.increment(count, shared_counter); }));
    }

    for (auto &t : threads)
    {
        t.join();
    }
}

int main(int argc, char const *argv[])
{

    // Important to initialize the logging
    google::InitGoogleLogging(argv[0]);
    
    // Set the destination for logging
    FLAGS_logtostderr = false;
    FLAGS_log_dir = "C:/projects/logs";

    // shared_counter has been initialized to 0
    shared_counter = 0;

    const int expected{100000};
    // test_increment_single_thread(expected);
    // // Should be expected
    // std::cout << "Counter value should be: " << expected << std::endl;
    // std::cout << "Counter value is: " << shared_counter << std::endl;
    // assert(shared_counter == expected);

    // Prove that it can fail
    // assert(shared_counter == 1);

    // Reset the environment
    shared_counter = 0;

    const int threadCount = 4;
    const int targetResult{expected * threadCount};

    test_increment_multiple_threads(expected, threadCount);
    // Expected output: targetResult
    std::cout << "Final counter value should be: " << targetResult << std::endl;
    std::cout << "Final counter value is: " << shared_counter << std::endl;
    // assert(shared_counter == targetResult);

    return 0;
}
