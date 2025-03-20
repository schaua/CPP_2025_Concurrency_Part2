#include <iostream>
#include <map>
#include <numeric>
#include <thread>

class Student {
public:
    Student(const std::string& student_name, double gpa = 0) : name_(student_name), gpa_(gpa) {}

    std::string get_name() const {
        return name_;
    }

    double get_gpa() const {
        return gpa_;
    }

    void take_course(const std::string course_name, const double grade_received){
        my_grades_[course_name] = grade_received;
    }

    void calculate_gap(){
        double count{0};
        double sum = std::accumulate(my_grades_.begin(), my_grades_.end(), 0.0,
            [&](double value, const std::map<std::string, double>::value_type& p) {
                count++;
                return value + p.second;
            });
        gpa_ = sum / count;
    }

private:    
    std::string name_;
    double gpa_;
    std::map<std::string, double> my_grades_;
    
};

void myTask(Student& student){
    // The tasks will update the individual student information
    student.calculate_gap();
    // Remove race condition on cout  move this to main thread
    // // Output that information to a stream
    // std::cout << student.get_name() << " gpa: " << student.get_gpa() << std::endl;
}

int main(int argc, char const *argv[])
{
    std::string course_name = "C++ Multithreading";
    double course_grade = 4.0;
    Student student1{"Joao", 0};
    Student student2("Angela", 0);
    for (int i = 0; i < 10; i++){
        course_name += std::to_string(i);
        student1.take_course(course_name, 2.0 + i );
        student2.take_course(course_name, 2.1 + i );
    }        

    // Each task updates an unique student
    
    // The is the task processing system
    std::thread t1(myTask, std::ref(student1));
    std::thread t2(myTask, std::ref(student2));

    std::cout << "Tasks processing system running in parallel" << std::endl;

    // Waiting for tasks to finish
    t1.join();
    t2.join();

    // // Output the unique student information to a stream
    std::cout << student1.get_name() << " gpa: " << student1.get_gpa() << std::endl;
    std::cout << student2.get_name() << " gpa: " << student2.get_gpa() << std::endl;

    
    return 0;
}
