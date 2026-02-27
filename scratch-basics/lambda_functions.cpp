#include <iostream>
#include <thread>

using namespace std;

int main() {
  int num = 76;
  char str[256];
  stringstream stream;

  thread t1([num, &str](int num2, char* str2){
    cout << "Thread id: " << this_thread::get_id() << endl;
    cout << "sum: " << num + num2 << endl;

    snprintf(str, sizeof(str), "Hello from thread %d", num2);
  }, 42, str);

  t1.join();

  cout << "Main thread id: " << this_thread::get_id() << endl;
  cout << "Value of str after thread creation: " << str << endl;

  return 0;
}
