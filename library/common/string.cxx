#include "string.hxx"

String::String() {
	operator()();
}

String::String(const char* str) {
	operator()(str);
}

String::String(const String& str) {
	operator()(str);
}

String::~String() {
	if (Source_) delete[] Source_;
}

String& String::operator()() {
	Source_ = 0;
	return *this;
}

String& String::operator()(const char* str) {
	size_t strLength = StringUtils::GetLength(str);
	Source_ = new char[strLength + 1];
	if (Source_) MemoryUtils::Copy(Source_, str, strLength + 1);
	return *this;
}

String& String::operator()(const String& str) {
	size_t strLength = str.GetLength();
	Source_ = new char[strLength + 1];
	if (Source_) MemoryUtils::Copy(Source_, str.Source_, strLength + 1);
	return *this;
}

char& String::operator[](size_t index) {
	return Source_[index];
}

const char& String::operator[](size_t index) const {
	return Source_[index];
}

String& String::operator+=(const String& str) {
	if (Source_ && str.Source_) {
		size_t sourceLen = GetLength();
		size_t strLen = str.GetLength();
		char* newSource = new char[sourceLen + strLen + 1];
		if (newSource) {
			MemoryUtils::Copy(newSource, Source_, sourceLen);
			MemoryUtils::Copy(&newSource[sourceLen], str.Source_, strLen + 1);
			delete[] Source_;
			Source_ = newSource;
		}
	}
	return *this;
}

String& String::operator=(const String& str) {
	size_t strLength = str.GetLength();
	char* newSource = new char[strLength + 1];
	if (newSource) {
		if (Source_) delete[] Source_;
		Source_ = newSource;
		MemoryUtils::Copy(Source_, str.Source_, strLength + 1);
	}
	return *this;
}

String String::operator+(const String& str) {
	String newString(Source_);
	newString += str;
	return newString;
}

bool String::operator==(const String& rightStr) const {
	if (!Source_ || !rightStr.Source_) return false;
	size_t sourceLength = GetLength();
	size_t rightStrLength = rightStr.GetLength();
	if (sourceLength != rightStrLength) return false;
	return MemoryUtils::Compare(Source_, rightStr.Source_, sourceLength);
}

bool String::operator!=(const String& rightStr) const {
	return !operator==(rightStr);
}

char* String::GetRawString() const {
	return Source_;
}

size_t String::GetLength() const {
	if (Source_) return StringUtils::GetLength(Source_);
	return 0;
}

size_t String::FindFirstMatch(const String& subStr, size_t startPosition) const {
	if (Source_ && subStr.Source_) return StringUtils::FindFirstSubstr(Source_, subStr.Source_, startPosition);
	return WRONG_INDEX;
}

size_t String::FindLastMatch(const String& subStr, size_t startPosition) const {
	if (Source_ && subStr.Source_) return StringUtils::FindLastSubstr(Source_, subStr.Source_, startPosition);
	return WRONG_INDEX;
}

String String::Erase(size_t position, size_t count) {
	size_t myLength = GetLength();
	if (position >= myLength) position = 0;
	if (count >= myLength) count = myLength - count;

	String newString(String(Source_).GetSubString(0, position));
	newString += &Source_[position + count];

	return newString;
}

String String::GetSubString(size_t startPosition, size_t length) const {
	if (!Source_) return String();
	if (!length) length = GetLength() - startPosition;

	char* resStr = new char[length + 1];
	MemoryUtils::Copy(resStr, &Source_[startPosition], length);
	resStr[length] = 0;
	return String(resStr);
}

List<String::Part> String::Split(const String& delimeter) const {
	if (!Source_ && !Source_[0]) return {};

	List<Part> parts = {};
	size_t prevDelimeterPos = 0, nextDelimeterPos = 0;
    size_t startPartPos = 0, partLength = 0;
	const size_t pathLength = GetLength();
	const size_t delimeterLength = delimeter.GetLength();

	if (!Source_[0]) return parts;
    while (!StringUtils::FindFirstSubstr(&Source_[prevDelimeterPos], delimeter.Source_)) ++prevDelimeterPos;
    
    do {
        nextDelimeterPos =  StringUtils::FindFirstSubstr(Source_, delimeter.Source_, prevDelimeterPos + delimeterLength);
        startPartPos = !StringUtils::FindFirstSubstr(&Source_[prevDelimeterPos], delimeter.Source_) ? prevDelimeterPos + delimeterLength : prevDelimeterPos;
        if (nextDelimeterPos != WRONG_INDEX) {
            partLength = nextDelimeterPos - startPartPos;
            parts.Append({ startPartPos, partLength });
            prevDelimeterPos = nextDelimeterPos;
        }
        else {
            partLength = pathLength - startPartPos;
            if (partLength) parts.Append({ startPartPos, partLength });
        }
    } while (nextDelimeterPos != WRONG_INDEX);

	return parts;
}