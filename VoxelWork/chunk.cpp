#include "chunk.h"


chunk::chunk()
{
	chunkData = new block[16 * 16 * 16];
	
}

chunk::~chunk()
{
	
	delete[] chunkData;
}



void chunk::deleteBlock(uint16 x, uint16 y, uint16 z)
{
	if ((chunkData+(CHUNKSIZE* CHUNKSIZE *x+ CHUNKSIZE *y+z))->solid & 0b1) //Ptr + theIndex we are looking for
	{
		//Block is solid - remove from geom
		chunkSolid[x * CHUNKSIZE + y] = chunkSolid[x * CHUNKSIZE + y] & ~(0b1 << z); //E.g Z=3 -> 11110111

		//Mark Geom Updated
		geomUpdated = true;
	}
	else
	{
		; //Remove model etc
	}

	(chunkData + (CHUNKSIZE * CHUNKSIZE * x + CHUNKSIZE * y + z))->id = 0;
	(chunkData + (CHUNKSIZE * CHUNKSIZE * x + CHUNKSIZE * y + z))->solid = 0;
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

				chunkSolid[i * CHUNKSIZE + j] = chunkSolid[i * CHUNKSIZE + j] | (0b1 << k); //Shift left k bits so that we set it to solid
			}
		}
	}
	//Mark for geom update
	geomUpdated = true;
}

void chunk::updateGeom()
{
	///// ----PART 1 BUILD CHUNKMESH---- /////
	
	//Create chunkSolid
	for (uint32 x = 0; x < CHUNKSIZE+2; x++)
	{
		for (uint32 y = 0; y < CHUNKSIZE+2; y++)
		{
			for (uint32 z = 0; z < CHUNKSIZE+2; z++)
			{
				//If block is solid
				if (getBlock(x-1,y-1,z-1).solid) //If returned block is solid
				{
					//[z + (y * sizeof(chunk_p))]
					//X axis
					axis_col[z + (y * 18)].data |= (0b1 << x);

					// Y axis
					axis_col[y + (x * 18)+ chunk_size_p2].data |= (0b1 << z);

					// Z axis
					axis_col[z + (x * 18) + chunk_size_p2*2].data |= (0b1 << y);
				}
			}
		}
	}


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

	geomUpdated = false;
	
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
	

	///// ----PART 2 GREEDYMESH---- /////
	for (int i = 0; i < 6; i++) //Don't use auto& i here for readability ~ Once for each axis (+ve and -ve)
	{
		for (int plane = 0; plane < 16; plane++)
		{
			if (auto search = data[i].find(plane); search != data[i].end())
			{
				std::vector<greedyQuad> quads = greedyMeshBinaryPlane(data[i][plane]); //Plane found inside data, begin greedy meshing!
				//Now add to vertices based upon direction
				for (auto& quad : quads)
				{
					switch (i)
					{
					case 0: //X-Forwards Planes
						//TRIANGLE 1
						//Bottom Left
						vertices.push_back(glm::vec3((float)plane+1.f, (float)quad.y + (float)quad.h, (float)quad.x));
						//Top Left
						vertices.push_back(glm::vec3((float)plane+1.f, (float)quad.y, (float)quad.x));
						//Top Right
						vertices.push_back(glm::vec3((float)plane+1.f, (float)quad.y, (float)quad.x + (float)quad.w));
						//TRIANGLE 2
						//Bottom Left
						vertices.push_back(glm::vec3((float)plane+1.f, (float)quad.y + (float)quad.h, (float)quad.x));
						//Top Right
						vertices.push_back(glm::vec3((float)plane+1.f, (float)quad.y, quad.x + (float)quad.w));
						//Bottom Right
						vertices.push_back(glm::vec3((float)plane+1.f, (float)quad.y + (float)quad.h, quad.x + (float)quad.w));
						break;
					case 1: //X-Backwards Planes
						break;
					case 2: //Y-Up Planes
						break;
					case 3: //Y-Down Planes
						break;
					case 4: //Z-Right Planes
						break;
					case 5: //Z-Left Planes
						break;
					}
					//vertices.push_back(glm::vec3(quad.x,3,4))
				}
			}
			
		}
		
	}
	prepareRender();


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
				//5:48

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

block chunk::getBlock(uint32 x, uint32 y, uint32 z)
{
	block EmptyBlock = { 0,0 };
	if (x < 0 or y < 0 or z < 0 or x>15 or y>15 or z>15) return EmptyBlock;
	return *(chunkData+z+(y*CHUNKSIZE)+(x*CHUNKSIZE*CHUNKSIZE));
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
	if(geomUpdated)
	{
		//Update Geometry

		updateGeom();
		geomUpdated = false;
	}
}

void chunk::render(camera& currentCamera)
{
	ResourceManager::GetShader("triangle")->Use(); //Select and use (via glUseProgram) the correct shader.

	
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)1280.f / (float)720.f, 0.1f, 100.0f);
	glm::mat4 view = currentCamera.cameraView;

	//Transform the trans matrix (model matrix)
	

	// --Bind Uniforms--
	// In this case our triangle only has 3 Uniforms, and they are all matrices.
	(*ResourceManager::GetShader("triangle")).SetMatrix4("transform", transform);
	(*ResourceManager::GetShader("triangle")).SetMatrix4("view", view);
	(*ResourceManager::GetShader("triangle")).SetMatrix4("projection", proj);
	glBindVertexArray(chunkVAO); //Now bind openGL to the correct vertex array
	glDrawArrays(GL_TRIANGLES, 0, vertices.size()+1); // Draw a triangle (starting at index 0 and increasing to 6 verts)
}

unsigned int chunk::prepareRender()
{
	unsigned int VBO;
	glGenVertexArrays(1, &chunkVAO); //Generate buffers and arrays
	glGenBuffers(1, &VBO);

	//Bind OpenGL to VAO object
	glBindVertexArray(chunkVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
	//vertices.size() * sizeof(float)

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0); //Unbind Array buffer

	//Unbind Vertex Array Object so we don't end up (accidentally modifying it)
	glBindVertexArray(0);

	return chunkVAO;
}