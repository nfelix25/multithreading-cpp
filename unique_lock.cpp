#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

void work(int &count, timed_mutex &mtx) {
  for (int i = 0; i < 5; i++) {
    // lock_guard<mutex> guard(mtx);
    // Instead of lock_guard, we can use unique_lock which is more flexible and allows us to lock and unlock manually. It also supports deferred locking, timed locking, and recursive locking.
    // We need defer_lock to be able to manually lock and unlock the mutex, otherwise the unique_lock will automatically acquire the lock when it is created, which is not what we want in this case since we want to try to acquire the lock with a timeout.
    unique_lock<timed_mutex> lock(mtx, defer_lock);

    cout << "Thread " << this_thread::get_id() << " is trying to acquire the lock..." << endl;

    // This will try to acquire the lock for 500 milliseconds, if it fails to acquire the lock within that time, it will return false and we can handle that case accordingly. In this example, we will just skip the increment if we fail to acquire the lock.

    if (!lock.try_lock_for(chrono::milliseconds(500))) {
      cout << "Failed to acquire lock, skipping increment" << endl;
      continue; // Does not need manually unlock the mutex since we never acquired it, we can just skip the increment and move on to the next iteration.
    }

    int stime = rand() % 1000 + 1;
    // Wait for a random amoutn of ms from 1-1000
    // Over 500 will cause the other threads mutex to time out, fail to aquire lock, and skip increment
    this_thread::sleep_for(chrono::milliseconds(stime));
    cout << "Random sleep time: " << stime << " ms" << endl;

    count++;
    // We don't need to manually unlock the mutex, the unique_lock will automatically release the lock when it goes out of scope. However, we can manually unlock it if we want to.
    lock.unlock();
  }
};

int main() {
  int count = 0;

  // For this contrived example we will use a timed_mutex
  timed_mutex mtx;

  thread t1(work, ref(count), ref(mtx));
  thread t2(work, ref(count), ref(mtx));

  t1.join();
  t2.join();

  cout << "Count: " << count << endl;

  return 0;
}

// If all increments succeed, we would expect the final count to be 10 (5 increments from each thread). However, due to the random sleep times and the 500 ms timeout for acquiring the lock, some increments may be skipped if a thread fails to acquire the lock within the timeout period. This can lead to a final count that is less than 10, and it may vary between runs of the program. An example output might look like this:

// Thread 0x7000035b7000 is trying to acquire the lock...
// Thread 0x70000363a000 is trying to acquire the lock...
// Failed to acquire lock, skipping increment
// Thread 0x70000363a000 is trying to acquire the lock...
// Random sleep time: 808 ms
// Thread 0x7000035b7000 is trying to acquire the lock...
// Failed to acquire lock, skipping increment
// Thread 0x70000363a000 is trying to acquire the lock...
// Random sleep time: 250 ms
// Thread 0x7000035b7000 is trying to acquire the lock...
// Random sleep time: 74 ms
// Thread 0x7000035b7000 is trying to acquire the lock...
// Failed to acquire lock, skipping increment
// Thread 0x70000363a000 is trying to acquire the lock...
// Random sleep time: 659 ms
// Thread 0x7000035b7000 is trying to acquire the lock...
// Failed to acquire lock, skipping increment
// Thread 0x70000363a000 is trying to acquire the lock...
// Failed to acquire lock, skipping increment
// Random sleep time: 931 ms
// Count: 5
