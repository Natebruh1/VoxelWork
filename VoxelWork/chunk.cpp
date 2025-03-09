#include "chunk.h"
#include "camera.h";
#include "ChunkSpace.h"
#include "LightManager.h"
//Instantiate Static Variables
std::vector<uint16>			chunk::knownTextures;
SparseBindlessTextureArray	chunk::ChunkTextures;

chunk::chunk()
{
	chunkData = new block[16 * 16 * 16];
	lightfield = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 }; //X, Y and Z in the bits
	glGenVertexArrays(1, &chunkVAO); //Generate buffers and arrays
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &texIndexSSBO);
	glGenBuffers(1, &lightIndexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32) * CHUNKSIZE * CHUNKSIZE * CHUNKSIZE * 6, nullptr, GL_DYNAMIC_DRAW);
	
}

std::vector<uint32>* chunk::serialize()
{
	
	//BlockData
	
	ids.reserve(4096);
	uint32_t currentRun;
	uint32_t currentLength=1;
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				if (x == 0 && y == 0 && z == 0)
				{
					//Start the first run
					currentRun = (uint32)(chunkData + (z)+(16 * y) + (256 * x))->id;
					
				}
				if (currentRun != (uint32)(chunkData + (z)+(16 * y) + (256 * x))->id)
				{
					//We've changed block, push to IDS
					ids.push_back(currentRun);
					ids.push_back(currentLength);

					currentLength = 0;//Reset the length of the run
					currentRun = (uint32)(chunkData + (z)+(16 * y) + (256 * x))->id;
				}

				currentLength++;
				
			}
		}
	}
	return &ids;
	


	
	
}

chunk::~chunk()
{
	delete[] chunkData;
	delete[] lightfield;
	//Delete Buffers
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &texIndexSSBO);
	
	glDeleteVertexArrays(1, &chunkVAO);
	
	//Delete std::vector
	chunkQuads.clear();
	vertices.clear();
	data.clear();
	textureIndices.clear();


	//Join Threads
	try
	{
	
		if (lightCalcThread.joinable())
		{
			lightCalcThread.join();
		}

	}
	catch (std::exception e)
	{
		std::cout <<"Error Joining Light Thread during chunk dectruction : "<< e.what() << std::endl;
	}

	
	
}



void chunk::deleteBlock(uint16 x, uint16 y, uint16 z)
{
	

	(chunkData + (CHUNKSIZE * CHUNKSIZE * x + CHUNKSIZE * y + z))->id = 0b0;
	(chunkData + (CHUNKSIZE * CHUNKSIZE * x + CHUNKSIZE * y + z))->solid = 0b0;
	//Mark Geom Updated
	geomUpdated = true;
}

void chunk::setBlock(uint32 x, uint32 y, uint32 z, uint32 id)
{
	block& blockRef = getBlock(x, y, z);
	blockRef.id = id;
	blockRef.solid = blockLibrary.BlockDefaultSolid[blockLibrary[id]] ? 0b1 : 0b0; //Check if the block defaults to solid
	geomUpdated = true;
	edited = true;
}

void chunk::setBlockArray(uint32 id, uint32 length, uint32 offset)
{
	if (!this) return;
	unsigned int solid = blockLibrary.BlockDefaultSolid[blockLibrary[id]];
	block copyBlock={id,solid};
	
	for (uint32 i = 0; i < length; i++)
	{
		memcpy(chunkData + offset+i, &copyBlock, sizeof(block));
	}
	geomUpdated = true;
}

inline block& chunk::getBlock(int x, int y, int z)
{
	
	if (x < 0 or y < 0 or z < 0 or x>15 or y>15 or z>15)
	{
		
		int targetX = 0;	//Target chunk coords
		int targetY= 0;
		int targetZ = 0;

		int blockX = x; //Target block in adjacent chunk
		int blockY = y;
		int blockZ = z;
		if (x < 0) { targetX = -1; blockX = 15; }		//X
		else if (x > 15) { targetX = 1; blockX = 0; }
		

		if (y < 0) { targetY = -1; blockY = 15; }		//Y
		else if (y > 15) { targetY = 1; blockY = 0; }
		

		if (z < 0) { targetZ = -1; blockZ = 15; }		//Z
		else if (z > 15) { targetZ = 1; blockZ = 0; }
		

		if (chunk* trgChunk = static_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x + targetX, chunkCoords.y + targetY, chunkCoords.z + targetZ); trgChunk == nullptr)
		{
			return EmptyBlock;
		}
		else
		{
			return trgChunk->getBlock(blockX, blockY, blockZ);
		}
		
	}
	return *(chunkData + z + (y * CHUNKSIZE) + (x * CHUNKSIZE * CHUNKSIZE));
}

inline block* chunk::getBlockAddr(int x, int y, int z)
{
	if (x < 0 or y < 0 or z < 0 or x>15 or y>15 or z>15)
	{

		int targetX = 0;	//Target chunk coords
		int targetY = 0;
		int targetZ = 0;

		int blockX = x; //Target block in adjacent chunk
		int blockY = y;
		int blockZ = z;
		if (x < 0) { targetX = -1; blockX = 15; }		//X
		else if (x > 15) { targetX = 1; blockX = 0; }


		if (y < 0) { targetY = -1; blockY = 15; }		//Y
		else if (y > 15) { targetY = 1; blockY = 0; }


		if (z < 0) { targetZ = -1; blockZ = 15; }		//Z
		else if (z > 15) { targetZ = 1; blockZ = 0; }
		if (chunk* trgChunk = static_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x + targetX, chunkCoords.y + targetY, chunkCoords.z + targetZ); trgChunk == nullptr)
		{
			return nullptr;
		}
		else
		{
			return trgChunk->getBlockAddr(blockX, blockY, blockZ);
		}
	}
	return (chunkData + z + (y * CHUNKSIZE) + (x * CHUNKSIZE * CHUNKSIZE));
}


void chunk::createFullChunk()
{
	for (unsigned int i = 0; i < CHUNKSIZE; i++)
	{
		for (unsigned int j = 0; j < CHUNKSIZE; j++)
		{
			for (unsigned int k = 0; k < CHUNKSIZE; k++)
			{
				if (!(chunkData + (CHUNKSIZE * CHUNKSIZE * i + CHUNKSIZE * j + k)))
				{
					block* tempInArr = chunkData;
					tempInArr += (CHUNKSIZE * CHUNKSIZE * i + CHUNKSIZE * j + k);
					tempInArr = new block();
				}
				//Update new block
				(chunkData + (CHUNKSIZE * CHUNKSIZE * i + CHUNKSIZE * j + k))->id = 1;
				(chunkData + (CHUNKSIZE * CHUNKSIZE * i + CHUNKSIZE * j + k))->solid = 0b1;

				
			}
		}
	}
	//Mark for geom update
	geomUpdated = true;
}

