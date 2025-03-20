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
    
    attachScript("scripts/test_script.lua");
}

void Model::tick()
{
	node::tick();
	//Tick Model Parts
	root.tick();
	
	//std::cout << deltaTime << "\n";
	update(deltaTime);
}

void Model::RegisterFunctions()
{
    //Create metatable for Model FIRST
    if (luaL_newmetatable(L, "ModelMeta")) {  // Ensure it's only created once
        // Set __index to itself
        lua_pushstring(L, "__index");
        lua_pushvalue(L, -2); // Push ModelMeta itself
        lua_settable(L, -3);

        //Register PlayAnimation method
        lua_pushcfunction(L, lua_PlayAnimation);
        lua_setfield(L, -2, "PlayAnimation");

        //Register AddAnimation method
        lua_pushcfunction(L, lua_AddAnimation);
        lua_setfield(L, -2, "AddAnimation");

        //Register Move method
        lua_pushcfunction(L, lua_Move);
        lua_setfield(L, -2, "Move");

        //Register Set Interpolation Method
        lua_pushcfunction(L, lua_SetAnimationInterpolation);
        lua_setfield(L, -2, "SetAnimationInterpolation");
    }


    lua_pop(L, 1); // Pop ModelMeta from stack

    //Now push Model as a userdata and set its metatable
    Model** ptr = static_cast<Model**>(lua_newuserdata(L, sizeof(Model*)));
    *ptr = this;
    
    luaL_getmetatable(L, "ModelMeta"); // Now it exists!
    lua_setmetatable(L, -2);
    
    lua_setglobal(L, "Model"); // Store Model in Lua
    
    //Create Node table
    lua_newtable(L);
    
    // Push the same instance as an upvalue for `Node`
    node** nodePtr = static_cast<node**>(lua_newuserdata(L, sizeof(node*)));
    *nodePtr = this; // `Model` derives from `node`
    
    //Bind `lua_tick` with the upvalue
    lua_pushcclosure(L, lua_tick, 1);
    lua_setfield(L, -2, "tick");
    
    lua_setglobal(L, "Node"); // Set table as global "Node"

}

void Model::CreateLuaModelInstance(lua_State* L)
{
    Model** userdata = static_cast<Model**>(lua_newuserdata(L, sizeof(Model*)));
    *userdata = this;

    luaL_getmetatable(L, "ModelMeta");
    lua_setmetatable(L, -2);

    lua_setglobal(L, "Model");  // Now Lua can access `Model`
}


