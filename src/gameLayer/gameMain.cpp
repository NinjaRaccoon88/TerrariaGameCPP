#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>
#include <gameLayer/assetManager.h>
#include <gameLayer/gameMap.h>
#include <gameLayer/helper.h>
#include <raymath.h>

struct GameData
{
	GameMap gameMap;
	Camera2D camera;

	//TEMPORARY
	int selectedBlock = Block::leaves;

}gameData; // declares a variable called gameData of type GameData right here

AssetManager assetManager;

bool initGame()
{
	assetManager.loadAll();

	gameData.gameMap.create(700, 500); // Create the game map

	for (int i = 0; i < 700; i++)
	{
		for (int j = 0; j < 500; j++)
		{
			gameData.gameMap.getBlockUnsafe(i, j).type = Block::stone;
		}
	}

	// Spawning some test blocks to see how it works
	gameData.gameMap.getBlockUnsafe(0, 0).type = Block::dirt;
	gameData.gameMap.getBlockUnsafe(1, 1).type = Block::boneBricks;
	gameData.gameMap.getBlockUnsafe(2, 2).type = Block::sandChest;
	gameData.gameMap.getBlockUnsafe(3, 3).type = Block::table;
	gameData.gameMap.getBlockUnsafe(4, 4).type = Block::skin;
	gameData.gameMap.getBlockUnsafe(5, 5).type = Block::head;

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

	// Converts mouse position from screen pixles to world coordinates
	// accounts for the camera position, zoom and offset.
	Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), gameData.camera);

	// Converts world position to block grid coord
	// floor rounds down so holding a cursor anywhere inside a block selects that block
	int blockX = (int)floor(worldPos.x);
	int blockY = (int)floor(worldPos.y);

	if (IsKeyDown(KEY_ONE))
	{
		gameData.selectedBlock = Block::leaves;
	}
	if (IsKeyDown(KEY_TWO))
	{
		gameData.selectedBlock = Block::woodLog;
	}


	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
		if (b)
		{
			*b = {};
		}
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		auto b = gameData.gameMap.getBlockSafe(blockX, blockY);
		if (b)
		{
			b->type = gameData.selectedBlock;
		}
	}
	
	BeginMode2D(gameData.camera); // everything drawn after this is affected by the camera

	// converts top-left corner of the screen to world coordinates
	// tells us where the camera can see from on the top-left
	Vector2 topLeftView = GetScreenToWorld2D
	(
		{ 0,0 },
		gameData.camera
	);

	// converts bottom-right corner of the screen to world coordinates
	// tells us where the camera can see from on the right-bottom
	Vector2 bottomRightView = GetScreenToWorld2D
	(
		{(float)GetScreenWidth(), 
		(float)GetScreenHeight()}, 
		gameData.camera
	);

	// Convert world view bounds to block grid coordinates
	// -1/+1 add a small buffer so blocks at the very edge don't pop in/out
	int startXView = (int)floorf(topLeftView.x - 1);
	int endXView = (int)ceilf(bottomRightView.x + 1);
	int startYView = (int)floorf(topLeftView.y - 1);
	int endYView = (int)ceilf(bottomRightView.y + 1);

	// clamps view bounds to stay inside actual map boundaries
	// prevents trying to draw blocks that don't exist outside the map
	startXView = Clamp(startXView, 0, gameData.gameMap.w - 1);
	endXView = Clamp(endXView, 0, gameData.gameMap.w - 1);
	startYView = Clamp(startYView, 0, gameData.gameMap.h - 1);
	endYView = Clamp(endYView, 0, gameData.gameMap.h - 1);

	// nested loop visits every single block in map
	for (int y = startYView; y <= endYView; y++) // loop every row
	{
		for (int x = startXView; x < endXView; x++) // loop every column
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
	// draw selected block
	DrawTexturePro(
		assetManager.frame,
		{ 0,0, (float)assetManager.frame.width, (float)assetManager.frame.height }, // source
		{ (float)blockX, (float)blockY, 1, 1 }, // dest
		{ 0,0 }, // origin (top-left corner)
		0.0f, // rotation
		WHITE // tint
		);

	DrawFPS(10, 10);

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

