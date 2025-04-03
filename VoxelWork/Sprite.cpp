#include "Sprite.h"

bool Sprite::UIEnabled = true;


void Sprite::render(camera& cam)
{
	if (UIEnabled)
	{
		ResourceManager::GetShader("2D")->Use().SetInteger("image", 0);
		ResourceManager::GetShader("2D")->SetMatrix4("projection", projection);
		
		if (!tex)
		{
			std::cout << "Failed to load texture: " << textureName << std::endl;
			return;
		}
		else
		{
			renderer->DrawSprite(*tex, getPosition(), glm::vec2(tex->Width, tex->Height) * getScale(), 0.f, glm::vec3(1.f, 1.f, 1.f));
		}
		
	}
	
}

void Sprite::SetTexture(std::string texName)
{
	
	auto r =ResourceManager::LoadTexture(texName.c_str(), true, texName);
	
	//std::cout << r.Width;
	
	textureName = texName;
	tex = ResourceManager::GetTexture(textureName);
}

Sprite::~Sprite()
{
	node::~node();
}
