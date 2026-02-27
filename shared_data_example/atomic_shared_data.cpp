#include <iostream>
#include <thread>
#include <atomic>

using namespace std;

/* *** How Atomics Fix It ***
 * Declaring `std::atomic<int> count{0};` makes the compiler generate a single atomic
 * instruction (e.g., `lock add DWORD PTR [count], 1` or a compare-and-swap loop).
 * The `lock` prefix forces the CPU to treat the update as indivisible across cores, so
 * the read-modify-write can’t be interleaved, and every increment is counted.
 */

int main() {
  atomic<int> count(0); // Must use constructor to initialize an atomic variable

  const int ITERATIONS = 1E6;

  thread t1([&count](){
    for (int i = 0; i < ITERATIONS; i++) {
      count++;
    }
  });

  thread t2([&count](){
    for (int i = 0; i < ITERATIONS; i++) {
      count++;
    }
  });

  t1.join();
  t2.join();

  cout << "Count: " << count << endl;

  return 0;
}
