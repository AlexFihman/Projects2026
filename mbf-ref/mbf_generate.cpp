#include "mbf_generate.h"
#include "func_lists.h"

static bool cmp6f(mbf6 f1, mbf6 f2) {
    return ((~f1) | f2) == F6_MAX;
}

mbf6 make6(std::mt19937& rng) {
    int N6 = funcsize[6];
    int i1 = rng() % N6;
    return funclists[6][i1];
}

#ifdef MBF_USE_BOOST

static bool cmp7(mbf7 f1, mbf7 f2) {
    return ((~f1) | f2) == F7_MAX;
}

static bool cmp8(mbf8 f1, mbf8 f2) {
    return ((~f1) | f2) == F8_MAX;
}


mbf7 make7(std::mt19937& rng) {
    int N6 = funcsize[6];
    mbf6 f1, f2;
    do {
        int i1 = rng() % N6;
        int i2 = rng() % N6;
        f1 = funclists[6][i1];
        f2 = funclists[6][i2];
    } while (!cmp6f(f1, f2));
    mbf7 result = f1;
    result = (result << 64) | f2;
    return result;
}

mbf8 make8(std::mt19937& rng) {
    mbf7 f1, f2;
    do {
        f1 = make7(rng);
        f2 = make7(rng);
    } while (!cmp7(f1, f2));
    mbf8 result = f1;
    result = (result << 128) | f2;
    return result;
}

mbf9 make9(std::mt19937& rng) {
    mbf8 f1, f2;
    do {
        f1 = make8(rng);
        f2 = make8(rng);
    } while (!cmp8(f1, f2));
    mbf9 result = f1;
    result = (result << 256) | f2;
    return result;
}

#else

static bool cmp7(mbf7 f1, mbf7 f2) {
    return cmp6f(f1.f1, f2.f1) && cmp6f(f1.f2, f2.f2);
}

static bool cmp8(mbf8 f1, mbf8 f2) {
    return cmp7(f1.f1, f2.f1) && cmp7(f1.f2, f2.f2);
}

mbf7 make7(std::mt19937& rng) {
    int N6 = funcsize[6];
    mbf6 f1, f2;
    do {
        int i1 = rng() % N6;
        int i2 = rng() % N6;
        f1 = funclists[6][i1];
        f2 = funclists[6][i2];
    } while (!cmp6f(f1, f2));
    return {f1, f2};
}

mbf8 make8(std::mt19937& rng) {
    mbf7 f1, f2;
    do {
        f1 = make7(rng);
        f2 = make7(rng);
    } while (!cmp7(f1, f2));
    return {f1, f2};
}

mbf9 make9(std::mt19937& rng) {
    mbf8 f1, f2;
    do {
        f1 = make8(rng);
        f2 = make8(rng);
    } while (!cmp8(f1, f2));
    return {f1, f2};
}

#endif
