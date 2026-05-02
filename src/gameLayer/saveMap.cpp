#include <gameLayer/saveMap.h>
#include <platform/asserts.h>

bool saveBlockDataToFile(std::vector<Block> blocks, int w, int h, const char* fileName)
{
	// open the file in binary mode
	std::ofstream f(fileName, std::ios::binary);

	// if the file can't be opened, return false
	if (!f.is_open()) { return false; }

	// only works in development build
	permaAssertDevelopement(blocks.size() == w * h);
	permaAssertDevelopement(blocks.size() != 0);

	// works in production mode so it doesn't crash the game
	if (blocks.size() != w * h) { return false; }
	if (blocks.size() == 0) { return false; }
	
	f.write((const char*)&w, sizeof(w));
	f.write((const char*)&h, sizeof(h));

	f.write((const char*)blocks.data(), sizeof(Block) * blocks.size());

	f.close();

	return true;
}

bool loadBlockDataToFile(std::vector<Block>& blocks, int& w, int& h, const char* fileName)
{
	// always clear first
	blocks.clear();
	w = 0;
	h = 0;

	std::ifstream f(fileName, std::ios::binary);

	if (!f.is_open()) { return false; }

	// read dimensions
	f.read((char*)&w, sizeof(w));
	f.read((char*)&h, sizeof(h));

	// if reading failed or values are less then 0 - return false
	if (!f || w <= 0 || h <= 0)
	{
		f.close();
		return false;
	}

	if (w > 10000) { f.close(); return false; } // probably corrupt data
	if (h > 10000) { f.close(); return false; } // probably corrupt data

	// read block data
	size_t blockCount = w * h;
	blocks.resize(blockCount);

	f.read((char*)blocks.data(), sizeof(Block) * blockCount);

	// if it fails
	if (!f)
	{
		blocks.clear();
		w = 0;
		h = 0;
		f.close();
		return false;
	}

	for (int i = 0; i < blocks.size(); i++)
	{
		blocks[i].sanitize();
	}

	f.close();
	return true;
}
