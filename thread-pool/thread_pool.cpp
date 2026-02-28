#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

// Create a utility mutex specifically for use with demo related cout, alternatively could and shoudl use osyncstream
std::mutex cout_mtx;

template<typename ElementType>
class BlockingQueue {
  private:
    const int _MAX_SIZE;
    std::queue<ElementType> _q;
    std::mutex _mtx;
    std::condition_variable _cv;

  public:
    BlockingQueue(int size): _MAX_SIZE(size) {
      std::lock_guard<std::mutex> guard(cout_mtx);
      std::cout << "BlockingQueue created with max size: " << _MAX_SIZE << std::endl;
    }

    void enqueue(ElementType el) {
      std::unique_lock<std::mutex> lock(_mtx);

      _cv.wait(lock, [this](){
        const int qsize = _q.size();

        if (qsize >= _MAX_SIZE) {
          std::lock_guard<std::mutex> guard(cout_mtx);
          std::cout << "\nQueue is full. Producer is waiting..." << std::endl << std::endl;
        }

        return qsize < _MAX_SIZE;
      });

      cout_mtx.lock();
      std::cout << "Enqueuing element - Current size: " << _q.size() << std::endl;
      cout_mtx.unlock();

      _q.push(el);
      lock.unlock();
      _cv.notify_one();
    }

    ElementType dequeue() {
      std::unique_lock<std::mutex> lock(_mtx);

      _cv.wait(lock, [this](){
        const bool is_empty = !_q.size();

        if (is_empty) {
          std::lock_guard<std::mutex> guard(cout_mtx);
          std::cout << "\nQueue is empty. Consumer is waiting..." << std::endl << std::endl;
        }

        return !is_empty;
      });

      cout_mtx.lock();
      std::cout << "Dequeuing element - Current size: " << _q.size() << std::endl;
      cout_mtx.unlock();

      ElementType ret = _q.front();
      _q.pop();
      lock.unlock();
      _cv.notify_one();

      return ret;
    }
};

int work(int id)
{
  int rand_time = 100 + rand() % 5000;

  cout_mtx.lock();
  std::cout << "Worker " << id << " is working for " << rand_time << "ms..." << std::endl;
  cout_mtx.unlock();

  // Simulate async work by sleeping for a random amount of time
  std::this_thread::sleep_for(std::chrono::milliseconds(rand_time));

  return id;
}

int main() {
  const int NUM_THREADS = 5;
  BlockingQueue<std::shared_future<int>> futures(2);

  // Run enqueue process in its own thread so that it doesn't block the main thread and allow the futures to run concurrently. This will allow us to see the output for all workers in a non-deterministic order, as they will be running concurrently.
  std::thread producer([&](){
    for (int i = 1; i <= NUM_THREADS; i++) {
      std::shared_future<int> f = std::async(std::launch::async, work, i);
      futures.enqueue(f);
    }
  });

  std::thread consumer([&](){
      for (int i = 0; i < NUM_THREADS; i++) {
        std::shared_future<int> f = futures.dequeue();
        int value = f.get();

        std::lock_guard<std::mutex> guard(cout_mtx);
        std::cout << "Returned: " << value << std::endl;
      }
  });

  producer.join();
  consumer.join();

  return 0;
}
