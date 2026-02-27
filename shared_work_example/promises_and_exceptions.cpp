#include <iostream>
#include <iomanip>
#include <thread>
#include <future>
#include <exception>

using namespace std;

double calculate_pi(long long terms) {
  double sum = 0.0;

  if (terms < 100) {
    throw runtime_error("Number of terms cannot be less than 100, for... Reasons... It makes sense, okay??? STOP HITTING YOURSELF!!!");
  }

  int sign = (terms % 2 == 0) ? 1 : -1;
  for (size_t i = 0; i < terms; i++) {
    sum += sign * 1.0 / (i * 2 + 1);
    sign = -sign;
  }
  return sum * 4;
};

int main() {
  promise<double> promise;

  auto do_calc = [&promise](int terms){
    try {
      auto result = calculate_pi(terms);
      promise.set_value(result);
    } catch (/* const exception &e */ ...) { // "..." is a catch-all handler, it will catch any exception, regardless of type. This is useful when you don't care about the specific type of exception, or when you want to handle all exceptions in the same way.

      // set_excepton expects an exception_ptr, which can be obtained by calling current_exception() in a catch block
      promise.set_exception(current_exception());
    }
  };

  thread t1(do_calc, 1E6);

  future<double> future = promise.get_future();
  double pi;

  try {
    pi = future.get();
  } catch (exception &e) {
    cout << "Error calculating pi: " << e.what() << endl;
    exit(1);
  }

  t1.join();

  cout << setprecision(15) << "Calculated pi: " << pi << endl;
}
