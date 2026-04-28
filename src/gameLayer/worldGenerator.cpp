#include <gameLayer/worldGenerator.h>
#include <gameLayer/randomStuff.h>
#include <FastNoiseSIMD/FastNoiseSIMD.h>

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

	// pick a random start position for the desert
	// kept away from edges so desert never spawns right at the map border
	int desertStart = getRandomInt(rng, 10, w - 210);

	// desert is at least 100 blocks wide, up to 200 blocks wide
	int desertEnd = desertStart + 100 + getRandomInt(rng, 0, 100);

	// safety clamp - make sure desert doesn't go past map edge
	if (desertEnd > w) { desertEnd = w; }


	// Create two separate noise generators - for dirt and for stone layers
	// unique_ptr automatically frees memory when it goes out of scope
	std::unique_ptr<FastNoiseSIMD> dirtNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> stoneNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> caveNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());

	// give each generator a different seed so they produce diff noise patterns
	dirtNoiseGenerator->SetSeed(seed++);
	stoneNoiseGenerator->SetSeed(seed++);
	caveNoiseGenerator->SetSeed(seed++);

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

	// allocate arrays to store one noise value per column
	float* dirtNoise = FastNoiseSIMD::GetEmptySet(w);
	float* stoneNoise = FastNoiseSIMD::GetEmptySet(w);

	// fill both arrays with noise values from all w columns at once
	dirtNoiseGenerator->FillNoiseSet(dirtNoise, 0, 0, 0, w, 1, 1);
	stoneNoiseGenerator->FillNoiseSet(stoneNoise, 0, 0, 0, w, 1, 1);

	// convert from [-1 1] to [0,1] cuz we need to use Lerp after which requires values 0-1
	for (int i = 0; i < w; i++)
	{
		dirtNoise[i] = (dirtNoise[i] + 1) / 2;
		stoneNoise[i] = (stoneNoise[i] + 1) / 2;
	}

	// allocate a FULL 2D array - w*h values instead of just w like terrain noise
	// caves need every x,y position sampled unlike terrain which only needs one row
	float* caveNoise = FastNoiseSIMD::GetEmptySet(w * h);

	// fill the full 2D noise set - note w and h are swapped compared to terrain
	caveNoiseGenerator->FillNoiseSet(caveNoise, 0, 0, 0, h, w, 1);

	// convert from [-1 1] to [0,1]
	for (int i = 0; i < w * h; i++)
	{
		caveNoise[i] = (caveNoise[i] + 1) / 2;
	}

	// lambda function - convenient shorthand to look up cave noise at any x,y position
	// converts 2D coords to 1D array index using same formula as getBlockUnsafe()
	auto getCaveNoise = [&](int x, int y)
		{
			return caveNoise[x + y * w];
		};

	//int dirtOffsetStart = -5; // minimum dirt thickness
	//int dirtOffsetEnd = 35; // maximum dirt thickness above stone

	//int stoneHeightStart = 80; // minimum depth of stone layer
	//int stoneHeightEnd = 170; // maximum depth of stone layer

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

			if (inDesert)
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

	// IMPORTANT: must free manually since FastNoiseSIMD uses raw pointers, not smart pointers
	FastNoiseSIMD::FreeNoiseSet(dirtNoise);
	FastNoiseSIMD::FreeNoiseSet(stoneNoise);
	FastNoiseSIMD::FreeNoiseSet(caveNoise);

	for (int i = 0; i < 20; i++) // spawn 20 worms total
	{
		// picks a random starting point
		// it's important for x and y to be floats
		float x = getRandomInt(rng, 10, w - 10); // anywhere horizontally, avoid edges
		float y = getRandomInt(rng, 51, h - 10); // underground only (51+ to stay below surface)

		// initial movement direction (-1 to 1 range)
		float dirX = (getRandomFloat(rng, -1, 1)); // negative = left, positive = right
		float dirY = (getRandomFloat(rng, -1, 1)); // negative = up, positive = down

		int wormLength = getRandomInt(rng, 200, 700); // how many steps worm takes
		float radius = 2.5f; // tunnel width in blocks

		int changeDirectionTime = getRandomInt(rng, 5, 20); // how often worm changes direction

		for (int j = 0; j < wormLength; j++)
		{
			// dig a circle around current position
			int intRadius = std::ceil(radius);
			for (int ox = -intRadius; ox <= intRadius; ox++)
			{
				for (int oy = -intRadius; oy <= intRadius; oy++)
				{
					float distSq = ox * ox + oy * oy;
					if (distSq <= radius * radius)
					{
						int digX = x + ox;
						int digY = y + oy;

						auto b = gameMap.getBlockSafe(digX, digY);
						if (b) 
						{
							b->type = Block::rubyBlock;
						}
					}
				}
			}

			changeDirectionTime--;
			if (changeDirectionTime <= 0)
			{
				changeDirectionTime = getRandomInt(rng, 5, 20);

				if (getRandomChance(rng, 0.7))
				{
					float keepFactor = 0.8;

					// big chance we keep a very similar direction
					dirX = dirX * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
					dirY = dirY * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
				}
				else
				{
					float keepFactor = 0.2;

					//big chance we keep a very similar direction
					dirX = dirX * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
					dirY = dirY * keepFactor + (getRandomFloat(rng, -1, 1)) * (1.f - keepFactor);
				}
			}
			// Move forward
			x += dirX * 1.5f;
			y += dirY * 1.5f;

			// random radius wobble
			radius += (getRandomFloat(rng, -0.2, 0.2));
			radius = std::clamp(radius, 2.2f, 8.5f);
		}
	}
}