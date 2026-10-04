#include "score_store.hpp"
#include "raylib.h"
#include <cstdio>
#include <cstring>

static const uint64_t kSalt = 0x9E3779B97F4A7C15ULL;

uint64_t ScoreDigest(int score) {
    uint64_t h = 1469598103934665603ULL ^ kSalt;
    unsigned char buf[sizeof(int)];
    std::memcpy(buf, &score, sizeof(int));
    for (size_t i = 0; i < sizeof(buf); ++i) {
        h ^= buf[i];
        h *= 1099511628211ULL;
    }
    return h;
}

int LoadHighScore(const std::string& path) {
    if (!FileExists(path.c_str())) return 0;
    char* txt = LoadFileText(path.c_str());
    if (txt == nullptr) return 0;
    int score = 0;
    unsigned long long stored = 0;
    int parsed = std::sscanf(txt, "%d %llu", &score, &stored);
    UnloadFileText(txt);
    if (parsed != 2 || score < 0) return 0;
    if (stored != ScoreDigest(score)) return 0;
    return score;
}

void SaveHighScore(const std::string& path, int score) {
    const char* out = TextFormat("%d %llu", score, (unsigned long long)ScoreDigest(score));
    SaveFileText(path.c_str(), (char*)out);
}
