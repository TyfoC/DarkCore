#pragma once
#ifndef STRING_UTILS_HXX
#define STRING_UTILS_HXX

#include "memory-utils.hxx"

class StringUtils {
	public:
	static size_t GetValueLength(size_t value, size_t base);
	static size_t GetValueLength(ptrdiff_t value, size_t base);
	static bool GetString(char* buffer, size_t value, size_t base);
	static bool GetString(char* buffer, ptrdiff_t value, size_t base);
	static void RaiseLetterCase(char* buffer);
	static void LowerLetterCase(char* buffer);
	static size_t GetLength(const char* str);
	static bool Compare(const char* first, const char* second);
	static size_t FindFirstSubstr(const char* source, const char* substr, size_t offset = 0);
	static bool Substr(char* destination, const char* source, size_t offset = 0, size_t length = 0);
	private:
	static const char Digits_[16];
};

#endif