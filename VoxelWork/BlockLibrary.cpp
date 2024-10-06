#include "BlockLibrary.h"

std::map<uint16_t, std::string>						BlockLibrary::idBlockLookup; //Block Id		->	Block Name
std::map<std::string, std::vector<const char*>>		BlockLibrary::BlockTextures; //Block Name	->	filepaths

BlockLibrary::BlockLibrary()
{
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

}
