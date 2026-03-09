# MBF Bit Statistics

Estimates the stationary bit probabilities of the uniform distribution over
monotone boolean functions (MBFs) on `n` variables using Metropolis-Hastings
MCMC, and from those probabilities computes the expected Hamming distance
between two independently drawn MBFs.

## Background

A **monotone boolean function** on `n` variables is a function
`f: {0,1}^n → {0,1}` satisfying `x ≤ y ⟹ f(x) ≤ f(y)` (componentwise).
The number of such functions is the Dedekind number D(n); for n=9 it is
astronomically large (~2.86 × 10^23).

### Key symmetries exploited

**Level symmetry.** The uniform distribution over MBFs is invariant under
permutation of the n input coordinates. Therefore all 2^n inputs at the same
Hamming weight level `k` have the same marginal probability `p_k = P(f(x)=1)`
for any fixed `x` with `|x|=k`. Pooling counts across all C(n,k) inputs at
level k gives C(n,k)× more effective samples for estimating `p_k`.

**Complement symmetry.** If `f` is a uniform random MBF then so is its
complement `g(x) = 1 - f(~x)`. This gives `p_k + p_{n-k} = 1`, so the upper
half of levels is determined by the lower half. Pooling level k with level n-k
gives a further 2× improvement.

### Expected Hamming distance

For two independently drawn MBFs f and g:

```
E[d_H(f, g)] = sum_x  2 * p_{|x|} * (1 - p_{|x|})
             = sum_{k=0}^{n} C(n,k) * 2 * p_k * (1 - p_k)
```

For n=9 the sum is dominated by levels 4 and 5 (each contributing ~42.5),
with small contributions from levels 3 and 6 (~0.5 each), and negligible
contributions from all other levels.

## Results (n=9)

Running 16 parallel chains × 10^8 steps (1.6 × 10^9 total):

| hw pair | C(9,hw) | p(hw=k)    | contribution |
|---------|---------|------------|--------------|
| 0+9     | 1       | ~0         | ~0           |
| 1+8     | 9       | ~0         | ~0           |
| 2+7     | 36      | ~3×10⁻⁸    | ~0           |
| 3+6     | 84      | ~2.97×10⁻³ | ~0.995       |
| 4+5     | 126     | ~0.2153    | ~85.16       |

**Estimated E[d_H] ≈ 86.15** (known long-run value: 86.15173)

Sample results across runs:

| run | estimate  | error  |
|-----|-----------|--------|
| 1   | 86.149688 | -0.002 |
| 2   | 86.154362 | +0.003 |
| 3   | 86.153212 | +0.002 |

Observed run-to-run std dev ≈ **0.002–0.003**.

## Initialization: `randomBoundaryInit`

Instead of burning in from the all-zeros function (100k MH steps), each chain
is initialized by:

1. Set `f(x) = 0` for `|x| < 4`, `f(x) = 1` for `|x| > 5`, random for `|x| ∈ {4,5}`
2. Fix any monotonicity violations between levels 4 and 5 by repeatedly
   picking a random outlier and setting it to the correcting value (always
   terminates; no new violations are created by any single fix)
3. Run 10k MH steps to allow the boundary to drift into levels 3 and 6
   (whose stationary probability p₃ ≈ 0.003 is nonzero but small)

This puts each chain immediately in the statistically relevant region,
reducing required burn-in from 100k to 10k steps.

## Autocorrelation and precision

The MH chain has integrated autocorrelation time τ_int ≈ 4000 steps for the
hw=4 pooled average (measured on a single chain of 500k steps). The single-
chain theoretical std dev for 10^8 steps is ~0.25 — far from 0.001 precision.

However, running N **independent** chains and averaging reduces variance as
1/N regardless of within-chain autocorrelation, because each chain starts from
a fresh independent initialization. Empirically, each batch of 16 chains ×
10^8 steps gives std dev ≈ **0.002–0.003**.

To reach std_dev < 0.001: need roughly **(0.003/0.001)² ≈ 9×** more chains,
i.e. ~150 chains × 10^8 steps. On 16-thread hardware this is ~9 sequential
batches (~20 minutes total).

**Key insight:** more independent chains is far more effective than one long
chain, because chain-to-chain variance is independent of τ_int.

### Remaining avenue: complement mixing

The slow oscillation in single-chain autocorrelation (bumps at lags ~7000 and
~15000) arises from the chain slowly diffusing between the two halves of MBF
space related by complement symmetry. Occasionally jumping `f → complement(f)`
(always accepted under the uniform target, O(2^n) cost) would directly disrupt
this and reduce τ_int, making long single-chain runs more efficient too.

## Build

```
make
```

Requires a C++17 compiler. The `MonotoneBooleanFunction` and `ShortList`
sources are symlinked from the `Metropolis-Hastings-MCMC` project.

## Usage

```
./bitstats
```

Runs 16 parallel chains (hardcoded; adjust for available cores), each for
10^8 steps, then prints per-level probabilities and the estimated Hamming
distance.

To change the number of threads or steps, edit `nthreads` and
`steps_per_thread` in `main.cpp`.
