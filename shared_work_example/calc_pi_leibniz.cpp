#include <iostream>
// #include <cmath>
#include <iomanip>
// #include <thread>

using namespace std;

double calculate_pi(long long terms) {
  double sum = 0.0;

  int sign = (terms % 2 == 0) ? 1 : -1;
  for (size_t i = 0; i < terms; i++) {
    sum += sign * 1.0 / (i * 2 + 1);
    sign = -sign;
  }
  return sum * 4;
};

int main() {
  const time_t start_time = time(nullptr);
  cout << "Calculating pi using the Leibniz formula..." << endl;
  cout << "Start time: " << ctime(&start_time) << endl;

  double pi = calculate_pi(1E9);

  const time_t end_time = time(nullptr);
  cout << "End time: " << ctime(&end_time) << endl;
  cout << "Total time taken: " << difftime(end_time, start_time) << " seconds" << endl;

  cout << setprecision(15) << "Calculated pi: " << pi << endl;

  return 0;
}