void chunk::updateGeom(bool withNeighbour)
{
	


	//Clear Axis Cols
	//We use C-Style code here to efficiently clear the data
	memset(axis_col, 0, sizeof(axis_col));


	if (ChunkTextures.isGenerated == false)
	{
		
		ChunkTextures.Generate(128, 128, 6*blockLibrary.idBlockLookup.size()); //WIP 6 should be replaced with 6* blockLibrary.size();
	}
	///// ----PART 1 BUILD CHUNKMESH---- /////

	//Clear blockID's so we can recreate textures later

	//knownTextures.clear();
	//knownTextures.push_back(0); //Add air
	//ChunkTextures.addImage(blockLibrary.BlockTextures["Air"][0]); //Add one texture per axis
	//ChunkTextures.addImage(blockLibrary.BlockTextures["Air"][1]);
	//ChunkTextures.addImage(blockLibrary.BlockTextures["Air"][2]);
	//ChunkTextures.addImage(blockLibrary.BlockTextures["Air"][3]);
	//ChunkTextures.addImage(blockLibrary.BlockTextures["Air"][4]);
	//ChunkTextures.addImage(blockLibrary.BlockTextures["Air"][5]);

	//Create chunkSolid
	for (uint32 x = 0; x < CHUNKSIZE+2; x++)
	{
		for (uint32 y = 0; y < CHUNKSIZE+2; y++)
		{
			for (uint32 z = 0; z < CHUNKSIZE+2; z++)
			{
				//Add block ID to texture pool
				if (x < 16 and y < 16 and z < 16)
				{

					if (std::find(knownTextures.begin(), knownTextures.end(), getBlock(x, y, z).id) == knownTextures.end()) //If we haven't yet found this ID then add it
					{
						if (getBlock(x, y, z).solid == false) goto checkSolid; //No Texture support for unsolid blocks //Use a goto here since we need to check this but can't skip this array
						int ide = getBlock(x, y, z).id;
						knownTextures.push_back(getBlock(x, y, z).id);
						std::string blockName = blockLibrary.idBlockLookup[getBlock(x, y, z).id]; // Get the blockname

						ChunkTextures.addImage(blockLibrary.BlockTextures[blockName][0].c_str()); //Add one texture per axis
						ChunkTextures.addImage(blockLibrary.BlockTextures[blockName][1].c_str());
						ChunkTextures.addImage(blockLibrary.BlockTextures[blockName][2].c_str());
						ChunkTextures.addImage(blockLibrary.BlockTextures[blockName][3].c_str());
						ChunkTextures.addImage(blockLibrary.BlockTextures[blockName][4].c_str());
						ChunkTextures.addImage(blockLibrary.BlockTextures[blockName][5].c_str());

						
					}
				}

				


				checkSolid:
				//If block is solid
				if (getBlock(x-1,y-1,z-1).solid) //If returned block is solid
				{
					//[z + (y * sizeof(chunk_p))]
					//X axis
					axis_col[z + (y * 18)].data |= (0b1 << x);

					// Y axis
					axis_col[x + (y * 18)+ chunk_size_p2].data |= (0b1 << z);

					// Z axis
					axis_col[x + (z * 18) + chunk_size_p2*2].data |= (0b1 << y);
				}
				
			}
		}
	}

	//Clear Mask Col
	
	
	
	//Face culling
	for (int axis = 0; axis < 3; axis++) //X then Y then Z
	{
		for (int i = 0; i < chunk_size_p2; i++)
		{
			chunk_p paddedChunk = axis_col[(chunk_size_p2 * axis) + i];
			//Ascending Mask, bitshift right 1 to see if connecting faces touch solid or not
			mask_col[(chunk_size_p2 * (axis * 2 + 1)) + i].data = paddedChunk.data & ~(paddedChunk.data >> 1);

			//Descending Mask, bitshift right 1 to see if connecting faces touch solid or not
			mask_col[(chunk_size_p2 * (axis * 2)) + i].data = paddedChunk.data & ~(paddedChunk.data << 1);
		}
	}

	
	data.clear();
	//Binary Greedy Meshing
	for (int axis = 0; axis < 6; axis++)
	{
		
		for (int y = 0; y < CHUNKSIZE; y++)
		{
			for (int z = 0; z < CHUNKSIZE; z++)
			{
				
				int col_index = 1+z + ((y+1) * (CHUNKSIZE + 2)) + ((chunk_size_p2)*axis); //We add 1 here because we don't want to start on the neighbouring chunk
				uint16 col = mask_col[col_index].data >> 1;
				//Remove the left most padding
				col = col & ~(1 << CHUNKSIZE);

				//WIP
				while (col != 0)
				{
					uint16 x = trailingZeros(col);
					//Clear the least significant bit
					col &= col - 1;

					//glm::ivec3 voxelPosition = {}; //Defualt Initialization
					////Get the axis based upon switch statement
					//switch (axis)
					//{
					//case 0:
					//case 1:
					//	voxelPosition = { y,z,x };
					//	break;
					//case 2:
					//case 3:
					//	voxelPosition = { x,y,z };
					//	break;
					//case 4:
					//case 5:
					//	voxelPosition = { x,z,y };
					//	break;
					//}

					//Work on block hashing
					//Transform to axis based positions and then transform to chunk based positions
					
					auto& data_block = data[axis][x];
					
					data_block.resize(16, 0);
					data_block[z] |= (0b1 <<y);
					
				}
				
			}
		}
	}
	
	vertices.clear();
	chunkQuads.clear();
	///// ----PART 2 GREEDYMESH---- /////
	greedyMeshChunk();
	prepareRender();

	geomUpdated = false;
	
	if (withNeighbour) //If we're not updating because a neighbour told us to
	{
		//Mark neighbouring chunks for updates
		ChunkSpace* chunkParent = static_cast<ChunkSpace*>(parent);
		if (auto ch = chunkParent->getChunk(chunkCoords.x - 1, chunkCoords.y, chunkCoords.z); ch) { ch->neighbourUpdate(); } //x-1
		if (auto ch = chunkParent->getChunk(chunkCoords.x + 1, chunkCoords.y, chunkCoords.z); ch) { ch->neighbourUpdate(); } //x+1
		if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y - 1, chunkCoords.z); ch) { ch->neighbourUpdate(); } //y-1
		if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y + 1, chunkCoords.z); ch) { ch->neighbourUpdate(); } //y+1
		if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z - 1); ch) { ch->neighbourUpdate(); } //z-1
		if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z + 1); ch) { ch->neighbourUpdate(); } //z+1
	}
	neighbourUpdated = false; //Now we can mark our neighbours to update
}

void chunk::alertNeighbourToUpdate()
{
	ChunkSpace* chunkParent = static_cast<ChunkSpace*>(parent);
	if (auto ch = chunkParent->getChunk(chunkCoords.x - 1, chunkCoords.y, chunkCoords.z); ch) { ch->neighbourUpdate(); } //x-1
	if (auto ch = chunkParent->getChunk(chunkCoords.x + 1, chunkCoords.y, chunkCoords.z); ch) { ch->neighbourUpdate(); } //x+1
	if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y - 1, chunkCoords.z); ch) { ch->neighbourUpdate(); } //y-1
	if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y + 1, chunkCoords.z); ch) { ch->neighbourUpdate(); } //y+1
	if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z - 1); ch) { ch->neighbourUpdate(); } //z-1
	if (auto ch = chunkParent->getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z + 1); ch) { ch->neighbourUpdate(); } //z+1
}



std::vector<greedyQuad> chunk::greedyMeshBinaryPlane(std::map<int,std::vector<uint16>>& inDat)
{
	auto quads = std::vector<greedyQuad>();
	for (int plane = 0; plane < 16; plane++)
	{
		for (int row = 0; row < inDat.size(); row++)
		{
			int y = 0;
			while (y < 16)
			{
				if (auto search = inDat.find(plane); search == inDat.end()) break; //If inDat doesn't have this plane then we can skip this section
				y += trailingZeros(inDat[plane][row] >> y);
				

				if (y >= 16)
				{
					continue;
				}

				//Get height of greedy quad
				int h = trailingOnes(inDat[plane][row] >> y);
				//If the shift left has value (which is always in this case since the shift amount is less than 16 always) then hMask is that -1. Else we use ~0 (equivalent of 0xff for uint16_t type)
				uint16 hMask = checkedShl(1, h).has_value() ? checkedShl(1, h).value() - 1 : ~0;

				//Offset mask by the Y Position
				uint16 mask = hMask << y;

				//Now start growing the width of the greedy quad by using the mask
				int width = 1;
				//Grow horizontally across row

				while (row + width < 16)
				{
					//Get the height of the next row
					uint16 nextRowH = inDat[plane][row + width] >> y & hMask;
					
					if (nextRowH != hMask)
					{
						//The next part doesn't fit the mask so we have our quad
						break;
					}
					// Delete bits that we now know fit the greedy quad so that they don't get reused
					inDat[plane][row + width] = inDat[plane][row + width] & ~mask; //or !hMask
					width += 1;
				}
				//Construct the Greedy Quad
				greedyQuad newQuad = { y,h,row,width }; // Y H X W

				quads.push_back(newQuad);

				y += h;
			}

		}
	}
		
	
	




	return quads;
}

