#pragma once

struct DifficultySettings {
    float enemySpeedMult;
    float spawnDelayMult;
    float difficultyRamp;
    int   bossScoreThresh;
    bool  hasPhase2;
    bool  hasPhase3;
    // Boss scaling
    int   bossHP;             // Total boss health
    float bossBulletSpeed;    // Boss bullet travel speed (px/s)
    float bossShootCooldown;  // Seconds between boss volleys (lower = faster firing)
    bool  bossAimedShot;      // Hard: adds one bullet aimed straight at the player each volley
    bool  bossBurst;          // Extreme: fires a second volley 0.15s after the first
    // Miss penalty
    int   missPenaltyGrace;   // Number of enemies the player can let through before HP penalty starts
    // Player scaling
    float playerSpeedMult;    // Multiplies player base move speed (360 px/s)
    int   playerBulletCount;  // Number of bullets fired per shot, fanned out if >1
    // Sniper (Haruna) / AR (Junko) phase scaling — none of this scaled with
    // difficulty before; that's why those phases stayed trivial regardless
    // of tier.
    int   sniperHP;
    float sniperCooldown;     // Seconds between aim locks (lower = fires more often)
    int   sniperCount;
    int   arHP;
    float arCooldown;
    int   arCount;
    // Difficulty-exclusive twists (feature toggles, not stat scaling) —
    // same idea as bossAimedShot/bossBurst above, applied to Haruna/Junko.
    bool  sniperSpray;   // Hard: Haruna fires a 3-round unaimed spray between her aimed shots
    bool  arRageEnabled; // Extreme: last remaining Junko unit fires much faster
    bool  bossSpiralEnabled; // Endless late-loops: boss also fires a rotating 360-degree ring
};

// Field order: enemySpeedMult, spawnDelayMult, difficultyRamp, bossScoreThresh, hasPhase2, hasPhase3,
//              bossHP, bossBulletSpeed, bossShootCooldown, bossAimedShot, bossBurst,
//              missPenaltyGrace, playerSpeedMult, playerBulletCount,
//              sniperHP, sniperCooldown, sniperCount, arHP, arCooldown, arCount,
//              sniperSpray, arRageEnabled, bossSpiralEnabled
inline const DifficultySettings DIFF_EASY    = { 0.8f, 1.5f, 0.03f, 300, false, false,  40,
240.0f, 0.9f,  false, false, 0, 1.4f,  1,  3, 2.5f, 3,  3, 2.5f, 5,  false, false, false };
inline const DifficultySettings DIFF_NORMAL  = { 1.2f, 1.0f, 0.05f, 150, true,  true,   50,
280.0f, 0.7f,  false, false, 0, 1.4f,  1,  4, 2.0f, 3,  4, 2.2f, 5,  false, false, false };
inline const DifficultySettings DIFF_HARD    = { 1.6f, 0.7f, 0.08f, 100, true,  true,   70,
320.0f, 0.5f,  true,  false, 3, 1.5f, 3,  5, 1.8f, 4,  4, 2.0f, 5,  true,  false, false };
inline const DifficultySettings DIFF_EXTREME = { 1.8f, 0.4f, 0.15f,  50, true,  true,  100,
370.0f, 0.35f, true,  true,  5, 1.7f,  3,  6, 1.5f, 5,  5, 1.6f, 6,  true,  true,  false };

// Endless: always runs the full boss -> sniper -> AR cycle; Game loops
// back into another boss wave (scaling stats up) instead of ending on
// hasPhase3 completion. See Game::EndCycleOrLoop, which also escalates
// sniperHP/arHP/fire-rate further each loop, and unlocks bossAimedShot,
// bossBurst, and a rotating bossSpiralEnabled ring pattern progressively
// as loopCount (shown as WAVE in the HUD) climbs.
inline const DifficultySettings DIFF_ENDLESS = { 1.5f, 0.8f, 0.06f, 200, true,  true,   60,
300.0f, 0.6f,  true,  false, 2, 1.8f,  5,  7, 1.3f, 5,  6, 1.4f, 6,  true,  true,  false };

// The difficulty table lives here rather than in main.cpp so the replay
// verifier can resolve a replay's difficulty index without pulling in the
// renderer. Appending is safe; reordering invalidates stored replays, so it
// requires a GAMEPLAY_VERSION bump.
inline const char* const DIFF_NAMES[] = { "EASY", "NORMAL", "HARD", "EXTREME", "ENDLESS" };
inline const DifficultySettings DIFF_LIST[] = { DIFF_EASY, DIFF_NORMAL, DIFF_HARD, DIFF_EXTREME, DIFF_ENDLESS };
constexpr int DIFF_COUNT    = 5;
constexpr int ENDLESS_INDEX = 4;
