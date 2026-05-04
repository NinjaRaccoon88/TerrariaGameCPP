#pragma once
#include <vector>
#include <gameLayer/blocks.h>
#include <fstream>

bool saveBlockDataToFile(const std::vector<Block> &blocks, int w, int h, const char* fileName);

bool loadBlockDataToFile(std::vector<Block> &blocks, int &w, int &h, const char* fileName);