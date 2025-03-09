#pragma once
#include "PartSpace.h"
#include <numbers>
struct ModelNode
{
	std::vector<ModelNode*> children;
	std::string attachedPart;
	node3D localAttachment;
	void render(camera& currentCamera, glm::mat4 runningTransform)
	{
		//Render this
		PartSpace::RenderPartTransformed(attachedPart, localAttachment, runningTransform, currentCamera);
		//Render Children
		for (auto& child : children)
		{
			child->render(currentCamera, localAttachment.transform*runningTransform);
		}
	}
	~ModelNode()
	{
		for (auto& child : children)
		{
			delete child;
		}
	}
};

class Model :public node3D
{
public:
	ModelNode root;
	virtual void render(camera& currentCamera) override;
	Model();
	
};