std::vector<greedyQuad> chunk::greedyMeshBinaryPlane(std::vector<uint16>& inDat)
{
	auto quads = std::vector<greedyQuad>();
	
	for (int row = 0; row < inDat.size(); row++)
	{
		int y = 0;
		while (y < 16)
		{
			
			y += trailingZeros(inDat[row] >> y);
			

			if (y >= 16)
			{
				continue;
			}

			//Get height of greedy quad
			int h = trailingOnes(inDat[row] >> y);
			//If the shift left has value (which is always in this case since the shift amount is less than 16 always) then hMask is that -1. Else we use ~0 (equivalent of 0xff for uint16_t type)
			uint16 hMask = checkedShl(1, h).has_value() ? checkedShl(1, h).value() - 1 : ~0;

			//Offset mask by the Y Position
			uint16 mask = hMask << y;

			//Now start growing the width of the greedy quad by using the mask
			int width = 1;
			//Grow horizontally across row

			while (row + width < 16)
			{
				//Get the height of the next row
				uint16 nextRowH = inDat[row + width] >> y & hMask;

				if (nextRowH != hMask)
				{
					//The next part doesn't fit the mask so we have our quad
					break;
				}
				// Delete bits that we now know fit the greedy quad so that they don't get reused
				inDat[row + width] = inDat[row + width] & ~mask; //or !hMask
				width += 1;
			}
			//Construct the Greedy Quad
			greedyQuad newQuad = { row,width,y,h, }; // X W Y H

			quads.push_back(newQuad);

			y += h;
		}

	}
	
	return quads;
}

void chunk::greedyMeshChunk()
{
	//Prepares geom from data
	for (int i = 0; i < 6; i++) //Don't use auto& i here for readability ~ Once for each axis (+ve and -ve)
	{
		for (int plane = 0; plane < 16; plane++)
		{
			if (auto search = data[i].find(plane); search != data[i].end())
			{
				std::vector<greedyQuad> quads = greedyMeshBinaryPlane(data[i][plane]); //Plane found inside data, begin greedy meshing!
				chunkQuads.insert(chunkQuads.end(), std::make_move_iterator(quads.begin()),
					std::make_move_iterator(quads.end()));

				//Now add to vertices based upon direction
				for (auto& quad : quads)
				{
					
					switch (i)
					{
					case 0: //X-Forwards Planes
						//Calculate AO
						

						//TRIANGLE 1
						//Bottom Left
						vertices.push_back({ glm::vec3((float)plane, (float)quad.y + (float)quad.h, (float)quad.x), //Position
							0,																						//Axis
							{0,quad.h} });																			//Width/Height
						//Top Left
						vertices.push_back({ glm::vec3((float)plane, (float)quad.y, (float)quad.x),
							0,
							{ 0,0 } });
						//Top Right
						vertices.push_back({ glm::vec3((float)plane, (float)quad.y, (float)quad.x + (float)quad.w),
							0,
							{quad.w,0} });
						//TRIANGLE 2
						//Bottom Left
						vertices.push_back({ glm::vec3((float)plane, (float)quad.y + (float)quad.h, (float)quad.x),
							0,
							{0,quad.h} });
						//Top Right
						vertices.push_back({ glm::vec3((float)plane, (float)quad.y, quad.x + (float)quad.w),
							0,
							{quad.w,0} });
						//Bottom Right
						vertices.push_back({ glm::vec3((float)plane, (float)quad.y + (float)quad.h, quad.x + (float)quad.w),
							0,
							{quad.w,quad.h} });
						break;
					case 1: //X-Backwards Planes
						//TRIANGLE 1
						//Top Right
						vertices.push_back({ glm::vec3((float)plane + 1.f, (float)quad.y, (float)quad.x + (float)quad.w),
							1,
							{quad.w,0} });
						//Top Left
						vertices.push_back({ glm::vec3((float)plane + 1.f, (float)quad.y, (float)quad.x),
							1,
							{0,0} });
						//Bottom Left
						vertices.push_back({ glm::vec3((float)plane + 1.f, (float)quad.y + (float)quad.h, (float)quad.x),
							1,
							{0,quad.h} });

						//TRIANGLE 2
						//Bottom Right
						vertices.push_back({ glm::vec3((float)plane + 1.f, (float)quad.y + (float)quad.h, quad.x + (float)quad.w),
							1,
							{quad.w,quad.h} });
						//Top Right
						vertices.push_back({ glm::vec3((float)plane + 1.f, (float)quad.y, quad.x + (float)quad.w),
							1,
							{quad.w,0} });
						//Bottom Left
						vertices.push_back({ glm::vec3((float)plane + 1.f, (float)quad.y + (float)quad.h, (float)quad.x),
							1,
							{0,quad.h} });
						break;
					case 2: //Z-Right Planes
						//TRIANGLE 1
						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)quad.y, (float)plane),
							2,
							{quad.w,0} });
						//Top Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)quad.y, (float)plane),
							2,
							{0,0} });
						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)quad.y + (float)quad.h, (float)plane),
							2,
							{0,quad.h} });

						//TRIANGLE 2
						//Bottom Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)quad.y + (float)quad.h, (float)plane),
							2,
							{quad.w,quad.h} });
						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)quad.y, (float)plane),
							2,
							{quad.w,0} });
						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)quad.y + (float)quad.h, (float)plane),
							2,
							{0,quad.h} });

						break;
					case 3: //Z-Left Planes
						//TRIANGLE 1
						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)quad.y + (float)quad.h, (float)plane + 1.f),
							3,
							{0,quad.h} });
						//Top Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)quad.y, (float)plane + 1.f),
							3,
							{0,0} });
						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)quad.y, (float)plane + 1.f),
							3,
							{quad.w,0} });
						//TRIANGLE 2
						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)quad.y + (float)quad.h, (float)plane + 1.f),
							3,
							{0,quad.h} });
						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)quad.y, (float)plane + 1.f),
							3,
							{quad.w,0} });
						//Bottom Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)quad.y + (float)quad.h, (float)plane + 1.f),
							3,
							{quad.w,quad.h} });
						break;
					case 4: //Y-Up Planes
						//TRIANGLE 1
						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)plane, (float)quad.y + (float)quad.h),
							4,
							{0,quad.h} });
						//Top Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)plane, (float)quad.y),
							4,
							{0,0} });
						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)plane, (float)quad.y),
							4,
							{quad.w,0} });
						//TRIANGLE 2
						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)plane, (float)quad.y + (float)quad.h),
							4,
							{0,quad.h} });
						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)plane, (float)quad.y),
							4,
							{quad.w,0} });
						//Bottom Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)plane, (float)quad.y + (float)quad.h),
							4,
							{quad.w,quad.h} });

						break;
					case 5: //Y-Down Planes

						//TRIANGLE 1

						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)plane + 1.f, (float)quad.y),
							5,
							{quad.w,0} });

						//Top Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)plane + 1.f, (float)quad.y),
							5,
							{0,0} });

						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)plane + 1.f, (float)quad.y + (float)quad.h),
							5,
							{0,quad.h} });

						//TRIANGLE 2
						//Bottom Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)plane + 1.f, (float)quad.y + (float)quad.h),
							5,
							{quad.w,quad.h} });

						//Top Right
						vertices.push_back({ glm::vec3((float)quad.x + (float)quad.w, (float)plane + 1.f, (float)quad.y),
							5,
							{quad.w,0} });

						//Bottom Left
						vertices.push_back({ glm::vec3((float)quad.x, (float)plane + 1.f, (float)quad.y + (float)quad.h),
							5,
							{0,quad.h} });
						break;
					}
					//vertices.push_back(glm::vec3(quad.x,3,4))
				}
			}

		}

	}
}

