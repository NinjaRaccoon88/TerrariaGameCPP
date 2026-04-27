#pragma once
#include "gameMap.h"

// generateWorld now accepts all tweakable values as parameters
// instead of having them hardcoded inside the function
// this allows ImGui to control them from gameMain.cpp in real time
void generateWorld
		(
			GameMap& gameMap, int seed,
			int dirtOffsetStart, int dirtOffsetEnd,
			int stoneHeightStart, int stoneHeightEnd,
			float dirtFrequency, float stoneFrequency
		);