#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>
#include <chrono>

void print_vector(const std::vector<int>& vec) {
    for (const auto& v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> data(1000000);
    std::iota(data.begin(), data.end(), 0); // Fill with 0, 1, 2, ..., 999999

    std::vector<int> result(data.size());

    auto start = std::chrono::high_resolution_clock::now();
    std::transform(data.begin(), data.end(), result.begin(), [](int x) { return x * 2; });
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Default transform duration: " << duration.count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::transform(std::execution::seq, data.begin(), data.end(), result.begin(), [](int x) { return x * 2; });
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Sequential transform duration: " << duration.count() << " seconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    std::transform(std::execution::par, data.begin(), data.end(), result.begin(), [](int x) { return x * 2; });
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Parallel transform duration: " << duration.count() << " seconds" << std::endl;

    return 0;
}