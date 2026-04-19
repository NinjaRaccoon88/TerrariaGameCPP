#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>
#include <gameLayer/assetManager.h>
#include <gameLayer/gameMap.h>

struct GameData
{
	GameMap gameMap;

}gameData; // declares a variable called gameData of type GameData right here

AssetManager assetManager;

bool initGame()
{
	assetManager.loadAll();

	gameData.gameMap.create(30, 10); // Create the game map

	// Spawning some test blocks to see how it works
	gameData.gameMap.getBlockUnsafe(0, 0).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(1, 1).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(2, 2).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(3, 3).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(4, 4).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(5, 5).type = Block::dirt;

	return true;
}

bool updateGame()
{
	float deltaTime = GetFrameTime(); // time in seconds since last frame
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; } // clamps deltaTime to max 0.2 seconds

	ClearBackground({ 75,75,150,255 });

	// nested loop visits every single block in map
	for (int y = 0; y < gameData.gameMap.h; y++) // loop every row
	{
		for (int x = 0; x < gameData.gameMap.w; x++) // loop every column
		{
			// gets reference to the block at x,y
			auto& b = gameData.gameMap.getBlockUnsafe(x, y);

			// only draw if block is not air (no point drawing empty block obv)
			if (b.type != Block::air)
			{
				float size = 32; // each block takes 32x32 pixels

				// convert block coordinates to pixel position
				float posX = x * size;
				float posY = y * size;

				DrawTexturePro(
					assetManager.dirt, // texture to draw
					Rectangle{ 0.f, 0.f, (float)assetManager.dirt.width,
					(float)assetManager.dirt.height }, // rectangle uses the whole texture

					// destination rectangle - where and how big :D 
					{ posX, posY, size, size }, 

					{ 0,0 }, // origin/pivot point for rotation (top-left corner)
					0.0f, // rotation in degrees
					WHITE // tint (WHITE - texture draw with no color change)
				);
			}
		}
	}
	return true;
}

void closeGame()
{
	std::cout << "\nGAME IS CLOSED\n";
	std::ofstream f(RESOURCES_PATH "f.txt");
	f << "\nCLOSED\n";
	f.close();
}

void closeLevel()
{
	// resets the game state
	gameData = {};

	// ...
}

void loadLevel()
{
	// resets the game state
	gameData = {};

	// ... do loading shit
}

