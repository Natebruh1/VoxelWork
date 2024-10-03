#pragma once
#ifndef SBATEXTURE_H
#define SBATEXTURE_H

#include <vector>
#include <glad/glad.h>
class SparseBindlessTextureArray
{
public:
    SparseBindlessTextureArray(GLint width, GLint height, GLint layers, GLenum internalFormat);
    SparseBindlessTextureArray();
    ~SparseBindlessTextureArray();


    void Generate(unsigned int width, unsigned int height, unsigned int layers, std::vector<unsigned char*> textures);
    //For Sparse Arrays
    void allocatePage(GLint mipLevel, GLint xOffset, GLint yOffset, GLint zOffset, GLint width, GLint height, GLint depth);
    //For Bindless handling
    GLuint64 getBindlessHandle() const;
    void makeResident();
    void makeNonResident();

    //Add image
    void addImage(unsigned char* imageData, GLint layer);

    GLint width, height, layers; //Array Texture
    GLenum internalFormat; //Texture
    unsigned int imageFormat; // format of loaded image
    // texture configuration
    unsigned int Wrap_S; // wrapping mode on S axis
    unsigned int Wrap_T; // wrapping mode on T axis
    unsigned int filterMin; // filtering mode if texture pixels < screen pixels
    unsigned int filterMax; // filtering mode if texture pixels > screen pixels
private:
    GLuint ID; //Texture
    GLuint64 bindlessHandle; //Texture
    
    bool isResident; //Bindless Texture

    
    
};

#endif