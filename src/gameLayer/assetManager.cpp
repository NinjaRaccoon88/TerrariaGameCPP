#include <gameLayer/assetManager.h>

void AssetManager::loadAll()
{
	dirt = LoadTexture(RESOURCES_PATH "dirt.png");

	textures = LoadTexture(RESOURCES_PATH "textures.png");
}
