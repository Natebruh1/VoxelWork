#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "json.hpp"
class Model;
class ModelNode;
class PartSpace;
class ModelLibrary
{

public:
	ModelLibrary();
	void LoadModels();
	void LoadParts();
	static Model* LoadModel(std::string path);
	static std::unordered_map<std::string, Model*> modelList;
	~ModelLibrary();
private:
	static Model* LoadModel(nlohmann::json& modelJson);
	static void ParsePart(nlohmann::json& part,ModelNode* localRoot,Model* baseModel, std::string nameOverride="");
	std::vector<PartSpace*> loadedParts;

};

