# Metropolis-Hastings MCMC on Monotone Boolean Functions

Implements a Metropolis-Hastings random walk on the space of monotone boolean
functions (MBFs) targeting the uniform distribution, and studies mixing via a
ring experiment.

## Components

### `MonotoneBooleanFunction`

Represents an MBF `f: {0,1}^n → {0,1}` stored as a flat boolean array of
size 2^n. Maintains a `ShortList` of **min-cut positions** — inputs where
flipping `f` preserves monotonicity — updated incrementally after each flip.

Key operations:
- `flipRandom()` — flip a uniformly random min-cut position (plain random walk)
- `metropolisStep()` — MH-corrected step targeting the uniform distribution
- `toRecord()` / `recordHammingDistance()` — pack the function into 8×uint64
  for fast bitwise Hamming distance computation

### `ShortList`

O(1) insert, remove, contains, and uniform random element for a set of
integers. Backed by a dense array + index array for O(1) random access.

## The MH correction

The plain `flipRandom` walk picks uniformly from the current min-cut set of
size `m(f)`. This does **not** satisfy detailed balance for the uniform
distribution because adjacent states can have different numbers of min-cuts.

`metropolisStep` adds the standard MH correction:

1. Propose flipping a random min-cut position (same as `flipRandom`)
2. After the flip, `m(f')` is recomputed incrementally
3. Accept with probability `min(1, m(f) / m(f'))`; if rejected, flip back

The reverse move is always available (the flipped position remains a min-cut
in `f'`), so the MH ratio reduces cleanly to the ratio of min-cut counts.

## Ring experiment

To measure the mixing time and equilibrium Hamming distance, `main.cpp` runs
a **ring experiment**:

1. Generate a ring of N=10001 MBFs where each function is one MH step away
   from the previous. Adjacent functions start 0–1 bits apart.
2. Evolve all functions **independently** via `metropolisStep`.
3. Track the average Hamming distance between ring-adjacent functions over
   time.

Initially distances are near 0. As the functions diverge under independent
evolution the average distance grows, saturating at the equilibrium value
`E[d_H(f, g)]` for two independently drawn uniform MBFs.

### Results (n=9, N=10001)

| Phase        | Rounds     | Avg distance |
|--------------|------------|--------------|
| Start        | 0          | ~0.5         |
| Growing      | 1–7000     | 0.5 → ~86    |
| Equilibrium  | ≥ ~8000    | ~86.15       |

The chain **mixes at ~8000 rounds**. The equilibrium Hamming distance
converges to **≈ 86.15**, consistent with the value computed by the
`MBF-bit-statistics` project from per-bit probabilities.

After mixing, the program accumulates a running mean and stops when the
estimated standard deviation of the mean drops below `target_std = 0.001`
(takes ~17500 post-mixing rounds with N=10001).

## Build & run

```
make
./walk
```

## Files

| File | Description |
|------|-------------|
| `MonotoneBooleanFunction.h/cpp` | MBF data structure and MH walk |
| `ShortList.h/cpp` | O(1) random-access set |
| `main.cpp` | Ring experiment |
| `Makefile` | Builds `walk` executable |
