#pragma once

// C++ likes to align to 4 block boundaries, so having 16 bits in a struct is the same as having 32 bits in a struct.

struct block
{	// Block should be 80 bytes ~ 327kb per chunk.

	//Right now id is 15 bits but you can increase the total bit usage if not going over 4 byte alignment
	//Block
	unsigned int id : 31; //Block-ID
	unsigned int solid : 1;

	unsigned int lightLevel : 6=0; //0-15
	
};

