// Even the cpp_reference example won't compile on instructor's machine
// likely tbb version or g++ version mismatch

#include <mutex>
#include <execution>

int main(int argc, char const *argv[])
{
    int x = 0;
    std::mutex m;
    int a[] = {1, 2};
    std::for_each(std::execution::par, std::begin(a), std::end(a), [&](int)
    {
        std::lock_guard<std::mutex> guard(m);
        ++x; // correct
    });
    return 0;
}

