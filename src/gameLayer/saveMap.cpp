#include <gameLayer/saveMap.h>
#include <platform/asserts.h>

// VERSION 1 - old format, only had type
// this is a snapshot of what Block looked like when version 1 was saved
struct BlockSaveRepresentation1 
{

	std::uint16_t type = 0; // only type, nothing else

	// converts this old format back into a modern block
	Block toBlock()
	{
		Block b;
		b.type = type;
		// durability not in v1, so Block's default value of 1 is used automatically
		return b;
	}
};
// VERSION 2 - current format, added durability
struct BlockSaveRepresentation2 
{

	std::uint16_t type = 0;
	std::uint16_t durability = 0; // new field added in v2

	// converts this format into a modern Block
	Block toBlock()
	{
		Block b;
		b.type = type;
		b.durability = durability; // now we restore durability too
		return b;
	}
};

// this number gets written to every save file
// current version of the file save system
const int VERSION = 2;

BlockSaveRepresentation2 toBlockRepresentation(Block b)
{
	BlockSaveRepresentation2 rez;
	rez.type = b.type;
	rez.durability = b.durability;
	return rez;
}

bool saveBlockDataToFile(const std::vector<Block> &blocks, int w, int h, const char* fileName)
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
	
	// write VERSION number first - this is the "header"
	// every file starts with this so loader knows what format
	f.write((const char*)&VERSION, sizeof(VERSION));
	f.write((const char*)&w, sizeof(w));
	f.write((const char*)&h, sizeof(h));

	// write each block using the CURRENT representation (v2)
	for (int i = 0; i < blocks.size(); i++)
	{
		auto b = toBlockRepresentation(blocks[i]); // converts to saveable format
		f.write((const char*)&b, sizeof(b)); // write only what v2 needs
	}

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

	int readVersion = 0;

	// read the version
	f.read((char*)&readVersion, sizeof(readVersion));
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

	// switch on version - each case knows exactly how that version was saved
	switch (readVersion)
	{
		case 1:
		{
			// read block data
			size_t blockCount = w * h;
			blocks.resize(blockCount);

			for (int i = 0; i < blockCount; i++)
			{
				BlockSaveRepresentation1 read; // use V1 struct to read V1 data
				f.read((char*)&read, sizeof(read)); // read ONLY type

				if (!f) // read failed - file is corrupt
				{
					blocks.clear();
					w = 0;
					h = 0;
					f.close();
					return false;
				}

				blocks[i] = read.toBlock(); // convert V1 data to modern Block
				// durability will be default value 1 since v1 didn't have it
			}

			break;
		}
		case 2:
		{
			// read block data
			size_t blockCount = w * h;
			blocks.resize(blockCount);

			for (int i = 0; i < blockCount; i++)
			{
				BlockSaveRepresentation2 read; // use V2 struct to read V2 data
				f.read((char*)&read, sizeof(read)); // reads the type + durability

				if (!f)
				{
					blocks.clear();
					w = 0;
					h = 0;
					f.close();
					return false;
				}

				blocks[i] = read.toBlock(); // converts V2 data to modern block
			}

			break;
		}
		default:
		{
			// incorrect/unknown version - file from future version or corrupt
			// can't load it, return false
			w = 0;
			h = 0;
			f.close();
			return false;
		}
	}

	// sanitize every block - reset invalid types to air
	// protects against corrupt type values
	for (int i = 0; i < blocks.size(); i++)
	{
		blocks[i].sanitize();
	}

	f.close();
	return true;
}
