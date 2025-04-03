#include "WorldSpace.h"
#include "camera.h"
#include "LightManager.h"
#include "ModelLibrary.h"
#include "Model.h"
WorldSpace* WorldSpace::CurrentWorld = nullptr;
int WorldSpace::currentHand = 0;
namespace Models
{
	node* models = nullptr;
}
void WorldSpace::generate(glm::ivec3 coords)
{
	if (std::find(knownChunks.begin(), knownChunks.end(), coords) == knownChunks.end())
	{
		chunkMarkedGenerate.push_back(coords);
		knownChunks.push_back(coords);
	}
	else
	{
		//Load Chunk
		//if (std::find())
	}
	
}

void WorldSpace::generate(int x, int y, int z)
{
	std::unique_lock<std::mutex> lock(chunkListLock);
	glm::ivec3 chunkToAdd = glm::ivec3(x, y, z);
	//if (std::find(knownChunks.begin(), knownChunks.end(), chunkToAdd) != knownChunks.end())
	//{
	//	//Chunk known, load from disc
	//}
	/*else
	{*/
	//Chunk unknown, generate

	chunk* newChunk = getChunk(x, y, z);
	newChunk->setEdited(true);
	

	//Generate chunk
	int worldX = x * 16;
	int worldY = y * 16;
	int worldZ = z * 16; //Do the calculations once
	for (int dx = 0; dx < 16; dx++)
	{
		for (int dy = 0; dy < 16; dy++)
		{
			for (int dz = 0; dz < 16; dz++)
			{
				//Convert to world space




				for (int i = 0; i < rules.size(); i++) //Go through rules
				{
					if (rules[i](worldX + dx, worldY + dy, worldZ + dz))
					{
						/*std::cout << 256 * dx + 16 * dy + dz << std::endl;
						if (256 * dx + 16 * dy + dz == 4095)
						{
							std::cout << "\n";
						}*/
						block* currBlock = (newChunk->getData() + 256 * dx + 16 * dy + dz);
						currBlock->id = blockPalette[i];
						currBlock->solid = blockLibrary.BlockDefaultSolid[blockLibrary[blockPalette[i]]];
						break;
					}
				}

			}
		}

	}

	newChunk->rebuildChunk();

//}
}

void WorldSpace::threadLoop(uint16 thread)
{
	while (workerRunning[thread])
	{
		{
			//Scope for the lock
			std::unique_lock<std::mutex> lock(mutex[thread]);
			while (!haveWork[thread])
			{
				//Wait for work
				workReady[thread].wait(lock);
			}
		}
		
		//Do the work (gen Chunk)
		if (workerRunning[thread]) //Because the worker could stop running whilst we are waiting for the thread to get work
		{
			//std::cout << "Working this thread : " << thread << std::endl;
			for (auto i = perThreadInfo[thread].start; i < perThreadInfo[thread].start + perThreadInfo[thread].size; i++)
			{
				generate(i->x, i->y, i->z);
			}
			//std::cout << "Finished working this thread : " << thread << std::endl;
		}
		
		
		{
			std::unique_lock<std::mutex> lock(mutex[thread]);
			haveWork[thread] = false;
		}
		workReady[thread].notify_one();
	}
}