void chunk::updateChunkLighting(uint16* field)
{

	glm::ivec3 lightVoxelCoords = glm::ivec3(1, 0, 1); //Position of light
	unsigned int lightLevel = 6; //Strength of light

	glm::ivec3 localLightCoords = lightVoxelCoords - 16 * chunkCoords;


	//TODO
	// Update to use thread pool
	// Update to read from light manager


	if (field)
	{
		// We have light data already and want to flood without worrying about lights
		lightCalcThread = std::thread([this, localLightCoords, lightLevel]() {floodBlockLighting(localLightCoords, lightLevel, lightfield); });
	}


	//For Lights in LightManager
		//Get Light VoxelCoords
		//if LightVoxelCoords-(16.0*chunkCoords) > -17 and < 16 then
			
			//If LightCoords inside this chunk then start flooding
				//flood
					//If flood tries to go out of bounds then mark a chunk for updateChunkLighting
			//else
	// For...
		// GetCoords...
	//For now assume we have a light at 1,0,1


	//Light Data
	
	if ((abs(localLightCoords.x) <= lightLevel) and (abs(localLightCoords.y) <= lightLevel) and (abs(localLightCoords.z) <= lightLevel))
	{
		//The coordinates where the light started
		glm::ivec3 startingChunkCoords = chunkCoords - glm::ivec3((int)localLightCoords.x < 0, (int)localLightCoords.y < 0, (int)localLightCoords.z<0);
		if (startingChunkCoords == chunkCoords) {
			
			
			
			//Calculate this chunks lighting
			
			if (field)
			{
				lightCalcThread = std::thread([this, localLightCoords, lightLevel]() {floodBlockLighting(localLightCoords, lightLevel,lightfield); });
			}
			else
			{
				lightCalcThread = std::thread([this, localLightCoords, lightLevel]() {floodBlockLighting(localLightCoords, lightLevel); });
			}
			//floodBlockLighting(localLightCoords, allowedDirections, 8);
			
			//...neighbouringChunk->updateChunkLighting()
		}
		
		
	}
	lightUpdated = false;
}

