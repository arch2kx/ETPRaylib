#include "replay.hpp"
#include "game.hpp"
#include "det.hpp"
#include "difficulty.hpp"
#include "input_state.hpp"
#include "rng.hpp"
#include "raylib.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: make_replay <out.etprep> <seed> [score-offset]\n");
        return 2;
    }
    SetTraceLogLevel(LOG_ERROR);

    uint64_t seed = strtoull(argv[2], nullptr, 10);
    int offset = argc > 3 ? atoi(argv[3]) : 0;

    Replay r;
    r.seed            = seed;
    r.difficultyIndex = 1;
    r.endless         = 0;

    Game g(DIFF_LIST[r.difficultyIndex], false, seed, true);
    Rng scripted(seed ^ 0xABCDEF);
    uint8_t cur = 0;
    for (uint32_t i = 0; i < 120u * 180u; i++) {
        // Hold still and shoot: random flailing dies in seconds and scores
        // nothing, which would make the verifier test vacuous.
        cur = 16;
        if (scripted.Range(240) == 0) cur |= (uint8_t)(1 << scripted.Range(4));
        r.inputs.push_back(cur);
        g.Update(det::FIXED_DT, InputState::Unpack(cur));
        if (g.IsGameOver() || g.IsGameWon()) break;
    }

    r.claimedScore = g.GetScore() + offset;
    if (!SaveReplay(argv[1], r)) { printf("could not write %s\n", argv[1]); return 1; }
    printf("wrote %s score=%d ticks=%zu\n", argv[1], r.claimedScore, r.inputs.size());
    return 0;
}
