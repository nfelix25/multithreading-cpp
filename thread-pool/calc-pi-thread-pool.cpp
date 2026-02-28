#include <chrono>
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <iomanip>
#include <queue>

const int NUM_CORES = std::thread::hardware_concurrency();
const long long terms = 1E9;

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
  auto start = std::chrono::steady_clock::now();

  std::cout << "Calculating " << terms << " digits of pi using " << NUM_CORES << " threads..." << std::endl;

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

  auto end = std::chrono::steady_clock::now();
  std::cout << "Total time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0  << " seconds" << std::endl;

  std::cout << "True pi: " << std::setprecision(15) << M_PI << std::endl;
  std::cout << std::fixed << std::setprecision(15) << "Calc pi: " << pi << std::endl;
}

// Hyperthreading with 8 real cores

// Calculating 1000000000 digits of pi using 16 threads...
// Total time taken: 0.462 seconds

// 2x cores
// ~1.37x speed

// Calculating 1000000000 digits of pi using 8 threads...
// Total time taken: 0.63 seconds

// 2x cores
// ~1.85x speed

// Calculating 1000000000 digits of pi using 4 threads...
// Total time taken: 1.159 seconds

// 2x cores
// ~2.04x speed

// Calculating 1000000000 digits of pi using 2 threads...
// Total time taken: 2.371 seconds

// 2x cores
// ~1.88x speed

// Calculating 1000000000 digits of pi using 1 threads...
// Total time taken: 4.449 seconds

/* Hyperthreading logical cores each have their own own registers and state,
 * but the two logical threads on a single core share the L1 cache, L2 cache, and execution units (ALUs/FPUs).
 * That is why we see a roughly linear performance increase from 1 -> 2 -> 4 -> 8 cores, but much less increase from 8 -> 16.
 *
 * Hyper-threading is most effective when one thread is waiting for data,
 * allowing the other thread to use the idle execution resources, which is not the case in this compute-bound example.
 */
