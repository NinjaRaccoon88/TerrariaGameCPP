#include <gameLayer/worldGenerator.h>
#include <gameLayer/randomStuff.h>
#include <FastNoiseSIMD/FastNoiseSIMD.h>

void generateWorld(GameMap& gameMap, int seed)
{
	const int w = 900;
	const int h = 500;

	gameMap.create(w, h);

	std::ranlux24_base rng(seed++); // rng seeded - same seed = same world

	// Create two separate noise generators - for dirt and for stone layers
	// unique_ptr automatically frees memory when it goes out of scope
	std::unique_ptr<FastNoiseSIMD> dirtNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());
	std::unique_ptr<FastNoiseSIMD> stoneNoiseGenerator(FastNoiseSIMD::NewFastNoiseSIMD());

	// give each generator a different seed so they produce diff noise patterns
	dirtNoiseGenerator->SetSeed(seed++);
	stoneNoiseGenerator->SetSeed(seed++);

	// SimplexFractal = smooth organic looking noise (faster than Perlin)
	dirtNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	dirtNoiseGenerator->SetFractalOctaves(1); // simple smooth waves - good for surface
	dirtNoiseGenerator->SetFrequency(0.02); // how zoomed in the noise is, higher = more chaos

	stoneNoiseGenerator->SetNoiseType(FastNoiseSIMD::NoiseType::SimplexFractal);
	stoneNoiseGenerator->SetFractalOctaves(4); // more detail layered on top of each other
	stoneNoiseGenerator->SetFrequency(0.01); // lower frequency = slower/wider waves

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

	int dirtOffsetStart = -5; // minimum dirt thickness
	int dirtOffsetEnd = 35; // maximum dirt thickness above stone

	int stoneHeightStart = 80; // minimum depth of stone layer
	int stoneHeightEnd = 170; // maximum depth of stone layer

	for (int x = 0; x < w; x++) // move column by column across the map
	{
		// this is LERP formula: start + (end - start) * t
		int stoneHeight = stoneHeightStart + (stoneHeightEnd - stoneHeightStart) * stoneNoise[x];
		int dirtHeight = dirtOffsetStart + (dirtOffsetEnd - dirtOffsetStart) * dirtNoise[x];

		// dirt surface = stone surface minus the dirt thickness offset
		// so dirt always sits a natural distance above stone
		dirtHeight = stoneHeight - dirtHeight;

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

	// IMPORTANT: must free manually since FastNoiseSIMD uses raw pointers, not smart pointers
	FastNoiseSIMD::FreeNoiseSet(dirtNoise);
	FastNoiseSIMD::FreeNoiseSet(stoneNoise);
}