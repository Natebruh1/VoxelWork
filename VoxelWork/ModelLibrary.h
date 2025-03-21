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
class ModelLibrary
{

public:
	ModelLibrary();
	void LoadModels();
	static std::unordered_map<std::string, Model*> modelList;
private:
	void LoadModel(nlohmann::json& modelJson);
	void ParsePart(nlohmann::json& part,ModelNode* localRoot,Model* baseModel, std::string nameOverride="");

};

