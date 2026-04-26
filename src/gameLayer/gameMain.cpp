#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>
#include <gameLayer/assetManager.h>
#include <gameLayer/gameMap.h>
#include <gameLayer/helper.h>
#include <raymath.h>
#include <gameLayer/randomStuff.h>
#include <gameLayer/worldGenerator.h>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#include "ImGuiStyle.h"
#pragma endregion

struct GameData
{
	GameMap gameMap;
	Camera2D camera;

	//TEMPORARY
	int selectedBlock = Block::leaves;
	int selectedWall = Wall::skinWall;

}gameData; // declares a variable called gameData of type GameData right here

AssetManager assetManager;

//TEMPORARY
std::ranlux24_base rng(std::random_device{}()); // seeded with a truly random value

bool initGame()
{
	assetManager.loadAll();

	generateWorld(gameData.gameMap);


	gameData.camera.target = { 20 ,120 }; // the point in the world the camera is looking at
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
	static float CAMERA_SPEED = 7.0f;
	if (IsKeyDown(KEY_LEFT)) gameData.camera.target.x -= CAMERA_SPEED * deltaTime;
	if (IsKeyDown(KEY_RIGHT)) gameData.camera.target.x += CAMERA_SPEED * deltaTime;
	if (IsKeyDown(KEY_UP)) gameData.camera.target.y -= CAMERA_SPEED * deltaTime;
	if (IsKeyDown(KEY_DOWN)) gameData.camera.target.y += CAMERA_SPEED * deltaTime;

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
	if (IsKeyDown(KEY_THREE))
	{
		gameData.selectedWall = Wall::silverBlockWall;
	}
	if (IsKeyDown(KEY_FOUR))
	{
		gameData.selectedWall = Wall::snowWall;
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
		if (b && b->type != gameData.selectedBlock) // only place if different block
		{
			b->type = gameData.selectedBlock;
			b->variation = getRandomInt(rng, 0, 3); // picks 0,1,2 or 3 randomly
		}
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		auto b = gameData.gameMap.getWallSafe(blockX, blockY);
		if (b)
		{
			*b = {};
		}
	}
	if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE))
	{
		auto b = gameData.gameMap.getWallSafe(blockX, blockY);
		if (b && b->type != gameData.selectedWall) // only place if different block
		{
			b->type = gameData.selectedWall;
			b->variation = getRandomInt(rng, 0, 3); // picks 0, 1,2 or 3 randomly
		}
	}
	
	// temporarily add this inside updateGame() to see the values
	DrawText(TextFormat("Selected Wall: %d", gameData.selectedWall), 10, 30, 20, RED);

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

	// nested loop visits every single WALL block in map
	for (int y = startYView; y <= endYView; y++) // loop every row first
	{
		for (int x = startXView; x < endXView; x++) // loop every column
		{
			// gets reference to the WALL block in map
			auto& b = gameData.gameMap.getWallUnsafe(x, y);

			// only draw if block is not air (no point drawing empty block)
			if (b.type != Wall::air)
			{
				// I have no idea why (-3) but if it works - it works
				// I added 2 new blocks to the texture and there is air block
				// so maybe I need to do -3 for those 3 blocks, welp
				Rectangle sourceRect = getTextureAtlas(b.type - 3 + Block::BLOCKS_COUNT, b.variation, 32, 32);

				// Drawing
				DrawTexturePro
				(
					assetManager.walls,
					sourceRect, // source
					{ (float)x, (float)y, 1,1 }, // dest
					{ 0,0 }, // origin/pivot point for rotation (top - left corner)
					0.0f, // rotation in degrees
					WHITE // tint (WHITE - texture draw with no color change)
				);
			}
		}
	}

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
				Rectangle sourceRect = getTextureAtlas(b.type, b.variation, 32, 32);

				if (b.type == Block::woodLog)
				{
					// Checking for the block above (y - 1)
					Block* above = gameData.gameMap.getBlockSafe(x, y - 1);

					// Checking for the block below (y + 1)
					Block* below = gameData.gameMap.getBlockSafe(x, y + 1);

					// Checking for the block on the left (x - 1)
					Block* left = gameData.gameMap.getBlockSafe(x - 1, y);

					// Checking for the block on the right (x + 1)
					Block* right = gameData.gameMap.getBlockSafe(x + 1, y);

					// Bool for leaves
					bool leafAbove = above && above->type == Block::leaves;
					bool logBelow = below && below->type == Block::woodLog;
					bool leafLeft = left && left->type == Block::leaves;
					bool leafRight = right && right->type == Block::leaves;
					bool logAbove = above && above->type == Block::woodLog;
					bool leafBelow = below && below->type == Block::leaves;

					// Fucking 8 if statements upcoming, hardcoding is a way to go!
					// also picking the rigth order is pain in the ass

					// if we have another log below us, place regular wood Log
					if (leafLeft && leafRight && !logBelow && !logAbove) // most specific first
						sourceRect = getTextureAtlas(5, b.variation, 32, 32); // surrounded by leaves
					else if (leafLeft && leafRight)
						sourceRect = getTextureAtlas(1, b.variation, 32, 32); // leaves both sides
					else if (leafLeft)
						sourceRect = getTextureAtlas(3, b.variation, 32, 32); // leaf left only
					else if (leafRight)
						sourceRect = getTextureAtlas(2, b.variation, 32, 32); // leaf right only
					else if (!logBelow && !logAbove)
						sourceRect = getTextureAtlas(7, b.variation, 32, 32); // completely isolated stump
					else if (!logBelow)
						sourceRect = getTextureAtlas(4, b.variation, 32, 32); // Stump with log above
					else if (!logAbove && !leafBelow)
						sourceRect = getTextureAtlas(6, b.variation, 32, 32); // top of tree, no leaves
					else if (logBelow)
						sourceRect = getTextureAtlas(0, b.variation, 32, 32); // regular log
					
					// Drawing
					DrawTexturePro
					(
						assetManager.trees,
						sourceRect, // source
						{ (float)x, (float)y, 1,1 }, // dest
						{ 0,0 }, // origin/pivot point for rotation (top - left corner)
						0.0f, // rotation in degrees
						WHITE // tint (WHITE - texture draw with no color change)
					);
				}
				else {
					DrawTexturePro(
						assetManager.textures, // texture to draw

						// source - picks the correct tile from the atlas
						sourceRect,

						// destination rectangle - where and how big :D 
						{ (float)x, (float)y, 1, 1 },

						{ 0,0 }, // origin/pivot point for rotation (top-left corner)
						0.0f, // rotation in degrees
						WHITE // tint (WHITE - texture draw with no color change)
					);
				}
			}
		}
	};
	// draw selected block
	DrawTexturePro
		(
		assetManager.frame,
		{ 0,0, (float)assetManager.frame.width, (float)assetManager.frame.height }, // source
		{ (float)blockX, (float)blockY, 1, 1 }, // dest
		{ 0,0 }, // origin (top-left corner)
		0.0f, // rotation
		WHITE // tint
		);

	EndMode2D(); // stop camera rendering

	ImGui::Begin("Game Controll");

	ImGui::SliderFloat("Camera zoom:", &gameData.camera.zoom, 10, 150);
	ImGui::SliderFloat("Camera speed:", &CAMERA_SPEED, 5, 30);

	ImGui::End();

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

