#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <memory>
#include <cmath>
#include "ShortList.h"
#include "MonotoneBooleanFunction.h"

int main()
{
    std::random_device rd;
    std::mt19937 rng(rd());
    const int dim = 9;
    const int N = 10001;        // ring size
    const int mix_rounds = 8000;
    const double target_std = 0.001;

    // Burn-in a single walk to a random starting point
    MonotoneBooleanFunction seed(dim, rng);
    for (int i = 0; i < 10000; i++)
        seed.flipRandom();

    // Generate the ring: func[i+1] is func[i] after one metropolis step.
    std::vector<std::unique_ptr<MonotoneBooleanFunction>> funcs;
    funcs.reserve(N);
    funcs.push_back(std::make_unique<MonotoneBooleanFunction>(seed, rng));
    for (int i = 1; i < N; i++) {
        funcs.push_back(std::make_unique<MonotoneBooleanFunction>(*funcs[i-1], rng));
        funcs[i]->metropolisStep();
    }

    Record r1, r2;

    // Mixing phase
    std::cout << "Mixing (" << mix_rounds << " rounds)...\n";
    for (int round = 0; round < mix_rounds; round++)
        for (int i = 0; i < N; i++)
            funcs[i]->metropolisStep();

    // Accumulation phase: running mean and variance of per-round averages.
    // Stop when estimated std dev of the mean drops below target_std.
    std::cout << "Accumulating...\n\n";
    std::cout << std::setw(8) << "round" << std::setw(14) << "running_mean"
              << std::setw(14) << "std_of_mean" << "\n";

    double sum = 0.0, sum_sq = 0.0;
    int t = 0;
    while (true) {
        for (int i = 0; i < N; i++)
            funcs[i]->metropolisStep();

        double total_dist = 0.0;
        for (int i = 0; i < N; i++) {
            funcs[i]->toRecord(r1);
            funcs[(i + 1) % N]->toRecord(r2);
            total_dist += MonotoneBooleanFunction::recordHammingDistance(r1, r2);
        }
        double sample = total_dist / N;
        t++;
        sum += sample;
        sum_sq += sample * sample;

        if (t % 500 == 0) {
            double mean = sum / t;
            double var_sample = (sum_sq / t) - mean * mean;
            double std_of_mean = std::sqrt(var_sample / t);
            std::cout << std::setw(8) << t
                      << std::setw(14) << std::fixed << std::setprecision(6) << mean
                      << std::setw(14) << std::scientific << std::setprecision(3) << std_of_mean
                      << "\n";
            if (std_of_mean < target_std)
                break;
        }
    }

    return 0;
}
