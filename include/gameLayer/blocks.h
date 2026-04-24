#pragma once
#include <cstdint>

struct Block
{
	enum
	{
		air = 0,
		dirt,
		grassBlock,
		stone,
		grass,
		sand,
		sandRuby,
		sandStone,
		woodPlank,
		stoneBricks,
		clay,
		woodLog,
		leaves,
		copper,
		iron,
		gold,
		copperBlock,
		ironBlock,
		goldBlock,
		bricks,
		snow,
		ice,
		rubyBlock,
		platform,
		workBench,
		glass,
		furnace,
		painting,
		sappling,
		snowBlueRuby,
		blueRubyBlock,
		door,
		jar,
		table,
		wordrobe,
		bookShelf,
		snowBricks,
		iceTable,
		iceWordrobe,
		iceBookShelf,
		icePlatform,
		sandTable,
		sandWordrobe,
		sandBookShelf,
		sandPlatform,
		woodenChest,
		iceChest,
		sandChest,
		boneChest,
		boneBricks,
		boneBench,
		boneWordrobe,
		boneBookShelf,
		bonePlatform,
		head,
		skin,

		BLOCKS_COUNT

	};

	std::uint16_t type = 0; // type of a block

	std::uint8_t variation = 0; // variation of the block
};

struct Wall
{
	enum
	{
		air = 0,
		dirtWall,
		stoneWall,
		woodWall,
		sandStoneWall,
		brickWall,
		glassWall,
		copperBlockWall,
		silverBlockWall,
		goldBlockWall,
		snowWall,
		sandWall,
		stoneBricksWall,
		rubyBlockWall,
		heroglyphWall,
		blueRubyWall,
		plankedWall,
		snowBrickWall,
		boneBrickWall,
		headWall,
		skinWall,

		WALL_COUNT
	};

	std::uint16_t type = 0; // type of a block

	std::uint8_t variation = 0; // variation of the block
};