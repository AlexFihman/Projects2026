#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cmath>

#include "func_lists.h"
#include "mbf_generate.h"

static int hamming7(mbf7 a, mbf7 b) {
    return __builtin_popcountll(a.f1 ^ b.f1)
         + __builtin_popcountll(a.f2 ^ b.f2);
}

static int hamming8(mbf8 a, mbf8 b) {
    return hamming7(a.f1, b.f1)
         + hamming7(a.f2, b.f2);
}

struct ThreadResult {
    long long sum;
    long long sum_sq;
};

static void worker(int n, unsigned seed, ThreadResult& result) {
    std::mt19937 rng(seed);
    long long s = 0, s2 = 0;
    for (int i = 0; i < n; i++) {
        mbf8 a = make8(rng);
        mbf8 b = make8(rng);
        int d = hamming8(a, b);
        s += d;
        s2 += (long long)d * d;
    }
    result.sum = s;
    result.sum_sq = s2;
}

int main(int argc, char* argv[]) {
    int N = 1000000;
    int n_threads = std::thread::hardware_concurrency();
    if (n_threads == 0) n_threads = 4;

    if (argc >= 2) N = atoi(argv[1]);
    if (argc >= 3) n_threads = atoi(argv[2]);

    func_lists_init();

    std::random_device rd;
    std::vector<std::thread> threads(n_threads);
    std::vector<ThreadResult> results(n_threads);

    int per_thread = N / n_threads;
    int remainder = N % n_threads;

    for (int i = 0; i < n_threads; i++) {
        int count = per_thread + (i < remainder ? 1 : 0);
        threads[i] = std::thread(worker, count, rd(), std::ref(results[i]));
    }
    for (auto& t : threads) t.join();

    long long total_sum = 0, total_sum_sq = 0;
    for (auto& r : results) {
        total_sum += r.sum;
        total_sum_sq += r.sum_sq;
    }

    double mean = (double)total_sum / N;
    double variance = (double)total_sum_sq / N - mean * mean;
    double stddev = std::sqrt(variance);

    std::cout.precision(17);
    std::cout << "N = " << N << ", threads = " << n_threads << std::endl;
    std::cout << "Mean Hamming distance: " << mean << std::endl;
    std::cout << "Std deviation:         " << stddev << std::endl;

    return 0;
}
