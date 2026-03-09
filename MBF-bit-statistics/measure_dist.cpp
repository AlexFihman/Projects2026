#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include "ShortList.h"
#include "MonotoneBooleanFunction.h"

int main()
{
    std::random_device rd;
    std::mt19937 rng(rd());
    const int dim   = 10;
    const int nbits = 1 << dim;
    const int mid   = dim / 2;  // level 5
    const uint64_t steps = 10000000ULL;

    // Collect mid-level bit indices
    std::vector<int> mid_bits;
    for (int i = 0; i < nbits; i++)
        if (__builtin_popcount(i) == mid)
            mid_bits.push_back(i);
    int M = mid_bits.size();  // C(10,5) = 252

    MonotoneBooleanFunction mbf(dim, rng);
    for (int i = 0; i < 100000; i++)
        mbf.metropolisStep();

    std::vector<uint64_t> hist(M + 1, 0);

    for (uint64_t s = 0; s < steps; s++) {
        mbf.metropolisStep();
        int count = 0;
        for (int i : mid_bits)
            count += mbf.getFunctionValue(i);
        hist[count]++;
    }

    std::cout << "Distribution of hw=" << mid << " count (out of " << M << "):\n";
    std::cout << std::setw(8) << "count" << std::setw(12) << "freq" << std::setw(10) << "%\n";
    for (int k = 0; k <= M; k++) {
        if (hist[k] == 0) continue;
        std::cout << std::setw(8) << k
                  << std::setw(12) << hist[k]
                  << std::setw(10) << std::fixed << std::setprecision(4)
                  << 100.0 * hist[k] / steps << "%\n";
    }

    return 0;
}
