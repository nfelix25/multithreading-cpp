#include <iostream>
#include <iomanip>
#include <thread>
#include <future>
#include <exception>

// The packaged_task will automatically handle the promise and future objects, so we can get the future directly from the task
// And not worry about the promise object, as the task will handle it internally.
double calculate_pi(long long terms) {
  double sum = 0.0;

  // packaged_task will also handle exceptions thrown in the task.
  if (terms <= 0) {
    throw std::invalid_argument("Number of terms must be positive.");
  }

  int sign = (terms % 2 == 0) ? 1 : -1;
  for (size_t i = 0; i < terms; i++) {
    sum += sign * 1.0 / (i * 2 + 1);
    sign = -sign;
  }

  return sum * 4;
};

int main() {
  // Packaged tasks allow us to wrap a function and its arguments into a single object that can be executed asynchronously.
  // Takes type of the function sig as <return_type(param, types)>
  std::packaged_task<double(long long)> task1(calculate_pi);

  // Task automatically handles the promise and future objects, so we can get the future directly from the task.
  // And we do not need to manually interface with the promise object, as the task will handle it internally.
  std::future<double> future1 = task1.get_future();

  std::thread t1(std::move(task1), 1E9);

  try
  {
    double result = future1.get();
    std::cout << std::setprecision(15) << "Calculated pi: " << result << std::endl;
  }
  catch (std::exception &e)
  {
    std::cout << "Error calculating pi: " << e.what() << std::endl;
  }

  t1.join();

  return 0;
}
