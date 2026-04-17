#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>
#include <gameLayer/assetManager.h>

struct GameData
{
	Texture dirtTexture;

}gameData; // declares a variable called gameData of type GameData right here

AssetManager assetManager;

bool initGame()
{
	gameData.dirtTexture = LoadTexture(RESOURCES_PATH "dirt.png");

	return true;
}

bool updateGame()
{
	float deltaTime = GetFrameTime();
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

	DrawTexturePro(gameData.dirtTexture, { 0,0, (float)gameData.dirtTexture.width, (float)gameData.dirtTexture.height },
		{ 50, 50,100 ,100 }, {}, 0, WHITE);

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

