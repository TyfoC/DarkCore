#pragma once
#ifndef STRING_HXX
#define STRING_HXX

#include "string-utils.hxx"
#include "list.hxx"

class DEFINE_SPECIAL(PACKED_DEFINITION) String {
	public:
	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		size_t	StartOffset;
		size_t	Length;
	} Part, *PPart;

	using Parts = List<Part>;

	String();
	String(const char* str);
	String(const String& str);
	~String();

	char& operator[](size_t index);
	const char& operator[](size_t index) const;

	String& operator()();
	String& operator()(const char* str);
	String& operator()(const String& str);

	String& operator=(const String& str);
	String operator+(const String& str);
	String& operator+=(const String& str);
	bool operator==(const String& rightStr) const;
	bool operator!=(const String& rightStr) const;

	char* GetRawString() const;

	size_t GetLength() const;
	size_t FindFirstMatch(const String& subStr, size_t startPosition = 0) const;
	size_t FindLastMatch(const String& subStr, size_t startPosition = 0) const;
	String Erase(size_t position = 0, size_t count = 0);

	String GetSubString(size_t startPosition, size_t length = 0) const;
	List<Part> Split(const String& delimeter) const;
	private:
	char*	Source_;
};

#endif