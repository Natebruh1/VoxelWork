#pragma once
#include "node3D.h"
#include "chunk.h"
#include "json.hpp"
#include "filesystem"
//Needed to hold chunks
#include <map>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

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
    virtual void serialize(nlohmann::json& data, bool store=false);
    void saveToDisc();
    void loadFromDisc();
    void updateLoadedRegions(glm::vec3 worldPos, int renderDistance=16);
    LightManager* getLightsManager() { return lightsManager;};


   
private:
    
    nlohmann::json saveData;
    

    
protected:
    std::map<int, std::map<int, std::map<int, chunk*>>> chunks;
    bool withLight = true;
    std::vector<glm::ivec3>loadedRegions;
    std::vector<nlohmann::json> loadedRegionsData;


    LightManager* lightsManager;
};

