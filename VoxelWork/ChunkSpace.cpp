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
    //transform = glm::scale(transform, glm::vec3(0.5f, 0.1f, 0.5f));
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
    if (chunks.size() > 0)
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
        
        std::vector<uint32_t>* blockToStore = chk->serialize();
        
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

void ChunkSpace::loadFromDisc()
{
    glm::ivec3 targetLoad = glm::ivec3(0, 0, 1);

    std::ifstream csData("ChunkSpace.dat");

    if (csData)
    {
        std::filebuf* raw = csData.rdbuf();

        std::size_t size = raw->pubseekoff(0, csData.end, csData.in);
        raw->pubseekpos(13, csData.in);
        int coordDepth = -1;


        int currentX=9999999;
        int currentY=9999999;
        int currentZ=9999999;
        bool targetChunkLocFound = false;
        while (raw->sgetc() != EOF)
        {
            std::string intAsString = "";
            char ch = raw->sgetc();
            if (ch == '{')
            {
                coordDepth++;
                if ((!(coordDepth == 0 && currentX != targetLoad.x) || !(coordDepth == 1 && currentY != targetLoad.y) || !(coordDepth == 2 && currentZ != targetLoad.z)))
                {
                    raw->snextc();
                    raw->snextc();
                    
                    intAsString = "";
                    //std::cout << raw->sgetc() << std::endl;
                    while (raw->sgetc() != '"')
                    {

                        char c = raw->sgetc();

                        intAsString += c;
                        raw->snextc();

                    }
                    //std::cout << intAsString << std::endl;
                    switch (coordDepth)
                    {
                    case 0:
                        currentX = stoi(intAsString);
                        break;
                    case 1:
                        currentY = stoi(intAsString);
                        break;
                    case 2:
                        currentZ = stoi(intAsString);
                        break;
                    }
                    //std::cout << intAsString << std::endl;
                    intAsString.clear();
                }
                else
                {
                    
                    ch=raw->snextc();
                }

                
            }
            else if (ch == '}')
            {
                coordDepth--;
                ch=raw->snextc();
            }
            else
            {
                ch = raw->snextc();
                
                if (ch == '"')// || ch=='-'
                {
                    intAsString = ""; //-2, -1, 1
                    //if (ch!='-') ch = raw->snextc(); //Increment Character unless already on negative
                    ch = raw->snextc();
                    while (ch != '"')
                    {
                        //char c = raw->sgetc();

                        intAsString += ch;
                        ch=raw->snextc();//Set char counter to next char

                    }
                    //std::cout << intAsString << std::endl;
                    switch (coordDepth)
                    {
                    case 0:
                        currentX = stoi(intAsString);
                        break;
                    case 1:
                        currentY = stoi(intAsString);
                        break;
                    case 2:
                        if (intAsString[0] == ':')
                        {
                            break;
                        }
                        currentZ = stoi(intAsString);
                        break;
                    }
                    //std::cout << intAsString << std::endl;
                    intAsString.clear();
                }
                
            }
            //std::cout << currentX << "," << currentY << "," << currentZ << std::endl;
            if (currentY == 1)
            {
                std::cout << ch;
            }
            if (currentX == targetLoad.x && currentY == targetLoad.y && currentZ == targetLoad.z)
            {
                //Chunk found
                targetChunkLocFound = true;
                break;
            }
        }
        if (targetChunkLocFound)
        {
            std::cout << "TARGET CHUNK FOUND\n";
            char c;
            //col 30796
            c = raw->snextc();
            c = raw->snextc();
            c = raw->snextc();
            c = raw->snextc();
            c = raw->snextc();
            c = raw->snextc();
            c = raw->snextc();
            c = raw->snextc();
            std::string blockID = "";
            std::string runLength = "";
            bool onRunLength = false;
            std::vector<int> finalArr;
            while (raw->sgetc() != ']')
            {
                

                if (onRunLength)
                {
                    if (raw->sgetc() != ',') //Don't want commas in run length
                    {
                        runLength += raw->sgetc();
                    }
                    
                    if (raw->sgetc() == ',')
                    {
                        onRunLength = !onRunLength;
                        int l = stoi(runLength);
                        int id = stoi(blockID);
                        finalArr.push_back(id);
                        finalArr.push_back(l);
                        blockID.clear();
                        runLength.clear();
                    }
                }
                else
                {
                    if (raw->sgetc()!=',') //Don't want commas in blockID
                    {
                        blockID += raw->sgetc();
                    }
                    
                    if (raw->sgetc() == ',')
                    {
                        onRunLength = !onRunLength;
                    }
                }

                c=raw->snextc();
            }
            int off = 0;
            chunk* chnk;
            if (chnk = getChunk(targetLoad.x, targetLoad.y, targetLoad.z); !chnk)
            {
                chnk = addChunk(targetLoad.x, targetLoad.y, targetLoad.z);
            }
            for (int i = 0; i < finalArr.size(); i+=2)
            {
                
                chnk->setBlockArray(finalArr[i], finalArr[i + 1], off);
                
                off += finalArr[i + 1];
                std::cout << "Offset : " << off << std::endl;

            }
            getChunk(targetLoad.x, targetLoad.y, targetLoad.z)->rebuildChunk();
        }



        
        csData.close();
        
    }
    
}

