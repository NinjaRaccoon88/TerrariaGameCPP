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

	// loading all tree structures grouped by size
	// avoids reloading from disk every time a tree is spawned
	std::vector<Structure> smallTrees; // trees 6,7,8,9
	std::vector<Structure> mediumTrees; // trees 2,3,4,10
	std::vector<Structure> largeTrees; // trees 1,5,12,14
	std::vector<Structure> hugeTrees; // trees 11,13

	// helper lambda to load tree bin file and push it into the correct vector
	// avoids repeating the same load code 14 times
	auto loadTree = [&](std::vector<Structure>& vec, const char* path)
		{
			Structure s;
			loadBlockDataToFile(s.mapData, s.w, s.h, path); // load from disk into struct
			vec.push_back(s); // add to the vector
		};

	// Loading all tree bin files in order small - huge :D
	loadTree(smallTrees, RESOURCES_PATH "structures/tree6.bin");
	loadTree(smallTrees, RESOURCES_PATH "structures/tree7.bin");
	loadTree(smallTrees, RESOURCES_PATH "structures/tree8.bin");
	loadTree(smallTrees, RESOURCES_PATH "structures/tree9.bin");
	loadTree(mediumTrees, RESOURCES_PATH "structures/tree2.bin");
	loadTree(mediumTrees, RESOURCES_PATH "structures/tree3.bin");
	loadTree(mediumTrees, RESOURCES_PATH "structures/tree4.bin");
	loadTree(mediumTrees, RESOURCES_PATH "structures/tree10.bin");
	loadTree(largeTrees, RESOURCES_PATH "structures/tree1.bin");
	loadTree(largeTrees, RESOURCES_PATH "structures/tree5.bin");
	loadTree(largeTrees, RESOURCES_PATH "structures/tree12.bin");
	loadTree(largeTrees, RESOURCES_PATH "structures/tree14.bin");
	loadTree(hugeTrees, RESOURCES_PATH "structures/tree11.bin");
	loadTree(hugeTrees, RESOURCES_PATH "structures/tree13.bin");

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
	
	// DONE: fucking lambda spawn worm refactoring took me a while to make but it was worth it (I guess)
	// lambda that captures everything from generateWorld scope by refernece [&]
	// parameters: starting position, max tunnel length, max tunnel radius
	auto spawnWorm = [&](float startX, float startY, float maxLength, float maxR)
		{
			// picks a random starting point
			// it's important for x and y to be floats
			float x = startX; // worm current X position k
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
							if (b && digY > dirtHeights[digX] + 8) // never dig surfaces
							{
								b->type = Block::air;
							}
						}
					}
				}

				changeDirectionTime--;
				if (changeDirectionTime <= 0) // timer ran out, time to maybe change direction
				{
					changeDirectionTime = getRandomInt(rng, 15, 40); // reset timer

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

	/* TODO: lambda functions for :
	- dirt layer - DONE
	- stone layer - DONE
	- normal caves - DONE
	- extra mountain - DONE

	LATER:
	- different types of caves
	- dungeons
	- special structures
	- procedural structures made of multiple pieces
	- ores - DONE
	- ice biom - DONE (almost - needs ore)
	- sky islands
	*/ 
	
#pragma region spawnCluster
	auto spawnCluster = [&]
	(
		int startX, int startY, 
		int minLength, int maxLength, 
		int replaceType, int placeType
	)
		{
			// pick ONE dominant direction per cluster
			// 0=right, 1=left, 2=down, 3=up
			int primaryDir = getRandomInt(rng, 0, 3);
			float clusterX = startX; 
			float clusterY = startY;
			int clusterLength = getRandomInt(rng, minLength, maxLength);

			for (int step = 0; step < clusterLength; step++)
			{
				// move mostly in primary direction with occasional perpendicular step
				if (primaryDir == 0) { clusterX += 1; }
				else if (primaryDir == 1) { clusterX -= 1; }
				else if (primaryDir == 2) { clusterY += 1; }
				else if (primaryDir == 3) { clusterY -= 1; }

				// small chance to step sideways for a more blocky natural shape
				if (getRandomChance(rng, 0.3f))
				{
					if (primaryDir == 0 || primaryDir == 1)
					{
						clusterY += getRandomInt(rng, -1, 1); // occasional vertical nudge
					}
					else
					{
						clusterX += getRandomInt(rng, -1, 1); // occasional horizontal nudge
					}
				}

				auto b = gameMap.getBlockSafe((int)clusterX, (int)clusterY);

				if (b && b->type == replaceType)
				{
					b->type = placeType;
				}
			}
		};
#pragma endregion spawnCluster

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

					gameMap.getBlockUnsafe(x, y) = b; // write block into map
				}
			}
		};

	// Ice Biom Variables
	// pick a random start pos for the ice biom
	// keeping away from edges as always :D
	int iceStart = getRandomInt(rng, 10, w - 210);

	// ice biom will be at least 100 blocks wide, up to 200 blocks wide
	int iceEnd = iceStart + 100 + getRandomInt(rng, 0, 100);

	int iceHalfWidth = (iceEnd - iceStart) / 2;
	int iceMid = iceStart + iceHalfWidth;

	// Mountain Variables
	int mountainMid = getRandomInt(rng, 100, w - 210);
	int mountainHalfWidth = getRandomInt(rng, 30, 80);
	int mountainStart = mountainMid - mountainHalfWidth;
	int mountainEnd = mountainMid + mountainHalfWidth;
	int mountainMaxHeight = 60;

	// Desert Variables
	// pick a random start position for the desert
	// kept away from edges so desert never spawns right at the map border
	int desertStart = getRandomInt(rng, 10, w - 210);

	// desert is at least 100 blocks wide, up to 200 blocks wide
	int desertEnd = desertStart + 100 + getRandomInt(rng, 0, 100);

	// safety clamp - make sure desert doesn't go past map edge
	if (desertEnd > w) { desertEnd = w; }

	// safety clamp for mountain
	if (mountainEnd > w) { mountainEnd = w; }

	// safety clamp for the ice biom
	if (iceEnd > w) { iceEnd = w; }

	// keep repicking until ice biom doesn't overlap with desert
	while (iceStart < desertEnd && iceEnd > desertStart)
	{
		iceStart = getRandomInt(rng, 10, w - 210);
		iceEnd = iceStart + 100 + getRandomInt(rng, 0, 100);
		iceHalfWidth = (iceEnd - iceStart) / 2;
		iceMid = iceStart + iceHalfWidth;
	}

	// keep repicking until mountain doesn't overlap Desert and Ice Biom
	while ((mountainStart < desertEnd && mountainEnd > desertStart) || 
		   (mountainStart < iceEnd && mountainEnd > iceStart))
	{
		mountainMid = getRandomInt(rng, 100, w - 210);
		mountainStart = mountainMid - mountainHalfWidth;
		mountainEnd = mountainMid + mountainHalfWidth;
	}

	// generates the Ice Biom
	auto addIceBiom = [&]()
		{
			// ...

			float iceDistance = 0.0f; // declared here so X and Y loop can see it

			for (int x = 0; x < w; x++)
			{
			
				// bool to check whether current x column falls within ice biom boundaries
				bool inIce = (x >= iceStart && x <= iceEnd);

				if (inIce)
				{
					// how far is current column from the center (always positive due to abs)
					int distanceFromIceMid = std::abs(x - iceMid);

					// This gives a value from 0 at edge to 1 at center
					// this creates a smoother gradient from edge to center
					iceDistance = 1 - distanceFromIceMid / float(iceHalfWidth);

					for (int y = 0; y < h; y++) // fill this column from bottom to the top
					{
						// getting reference to the current block
						auto& b = gameMap.getBlockUnsafe(x, y);

						// TRIANGLE FIRST - underground ice structure
						// underground ice structure - similar to desert triangle
						// upside down triangle
						int iceUndergroundStart = stoneHeights[x];
						int iceUndergroundDepth = 80; // how deep the ice goes

						int triangleIceY = iceUndergroundStart + iceDistance * iceUndergroundDepth;

						if (y > iceUndergroundStart && y < triangleIceY)
						{
							if (b.type == Block::stone) { b.type = Block::ice; }
						}

						// GRADIENT SECOND - surface block conversion with edge fallof
						// how wide the gradient transition is on each edge
						int gradientWidth = 10;

						// calculate edge factor - only 0 to 1 within gradientWidth blocks of each edge
						float edgeFactor = 1.0f; // default = fully ice (center)

						int distFromLeft = x - iceStart;
						int distFromRight = iceEnd - x;
						int closestEdge = std::min(distFromLeft, distFromRight);

						if (closestEdge < gradientWidth)
						{
							edgeFactor = closestEdge / float(gradientWidth);
							// 0 at very edge, 1 at gradientWidth blocks inside
						}

						// use iceDistance as the chance - center always converts, edges rarely do
						if (getRandomChance(rng, edgeFactor))
						{
							if (b.type == Block::grassBlock) { b.type = Block::snow; }
							else if (b.type == Block::dirt) { b.type = Block::snow; }
						}
					}
				}
			}
		};

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
							if (i > riseAmount * 0.8) // top 20% -> snow
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

				if (getRandomChance(rng, 0.1)) // 10% on spawning dirt block instead of grass
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

	// creates caves using perlin noise
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
			float desertDistance = 0.f; // declared here so y loop can see it

			for (int x = 0; x < w; x++)
			{
				
				// simple bool - true if current x column falls within desert boundaries
				bool inDesert = (x >= desertStart && x <= desertEnd);

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

					// find the CENTER column of the desert
					int desertMid = (desertStart + desertEnd) / 2;

					// half the total width of the desert
					int desertHalfWidth = (desertEnd - desertStart) / 2;

					// how far is current column from the center (always positive due to abs)
					int distanceFromDesertMid = std::abs(x - desertMid);

					// This gives a value from 0 at edge to 1 at center
					// this creates a smoother gradient from edge to center
					desertDistance = 1 - distanceFromDesertMid / float(desertHalfWidth);

				}

				for (int y = 0; y < h; y++) // now fill this column top to bottom
				{
					auto& b = gameMap.getBlockUnsafe(x, y); // reference to the existing block

					if (b.type == Block::grassBlock) b.type = grassType;
					else if (b.type == Block::dirt)  b.type = dirtType;
					else if (b.type == Block::stone) b.type = stoneType;

					if (inDesert) // desert
					{
						// where the triangle stone shape starts (just below surface stone)
						int desertStoneStart = 10 + stoneHeights[x];
						int desertStoneDepth = 20 + stoneHeights[x]; // how deep the triangle goes

						// this creates a pyramid/triangle shape when viewed from the side
						int triangleStoneY = desertStoneStart + desertDistance * desertStoneDepth;

						// Apply stone if below the triangle
						if (y > triangleStoneY)
						{
							b.type = Block::stone;
						}
					}
				}
			}
		};

	// adds random sand - close to surface and deep below the surface
	auto addRandomSand = [&]()
		{
			// ...

			for (int x = 0; x < w; x++)
			{

				for (int y = 0; y < h; y++)
				{
					auto& b = gameMap.getBlockUnsafe(x,y);

#pragma region SandSpawnerLayer1
					// Sand Layer 1 - near surface, replaces dirt
					if (b.type == Block::dirt || b.type == Block::grassBlock)
					{
						if (getRandomChance(rng, 0.002))
						{
							// 
							spawnCluster(x, y, 2, 4, Block::dirt, Block::sand);
							spawnCluster(x, y, 2, 4, Block::grassBlock, Block::sand);
						}
					}
#pragma endregion SandSpawnerLayer1
#pragma region SandSpawnerLayer2
					if (b.type == Block::stone)
					{
						if (y > dirtHeights[x] + 150) // at least 150 blocks below surface
						{
							if (getRandomChance(rng, 0.0005)) // 0.5% to spawn
							{
								spawnCluster(x, y, 5, 10, Block::stone, Block::sand);
							}
						}
					}
#pragma endregion SandSpawnerLayer2
				}
			}
		};

	// creates layer of all kind of ores!
	auto addOres = [&]()
		{
			// ...

			for (int x = 0; x < w; x++)
			{

				for (int y = 0; y < h; y++)
				{

					auto& b = gameMap.getBlockUnsafe(x, y);

#pragma region CopperSpawner
					// copper - spawns in dirt or stone, just below surface
					if (b.type == Block::dirt || b.type == Block::stone)
					{
						if (y > dirtHeights[x] + 5) // at least 5 below surface
						{
							if (getRandomChance(rng, 0.002)) // 0.2% chance per block
							{
								spawnCluster(x, y, 2, 4, Block::dirt, Block::copper);
								spawnCluster(x, y, 2, 4, Block::stone, Block::copper);
							}
						}
					}
#pragma endregion CopperSpawner
#pragma region IronSpawner
					// iron - spawns in stone
					if (b.type == Block::stone)
					{
						if (y > dirtHeights[x] + 35) // must be 35 blocks below surface
						{
							// very little chance of spawning iron ore
							if (getRandomChance(rng, 0.00087)) // 0.087% chance of spawning
							{
								spawnCluster(x, y, 3, 5, Block::stone, Block::iron);
							}
						}
					}
#pragma endregion IronSpawner
#pragma region GoldSpawner
					// gold spawns in stones way deeper than copper and iron
					if (b.type == Block::stone)
					{
						if (y > dirtHeights[x] + 100)
						{
							// very little chance of spawning gold ore
							if (getRandomChance(rng, 0.0005)) // 0.05% chance - rarest
							{
								spawnCluster(x, y, 3, 5, Block::stone, Block::gold);
							}
						}
					}
#pragma endregion GoldSpawner
#pragma region RubySpawnerInSand
					// Ruby ore spawner below the surface
					if (b.type == Block::sand)
					{
						if (y > dirtHeights[x] + 150)
						{
							if (getRandomChance(rng, 0.33))
							{
								spawnCluster(x, y, 5, 10, Block::sand, Block::sandRuby);
							}
						}
					}
#pragma endregion RubySpawnerInSand
#pragma region RubySpawner
					// Ruby ore spawner below the surface
					if (b.type == Block::sandStone)
					{
						if (getRandomChance(rng, 0.002)) // 0.2% chance of spawning ruby ore in desert
						{
							spawnCluster(x, y, 5, 10, Block::sandStone, Block::sandRuby);
						}
					}
		
#pragma endregion RubySpawner

				}
			}
		};

	// Calling lambda functions (finally LFG!)
	createStoneLayer();		// 1. Build base terrain (must be first)
	addDesert();			// 2. Replace blocks in desert area (generate desert biom)
	addIceBiom();			// 3. Generate Ice Biom
	addOneExtraMountain();  // 4. Raise terrain for mountain
	addNormalCaves();		// 5. Carve out cave systems
							// 6. Place trees on grass surface (needs refactoring)
