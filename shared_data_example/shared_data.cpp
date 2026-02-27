#include <iostream>
#include <thread>

using namespace std;

int main() {
  int count = 0;
  const int ITERATIONS = 1E6;

  thread t1([&count](){
    for (int i = 0; i < ITERATIONS; i++) {
      ++count;
    }
  });

  thread t2([&count](){
    for (int i = 0; i < ITERATIONS; i++) {
      ++count;
    }
  });

  t1.join();
  t2.join();

  cout << "Count: " << count << endl;
}


/* When ITERATIONS is 1000 or 10000
 * count ends up as 2000 and 20000 respectively (contsistently).
 * When ITERATIONS is 1E6, count ends up aroudn 1.04E6
 *
 * Both threads increment the same `int count` without synchronization,
 * so you hit a classic data race. Each `++count` compiles to roughly:
 * “load count → add 1 → store count.” When two cores run those three steps
 * at the same time, one thread can overwrite the other’s increment before it’s stored,
 *  losing an update.
 *
 * With small `ITERATIONS`, the race happens rarely so you often see the
 * correct 2× result; at 1e6 iterations it occurs frequently,
 * so the final value hovers just above 1,000,000.
 *
 *
 *
 * Assembly Behavior**
 - The `++count` inside each lambda (`calc_pi.cpp:10` and `calc_pi.cpp:16`) is compiled into three separate instructions: load `count` from memory into a register, increment the register, and store it back. For example, Clang/GCC on x86-64 typically emit something like:
   `mov eax, DWORD PTR [count]`
   `add eax, 1`
   `mov DWORD PTR [count], eax`
   No `lock` prefix or atomic instruction is used because `count` is an ordinary `int`.
 - Each thread executes that read-modify-write sequence independently. Between any two of those instructions, the hardware scheduler can let the other thread run, so there’s no guarantee that the load observed the most recent store from the other core.

----KEY BIT----
 ***CPU caches keep private copies of the cache line that holds `count`, and coherence only synchronizes them when someone performs a write, so overlapping sequences will keep invalidating each other.***

 **Interleaving Timeline**
 - Thread A loads `count` (say 1234567) into `eax`.
 - Before A stores, Thread B also loads the same 1234567 into `ebx`.
 - A adds 1 and stores 1234568.
 - B adds 1 and stores 1234568 as well, overwriting A’s update. One increment vanishes.
 - This race can occur thousands of times when `ITERATIONS` is 1e6, so the observed total hovers just above 1,000,000 instead of 2,000,000.
 */
