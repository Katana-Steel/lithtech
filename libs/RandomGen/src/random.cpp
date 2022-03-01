#include "lith_random.hpp"
#include <chrono>
#include <random>

namespace katana_steel::lithtech {

struct RandGen {
  std::mt19937_64 mt;
};

static RandGen r;

float getRandom(float min, float max) noexcept {
  try {
    if(min>max)
      std::swap(min, max);
    std::uniform_real_distribution<float> urd{min, max};
    return urd(r.mt);
  } catch(...) {
    return 0.0f;
  }
}

int getRandom(int min, int max) noexcept {
  try {
    if(min>max)
      std::swap(min, max);
    std::uniform_int_distribution<int> urd{min, max};
    return urd(r.mt);
  } catch(...) {
    return 0;
  }
}

void seed(int s) noexcept {
  if (s == 0) {
    std::chrono::system_clock clk;
    r.mt.seed(clk.to_time_t(clk.now()));
  } else {
    r.mt.seed(s);
  }
}

} // namespace katana_steel::lithtech