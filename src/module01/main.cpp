// #include <iostream>
// #include <map>
// #include <numeric>
// #include <thread>
// #include <future>

// class Student {
// public:
//     Student(const std::string& student_name, double gpa = 0) : name_(student_name), gpa_(gpa) {}

//     std::string get_name() const {
//         return name_;
//     }

//     double get_gpa() const {
//         return gpa_;
//     }

//     void take_course(const std::string course_name, const double grade_received){
//         my_grades_[course_name] = grade_received;
//     }

//     void calculate_gap(){
//         double count{0};
//         double sum = std::accumulate(my_grades_.begin(), my_grades_.end(), 0.0,
//             [&](double value, const std::map<std::string, double>::value_type& p) {
//                 count++;
//                 return value + p.second;
//             });
//         gpa_ = sum / count;
//     }

// private:    
//     std::string name_;
//     double gpa_;
//     std::map<std::string, double> my_grades_;
    
// };

// void myTask(Student& student){
//     // The tasks will update the individual student information
//     student.calculate_gap();
//     // Remove race condition on cout  move this to main thread
//     // // Output that information to a stream
//     // std::cout << student.get_name() << " gpa: " << student.get_gpa() << std::endl;
// }

// void myTask2(Student& student, std::promise<double> promise)
// {
//     student.calculate_gap();
//     promise.set_value(student.get_gpa());
// }

// int main(int argc, char const *argv[])
// {
//     std::string course_name = "C++ Multithreading";
//     double course_grade = 4.0;
//     Student student1{"Joao", 0};
//     Student student2("Angela", 0);
//     for (int i = 0; i < 10; i++){
//         course_name += std::to_string(i);
//         student1.take_course(course_name, 2.0 + i );
//         student2.take_course(course_name, 2.1 + i );
//     }        

//     // Each task updates an unique student and a promise passed by ref
//     std::promise<double> p1;
//     std::promise<double> p2;
//     auto f1 = p1.get_future();
//     auto f2 = p2.get_future();
     
//     // The is the task processing system
//     std::thread t1(myTask2, std::ref(student1), std::move(p1));
//     std::thread t2(myTask2, std::ref(student2), std::move(p2));

//     std::cout << "Tasks processing system running in parallel" << std::endl;


//     double gpa1;

//     // This is not necessary get() has a wait() internally
//     //if (f1.wait_for(std::chrono::duration{1000}) == std::future_status::ready)
//     //{
//         gpa1 = f1.get();
//     //}    
//     auto gpa2 = f2.get();


//     // // Output the unique student information to a stream
//     std::cout << student1.get_name() << " gpa: " << gpa1 << std::endl;
//     std::cout << student2.get_name() << " gpa: " << gpa2 << std::endl;

    
//     // Waiting for tasks to finish
//     t1.join();
//     t2.join();
//     return 0;
// }

#include <iostream>
#include <vector>
#include <numeric>
#include <future>

// This may or may not run on a separate thread
double calculate_gpa(const std::vector<int>& grades) {
    if (grades.empty()) return 0.0;
    double sum = std::accumulate(grades.begin(), grades.end(), 0);
    return sum / grades.size();
}

int main() {
    std::vector<int> grades = {90, 85, 78, 92, 88};

    // Get a future from std:async for the result of the task.
    std::future<double> gpa_future = std::async(std::launch::async, calculate_gpa, std::ref(grades));
    std::packaged_task<double (const std::vector<int>&)> my_task(calculate_gpa);
    std::future<double> f1 = my_task.get_future();

    // execute the task
    // assign to thread pool 
    // threadPool.assign(my_task, std::ref(grades));

    // execute it directly
    // my_task(std::ref(grades));
    // even call async

    std::thread t1(my_task, std::ref(grades));

    t1.join();
    auto value = f1.get();

    // The get() method will wait until the future is ready.
    double gpa = gpa_future.get();
    std::cout << "Calculated GPA: " << gpa << std::endl;

    return 0;
}
