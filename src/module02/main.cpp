#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <future>
#include <thread>
#include <numeric>
#include <queue>
#include <mutex>
#include <condition_variable>

struct Student
{
    int id;
    double gpa;
    std::vector<int> grades;
};

template <typename T>
class Actor
{
public:
    Actor() : stop(false)
    {
        worker = std::thread([this]()
                             { this->processMessages(); });
    }

    ~Actor()
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        cv.notify_all();
        if (worker.joinable())
        {
            worker.join();
        }
    }

    void send(std::function<void()> message)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            message_queue.push(message);
        }
        cv.notify_one();
    }

private:
    void processMessages()
    {
        while (true)
        {
            std::function<void()> message;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                cv.wait(lock, [this]()
                        { return !message_queue.empty() || stop; });
                if (stop && message_queue.empty())
                {
                    return;
                }
                message = std::move(message_queue.front());
                message_queue.pop();
            }
            message(); // actor is too flexible
        }
    }

    std::thread worker;
    std::queue<std::function<void()>> message_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    bool stop;
};

template <typename T>
class ActorSystem : public Actor<T>
{
public:
    std::future<void> get_future()
    {
        return whenDone.get_future();
    }

    void load(const std::vector<T> &data)
    {
        this->send([this, data]()
                   { this->data = data; });
    }

    void clean(std::function<void(T &)> cleanFunc)
    {
        this->send([this, cleanFunc]()
                   {
            for (auto& item : data) {
                cleanFunc(item);
            } });
    }

    void forEach(const std::function<void(const T &)> &task)
    {
        this->send([this, task]()
                   {
            std::vector<std::thread> threads;
            for (const auto& item : data) {
                threads.emplace_back([&task, &item]() {
                    task(item);
                });
            }
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
                
            }
            whenDone.set_value();
        });
    }

private:
    std::promise<void> whenDone;
    std::vector<T> data;
};

int main()
{
    std::mutex output_mut;
    std::vector<Student> students = {
        {1, 0.0, {95, 85, 76}},
        {2, 0.0, {88, 92, 110}},
        {3, 0.0, {45, 60, -10}}};

    ActorSystem<Student> actorSystem;
    auto whenDone = actorSystem.get_future();    
    auto cleanGrades = [](Student &student)
    {
        for (auto &grade : student.grades)
        {
            if (grade < 0)
                grade = 0;
            if (grade > 100)
                grade = 100;
        }
    };

    auto processGrades = [&output_mut](const Student &student)
    {
        double average = std::accumulate(student.grades.begin(), student.grades.end(), 0.0) / student.grades.size();
        std::lock_guard<std::mutex> lock(output_mut);
        std::cout << "Student " << student.id << " average grade: " << average << std::endl;
    };

    actorSystem.load(students);
    actorSystem.clean(cleanGrades);
    actorSystem.forEach(processGrades);

    // Make sure the actor is done processing all messages before exiting
    whenDone.get();

    return 0;
}
