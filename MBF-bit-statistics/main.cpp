#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <thread>
#include <cmath>
#include "ShortList.h"
#include "MonotoneBooleanFunction.h"

// Generate a random MBF by:
//   - setting hw < lo_level to 0, hw > hi_level to 1
//   - randomising hw in [lo_level, hi_level]
//   - fixing any monotonicity violations by repeatedly picking a random
//     outlier and setting it to the correcting value (always terminates)
static void randomBoundaryInit(MonotoneBooleanFunction& mbf, int dim,
                               int lo_level, int hi_level, std::mt19937& rng)
{
    const int nbits = 1 << dim;
    std::uniform_int_distribution<int> coin(0, 1);
    std::vector<bool> values(nbits);

    for (int i = 0; i < nbits; i++) {
        int hw = __builtin_popcount(i);
        if      (hw < lo_level) values[i] = false;
        else if (hw > hi_level) values[i] = true;
        else                    values[i] = coin(rng);
    }

    // Fix violations between adjacent levels lo_level..hi_level+1.
    // A violation is an edge (i→j) with hw(j)=hw(i)+1 where f(i)=1, f(j)=0.
    // Flipping the outlier to the correcting value removes all its violations
    // without creating new ones, so this loop always terminates.
    while (true) {
        std::vector<int> outliers;
        std::vector<bool> in_outliers(nbits, false);

        for (int i = 0; i < nbits; i++) {
            if (!values[i]) continue;
            int hw = __builtin_popcount(i);
            if (hw < lo_level || hw >= hi_level) continue;
            for (int k = 0; k < dim; k++) {
                if (i & (1 << k)) continue;      // k is already set
                int j = i | (1 << k);             // j covers i, hw(j)=hw+1
                if (!values[j]) {
                    if (!in_outliers[i]) { outliers.push_back(i); in_outliers[i] = true; }
                    if (!in_outliers[j]) { outliers.push_back(j); in_outliers[j] = true; }
                }
            }
        }
        if (outliers.empty()) break;

        int idx = outliers[rng() % outliers.size()];
        // level-low outlier (f=1, successor=0): set to 0
        // level-high outlier (f=0, predecessor=1): set to 1
        values[idx] = (__builtin_popcount(idx) > lo_level);
    }

    mbf.initFromValues(values);
}

static void run_chain(int dim, uint64_t steps, uint64_t seed, std::vector<uint64_t>& counts)
{
    std::mt19937 rng(seed);
    const int nbits = 1 << dim;

    MonotoneBooleanFunction mbf(dim, rng);
    randomBoundaryInit(mbf, dim, 5, 5, rng);
    // Short burn-in to let boundary drift into levels 3 and 6
    // (init sets hw<4=0 and hw>5=1, but p3≈0.003 so some steps needed)
    for (int i = 0; i < 10000; i++)
        mbf.metropolisStep();

    for (uint64_t s = 0; s < steps; s++) {
        mbf.metropolisStep();
        for (int i = 0; i < nbits; i++)
            if (mbf.getFunctionValue(i))
                counts[i]++;
    }
}

int main()
{
    const int dim      = 10;
    const int nbits    = 1 << dim;  // 512
    const int nthreads = std::thread::hardware_concurrency();
    const uint64_t steps_per_thread = 100000000ULL;
    const uint64_t total_steps = steps_per_thread * nthreads;

    std::cout << "Running " << nthreads << " chains x "
              << steps_per_thread << " steps = " << total_steps << " total\n\n";

    // Each thread gets its own counts vector; merge at the end.
    std::vector<std::vector<uint64_t>> thread_counts(nthreads, std::vector<uint64_t>(nbits, 0));
    std::vector<std::thread> threads;

    std::random_device rd;
    for (int t = 0; t < nthreads; t++)
        threads.emplace_back(run_chain, dim, steps_per_thread,
                             (uint64_t)rd() << 32 | rd(), std::ref(thread_counts[t]));
    for (auto& th : threads) th.join();

    // Merge counts
    std::vector<uint64_t> counts(nbits, 0);
    for (int t = 0; t < nthreads; t++)
        for (int i = 0; i < nbits; i++)
            counts[i] += thread_counts[t][i];

    // Pool by level and exploit complement symmetry (p_k + p_{dim-k} = 1)
    std::vector<uint64_t> level_counts(dim + 1, 0);
    std::vector<int>      level_size  (dim + 1, 0);
    for (int i = 0; i < nbits; i++) {
        int hw = __builtin_popcount(i);
        level_counts[hw] += counts[i];
        level_size[hw]++;
    }

    std::cout << std::setw(8)  << "hw pair"
              << std::setw(8)  << "C(9,hw)"
              << std::setw(16) << "p(hw=k)"
              << std::setw(16) << "2p(1-p)"
              << std::setw(16) << "contribution"
              << "\n";

    double est_hamming = 0.0;
    for (int hw = 0; hw <= dim / 2; hw++) {
        int hw2 = dim - hw;
        int n = level_size[hw];
        double pooled_p = (double)(level_counts[hw] + (uint64_t)n * total_steps - level_counts[hw2])
                          / (2.0 * n * total_steps);
        double contrib_per_bit = 2.0 * pooled_p * (1.0 - pooled_p);
        double contribution = contrib_per_bit * n * (hw == hw2 ? 1 : 2);
        est_hamming += contribution;
        std::cout << std::setw(5)  << hw << "+" << std::setw(1) << hw2
                  << std::setw(8)  << n
                  << std::setw(16) << std::scientific << std::setprecision(6) << pooled_p
                  << std::setw(16) << std::scientific << std::setprecision(6) << contrib_per_bit
                  << std::setw(16) << std::fixed      << std::setprecision(6) << contribution
                  << "\n";
    }

    std::cout << "\nEstimated avg Hamming distance: "
              << std::fixed << std::setprecision(6) << est_hamming << "\n";

    return 0;
}