#pragma region TreeSpawner
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
					
					// pick a random category then random tree from it
					int category = getRandomInt(rng, 0, 3);
					Structure* chosenTree = nullptr; // pointer to whichever tree we pick
					int spacing = 1; // how many blocks to skip after planting to prevent overlpping 

					// empty() check prevents crash if a bin file failed to load
					if (category == 0 && !smallTrees.empty())
					{
						// getRandomInt picks a random index within a vector
						chosenTree = &smallTrees[getRandomInt(rng, 0, smallTrees.size() - 1)];
						spacing = 1;
					}
					else if (category == 1 && !mediumTrees.empty())
					{
						chosenTree = &mediumTrees[getRandomInt(rng, 0, mediumTrees.size() - 1)];
						spacing = 3;
					}
					else if (category == 2 && !largeTrees.empty())
					{
						chosenTree = &largeTrees[getRandomInt(rng, 0, largeTrees.size() - 1)];
						spacing = 5;
					}
					else if (category == 3 && !hugeTrees.empty())
					{
						chosenTree = &hugeTrees[getRandomInt(rng, 0, hugeTrees.size() - 1)];
						spacing = 8;
					}

					// only spawn if we actually picked a tree and it has valid data
					if (chosenTree && chosenTree->w > 0)
					{
						Vector2 spawnPos{ (float)x, (float)y };

						spawnPos.x -= chosenTree->w / 2.0f; // center tree horizontally on column
						spawnPos.y -= chosenTree->h; // place tree above grass block

						chosenTree->pasteIntoMap(gameMap, spawnPos);

						x += spacing; // we don't plant a tree to overlap this one
					}
				}
				else
				{
					break; // hit non grass solid block, stop scanning
				}
			}
		}
	}
#pragma endregion TreeSpawner
	dirtLayer();			// 7. Randomly replace some grass with dirt
							// 8. Carve organic worm tunnels
#pragma region WormSpawner
	for (int i = 0; i < 12; i++) // spawn 12 worms total
	{
		spawnWorm
		(
			getRandomInt(rng, 10, w - 10), // random X avoiding map edges
			getRandomInt(rng, 100, h - 10), // random Y underground (100+ avoid surface)
			getRandomInt(rng, 150, 350), // random length between 150-350 steps
			6.0f // max radius - tunnels never wider than 6 blocks
		);
	}
#pragma endregion WormSpawner
	addRandomSand();		// 9. Random sand
	addOres();				// 10. Scatter ores underground (must be after caves and sand)

	// IMPORTANT: must free manually since FastNoiseSIMD uses raw pointers, not smart pointers
	FastNoiseSIMD::FreeNoiseSet(dirtNoise);
	FastNoiseSIMD::FreeNoiseSet(stoneNoise);
	FastNoiseSIMD::FreeNoiseSet(caveNoise);
	FastNoiseSIMD::FreeNoiseSet(caveNoise2);
	FastNoiseSIMD::FreeNoiseSet(caveBlendNoise);
}