# MBF First Layers

Counts **Monotone Boolean Functions** (MBFs) on `{0,1}^9` grouped by the number of ones in their truth table (the "layer").

## What is a Monotone Boolean Function?

A function `f: {0,1}^n → {0,1}` is monotone if `x ≤ y` componentwise implies `f(x) ≤ f(y)`.
It is stored as a truth table — a bitset of 2^n bits, where bit `i` holds `f(binary(i))`.

## Algorithm

1. **Build the lex-max MBF** with exactly `k` bits set by greedily flipping the smallest
   flippable zero-bit from the all-zero function.  A zero-bit is flippable if all its
   supermasks (componentwise supersets) are already 1.

2. **Iterate lex-prev**: repeatedly find the lexicographically largest valid MBF with the
   same bit count that is smaller than the current one, until none exists.

3. The **counter** — starting at 1 and incremented at each step — gives the total number
   of MBFs at that layer.

### Lex-prev construction

To find the predecessor of `f`:
- Scan from the highest set-bit `j` downward.
- Skip `j` if any prefix bit (index < `j`) has `j` as a supermask (dropping `j` would
  break monotonicity for that bit).
- Pre-commit all supermasks of prefix bits at indices > `j` (required by monotonicity).
- Greedily fill remaining slots (smallest index first) by committing each candidate bit
  together with all its not-yet-committed supermasks, if the total cost fits the budget.
  This look-ahead ensures we don't waste slots on bits that block smaller, more
  lex-significant ones.

## Build & Run

```bash
make
./mbf <k>        # print k,count
./mbf <k> -v     # verbose: print every function in the sequence
```

## Results (dimension 9)

| k  | count     |
|----|-----------|
| 0  | 1         |
| 1  | 1         |
| 2  | 9         |
| 3  | 36        |
| 4  | 120       |
| 5  | 378       |
| 6  | 1 134     |
| 7  | 3 318     |
| 8  | 9 570     |
| 9  | 27 099    |
| 10 | 76 021    |
| 11 | 209 868   |
| 12 | 573 426   |
| 13 | 1 542 219 |
| 14 | 4 091 157 |
