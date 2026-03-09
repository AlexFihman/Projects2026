#include <iostream>
#include "MonotoneBooleanFunction.h"

const int rsize = 8;
const int size = 512;

int hammingDistance(uint64_t x, uint64_t y) {
    return std::bitset<64>(x ^ y).count();
}

MonotoneBooleanFunction::MonotoneBooleanFunction(int dim, std::mt19937& rng) :
         dimension(dim), weight(0), functionArray(nullptr), rng(rng), min_cuts(rng) {

    functionArray = new bool[1 << dim](); // Initialize all values to false
    for (int i=0; i< (1 << dim); i++) functionArray[i] = false;
    updateMinCuts();
}

MonotoneBooleanFunction::MonotoneBooleanFunction(const MonotoneBooleanFunction& other, std::mt19937& rng) :
        dimension(other.dimension), weight(other.weight), functionArray(nullptr),
        rng(rng), min_cuts(other.min_cuts, rng) {
    int n = 1 << dimension;
    functionArray = new bool[n];
    std::copy(other.functionArray, other.functionArray + n, functionArray);
}

void MonotoneBooleanFunction::initFromValues(const std::vector<bool>& values) {
    int n = 1 << dimension;
    weight = 0;
    for (int i = 0; i < n; i++) {
        functionArray[i] = values[i];
        if (values[i]) weight++;
    }
    updateMinCuts();
}

MonotoneBooleanFunction::~MonotoneBooleanFunction() {
    delete[] functionArray;
}

bool MonotoneBooleanFunction::getFunctionValue(int index) const {
    return functionArray[index];
}

void MonotoneBooleanFunction::setFunctionValue(int index, bool value) {
    functionArray[index] = value;
    updateMinCuts();
}

void MonotoneBooleanFunction::flip(int index) {
    functionArray[index] = !functionArray[index];
    weight += functionArray[index]? 1 : -1;
    updateMinCutsFast(index);
}

void MonotoneBooleanFunction::flipRandom() {
    int index = getRandomMinCut();
    functionArray[index] = !functionArray[index];
    weight += functionArray[index]? 1 : -1;
    updateMinCutsFast(index);
}

bool MonotoneBooleanFunction::metropolisStep() {
    int old_cuts = min_cuts.getSize();
    int index = getRandomMinCut();
    flip(index);
    int new_cuts = min_cuts.getSize();

    // Acceptance ratio for uniform target: min(1, old_cuts / new_cuts).
    // If new_cuts <= old_cuts the move is always accepted.
    if (new_cuts > old_cuts) {
        std::uniform_real_distribution<double> u(0.0, 1.0);
        if (u(rng) * new_cuts > old_cuts) {
            flip(index); // reject: restore previous state
            return false;
        }
    }
    return true;
}

bool MonotoneBooleanFunction::checkMinCut(int index) const {
    for (int k=0; k < dimension; k++) {
      int idx2 = index ^ (1 << k);
      //std::cout << "idx2: " << idx2 << "\t" << "value: " << functionArray[index] << std::endl;
      if (idx2 < index && functionArray[idx2]) return false;
      if (idx2 > index && !functionArray[idx2]) return false;
    }
    return true;
}

void MonotoneBooleanFunction::updateMinCuts() {
    for (int i=0; i< (1 << dimension); i++) {
      if (checkMinCut(i)) {
        min_cuts.insert(i);
      } else {
        min_cuts.remove(i);
      }
    }
}

void MonotoneBooleanFunction::updateMinCutsFast(int index) {
    for (int k=0; k < dimension; k++) {
      int idx2 = index ^ (1 << k);   
      if (checkMinCut(idx2)) {
        min_cuts.insert(idx2);
      } else {
        min_cuts.remove(idx2);
      }
    }
}

int MonotoneBooleanFunction::getRandomMinCut() const {
    return min_cuts.getRandomElement();
}

void MonotoneBooleanFunction::printMinCuts() const {
    min_cuts.print();
}

int MonotoneBooleanFunction::getWeight() const {
    return weight;
}

int MonotoneBooleanFunction::minCutSize() const {
    return min_cuts.getSize();
}

void MonotoneBooleanFunction::toRecord(Record& r) {
    for (int j = 0; j < rsize; j++) r.data[j] = 0;
    for (int j = 0; j < size; j++) {
        if (getFunctionValue(j)) {
            int p = (size - j - 1) / 64;
            int k = (size - j - 1) % 64;
            r.data[p] ^= (1ULL << k);
        }
    }
}

int MonotoneBooleanFunction::recordHammingDistance(const Record& rec1, const Record& rec2) {
    int distance = 0;
    for (int i = 0; i < 8; ++i) {
        distance += hammingDistance(rec1.data[i], rec2.data[i]);
    }
    return distance;
}