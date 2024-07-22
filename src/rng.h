//
// Created by Benjamin Fawthrop on 7/22/24.
//

#ifndef OPSYSPROJ_RNG_H
#define OPSYSPROJ_RNG_H


// Class to handle the pseudo-random number generator state
class RandomGenerator {
public:
    RandomGenerator(long seed) {
        srand48(seed);
    }

    double drand48() {
        // generates a random double precision floating-point number between 0 and 1.
        my_seed = (0x5DEECE66DULL * my_seed + 0xB) & ((1ULL << 48) - 1);
        return static_cast<double>(my_seed) / (1ULL << 48);
    }

    void srand48(long seedval) {
        // initializes seed for predictable output
        my_seed = (static_cast<unsigned long long>(seedval) << 16) | 0x330E;
    }

private:
    unsigned long long my_seed;
};


#endif //OPSYSPROJ_RNG_H
