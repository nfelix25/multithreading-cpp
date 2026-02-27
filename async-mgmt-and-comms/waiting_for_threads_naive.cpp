#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

int main() {
  std::cout << "Main thread is starting a worker thread..." << std::endl;

  // Creating a scenario where the main thread needs to wait for the worker thread to complete.

  // Using atomic for maximum portability and to avoid issues with memory visibility across threads.
  std::atomic<bool> ready(false);

  // The opposite of the above would be:
  // thread_local bool ready = false; // Would hang indefinitely
  // This would ensure that each thread gets its own copy of "ready", regardless of the lambda signature below.

  std::thread t1([&ready](){
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // Simulate work
    ready = true;
  });

  // Assumign we needed to do work before joining the thread. Otherwise the join would wait for the thread to finish, so we would not need to do any additional waiting.

  // Naive wait using polling
  while(!ready) {
    // This is inefficient as it consumes CPU cycles while waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Avoid busy waiting
  }

  std::cout << "Worker thread has completed. Ready: " << std::boolalpha << ready << std::endl;

  t1.join();
}
