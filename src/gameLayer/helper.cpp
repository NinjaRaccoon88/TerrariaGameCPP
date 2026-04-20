#include <gameLayer/helper.h>

// takes a grid position in the atlas and returns the correct pixel Rectangle
Rectangle getTextureAtlas(int x, int y, int cellSizePixelsX, int cellSizePixelsY)
{
	return Rectangle{ 
		// pixel x = which column * tile width
		(float)x * cellSizePixelsX, 
		
		// pixel y = which row * tile height
		(float)y * cellSizePixelsY,

		// width of one tile in pixels
		(float)cellSizePixelsX, 

		// height of one tile in pixels
		(float)cellSizePixelsY };
}
