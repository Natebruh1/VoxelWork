#include "PartSpace.h"
#include "camera.h"
#include "LightManager.h"

std::map<std::string, PartSpace*> PartSpace::partLibrary;


PartSpace::PartSpace() : ChunkSpace()
{
	withLight = false;
	if (withLight)
	{
		lightsManager = new LightManager();
		lightsManager->setWorldSpace(this);
	}
	//std::map<std::string, PartSpace> PartSpace::partLibrary;
}

void PartSpace::serialize(nlohmann::json& data, bool store)
{
}

void PartSpace::LoadFromLibrary(std::string partName)
{
}

void PartSpace::RegisterToLibrary(std::string partName, std::string fileToLoad)
{
	chunks.clear();

	//Load from file
	if (fileToLoad == "NOFILE")
	{
		chunk* partChunk = new chunk();
		partChunk->createFullChunk();
		addChunk(0, 0, 0, *partChunk);
	}
	else
	{
		try
		{
			std::ifstream f(fileToLoad);
			nlohmann::json data = nlohmann::json::parse(f);
			RegisterToLibrary(partName, data);

		}
		catch (std::exception e)
		{
			std::ofstream errorFile("errors.log");
			if (errorFile.is_open())
			{
				errorFile << "\nError reading part at : " << fileToLoad << std::endl;

				errorFile.close();
			}
		}
	}


	

}

void PartSpace::RegisterToLibrary(std::string partName, nlohmann::json& jObj)
{
	nlohmann::json data = jObj;
	chunkLoadTracker loadTracker;
	//auto xChunk = data.at("BlockData").begin(); xChunk!= data.at("BlockData").end();++xChunk
	for (auto& [xKey, xVal] : data.at("BlockData").items())
	{
		for (auto& [yKey, yVal] : xVal.items())
		{
			for (auto& [zKey, zVal] : yVal.items())
			{
				chunk* partChunk = new chunk();
				bool onOdd = false;
				uint32 offset = 0;
				for (size_t i = 0; i < zVal.size(); i++)
				{


					onOdd = !onOdd;
					if (onOdd)
					{
						partChunk->setBlockArray(zVal[i], zVal[i + 1], offset);
						offset += zVal[i + 1].get<int>();
					}
				}
				//Update the iterators for the chunk
				auto x_it = stoi(xKey);
				auto y_it = stoi(yKey);
				auto z_it = stoi(zKey);
				addChunk(x_it, y_it, z_it, *partChunk);

			}
		}
	}
	tick();


	//Add to partLibrary
	partLibrary[partName] = this;
}

void PartSpace::RenderPart(std::string partName, node3D& Owner, camera& currentCamera)
{
	PartSpace* requestedPart = partLibrary.at(partName);
	if (requestedPart != nullptr)
	{
		
		for (auto& child : requestedPart->children)
		{
			static_cast<node3D*>(child)->transform = requestedPart->transform * Owner.transform;
		}
		requestedPart->render(currentCamera);
	}
	
}

void PartSpace::RenderPartTransformed(std::string partName, node3D& Owner, glm::mat4 transform, camera& currentCamera)
{

	if (partName == "") return; //Early return
	PartSpace* requestedPart = partLibrary.at(partName);
	if (requestedPart != nullptr)
	{

		for (auto& child : requestedPart->children)
		{
			static_cast<node3D*>(child)->transform = transform * Owner.transform * requestedPart->transform;
		}
		requestedPart->render(currentCamera);
	}
}

void PartSpace::SetBlock(std::string partName, int x, int y, int z, uint32 id)
{
	int chunkX = x / 16;
	int chunkY = y / 16;
	int chunkZ = z / 16;

	PartSpace* requestedPart = partLibrary.at(partName);
	if (requestedPart != nullptr)
	{
		requestedPart->getChunk(chunkX, chunkY, chunkZ)->setBlock(x % 16, y % 16, z % 16, id);
		requestedPart->tick();
	}
	
}



