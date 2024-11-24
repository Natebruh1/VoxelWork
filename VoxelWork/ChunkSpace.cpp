#include "ChunkSpace.h"
#include "camera.h"
#include "LightManager.h"
ChunkSpace::ChunkSpace()
{
    if (withLight)
    {
        lightsManager = new LightManager();
        lightsManager->setWorldSpace(this);
    }
    
}
void ChunkSpace::tick()
{
    if (withLight)
    {
        lightsManager->tick();
    }
    node::tick();
    if (withLight)
    {
        lightsManager->lightTick();
    }
}
ChunkSpace::~ChunkSpace()
{
    chunks.clear();
    saveData.clear();
   // std::cout << "Join:00000\n";
    
}
chunk* ChunkSpace::addChunk(int x, int y, int z)
{
    chunk* retChunk;
    if (auto searchX = chunks.find(x); searchX == chunks.end())
    {
        //X not found
        retChunk = new chunk();
        addChild(*retChunk);
        //Add to chunkmap
        chunks[x][y][z] = retChunk;
        retChunk->setChunkCoords(x, y, z);
        retChunk->alertNeighbourToUpdate();
        return retChunk;
    }
    else
    {
        //We have this x value
        if (auto searchY = chunks[x].find(y); searchY == chunks[x].end())
        {
            //Y not found
            retChunk = new chunk();
            addChild(*retChunk);
            //Add to chunkmap
            chunks[x][y][z] = retChunk;
            retChunk->setChunkCoords(x, y, z);
            retChunk->alertNeighbourToUpdate();
            return retChunk;
        }
        else
        {
            if (auto searchZ = chunks[x][y].find(z); searchZ == chunks[x][y].end())
            {
                //z not found
                retChunk = new chunk();
                addChild(*retChunk);
                //Add to chunkmap
                chunks[x][y][z] = retChunk;
                retChunk->setChunkCoords(x, y, z);
                retChunk->alertNeighbourToUpdate();
                return retChunk;
            }
        }
    }

    
    return nullptr;
}

chunk* ChunkSpace::addChunk(int x, int y, int z, chunk& chunkToAdd)
{
    if (auto searchX = chunks.find(x); searchX == chunks.end())
    {
        //X not found
        addChild(chunkToAdd);
        
        chunks[x][y][z]=&chunkToAdd;
        
        chunkToAdd.setChunkCoords(x, y, z);
        chunkToAdd.alertNeighbourToUpdate();
        return &chunkToAdd;
    }
    else
    {
        //We have this x value
        if (auto searchY = chunks[x].find(y); searchY == chunks[x].end())
        {
            //Y not found
            addChild(chunkToAdd);
            chunks[x][y][z] = &chunkToAdd;
            chunkToAdd.setChunkCoords(x, y, z);
            chunkToAdd.alertNeighbourToUpdate();
            return &chunkToAdd;
        }
        else
        {
            if (auto searchZ = chunks[x][y].find(z); searchZ == chunks[x][y].end())
            {
                //z not found
                addChild(chunkToAdd);
                chunks[x][y][z] = &chunkToAdd;
                chunkToAdd.setChunkCoords(x, y, z);
                chunkToAdd.alertNeighbourToUpdate();
                return &chunkToAdd;
            }
        }
    }
    return nullptr;
}

chunk* ChunkSpace::getChunk(int x, int y, int z)
{
    if (auto searchX = chunks.find(x); searchX != chunks.end())
    {
        if (auto searchY = chunks[x].find(y); searchY != chunks[x].end())
        {
            if (auto searchZ = chunks[x][y].find(z); searchZ != chunks[x][y].end())
            {
                return chunks[x][y][z];
            }
        }
    }
    return nullptr;
}

void ChunkSpace::render(camera& currentCamera)
{
    //Below is a direction culler (not fully tested) it should cull chunks you can't see
    //for (auto& x : chunks)
    //{
    //    for (auto& y : x.second)
    //    {
    //        for (auto& z : y.second)
    //        {
    //            //Get the dot of cameraFront and chunkCorner-cameraPos to find if we're facing it
    //            if (glm::dot(currentCamera.getFront(), (glm::vec3(x.first,y.first,z.first) * 16.f) - currentCamera.getPosition()) < 0)
    //            {
    //                z.second.setVisible(false);
    //                
    //            }
    //            else
    //            {
    //                z.second.setVisible(true);
    //            }
    //        }
    //    }
    //}
        


    node::render(currentCamera);
}

void ChunkSpace::serialize(nlohmann::json& data, bool store)
{
    
    for (auto& child : children)
    {
        chunk* chk = reinterpret_cast<chunk*>(child);
        
        std::vector<uint32_t>* blockToStore = chk->serialize(data);
        
        for (auto& blk :  *blockToStore)
        {
            data["BlockData"][std::to_string(chk->getChunkCoords().x)][std::to_string(chk->getChunkCoords().y)][std::to_string(chk->getChunkCoords().z)].push_back(blk);

        }
        blockToStore->clear();
        
    }
    

    if (store) //Store data
    {
        saveData = data;
    }
        
}

void ChunkSpace::saveToDisc()
{
    //Temp Save
    std::ofstream myfile("ChunkSpace.dat");
    if (myfile.is_open())
    {
        myfile << saveData;
        myfile.close();
    }
    
}


