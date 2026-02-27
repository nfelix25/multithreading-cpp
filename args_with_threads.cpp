#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

void work(int &count, mutex &mtx) {
  for (int i = 0; i < 1E6; i++) {
    mtx.lock();
    count++;
    mtx.unlock();
  }
}

int main(int argc, char* argv[]) {
  int count = 0;

  mutex mtx;

  // To pass args into the thread function we can use variadic args and `std::ref` to pass by reference.
  thread t1(work, ref(count), ref(mtx));
  thread t2(work, ref(count), ref(mtx));

  t1.join();
  t2.join();

  cout << "Count: " << count << endl;

  return 0;
}
