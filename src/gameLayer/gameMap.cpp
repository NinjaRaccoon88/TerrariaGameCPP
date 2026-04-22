#include <gameLayer/gameMap.h>
#include <platform/asserts.h>

void GameMap::create(int w, int h)
{
	*this = {}; // resets the struct to default values (clears)
	mapData.resize(w * h); // resize the mapData vector array
	wallData.resize(w * h); // resize the wallData vector array

	this->w = w; // store width
	this->h = h; // store height

	for (auto& e : mapData) { e = {}; } // loops every block and resets it to default (clears)
	for (auto& e : wallData) { e = {}; } // loop every block and resets it to default value
}

// crashing if you go out of bounds, you when you're sure the coord are valid
Block& GameMap::getBlockUnsafe(int x, int y)
{
	// crash in dev if map not initialized
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map data is not initialized");

	// crash if out of bounds
	permaAssertCommentDevelopement(x >= 0 &&
		y >= 0 && x < w && y < h, "getBlockUnsafe out of bounds error");

	// return a REFERENCE, so you can modify the block directly
	return mapData[x + y * w];
}

// returns nullptr if out of bounds
Block* GameMap::getBlockSafe(int x, int y)
{
	// check in dev if map not initialized
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map data is not initialized");

	// out of bounds - return nullptr instead of crashing
	if (x < 0 || y < 0 || x >= w || y >= h) { return nullptr; }

	// returns a POINTER, caller must check for nullptr before using
	return &mapData[x + y * w];
}

// crashing if you go out of bounds, you when you're sure the coord are valid
Wall& GameMap::getWallUnsafe(int x, int y)
{
	// crash in dev if map not initialized
	permaAssertCommentDevelopement(wallData.size() == w * h, "WALL data is not initialized");

	// crash if out of bounds
	permaAssertCommentDevelopement(x >= 0 &&
		y >= 0 && x < w && y < h, "getWallUnsafe out of bounds error");

	// return a REFERENCE, so you can modify the block directly
	return wallData[x + y * w];
}

// returns nullptr if out of bounds
Wall* GameMap::getWallSafe(int x, int y)
{
	// check in dev if map not initialized
	permaAssertCommentDevelopement(wallData.size() == w * h, "WALL data is not initialized");

	// out of bounds - return nullptr instead of crashing
	if (x < 0 || y < 0 || x >= w || y >= h) { return nullptr; }

	// returns a POINTER, caller must check for nullptr before using
	return &wallData[x + y * w];
}