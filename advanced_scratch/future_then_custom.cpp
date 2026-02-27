#include <iostream>
#include <future>
#include <thread>
#include <utility>

// A generic then function wrapper for std::future
template<typename Fut, typename Work>
auto then(Fut f, Work w) -> std::future<decltype(w(f.get()))> {
    return std::async(std::launch::async, [f = std::move(f), w = std::move(w)]() mutable {
        // f.get() blocks until the value is ready, then the lambda runs asynchronously
        return w(f.get());
    });
}

// Example asynchronous function
std::future<int> async_add(int a, int b) {
    return std::async(std::launch::async, [=](){
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate work
        return a + b;
    });
}

int main() {
    // 1. Start the initial asynchronous operation
    auto future_sum = async_add(40, 2);

    // 2. Attach a continuation using the 'then' wrapper
    auto future_result = then(std::move(future_sum), [](int n) {
        std::cout << "Sum is " << n << std::endl;
        return n * 2; // Return a new value for the next stage
    });

    // 3. Attach another continuation
    auto future_final = then(std::move(future_result), [](int n) {
        std::cout << "Result doubled is " << n << std::endl;
    });

    // 4. Wait for the final result (or the chain to complete)
    future_final.get(); // get() blocks until the result is available

    return 0;
}
