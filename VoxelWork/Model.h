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
        virtual void update(float deltaTime) = 0;
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
        //Animation Constructor
        Animation(std::function<void(const propType&)> set, std::function<propType()> get,
            propType start, propType end, float dur)
            : setter(set), getter(get), startValue(start), endValue(end), duration(dur) {}

        void update(float deltaTime) override
        {
            if (elapsed >= duration) return;
            elapsed += deltaTime;
            float t = std::min(elapsed / duration, 1.0f);
            //Linearly set animation value - could change to different inherited types (i.e slerp, cubic, quartic, squared, sine)
            setter(lerp(startValue, endValue, t));

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
    void addAnimation(std::string animName, propType& trackedProperty, propType finalVal, float animationLength)
    {
        

        animations[animName].push_back(std::make_unique<Animation<propType>>(
            [&trackedProperty](const propType& value)
            {
                trackedProperty = value;
            }, // Setter function
            [&trackedProperty]() -> propType //Lambda return type notation
            { return trackedProperty; }, // Getter function
            trackedProperty, finalVal, animationLength
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

            // Remove emptied unique_ptrs from the original container
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
            anim->update(deltaTime);
        }
        activeAnimations.erase(std::remove_if(activeAnimations.begin(), activeAnimations.end(),
            [](const std::unique_ptr<AnimationBase>& anim) { return anim->isFinished(); }),
            activeAnimations.end());
    }

private:
    std::vector<std::unique_ptr<AnimationBase>> activeAnimations;
};

