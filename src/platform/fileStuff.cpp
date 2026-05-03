#include <platform/fileStuff.h>

bool writeEntireFile(const char* fileName, const void* data, size_t size)
{
	// 1. Open file in binary write mode
	std::ofstream f(fileName, std::ios::binary);

	// 2. Check it opened
	if (!f.is_open()) { return false; }

	// 3. Write the data
	f.write((const char*)data, size);

	// 4. Check write succeeded
	if (!f) { return false; }

	// 5. Close and return the success
	f.close();

	return true;
}

std::vector<unsigned char> readEntireFile(const char* fileName)
{
	// reading from file
	std::ifstream f(fileName, std::ios::binary);

	// if file can't be opened return empty vector
	if (!f.is_open()) { return {}; }

	// seek to END of file	
	f.seekg(0, std::ios::end);
	size_t fileSize = f.tellg();
	f.seekg(0, std::ios::beg); // seek back to the beginning to read from start

	// allocate the buffer to hold the data
	std::vector<unsigned char> buffer(fileSize);

	// read fileSize into the buffer
	f.read((char*)buffer.data(), fileSize);

	// check if read succeeded
	if (!f) { return {}; }

	f.close();

	return buffer;
}