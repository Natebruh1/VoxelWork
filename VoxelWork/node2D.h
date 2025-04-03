#pragma once
#include "node.h"
class node2D :
    public node
{
public:
    glm::vec2& getPosition()
    {
        return position;
    }
    glm::vec2& getScale()
    {
        return scale;
    }
    void setPosition(glm::vec2 newPos)
    {
        position = newPos;
    }
    void setScale(glm::vec2 newScale)
    {
        scale = newScale;
    }
private:
    glm::vec2 position = glm::vec2(0.f, 0.f);
    glm::vec2 scale = glm::vec2(1.f, 1.f);
};

