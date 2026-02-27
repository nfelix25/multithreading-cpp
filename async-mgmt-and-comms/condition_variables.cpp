#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

int main() {
  std::condition_variable condition;
  std::mutex mtx;

  // No need for atmoic<bool> when using condition variables, as the condition variable will handle the synchronization and memory visibility between threads.
  bool ready = false;

  std::thread t1([&ready, &condition, &mtx /* & */](){
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // Using unique_lock instead of lock_guard so that we can unlock the mutex before notifying the condition variable.
    std::unique_lock<std::mutex> lock(mtx);
    ready = true;
    lock.unlock(); // Unlock the mutex before notifying to allow the waiting thread to proceed without being blocked by the mutex.

    condition.notify_one(); // Notify the waiting thread that the condition has been met.
  });

  // Needed for condition variable, will automatically be released when the thread goes to sleep, and reacquired when the thread wakes up.
  std::unique_lock<std::mutex> lock(mtx);

  while(!ready) {
    // KEY BIT: THIS WILL ONLY PRINT ONCE, because the condition variable will block the thread until it is notified, so we won't be looping and printing this message repeatedly.
    std::cout << "Main thread is waiting for worker thread to complete..." << std::endl;
    condition.wait(lock); // Wait for the condition variable to be notified. This will automatically release the mutex while waiting and reacquire it when notified.
  }

  std::cout << "Worker thread has completed. Ready: " << std::boolalpha << ready << std::endl;

  t1.join();

  return 0;
}


// condition_variables.cpp (time flows downward)

// Main thread (T0)                          Worker thread (T1)                    Shared state
// -----------------                         ------------------                    -------------------------
// create mtx + cv + ready=false                                                     mtx: unlocked, ready=false

// lock(mtx)                                                                        mtx: locked by T0
// while (!ready) -> true

// wait(lock):
//   1) enqueue T0 on cv wait queue
//   2) atomically unlock(mtx)
//   3) T0 sleeps                              sleep_for(2s)                        mtx: unlocked, ready=false

//                                             lock(mtx)                            mtx: locked by T1
//                                             ready = true                         ready=true
//                                             unlock(mtx)                          mtx: unlocked
//                                             notify_one(cv)  ---> wakes one waiter (T0)

// (T0 wakes, but wait returns only after
// re-locking mtx)
// relock(mtx)                                                                      mtx: locked by T0
// while (!ready) -> false (exit loop)

// print "Ready: true"
// unlock on scope end
// join(T1)


// Key behind-the-scenes rule:
// `cv.wait(lock)` is the critical atomic handoff: **release mutex + sleep** together,
// then on wakeup **reacquire mutex before continuing**.
// This prevents missed signals/races around `ready`.
