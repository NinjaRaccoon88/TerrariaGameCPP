#include <gameLayer/worldGenerator.h>
#include <gameLayer/randomStuff.h>
#include <FastNoiseSIMD/FastNoiseSIMD.h>
#include <gameLayer/structure.h>
#include <gameLayer/saveMap.h>

void generateWorld
	(
		GameMap& gameMap, int seed,
		int dirtOffsetStart, int dirtOffsetEnd,
		int stoneHeightStart, int stoneHeightEnd,
		float dirtFrequency, float stoneFrequency,
		float caveThreshold, int surfaceBuffer,
		float caveFrequency
	)
{
	const int w = 900;
	const int h = 500;

	gameMap.create(w, h);

	std::ranlux24_base rng(seed++); // rng seeded - same seed = same world

	// stores dirtHeight for every column so addNormalCaves can access it
	// declared outside lambdas so both createStoneLayer and addNormalCaves can use it
	std::vector<int> dirtHeights(w, 0); // w elements, all initialized to 0

	std::vector<int> stoneHeights(w, 0);

	// pick a random start position for the desert
	// kept away from edges so desert never spawns right at the map border
	int desertStart = getRandomInt(rng, 10, w - 210);

	// desert is at least 100 blocks wide, up to 200 blocks wide
	int desertEnd = desertStart + 100 + getRandomInt(rng, 0, 100);

	// safety clamp - make sure desert doesn't go past map edge
	if (desertEnd > w) { desertEnd = w; }

	// loading tree structure from file
	// avoids reloading from disk every time a tree is spawned
	Structure treeStructure;
	loadBlockDataToFile
	(
		treeStructure.mapData,
		treeStructure.w,
		treeStructure.h,
		RESOURCES_PATH "structures/tree.bin"
	);

	// Create two separate noise generators - for dirt and for stone layers
	// unique_ptr automatically frees memory when it goes out of scope
	std::unique_ptr<FastNoiseSIMD> dirtNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> stoneNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> caveNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	// second cave noise - wide open caverns
	std::unique_ptr<FastNoiseSIMD> caveNoiseGenerator2(FastNoiseSIMD::NewFastNoiseSIMD());
	// blend noise - controls how cave type dominates in each area
	std::unique_ptr<FastNoiseSIMD> caveBlendNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());

	// give each generator a different seed so they produce diff noise patterns
	dirtNoiseGenerator->SetSeed(seed++);
	stoneNoiseGenerator->SetSeed(seed++);
	caveNoiseGenerator->SetSeed(seed++);
	caveNoiseGenerator2->SetSeed(seed++);
	caveBlendNoiseGenerator->SetSeed(seed++);

	// SimplexFractal = smooth organic looking noise (faster than Perlin)
	dirtNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	dirtNoiseGenerator->SetFractalOctaves(1); // simple smooth waves - good for surface
	dirtNoiseGenerator->SetFrequency(dirtFrequency); // how zoomed in the noise is, higher = more chaos

	stoneNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	stoneNoiseGenerator->SetFractalOctaves(4); // more detail layered on top of each other
	stoneNoiseGenerator->SetFrequency(stoneFrequency); // lower frequency = slower/wider waves

	caveNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	caveNoiseGenerator->SetFractalOctaves(3);
	// TODO: make this editable in ImGui (done)
	caveNoiseGenerator->SetFrequency(caveFrequency);

	// wide caverns - lower frequency, more octaves
	caveNoiseGenerator2->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	caveNoiseGenerator2->SetFractalOctaves(2);
	caveNoiseGenerator2->SetFrequency(0.008f); // lower = bigger caves

	// blend noise - very low frequency so transitions are wide and gradual
	caveBlendNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	caveBlendNoiseGenerator->SetFractalOctaves(1);
	caveBlendNoiseGenerator->SetFrequency(0.003f); // very low = large blend regions

	// allocate arrays to store one noise value per column
	float* dirtNoise = FastNoiseSIMD::GetEmptySet(w);
	float* stoneNoise = FastNoiseSIMD::GetEmptySet(w);

	// allocate a FULL 2D array - w*h values instead of just w like terrain noise
	// caves need every x,y position sampled unlike terrain which only needs one row
	float* caveNoise = FastNoiseSIMD::GetEmptySet(w * h);

	float* caveNoise2 = FastNoiseSIMD::GetEmptySet(w * h);
	float* caveBlendNoise = FastNoiseSIMD::GetEmptySet(w * h);

	// fill both arrays with noise values from all w columns at once
	dirtNoiseGenerator->FillNoiseSet(dirtNoise, 0, 0, 0, w, 1, 1);
	stoneNoiseGenerator->FillNoiseSet(stoneNoise, 0, 0, 0, w, 1, 1);

	// fill the full 2D noise set - note w and h are swapped compared to terrain
	caveNoiseGenerator->FillNoiseSet(caveNoise, 0, 0, 0, h, w, 1);

	caveNoiseGenerator2->FillNoiseSet(caveNoise2, 0, 0, 0, h, w, 1);
	caveBlendNoiseGenerator->FillNoiseSet(caveBlendNoise, 0, 0, 0, h, w, 1);

	// convert from [-1 1] to [0,1] cuz we need to use Lerp after which requires values 0-1
	for (int i = 0; i < w; i++)
	{
		dirtNoise[i] = (dirtNoise[i] + 1) / 2;
		stoneNoise[i] = (stoneNoise[i] + 1) / 2;
	}

	// convert from [-1 1] to [0,1]
	for (int i = 0; i < w * h; i++)
	{
		caveNoise[i] = (caveNoise[i] + 1) / 2;
		caveNoise2[i] = (caveNoise2[i] + 1) / 2;
		caveBlendNoise[i] = (caveBlendNoise[i] + 1) / 2;
	}

	// lambda function - convenient shorthand to look up cave noise at any x,y position
	// converts 2D coords to 1D array index using same formula as getBlockUnsafe()
	auto getCaveNoise = [&](int x, int y)
		{
			return caveNoise[x + y * w];
		};

	auto getCaveNoise2 = [&](int x, int y)
		{
			return caveNoise2[x + y * w];
		};

	auto getBlendNoise = [&](int x, int y)
		{
			return caveBlendNoise[x + y * w];
		};

	//int dirtOffsetStart = -5; // minimum dirt thickness
	//int dirtOffsetEnd = 35; // maximum dirt thickness above stone

	//int stoneHeightStart = 80; // minimum depth of stone layer
	//int stoneHeightEnd = 170; // maximum depth of stone layer


	// TODO: NEEDS REFACTORING vvv
	for (int x = 0; x < w; x++) // move column by column across the map
	{
		// simple bool - true if current x column falls within desert boundaries
		bool inDesert = (x >= desertStart && x <= desertEnd);

		// this is LERP formula: start + (end - start) * t
		int stoneHeight = stoneHeightStart + (stoneHeightEnd - stoneHeightStart) * stoneNoise[x];
		int dirtHeight = dirtOffsetStart + (dirtOffsetEnd - dirtOffsetStart) * dirtNoise[x];

		// dirt surface = stone surface minus the dirt thickness offset
		// so dirt always sits a natural distance above stone
		dirtHeight = stoneHeight - dirtHeight;

		// default block types for normal biomes
		int dirtType = Block::dirt;
		int grassType = Block::grassBlock;
		int stoneType = Block::stone;

		if (inDesert)
		{
			// desert replaces dirt with sand
			dirtType = Block::sand;
			// desert has no grass - sand goes all the way to surface
			grassType = Block::sand;
			// desert has sand stones instead of regular stone
			stoneType = Block::sandStone;
		}

		for (int y = 0; y < h; y++) // now fill this column top to bottom
		{
			Block b; // default to air

			if (y > dirtHeight) // below surface = dirt O_o
			{
				b.type = dirtType;
			}

			if (y == dirtHeight) // exactly at surface = grass obv :D 
			{
				b.type = grassType;
			}

			if (y > stoneHeight) // below stone line = stone
			{
				b.type = stoneType;
			}

			if (inDesert) // desert
			{
				// find the CENTER column of the desert
				int desertMid = (desertStart + desertEnd) / 2; 

				// half the total width of the desert
				int desertHalfWidth = (desertEnd - desertStart) / 2;

				// how far is current column from the center (always positive due to abs)
				int distanceFromDesertMid = std::abs(x - desertMid);

				// This gives a value from 0 at edge to 1 at center
				// this creates a smoother gradient from edge to center
				float desertDistance = 1 - distanceFromDesertMid / float(desertHalfWidth);

				// where the triangle stone shape starts (just below surface stone)
				int desertStoneStart = 10 + stoneHeight;
				int desertStoneDepth = 20 + stoneHeight; // how deep the triangle goes

				// this creates a pyramid/triangle shape when viewed from the side
				int triangleStoneY = desertStoneStart + desertDistance * desertStoneDepth;
				
				// Apply stone if below the triangle
				if (y > triangleStoneY)
				{
					b.type = Block::stone;
				}
			}

			// carve out caves wherever noise is below thershold AND we're underground
			// 10 is an amount of buffer zone to prevent spawning caves right under grass block
			if (getCaveNoise(x, y) < caveThreshold && y > dirtHeight + surfaceBuffer)
			{
				b.type = Block::air;
			}

			gameMap.getBlockUnsafe(x, y) = b;
		}
	}
	
	// TODO: fucking lambda spawn worm refactoring took me a while to make but it was worth it (I guess)
	// lambda that captures everything from generateWorld scope by refernece [&]
	// parameters: starting position, max tunnel length, max tunnel radius
	auto spawnWorm = [&](float startX, float startY, float maxLength, float maxR)
		{
			// picks a random starting point
			// it's important for x and y to be floats
			float x = startX; // worm current X position
			float y = startY; // worm current Y position

			// initial movement direction (-1 to 1 range)
			float dirX = (getRandomFloat(rng, -1, 1)); // negative = left, positive = right
			float dirY = (getRandomFloat(rng, -1, 1)); // negative = up, positive = down

			int wormLength = (int)maxLength; // how many steps this worm takes total
			float radius = 2.5f; // starting tunnel width

			int changeDirectionTime = getRandomInt(rng, 5, 20); // how often worm changes direction
			
			for (int j = 0; j < wormLength; j++) // each step of the worm's journey
			{
				// dig a circle around current position
				// ceil rounds UP so we always cover the full radius
				int intRadius = std::ceil(radius);

				// loop every block in a square around the current position
				for (int ox = -intRadius; ox <= intRadius; ox++) // offset x
				{
					// loop every block in a square around current position
					for (int oy = -intRadius; oy <= intRadius; oy++) // offset y
					{
						// calculate distance from center using Pythagorean theorem
						// a^2 + b^2 = c^2
						float distSq = ox * ox + oy * oy;

						// only dig if we're inside the circle
						if (distSq <= radius * radius)
						{
							int digX = x + ox; // actual world position to dig
							int digY = y + oy;

							auto b = gameMap.getBlockSafe(digX, digY);
							if (b)
							{
								b->type = Block::air; // DEBUG PURPOSES - PLACE AIR WHEN FINISHED
							}
						}
					}
				}

				changeDirectionTime--;
				if (changeDirectionTime <= 0) // timer ran out, time to maybe change direction
				{
					changeDirectionTime = getRandomInt(rng, 5, 20); // reset timer

					if (getRandomChance(rng, 0.7)) // 70% chance - gentle turn
					{
						float keepFactor = 0.8; // keep 80% of old direction

						// blend old direction with new random direction
						// big chance we keep a very similar direction
						// 80% old + 20% new = subtle turn
						dirX = dirX * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
						dirY = dirY * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
					}
					else // else 30% chance - sharp turn
					{
						float keepFactor = 0.2; // keep only 20% of old direction

						//big chance we keep a very similar direction
						// 20% old + 80% new = dramatic direction change
						dirX = dirX * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
						dirY = dirY * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
					}
				}
				// Move worm position in current direction
				x += dirX * 1.5f;
				y += dirY * 1.5f;

				// randomly wobble the radius so tunnel width varies naturally
				radius += (getRandomFloat(rng, -0.2, 0.2)); // slightly wider or narrower each step

				// clamps the radius between 2.2 and 8.5 so tunnel never disappears or gets huge
				radius = std::clamp(radius, 2.2f, maxR);
			}

		};

	for (int i = 0; i < 20; i++) // spawn 20 worms total
	{
		spawnWorm
		(
			getRandomInt(rng, 10, w - 10), // random X avoiding map edges
			getRandomInt(rng, 51, h - 10), // random Y underground (51+ avoid surface)
			getRandomInt(rng, 200, 500), // random length between 200-500 steps
			8.5f // max radius - tunnels never wider than 8.5 blocks
		);
	}

	// Spawning tree structure randomly on the grass block
	// TODO: Upgrade this code (tree variations and etc)
	for (int x = 0; x < w; x++)
	{
		// 4% chance per column to attempt tree spawning
		if (getRandomChance(rng, 0.04))
		{
			
			for (int y = 0; y < h; y++)
			{
				auto type = gameMap.getBlockUnsafe(x, y).type;
				if (type == Block::air) // skip air, keep scanning down
				{
					continue;
				}

				if (type == Block::grassBlock) // found surface
				{
					// plant tree

					Vector2 spawnPos{ (float)x, (float)y };

					spawnPos.x -= treeStructure.w / 2.0f; // center tree horizontally on column
					spawnPos.y -= treeStructure.h; // place tree above grass block

					treeStructure.pasteIntoMap(gameMap, spawnPos);

					x += 3; // we don't plant a tree to overlap this one
				}
				else
				{
					break; // hit non grass solid block, stop scanning
				}
			}
		}
	}

	/* TODO: lambda functions for :
	- dirt layer
	- stone layer 
	- normal caves
	- extra mountain

	LATER:
	- different types of caves
	- dungeons
	- special structures
	- procedural structures made of multiple pieces
	- ores
	- ice biom
	- sky islands
	*/ 
	
	// creates the basic world shape with stone
	auto createStoneLayer = [&]()
		{
			// ...
			for (int x = 0; x < w; x++)
			{
				// all the terrain code here
				// EXCEPT the cave carving if statement

				// this is LERP formula: start + (end - start) * t
				int stoneHeight = stoneHeightStart + (stoneHeightEnd - stoneHeightStart) * stoneNoise[x];
				int dirtHeight = dirtOffsetStart + (dirtOffsetEnd - dirtOffsetStart) * dirtNoise[x];

				// dirt surface = stone surface minus the dirt thickness offset
				// so dirt always sits a natural distance above stone
				dirtHeight = stoneHeight - dirtHeight;
				
				// store this column so caves can use it later!
				dirtHeights[x] = dirtHeight;
				stoneHeights[x] = stoneHeight;

				// default block types for normal biomes
				int dirtType = Block::dirt;
				int grassType = Block::grassBlock;
				int stoneType = Block::stone;

				for (int y = 0; y < h; y++) // now fill this column top to bottom
				{
					Block b; // default to air

					if (y > dirtHeight) // below surface = dirt O_o
					{
						b.type = dirtType;
					}

					if (y == dirtHeight) // exactly at surface = grass obv :D 
					{
						b.type = grassType;
					}

					if (y > stoneHeight) // below stone line = stone
					{
						b.type = stoneType;
					}

					gameMap.getBlockUnsafe(x, y) = b;
				}
			}
		};

	int mountainMid = getRandomInt(rng, 100, w - 210);
	int mountainHalfWidth = getRandomInt(rng, 30, 80);
	int mountainStart = mountainMid - mountainHalfWidth;
	int mountainEnd = mountainMid + mountainHalfWidth;
	int mountainMaxHeight = 60;

	// creates one mountain with stone
	auto addOneExtraMountain = [&]()
		{
			// ...

			for (int x = 0; x < w; x++)
			{
				bool inMountain = (x >= mountainStart && x <= mountainEnd);

				if (inMountain)
				{
					// how far is current column from the center (always positive due to abs)
					int distanceFromMountainMid = std::abs(x - mountainMid);

					float t = 1.f - distanceFromMountainMid / float(mountainHalfWidth);
					int riseAmount = (int)(mountainMaxHeight * t);

					int mountainSurfaceY = dirtHeights[x];

					for (int i = 0; i < riseAmount; i++)
					{
						int placeY = mountainSurfaceY - i; // going UP the mountain
						
						auto b = gameMap.getBlockSafe(x, placeY);

						if (b) // always check first
						{
							// place block at (x, placeY)
							if (i > riseAmount * 70 / 100) // top 30% -> snow
							{
								b->type = Block::snow;
							}
							else
							{
								b->type = Block::stone;
							}
						}
					}
				}
			}
		};

	// changes the top few blocks to dirt
	auto dirtLayer = [&]()
		{
			// ...
			for (int x = 0; x < w; x++)
			{

				if (getRandomChance(rng, 0.004))
				{

					for (int y = 0; y < h; y++)
					{
						// get current block type
						auto type = gameMap.getBlockUnsafe(x, y).type;
						if (type == Block::air)
						{
							continue; // if it's not ground, we continue searching
						}
						// we need to find surface (grass block)
						if (type == Block::grassBlock)
						{
							gameMap.getBlockUnsafe(x, y).type = Block::dirt;
						}
						else
						{
							break; // hit non grass solid block, stop scanning
						}
					}
				}
			}
		};

	auto addNormalCaves = [&]()
		{
			// ...
			for (int x = 0; x < w; x++)
			{

				// read the stored dirtHeight for this column
				int dirtHeight = dirtHeights[x];

				for (int y = 0; y < h; y++)
				{
					// only the carving logic here

					// blend between two cave noises
					float cave1 = getCaveNoise(x, y); // tight tunnels
					float cave2 = getCaveNoise2(x, y); // wide caverns
					float blend = getBlendNoise(x,y); // 0 = all cave1, 1 = all cave2

					// lerp between the two cave noises using blend as 't'
					// this is LERP formula: start + (end - start) * t
					float blendedCave = cave1 + (cave2 - cave1) * blend;

					// carve out caves wherever noise is below thershold AND we're underground
					// 10 is an amount of buffer zone to prevent spawning caves right under grass block
					if (blendedCave < caveThreshold && y > dirtHeight + surfaceBuffer)
					{
						gameMap.getBlockUnsafe(x,y).type = Block::air;
					}
				}
			}
		};

	// generates the Desert Biom
	auto addDesert = [&]()
		{
			// ...
		};

	auto addRandomSand = [&]()
		{
			// ...
		};

	// IMPORTANT: must free manually since FastNoiseSIMD uses raw pointers, not smart pointers
	FastNoiseSIMD::FreeNoiseSet(dirtNoise);
	FastNoiseSIMD::FreeNoiseSet(stoneNoise);
	FastNoiseSIMD::FreeNoiseSet(caveNoise);
	FastNoiseSIMD::FreeNoiseSet(caveNoise2);
	FastNoiseSIMD::FreeNoiseSet(caveBlendNoise);
}