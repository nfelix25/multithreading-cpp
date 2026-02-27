#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

void work(int &count, mutex &mtx) {
  // Manual lock and unlock doesn't handle cases where an error may occur or other unexpected flows
  // We should exercise RAII (Resource Acquisition Is Initialization) which is a C++ idiom that ties resource management to object lifetime. `std::lock_guard` is a simple wrapper that provides a convenient RAII-style mechanism for owning a mutex for the duration of a scoped block. When a `std::lock_guard` object is created, it attempts to take ownership of the mutex it is given. When control leaves the scope in which the `std::lock_guard` object was created, for any reason, the destructor is automatically called, and the mutex is released.
  for (int i = 0; i < 1E6; i++) {
    lock_guard<mutex /* Type, such as timed_mutex etc...)*/> guard(mtx);
    // mtx.lock();
    count++;
    // mtx.unlock();
  }
}

int main() {
  int count = 0;

  mutex mtx;

  thread t1(work, ref(count), ref(mtx));
  thread t2(work, ref(count), ref(mtx));

  t1.join();
  t2.join();

  cout << "Count: " << count << endl;

  return 0;
}
