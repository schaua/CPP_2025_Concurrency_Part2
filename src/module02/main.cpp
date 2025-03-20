#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <future>
#include <thread>
#include <numeric>

struct Student {
    int id;
    double gpa;
    std::vector<int> grades;
};

template <typename T>
class Pipeline {
public:
    Pipeline& load(const std::vector<T>& data) {
        this->data = data;
        return *this;
    }

    Pipeline& clean(std::function<void(T&)> cleanFunc) {
        for (auto& item : data) {
            cleanFunc(item);
        }
        return *this;
    }

    void forEach(const std::function<void(const T&)>& task) const {
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
    }

private:
    std::vector<T> data;
};

int main() {
    std::mutex output_mut;
    std::vector<Student> students = {
        {1, 0.0, {95, 85, 76}}, // 85.33
        {2, 0.0, {88, 92, 110}}, // 93.3
        {3, 0.0, {45, 60, -10}} // 35
    };

    Pipeline<Student> pipeline;

    auto cleanGrades = [](Student& student) {
        for (auto& grade : student.grades) {
            if (grade < 0) grade = 0;
            if (grade > 100) grade = 100;
        }
    };

    auto processGrades = [&output_mut](const Student& student) {
        double average = std::accumulate(student.grades.begin(), student.grades.end(), 0.0) / student.grades.size();
        std::lock_guard<std::mutex> lock(output_mut);
        std::cout << "Student " << student.id << " average grade: " << average << std::endl;
    };


    pipeline
    .load(students)
    .clean(cleanGrades)
    .forEach(processGrades);

    // auto result = pipelineSystem.load(students);
    // auto result2 = pipelineSystem.clean(result2);
    // auto result3 = pipelineSystem.process(result2)
    // do something with result3

    return 0;
}
