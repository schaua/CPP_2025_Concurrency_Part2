#include <iostream>
#include <thread>
#include <chrono>
#include <stop_token>

void workerFunction(std::stop_token stopToken)
{
        int count = 0;
    // Register a callback that gets triggered when stop is requested
    std::stop_callback callback(stopToken, []
                               { std::cout << "[Worker] Stop requested! Cleaning up resources...\n"; });
    while (!stopToken.stop_requested())
    {
        std::cout << "[Worker] Running iteration " << ++count << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "[Worker] Gracefully exiting...\n";
}

int main()
{
    std::cout << "[Main] Starting worker thread...\n";
    // Create a stop_source and pass its stop_tokento the worker
    std::jthread worker(workerFunction);
    // Let the worker run for 3 seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "[Main] Requesting worker to stop...\n";
    worker.request_stop(); // Signal the worker to stop
    std::cout << "[Main] Waiting for worker to finish...\n";
    return 0;
}
