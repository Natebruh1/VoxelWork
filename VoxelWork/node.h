#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#include "globals.h"

#include <algorithm>
template <typename Key, typename Value, std::size_t size>
struct Map
{
	std::array<std::pair<Key, Value>, size>data;
	[[nodiscard]] constexpr Value at(const Key& key) const
	{
		const auto itr =
			std::find_if(begin(data), end(data),
				[&key](const auto& v) {return v.first == key; });
		if (itr != end(data))
		{
			return itr->second;
		}
		else
		{
			throw std::range_error("Index out of Bounds");
		}
	}
};



//Temp
#include<iostream>
class camera;
class LightManager;
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

