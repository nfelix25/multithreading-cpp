#include <iostream>
#include <future>
#include <chrono>
#include <mutex>

int work(int id, std::mutex& mtx) {
  int ret = id;

  for (int i = 0; i < 3; i++) {
    mtx.lock();
    std::cout << "Worker " << id << " is working..." << std::endl;
    mtx.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ret += i;
  }

  return ret;
}

int main() {
  // Async handles the management of the thread for us, so we don't have to worry about joining or detaching the thread.

  /*
   * If you don't store the returned std::future (e.g., in a local variable),
   * a temporary std::future object is created and immediately destroyed at the end of the full expression
   * MAKING IT BLOCKING.
   */

  std::mutex mtx;

  // Deferred launch policy will execute the function only when its result is needed (eg future.get), running without get will not print out anything.
  std::async(std::launch::deferred, work, 1, std::ref(mtx));

  // Default launch policy is acyns if async is available, otherwise deferred. So this will run the function asynchronously in a separate thread.
  std::async(work, 2, std::ref(mtx)); // Equiv to std::async(std::launch::async, , work, 2);

  std::future<int> f1 = std::async(std::launch::deferred, work, 3, std::ref(mtx));

  f1.get(); // This will execute the deferred function and print out the output for worker 3.

  /* BLOCKING --- not async */
  // Worker 2 is working...
  // Worker 2 is working...
  // Worker 2 is working...
  // Worker 3 is working...
  // Worker 3 is working...
  // Worker 3 is working...

  std::future<int> not_used_but_needed = std::async(work, 4, std::ref(mtx)); // Even if there was no return value, we would still need to store the future in a variable to avoid blocking behavior.

  std::future<int> f2 = std::async(std::launch::deferred, work, 5, std::ref(mtx));

  std::cout << f2.get() << std::endl; // This will execute the deferred function and print out the output for worker 5.

  /* Now non-blocking --- async as intended */
  // Worker 5 is working...
  // Worker 4 is working...
  // Worker 5 is working...
  // Worker 4 is working...
  // Worker 5 is working...
  // Worker 4 is working...

  return 0;
}
