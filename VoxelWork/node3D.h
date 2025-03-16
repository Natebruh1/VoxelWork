#pragma once
#include "node.h"
class node3D :
    public node
{
public:
    virtual void tick() override;
    virtual ~node3D() override;
    glm::mat4 transform = glm::mat4(1.f);
    glm::vec3& getPosition() { return position;};

    void setPosition(const glm::vec3& newPos)
    {
        position = newPos;
        updateTransform();
    }

    glm::vec3& getRotation() { return rotation; }
    void setRotation(const glm::vec3& newRot)
    {
        rotation = newRot;
        updateTransform();
    }

    glm::vec3& getScale() { return scale; }
    void setScale(const glm::vec3& newScale)
    {
        scale = newScale;
        updateTransform();
    }

protected:
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    //Up Vector for all 3D Objects.
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    void updateTransform()
    {
        
        // Identity Matrix
        transform = glm::mat4(1.0f);

        // Get local transform in T(ransmorn) R(otation) S(cale) order
        glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
        /*glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1));*/
        glm::quat rotationQuat = glm::quat(glm::radians(rotation));
        glm::mat4 R = glm::mat4_cast(rotationQuat);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

        // Multiply: Translation * Rotation * Scale
        transform = T * R * S;
    }

};

