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
  cout_mtx.lock();
  std::cout << "Worker " << id << " is working..." << std::endl;
  cout_mtx.unlock();

  // Simulate async work by sleeping for a random amount of time
  std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 2000));

  return id;
}

int main() {
  const int NUM_THREADS = 10;
  BlockingQueue<std::shared_future<int>> futures(4);

  // Run enqueue process in its own thread so that it doesn't block the main thread and allow the futures to run concurrently. This will allow us to see the output for all workers in a non-deterministic order, as they will be running concurrently.
  std::thread t1([&](){
    for (int i = 1; i <= NUM_THREADS; i++) {
      std::shared_future<int> f = std::async(std::launch::async, work, i);
      futures.enqueue(f);
    }
  });

  for (int i = 0; i < NUM_THREADS; i++) {
    std::shared_future<int> f = futures.dequeue();
    int value = f.get();

    std::lock_guard<std::mutex> guard(cout_mtx);
    std::cout << "Returned: " << value << std::endl;
  }

  t1.join();

  return 0;
}

// --- Example for trace ----------------

// BlockingQueue created with max size: 4

// Queue is empty. Consumer is waiting...

// Enqueuing element - Current size: 0
// Worker 1 is working...
// Dequeuing element - Current size: 1
// Worker 2 is working...
// Enqueuing element - Current size: 0
// Enqueuing element - Current size: 1
// Worker 3 is working...
// Enqueuing element - Current size: 2
// Worker 4 is working...
// Worker 5 is working...
// Enqueuing element - Current size: 3

// Queue is full. Producer is waiting...

// Worker 6 is working...
// Returned: 1
// Dequeuing element - Current size: 4
// Enqueuing element - Current size: 3

// Queue is full. Producer is waiting...

// Worker 7 is working...
// Returned: 2
// Dequeuing element - Current size: 4
// Returned: 3
// Dequeuing element - Current size: 3
// Enqueuing element - Current size: 2
// Enqueuing element - Current size: 3

// Queue is full. Producer is waiting...

// Worker 8 is working...
// Worker 9 is working...
// Returned: 4
// Dequeuing element - Current size: 4
// Returned: 5
// Dequeuing element - Current size: 3
// Returned: 6
// Enqueuing element - Current size: 2
// Dequeuing element - Current size: 3
// Enqueuing element - Current size: 2
// Worker 10 is working...
// Returned: 7
// Dequeuing element - Current size: 3
// Returned: 8
// Dequeuing element - Current size: 2
// Returned: 9
// Dequeuing element - Current size: 1
// Returned: 10
