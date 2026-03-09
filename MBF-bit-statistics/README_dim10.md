# MBF Bit Statistics: Dimension 10

Results from running the Metropolis-Hastings MCMC on n=10 monotone boolean
functions (MBFs), measuring per-bit marginal probabilities and the expected
Hamming distance between two independent uniform MBFs.

## Setup

- **Dimension**: n=10, so 2^10 = 1024 inputs
- **Chains**: 12 parallel chains (hardware_concurrency), each 10^8 steps
- **Initialization**: `randomBoundaryInit(mbf, dim, 5, 5, rng)` — sets hw<5=0,
  hw>5=1, randomizes hw=5, fixes violations, then 10k burn-in steps
- **Pooling**: all bits at the same Hamming weight share the same marginal p_k,
  and complement symmetry gives p_k + p_{n-k} = 1

## Marginal probabilities p_k

| hw pair | C(10,hw) | p(hw=k)      | 2p(1-p)      | contribution |
|---------|----------|--------------|--------------|--------------|
| 0+10    | 1        | 0.000000     | 0.000000     | 0.000000     |
| 1+9     | 10       | ~0.000000    | ~0.000000    | ~0.000000    |
| 2+8     | 45       | ~0.000000    | ~0.000000    | ~0.000000    |
| 3+7     | 120      | ~0.000028    | ~0.000056    | ~0.013        |
| 4+6     | 210      | ~0.019       | ~0.037       | ~15.56        |
| 5+5     | 252      | 0.500000     | 0.500000     | 126.000000   |

p_5 = 0.5 exactly by complement symmetry (n is even, so hw=5 = n/2).

**Estimated E[d_H(f,g)] ≈ 141.562** (reference run: 141.5617405609 ± 0.0002 SE)

The expected Hamming distance between two independently drawn uniform n=10 MBFs
is dominated by the middle level: 126 out of 141.56 comes from hw=5 bits alone.

## Initialization pitfall

An earlier run used `randomBoundaryInit(mbf, dim, 4, 6, rng)`, which initializes
hw=4 and hw=6 bits uniformly at random (p≈0.5). But the stationary marginal is
p_4≈0.019, far from 0.5. This causes a large upward bias (~0.008) in the
estimated Hamming distance because the chain needs many steps to escape the
wrong initialization region.

**Fix**: use `lo=hi=5` so only hw=5 bits are randomized. The hw=4 and hw=6 bits
start at their stationary-consistent boundary values (0 and 1 respectively),
and the 10k burn-in is sufficient.

## Distribution of hw=5 count

The program `measure_dist` records, at each MCMC step, how many of the 252
hw=5 inputs have f(x)=1. Over 10^7 steps:

| Statistic | Value |
|-----------|-------|
| Mean      | ~126 (=252/2, exact by symmetry) |
| Std dev   | ~21  |
| Range     | ~67–194 out of 252 |

For comparison, if all 252 bits were **independent** Bernoulli(0.5), the count
would follow Binomial(252, 0.5) with σ≈7.9. The observed σ≈21 is ~2.7× wider.

This reflects **positive correlation** between bits at the same level within a
single MBF: if many hw=5 inputs are 1, it is likely that many others are too
(the "boundary" of the MBF passes through a wide band around level 5 together).

The distribution is unimodal and roughly symmetric around 126.

### Implication for initialization

Since the empirical distribution of the hw=5 count is much wider than Binomial,
a good starting point should draw the hw=5 count from this empirical distribution
(or a Gaussian approximation N(126, 21^2)) rather than initializing all hw=5
bits independently at p=0.5. This reduces the burn-in needed when the chain
starts at an atypical configuration.

## Files

| File | Description |
|------|-------------|
| `main.cpp` | Parallel chains, pools by level, estimates E[d_H] |
| `measure_dist.cpp` | Measures empirical distribution of hw=n/2 count |
| `MonotoneBooleanFunction.h/cpp` | MBF data structure (shared with MCMC project) |
| `ShortList.h/cpp` | O(1) random-access set |
| `Makefile` | Builds `bitstats` and `measure_dist` |
