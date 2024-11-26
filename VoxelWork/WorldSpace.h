#pragma once
#include "ChunkSpace.h"
#include "FastNoiseLite.h"
#include <functional>
#include <thread>
#include <mutex>
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
private:
    //Height
    FastNoiseLite heightNoise;
    const float worldHeight = 24.f;


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
};

