#include "replay.hpp"
#include "difficulty.hpp"
#include <cstdio>

static void PutU8(std::vector<uint8_t>& b, uint8_t v) { b.push_back(v); }

static void PutU16(std::vector<uint8_t>& b, uint16_t v) {
    for (int i = 0; i < 2; i++) b.push_back((uint8_t)(v >> (i * 8)));
}

static void PutU32(std::vector<uint8_t>& b, uint32_t v) {
    for (int i = 0; i < 4; i++) b.push_back((uint8_t)(v >> (i * 8)));
}

static void PutU64(std::vector<uint8_t>& b, uint64_t v) {
    for (int i = 0; i < 8; i++) b.push_back((uint8_t)(v >> (i * 8)));
}

static bool GetU8(const std::vector<uint8_t>& b, size_t& p, uint8_t& v) {
    if (p + 1 > b.size()) return false;
    v = b[p++];
    return true;
}

static bool GetU16(const std::vector<uint8_t>& b, size_t& p, uint16_t& v) {
    if (p + 2 > b.size()) return false;
    v = 0;
    for (int i = 0; i < 2; i++) v |= (uint16_t)b[p++] << (i * 8);
    return true;
}

static bool GetU32(const std::vector<uint8_t>& b, size_t& p, uint32_t& v) {
    if (p + 4 > b.size()) return false;
    v = 0;
    for (int i = 0; i < 4; i++) v |= (uint32_t)b[p++] << (i * 8);
    return true;
}

static bool GetU64(const std::vector<uint8_t>& b, size_t& p, uint64_t& v) {
    if (p + 8 > b.size()) return false;
    v = 0;
    for (int i = 0; i < 8; i++) v |= (uint64_t)b[p++] << (i * 8);
    return true;
}

bool SaveReplay(const std::string& path, const Replay& r) {
    std::vector<uint8_t> b;
    PutU32(b, REPLAY_MAGIC);
    PutU16(b, r.format);
    PutU32(b, r.gameplayVersion);
    PutU64(b, r.seed);
    PutU8(b, r.difficultyIndex);
    PutU8(b, r.endless);
    PutU32(b, (uint32_t)r.claimedScore);
    PutU32(b, (uint32_t)r.inputs.size());
    b.insert(b.end(), r.inputs.begin(), r.inputs.end());

    FILE* f = fopen(path.c_str(), "wb");
    if (f == nullptr) return false;
    size_t wrote = fwrite(b.data(), 1, b.size(), f);
    fclose(f);
    return wrote == b.size();
}

ReplayLoad LoadReplay(const std::string& path, Replay& out) {
    FILE* f = fopen(path.c_str(), "rb");
    if (f == nullptr) return ReplayLoad::NOT_FOUND;
    std::vector<uint8_t> b;
    uint8_t chunk[4096];
    size_t n;
    while ((n = fread(chunk, 1, sizeof(chunk), f)) > 0) b.insert(b.end(), chunk, chunk + n);
    fclose(f);

    size_t p = 0;
    uint32_t magic = 0, ver = 0, count = 0, score = 0;
    uint16_t format = 0;
    if (!GetU32(b, p, magic)) return ReplayLoad::TRUNCATED;
    if (magic != REPLAY_MAGIC) return ReplayLoad::BAD_MAGIC;
    if (!GetU16(b, p, format)) return ReplayLoad::TRUNCATED;
    if (format != REPLAY_FORMAT) return ReplayLoad::BAD_FORMAT;
    if (!GetU32(b, p, ver)) return ReplayLoad::TRUNCATED;
    if (ver != GAMEPLAY_VERSION) return ReplayLoad::BAD_VERSION;
    if (!GetU64(b, p, out.seed)) return ReplayLoad::TRUNCATED;
    if (!GetU8(b, p, out.difficultyIndex)) return ReplayLoad::TRUNCATED;
    if (!GetU8(b, p, out.endless)) return ReplayLoad::TRUNCATED;
    if (!GetU32(b, p, score)) return ReplayLoad::TRUNCATED;
    if (!GetU32(b, p, count)) return ReplayLoad::TRUNCATED;

    if (out.difficultyIndex >= (uint8_t)DIFF_COUNT) return ReplayLoad::BAD_DIFFICULTY;
    if (count > REPLAY_MAX_TICKS) return ReplayLoad::TOO_LONG;
    if (p + count != b.size()) return ReplayLoad::TRUNCATED;

    out.format          = format;
    out.gameplayVersion = ver;
    out.claimedScore    = (int32_t)score;
    out.inputs.assign(b.begin() + (long)p, b.end());
    return ReplayLoad::OK;
}

const char* ReplayLoadMessage(ReplayLoad r) {
    switch (r) {
        case ReplayLoad::OK:             return "ok";
        case ReplayLoad::NOT_FOUND:      return "file not found";
        case ReplayLoad::BAD_MAGIC:      return "not a replay file";
        case ReplayLoad::BAD_FORMAT:     return "unsupported replay format";
        case ReplayLoad::BAD_VERSION:    return "recorded by a different game version";
        case ReplayLoad::TRUNCATED:      return "truncated or trailing data";
        case ReplayLoad::TOO_LONG:       return "input stream implausibly long";
        case ReplayLoad::BAD_DIFFICULTY: return "unknown difficulty index";
    }
    return "unknown";
}
