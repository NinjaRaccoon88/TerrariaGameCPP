#pragma once
#include "gameMap.h"

void generateWorld
		(
			GameMap& gameMap, int seed,
			int dirtOffsetStart, int dirtOffsetEnd,
			int stoneHeightStart, int stoneHeightEnd,
			float dirtFrequency, float stoneFrequency
		);