#include "raylib.h"
#include "game.hpp"
#include "det.hpp"
#include "rng.hpp"
#include <cstdio>
#include <vector>

static std::vector<uint8_t> ScriptInputs(uint64_t seed, int ticks) {
    Rng r(seed);
    std::vector<uint8_t> v;
    v.reserve(ticks);
    uint8_t cur = 0;
    for (int i = 0; i < ticks; i++) {
        if (r.Range(12) == 0) cur = (uint8_t)r.Range(32);
        v.push_back(cur);
    }
    return v;
}

static uint64_t RunOnce(uint64_t seed, const std::vector<uint8_t>& inputs, int* outScore) {
    Game g(DIFF_NORMAL, false, seed, /*headless=*/true);
    uint64_t h = 0;
    for (size_t i = 0; i < inputs.size(); i++) {
        g.Update(det::FIXED_DT, InputState::Unpack(inputs[i]));
        h = g.StateHash();
        if (g.IsGameOver() || g.IsGameWon()) break;
    }
    *outScore = g.GetScore();
    return h;
}

int main(int argc, char** argv) {
    SetTraceLogLevel(LOG_ERROR);
    const int TICKS = 120 * 60;
    int fails = 0;

    for (uint64_t seed : {1ULL, 42ULL, 99991ULL}) {
        auto inputs = ScriptInputs(seed ^ 0xABCDEF, TICKS);
        int s1 = 0, s2 = 0;
        uint64_t h1 = RunOnce(seed, inputs, &s1);
        uint64_t h2 = RunOnce(seed, inputs, &s2);
        bool ok = (h1 == h2) && (s1 == s2);
        if (!ok) fails++;
        printf("seed %-6llu  score %d/%d  hash %016llx/%016llx  %s\n",
               (unsigned long long)seed, s1, s2,
               (unsigned long long)h1, (unsigned long long)h2, ok ? "MATCH" : "DIVERGED");
    }

    int sa = 0, sb = 0;
    auto ia = ScriptInputs(7, TICKS);
    uint64_t ha = RunOnce(1234, ia, &sa);
    uint64_t hb = RunOnce(5678, ia, &sb);
    printf("different seeds diverge: %s\n", ha != hb ? "yes" : "NO (suspicious)");

    if (argc > 1) {
        FILE* f = fopen(argv[1], "wb");
        if (f == nullptr) { printf("could not write %s\n", argv[1]); return 2; }
        for (uint64_t seed : {1ULL, 42ULL, 99991ULL}) {
            auto inputs = ScriptInputs(seed ^ 0xABCDEF, TICKS);
            int sc = 0;
            uint64_t h = RunOnce(seed, inputs, &sc);
            fprintf(f, "seed=%llu score=%d hash=%016llx\n",
                    (unsigned long long)seed, sc, (unsigned long long)h);
        }
        fclose(f);
        printf("wrote %s\n", argv[1]);
    }

    printf("%s\n", fails == 0 ? "DETERMINISM OK" : "DETERMINISM BROKEN");
    return fails;
}
