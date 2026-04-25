#include <gameLayer/worldGenerator.h>
#include <gameLayer/randomStuff.h>
#include <FastNoiseSIMD/FastNoiseSIMD.h>

void generateWorld(GameMap& gameMap, int seed)
{
	const int w = 900;
	const int h = 500;

	gameMap.create(w, h);

	std::ranlux24_base rng(seed); // rng seeded - same seed = same world


	// how many columns to keep moving in the SAME direction before changing
	// starts at a random value between 5-40 columns
	int keepDirectionTimeDirt = getRandomInt(rng, 5, 40);
	// current movement direction: fast down, slow down, flat, slow up and fast up
	int directionDirt = getRandomInt(rng, -2, 2);

	// same two variables but for stone layer independently
	int keepDirectionTimeStone = getRandomInt(rng, 5, 40);
	int directionStone = getRandomInt(rng, -2, 2);

	int dirtHeight = 70; // starting Y position of the dirt surface
	int stoneHeight = 90; // starting Y position of the stone layer

	for (int x = 0; x < w; x++) // move column by column across the map
	{

		// count down the timer for dirt duration
		keepDirectionTimeDirt--;
		if (keepDirectionTimeDirt <= 0)
		{
			// timer ran out - pick a new random direction and reset timer
			keepDirectionTimeDirt = getRandomInt(rng, 5, 40);
			directionDirt = getRandomInt(rng, -2, 2);
		}

		// direction -1 = slow downward movement
		// only 25% chance to actually move, making it smooth
		if (directionDirt == -1)
		{
			if (getRandomChance(rng, 0.25))
			{
				dirtHeight--;
			}
		}
		// direction -2 = fast downward movement
		// two separate 25% chance = up to 2 steps down per column
		else if (directionDirt == -2)
		{
			if (getRandomChance(rng, 0.25))
			{
				dirtHeight--;
			}

			if (getRandomChance(rng, 0.25))
			{
				dirtHeight--;
			}
		}
		// direction 1 = slow upward movement
		else if (directionDirt == 1)
		{
			if (getRandomChance(rng, 0.25))
			{
				dirtHeight++;
			}
		}
		// direction 2 = fast upward movement
	   // two separate 25% chances = up to 2 steps up per column
		else if (directionDirt == 2)
		{
			if (getRandomChance(rng, 0.25))
			{
				dirtHeight++;
			}

			if (getRandomChance(rng, 0.25))
			{
				dirtHeight++;
			}
		}

		// clamp dirt height so terrain never goes too high or too low
		if (dirtHeight < 50) { dirtHeight = 50; } // max mountain height
		if (dirtHeight > 90) { dirtHeight = 90; } // min valley depth

		//same code for stone
		keepDirectionTimeStone--;
		if (keepDirectionTimeStone <= 0)
		{
			keepDirectionTimeStone = getRandomInt(rng, 5, 40);
			directionStone = getRandomInt(rng, -2, 2);
		}

		if (directionStone == -1)
		{
			if (getRandomChance(rng, 0.25))
			{
				stoneHeight--;
			}
		}
		else if (directionStone == -2)
		{
			if (getRandomChance(rng, 0.25))
			{
				stoneHeight--;
			}

			if (getRandomChance(rng, 0.25))
			{
				stoneHeight--;
			}
		}
		else if (directionStone == 1)
		{
			if (getRandomChance(rng, 0.25))
			{
				stoneHeight++;
			}
		}
		else if (directionStone == 2)
		{
			if (getRandomChance(rng, 0.25))
			{
				stoneHeight++;
			}

			if (getRandomChance(rng, 0.25))
			{
				stoneHeight++;
			}
		}

		// clamp stone height within its own range
		if (stoneHeight < 60) { stoneHeight = 60; }
		if (stoneHeight > 120) { stoneHeight = 120; }

		for (int y = 0; y < h; y++) // now fill this column top to bottom
		{
			Block b; // default to air

			if (y > dirtHeight) // below surface = dirt O_o
			{
				b.type = Block::dirt;
			}

			if (y == dirtHeight) // exactly at surface = grass obv :D 
			{
				b.type = Block::grassBlock;
			}

			if (y > stoneHeight) // below stone line = stone
			{
				b.type = Block::stone;
			}

			gameMap.getBlockUnsafe(x, y) = b;
		}
	}
}