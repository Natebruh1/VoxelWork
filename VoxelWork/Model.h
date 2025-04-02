#pragma once
#include "PartSpace.h"
#include <numbers>
#include "WorldSpace.h"
struct ModelNode
{
	std::vector<ModelNode*> children;
	std::string attachedPart;
	node3D localAttachment;
	void render(camera& currentCamera, glm::mat4 runningTransform)
	{
        //std::cout << localAttachment.transform[3][0] <<","<< localAttachment.transform[3][1] << "," << localAttachment.transform[3][2] << "," << localAttachment.transform[3][3] << "," << "\n";
		//Render this
		PartSpace::RenderPartTransformed(attachedPart, localAttachment, runningTransform, currentCamera);
		//Render Children
		for (auto& child : children)
		{
            glm::mat4 childWorldTransform = runningTransform * localAttachment.transform;
            child->render(currentCamera, childWorldTransform);
		}
	}
	~ModelNode()
	{
		for (auto& child : children)
		{
			delete child;
		}
	}
    void tick()
    {
        
        localAttachment.tick();
        for (auto& child : children)
        {
            //child->localAttachment.setRotation(child->localAttachment.getRotation()+ glm::vec3(0.f, 90.f, 0.f)*0.01f);
            //child->localAttachment.transform = localAttachment.transform * child->localAttachment.transform;
            //child->localAttachment.transform = localAttachment.transform * glm::translate(glm::mat4(1.0f), child->localAttachment.getPosition());
            child->tick();
        }
    }
};

class Model :public node3D
{
public:
	ModelNode root;
	virtual void render(camera& currentCamera) override;
	Model();
    virtual void tick() override;

    struct AnimationBase
    {
        int luaFunctionRef = LUA_REFNIL; //Reference to Lua function for custom interpolation
        virtual void setLuaFunction(int funcRef) { luaFunctionRef = funcRef; }
        virtual void update(float deltaTime, lua_State* L) = 0;
        virtual bool isFinished() const = 0;
        virtual ~AnimationBase() {};
        
    };

    template <typename propType>
    struct Animation : public AnimationBase
    {
        std::function<void(const propType&)> setter;
        std::function<propType()> getter;
        propType startValue{};
        propType endValue;
        float duration;
        float elapsed = 0.0f;
        bool withRelative = false;
        //Animation Constructor
        Animation(std::function<void(const propType&)> set, std::function<propType()> get,
            propType start, propType end, float dur,bool withR)
            : setter(set), getter(get), startValue(start), endValue(end + (withR ? start : propType{})), duration(dur), withRelative(withR)
        {
            /*if (withR)
            {
                endValue = startValue + endValue;
            }*/
        }

        void update(float deltaTime, lua_State* L) override
        {
            if (elapsed >= duration) return;
            elapsed += deltaTime;
            float t = std::min(elapsed / duration, 1.0f);
            //Linearly set animation value - could change to different inherited types (i.e slerp, cubic, quartic, squared, sine)

            propType interpolatedValue;
            if (luaFunctionRef != LUA_REFNIL)
            {
                lua_rawgeti(L, LUA_REGISTRYINDEX, luaFunctionRef); // Get the Lua function
                lua_pushnumber(L, t); // Only pass 't' for generic use

                if (lua_pcall(L, 1, 1, 0) == LUA_OK)
                {
                    //Call these in compile time for efficiency
                    if constexpr (std::is_same_v<propType, double> or std::is_same_v<propType,float>) //Double or float only
                    {
                        if (lua_isnumber(L, -1))
                            interpolatedValue = static_cast<propType>(lua_tonumber(L, -1));
                    }
                    else if constexpr (std::is_same_v<propType, glm::vec3>) //Vector Only
                    {
                        if (lua_istable(L, -1))
                        {
                            lua_rawgeti(L, -1, 1);
                            lua_rawgeti(L, -2, 2);
                            lua_rawgeti(L, -3, 3);

                            interpolatedValue.x = static_cast<float>(lua_tonumber(L, -3));
                            interpolatedValue.y = static_cast<float>(lua_tonumber(L, -2));
                            interpolatedValue.z = static_cast<float>(lua_tonumber(L, -1));

                            lua_pop(L, 4); // Remove table and values
                        }
                    }
                }
                else
                {
                    std::cerr << "Error calling Lua interpolation function: " << lua_tostring(L, -1) << std::endl;
                    lua_pop(L, 1);
                    interpolatedValue = startValue + (endValue - startValue) * t; // Default Linear Interpolation
                }
            }
            else
            {
                interpolatedValue = startValue + (endValue - startValue) * t;
            }

            setter(interpolatedValue);
            

            /*std::cout << "Start: " << startValue[0]
                << " End: " << endValue[0]
                << " t: " << t
                << " New: " << lerp(startValue, endValue, t)[0] << "\n";*/
        }
        
