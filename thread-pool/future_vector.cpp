#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <mutex>

std::mutex g_mtx;

int work(int id)
{
  std::unique_lock<std::mutex> lock(g_mtx);
  std::cout << "Worker " << id << " is working..." << std::endl;
  lock.unlock();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  return id;
}

int main ()
{
  // Get number of hardware cores
  const int NUM_THREADS = std::thread::hardware_concurrency();

  for (int i = 0; i < NUM_THREADS; i++) {
    // This future will run syncronously as the bariable f goes out of scope and its destructor is BLOCKING, waiting for the thread to finish before moving on to the next iteration of the loop. So we will see the output for worker 0, then worker 1, etc. in order.
    std::future<int> f = async(std::launch::async, work, i);
  }

  //Worker 0 is working...
  // Worker 1 is working...
  // Worker 2 is working...
  // Worker 3 is working...

  // We can solve the above using a vector to store the futures, so that they don't go out of scope until we are done with them. This will allow all the threads to run concurrently and we will see the output for all workers in a non-deterministic order.
  std::vector<std::future<int>> futures;

  // However, if we do the below it will not compile as futures cannot be copied, we could just move the future creation into the vector. But, will instead introduce shared futures
  // for (int i = 0; i < NUM_THREADS; i++) {
  //   std::future<int> f = async(std::launch::async, work, i);
  //   futures.push_back(f);
  // }

  // Shared futures have a constructor which takes a future and allows it to be shared across multiple threads. This allows us to store the futures in a vector without having to worry about them going out of scope and blocking the main thread.
  std::vector<std::shared_future<int>> shared_futures;

  for (int i = 0; i < NUM_THREADS; i++) {
    std::shared_future<int> f = std::async(std::launch::async, work, i);
    shared_futures.push_back(f);
  }

  // Worker 0 is working...
  // Worker 2 is working...
  // Worker 3 is working...
  // Worker 1 is working...

  for (auto f : shared_futures) {
    std:: cout << "Returned: " << f.get() << std::endl; // This will block until the future is ready, but since all the futures are running concurrently, we will see the output for all workers before we see the output for the return values.
  }

  return 0;
}
