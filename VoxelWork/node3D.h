#pragma once
#include "node.h"
class node3D :
    public node
{
public:
    virtual void tick() override;
    glm::mat4 transform = glm::mat4(1.f);
protected:
    glm::vec3 position;
    glm::vec3 rotation;

    //Up Vector for all 3D Objects.
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    

};

