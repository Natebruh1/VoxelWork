#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>


#include <vector>
#include <thread>
#include <mutex>
class ChunkSpace;
struct light {
	glm::ivec3 pos;
	uint8_t strength;
};

class LightManager
{
public:
	LightManager();
	~LightManager();
	void tick();
	void lightTick();
	void threadedLightUpdate(uint16_t thread);
	void start();
	void setWorldSpace(ChunkSpace* cSpace) { worldSpace = cSpace; };
	
	//Light Managing
	void setLightUpdated(int32_t lIDX);


private:
	ChunkSpace* worldSpace;
	std::vector<light> lights;
	std::vector<light> threadLights; //Size equal to cores (12)
	uint32_t currentLight=0;
	std::vector<bool> lightUpdated;

	//Threading
	const int NumWorkers = 12; //Cores on Computer
	std::condition_variable workReady[12];
	std::mutex mutex[12];
	std::vector<bool> haveWork;
	std::vector<bool> workerRunning;
	std::thread t[12];
};

