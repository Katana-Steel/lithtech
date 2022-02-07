#ifndef LITH_RANDOM_LIB
#define LITH_RANDOM_LIB

namespace katana_steel::lithtech {
    float getRandom(float min=0.0f, float max=1.0f) noexcept;
    int getRandom(int min, int max) noexcept;
    void seed(int s=0) noexcept;
}

#endif