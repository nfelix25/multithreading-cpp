#include <iostream>
#include <iomanip>
#include <thread>
#include <future>

using namespace std;

double calculate_pi(long long terms) {
  double sum = 0.0;

  int sign = (terms % 2 == 0) ? 1 : -1;
  for (size_t i = 0; i < terms; i++) {
    sum += sign * 1.0 / (i * 2 + 1);
    sign = -sign;
  }
  return sum * 4;
};

int main() {
  const time_t start_time = time(nullptr);
  cout << "Calculating pi using the Leibniz formula..." << endl;
  cout << "Start time: " << ctime(&start_time) << endl;

  // A promise can be used to store a value that will be set by one thread and retrieved by another thread. In this example, we will use a promise to store the calculated value of pi and retrieve it in the main thread after the calculation is done. The promise is created in the main thread and passed to the worker thread that calculates pi. The worker thread sets the value of pi in the promise, and the main thread retrieves it using a future.
  promise<double> promise;

  auto do_calculation = [&](int terms){
    cout << "Calculating pi in thread " << this_thread::get_id() << "..." << endl;

    auto result = calculate_pi(terms);

    // future.get() will block until the value is set in the promise, so we need to set the value in the promise before we can retrieve it in the main thread.
    promise.set_value(result);
  };

  thread t1(do_calculation, 1E8);

  future<double> future = promise.get_future();

  t1.join();

  const time_t end_time = time(nullptr);
  cout << "End time: " << ctime(&end_time) << endl;
  cout << "Total time taken: " << difftime(end_time, start_time) << " seconds" << endl;

  cout << setprecision(15) << "Calculated pi: " << future.get() << endl;

  return 0;
}
