// Currently not running for instructor's demonstration due to TBB error

#include <vector>
#include <string>
#include <algorithm>
#include <execution>
#include <iostream>
#include <chrono>

struct Event {
    std::string description;
    int start_time; // Represented as an integer for simplicity
    int end_time;
};

std::vector<Event> generateSampleEvents(int num_events) {
    std::vector<Event> events;
    for (int i = 0; i < num_events; ++i) {
        events.push_back({"Event " + std::to_string(i), i * 10, (i + 1) * 10});
    }
    return events;
}

void sortEventsSequentialDefault(std::vector<Event>& events) {
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.start_time < b.start_time;
    });
}

void sortEventsSequentialExplicit(std::vector<Event>& events) {
    std::sort(std::execution::seq, events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.start_time < b.start_time;
    });
}
void sortEventsParallel(std::vector<Event>& events) {
    std::sort(std::execution::par, events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.start_time < b.start_time;
    });
}

void measurePerformance() {
    auto events = generateSampleEvents(1000000); // 1 million events

    auto start = std::chrono::high_resolution_clock::now();
    sortEventsSequentialDefault(events);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Sequential (default) sort duration: " << duration.count() << " seconds\n";

    events = generateSampleEvents(1000000); // Regenerate events

    start = std::chrono::high_resolution_clock::now();
    sortEventsSequentialExplicit(events);
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Sequential (explicit) sort duration: " << duration.count() << " seconds\n";

    events = generateSampleEvents(1000000); // Regenerate events
    start = std::chrono::high_resolution_clock::now();
    sortEventsParallel(events);
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Parallel sort duration: " << duration.count() << " seconds\n";
}

int main() {
    measurePerformance();
    return 0;
}