std::vector<uint32> chunk::floodBlockLighting(glm::ivec3 voxelPos, unsigned int lightStrength, uint16* field)
{
	unsigned int x = voxelPos.x;
	unsigned int y = voxelPos.y;
	unsigned int z = voxelPos.z;

	std::vector<node*> neighbourToUpdate;
	neighbourToUpdate.resize(6);

	//Compare light to overlap map and remove light bit if necessary
	unsigned int currLightLevel = lightStrength;
	std::map<unsigned int,uint16*> lightfieldLevels; //Contains different lightfield levels to saturate
	std::lock_guard<std::mutex> lk(lightMutex);
	if (field)
	{
		
		for (int vX = 0; vX < 16; vX++)
		{
			for (int vY = 0; vY < 16; vY++)
			{
				for (int vZ = 0; vZ < 16; vZ++)
				{
					if (((*(field + vX + 16 * (vY)) >> vZ) & 0b1) > 0) //Essentially if we have light in the field detected
					{
						int potentialLightStrength = getBlock(vX, vY, vZ).lightLevel;
						if (lightfieldLevels.contains(potentialLightStrength)) //If we've already detected a voxel at this light level
						{
							*(lightfieldLevels[potentialLightStrength] + vX + (16 * vY)) |= 0b1 << vZ; //Write in the lightfield bit
						}
						else //We don't have this lightlevel yet found
						{
							lightfieldLevels[potentialLightStrength] = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
							*(lightfieldLevels[potentialLightStrength] + vX + (16 * vY)) |= 0b1 << vZ;
						}
					}
				}
			}
		}
		//Go through field and split it into separate bits (based upon lightStrength) that can be read into a map, then each time that we decrease currLightLevel,
		//We read in the bits of the map
		//std::cout << "Field:Line 775\n";
		
	}
	else //Default lightfield (empty), i.e we're not reading from a neighbour
	{
		
		
		if (getBlock(voxelPos.x, voxelPos.y, voxelPos.z).lightLevel >= lightStrength) return lightingIndices; //No point in doing duplicate calculations
		if ((lightStrength - 1 < 16) and (getBlock(voxelPos.x, voxelPos.y, voxelPos.z).lightLevel < lightStrength - 1))
		{
			getBlock(voxelPos.x, voxelPos.y, voxelPos.z).lightLevel = lightStrength;
		}
		lightfieldLevels[currLightLevel] = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
		*(lightfieldLevels[currLightLevel] + x + (y * 16)) = 0b1 << z;
	}
	
	
	
	

	
	//Represent Light as single bit in lightField
	//Find New Bits in lightField by comparing the current lightField and old Lightfield and set the blocks lightLevel to lightStrength (if it's higher)
	//Reduce Light Strength by 1
	//Saturate (spread) out
	//Then compare saturated light to solid block bitmap and copy into an overlap map
	//Saturate the overlap map based upon its local vector

	
	if (lightfield) delete[] lightfield;
	lightfield = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
	uint16* oldLightfield = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
	
	//For Readability
	

	//*(lightfield+x + (y * 16)) = (0b1 << z); //Set the light origin to its position
	
	uint16* overlapMap = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
	memset(overlapMap, 0, 256*sizeof(*overlapMap));

	//std::cout << "StartLightSpread:Line 816\n";
	while (currLightLevel > 0)
	{	
		//Read in lightfieldLevel
		if (lightfieldLevels.contains(currLightLevel))
		{
			for (uint16 dy = 0; dy < 16; dy++)
			{
				for (uint16 dx = 0; dx < 16; dx++)
				{
					if (!lightfield) lightfield = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 }; //Create lightfield if not created
					*(lightfield + dx + (dy * 16)) = *(lightfieldLevels[currLightLevel] + dx + (dy * 16));
					//delete[] (lightfieldLevels[currLightLevel]);
				}
			}
		}
		
		//Change the light of the blocks
		uint16 newLights[16 * 16]{};
		for (uint16 dy = 0; dy < 16; dy++)
		{
			for (uint16 dx = 0; dx < 16; dx++)
			{
				if (*(lightfield + dx + (dy * 16)) == 0) 
					continue; 
				
				newLights[dx + (16 * dy)] = *(lightfield + dx+(16*dy)) & ~*(oldLightfield + dx + (16 * dy)); // Copy in the newlights
				
				
				
				uint16 e = (axis_col[(dx + 1) + ((dy + 1) * 18) + chunk_size_p2].data >> 1);
				//Find where solid blocks overlaps with light blocks we are trying to make
				*(overlapMap + dx + (16 * dy)) |= ((newLights[dx + (16 * dy)]) & (axis_col[(dx + 1) + ((dy + 1) * 18) + chunk_size_p2].data>>1));
				
				
				//Get a map of projected light trying to overlap with solid blocks
				//if (*(overlapMap + dx + (16 * dy)))
				//{
				//	//Saturate the Overlap Map
				//	//X Direction
				//	if (dx >= x and dx < 15) //If current X is greater than or equal to lightsource X
				//	{
				//		//Grow in positive x Direction
				//		*(overlapMap + dx + 1 + (16 * dy)) = *(overlapMap + dx + (16 * dy));

				//	}
				//	if (dx <= x and dx > 0) //If current X is less than or equal to lightsource X
				//	{
				//		//Grow in negative x Direction
				//		*(overlapMap + dx - 1 + (16 * dy)) = *(overlapMap + dx + (16 * dy));

				//	}

				//	//Y Direction

				//	if (dy >= y and dy < 15) //If current Y is greater than or equal to lightsource Y
				//	{
				//		//Grow in positive y Direction
				//		*(overlapMap + dx + (16 * (dy + 1))) = *(overlapMap + dx + (16 * dy));
				//	}
				//	if (dy <= y and dy > 0) //If current Y is less than or equal to lightsource Y
				//	{
				//		//Grow in negative y Direction
				//		*(overlapMap + dx + (16 * (dy - 1))) = *(overlapMap + dx + (16 * dy));
				//	}

				//	// Z Direction

				//	//Grow in positive z Direction
				//	if (z >= *(overlapMap + dx + (16 * dy)))
				//	{
				//		*(overlapMap + dx + (16 * dy)) |= *(overlapMap + dx + (16 * dy)) << 1;
				//	}


				//	//Grow in negative z Direction
				//	if (z <= *(overlapMap + dx + (16 * dy)))
				//	{
				//		*(overlapMap + dx + (16 * dy)) |= *(overlapMap + dx + (16 * dy)) >> 1;
				//	}
				//}
				
				

				//Now that the overlap map is saturated, we use it as a mask to hide certain areas on new lights;

				newLights[dx + (16 * dy)] &= ~*(overlapMap + dx + (16 * dy));
				*(lightfield + dx + (16 * dy)) &= ~*(overlapMap + dx + (16 * dy));
				//Now we write new Lights into their respective blocks in chunkData
				for (int dz = 0; dz < 16; dz++)
				{
					if ((newLights[dx + 16 * dy] & (0b1 << dz)) > 0)
					{
						if (auto blockPtr = (chunkData + dz + (dy * 16) + (dx * 256)); blockPtr->lightLevel < currLightLevel)
						{
							blockPtr->lightLevel = currLightLevel;
						}
						
					}
				}
				

				
			}
			
		}
		//std::cout << "LightSpreadHalfPoint:Line 922\n";
		for (int i=0; i<256;i++)
			*(oldLightfield + i) = *(lightfield + i); //Update the oldLights to match the new
		for (uint16 dy = 0; dy < 16; dy++)
		{
			for (uint16 dx = 0; dx < 16; dx++)
			{
				//Saturate newLights in X Direction
				if (dx < 15)
					*(lightfield + dx + 1 + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0)));
				else if (auto neighbourChunk = reinterpret_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x + 1, chunkCoords.y, chunkCoords.z); !field and neighbourChunk)
				{
					//NeighbourChunk should always have a lightfield since it's created inside the constructor
					*(neighbourChunk->lightfield + 0 + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0)));
					if (*(lightfield + dx + (16 * (dy + 0))) != 0) //Update the light levels of blocks
					{
						int count = 0;
						for (uint32 intSlice = *(lightfield + dx + (16 * (dy + 0))); intSlice != 0; intSlice=intSlice >> 1)
						{
							auto blockPtr = (neighbourChunk->chunkData + ((intSlice & 0b1) * count) + 16 * dy + (256 * 0));
							blockPtr->lightLevel = currLightLevel>blockPtr->lightLevel ? currLightLevel : blockPtr->lightLevel;
							count++;
						}
						neighbourToUpdate[0] = neighbourChunk;
					}
					
				}
				if (dx > 0)
					*(lightfield + dx - 1 + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0)));
				else if (auto neighbourChunk = reinterpret_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x - 1, chunkCoords.y, chunkCoords.z); !field and neighbourChunk)
				{
					//NeighbourChunk should always have a lightfield since it's created inside the constructor
					*(neighbourChunk->lightfield + 15 + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0)));

					if (*(lightfield + dx + (16 * (dy + 0))) != 0) //Update the light levels of blocks
					{
						int count = 0;
						for (uint32 intSlice = *(lightfield + dx + (16 * (dy + 0))); intSlice != 0; intSlice = intSlice >> 1)
						{
							auto blockPtr = (neighbourChunk->chunkData + ((intSlice & 0b1) * count) + 16 * dy + (256 * 15));
							blockPtr->lightLevel = currLightLevel > blockPtr->lightLevel ? currLightLevel : blockPtr->lightLevel;
							count++;
						}
						neighbourToUpdate[1] = neighbourChunk;
					}
					
				}
				//Now in Y
				//Saturate newLights in Y Direction
				if (dy < 15)
					*(lightfield + dx + 0 + (16 * (dy + 1))) |= *(lightfield + dx + (16 * (dy + 0)));
				else if (auto neighbourChunk = reinterpret_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x, chunkCoords.y + 1, chunkCoords.z); !field and neighbourChunk)
				{
					//NeighbourChunk should always have a lightfield since it's created inside the constructor
					*(neighbourChunk->lightfield + dx + (16 * (0))) |= *(lightfield + dx + (16 * (dy + 0)));

					if (*(lightfield + dx + (16 * (dy + 0))) != 0) //Update the light levels of blocks
					{
						int count = 0;
						for (uint32 intSlice = *(lightfield + dx + (16 * (dy + 0))); intSlice != 0; intSlice = intSlice >> 1)
						{
							auto blockPtr = (neighbourChunk->chunkData + ((intSlice & 0b1) * count) + 16 * 0 + (256 * dx));
							blockPtr->lightLevel = currLightLevel > blockPtr->lightLevel ? currLightLevel : blockPtr->lightLevel;
							count++;
						}
						neighbourToUpdate[2] = neighbourChunk;
					}
				}
				if (dy > 0)
					*(lightfield + dx + 0 + (16 * (dy - 1))) |= *(lightfield + dx + (16 * (dy + 0)));
				else if (auto neighbourChunk = reinterpret_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x, chunkCoords.y-1, chunkCoords.z); !field and neighbourChunk)
				{
					//NeighbourChunk should always have a lightfield since it's created inside the constructor
					*(neighbourChunk->lightfield + dx + (16 * (15))) |= *(lightfield + dx + (16 * (dy + 0)));

					if (*(lightfield + dx + (16 * (dy + 0))) != 0) //Update the light levels of blocks
					{
						int count = 0;
						for (uint32 intSlice = *(lightfield + dx + (16 * (dy + 0))); intSlice != 0; intSlice = intSlice >> 1)
						{
							auto blockPtr = (neighbourChunk->chunkData + ((intSlice & 0b1) * count) + 16 * 15 + (256 * dx));
							blockPtr->lightLevel = currLightLevel > blockPtr->lightLevel ? currLightLevel : blockPtr->lightLevel;
							count++;
						}
						neighbourToUpdate[3] = neighbourChunk;
					}
				}

				//Now in Z we just bitshift left and right to grow in Z

				//Check Right most bit should shift into adjacent chunk
				if (*(lightfield + dx + (16 * (dy + 0))) & 0b1)
				{
					if (auto neighbourChunk = reinterpret_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z - 1); !field and neighbourChunk)
					{
						*(neighbourChunk->lightfield + dx + (16 * (dy))) |= 0b1000000000000000;
						auto blockPtr = (neighbourChunk->chunkData + 15 + (16 * dy) + (256 * dx));
						blockPtr->lightLevel = currLightLevel > blockPtr->lightLevel ? currLightLevel : blockPtr->lightLevel;
						neighbourToUpdate[4] = neighbourChunk;
					}
				}

				//Check Left most bit should shift into adjacent chunk
				if (*(lightfield + dx + (16 * (dy + 0))) & 0b1000000000000000)
				{
					if (auto neighbourChunk = reinterpret_cast<ChunkSpace*>(parent)->getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z - 1); !field and neighbourChunk)
					{
						*(neighbourChunk->lightfield + dx + (16 * (dy))) |= 0b1;
						auto blockPtr = (neighbourChunk->chunkData + 0 + (16 * dy) + (256 * dx));
						blockPtr->lightLevel = currLightLevel > blockPtr->lightLevel ? currLightLevel : blockPtr->lightLevel;
						neighbourToUpdate[5] = neighbourChunk;
					}
				}
				*(lightfield + dx + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0))) >> 1;
				*(lightfield + dx + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0))) << 1;

			}
		}
		//Reduce the light level
		currLightLevel -= 1;
		//std::cout << "LightLevelDone:Line 1042\n";
	}

	//std::cout << "StartLightIndices:Line 1045\n";
	
	lightingIndices.clear();
	lightingIndices.reserve(CHUNKSIZE* CHUNKSIZE* CHUNKSIZE * 6);
	
	for (int x = 0; x < CHUNKSIZE; x++)
	{
		for (int y = 0; y < CHUNKSIZE; y++)
		{
			for (int z = 0; z < CHUNKSIZE; z++)
			{
				// -- LIGHTING -- //
				//X-Positive
				if ((axis_col[(x + 0) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X-1 (X+0 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					/*auto a = getBlock(x - 1, y, z).lightLevel;
					if (a > 5)
					{
						std::cout << "A!";
					}*/

					lightingIndices.push_back(getBlock(x - 1, y, z).lightLevel);
				}

				//X-Negative
				if ((axis_col[(x + 2) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x + 1, y, z).lightLevel);
				}

				//Z-Positive
				if ((axis_col[(x + 1) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y, z - 1).lightLevel);
				}

				//Z-Negative
				if ((axis_col[(x + 1) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y, z + 1).lightLevel);
				}

				//Y-Positive
				if ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y - 1, z).lightLevel);
				}

				//Y-Negative
				if ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y + 1, z).lightLevel);
				}
			}
		}
	}
	
	//std::cout << "MarkingNeighbours:Line 1126\n";
	try
	{
		
		for (int neighbour = 0; neighbour < neighbourToUpdate.size();neighbour++) //Mark neighbours to update
		{
			if ((neighbourToUpdate.at(neighbour)))
			{
				if (neighbourToUpdate[neighbour]) reinterpret_cast<chunk*>(neighbourToUpdate[neighbour])->alertUpdateLight();
			}
			
			
		}
		
		
	}
	catch (std::exception e)
	{
		std::cout << "Error Marking Neighbours"<<std::endl;
	}
	//neighbourToUpdate.clear();
	
	//Cleanup arrays before leaving scope
	
	delete[] oldLightfield;
	delete[] overlapMap;
	lightDispatchCompleted = true;
	
	return lightingIndices;
}

