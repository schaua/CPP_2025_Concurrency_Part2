#include <iostream>
#include <thread>
#include <barrier>
#include <vector>

// Moving the thread logic to access the barrier directly
// solved the runtime error on the thread logic being 
// callable.


// void schedule_task(int id, std::barrier<>& sync_point) {
//     sync_point.arrive_and_wait();
//     std::cout << "Task " << id << " is scheduled.\n";
// }


int main() {
    const int num_threads = 5;

    auto on_completion = []() noexcept 
    {
         std::cout << "All tasks are ready to be scheduled." << std::endl; 
    };
    
    std::barrier sync_point(num_threads, on_completion);

    auto schedule_task = [&](int id){
        sync_point.arrive_and_wait();
        std::cout << "Task " << id << " is scheduled.\n";
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(schedule_task, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}