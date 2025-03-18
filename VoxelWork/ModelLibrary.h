#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "json.hpp"
class ModelLibrary
{

public:
	ModelLibrary();
	void LoadModels();

private:
	void LoadModel(nlohmann::json& modelJson);
};

