#pragma once
#include "node3D.h"
#include "chunk.h"
//Needed to hold chunks
#include <map>



class ChunkSpace :
    public node3D
{
public:
    chunk* addChunk(int x, int y, int z);
    chunk* addChunk(int x, int y, int z, chunk& chunkToAdd);
    chunk* getChunk(int x, int y, int z);
private:
    std::map<int, std::map<int, std::map<int, chunk>>> chunks;
};

