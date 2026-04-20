#pragma once
#include <raylib.h>

// takes a grid position in the atlas and returns the correct pixel Rectangle
Rectangle getTextureAtlas(int x, int y, int cellSizePixelsX, int cellSizePixelsY);