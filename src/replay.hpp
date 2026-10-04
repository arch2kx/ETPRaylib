#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Bump whenever anything that affects the simulation changes: entity speeds,
// difficulty values, the trig polynomials, the timestep. Replays recorded
// under a different value cannot be verified and must be rejected rather
// than silently producing a wrong score.
constexpr uint32_t GAMEPLAY_VERSION = 1;

constexpr uint32_t REPLAY_MAGIC   = 0x52505445;
constexpr uint16_t REPLAY_FORMAT  = 1;

constexpr uint32_t REPLAY_MAX_TICKS = 120u * 60u * 60u * 6u;

struct Replay {
    uint16_t format         = REPLAY_FORMAT;
    uint32_t gameplayVersion = GAMEPLAY_VERSION;
    uint64_t seed           = 0;
    uint8_t  difficultyIndex = 0;
    uint8_t  endless        = 0;
    int32_t  claimedScore   = 0;
    std::vector<uint8_t> inputs;
};

bool SaveReplay(const std::string& path, const Replay& r);

enum class ReplayLoad { OK, NOT_FOUND, BAD_MAGIC, BAD_FORMAT, BAD_VERSION, TRUNCATED, TOO_LONG, BAD_DIFFICULTY };

ReplayLoad LoadReplay(const std::string& path, Replay& out);
const char* ReplayLoadMessage(ReplayLoad r);
