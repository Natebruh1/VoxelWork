#include "ModelLibrary.h"

ModelLibrary::ModelLibrary()
{
	LoadModels();
}

void ModelLibrary::LoadModels()
{
	
	std::string path = "./mods/models";
	auto aPath = std::filesystem::path(path);
	if (std::filesystem::is_directory(aPath)) //If we can find the directory
	{
		for (const auto& entry : std::filesystem::directory_iterator(path)) //Loop through the contents
		{

			if (entry.path().extension().string() == ".dat") //If they're dat files
			{
				try {
					//We have a block file
					std::fstream file(entry.path());
					nlohmann::json jObj;
					file >> jObj;
					file.close();
					LoadModel(jObj);
				}
				catch (std::exception e)
				{
					std::ofstream errorFile("errors.log");
					if (errorFile.is_open())
					{
						errorFile << "Error adding model : " << entry.path().filename().string() << std::endl;

						errorFile.close();
					}
				}
			}
		}
	}

}

void ModelLibrary::LoadModel(nlohmann::json& modelJson)
{
	std::cout << modelJson.dump() << std::endl<<std::endl;
	
}
