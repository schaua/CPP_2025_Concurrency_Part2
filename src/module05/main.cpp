#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <chrono>

// Thread-safe queue template for pipeline stages
template<typename T>
class ConcurrentQueue {
public:
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
        condVar_.notify_one();
    }
    bool try_pop(T& result) {
        std::lock_guard<std::mutex> lock(mutex_);
        if(queue_.empty()) return false;
        result = queue_.front();
        queue_.pop();
        return true;
    }
    void wait_pop(T& result) {
        std::unique_lock<std::mutex> lock(mutex_);
        condVar_.wait(lock, [this] { return !queue_.empty() || finished_; });
        if(!queue_.empty()) {
            result = queue_.front();
            queue_.pop();
        }
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    void set_finished() {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        condVar_.notify_all();
    }
    bool is_finished() {
        std::lock_guard<std::mutex> lock(mutex_);
        return finished_;
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condVar_;
    bool finished_{false};
};

// FileLister: Lists files from a given directory.
class FileLister {
public:
    FileLister(const std::string& directory, ConcurrentQueue<std::string>& queue)
      : directory_(directory), queue_(queue) {}

    void list_files() {
        for (const auto& entry : std::filesystem::directory_iterator(directory_)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().string();
                std::cout << "[Lister] Found file: " << filename << "\n";
                queue_.push(filename);
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate delay
            }
        }
        queue_.set_finished();
    }
private:
    std::string directory_;
    ConcurrentQueue<std::string>& queue_;
};

// FileValidator: Validates file format (here, ensuring a ".txt" extension).
class FileValidator {
public:
    FileValidator(ConcurrentQueue<std::string>& inQueue, ConcurrentQueue<std::string>& outQueue)
      : inQueue_(inQueue), outQueue_(outQueue) {}

    void validate_files() {
        std::string filename;
        while (!inQueue_.is_finished()) {
            inQueue_.wait_pop(filename);
            if (filename.empty() && inQueue_.empty()) break;
            if (filename.size() >= 4 && filename.substr(filename.size()-4) == ".cpp") {
                std::cout << "[Validator] Validated file: " << filename << "\n";
                outQueue_.push(filename);
            } else {
                std::cout << "[Validator] Invalid file: " << filename << "\n";
            }
        }
        outQueue_.set_finished();
    }
private:
    ConcurrentQueue<std::string>& inQueue_;
    ConcurrentQueue<std::string>& outQueue_;
};

// FileProcessor: Processes validated files.
class FileProcessor {
public:
    FileProcessor(ConcurrentQueue<std::string>& queue)
      : queue_(queue) {}

    void process_files() {
        std::string filename;
        while (!queue_.is_finished()) {
            queue_.wait_pop(filename);
            if (filename.empty() && queue_.empty()) break;
            std::cout << "[Processor] Processing file: " << filename << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Simulate work
        }
    }
private:
    ConcurrentQueue<std::string>& queue_;
};

int main() {
    // Create queues for pipeline stages
    ConcurrentQueue<std::string> fileQueue;
    ConcurrentQueue<std::string> validatedQueue;

    // Instantiate pipeline components
    FileLister lister("C:/projects/CPP_2025_Concurrency_Part2/src/module05", fileQueue);
    FileValidator validator(fileQueue, validatedQueue);
    FileProcessor processor(validatedQueue);

    // Look for bug in Validator and Processor duplicating effort on the same file
    // Run each stage concurrently
    std::thread t1(&FileLister::list_files, &lister);
    std::thread t2(&FileValidator::validate_files, &validator);
    std::thread t3(&FileProcessor::process_files, &processor);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "Pipeline processing complete.\n";
    return 0;
}
