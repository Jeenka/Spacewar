#pragma once

#include <cstdint>

struct KeyEvent
{
    enum class Action : uint8_t { Press, Release };
    int key;
    Action action;
};

struct MouseEvent
{
    enum class Action : uint8_t { Move, ButtonPress, ButtonRelease };
    float x, y;
    int button;
    Action action;
};