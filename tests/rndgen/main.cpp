#include "lith_random.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <vector>

using katana_steel::lithtech::getRandom;

void testRandomFloat() {
  std::vector<float> rndList;
  rndList.reserve(500);
  for (size_t i = 0; i < rndList.capacity(); i++)
    rndList.push_back(getRandom());

  auto [minimum, maximum] = std::minmax_element(rndList.begin(), rndList.end());
  auto avg =
      std::accumulate(rndList.begin(), rndList.end(), 0.0f) / rndList.size();
  std::cout << float(*minimum) << ' ' << avg << ' ' << float(*maximum) << '\n';
  if (*minimum >= 0.0f && *maximum <= 1.0f && *minimum < *maximum)
    if (avg < 0.6 && avg > 0.4)
      return;
  throw "rnd float outside params";
}

int main() {
  katana_steel::lithtech::seed();
  testRandomFloat();
  
}