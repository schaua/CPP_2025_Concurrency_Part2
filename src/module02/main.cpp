#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <numeric>

class Student
{
public:
    Student(int id) : student_id(id) {}
    void calculate_gpa(const std::vector<int> &grades)
    {
        double average = std::accumulate(grades.begin(), grades.end(), 0.0) / grades.size();

        // This function only modifies thread data
        gpa = average;
    }
    const double get_gpa() const { return gpa; }
    const int get_id() const { return student_id; }

private:
    double gpa;
    int student_id;
};

class ThreadPool
{
public:
    ThreadPool(size_t numThreads) : stop(false)
    {
        for (size_t i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this]
                {
                    while (true) {
                    std::function<void()> task;
                    { // scope management for unique_lock
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                    } 
                }
            );
        }
    }
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using returnType = typename std::invoke_result<F, Args...>::type;
        auto task = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<returnType> result = task->get_future();

        { // scope block for lock_guard
            std::lock_guard<std::mutex> lock(queueMutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task]()
                          { (*task)(); });
        }
        condition.notify_one();
        return result;
    }
    ~ThreadPool()
    {

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
        {
            if (worker.joinable())
                worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};
// Example usage
int main()
{

    std::vector<Student> students = {
        Student(1),
        Student(2),
        Student(3)};

    std::vector<int> grades_student1 = {85, 90, 78, 92};
    std::vector<int> grades_student2 = {88, 76, 95, 89};
    std::vector<int> grades_student3 = {90, 91, 85, 87};

    std::vector<std::future<void>> futures;

    ThreadPool pool(4);
    // Enqueue tasks
    futures.push_back(pool.enqueue([&students, &grades_student1]
                                   {
          students[0].calculate_gpa(grades_student1);
          return; }));
    futures.push_back(pool.enqueue([&students, &grades_student2]
                                   {
          students[1].calculate_gpa(grades_student2);
          return; }));
    futures.push_back(pool.enqueue([&students, &grades_student3]
                                   {
          students[2].calculate_gpa(grades_student3);
          return; }));

    // Wait for all tasks to finish
    for (auto &f : futures)
    {
        f.get();
    }
    // Print the results
    for (const auto &student : students)
    {
        std::cout << "Student ID: " << student.get_id() << " GPA: " << student.get_gpa() << std::endl;
    }
    return 0;
}