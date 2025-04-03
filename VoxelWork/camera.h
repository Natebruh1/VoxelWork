#pragma once
#include "node3D.h"
class camera :
    public node3D
{
public:
    camera();
    virtual ~camera() override;
    virtual void tick() override;


    void processInput(GLFWwindow* const& windowRef,float dt);
    glm::mat4 cameraView;

    void RotateView(float x, float y);
    float getYaw();
    glm::vec3& getFront();
    //Block Editing
    glm::ivec3 ReturnClosestBlock(int offset=0);
    void DeleteBlock();
    void PlaceBlock();
private:
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection;

    
    //Axis Vectors
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    glm::vec3 cameraFront;
    
    //Camera Movespeed
    float speed = 4.5f;

    //For looking around
    float yaw=0.f;
    float pitch=0.f;


    
};

