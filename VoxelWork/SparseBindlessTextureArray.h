#pragma once
#ifndef SBATEXTURE_H
#define SBATEXTURE_H

#include <vector>
#include <glad/glad.h>
#include <iostream>
#include <algorithm>
#include <GLFW/glfw3.h>
class SparseBindlessTextureArray
{
public:
    SparseBindlessTextureArray(GLint width, GLint height, GLint layers, GLenum internalFormat);
    SparseBindlessTextureArray();
    ~SparseBindlessTextureArray();


    void Generate(unsigned int width, unsigned int height, unsigned int layers, std::vector<unsigned char*> textures);
    void Generate(unsigned int width, unsigned int height, unsigned int layers);
    //For Sparse Arrays
    void allocatePage(GLint mipLevel, GLint xOffset, GLint yOffset, GLint zOffset, GLint width, GLint height, GLint depth);
    //For Bindless handling
    GLuint64& getBindlessHandle();
    void makeResident();
    void makeNonResident();

    //Add image
    void addImage(unsigned char* imageData, GLint layer);
    void addImage(const char* file);

    //Clear Images
    void clear();

    GLint width, height, layers; //Array Texture
    GLenum internalFormat; //Texture
    unsigned int imageFormat; // format of loaded image
    // texture configuration
    unsigned int Wrap_S; // wrapping mode on S axis
    unsigned int Wrap_T; // wrapping mode on T axis
    unsigned int filterMin; // filtering mode if texture pixels < screen pixels
    unsigned int filterMax; // filtering mode if texture pixels > screen pixels
    //To make sure we don't generate this texture multiple times
    bool isGenerated = false;
private:
    GLuint ID; //Texture
    GLuint64 bindlessHandle; //Texture

    bool isResident; //Bindless Texture
    unsigned int committedLayers = 0; //How many layers we have committed to the texture

    
    
};

#endif