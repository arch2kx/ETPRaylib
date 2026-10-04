#include "replay.hpp"
#include "game.hpp"
#include "det.hpp"
#include "difficulty.hpp"
#include "input_state.hpp"
#include "raylib.h"
#include <cstdio>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: verify_replay <file.etprep> [expected-score]\n");
        return 2;
    }
    SetTraceLogLevel(LOG_ERROR);

    Replay r;
    ReplayLoad rc = LoadReplay(argv[1], r);
    if (rc != ReplayLoad::OK) {
        printf("REJECT %s: %s\n", argv[1], ReplayLoadMessage(rc));
        return 3;
    }

    Game g(DIFF_LIST[r.difficultyIndex], r.endless != 0, r.seed, true);
    uint32_t ticks = 0;
    for (uint8_t packed : r.inputs) {
        g.Update(det::FIXED_DT, InputState::Unpack(packed));
        ticks++;
        if (g.IsGameOver() || g.IsGameWon()) break;
    }

    int actual = g.GetScore();
    printf("replay    difficulty=%s endless=%d seed=%llu ticks=%u/%zu\n",
           DIFF_NAMES[r.difficultyIndex], (int)r.endless,
           (unsigned long long)r.seed, ticks, r.inputs.size());
    printf("claimed   %d\n", r.claimedScore);
    printf("recomputed %d\n", actual);

    if (actual != r.claimedScore) {
        printf("REJECT: claimed score does not match the simulation\n");
        return 1;
    }

    if (argc > 2) {
        int expected = atoi(argv[2]);
        if (actual != expected) {
            printf("REJECT: score %d does not match expected %d\n", actual, expected);
            return 1;
        }
    }

    if (!g.IsGameOver() && !g.IsGameWon()) {
        printf("REJECT: replay ended without the run finishing\n");
        return 1;
    }

    printf("ACCEPT %d\n", actual);
    return 0;
}
