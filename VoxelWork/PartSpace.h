#pragma once
#include "ChunkSpace.h"

struct chunkLoadTracker { int x = 0; int y = 0; int z = 0; };


class PartSpace :
    public ChunkSpace
{
public:
    PartSpace();
    virtual void serialize(nlohmann::json& data, bool store = false) override;
    void LoadFromLibrary(std::string partName);
    void RegisterToLibrary(std::string partName, std::string fileToLoad);
    static void RenderPart(std::string partName, node3D& Owner, camera& currentCamera);
    static void RenderPartTransformed(std::string partName, node3D& Owner,glm::mat4 transform, camera& currentCamera);
    static std::map<std::string, PartSpace*> partLibrary;
    static void SetBlock(std::string partName, int x, int y, int z, uint32 id);
private:
    

};

