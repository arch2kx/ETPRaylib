#pragma once
#include <cstdint>

class Rng {
public:
    explicit Rng(uint64_t seed = 0x853c49e6748fea9bULL) { Seed(seed); }

    void Seed(uint64_t seed) {
        state = 0u;
        inc   = (seed << 1u) | 1u;
        Next();
        state += seed;
        Next();
    }

    uint32_t Next() {
        uint64_t old = state;
        state = old * 6364136223846793005ULL + inc;
        uint32_t xorshifted = (uint32_t)(((old >> 18u) ^ old) >> 27u);
        uint32_t rot = (uint32_t)(old >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31u));
    }

    int Range(int maxExclusive) {
        if (maxExclusive <= 0) return 0;
        return (int)(Next() % (uint32_t)maxExclusive);
    }

    uint64_t State() const { return state; }

private:
    uint64_t state;
    uint64_t inc;
};
