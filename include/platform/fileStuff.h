#pragma once
#include <vector>
#include <fstream>

//				THE THREE FILE STREAM TYPES
// std::ofstream f; OUTPUT = writing to file
// std::ifstream f; INPUT = reading from file
// std::fstream f; BOTH reading and writing

//				OPENING AND CLOSING
// open in binary mode - raw bytes, no text conversation
// std::ofstream f (fileName, std::ios::binary);

// always check if it opened successfully!
// if (!f.is_open()) { return false; }

// close when done (also happens automatically when f goes out of scope)
// f.close();

//				WRITING BYTES
// write size bytes from data pointer
// needs to cast to char* because fstream works byte by byte
// f.write((const char*)data, size);

// check if write succeeded
// if (!f) { return false; } f evaluates to false if something went wrong


//	seekg()  is used to move the get pointer to a desired location
//
//	seekp() is used to move the put pointer to a desired location
//
//	tellp()  is used to know where the put pointer is in a file.
//
//	tellg() is used to know where the get pointer is in a file.


//				READING BYTES
// first you need to know how big the file is to allocate enough space
// f.seekg(0, std::ios::end); seek to END of file
// size_t fileSize = f.tellg(); tell me current position = file size!
// f.seekg(0, std::ios::beg); seek back to BEGINNING

// allocate buffer to hold the data
// std::vector<unsigned char> buffer(fileSize);

// read fileSize bytes into buffer
// f.read((char*)buffer.data(), fileSize();

bool writeEntireFile(const char* fileName, const void* data, size_t size);

std::vector<unsigned char> readEntireFile(const char* fileName);
