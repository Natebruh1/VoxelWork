#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "globals.h"

//Temp
#include<iostream>
class camera;
class node
{
public:
	node();
	virtual ~node();
	virtual void tick();
	virtual void render(camera& currentCamera);
	virtual node& addChild(node& newChild);
	virtual node& removeChild(node& childToRemove);
	void setVisible(bool newVis) { visible = newVis; };
protected:
	bool visible = true;
	std::vector<node*> children;
	node* parent=nullptr;

	
};

