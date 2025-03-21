#include "ModelLibrary.h"
#include "Model.h"
#include "WorldSpace.h"
std::unordered_map<std::string, Model*> ModelLibrary::modelList;
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
	//std::cout << modelJson.dump() << std::endl<<std::endl;
	Model* newModel;
	newModel = new Model();
	std::string name = modelJson.at("name").get<std::string>();
	std::string scriptName="";
	if (modelJson.contains("Script"))
		scriptName = modelJson["Script"];


	nlohmann::json parts = modelJson.at("Root").at("Parts").get<nlohmann::json>();

	ParsePart(parts, &(newModel->root),newModel);
	
	modelList[name] = newModel;

	//Attach script
	if (scriptName!="")
		newModel->attachScript(scriptName);
}



void ModelLibrary::ParsePart(nlohmann::json& part, ModelNode* localRoot,Model* baseModel, std::string nameOverride)
{
	//Get the first key in parts
	if (nameOverride == "")
	{
		if (!part.empty())
		{
			localRoot->attachedPart = part.begin().value()["name"]; //Add child
			part = part.begin().value().get<nlohmann::json>();
		}
	}
	else
	{
		localRoot->attachedPart = part["name"];
	}
	
	
	std::vector<float> defaultVec = { 0.f,0.f,0.f };
	//Get Position
	std::vector<float> pos=part.contains("Position") ? part.at("Position").get<std::vector<float>>() : defaultVec;
	localRoot->localAttachment.setPosition(glm::vec3(pos[0], pos[1], pos[2]));
	//Get Rotation
	std::vector<float> rot = part.contains("Rotation") ? part.at("Rotation").get<std::vector<float>>() : defaultVec;
	localRoot->localAttachment.setRotation(glm::vec3(rot[0], rot[1],rot[2]));
	//Get Scale
	std::vector<float> sca = part.contains("Scale") ? part.at("Scale").get<std::vector<float>>() : std::vector<float>({1.0,1.0,1.0}); //Defaults to 1.0;
	localRoot->localAttachment.setScale(glm::vec3(sca[0], sca[1], sca[2]));
	//std::cout << part.dump();
	if (part.contains("Animations"))
	{
		//Add animations
		for (auto animIt = part["Animations"].begin(); animIt != part["Animations"].end(); animIt++)
		{
			if (animIt.value().contains("Position"))
			{

				std::vector<float> finalPos=animIt.value()["Position"].get<std::vector<float>>();
				bool rel = animIt.value().contains("Relative") && animIt.value()["Relative"].get<bool>();
				baseModel->addAnimation(animIt.key(), localRoot->localAttachment.getPosition(), glm::vec3(finalPos[0], finalPos[1], finalPos[2]), animIt.value()["Length"].get<float>(), rel);
			}
			//Rotation
			if (animIt.value().contains("Rotation"))
			{

				std::vector<float> finalPos = animIt.value()["Rotation"].get<std::vector<float>>();
				bool rel = animIt.value().contains("Relative") && animIt.value()["Relative"].get<bool>();
				baseModel->addAnimation(animIt.key(), localRoot->localAttachment.getRotation(), glm::vec3(finalPos[0], finalPos[1], finalPos[2]), animIt.value()["Length"].get<float>(), rel);
			}

			if (animIt.value().contains("Scale"))
			{

				std::vector<float> finalPos = animIt.value()["Scale"].get<std::vector<float>>();
				bool rel = animIt.value().contains("Relative") && animIt.value()["Relative"].get<bool>();
				baseModel->addAnimation(animIt.key(), localRoot->localAttachment.getRotation(), glm::vec3(finalPos[0], finalPos[1], finalPos[2]), animIt.value()["Length"].get<float>(), rel);
			}

		}
	}
	
	//Recurse through children
	for (auto childIt=part["Children"].begin(); childIt != part["Children"].end();childIt++)
	{
		ModelNode* childNode = new ModelNode();
		localRoot->children.push_back(childNode);
		//nlohmann::json jView = childIt.value().get<nlohmann::json>();
		//auto st = childIt.key();
		ParsePart(childIt.value(), childNode, baseModel,childIt.key());
	}
}
