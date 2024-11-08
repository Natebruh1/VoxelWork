#include "node.h"
#include "camera.h";
//#include "camera.h";
node::node()
{

}

node::~node()
{
	
	for (int i = 0;i<children.size();i++)
	{
		if (children[0] != nullptr)
		{
			delete children[0];
		}
	}
	children.clear();
	if (parent != nullptr)
	{
		parent->removeChild(*this);
	}
}

void node::tick()
{
	for (auto& child : children)
	{
		if (child->visible) //Only render visible children
		{
			child->tick();
		}

	}
}

void node::render(camera& currentCamera)
{
	for (auto& child : children)
	{
		if (child->visible) //Only render visible children
		{
			child->render(currentCamera);
			
		}
		
	}
}

node& node::addChild(node& newChild)
{
	if (auto search = std::find(children.begin(), children.end(), &newChild); search == children.end()) //If we can't find the child in our tree.
	{
		children.push_back(&newChild);
		//Remove our newChild from its old parent
		if (newChild.parent != nullptr)
		{
			newChild.parent->removeChild(newChild);
		}
		newChild.parent = this;
		
	}
	return newChild;
}

node& node::removeChild(node& childToRemove)
{
	if (auto search = std::find(children.begin(), children.end(), &childToRemove); search != children.end()) //If we can find the child we want to remove
	{
		childToRemove.parent = nullptr;
		children.erase(search); //Erase via the iterator
	}
	return childToRemove;
}
