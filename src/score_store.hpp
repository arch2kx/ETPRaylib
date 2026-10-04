#pragma once
#include <cstdint>
#include <string>

uint64_t ScoreDigest(int score);
int LoadHighScore(const std::string& path);
void SaveHighScore(const std::string& path, int score);
