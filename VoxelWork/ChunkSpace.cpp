#include "ChunkSpace.h"

chunk* ChunkSpace::addChunk(int x, int y, int z)
{
    chunk* retChunk;
    if (auto searchX = chunks.find(x); searchX == chunks.end())
    {
        //X not found
        retChunk = new chunk();
        addChild(*retChunk);
        //Add to chunkmap
        chunks[x][y][z] = *retChunk;
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
            chunks[x][y][z] = *retChunk;
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
                chunks[x][y][z] = *retChunk;
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
        chunks[x][y][z] = chunkToAdd;
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
            chunks[x][y][z] = chunkToAdd;
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
                chunks[x][y][z] = chunkToAdd;
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
                return &chunks[x][y][z];
            }
        }
    }
    return nullptr;
}