void chunk::fastBlockFloodLighting(light& currLight)
{
	if (chunkQuads.size() == 0) return; //We don't want to calculate on empty/full chunks
	unsigned int x = currLight.pos.x;
	unsigned int y = currLight.pos.y;
	unsigned int z = currLight.pos.z;

	std::vector<node*> neighbourToUpdate;
	neighbourToUpdate.resize(6);

	//Compare light to overlap map and remove light bit if necessary
	int currLightLevel = currLight.strength;
	std::map<unsigned int, uint16*> lightfieldLevels; //Contains different lightfield levels to saturate
	uint16* overlapMap = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
	uint16* oldLightfield = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
	std::unique_lock<std::mutex> lock(lightMutex);
	if (lightfield)
	{
		
		auto a = *(lightfield + 1);
		memset(lightfield, 0, 256 * sizeof(*lightfield)); //Set array to 0
		
	}
	
	if (currLightLevel > 0)
	{
		getBlock(x, y, z).lightLevel = currLightLevel;

	}
	else
	{
		getBlock(x, y, z).lightLevel = -1;
		for (int i = 0; i < 15; i++) //For each potentialLightStrength
		{
			//Remove the light (including from neighbouring chunks)
			for (uint16 dy = 0; dy < 16; dy++)
			{
				for (uint16 dx = 0; dx < 16; dx++)
				{
					for (uint16 dz = 0; dz < 16; dz++)
					{
						if (getBlock(dx, dy, dz).lightLevel == -1)
						{
							auto addr = getBlockAddr(dx, dy, dz);
							//Z
							if (dz > 0)
							{
								auto a = (addr -1);
								a->lightLevel > 0 ? a->lightLevel = -1 : a->lightLevel = 0;
							}
							else
							{
								auto a = getBlock(dx, dy, dz-1);
								a.lightLevel > 0 ? a.lightLevel = -1 : a.lightLevel = 0;
							}
							if (dz < 15)
							{
								auto a = (addr + 1);
								a->lightLevel > 0 ? a->lightLevel = -1 : a->lightLevel = 0;
							}
							else
							{
								auto a = getBlock(dx, dy, dz+1);
								a.lightLevel > 0 ? a.lightLevel = -1 : a.lightLevel = 0;
							}
							//Y
							if (dy > 0)
							{
								auto a = (addr - 16);
								a->lightLevel > 0 ? a->lightLevel = -1 : a->lightLevel = 0;
							}
							else
							{
								auto a = getBlock(dx, dy-1, dz);
								a.lightLevel > 0 ? a.lightLevel = -1 : a.lightLevel = 0;
							}
							if (dy < 15)
							{
								auto a = (addr + 16);
								a->lightLevel > 0 ? a->lightLevel = -1 : a->lightLevel = 0;
							}
							else
							{
								auto a = getBlock(dx, dy + 1, dz);
								a.lightLevel > 0 ? a.lightLevel = -1 : a.lightLevel = 0;
							}
							//X
							if (dx > 0)
							{
								auto a = (addr - 256);
								a->lightLevel > 0 ? a->lightLevel = -1 : a->lightLevel = 0;;
							}
							else
							{
								auto a = getBlock(dx-1, dy, dz);
								a.lightLevel > 0 ? a.lightLevel = -1 : a.lightLevel = 0;
							}
							if (dx < 15)
							{
								auto a = (addr + 256);
								a->lightLevel > 0 ? a->lightLevel = -1 : a->lightLevel = 0;;
							}
							else
							{
								auto a = getBlock(dx+1, dy, dz);
								a.lightLevel > 0 ? a.lightLevel = -1 : a.lightLevel = 0;
							}
						}
					}
				}
			}

		}
	}
	for (int level=0;level<16;level++) lightfieldLevels[level] = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 };
	for (int vX = 0; vX < 16; vX++)
	{
		for (int vY = 0; vY < 16; vY++)
		{
			for (int vZ = 0; vZ < 16; vZ++)
			{
				block potentialLightStrength = getBlock(vX, vY, vZ);
				if (vX == 0 || vX == 15 || vY == 0 || vY == 15 || vZ == 0 || vZ == 15)
				{
					if (vX == 0)
					{
						if (auto neighbourBlock = getBlock(vX - 1, vY, vZ); neighbourBlock.lightLevel > 0)
						{
							potentialLightStrength.lightLevel = neighbourBlock.lightLevel - 1;
						}
					}
					if (vX == 15)
					{
						if (auto neighbourBlock = getBlock(vX + 1, vY, vZ); neighbourBlock.lightLevel > 0)
						{
							potentialLightStrength.lightLevel = neighbourBlock.lightLevel - 1;
						}
					}
					if (vY == 0)
					{
						if (auto neighbourBlock = getBlock(vX, vY - 1, vZ); neighbourBlock.lightLevel > 0)
						{
							potentialLightStrength.lightLevel = neighbourBlock.lightLevel - 1;
						}
					}
					if (vY == 15)
					{
						if (auto neighbourBlock = getBlock(vX, vY + 1, vZ); neighbourBlock.lightLevel > 0)
						{
							potentialLightStrength.lightLevel = neighbourBlock.lightLevel - 1;
						}
					}
					if (vZ == 0)
					{
						if (auto neighbourBlock = getBlock(vX, vY, vZ - 1); neighbourBlock.lightLevel > 0)
						{
							potentialLightStrength.lightLevel = neighbourBlock.lightLevel - 1;
						}
					}
					if (vZ == 15)
					{
						if (auto neighbourBlock = getBlock(vX, vY, vZ + 1); neighbourBlock.lightLevel > 0)
						{
							potentialLightStrength.lightLevel = neighbourBlock.lightLevel - 1;
						}
					}

				}
				else
				{


					


					*(lightfieldLevels[potentialLightStrength.lightLevel] + vX + (16 * vY)) |= 0b1 << vZ; //Write in the lightfield bit
				}
				
			}
		}
	}
	
	
	//Saturate

	for (int i = 0; i < 15; i++) //For each potentialLightStrength
	{
		
		uint16 newLights[16 * 16]{};
		if (!lightfield) lightfield = new uint16[CHUNKSIZE * CHUNKSIZE]{ 0 }; //Create lightfield if not created

		if (lightfieldLevels.contains(currLightLevel-i))
		{
			for (uint16 dy = 0; dy < 16; dy++)
			{
				for (uint16 dx = 0; dx < 16; dx++)
				{
					
					*(lightfield + dx + (dy * 16)) = *(lightfieldLevels[currLightLevel-i] + dx + (dy * 16));
					//delete[] (lightfieldLevels[currLightLevel]);
				}
			}
		}

		for (uint16 dy = 0; dy < 16; dy++)
		{
			for (uint16 dx = 0; dx < 16; dx++)
			{
				if (*(lightfield + dx + (dy * 16)) == 0)
					continue;

				newLights[dx + (16 * dy)] = *(lightfield + dx + (16 * dy)) & ~*(oldLightfield + dx + (16 * dy)); // Copy in the newlights



				//uint16 e = (axis_col[(dx + 1) + ((dy + 1) * 18) + chunk_size_p2].data >> 1);
				//Find where solid blocks overlaps with light blocks we are trying to make
				*(overlapMap + dx + (16 * dy)) |= ((newLights[dx + (16 * dy)]) & (axis_col[(dx + 1) + ((dy + 1) * 18) + chunk_size_p2].data >> 1));

				newLights[dx + (16 * dy)] &= ~*(overlapMap + dx + (16 * dy)); //Remove the 1s in overlapMap from newLights


				*(lightfield + dx + (16 * dy)) &= ~*(overlapMap + dx + (16 * dy));
				//Now we write new Lights into their respective blocks in chunkData
				for (int dz = 0; dz < 16; dz++)
				{
					if ((newLights[dx + 16 * dy] & (0b1 << dz)) > 0)
					{
						
						if (auto blockPtr = getBlockAddr(dx,dy,dz); blockPtr->lightLevel < currLightLevel && blockPtr->lightLevel!=-1)
						{

							blockPtr->lightLevel = std::max(0,currLightLevel);
						}
						if (auto blockPtr = (chunkData + dz + (dy * 16) + (dx * 256)); blockPtr->lightLevel == -1)
						{
							blockPtr->lightLevel = 0;
						}
					}
				}


				//Now spread outwards
				if (dx < 15) //Spread Right
				{
					*(lightfield + dx + 1 + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0)));
				}
				if (dx > 0) // Spread Left
				{
					*(lightfield + dx - 1 + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0)));
				}

				if (dy < 15) //Spread Up
				{
					*(lightfield + dx + 0 + (16 * (dy + 1))) |= *(lightfield + dx + (16 * (dy + 0)));
				}
				if (dy > 0) // Spread Down
				{
					*(lightfield + dx + 0 + (16 * (dy - 1))) |= *(lightfield + dx + (16 * (dy + 0)));
				}

				*(lightfield + dx + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0))) >> 1; //Spread Forward/Back
				*(lightfield + dx + (16 * (dy + 0))) |= *(lightfield + dx + (16 * (dy + 0))) << 1; 

			}
		}


		


	}
	
	lightingIndices.clear();
	lightingIndices.reserve(CHUNKSIZE* CHUNKSIZE* CHUNKSIZE * 6);

	for (int x = 0; x < CHUNKSIZE; x++)
	{
		for (int y = 0; y < CHUNKSIZE; y++)
		{
			for (int z = 0; z < CHUNKSIZE; z++)
			{
				// -- LIGHTING -- //
				//X-Positive
				if ((axis_col[(x + 0) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X-1 (X+0 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					/*auto a = getBlock(x - 1, y, z).lightLevel;
					if (a > 5)
					{
						std::cout << "A!";
					}*/

					lightingIndices.push_back(getBlock(x - 1, y, z).lightLevel);
				}

				//X-Negative
				if ((axis_col[(x + 2) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x + 1, y, z).lightLevel);
				}

				//Z-Positive
				if ((axis_col[(x + 1) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y, z - 1).lightLevel);
				}

				//Z-Negative
				if ((axis_col[(x + 1) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y, z + 1).lightLevel);
				}

				//Y-Positive
				if ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y - 1, z).lightLevel);
				}

				//Y-Negative
				if ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) //If X+1 (X+2 since xyz starts -1,-1,-1) block is solid
				{
					lightingIndices.push_back(0); //Surface us covered in solid so it has no light
				}
				else
				{
					lightingIndices.push_back(getBlock(x, y + 1, z).lightLevel);
				}
			}
		}
	}


	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexSSBO);


	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, lightingIndices.size() * sizeof(uint32), &lightingIndices[0]); //
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	lightDispatchCompleted = true;
	std::cout << "FastLightUpdateDone" << std::endl;

}








