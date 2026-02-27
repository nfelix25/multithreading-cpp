#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

const int ITERATIONS = 1E6;

int main() {
  int count = 0;

  mutex mtx;

  auto func1 = [& /* Ampersand alone captures any local vars by ref */](){
    for(int i = 0; i < ITERATIONS; i++) {
      mtx.lock(); // Manually lock the mutex before modifying `count`
      count++;
      mtx.unlock(); // Manually unlock the mutex after modifying `count`
    }

    // Locking inside the loop (one `lock_guard` per iteration) makes each increment mutually exclusive but only for the instant it takes to update `count`. That allows other non-critical work in the loop to overlap across threads, but each iteration pays the cost of lock/unlock, which can dominate the run time if the per-iteration work is tiny (like just `++count`). With a high iteration count and little work per iteration, the fine-grained locking can be significantly slower than taking the lock once.
  };

  // The two functions server to show mutex options, both still access the same `count` and `mtx` by reference. Eg without the below guard the total ends up being not quite 2E6 eg 1978122
  auto func2 = [&count, &mtx](){
    lock_guard<mutex> guard(mtx); // RAII-style lock that automatically releases when `guard` goes out of scope (function ends in this case, or loop block if placed inside for)
    for(int i = 0; i < ITERATIONS; i++) {
      // The guard could also be placed here and functioanlly be the same as the manual lock/unlock with minor add'l overhead
      count++;
    }

    // Locking once around the entire `for` block keeps the critical section coarse: there’s just one mutex acquisition per thread, so the overhead of locking/unlocking is minimal. The downside is zero concurrency—while one thread holds the mutex, the other can’t touch the shared data at all, so both effectively run serially for the whole loop.
  };

  thread t1(func1);
  thread t2(func2);

  t1.join();
  t2.join();

  cout << "Count: " << count << endl;

  int iter = 5;

  auto lock_in_func = [&](){
    mtx.lock();
    for(int i = 0; i < iter; i++) {
      cout << "Thread " << this_thread::get_id() << " has the lock in lock_in_func" << endl;
      count++;
      this_thread::sleep_for(chrono::milliseconds(100)); // Simulate some work while holding the lock
    }
    mtx.unlock();
  };

  auto lock_in_for = [&](){
    for(int i = 0; i < iter; i++) {
      mtx.lock();
      cout << "Thread " << this_thread::get_id() << " has the lock in lock_in_for" << endl;
      count++;
      mtx.unlock();
      this_thread::sleep_for(chrono::milliseconds(100)); // Simulate some work while holding the lock
    }
  };

  /* The lock_in_func interations all take place consecutively for the
   * different threads in turn. All t3 and then all t4.
   *
   * Whereas the lock_in_for threads will alternate individual counts
   * between t5 and t6, illustrating what was said in the comments earlier.
   */

  // Thread 0x70000cc13000 has the lock in lock_in_func
  // Thread 0x70000cc13000 has the lock in lock_in_func
  // Thread 0x70000cc13000 has the lock in lock_in_func
  // Thread 0x70000cc13000 has the lock in lock_in_func
  // Thread 0x70000cc13000 has the lock in lock_in_func
  // Thread 0x70000cc96000 has the lock in lock_in_func
  // Thread 0x70000cc96000 has the lock in lock_in_func
  // Thread 0x70000cc96000 has the lock in lock_in_func
  // Thread 0x70000cc96000 has the lock in lock_in_func
  // Thread 0x70000cc96000 has the lock in lock_in_func
  // Thread 0x70000cc13000 has the lock in lock_in_for
  // Thread 0x70000cc96000 has the lock in lock_in_for
  // Thread 0x70000cc13000 has the lock in lock_in_for
  // Thread 0x70000cc96000 has the lock in lock_in_for
  // Thread 0x70000cc13000 has the lock in lock_in_for
  // Thread 0x70000cc96000 has the lock in lock_in_for
  // Thread 0x70000cc13000 has the lock in lock_in_for
  // Thread 0x70000cc96000 has the lock in lock_in_for
  // Thread 0x70000cc13000 has the lock in lock_in_for
  // Thread 0x70000cc96000 has the lock in lock_in_for

  thread t3(lock_in_func);
  thread t4(lock_in_func);

  t3.join();
  t4.join();

  thread t5 = thread(lock_in_for);
  thread t6 = thread(lock_in_for);

  t5.join();
  t6.join();

  return 0;
}
