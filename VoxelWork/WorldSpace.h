#pragma once
#include "ChunkSpace.h"
#include "FastNoiseLite.h"
#include "functional"
class WorldSpace :
    public ChunkSpace
{
public:
    void generate(int x, int y, int z);
    WorldSpace();
private:
    //Height
    FastNoiseLite heightNoise;
    const float worldHeight = 48.f;


    std::vector<glm::ivec3> knownChunks;

    std::vector<std::function<bool(int x, int y, int z)>> rules; //Lambda to check if the rule criteria has been met
    std::vector<int> blockPalette;
};

