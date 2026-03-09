#pragma once

#include <bitset>
#include <vector>
#include "ShortList.h"

struct Record {
    uint64_t data[8];
};

class MonotoneBooleanFunction {
private:
    int dimension; // Dimension of the boolean function
    int weight;
    bool* functionArray; // Array to store the boolean function
    std::mt19937& rng; // Reference to Mersenne Twister random number generator
    ShortList min_cuts; // ShortList to store the minimum cuts
    bool checkMinCut(int index) const;
    void updateMinCuts();
    void updateMinCutsFast(int index);

public:
    MonotoneBooleanFunction(int dim, std::mt19937& rng);
    MonotoneBooleanFunction(const MonotoneBooleanFunction& other, std::mt19937& rng);

    // Bulk-set all function values and recompute weight + min-cuts in one pass.
    void initFromValues(const std::vector<bool>& values);

    ~MonotoneBooleanFunction();

    bool getFunctionValue(int index) const;

    void setFunctionValue(int index, bool value);

    void flip(int index);

    void flipRandom();

    // MH step targeting the uniform distribution over MBFs.
    // Returns true if the proposed move was accepted.
    bool metropolisStep();

    int getRandomMinCut() const;

    void printMinCuts() const;

    int getWeight() const;

    int minCutSize() const;

    void toRecord(Record& r);

    static int recordHammingDistance(const Record& rec1, const Record& rec2);
};
