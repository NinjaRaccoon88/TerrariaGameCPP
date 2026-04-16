#include <raylib.h>
#include "gameLayer/gameMain.h"
#include <iostream>
#include <fstream>

bool initGame()
{
	return true;
}

bool updateGame()
{
	DrawText("Congrats I have just created my 4th window", 190, 200, 20, RED);

	return true;
}

void closeGame()
{
	std::cout << "\nGAME IS CLOSED\n";
	std::ofstream f(RESOURCES_PATH "f.txt");
	f << "\nCLOSED\n";
	f.close();
}