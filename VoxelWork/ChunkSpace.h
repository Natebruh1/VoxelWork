#pragma once
#include "node3D.h"
#include "chunk.h"
#include "json.hpp"
//Needed to hold chunks
#include <map>
#include <fstream>
#include <string>
#include <vector>

class LightManager;

class ChunkSpace :
    public node3D
{
public:
    ChunkSpace();
    virtual void tick() override;
    virtual ~ChunkSpace() override;
public:
    chunk* addChunk(int x, int y, int z);
    chunk* addChunk(int x, int y, int z, chunk& chunkToAdd);
    chunk* getChunk(int x, int y, int z);
    virtual void render(camera& currentCamera) override;
    void serialize(nlohmann::json& data, bool store=false);
    void saveToDisc();

    LightManager* getLightsManager() { return lightsManager;};
private:
    std::map<int, std::map<int, std::map<int, chunk*>>> chunks;
    nlohmann::json saveData;
    LightManager* lightsManager;
protected:
    bool withLight = true;
};

