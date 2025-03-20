#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <shared_mutex>
#include <numeric>
#include <condition_variable>

std::shared_mutex mtx;
std::condition_variable_any cv;
std::map<int, double> student_gpa;
int completed_threads = 0;

void calculate_gpa(const int student_id, const std::vector<int>& grades) {
    double average = std::accumulate(grades.begin(), grades.end(), 0.0) / grades.size();
    
    {
        student_gpa[student_id] = average;
        // This thread is going to modify the shared data, so we need to lock the shared mutex exclusively.
        std::unique_lock<std::shared_mutex> lock(mtx);
        completed_threads++;  // Shared information that needs to be protected from concurrent access
    }
    cv.notify_one();
}

int main() {
    std::vector<int> grades_student1 = {85, 90, 78, 92};
    std::vector<int> grades_student2 = {88, 76, 95, 89};
    std::vector<int> grades_student3 = {90, 91, 85, 87};

    std::jthread t1(calculate_gpa, 1, std::ref(grades_student1));
    std::jthread t2(calculate_gpa, 2, std::ref(grades_student2));
    std::jthread t3(calculate_gpa, 3, std::ref(grades_student3));

    {
        // The main thread waits until all the threads have completed.
        std::unique_lock<std::shared_mutex> lock(mtx);

        // While waiting for the condition variable, the shared mutex is unlocked
        // allowing any worker thread to lock the shared mutex and modify the shared data.
        // The thread keeps waiting until the provided lambda returns true.
        cv.wait(lock, [] { return completed_threads == 3; });
    }  // need the unique_lock to be released before the shared_lock can be obtained
       // otherwise, the shared_lock will block indefinitely - deadlock!

    // Use shared lock to get a read lock on the shared data
    std::shared_lock<std::shared_mutex> read_lock(mtx);
    for (const auto& [id, gpa] : student_gpa) {
        std::cout << "Student ID: " << id << ", GPA: " << gpa << std::endl;
    }

    return 0;
}