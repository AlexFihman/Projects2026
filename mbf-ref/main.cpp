#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstdlib>
#include <random>

#include "time_sec.h"
#include "func_lists.h"
#include "mbf_generate.h"

static std::mutex coutMutex;
static int seed;
static int last_seed;
static double startTime;

static void threadFunction() {
    std::mt19937 rng;
    while (true) {
        int arg;
        {
            std::lock_guard<std::mutex> lock(coutMutex);
            arg = seed++;
        }
        if (arg > last_seed)
            break;
        rng.seed(arg);
        mbf9 m = make9(rng);
        double t1 = TimeSec();

        std::lock_guard<std::mutex> lock(coutMutex);
#ifdef MBF_USE_BOOST
        std::cout << arg << "\t" << (t1 - startTime) << "\t" << m << std::endl;
#else
        // Without Boost, print the struct fields in hex
        auto printMbf8 = [](const mbf8& v) {
            std::cout << std::hex
                << v.f1.f1 << " " << v.f1.f2 << " "
                << v.f2.f1 << " " << v.f2.f2;
        };
        std::cout << std::dec << arg << "\t" << (t1 - startTime) << "\t";
        printMbf8(m.f1);
        std::cout << " ";
        printMbf8(m.f2);
        std::cout << std::endl;
#endif
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <seed> <count> <threads>" << std::endl;
        return 1;
    }

    seed = atoi(argv[1]);
    int count = atoi(argv[2]);
    last_seed = seed + count - 1;
    int n_thread = atoi(argv[3]);

    func_lists_init();
    std::cout << "funcsize[6] = " << funcsize[6] << std::endl;

    startTime = TimeSec();

    std::vector<std::thread> threads(n_thread);
    for (int i = 0; i < n_thread; i++)
        threads[i] = std::thread(threadFunction);
    for (int i = 0; i < n_thread; i++)
        threads[i].join();

    return 0;
}
