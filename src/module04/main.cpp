#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

class TaskLogic
{
public:
    void increment(const int loops, int &counter)
    {
        for (int i{0}; i < loops; ++i)
        {
            ++counter;
        }
    }
};

// Create the shared value
int shared_counter = 0;

void test_increment_single_thread(const int count)
{
    std::cout << "Starting test_increment_single_thread" << std::endl;
    TaskLogic task;
    task.increment(count, shared_counter);

}

void test_increment_multiple_threads(const int count, const int threadCount) {
    std::cout << "Starting test_increment_multiple_threads" << std::endl;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < threadCount; ++i) {
        threads.push_back(std::thread([&]() {
            TaskLogic task;
            task.increment(count, shared_counter);
        }));
    }

    for (auto& t : threads) {
        t.join();
    }   

}

int main(int argc, char const *argv[])
{
    const int expected{100000};
    test_increment_single_thread(expected);
    // Should be expected
    std::cout << "Counter value should be: " << expected << std::endl;
    std::cout << "Counter value is: " << shared_counter << std::endl;
    assert(shared_counter == expected);
    // Prove that it can fail
    // assert(shared_counter == 1);

    // const int threadCount = 4;
    // const int targetResult{expected*threadCount};

    // test_increment_multiple_threads(expected, threadCount);
    // // Expected output: targetResult
    // std::cout << "Final counter value should be: " << targetResult << std::endl;
    // std::cout << "Final counter value is: " << shared_counter << std::endl;
    // // assert(shared_counter == targetResult);

    return 0;
}
