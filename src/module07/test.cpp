#include <coroutine>
#include <iostream>
#include <queue>
#include <optional>
#include <thread>
#include <chrono>

template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T value;
        std::exception_ptr exception;

        Generator get_return_object() {
            return Generator{handle_type::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T v) {
            value = v;
            return {};
        }

        void return_void() {}

        void unhandled_exception() {
            exception = std::current_exception();
        }
    };

    handle_type coro;

    Generator(handle_type h) : coro(h) {}
    ~Generator() { if (coro) coro.destroy(); }

    std::optional<T> next() {
        if (coro.done()) return std::nullopt;
        coro.resume();
        if (coro.promise().exception) std::rethrow_exception(coro.promise().exception);
        return coro.promise().value;
    }
};

Generator<int> producer(int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << "generated: " << i << std::endl;
        co_yield i;
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
    }
}

void consumer(Generator<int>& gen) {
    while (auto value = gen.next()) {
        std::cout << "Consumed: " << *value << std::endl;
    }
}

int main() {
    auto gen = producer(10);
    consumer(gen);
    return 0;
}