#pragma once
#include <vector>
#include <gameLayer/blocks.h>

struct GameMap
{
	int w = 0; // map width in blocks
	int h = 0; // map height in blocks

	std::vector<Block> mapData; // flat 1D array storing ALL blocks in the map

	// TEMPORARY - Wall Challenge
	std::vector<Wall> wallData; // flat 1D array storing ALL 'WALL' blocks

	void create(int w, int h); // initializing the map

	// get block by position, no bounds check before protection
	Block& getBlockUnsafe(int x, int y);

	// get block by position, safe version that returns nullptr if out of bounds
	Block* getBlockSafe(int x, int y);

	// get wall by position, no bounds check before protection
	Wall& getWallUnsafe(int x, int y);

	// get wall by position, safe version that returns nullptr if out of bounds
	Wall* getWallSafe(int x, int y);
};