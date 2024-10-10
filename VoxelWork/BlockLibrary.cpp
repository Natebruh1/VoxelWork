#include "BlockLibrary.h"

std::map<uint16_t, std::string>						BlockLibrary::idBlockLookup;		//Block Id		->	Block Name
std::map<std::string, std::vector<std::string>>		BlockLibrary::BlockTextures;		//Block Name	->	filepaths
std::map<std::string, bool>							BlockLibrary::BlockDefaultSolid;	//Block Name	->	Default Solid or not?


BlockLibrary::BlockLibrary()
{
	//In future, to make this dynamic, will read this data in from disk aswell. (Allows for easy mod support).

	//id BlockLookup
	BlockLibrary::idBlockLookup[0] = "Air";
	BlockLibrary::idBlockLookup[1] = "Stone";
	BlockLibrary::idBlockLookup[2] = "Dirt";
	BlockLibrary::idBlockLookup[3] = "Grass";



	//BlockTextures
	//Air
	BlockTextures["Air"].push_back("");
	BlockTextures["Air"].push_back("");
	BlockTextures["Air"].push_back("");
	BlockTextures["Air"].push_back("");
	BlockTextures["Air"].push_back("");
	BlockTextures["Air"].push_back("");

	//Stone
	BlockTextures["Stone"].push_back("textures/stone.png");
	BlockTextures["Stone"].push_back("textures/stone.png");
	BlockTextures["Stone"].push_back("textures/stone.png");
	BlockTextures["Stone"].push_back("textures/stone.png");
	BlockTextures["Stone"].push_back("textures/stone.png");
	BlockTextures["Stone"].push_back("textures/stone.png");

	//Dirt
	BlockTextures["Dirt"].push_back("textures/dirt.png");
	BlockTextures["Dirt"].push_back("textures/dirt.png");
	BlockTextures["Dirt"].push_back("textures/dirt.png");
	BlockTextures["Dirt"].push_back("textures/dirt.png");
	BlockTextures["Dirt"].push_back("textures/dirt.png");
	BlockTextures["Dirt"].push_back("textures/dirt.png");

	//Grass
	BlockTextures["Grass"].push_back("textures/grassSide.png");
	BlockTextures["Grass"].push_back("textures/grassSide.png");
	BlockTextures["Grass"].push_back("textures/grassSide.png");
	BlockTextures["Grass"].push_back("textures/grassSide.png");
	BlockTextures["Grass"].push_back("textures/dirt.png");
	BlockTextures["Grass"].push_back("textures/grassTop.png");


	// Default Solid
	BlockDefaultSolid["Air"]=false;
	BlockDefaultSolid["Stone"] = true;
	BlockDefaultSolid["Dirt"] = true;
	BlockDefaultSolid["Grass"] = true;


	//Load in blocks from ./mods/blocks
	LoadBlocksFromFile();


}

void BlockLibrary::LoadBlocksFromFile()
{
	std::string path = "./mods/blocks";
	auto aPath = std::filesystem::path(path);
	if (std::filesystem::is_directory(aPath)) //If we can find the directory
	{
		for (const auto& entry : std::filesystem::directory_iterator(path)) //Loop through the contents
		{
			
			if (entry.path().extension().string() == ".dat") //If they're dat files
			{
				//We have a block file
				std::fstream file(entry.path());
				nlohmann::json jObj;
				file >> jObj;
				file.close();
				//Set an ID to a name
				std::string blockName = jObj["Name"].template get<std::string>();
				BlockLibrary::idBlockLookup[idBlockLookup.size()] = blockName;
				BlockTextures[blockName].push_back(jObj["Textures"]["0"].template get<std::string>());
				BlockTextures[blockName].push_back(jObj["Textures"]["1"].template get<std::string>());
				BlockTextures[blockName].push_back(jObj["Textures"]["2"].template get<std::string>());
				BlockTextures[blockName].push_back(jObj["Textures"]["3"].template get<std::string>());
				BlockTextures[blockName].push_back(jObj["Textures"]["4"].template get<std::string>());
				BlockTextures[blockName].push_back(jObj["Textures"]["5"].template get<std::string>());
				
				BlockDefaultSolid[blockName] = jObj["Solid"].template get<bool>();
			}
		}
	}
	
		
}

std::string BlockLibrary::operator[](uint32_t id)
{
	return idBlockLookup[id];
}
