#include "Model.h"

void Model::render(camera& currentCamera)
{
	
	root.render(currentCamera, transform);
}

Model::Model()
{
	root.attachedPart = "Part";
	ModelNode* chld = new ModelNode;
	chld->attachedPart = "Part";
	root.children.push_back(chld);

	//Transform for spider Lef
	//chld->localAttachment.transform = glm::translate(chld->localAttachment.transform, glm::vec3(0.f, 0.f, 4.f));
	//chld->localAttachment.transform = glm::rotate(chld->localAttachment.transform,	(float)std::numbers::pi*2.f/3.f, glm::vec3(1.f, 0.f, 0.f));
	chld->localAttachment.setPosition(glm::vec3(0.f, 0.f, 4.f));
	chld->localAttachment.setRotation(glm::vec3(120.f, 0.f, 0.f));
	addAnimation<glm::vec3>("RotateRoot", root.localAttachment.getRotation(), glm::vec3(-60.f, 0.f, 0.f), 5.f);
	
	addAnimation<glm::vec3>("Rotate", chld->localAttachment.getRotation(), glm::vec3(140.f, 0.f, 0.f), 5.f);
	playAnimation("Rotate");
	playAnimation("RotateRoot");
}

void Model::tick()
{
	node::tick();
	//Tick Model Parts
	root.tick();
	
	//std::cout << deltaTime << "\n";
	update(deltaTime);
}


