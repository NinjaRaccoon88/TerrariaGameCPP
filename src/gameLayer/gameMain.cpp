#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>

struct GameData
{
	float positionX = 100;
	float positionY = 100;
}gameData; // declares a variable called gameData of type GameData right here

bool initGame()
{
	return true;
}

bool updateGame()
{
	float deltaTime = GetFrameTime();
	if (deltaTime > 1.f / 5) { deltaTime = 1 / 5.f; }

	if (IsKeyDown(KEY_A)) { gameData.positionX -= 100.0f * deltaTime; }
	if (IsKeyDown(KEY_D)) { gameData.positionX += 100.0f * deltaTime; }
	if (IsKeyDown(KEY_W)) { gameData.positionY -= 100.0f * deltaTime; }
	if (IsKeyDown(KEY_S)) { gameData.positionY += 100.0f * deltaTime; }

	DrawRectangle(gameData.positionX, gameData.positionY, 50, 50, { 255,0,200,255 });

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