        bool isFinished() const override
        {
            return elapsed >= duration;
        }

        static propType lerp(const propType& a, const propType& b, float t)
        {
            return a + (b - a) * t; // Works for float, glm::vec3, etc.
        }
        

    };

    std::unordered_map<std::string, std::vector<std::unique_ptr<AnimationBase>>> animations;

    template <typename propType>
    void addAnimation(std::string animName, propType& trackedProperty, propType finalVal, float animationLength, bool withRelative=false)
    {
        

        animations[animName].push_back(std::make_unique<Animation<propType>>(
            [&trackedProperty](const propType& value)
            {
                trackedProperty = value;
            }, // Setter function
            [&trackedProperty]() -> propType //Lambda return type notation
            { return trackedProperty; }, // Getter function
            trackedProperty, finalVal, animationLength,
            withRelative
        ));
        
    }

    void playAnimation(std::string animName)
    {
        if (animations.find(animName) != animations.end())
        {
            for (auto& anim : animations[animName])
            {
                std::cout << "Starting animation: " << animName << std::endl;
                activeAnimations.push_back(std::move(anim)); // Move instead of copy
            }

            //Remove emptied unique_ptrs from the original container
            animations[animName].erase(
                std::remove_if(animations[animName].begin(), animations[animName].end(),
                    [](const std::unique_ptr<AnimationBase>& anim) { return anim == nullptr; }),
                animations[animName].end());
        }
    }

    void update(float deltaTime)
    {
        for (auto& anim : activeAnimations)
        {
            anim->update(deltaTime,L);
        }
        activeAnimations.erase(std::remove_if(activeAnimations.begin(), activeAnimations.end(),
            [](const std::unique_ptr<AnimationBase>& anim) { return anim->isFinished(); }),
            activeAnimations.end());
    }
    
protected:
    virtual void RegisterFunctions() override;
    void CreateLuaModelInstance(lua_State* L);
private:
    std::vector<std::unique_ptr<AnimationBase>> activeAnimations; //Unique pointer to underived type

    //Lua stuff
    static int lua_PlayAnimation(lua_State* L)
    {
        //Get instance from upvalue
        Model* model = *static_cast<Model**>(luaL_checkudata(L, 1, "ModelMeta"));
        if (!model)
        {
            return luaL_error(L, "Invalid Model instance");
        }

        const char* animName = luaL_checkstring(L, 2);
        model->playAnimation(animName);

        return 0;
    }
    static int lua_Move(lua_State* L)
    {
        //Get instance from metatable
        Model* model = *static_cast<Model**>(luaL_checkudata(L, 1, "ModelMeta"));
        if (!model)
        {
            return luaL_error(L, "Invalid Model instance");
        }
        //First Argument is Model object
        double xChange = luaL_checknumber(L, 2); //2nd argument
        double yChange = luaL_checknumber(L, 3); //3rd argument
        double zChange = luaL_checknumber(L, 4); //4th argument

        model->setPosition(model->getPosition() + glm::vec3(xChange, yChange, zChange));
        return 0; //Number of return arguments
    }
    static int lua_AddAnimation(lua_State* L)
    {
        //Get instance from upvalue
        Model* model = *static_cast<Model**>(luaL_checkudata(L, 1, "ModelMeta"));
        if (!model)
        {
            return luaL_error(L, "Invalid Model instance");
        }

        const char* animName = luaL_checkstring(L, 2);
        const char* animatedProperty= luaL_checkstring(L, 3);
        
        if (std::string(animatedProperty) == std::string("lua"))
        {
            const char* propertyName = luaL_checkstring(L, 4);
            lua_getglobal(L,propertyName); //push property to the top of the stack
            double propertyValue = (double)lua_tonumber(L, -1); //Get the top of the stack
            
            double finalValue = luaL_checknumber(L, 5);
            double time = luaL_checknumber(L, 6);
            model->addAnimation<double>(std::string(animName), propertyValue, finalValue, time);
        }
        if (std::string(animatedProperty) == std::string("rotation"))
        {
            std::vector<double> rotAmount = extractLuaArray(L, 4, double{});
 
            double t = luaL_checknumber(L, 5); //Duration

            auto partID = extractLuaArray(L, 6, int{});
            ModelNode* targetedPart = getPartFromID(model, partID);
           

            model->addAnimation<glm::vec3>(std::string(animName), targetedPart->localAttachment.getRotation(), glm::vec3(rotAmount[0],rotAmount[1],rotAmount[2]), t);
        }
        if (std::string(animatedProperty) == std::string("position"))
        {
            std::vector<double> rotAmount = extractLuaArray(L, 4, double{});

            double t = luaL_checknumber(L, 5); //Duration

            auto partID = extractLuaArray(L, 6, int{});
            ModelNode* targetedPart = getPartFromID(model, partID);


            model->addAnimation<glm::vec3>(std::string(animName), targetedPart->localAttachment.getPosition(), glm::vec3(rotAmount[0], rotAmount[1], rotAmount[2]), t);
        }
        if (std::string(animatedProperty) == std::string("scale"))
        {
            std::vector<double> rotAmount = extractLuaArray(L, 4, double{});

            double t = luaL_checknumber(L, 5); //Duration

            auto partID = extractLuaArray(L, 6, int{});
            ModelNode* targetedPart = getPartFromID(model, partID);


            model->addAnimation<glm::vec3>(std::string(animName), targetedPart->localAttachment.getScale(), glm::vec3(rotAmount[0], rotAmount[1], rotAmount[2]), t);
        }
        //model->playAnimation(animName);

        return 0;
    }

