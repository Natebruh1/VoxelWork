#include "ResourceManager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Instantiate static variables
std::map<std::string, Texture>                          ResourceManager::Textures;
std::map<std::string, Shader>                           ResourceManager::Shaders;
std::map<std::string, SparseBindlessTextureArray>       ResourceManager::SBTextures;

Shader ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name)
{
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader* ResourceManager::GetShader(std::string name)
{
    return &Shaders[name];
}

Texture ResourceManager::LoadTexture(const char* file, bool alpha, std::string name)
{
    Textures[name] = loadTextureFromFile(file, alpha);
    return Textures[name];
}

Texture* ResourceManager::GetTexture(std::string name)
{
    return &Textures[name];
}

SparseBindlessTextureArray ResourceManager::LoadSBTexArray(std::vector<const char*> files, bool alpha, std::string name)
{
    SBTextures[name] = loadSBTexArrayFromFile(files, alpha);
    return SBTextures[name];
}

SparseBindlessTextureArray* ResourceManager::GetSBTexArray(std::string name)
{
    return &SBTextures[name];
}

void ResourceManager::Clear()
{
    //Delete all shaders	
    for (auto iter : Shaders)
        glDeleteProgram(iter.second.ID);
    //Delete all textures
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second.ID);

    //Delete all Sparse Bindless Texture Arrays
    //It is skipped since they have a destructor which has custom functionality - they should clear as soon as they go out of scope
}

Shader ResourceManager::loadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    try
    {
        // open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        if (gShaderFile != nullptr)
        {
            std::ifstream geometryShaderFile(gShaderFile);
            std::stringstream gShaderStream;
            gShaderStream << geometryShaderFile.rdbuf();
            geometryShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::exception e)
    {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    const char* gShaderCode = geometryCode.c_str();
    // 2. now create shader object from source code
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
    return shader;
}

Texture ResourceManager::loadTextureFromFile(const char* file, bool alpha)
{
    // create texture object
    Texture texture;
    if (alpha)
    {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // load image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, STBI_rgb_alpha);
    // now generate texture
    texture.Generate(width, height, data);
    // and finally free image data
    stbi_image_free(data);
    return texture;
}

SparseBindlessTextureArray ResourceManager::loadSBTexArrayFromFile(std::vector<const char*> files, bool alpha)
{
    SparseBindlessTextureArray texture;
    if (alpha)
    {
        texture.internalFormat = GL_RGBA;
        texture.imageFormat = GL_RGBA;
    }
    // load image
    std::vector<int> width, height, nrChannels;
    std::vector<unsigned char*> textures;
    for (int i = 0; i < files.size();i++)
    {
        textures.push_back( stbi_load(files[i], &width[i], &height[i], &nrChannels[i], STBI_rgb_alpha));
    }
    if (textures.size() == 0) return texture; //Early return so that all data is initialized
    // now generate texture
    texture.Generate(width[0], height[0], textures.size(), textures);
    // and finally free image data
    for (auto& tex : textures)
    {
        stbi_image_free(tex);
    }
    
    return texture;
}
