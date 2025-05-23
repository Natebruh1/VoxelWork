#pragma once
#include "node3D.h"
class camera :
    public node3D
{
public:
    camera();
    virtual void tick() override;
    void processInput(GLFWwindow* const& windowRef,float dt);
    glm::mat4 cameraView;

    void RotateView(float x, float y);
private:
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraDirection;

    
    //Axis Vectors
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;
    glm::vec3 cameraFront;
    
    //Camera Movespeed
    float speed = 2.5f;

    //For looking around
    float yaw=0.f;
    float pitch=0.f;
};

