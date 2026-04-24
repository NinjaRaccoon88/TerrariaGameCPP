#include <gameLayer/worldGenerator.h>
#include <gameLayer/randomStuff.h>

void generateWorld(GameMap& gameMap, int seed)
{
	const int w = 900; // map width in blocks
	const int h = 500; // map height in blocks

	gameMap.create(w, h); // initializing the map with given dimensions

	int stoneSize = 380; // how many rows form the bottom are stone
	int dirtSize = 50; // how many rows above stone with dirt

	// create rng seeded with given seed
	std::ranlux24_base rng(seed); // same seed = same world every time

	for (int x = 0; x < w; x++) // loop every column (width)
	{
		for (int y = 0; y < h; y++) // loop every row (height)
		{
			Block b; // create empty block, default = air

			if (y < h - (dirtSize + stoneSize))
			{
				// air
			}
			else
			if (y == h - (dirtSize + stoneSize))
			{
				b.type = Block::grassBlock; // grass...
			}
			else
			if (y < h - stoneSize)
			{
				b.type = Block::dirt; // dirt O_O
			}
			else
			{
				b.type = Block::stone; // everything below is stone :D

				if (getRandomChance(rng, 0.1)) // 10% chance on spawning gold
				{
					b.type = Block::gold;
				}
			}

			gameMap.getBlockUnsafe(x, y) = b; // write the block into the map
		}
	}
}
