#include "WorldSpace.h"
#include "camera.h"
#include "LightManager.h"

void WorldSpace::generate(int x, int y, int z)
{
	glm::ivec3 chunkToAdd = glm::ivec3(x, y, z);
	if (std::find(knownChunks.begin(), knownChunks.end(), chunkToAdd) != knownChunks.end())
	{
		//Chunk known, load from disc
	}
	else
	{
		//Chunk unknown, generate
		knownChunks.push_back(chunkToAdd);
		chunk* newChunk = new chunk();

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
							currBlock->solid= blockLibrary.BlockDefaultSolid[blockLibrary[blockPalette[i]]];
							break;
						}
					}
					
				}
			}
			
		}

		newChunk->rebuildChunk();
		addChunk(x,y,z,*newChunk);
	}
}

WorldSpace::WorldSpace()
{
	withLight = false; //We don't want light


	rules.push_back
	(
		[=](int x, int y, int z) {
			return (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y;
		}
	);
	blockPalette.push_back(1);


	rules.push_back
	(
		[=](int x, int y, int z) {
			return (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y-1;
		}
	);
	blockPalette.push_back(2);

	rules.push_back
	(
		[=](int x, int y, int z) {
			return (heightNoise.GetNoise((float)x, (float)z) * worldHeight) > y-2;
		}
	);
	blockPalette.push_back(3);


	//Default is air
	rules.push_back
	(
		[=](int x, int y, int z) {
			return true; //Return Stone
		}
	);
	blockPalette.push_back(0);
}
