#include <gameLayer/structure.h>
#include <platform/asserts.h>

void Structure::create(int w, int h)
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
Block& Structure::getBlockUnsafe(int x, int y)
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
Block* Structure::getBlockSafe(int x, int y)
{
	// check in dev if map not initialized
	permaAssertCommentDevelopement(mapData.size() == w * h, "Map data is not initialized");

	// out of bounds - return nullptr instead of crashing
	if (x < 0 || y < 0 || x >= w || y >= h) { return nullptr; }

	// returns a POINTER, caller must check for nullptr before using
	return &mapData[x + y * w];
}

// crashing if you go out of bounds, you when you're sure the coord are valid
Wall& Structure::getWallUnsafe(int x, int y)
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
Wall* Structure::getWallSafe(int x, int y)
{
	// check in dev if map not initialized
	permaAssertCommentDevelopement(wallData.size() == w * h, "WALL data is not initialized");

	// out of bounds - return nullptr instead of crashing
	if (x < 0 || y < 0 || x >= w || y >= h) { return nullptr; }

	// returns a POINTER, caller must check for nullptr before using
	return &wallData[x + y * w];
}

void Structure::copyFromMap(GameMap& map, Vector2 start, Vector2 end)
{
	// TODO: shit tons of IF checks to make sure everything works just fine
	if (end.x > map.w) { end.x = map.w - 1; }
	if (start.x > map.w) { start.x = map.w - 1; }

	if (end.y > map.h) { end.y = map.h - 1; }
	if (start.y > map.h) { start.y = map.h - 1; }

	if (end.x < 0) { end.x = 0; }
	if (end.y < 0) { end.y = 0; }

	if (start.x < 0) { start.x = 0; }
	if (start.y < 0) { start.y = 0; }

	if (start.x > end.x) { std::swap(start.x, end.x); }
	if (start.y > end.y) { std::swap(start.y, end.y); }

	Vector2 size = Vector2{ end.x - start.x + 1, end.y - start.y + 1 };

	if (size.x > map.w) { return; }
	if (size.y > map.w) { return; }

	create(size.x, size.y);

	for (int y = 0; y < size.y; y++)
	{
		for (int x = 0; x < size.x; x++)
		{
			getBlockUnsafe(x, y) = map.getBlockUnsafe(x + start.x, y + start.y);
		}
	}
}

void Structure::pasteIntoMap(GameMap& map, Vector2 start)
{
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			auto b = map.getBlockSafe(x + start.x, y + start.y);

			if (b)
			{
				*b = getBlockUnsafe(x, y);
			}
		}
	}
}


