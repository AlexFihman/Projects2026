#include <iostream>
#include <optional>
#include <vector>
#include <bitset>

enum class Relation { LESS, EQUAL, GREATER, INCOMPARABLE };

constexpr int DIM  = 9;
constexpr int SIZE = 1 << DIM;  // 512

class MonotoneBooleanFunction {
public:
    std::bitset<SIZE> values;  // values[i] = f(binary representation of i)

    MonotoneBooleanFunction() : values(0) {}

    bool get(int i) const { return values[i]; }
    void set(int i, bool v) { values[i] = v; }

    // Can bit i be flipped while preserving monotonicity?
    bool can_flip(int i) const {
        if (!values[i]) {
            // Flipping 0 -> 1: all supermasks of i must already be 1
            for (int j = i + 1; j < SIZE; j++)
                if ((j & i) == i && !values[j])
                    return false;
        } else {
            // Flipping 1 -> 0: all submasks of i must already be 0
            for (int j = 0; j < i; j++)
                if ((j & i) == j && values[j])
                    return false;
        }
        return true;
    }

    bool operator==(const MonotoneBooleanFunction& o) const { return values == o.values; }
    bool operator!=(const MonotoneBooleanFunction& o) const { return values != o.values; }

    // Lexicographic order: compare truth tables from index 0 upward
    bool operator<(const MonotoneBooleanFunction& o) const {
        for (int i = 0; i < SIZE; i++) {
            if (values[i] != o.values[i])
                return !values[i];  // 0 < 1
        }
        return false;  // equal
    }
    bool operator> (const MonotoneBooleanFunction& o) const { return o < *this; }
    bool operator<=(const MonotoneBooleanFunction& o) const { return !(o < *this); }
    bool operator>=(const MonotoneBooleanFunction& o) const { return !(*this < o); }

    bool is_monotone() const {
        for (int i = 0; i < SIZE; i++) {
            if (!values[i]) continue;
            for (int j = i + 1; j < SIZE; j++)
                if ((j & i) == i && !values[j])
                    return false;
        }
        return true;
    }

    void print_set_bits() const {
        for (int i = 0; i < SIZE; i++)
            if (values[i]) std::cout << " " << i;
        std::cout << "\n";
    }

    std::vector<int> flippable_bits() const {
        std::vector<int> result;
        for (int i = 0; i < SIZE; i++)
            if (can_flip(i))
                result.push_back(i);
        return result;
    }
};

// Returns the lex-prev valid MBF with the same number of bits set.
// Scans for the latest 1-bit j we can drop; keeps bits before j unchanged,
// then greedily fills the remaining slots at positions > j with the smallest
// valid indices (producing the lex-max completion, so the result is as close
// to mbf as possible while still being smaller).
std::optional<MonotoneBooleanFunction> lex_prev(const MonotoneBooleanFunction& mbf) {
    int k = (int)mbf.values.count();

    for (int j = SIZE - 1; j >= 0; j--) {
        if (!mbf.get(j)) continue;  // only consider 1-bits

        // Build prefix: mbf's bits at indices < j
        MonotoneBooleanFunction work;
        int prefix_count = 0;
        bool j_required = false;
        for (int p = 0; p < j; p++) {
            if (!mbf.get(p)) continue;
            work.set(p, true);
            prefix_count++;
            if ((j & p) == p) j_required = true;  // j is a supermask of prefix bit p
        }
        if (j_required) continue;  // dropping j would violate monotonicity for p

        int remaining = k - prefix_count;  // bits to place at indices > j (j itself is dropped)
        if (remaining <= 0) continue;

        // Step 1: pre-commit all supermasks of prefix bits at > j (required for monotonicity)
        std::bitset<SIZE> committed;
        int rem = remaining;
        for (int p = 0; p < j; p++) {
            if (!work.get(p)) continue;
            for (int m = j + 1; m < SIZE; m++)
                if ((m & p) == p && !committed[m]) { committed[m] = true; rem--; }
        }
        if (rem < 0) continue;  // prefix requires more bits than available slots

        // Step 2: lex-max fill for remaining 'rem' slots.
        // For each candidate p (smallest first), cost = p + all uncommitted supermasks at > j.
        // If cost fits, commit the whole group (this makes p achievable and is lex-optimal).
        for (int p = j + 1; p < SIZE && rem > 0; p++) {
            if (work.get(p) || committed[p]) continue;
            int cost = 0;
            for (int m = p; m < SIZE; m++)
                if ((m & p) == p && !work.get(m) && !committed[m]) cost++;
            if (cost > rem) continue;
            for (int m = p; m < SIZE; m++)
                if ((m & p) == p && !work.get(m) && !committed[m]) { committed[m] = true; rem--; }
        }
        if (rem != 0) continue;

        // Add committed bits to work (largest first so supermasks precede submasks)
        for (int m = SIZE - 1; m >= 0; m--)
            if (committed[m]) work.set(m, true);

        return work;
    }
    return std::nullopt;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <bits_to_set> [-v]\n";
        return 1;
    }
    int target = std::stoi(argv[1]);
    bool verbose = (argc == 3 && std::string(argv[2]) == "-v");
    if (target < 0 || target > SIZE) {
        std::cerr << "Error: bits_to_set must be in [0, " << SIZE << "]\n";
        return 1;
    }

    MonotoneBooleanFunction mbf;

    // Build initial function by greedily flipping the smallest available zero bit
    for (int step = 0; step < target; step++) {
        int chosen = -1;
        for (int i = 0; i < SIZE; i++) {
            if (!mbf.get(i) && mbf.can_flip(i)) { chosen = i; break; }
        }
        if (chosen == -1) {
            std::cerr << "No flippable zero bit found after " << step << " steps\n";
            return 1;
        }
        mbf.set(chosen, true);
    }

    if (verbose) {
        std::cout << "Initial function has " << mbf.values.count() << " bits set:";
        mbf.print_set_bits();
    }

    int counter = 1;

    while (true) {
        auto prev = lex_prev(mbf);
        if (!prev) break;
        mbf = *prev;
        counter++;
        if (verbose) {
            std::cout << "Step " << counter << (mbf.is_monotone() ? "" : " [INVALID]") << ":";
            mbf.print_set_bits();
        }
    }

    std::cout << target << "," << counter << "\n";
    return 0;
}
