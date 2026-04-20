#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>
#include <gameLayer/assetManager.h>
#include <gameLayer/gameMap.h>
#include <gameLayer/helper.h>

struct GameData
{
	GameMap gameMap;
	Camera2D camera;

}gameData; // declares a variable called gameData of type GameData right here

AssetManager assetManager;

bool initGame()
{
	assetManager.loadAll();

	gameData.gameMap.create(30, 10); // Create the game map

	// Spawning some test blocks to see how it works
	gameData.gameMap.getBlockUnsafe(0, 0).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(1, 1).type = Block::boneBricks;
	gameData.gameMap.getBlockUnsafe(2, 2).type = Block::sandChest;
	gameData.gameMap.getBlockUnsafe(3, 3).type = Block::table;
	gameData.gameMap.getBlockUnsafe(4, 4).type = Block::wordrobe;
	gameData.gameMap.getBlockUnsafe(5, 5).type = Block::blueRubyBlock;

	gameData.camera.target = { 0,0 }; // the point in the world the camera is looking at
	gameData.camera.rotation = 0.0f; // camera rotation in degrees (0 - no rotation obv)
	gameData.camera.zoom = 75.0f; // zoom level

	return true;
}

bool updateGame()
{
	float deltaTime = GetFrameTime(); // time in seconds since last frame
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; } // clamps deltaTime to max 0.2 seconds

	gameData.camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	ClearBackground({ 75,75,150,255 });

#pragma region Camera Movement
	// Moves camera target in world space
	if (IsKeyDown(KEY_LEFT)) gameData.camera.target.x -= 7.f * deltaTime;
	if (IsKeyDown(KEY_RIGHT)) gameData.camera.target.x += 7.f * deltaTime;
	if (IsKeyDown(KEY_UP)) gameData.camera.target.y -= 7.f * deltaTime;
	if (IsKeyDown(KEY_DOWN)) gameData.camera.target.y += 7.f * deltaTime;

#pragma endregion

	BeginMode2D(gameData.camera); // everything drawn after this is affected by the camera

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
				DrawTexturePro(
					assetManager.textures, // texture to draw

					// source - picks the correct tile from the atlas
					getTextureAtlas(b.type, 0, 32,32),

					// destination rectangle - where and how big :D 
					{ (float)x, (float)y, 1, 1},

					{ 0,0 }, // origin/pivot point for rotation (top-left corner)
					0.0f, // rotation in degrees
					WHITE // tint (WHITE - texture draw with no color change)
				);
			}
		}
	}

	EndMode2D(); // stop camera rendering

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

