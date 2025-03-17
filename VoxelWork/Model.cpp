#include "Model.h"

void Model::render(camera& currentCamera)
{
	
	root.render(currentCamera, transform);
}

Model::Model()
{
    root.attachedPart = "SpiderHead";
    for (int i = 0; i < 4; i++)
    {
        ModelNode* chld = new ModelNode;
        chld->attachedPart = "SpiderLeg";
        root.children.push_back(chld);

        ModelNode* legAttachment = new ModelNode;
        legAttachment->attachedPart = "SpiderLeg";

        chld->children.push_back(legAttachment);

        //Transform for spider Lef
        //chld->localAttachment.transform = glm::translate(chld->localAttachment.transform, glm::vec3(0.f, 0.f, 4.f));
        //chld->localAttachment.transform = glm::rotate(chld->localAttachment.transform,    (float)std::numbers::pi*2.f/3.f, glm::vec3(1.f, 0.f, 0.f));

        //Transform Model
        root.localAttachment.setPosition(glm::vec3(0.f, 0.f, -8.f));

        //Transform legs

        chld->localAttachment.setPosition(glm::vec3(0.f+1.f*i, 0.f, 4.f));
        chld->localAttachment.setRotation(glm::vec3(0.f, -30.f+15.f*i, 0.f));

        legAttachment->localAttachment.setPosition(glm::vec3(0.f, 0.f, 4.f));
        legAttachment->localAttachment.setRotation(glm::vec3(60.f, 0.f, 0.f));
        addAnimation<glm::vec3>("RotateLegRoot", chld->localAttachment.getRotation(), glm::vec3(-60.f, 0.f, 0.f), 5.f,true);

        addAnimation<glm::vec3>("RotateLegAttachment", legAttachment->localAttachment.getRotation(), glm::vec3(140.f, 0.f, 0.f), 7.f);
        playAnimation("RotateLegRoot");
        playAnimation("RotateLegAttachment");
    }
    for (int i = 0; i < 4; i++)
    {
        ModelNode* chld = new ModelNode;
        chld->attachedPart = "SpiderLeg";
        root.children.push_back(chld);

        ModelNode* legAttachment = new ModelNode;
        legAttachment->attachedPart = "SpiderLeg";

        chld->children.push_back(legAttachment);

        //Transform for spider Lef
        //chld->localAttachment.transform = glm::translate(chld->localAttachment.transform, glm::vec3(0.f, 0.f, 4.f));
        //chld->localAttachment.transform = glm::rotate(chld->localAttachment.transform,    (float)std::numbers::pi*2.f/3.f, glm::vec3(1.f, 0.f, 0.f));

        //Transform Model
        root.localAttachment.setPosition(glm::vec3(0.f, 0.f, -8.f));

        //Transform legs

        chld->localAttachment.setPosition(glm::vec3(1.f + 1.f * i, 0.f, 0.f));
        chld->localAttachment.setRotation(glm::vec3(0.f, 180.f+30.f-15.f*i, 0.f));

        legAttachment->localAttachment.setPosition(glm::vec3(0.f, 0.f, 4.f));
        legAttachment->localAttachment.setRotation(glm::vec3(60.f, 0.f, 0.f));
        addAnimation<glm::vec3>("RotateLegRoot", chld->localAttachment.getRotation(), glm::vec3(-60.f, 0.f, 0.f), 5.f,true);

        if (i % 2 == 0)
        {
            addAnimation<glm::vec3>("RotateLegAttachment", legAttachment->localAttachment.getRotation(), glm::vec3(70.f, 0.f, 0.f), 7.f);
        }
        else
        {
            
            addAnimation<glm::vec3>("RotateLegAttachment", legAttachment->localAttachment.getRotation(), glm::vec3(140.f, 0.f, 0.f), 7.f);
        }
        playAnimation("RotateLegRoot");
        playAnimation("RotateLegAttachment");

    }
    
    
}

void Model::tick()
{
	node::tick();
	//Tick Model Parts
	root.tick();
	
	//std::cout << deltaTime << "\n";
	update(deltaTime);
}


