# MBF Reference Implementation

Reference implementation of random Monotone Boolean Function generation using rejection sampling.

## Algorithm

1. **func_lists_init()** builds all monotone boolean functions up to level 6 (mbf6, 64-bit). This produces `funcsize[6] = 7,828,354` functions.

2. **make7/8/9()** use rejection sampling to generate random MBFs at higher levels:
   - `make7`: pick two random mbf6 functions, accept if compatible (f1 ≤ f2), combine into 128-bit mbf7
   - `make8`: pick two random mbf7, accept if compatible, combine into 256-bit mbf8
   - `make9`: pick two random mbf8, accept if compatible, combine into 512-bit mbf9

## Files

| File | Description |
|------|-------------|
| `mbf_types.h` | Type definitions (mbf5..mbf9), Boost or struct backend |
| `func_lists.h/cpp` | Function list construction (levels 0-6) |
| `mbf_generate.h/cpp` | Rejection sampling: make7, make8, make9 |
| `time_sec.h/cpp` | TimeSec() high-precision timer |
| `main.cpp` | Multi-threaded mbf9 generation |
| `hamming_exact.cpp` | Exact Hamming distance stats for MBF0-6 (full enumeration, multithreaded) |
| `hamming6.cpp` | Random sampling Hamming distance for MBF6 |
| `hamming7.cpp` | Random sampling Hamming distance for MBF7 |
| `hamming8.cpp` | Random sampling Hamming distance for MBF8 |
| `Makefile` | Build with `mingw32-make` or `make` |

## Build

```
make
```

Without Boost (default), uses nested structs with overloaded operators. With Boost:

```
make CXXFLAGS="-std=c++17 -O2 -DMBF_USE_BOOST"
```

## Usage

```
./mbf-ref <seed> <count> <threads>
```

- **seed**: starting RNG seed
- **count**: number of mbf9 values to generate
- **threads**: number of worker threads

## Hamming Distance Tools

### Exact enumeration (MBF0-6)

```
make hamming_exact
./hamming_exact
```

Enumerates all pairs at each level and computes exact mean and standard deviation. Level 6 has 7,828,354 functions (~61 trillion pairs) — requires significant compute. Consider using GPU for level 6.

### Random sampling (MBF6/7/8)

```
make hamming7
./hamming7 [N] [threads]
```

Defaults: N=1000000, threads=auto. Same usage for `hamming6` and `hamming8`.
