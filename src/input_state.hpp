#pragma once
#include <cstdint>

struct InputState {
    bool left  = false;
    bool right = false;
    bool up    = false;
    bool down  = false;
    bool shoot = false;

    uint8_t Pack() const {
        return (uint8_t)((left ? 1 : 0) | (right ? 2 : 0) | (up ? 4 : 0)
                       | (down ? 8 : 0) | (shoot ? 16 : 0));
    }

    static InputState Unpack(uint8_t b) {
        InputState s;
        s.left  = (b & 1)  != 0;
        s.right = (b & 2)  != 0;
        s.up    = (b & 4)  != 0;
        s.down  = (b & 8)  != 0;
        s.shoot = (b & 16) != 0;
        return s;
    }
};
