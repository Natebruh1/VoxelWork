#pragma once
#include "ChunkSpace.h"
class PartSpace :
    public ChunkSpace
{
public:
    PartSpace();
    void LoadFromLibrary(std::string partName);
    void RegisterToLibrary(std::string partName, std::string fileToLoad);
    static void RenderPart(std::string partName, node3D& Owner, camera& currentCamera);
    static void RenderPartTransformed(std::string partName, node3D& Owner,glm::mat4 transform, camera& currentCamera);
    static std::map<std::string, PartSpace*> partLibrary;
    static void SetBlock(std::string partName, int x, int y, int z, uint32 id);
private:
    

};

