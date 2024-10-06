#include "SparseBindlessTextureArray.h"

#include "stb_image.h"

SparseBindlessTextureArray::SparseBindlessTextureArray(GLint width, GLint height, GLint layers, GLenum internalFormat)
    : width(width), height(height), layers(layers), internalFormat(internalFormat), isResident(false)
{
    glGenTextures(1, &ID);
    //Bind
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    //Allocate storage for the texture
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, width, height, layers);

    //Enable Sparse Textures
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

    //Set Texture wrap/filter
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Get the bindless handle for the texture (to allow for lots of units)
    bindlessHandle = glGetTextureHandleARB(ID);

    //Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

}

SparseBindlessTextureArray::SparseBindlessTextureArray() : width(16), height(16), layers(0), internalFormat(GL_RGBA8), imageFormat(GL_RGBA8), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), filterMin(GL_LINEAR), filterMax(GL_LINEAR), isResident(false)
{
    

    
}

SparseBindlessTextureArray::~SparseBindlessTextureArray()
{
    //Make non-resident and delete
    if (isResident) {
        glMakeTextureHandleNonResidentARB(bindlessHandle);
    }
    glDeleteTextures(1, &ID);
}

void SparseBindlessTextureArray::Generate(unsigned int width, unsigned int height, unsigned int layers, std::vector<unsigned char*> textures) //Out of Date
{
    this->width = width;
    this->height = height;
    //Bind and Create Texture
    glGenTextures(1, &ID);
    //Bind
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    //Allocate storage for the texture
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, internalFormat, width, height, layers+128);

    //Enable Sparse Textures
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

    //Set Texture wrap/filter
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Get the bindless handle for the texture (to allow for lots of units)
    bindlessHandle = glGetTextureHandleARB(ID);

    //Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    //Now add the images
    for (int i=0;i<textures.size();i++)
    {
        addImage(textures[i], i);
    }
    isGenerated = true;
}

void SparseBindlessTextureArray::Generate(unsigned int width, unsigned int height, unsigned int layers)
{
    unsigned int pageMinXY = 128;
    
    width = std::max(width, pageMinXY);
    height = std::max(height, pageMinXY);
    this->width = width;
    this->height = height;
    this->layers = layers;
    //Bind and Create Texture
    glGenTextures(1, &ID);
    //Bind
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    
    
    
    //Enable Sparse Textures
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
    
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, 0); // Ensure correct page size index
       
    
    //Set Texture wrap/filter
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);
  
    
    
    
    //Allocate storage for the texture
    //glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, layers);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 128, 128, layers);
    
    //IMPORTANT YOU CAN'T DYNAMICALLY INCREASE LAYERS WITHOUT A RESTART SO THERE SHOULD BE LAYERS EQUAL TO AT LEAST 6*TOTAL BLOCKS IN LIBRARY AT STARTUP
    



    

    //Get the bindless handle for the texture (to allow for lots of units)
    
    bindlessHandle = glGetTextureHandleARB(ID);
    
    if (bindlessHandle == 0) {
        std::cout << "N Bindless Handle Created"<<std::endl;
    }
    //Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    isGenerated = true;
    /*for (int i = 0; i < layers; i++)
    {
        allocatePage(1, 0, 0, i, width, height, 1);
    }*/
    
}

void SparseBindlessTextureArray::allocatePage(GLint mipLevel, GLint xOffset, GLint yOffset, GLint zOffset, GLint width, GLint height, GLint depth)
{
    
    //Bind Texture and make Resident
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    
    makeResident();
    
    
   
    //Allocate page for sparse texture
    glTexPageCommitmentARB(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, zOffset, width, height, depth, GL_TRUE);
    //glTexPageCommitmentARB(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 128, 128, 0, GL_TRUE);
    if (auto err = glGetError(); err != GL_NO_ERROR) std::cerr << "OpenGL error: " << std::hex << err << std::endl; //Clean error detection line (limited info given however)

    
    //Unbind Texture
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

GLuint64& SparseBindlessTextureArray::getBindlessHandle()
{
    return bindlessHandle;
}

void SparseBindlessTextureArray::makeResident()
{
    if (!isResident) {
        glMakeTextureHandleResidentARB(bindlessHandle);
        isResident = true;
    }
}

void SparseBindlessTextureArray::makeNonResident()
{
    if (isResident) {
        glMakeTextureHandleNonResidentARB(bindlessHandle);
        isResident = false;
    }
}

void SparseBindlessTextureArray::addImage(unsigned char* imageData, GLint layer)
{
    //Allocate a new page
    allocatePage(0, 0, 0, layer, width, height, 1);

    //Bind
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    //Add the image
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, this->imageFormat, GL_UNSIGNED_BYTE, imageData);
    //Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

}

void SparseBindlessTextureArray::addImage(const char* file)
{
    if (auto err = glGetError(); err != GL_NO_ERROR) std::cerr << "OpenGL error: " << std::hex << err << std::endl; //Clean error detection line (limited info given however)
    
    


    int nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, STBI_rgb_alpha);
    
    //Allocate a new page
    
    allocatePage(0, 0, 0, committedLayers, width, height, 1);
    
    //Bind
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

    


    //Add the image
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, committedLayers, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    if (auto err = glGetError(); err != GL_NO_ERROR) std::cerr << "OpenGL error: " << std::hex << err << std::endl; //Clean error detection line (limited info given however)
    //Unbind
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    committedLayers += 1;
    
    stbi_image_free(data);
}

void SparseBindlessTextureArray::clear()
{
    makeNonResident();

    // Optionally clear the committed pages in the sparse array
    glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
    //for (int layer = 0; layer < 256; ++layer) {
    //    for (int x = 0; x < 1024; x += PAGE_SIZE_X) {
    //        for (int y = 0; y < 1024; y += PAGE_SIZE_Y) {
    //            // Mark pages as uncommitted (GL_FALSE)
    //            glTexturePageCommitmentARB(ID, 0, x, y, layer, PAGE_SIZE_X, PAGE_SIZE_Y, 1, GL_FALSE);
    //        }
    //    }
    //}
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    // Delete the texture to free memory
    glDeleteTextures(1, &ID);
}


