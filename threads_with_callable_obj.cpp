#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

class App {

  private:
    int count = 0;
    mutex mtx;

  public:
  // The operator() is a special member function that allows an object to be called as if it were a function. This is useful for creating callable objects, which can be used in places where a function pointer or lambda would be expected.
    void operator()() {
      for (int i = 0; i < 1E6; i++) {
        const lock_guard<mutex> guard(mtx);
        count++;
      }
    }

    int getCount() {
      int count = -1;
      cout << "Local count in app.getCount: " << count << endl;
      // Declaring local count to show how this-> access can be used when shadowing variables. In this case, the local count variable shadows the member variable count, so we need to use this->count to access the member variable.
      return this->count;
    }
};

int main() {
  App app;
  // app is a callable object, so we can pass it directly to the thread constructor without needing to specify a function pointer or lambda. The thread will call the operator() function of the App class when it starts executing.

  // mutex does not have a copy constructor, so therefor neither does App since it contains a mutex. This means that we cannot pass app by value to the thread constructor, as it would require copying the App object. Instead, we need to pass it by reference using std::ref, which allows us to pass a reference to the App object without copying it.
  thread t1(ref(app));
  thread t2(ref(app));

  t1.join();
  t2.join();

  int app_count = app.getCount();

  cout << "Count from app member: " << app_count << endl;

  return 0;
}