void ChunkSpace::updateLoadedRegions(glm::vec3 worldPos,int renderDistance)
{
    //Convert World Pos to Region Coords
    //Each Region Encompasses 16*16*16 chunks
    glm::ivec3 chunkCoords = floor(worldPos / 16.f);

    //Now find lower and upper bounds for chunks

    glm::ivec3 upperX = chunkCoords + glm::ivec3(renderDistance, 0, 0);
    glm::ivec3 lowerX = chunkCoords + glm::ivec3(-renderDistance, 0, 0);
    glm::ivec3 upperY = chunkCoords + glm::ivec3(0, renderDistance, 0);
    glm::ivec3 lowerY = chunkCoords + glm::ivec3(0, -renderDistance, 0);
    glm::ivec3 upperZ = chunkCoords + glm::ivec3(0, 0, renderDistance);
    glm::ivec3 lowerZ = chunkCoords + glm::ivec3(0, 0, -renderDistance);

    //Now convert to region bounds
    upperX = (upperX / 16) + glm::ivec3(1, 0, 0);
    lowerX = (lowerX / 16) - glm::ivec3(1, 0, 0);
    upperY = (upperY / 16) + glm::ivec3(0, 1, 0);
    lowerY = (lowerY / 16) - glm::ivec3(0, 1, 0);
    upperZ = (upperZ / 16) + glm::ivec3(0, 0, 1);
    lowerZ = (lowerZ / 16) - glm::ivec3(0, 0, 1);

    std::vector<bool> regionFound;
    std::vector<glm::ivec3> regionToAdd;
    regionFound.resize(loadedRegions.size(), false);

    for (int x = lowerX.x; x < upperX.x; x++)
    {
        for (int y = lowerY.y; y < upperY.y; y++)
        {
            for (int z = lowerZ.z; z < upperZ.z; z++)
            {
                if (auto idx=std::find(loadedRegions.begin(), loadedRegions.end(), glm::ivec3(x, y, z));idx != loadedRegions.end())
                {
                    regionFound[idx - loadedRegions.begin()] = true; 
                }
                else
                {
                    //Mark a region to be loaded (and added to loadedRegions)
                    regionToAdd.push_back(glm::ivec3(x, y, z));
                }
            }
        }
    }

    if (!std::filesystem::is_directory("world") || !std::filesystem::exists("world"))
    {
        std::filesystem::create_directory("world");
    }

    bool chunkDelete = false;
    //Now deload the loaded regions, at this point save associated chunks that are marked edited
    for (int i = 0; i < regionFound.size(); i++)
    {
        if (!regionFound[i]) //The region is in the loadedRegions,
        {
            
            std::string filename = "world\\";
            filename += std::to_string(loadedRegions[i].x) + "_" + std::to_string(loadedRegions[i].y) + "_" + std::to_string(loadedRegions[i].z)+".dat";
            

            //Save/Deload
            glm::ivec3 targetRegion = loadedRegions[i]; // Target region to deload;
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 16; y++)
                {
                    for (int z = 0; z < 16; z++)
                    {
                        if (auto chk=getChunk(x + targetRegion.x * 16, y + targetRegion.y * 16, z + targetRegion.z * 16);chk)
                        {
                            if (chk->getEdited()) //Only save chunk if it's edited
                            {

                                std::vector<uint32_t>* blockToStore = chk->serialize();
                                loadedRegionsData[i]["BlockData"][std::to_string(x)][std::to_string(y)][std::to_string(z)] = {};
                                //std::cout << loadedRegionsData[i].dump();
                                

                                for (auto& blk : *blockToStore)
                                {
                                    loadedRegionsData[i]["BlockData"][std::to_string(x)][std::to_string(y)][std::to_string(z)].push_back(blk);
                                }
                            }
                            //Delete chunk from children
                            delete chk;
                            chunkDelete = true;
                        }
                        

                        
                    }
                }
            }
            

            std::ofstream f;
            f.open(filename);
            f << loadedRegionsData[i];
            f.close();
            //Clear the data
            
            loadedRegions.erase(loadedRegions.begin()+i);
            loadedRegionsData.erase(loadedRegionsData.begin() + i);


            
        }

    }
    
    
    
    //Find all the new regions and create files/load them into ram
    
    for (auto& reg : regionToAdd)
    {
        std::string filename = "world\\";
        filename += std::to_string(reg.x)+"_" + std::to_string(reg.y) + "_" + std::to_string(reg.z) + ".dat";
        std::fstream f;
        f.open(filename, std::fstream::in | std::fstream::out);
        if (!f) f.open(filename, std::fstream::in | std::fstream::out | std::fstream::trunc); //Create the file

        nlohmann::json regionData;
        try
        {
            regionData = nlohmann::json::parse(f);
        }
        catch (std::exception e)
        {
            
            //File couldn't be loaded, create a new one
            for (int x = 0; x < 16; x++)
            {
                for (int y = 0; y < 16; y++)
                {
                    for (int z = 0; z < 16; z++)
                    {
                        regionData["BlockData"][std::to_string(x)][std::to_string(y)][std::to_string(z)].push_back(0);
                        
                    }
                }
            }
            
        }
        f.close();
        std::ofstream outFile;
        outFile.open(filename);
        outFile << regionData; //Outpute the regionData to the text file
        outFile.close();
        loadedRegions.push_back(reg);
        loadedRegionsData.push_back(regionData);
        
    }

}


