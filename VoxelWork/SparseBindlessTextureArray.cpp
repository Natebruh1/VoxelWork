#include "SparseBindlessTextureArray.h"

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

SparseBindlessTextureArray::SparseBindlessTextureArray() : width(0), height(0), layers(0), internalFormat(GL_RGB), imageFormat(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), filterMin(GL_LINEAR), filterMax(GL_LINEAR)
{
    //Generate Texture
	glGenTextures(1, &ID);

    
}

SparseBindlessTextureArray::~SparseBindlessTextureArray()
{
    //Make non-resident and delete
    if (isResident) {
        glMakeTextureHandleNonResidentARB(bindlessHandle);
    }
    glDeleteTextures(1, &ID);
}

void SparseBindlessTextureArray::Generate(unsigned int width, unsigned int height, unsigned int layers, std::vector<unsigned char*> textures)
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
}

void SparseBindlessTextureArray::allocatePage(GLint mipLevel, GLint xOffset, GLint yOffset, GLint zOffset, GLint width, GLint height, GLint depth)
{
    //Allocate page for sparse texture
    glTexPageCommitmentARB(ID, mipLevel, xOffset, yOffset, zOffset, width, height, depth, GL_TRUE);
}

GLuint64 SparseBindlessTextureArray::getBindlessHandle() const
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


