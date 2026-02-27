#include <iostream>
#include <thread>
#include <chrono>

void work() {
  std::cout << "Work thread id: " << std::this_thread::get_id() << std::endl;

  for (int i = 0; i < 10; i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Loop " << i << std::endl;
  }
}

int main() {
  std::thread t1(work);
  std::thread t2(work);

  t1.join();
  t2.join();

  // log current thread id
  std::cout << "Main thread id: " << std::this_thread::get_id() << std::endl;
}

/* both worker threads write to `std::cout` without synchronization.
 * Each insertion (`<< "Work thread id: "`, `<< std::this_thread::get_id()`, `<< std::endl`)
 * is a separate operation, and the scheduler can switch threads between any two operations.
 *
 * This leads to non-deterministic output, one example:
 * Work thread id: 0x700006d37000
 * Work thread id: 0x700006dba000
 * Loop Loop 0
 * 0
 * Loop 1
 * Loop 1
 * Loop 2
 * Loop 2
 * Loop Loop 3
 * ...
 */
