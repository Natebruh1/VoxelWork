#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "json.hpp"
class BlockLibrary
{
public:
	BlockLibrary();
	static std::map<uint16_t, std::string> idBlockLookup;
	static std::map<std::string, std::vector<std::string>> BlockTextures;
	static std::map<std::string, bool> BlockDefaultSolid;
	void LoadBlocksFromFile();
	std::string operator[](uint32_t id); //Operator overloading for easy access to idBlockLookup
};