int chunk::trailingZeros(const uint16& intRef)
{
	//Returns Trailing Zeros
	return std::countr_zero(intRef);
}

int chunk::trailingOnes(const uint16& intRef)
{
	return std::countr_one(intRef);
}

std::optional<uint16> chunk::checkedShl(uint16 value, int shift)
{
	if (shift >= 0 && shift < 16) //Maximum/Minimum size of bits to shift
	{
		return value << shift;
	}
	return std::nullopt; //Fail the function
}

void chunk::tick()
{
	node::tick();
	if(geomUpdated)
	{
		//Update Geometry

		updateGeom();
		neighbourUpdated = false; //If we've been marked already by a neighbour to update then we no longer need to
		
	}
	if (neighbourUpdated)
	{
		updateGeom(false); //Update this chunk but we no longer want to update neghbouring chunks
		neighbourUpdated = false;
		
	}
	if (lightDispatchCompleted)
	{
		//glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexSSBO);


		//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, lightingIndices.size() * sizeof(uint32), &lightingIndices[0]); //Try put data from std::future int
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		
		
	}
	if (lightUpdated) //Lightfield has been updated by neighbour
	{
		if (lightCalcThread.joinable())
		{
			std::cout << "Join:1235\n";
			lightCalcThread.join();
			
		}
		
		
		updateChunkLighting(lightfield);
	}
	

	transform = static_cast<ChunkSpace*>(parent)->transform;
	transform = glm::translate(transform, glm::vec3((float)chunkCoords.x * 16.f, (float)chunkCoords.y * 16.f, (float)chunkCoords.z * 16.f));
}

void chunk::render(camera& currentCamera)
{
	node::render(currentCamera);
	if (vertices.size() == 0)
	{
		//Don't render
	}
	else
	{
		ResourceManager::GetShader("triangle")->Use(); //Select and use (via glUseProgram) the correct shader.


		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)1280.f / (float)720.f, 0.1f, 450.0f);
		glm::mat4 view = currentCamera.cameraView;



		//Make the texture resident
		ChunkTextures.makeResident();


		//Transform the trans matrix (model matrix)


		// --Bind Uniforms--
		// In this case our triangle only has 3 Uniforms, and they are all matrices.
		(*ResourceManager::GetShader("triangle")).SetMatrix4("transform", transform);
		(*ResourceManager::GetShader("triangle")).SetMatrix4("view", view);
		(*ResourceManager::GetShader("triangle")).SetMatrix4("projection", proj);
		(*ResourceManager::GetShader("triangle")).SetBindlessTextureHandle("textureHandle", ChunkTextures.getBindlessHandle());


		//Bind Buffers and draw call


		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, texIndexSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightIndexSSBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindVertexArray(chunkVAO); //Now bind openGL to the correct vertex array
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // Draw a triangle (starting at index 0 and increasing to 6 verts)
		if (auto err = glGetError(); err != GL_NO_ERROR) std::cerr << "OpenGL error: " << std::hex << err << std::endl; //Clean error detection line (limited info given however)
		glBindVertexArray(0); //Unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	
}

