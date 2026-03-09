#include <iostream>
#include <cmath>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#include "func_lists.h"

static std::mutex coutMutex;

static void compute_level(int level) {
    int n = funcsize[level];
    int bits = 1 << level;
    mbf6 mask = (bits < 64) ? ((mbf6(1) << bits) - 1) : F6_MAX;

    int n_threads = std::thread::hardware_concurrency();
    if (n_threads == 0) n_threads = 4;
    if (n_threads > n) n_threads = n;

    std::atomic<int> progress(0);
    std::vector<long long> t_sum(n_threads, 0);
    std::vector<long long> t_sum_sq(n_threads, 0);

    auto worker = [&](int tid) {
        long long s = 0, s2 = 0;
        for (int i = tid; i < n; i += n_threads) {
            mbf6 fi = funclists[level][i];
            for (int j = 0; j < n; j++) {
                int d = __builtin_popcountll((fi ^ funclists[level][j]) & mask);
                s += d;
                s2 += (long long)d * d;
            }
            int done = ++progress;
            if (done % 1000 == 0 || done == n) {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cerr << "\rMBF" << level << ": " << done << "/" << n << std::flush;
            }
        }
        t_sum[tid] = s;
        t_sum_sq[tid] = s2;
    };

    std::vector<std::thread> threads(n_threads);
    for (int i = 0; i < n_threads; i++)
        threads[i] = std::thread(worker, i);
    for (auto& t : threads) t.join();

    long long total_sum = 0, total_sum_sq = 0;
    for (int i = 0; i < n_threads; i++) {
        total_sum += t_sum[i];
        total_sum_sq += t_sum_sq[i];
    }

    long long N = (long long)n * n;
    double mean = (double)total_sum / N;
    double variance = (double)total_sum_sq / N - mean * mean;
    double stddev = std::sqrt(variance);

    std::cerr << "\r                              \r";
    std::cout << "MBF" << level
              << ": n=" << n
              << ", bits=" << bits
              << ", mean=" << mean
              << ", std=" << stddev
              << std::endl;
}

int main() {
    func_lists_init();
    std::cout.precision(17);

    for (int level = 0; level <= 6; level++)
        compute_level(level);

    return 0;
}
