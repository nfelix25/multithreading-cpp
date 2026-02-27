#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

int main() {
  // Seed the random number generator once
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  std::condition_variable condition;
  std::mutex mtx;

  bool ready = false;
  std::atomic<bool> stop_noise{false};

  // Sends random "false alarms" to cause spurious wakeups
  std::thread noisy_notifier([&] {
    while (!stop_noise.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(300 + std::rand() % 1000));
      if (!stop_noise.load()) {
        std::cout << "Noisy notifier: notify_one() without changing shared state (wake may be non-actionable) (mutex: unchanged/not held by notifier).\n\n";
        condition.notify_one(); // notify without changing ready
      }
    }
  });

  std::thread worker([&](){
    std::cout << "Worker: starting simulated work (worker lock: not held).\n\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    std::cout << "Worker: attempting to acquire mutex to update `ready` (worker lock: not held yet).\n";
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Worker: mutex acquired; setting `ready=true` (mutex: locked by worker).\n";
    ready = true;
    lock.unlock();
    std::cout << "Worker: mutex released; calling notify_one() (mutex: unlocked).\n\n";
    condition.notify_one();
  });

  std::cout << "Main: acquiring mutex before wait (main lock: not held yet).\n";
  std::unique_lock<std::mutex> lock(mtx);

  // This check loop guards against spurious wakeups, which can occur when a thread is woken up without the condition being met. (Eg noisy_notifier)
  // By re-checking the condition in a loop, we ensure that the thread only proceeds when the condition is actually satisfied.
  // while(!ready) {
  //   std::cout << "Main thread is waiting for worker thread to complete..." << std::endl;
  //   condition.wait(lock);
  // }

  // condition_wait accepts a 2nd argument which is a predicate that will be evaluated after the thread is woken up, and if it returns false, the thread will go back to waiting. This is a more concise way to handle spurious wakeups without needing an explicit loop.

  condition.wait(lock, [&] {
    std::cout << "Main: predicate running with mutex held (mutex: locked by main).\n";
    if (!ready) {
      std::cout << "Main: `ready` is false; wait() will unlock mutex and block, then re-lock before rechecking (mutex: locked now, unlocked during wait).\n\n";
    }
    return ready;
  });

  std::cout << "Main: condition satisfied. Ready: " << std::boolalpha << ready << " (mutex: locked by main until `lock` goes out of scope)." << std::endl;

  stop_noise = true;
  noisy_notifier.join();
  worker.join();

  return 0;
}
