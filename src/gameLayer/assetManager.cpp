#include <gameLayer/assetManager.h>

void AssetManager::loadAll()
{
	dirt = LoadTexture(RESOURCES_PATH "dirt.png");

	textures = LoadTexture(RESOURCES_PATH "textures.png");

	frame = LoadTexture(RESOURCES_PATH "frame.png");

	// Importing tree variants texture
	trees = LoadTexture(RESOURCES_PATH "treetextures.png");

	// Importing texture with wall variants
	walls = LoadTexture(RESOURCES_PATH "texturesWithBackgroundVersion.png");
}
