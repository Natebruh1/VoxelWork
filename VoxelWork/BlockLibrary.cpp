#include "BlockLibrary.h"

std::map<uint16_t, std::string>						BlockLibrary::idBlockLookup;		//Block Id		->	Block Name
std::map<std::string, std::vector<const char*>>		BlockLibrary::BlockTextures;		//Block Name	->	filepaths
std::map<std::string, bool>							BlockLibrary::BlockDefaultSolid;	//Block Name	->	Default Solid or not?


BlockLibrary::BlockLibrary()
{
	//In future, to make this dynamic, will read this data in from disk aswell. (Allows for easy mod support).

	//id BlockLookup
	BlockLibrary::idBlockLookup[0] = "Air";
	BlockLibrary::idBlockLookup[1] = "Stone";
	BlockLibrary::idBlockLookup[2] = "Dirt";



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


	// Default Solid
	BlockDefaultSolid["Air"]=false;
	BlockDefaultSolid["Stone"] = true;
	BlockDefaultSolid["Dirt"] = true;



}

std::string BlockLibrary::operator[](uint32_t id)
{
	return idBlockLookup[id];
}
