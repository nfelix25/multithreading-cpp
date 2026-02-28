#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>

using std::cout;
using std::endl;

template<typename ElementType>
class BlockingQueue {
  private:
    const int _MAX_SIZE; // Leading underscore indicates this is a private member variable, and all caps indicates that it is a constant.
    std::queue<ElementType> _queue;
    std::mutex _mtx;
    std::condition_variable _cv;

  public:
    BlockingQueue(int size) : _MAX_SIZE(size) /* Initializer list allows us to set member vars before constructor */ {
      cout << "BlockingQueue created with max size: " << _MAX_SIZE << endl << endl;
    }

    void enqueue(ElementType element) {
      std::unique_lock<std::mutex> lock(_mtx);
      _cv.wait(lock, [this](){
        if (_queue.size() >= _MAX_SIZE) {
          cout << "\nQueue is full. Producer is waiting..." << endl << endl;
        }
        return _queue.size() < _MAX_SIZE; // Wait until there is space in the queue
      });

      cout << "Enqueuing element: " << element << endl;
      _queue.push(element);
      lock.unlock(); // Unlock before notifying to allow the waiting thread to proceed without being blocked by the mutex.
      _cv.notify_one(); // Notify one waiting thread that an element has been enqueued

      // Simulate async delay for random amount of time
      std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 500));
    }

    ElementType dequeue() {
      ElementType ret;

      std::unique_lock<std::mutex> lock(_mtx);
      _cv.wait(lock, [this](){
        if (_queue.empty()) {
          cout << "\nQueue is empty. Consumer is waiting..." << endl << endl;
        }
        return !_queue.empty(); // Wait until there is an element in the queue
      });

      cout << "Dequeuing element: " << _queue.front() << endl;
      ret = _queue.front();
      _queue.pop();
      lock.unlock(); // Unlock before notifying to allow the waiting thread to proceed without being blocked by the mutex.
      _cv.notify_one(); // Notify one waiting thread that an element has been dequeued

      // Simulate async delay for random amount of time
      std::this_thread::sleep_for(std::chrono::milliseconds(100 + rand() % 1000));

      return ret;
    }
};

int main() {
  constexpr int queue_size = 5; // constexpr must be initialized with a compile-time constant, so we cannot use a variable here.
  BlockingQueue<int> bq(queue_size);
  constexpr int num_elements = queue_size * 3; // Enqueue more elements than the queue can hold to demonstrate blocking behavior.

  // Using c++ array avoids having to pass in ref(elements), gives us size, and allows using for each loop in the producer thread. We could also use a vector, but then we would still need to pass in ref(elements) to avoid copying the vector into the thread, and we would need to use elements.size() instead of sizeof to get the number of elements.
  std::array<int, num_elements> elements;

  for (int i = 0; i < num_elements; i++) {
    elements[i] = i + 1;
  }

  std::thread producer([&bq](std::array<int, num_elements> elements){
    int num_elements = sizeof(elements) / sizeof(elements[0]);
    for (int x : elements) {
      bq.enqueue(x);
    }
  }, elements /* Does not need to be passed in since declared out and could be & in [], but doign so to keep with what format would be with C style array */);

  std::array<int, num_elements> consumed_elements;

  std::thread consumer([&bq, &consumed_elements](){
    for (int i = 0; i < consumed_elements.size(); i++) {
      consumed_elements[i] = bq.dequeue();
    }
  });

  producer.join();
  consumer.join();

  cout << "\nConsumed elements: ";
  for (int x : consumed_elements) {
    cout << x << " ";
  }
  cout << endl;

  return 0;
}


// BlockingQueue created with max size: 5

// Enqueuing element: 1
// Dequeuing element: 1

// Queue is empty. Consumer is waiting...

// Enqueuing element: 2
// Dequeuing element: 2
// Enqueuing element: 3
// Enqueuing element: 4
// Dequeuing element: 3
// Enqueuing element: 5
// Dequeuing element: 4
// Enqueuing element: 6
// Enqueuing element: 7
// Enqueuing element: 8
// Dequeuing element: 5
// Enqueuing element: 9
// Enqueuing element: 10
// Dequeuing element: 6
// Enqueuing element: 11
// Dequeuing element: 7
// Enqueuing element: 12

// Queue is full. Producer is waiting...

// Dequeuing element: 8
// Enqueuing element: 13

// Queue is full. Producer is waiting...

// Dequeuing element: 9
// Enqueuing element: 14
// Dequeuing element: 10
// Enqueuing element: 15
// Dequeuing element: 11
// Dequeuing element: 12
// Dequeuing element: 13
// Dequeuing element: 14
// Dequeuing element: 15

// Consumed elements: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
