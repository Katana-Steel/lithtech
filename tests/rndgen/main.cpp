#include "lith_random.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

using katana_steel::lithtech::getRandom;

void testRandomFloat(float in_min=0.0f, float in_max=1.0f)
{
  if (in_min > in_max)
  {
    std::swap(in_min, in_max);
  }
  std::vector<float> rndList;
  rndList.reserve(500);
  for (size_t i = 0; i < rndList.capacity(); i++)
    rndList.push_back(getRandom(in_min, in_max));

  auto [minimum, maximum] = std::minmax_element(rndList.begin(), rndList.end());
  auto avg =
      std::accumulate(rndList.begin(), rndList.end(), 0.0f) / rndList.size();
  auto mid = (in_max+in_min)/2.0f;
  std::cout << float(*minimum) << ' ' << avg << ' ' << float(*maximum) << '\n';
  if (*minimum >= in_min && *maximum <= in_max && *minimum < *maximum)
    if (avg < (mid+0.1f) && avg > (mid-0.1f))
      return;
  throw "rnd float outside params";
}

void testRangeReversed(float in_min=0.0f, float in_max=1.0f)
{
  if (in_max > in_min)
    std::swap(in_max, in_min);
  std::vector<float> rndList;
  rndList.reserve(500);
  for (size_t i = 0; i < rndList.capacity(); i++)
    rndList.push_back(getRandom(in_min, in_max));

  auto [minimum, maximum] = std::minmax_element(rndList.begin(), rndList.end());
  auto avg =
      std::accumulate(rndList.begin(), rndList.end(), 0.0f) / rndList.size();
  std::cout << float(*minimum) << ' ' << avg << ' ' << float(*maximum) << '\n';
  
  auto mid = (in_max+in_min)/2.0f;
  std::swap(in_max, in_min);
  if (*minimum >= in_min && *maximum <= in_max && *minimum < *maximum)
    if (avg < (mid+0.1f) && avg > (mid-0.1f))
      return;
  throw "rnd float outside params";
}

int main() {
  katana_steel::lithtech::seed(1337);
  std::cout << "seed: 1337, ";
  testRandomFloat();
  katana_steel::lithtech::seed(1337);
  std::cout << "seed: 1337, ";
  testRandomFloat(0.0f, -1.0f);
  katana_steel::lithtech::seed(1337);
  std::cout << "seed: 1337, ";
  testRangeReversed();
  return EXIT_SUCCESS;
}