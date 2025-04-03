#pragma once
#include "BlockLibrary.h";
#include "ModelLibrary.h"
#include "json.hpp"

#include <vector>
// Statics
static BlockLibrary blockLibrary;
static float dt = 0.f;
static float lerp(float a, float b, float t)
{
    return (1.f - t) * a + t * b;
}