WorldSpace::WorldSpace()
{
	withLight = false; //We don't want light
	heightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	heightNoise.SetSeed(20);	
	carver1.SetSeed(20);

	carver1.SetFrequency(0.03);
	carver2.SetFrequency(0.01);

	carverCopper.SetFrequency(0.01);
	carverCopper.SetSeed(20);
	
	ctx.AddGlobalVariable("world", num);

	ctx.CompileString("test", "world = world+1");

	/*
	rules.push_back(		// -- LUA TEST -- //
		[=](int x, int y, int z) {
			ctx.Run("test");
			return (((int)num->getValue()) % 2)>0;
		}
	);

	blockPalette.push_back(4); //Not air
	*/


	rules.push_back(		// -- SPAG CAVERNS -- //
		[=](int x, int y, int z) {
			auto sqnoise = (carver2.GetNoise((float)x, (float)y, (float)z)) * (carver2.GetNoise((float)x, (float)y, (float)z));
			return sqnoise<0.0005;
		}
	);
	blockPalette.push_back(0); //Air

	

	rules.push_back(		// -- CHAMBER CAVERNS -- //
		[=](int x, int y, int z) {
			return (carver1.GetNoise((float)x, (float)y, (float)z) > 0.55);
		}
	);
	blockPalette.push_back(0); //Air

	rules.push_back(		// -- CHAMBER CAVERNS CRUST -- //
		[=](int x, int y, int z) {

			return ((carver1.GetNoise((float)x, (float)y, (float)z) > 0.55) != (carver1.GetNoise((float)x, (float)y + 1, (float)z) > 0.55)) * (rand() % 10 < 3) && (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y;
		}
	);
	blockPalette.push_back(9); //Strata rock

	rules.push_back(		// -- Copper Chunk -- //
		[=](int x, int y, int z) {

			return (carverCopper.GetNoise((float)x+10, (float)y, (float)z-10) > 0.97f) && ((heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y);
		}
	);
	blockPalette.push_back(10); //Copper Ore


	rules.push_back
	(
		[=](int x, int y, int z) {		// -- WORLD HEIGHT -- //
			return (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y;
		}
	);
	blockPalette.push_back(1); //Stone


	rules.push_back
	(
		[=](int x, int y, int z) {
			return (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y-1;
		}
	);
	blockPalette.push_back(2);//Dirt

	rules.push_back
	(
		[=](int x, int y, int z) {
			return (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y-2;
		}
	);
	blockPalette.push_back(3);//Grass


	//Default is air
	rules.push_back
	(
		[=](int x, int y, int z) {
			return true; //Return Air
		}
	);
	blockPalette.push_back(0);



	// -- THREAD SETUP --
	for (int i = 0; i < NumWorkers; i++)
	{
		haveWork.push_back(false);
		workerRunning.push_back(true);
		t[i] = std::thread(&WorldSpace::threadLoop, this, i);
	}

	CurrentWorld = this;

	
}

WorldSpace::~WorldSpace()
{
	//std::cout << "--Starting WorldDestruction--" << std::endl;
	for (int i = 0; i < NumWorkers; i++)
	{
		

		
		
		//std::cout << "Thread attempt join at : " << i << std::endl;
		t[i].detach();
		//std::cout << "Thread joined at : " << i << std::endl;
	}
	//std::cout << "--Deserializing WorldSpace--" << std::endl;
}

void WorldSpace::tick()
{
	for (int i = 0; i < NumWorkers; i++)
	{
		//Prepare work for struct
		if (chunkMarkedGenerate.size() > 0)
		{
			if (chunkMarkedGenerate.size() <= NumWorkers && i < chunkMarkedGenerate.size())
			{
				perThreadInfo[i].start = &chunkMarkedGenerate[i];
				perThreadInfo[i].size = 1;
			}
			else
			{
				if (i == NumWorkers - 1) //On last thread
				{
					perThreadInfo[i].start = &chunkMarkedGenerate[(chunkMarkedGenerate.size() / NumWorkers)*i];
					perThreadInfo[i].size = (chunkMarkedGenerate.size() / NumWorkers) + (chunkMarkedGenerate.size() % NumWorkers); //Add remainder
				}
				else //On any other threads
				{
					perThreadInfo[i].start = &chunkMarkedGenerate[(chunkMarkedGenerate.size() / NumWorkers)*i];
					perThreadInfo[i].size = (chunkMarkedGenerate.size() / NumWorkers);
				}
			}
			auto s= perThreadInfo[i].start; //Start
			auto e = perThreadInfo[i].start + perThreadInfo[i].size; //Finish
			for (auto j = s; j < e; j++)
			{
				chunk* newChunk = new chunk();
				addChunk(j->x, j->y, j->z, *newChunk);
				
			}

			{
				std::unique_lock<std::mutex> lock(mutex[i]);
				haveWork[i] = true;
			}

			workReady[i].notify_one(); // Tell worker to start
		}

	}

	//FIND A WAY TO MAKE THIS ONLY WAIT IF THE EXECUTION IS FINISHED
	for (int i = 0; i < NumWorkers; i++)
	{
		std::unique_lock<std::mutex> lock(mutex[i]); //Wait for work to finish
		if (haveWork[i])
		{
			
			workReady[i].wait(lock);
			
			
		}
	}
	chunkMarkedGenerate.clear();
	node::tick();
}
