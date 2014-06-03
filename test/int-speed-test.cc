#include <cstdint>
#include <limits>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
using namespace std;
using namespace std::chrono;

class OInt {
public:

  static const int64_t udef=std::numeric_limits<int64_t>::min();
  int64_t value;
  OInt() {}
  OInt(int64_t value): value(value) {}
 
  friend OInt operator+(OInt a, OInt b) {
    int64_t v = a.value + b.value;
    if (a.value == udef) v=udef;
    if (b.value == udef) v=udef;
    return OInt(v);
  }
  friend std::ostream & operator<<(std::ostream & o, OInt v) {
    if (v.value == udef) return o << "?-Int";
    return o << v.value;
  }
};

double undef() {return std::numeric_limits<double>::quiet_NaN();}

template <typename IT>
void test(std::string name, IT undef) {
  const size_t value_count=500000;
  const size_t rep_count=2000;
  IT values[value_count];
  for (size_t i=0; i < value_count; ++i)
    values[i] = IT(i);
  

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine rng(seed);
  IT sum1(0);
  IT sum2(0);
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  {
    shuffle(begin(values), end(values), rng);
    for (size_t i = 0; i < rep_count; ++i) {
      for (IT x: values)
	sum1 = sum1 + x;
    }
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  {
    values[42] = undef;
    shuffle(begin(values), end(values), rng);
    for (size_t i = 0; i < rep_count; ++i)
      for (IT x: values)
	sum2 = sum2 + x;
  }
  high_resolution_clock::time_point t3 = high_resolution_clock::now();

  duration<double> d1 = duration_cast<duration<double>>(t2 - t1);
  duration<double> d2 = duration_cast<duration<double>>(t3 - t2);

  std::cout << name << " " << d1.count() << " " << d2.count() << " " << sum1 << " " << sum2 << std::endl;
}

int main() {
  test<double>("double", std::numeric_limits<double>::quiet_NaN());
  test<OInt>("cmove", OInt::udef);
}
    
