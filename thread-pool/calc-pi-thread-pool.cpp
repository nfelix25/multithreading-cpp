#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <iomanip>
#include <queue>

const int NUM_CORES = std::thread::hardware_concurrency();

// Completely pointtless overhead as NUM_BORES is the limiter, but added for practice
template<typename E>
class BlockingQueue {
  private:
    const int _SIZE = NUM_CORES;
    std::mutex _mtx;
    std::condition_variable _cv;
    std::queue<E> _q;

  public:
    void push(E el) {
      std::unique_lock<std::mutex> lock(_mtx);

      _cv.wait(lock, [this](){
        return _q.size() < _SIZE;
      });

      // Future is not copyable
      _q.push(std::move(el));
      lock.unlock();
      _cv.notify_one();
    }

    E pop() {
      std::unique_lock<std::mutex> lock(_mtx);

      _cv.wait(lock, [this](){
        return _q.size() > 0;
      });

      // Future is not copyable
      E ret = std::move(_q.front());
      _q.pop();
      lock.unlock();
      _cv.notify_one();

      return ret;
    }
};

double calculate_pi(long long terms, int start, int skip) {
  double sum = 0.0;

  int sign = (start % 2 == 0) ? 1 : -1;

  for (long long i = start; i < terms; i += skip) {
    double term = 1.0 / (i * 2 + 1);
    sum += sign * term;

    if (skip % 2) {
      sign = -sign;
    }
  }

  return sum * 4;
}


int main() {
  BlockingQueue<std::future<double>> results;

  double pi = 0.0;
  long long terms = 1E10;
  const time_t start_time = time(nullptr);

  std::cout << "Calculating " << terms << " digits of pi using " << NUM_CORES << " threads..." << std::endl;
  std::cout << "Start time: " << ctime(&start_time) << std::endl;

  std::thread producer([&](){
    for (int i = 0; i < NUM_CORES; i++) {
      results.push(std::async(calculate_pi, terms, i, NUM_CORES));
    }
  });

  std::thread consumer([&](){
    for (int i = 0; i < NUM_CORES; i++) {
      std::future<double> f = results.pop();
      pi += f.get();
    }
  });

  producer.join();
  consumer.join();

  const time_t end_time = time(nullptr);
  std::cout << "End time: " << ctime(&end_time) << std::endl;
  std::cout << "Total time taken: " << difftime(end_time, start_time) << " seconds" << std::endl;

  std::cout << std::fixed << std::setprecision(15) << "Calculated value of pi: " << pi << std::endl;
}