unsigned int chunk::prepareRender()
{
	
	
	

	//Bind OpenGL to VAO object
	glBindVertexArray(chunkVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	auto size = (vertices.size() * sizeof(float) * 3) + (vertices.size() * sizeof(unsigned int) * 3);
	if (vertices.size() == 0)
	{
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	}
	else
	{
		glBufferData(GL_ARRAY_BUFFER, size, &vertices[0], GL_DYNAMIC_DRAW);
	}
	
	

	//Add vertex attributes
	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertexData), (void*)0);
	glEnableVertexAttribArray(0);
	//Axis
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(vertexData), (void*)(offsetof(vertexData, axis)));
	glEnableVertexAttribArray(1);
	//TexCoords
	glVertexAttribIPointer(2, 2, GL_UNSIGNED_INT, sizeof(vertexData), (void*)(offsetof(vertexData, texCoords)));
	glEnableVertexAttribArray(2);
	
	;

	
	
	

	//Find Exact amount of voxel textures we need to store in SSBO
	uint32 totalTextures = knownTextures.size()*6; //One texture per face
	
	//glDeleteBuffers(1, &texIndexSSBO);
	//glGenBuffers(1, &texIndexSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, texIndexSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, texIndexSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32) * CHUNKSIZE * CHUNKSIZE * CHUNKSIZE * 12, nullptr, GL_DYNAMIC_DRAW);
	

	textureIndices.clear();
	//Populate SSBO with texture indices and add the image data to the texture
	for (int x = 0; x < CHUNKSIZE; x++)
	{
		for (int y = 0; y < CHUNKSIZE; y++)
		{
			for (int z = 0; z < CHUNKSIZE; z++)
			{
				if (getBlock(x, y, z).solid == false)
				{
					//Texture Indices
					textureIndices.push_back(uint32(-1)); //For now just push back texture -1 to clear pixels
					textureIndices.push_back(uint32(-1));
					textureIndices.push_back(uint32(-1));
					textureIndices.push_back(uint32(-1));
					textureIndices.push_back(uint32(-1));
					textureIndices.push_back(uint32(-1));
					//Ambient Occlusion
					textureIndices.push_back(uint32(0)); //Ambient Occlusion Value (per voxel)
					textureIndices.push_back(uint32(0)); //Ambient Occlusion Value (per voxel)
					textureIndices.push_back(uint32(0)); //Ambient Occlusion Value (per voxel)
					textureIndices.push_back(uint32(0)); //Ambient Occlusion Value (per voxel)
					textureIndices.push_back(uint32(0)); //Ambient Occlusion Value (per voxel)
					textureIndices.push_back(uint32(0)); //Ambient Occlusion Value (per voxel)
					
					continue;
				}

				// -- TEXTURES -- //

				auto search = std::find(knownTextures.begin(), knownTextures.end(), getBlock(x, y, z).id);
				auto index = std::distance(knownTextures.begin(), search);
				textureIndices.push_back(uint32((index * 6) + 0)); //For now just push back texture 0, eventually will use lookup to map position to blocks
				textureIndices.push_back(uint32((index * 6) + 1));
				textureIndices.push_back(uint32((index * 6) + 2));
				textureIndices.push_back(uint32((index * 6) + 3));
				textureIndices.push_back(uint32((index * 6) + 4));
				textureIndices.push_back(uint32((index * 6) + 5));

				// -- AMBIENT OCCLUSION -- //

				//Start finding occluded blocks
				unsigned int BlockSurrounding = 0;
				uint32 s = 0;
				// X-Positive
				//Bottom Left Vertex
				BlockSurrounding |= ((axis_col[(x)+((y+1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 0; // Left Block
				BlockSurrounding |= ((axis_col[(x)+((y) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 1; // Bottom Left Corner
				BlockSurrounding |= ((axis_col[(x)+((y) * 18) + chunk_size_p2].data & 0b1 << (z+1)) >> (z+1)) << 2; // Bottom Block
				
				
				//Bottom Right Vertex
				BlockSurrounding |= ((axis_col[(x)+((y) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 3; // Bottom Block
				BlockSurrounding |= ((axis_col[(x)+((y) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 4; // Bottom Right Corner
				BlockSurrounding |= ((axis_col[(x)+((y+1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 5; // Right Block
				//Top Right Vertex
				BlockSurrounding |= ((axis_col[(x)+((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 6; // Right Block
				BlockSurrounding |= ((axis_col[(x)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 7; // Top Right Corner
				BlockSurrounding |= ((axis_col[(x)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 8; // Top Block
				//Top Left Vertex
				BlockSurrounding |= ((axis_col[(x)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 9; // Top Block
				BlockSurrounding |= ((axis_col[(x)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 10; // Top Left Corner
				BlockSurrounding |= ((axis_col[(x)+((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 11; // Left Block

				textureIndices.push_back(BlockSurrounding); //Ambient Occlusion Value
				BlockSurrounding = 0;
				// X-Negative

				//Bottom Left Vertex
				BlockSurrounding |= ((axis_col[(x+2)+((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 0; // Left Block
				BlockSurrounding |= ((axis_col[(x+2)+((y) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 1; // Bottom Left Corner
				BlockSurrounding |= ((axis_col[(x+2)+((y) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 2; // Bottom Block
				//Bottom Right Vertex
				BlockSurrounding |= ((axis_col[(x+2)+((y) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 3; // Bottom Block
				BlockSurrounding |= ((axis_col[(x+2)+((y) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 4; // Bottom Right Corner
				BlockSurrounding |= ((axis_col[(x+2)+((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 5; // Right Block
				//Top Right Vertex
				BlockSurrounding |= ((axis_col[(x+2)+((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 6; // Right Block
				BlockSurrounding |= ((axis_col[(x+2)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 7; // Top Right Corner
				BlockSurrounding |= ((axis_col[(x+2)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 8; // Top Block
				//Top Left Vertex
				BlockSurrounding |= ((axis_col[(x+2)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 9; // Top Block
				BlockSurrounding |= ((axis_col[(x+2)+((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 10; // Top Left Corner
				BlockSurrounding |= ((axis_col[(x+2)+((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 11; // Left Block

				textureIndices.push_back(BlockSurrounding); //Ambient Occlusion Value
				BlockSurrounding = 0;
				//Z-Positive
				//Bottom Left Vertex
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 0; // Left Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 1; // Bottom Left Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 2; // Bottom Block
				//Bottom Right Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 3; // Bottom Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 4; // Bottom Right Corner
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 5; // Right Block
				//Top Right Vertex
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 6; // Right Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 7; // Top Right Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 8; // Top Block
				//Top Left Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 9; // Top Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 10; // Top Left Vertex
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z)) >> (z)) << 11; // Left Block

				textureIndices.push_back(BlockSurrounding); //Ambient Occlusion Value
				BlockSurrounding = 0;
				//Z-Negative
				//Bottom Left Vertex
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 0; // Left Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 1; // Bottom Left Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 2; // Bottom Block
				//Bottom Right Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 3; // Bottom Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 4; // Bottom Right Corner
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 5; // Right Block
				//Top Right Vertex
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 6; // Right Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 7; // Top Right Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z+2)) >> (z+2)) << 8; // Top Block
				//Top Left Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 9; // Top Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 10; // Top Left Vertex
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 1) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 11; // Left Block

				textureIndices.push_back(BlockSurrounding); //Ambient Occlusion Value
				BlockSurrounding = 0;
				//Y-Positive
				//Bottom Left Vertex
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 0; // Left Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 1; // Bottom Left Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 2; // Bottom Block
				//Bottom Right Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 3; // Bottom Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 4; // Bottom Right Corner
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 5; // Right Block
				//Top Right Vertex
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 6; // Right Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 7; // Top Right Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 8; // Top Block
				//Top Left Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 9; // Top Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 10; // Top Left Corner
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 0) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 11; // Left Block

				textureIndices.push_back(BlockSurrounding); //Ambient Occlusion Value
				BlockSurrounding = 0;
				//Y-Negative
				//Bottom Left Vertex
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 0; // Left Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 1; // Bottom Left Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 2; // Bottom Block
				//Bottom Right Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 3; // Bottom Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 0)) >> (z + 0)) << 4; // Bottom Right Corner
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 5; // Right Block
				//Top Right Vertex
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 6; // Right Block
				BlockSurrounding |= ((axis_col[(x + 2) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 7; // Top Right Corner
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 8; // Top Block
				//Top Left Vertex
				BlockSurrounding |= ((axis_col[(x + 1) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 9; // Top Block
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 2)) >> (z + 2)) << 10; // Top Left Corner
				BlockSurrounding |= ((axis_col[(x + 0) + ((y + 2) * 18) + chunk_size_p2].data & 0b1 << (z + 1)) >> (z + 1)) << 11; // Left Block
				textureIndices.push_back(BlockSurrounding); //Ambient Occlusion Value
				
				
				
			}
		}
	}
	
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, textureIndices.size() * sizeof(uint32), &textureIndices[0]);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, texIndexSSBO);
	//Unbind buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	
	//Lighting SSBO
	/*glDeleteBuffers(1, &lightIndexSSBO);
	glGenBuffers(1, &lightIndexSSBO);
	*/
	
	
	
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0); //Unbind Array buffer

	//Unbind Vertex Array Object so we don't end up (accidentally modifying it)
	glBindVertexArray(0);

	
	


	return chunkVAO;
}