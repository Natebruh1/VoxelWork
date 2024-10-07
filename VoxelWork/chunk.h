#pragma once
#include "block.h"
#include "node3D.h"
#include "ResourceManager.h"
#include "SparseBindlessTextureArray.h"

#include <stdint.h>

#include <bit>
#include <map>
#include <vector>
#include <optional>
#include <iterator>

#include <iostream>
using uint16 = uint16_t;
using uint32 = uint32_t;
struct chunk_padded {
    //Member Value
    unsigned int data : 18;

    //Constructor using initializer list
    chunk_padded() : data(0b0) {};


    uint32 operator[](uint32_t idx) //Getter only, can't pass bits by ref
    {
        return (data & (0b1 << idx)) != 0;
    };
    chunk_padded& operator=(chunk_padded& rhs) { return rhs; };

    chunk_padded& operator=(uint16& rhs) //For when we want to assign a uint16 to the middle bits
    {
        data = ~(data); //Flip data
        data |= 0b011111111111111110; //Make all the centre bits into 1s
        data = ~(data); //Flip data back (All the centre bits are now 0s, and the padding bits stay the same

        uint32 newData = rhs << 1; //Bit shift the data we're reading in by 1
        data |= newData; //Write in the rhs

        return *this;
    };

    // -- Example usage -- 
    // solidVoxels[0][16]=chunkSolid[0][16]

    chunk_padded& operator()(int leftBit, int rightBit)
    {

        uint32 newData = 0x0000;
        newData |= leftBit; //Write in left bit
        newData << 15; //Shift the left bit left
        newData |= rightBit; //Write in write bit
        data |= newData; //Combine the two values
        return *this;
    }

}; //Chunk Bit Padded (1 bit padding on each side). //Union here since there is a single value (which other members can't overwrite)
//Aliases
using chunk_p = chunk_padded;
const int chunk_size_p2 = 18 * 18;//sizeof(chunk_p[18]); //Single Face of 18*18 data
const int chunk_size_p3 = 18 * 18 * 18;//sizeof(chunk_p[18*18]); //Cuboid of 18*18*18 data, where there is padding for face culling on chunk borders

struct greedyQuad
{
    
    int x; //X
    int w; //Width
    int y; //Y
    int h; //Height
};


struct vertexData
{
    glm::vec3 pos;
    unsigned int axis;
    glm::ivec2 texCoords;
    
};

class chunk :
    public node3D
{
public:
    const int CHUNKSIZE = 16;

    // Chunk
    void deleteBlock(uint16 x, uint16 y, uint16 z);
    void createFullChunk();
    void updateGeom();
    unsigned int prepareRender();
    std::vector<greedyQuad> greedyMeshBinaryPlane(std::map<int, std::vector<uint16>>& inDat);
    std::vector<greedyQuad> greedyMeshBinaryPlane(std::vector<uint16>& inDat);

    //Block Editors
    block& getBlock(uint32 x, uint32 y, uint32 z);
    void setBlock(uint32 x, uint32 y, uint32 z, uint32 id);

    //Utility
    int trailingZeros(const uint16& intRef);
    int trailingOnes(const uint16& intRef);
    //We want to use optional here in case the function fails
    std::optional<uint16> checkedShl(uint16 value, int shift); //Checked Shift-Left

    // Node
    virtual void tick() override;
    virtual void render(camera& currentCamera) override;

    // Con/Decon
    ~chunk();
    chunk();

    
    
    
    std::vector<greedyQuad> chunkQuads;
private: 
    bool geomUpdated = false; //If true then during the next tick we recreate chunk geometry
    // Data
    block* chunkData; //Defines the chunk as created by blocks. Includes extraneous data.
    

    chunk_p axis_col[3*chunk_size_p2]; //We want to split the chunk into 3 separate chunks with data formatted for each axis
    chunk_p mask_col[3 * chunk_size_p2 * 2]; //The same as above except we want one mask for each face rather than each axis
    
    //std::map<uint32,std::map<int,>> data[6]; //Array of 6 (one for each face of a cube) two-depth HashMaps
    std::map<int, std::map<int, std::vector<uint16>>> data;
    //std::map<int, std::vector<std::vector<uint16>>> data;
    std::vector<vertexData> vertices;

    unsigned int chunkVAO;
    unsigned int VBO;

    //Texturing
    std::vector<uint32> textureIndices;
    static std::vector<uint16>knownTextures; //std::vector of BlockID's, the idea is to reduce redundancy
    static SparseBindlessTextureArray ChunkTextures; //Save's memory


};

