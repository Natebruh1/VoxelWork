#pragma once
#include "ChunkSpace.h"
#include "FastNoiseLite.h"
#include <functional>
#include <thread>
#include <mutex>

#include "LuaCPP/LuaCpp.hpp"
using namespace LuaCpp;
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;

namespace Models
{
    extern node* models;
}

struct chunkToGenAddress
{
    glm::ivec3* start;
    uint16 size;
};

class WorldSpace :
    public ChunkSpace
{
public:
    void generate(glm::ivec3 coords);
    
    void threadLoop(uint16 thread);
    WorldSpace();
    ~WorldSpace();
    virtual void tick() override;
    static WorldSpace* CurrentWorld;
    static node* getScene() { return CurrentWorld->parent; }
    static int currentHand;
    static bool SetBlockWorld(int x,int y, int z,uint32_t id)
    {
        
        if (auto ch = WorldSpace::CurrentWorld->getChunk(floor((float)x / 16.f), floor((float)y / 16.f), floor((float)z / 16.f)); ch)
        {
           int localX = ((x % 16) + 16) % 16;
           int localY = ((y % 16) + 16) % 16;
           int localZ = ((z % 16) + 16) % 16;
           ch->setBlock(localX, localY, localZ, id);
           return true;
        }
        else
        {
            return false; //Chunk not found
        }
    }
    static inline block* GetBlockWorld(int x, int y, int z)
    {
        if (auto ch = WorldSpace::CurrentWorld->getChunk(floor((float)x / 16.f), floor((float)y / 16.f), floor((float)z / 16.f)); ch)
        {
            int localX = ((x % 16) + 16) % 16;
            int localY = ((y % 16) + 16) % 16;
            int localZ = ((z % 16) + 16) % 16;
            return &(ch->getBlock(localX, localY, localZ));
        }
        else
        {
            return nullptr;
        }
    }
private:
    //Height
    FastNoiseLite heightNoise;
    FastNoiseLite carver1; //Chamber Caverns
    FastNoiseLite carver2;
    FastNoiseLite carverCopper;
    const float worldHeight = 48.f;


    std::vector<glm::ivec3> knownChunks;

    
    std::vector<std::function<bool(int x, int y, int z)>> rules; //Lambda to check if the rule criteria has been met
    
    std::vector<int> blockPalette;

    //Thread
    static const int NumWorkers = 12; //Cores on Computer
    std::condition_variable workReady[NumWorkers];
    std::mutex mutex[NumWorkers];
    std::mutex chunkListLock;
    std::vector<bool> haveWork;
    std::vector<bool> workerRunning;
    
    std::thread t[NumWorkers];

    std::vector<glm::ivec3> chunkMarkedGenerate;
    chunkToGenAddress perThreadInfo[NumWorkers];
    void generate(int x, int y, int z);

    // LuaContext
    LuaContext ctx;
    std::shared_ptr<Engine::LuaTNumber> num = std::make_shared<Engine::LuaTNumber>(0);
    //std::shared_ptr<Engine::LuaTTable> chunkData = std::make_shared<Engine::LuaTTable>(0);
};

