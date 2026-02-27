#include <iostream>
#include <iomanip>
#include <thread>
#include <future>
#include <functional>

using namespace std;

const int NUM_CORES = thread::hardware_concurrency() / 2;

double calculate_pi(long long terms_start, long long terms_end) {
  double sum = 0.0;

  int sign = (terms_start % 2 == 0) ? 1 : -1;

  for (size_t i = terms_start; i < terms_end; i++) {
    sum += sign * 1.0 / (i * 2 + 1);
    sign = -sign;
  }

  return sum * 4;
};

int main() {
  const time_t start_time = time(nullptr);
  cout << "Calculating pi using the Leibniz formula..." << endl;
  cout << "Start time: " << ctime(&start_time) << endl;
  mutex mtx;

  auto do_calculation = [&](long long terms_start, long long terms_end, promise<double> &promise){
    mtx.lock();
    cout << "Calculating terms " << terms_start << " to " << terms_end << " in thread " << this_thread::get_id() << "..." << endl;
    mtx.unlock();

    auto partial_result = calculate_pi(terms_start, terms_end);

    promise.set_value(partial_result);
  };

  thread threads[NUM_CORES];
  promise<double> promises[NUM_CORES];
  future<double> futures[NUM_CORES];

  for (long long i = 0; i < NUM_CORES; i++) {
    int threads_per_core = 1E10 / NUM_CORES;
    long long terms_start = i * threads_per_core;
    long long terms_end = (i + 1) * threads_per_core;
    threads[i] = thread(do_calculation, terms_start, terms_end, std::ref(promises[i]));
  }

  for (int i = 0; i < NUM_CORES; i++) {
    futures[i] = promises[i].get_future();
  }

  double pi = 0.0;

  for (int i = 0; i < NUM_CORES; i++) {
    pi += futures[i].get();
    threads[i].join();
  }

  const time_t end_time = time(nullptr);
  cout << "End time: " << ctime(&end_time) << endl;
  cout << "Total time taken: " << difftime(end_time, start_time) << " seconds" << endl;

  cout << setprecision(15) << "Calculated pi: " << pi << endl;

  return 0;
}

// 1E9 @ single thread
// Total time taken: 6 seconds
// Calculated pi: 3.14159265258805
//
// 1E10 @ 16 thread (1/core)
// Total time taken: 4 seconds
// Calculated pi: 3.14159265348899
//
// 1E10 @ 8 thread < 1/core
// Total time taken: 11 seconds
// Calculated pi: 3.14159265348821
