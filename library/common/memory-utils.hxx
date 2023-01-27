#pragma once
#ifndef MEMORY_UTILS_HXX
#define MEMORY_UTILS_HXX

#include "typedefs.hxx"

class MemoryUtils {
	public:
	static void Copy(void* destination, const void* source, size_t count);
	static void Fill(void* destination, uint8_t value, size_t count);
	static bool Compare(const void* first, const void* second, size_t count);
};

#endif