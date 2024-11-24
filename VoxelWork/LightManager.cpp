#include "LightManager.h"
#include "ChunkSpace.h"
LightManager::LightManager()
{
	start();
}
LightManager::~LightManager()
{
	for (int i = 0; i < NumWorkers; i++)
	{
		t[i].join();
	}
}
void LightManager::tick() //Called before any node ticks
{
	//std::cout << "a\n";
	for (int i = 0; i < NumWorkers; i++)
	{
		bool shouldThreadRun = false;
		//Prepare Work
		for (int l = 0; l < lights.size(); l++)
		{
			if (lightUpdated[l])
			{
				threadLights[i] = lights[l];
				lightUpdated[l] = false;
				shouldThreadRun = true;
				break;
			}
			
		}
		

		if (shouldThreadRun)
		{
			//Check we HaveWork in thread
			{ // Only use haveWork if other thread is not
				std::unique_lock<std::mutex> lock(mutex[i]);
				haveWork[i] = true;
			}
			workReady[i].notify_one(); //Inform worker that we want an update
		}
		
	}

	
}

void LightManager::lightTick() //Called after nodes tick - We can use the threads to process light in bg
{
	//std::cout << "Light Tick Starting " << std::endl;
	for (int i = 0; i < NumWorkers; i++)
	{
		std::unique_lock<std::mutex> lock(mutex[i]);
		while (haveWork[i])
		{
			workReady[i].wait(lock);
		}
	}
	//std::cout << "Light Tick Ending " << std::endl;
	//Now we're out of threads,
	//Mark lights to be erased

}

void LightManager::threadedLightUpdate(uint16_t thread)
{
	
	
	while (workerRunning[thread])
	{
		{
			std::unique_lock<std::mutex> lock(mutex[thread]);

			while (!haveWork[thread])
			{ // Wait
				
				workReady[thread].wait(lock);
			}
		}

		
		//Do Work
		light* l = &threadLights[thread];
		//Find out affected chunks (and mark in -1 to 1, xyz)
		glm::ivec3 lowerLimit = glm::ivec3(0, 0, 0);
		glm::ivec3 upperLimit = glm::ivec3(0, 0, 0);
		if (((l->pos.x) % 16) < l->strength) //We're going to overlap into x-negative
		{
			lowerLimit.x -= 1;
		}
		if (((l->pos.x) % 16) + l->strength>15) //We're going to overlap into x-postive
		{
			upperLimit.x += 1;
		}
		if (((l->pos.y) % 16) < l->strength) //We're going to overlap into y-negative
		{
			lowerLimit.y -= 1;
		}
		if (((l->pos.y) % 16) + l->strength > 15) //We're going to overlap into x-postive
		{
			upperLimit.y += 1;
		}if (((l->pos.z) % 16) < l->strength) //We're going to overlap into x-negative
		{
			lowerLimit.z -= 1;
		}
		if (((l->pos.z) % 16) + l->strength > 15) //We're going to overlap into x-postive
		{
			upperLimit.z += 1;
		}

		//std::cout << "Light limits found : " << thread << std::endl;
		//Modulus so that we're in local coords

		//If lightStrength is 0 then mark for destruction and run the quick flood saturate algorithm but check to see if it has a higher light neighbour (if not then destroy).
		if (worldSpace) //Only do the work if we have a worldSpace to act upon
		{
			if (auto c = worldSpace->getChunk(l->pos.x / 16, l->pos.y / 16, l->pos.z / 16); c)
			{
				c->fastBlockFloodLighting(*l);
			}
		}
		
		//std::cout << "Light flooded : " << thread << std::endl;


		{ //We No longer have work in this thread
			std::unique_lock<std::mutex> lock(mutex[thread]);
			haveWork[thread] = false;
		}
		workReady[thread].notify_one(); //Inform main thread
		//std::cout << "Main thread notified from thread : " << thread << std::endl;
	}
	


}

void LightManager::start()
{
	for (int i = 0; i < NumWorkers; i++)
	{
		haveWork.push_back(false);
		workerRunning.push_back(true);
	}
	//Create Threads
	light l = { glm::ivec3(1,0,1),5 };
	lights.push_back(l);
	lightUpdated.push_back(true);
	
	

	threadLights.resize(NumWorkers);
	for (uint16_t i = 0; i < 12; i++)
	{
		//std::cout << i << std::endl;
		//t[i] = std::thread([=]() {threadedLightUpdate(i); });
		t[i] = std::thread(&LightManager::threadedLightUpdate, this, i);
	}
	//std::cout << "Thread Pool Created\n";
}

void LightManager::setLightUpdated(int32_t lIDX)
{
	
	lightUpdated[lIDX] = true;
	
}