    static int lua_SetAnimationInterpolation(lua_State* L)
    {
        Model* model = *static_cast<Model**>(luaL_checkudata(L, 1, "ModelMeta"));
        if (!model)
        {
            return luaL_error(L, "Invalid Model instance");
        }

        const char* animName = luaL_checkstring(L, 2);

        if (!lua_isfunction(L, 3))
        {
            return luaL_error(L, "Expected a Lua function as the third argument");
        }

        lua_pushvalue(L, 3); // Copy the function to the top of the stack
        int funcRef = luaL_ref(L, LUA_REGISTRYINDEX); //Store function reference

        auto it = model->animations.find(animName);
        if (it != model->animations.end())
        {
            for (auto& anim : it->second)
            {
                anim->setLuaFunction(funcRef); 
            }
        }

        return 0;
    }

    static int lua_SpawnModel(lua_State* L)
    {
        if (!lua_isstring(L, 1))
        {
            return luaL_error(L, "Expected a string argument for model name");
        }

        // Get the string argument (model name)
        const char* modelName = lua_tostring(L, 1);

        // Call the existing C++ function to create a new model
        Model* newModel = ModelLibrary::LoadModel(std::string(modelName));  // Pass model name
        
        if (Models::models)
            Models::models->addChild(newModel);
        if (!newModel)
        {
            return luaL_error(L, "Failed to spawn model");
        }

        // Push the new model as a Lua userdata
        Model** ptr = static_cast<Model**>(lua_newuserdata(L, sizeof(Model*)));
        *ptr = newModel;

        //Set the metatable so it behaves like a Model object in Lua
        luaL_getmetatable(L, "ModelMeta");
        lua_setmetatable(L, -2);

        return 1;  // Returns 1 value
    }




    // LUA HELPER FUNCTIONS //


    static std::vector<int> extractLuaArray(lua_State* L, int argNum,int)
    {
        std::vector<int> extraValues = {};
        if (lua_istable(L, argNum))  //
        {
            lua_pushnil(L);  // Push first key
            while (lua_next(L, argNum) != 0)
            {
                if (lua_isinteger(L, -1))
                {
                    extraValues.push_back(lua_tointeger(L, -1));
                }
                lua_pop(L, 1);  // Remove value, keep key for next iteration
            }
        }
        return extraValues;
    }
    static std::vector<double> extractLuaArray(lua_State* L, int argNum,double)
    {
        std::vector<double> extraValues = {};
        
        if (lua_istable(L, argNum))  // If the 8th argument is a table
        {
            lua_pushnil(L);  // Push first key
            while (lua_next(L, argNum) != 0)
            {
                if (lua_isnumber(L, -1))
                {
                    extraValues.push_back(lua_tonumber(L, -1));
                }
                lua_pop(L, 1);  // Remove value, keep key for next iteration
            }
        }
        return extraValues;
    }
    static ModelNode* getPartFromID(Model* m, std::vector<int> id)
    {
        ModelNode* start = &(m->root);
        for (auto& i : id)
        {
            if (start->children.size() > i)
            {
                start = start->children[i];
            }
            else
            {
                return nullptr;
            }
        }
        return start;
    }
};

