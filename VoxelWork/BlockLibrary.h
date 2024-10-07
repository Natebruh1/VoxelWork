#pragma once
#include <map>
#include <vector>
#include <string>
class BlockLibrary
{
public:
	BlockLibrary();
	static std::map<uint16_t, std::string> idBlockLookup;
	static std::map<std::string, std::vector<const char*>> BlockTextures;
	static std::map<std::string, bool> BlockDefaultSolid;

	std::string operator[](uint32_t id); //Operator overloading for easy access to idBlockLookup
};

