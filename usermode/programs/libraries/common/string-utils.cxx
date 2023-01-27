#include "string-utils.hxx"

const char StringUtils::Digits_[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'A', 'B',
	'C', 'D', 'E', 'F'
};

/**
 * @param value [in] unsigned value for which you want to get the length
 * @param base [in] number base (2,...)
 * @return unsigned value length or WRONG_INDEX
*/
size_t StringUtils::GetValueLength(size_t value, size_t base) {
	if (base < 2) return WRONG_INDEX;
	
	size_t length = 1;
	while (value /= base) ++length;
	return length;
}

/**
 * @param value [in] signed value for which you want to get the length
 * @param base [in] number base (2,...)
 * @return signed value length or WRONG_INDEX
*/
size_t StringUtils::GetValueLength(ptrdiff_t value, size_t base) {
	size_t unsignedValue = value < 0 ? -value : value;
	return GetValueLength(unsignedValue, base);
}

/**
 * @param buffer [out] buffer to write the result to
 * @param value [in] unsigned value to be converted
 * @param base [in] number base (2,...,16)
 * @return true if the number system is in the range (2,...,16), otherwise false
*/
bool StringUtils::GetString(char* buffer, size_t value, size_t base) {
	if (base < 2 || base > 16) return false;
	if (!value) {
		buffer[0] = '0';
		buffer[1] = 0;
		return true;
	}

	size_t valueCopy = value;
	size_t length = 1;

	while (valueCopy /= base) ++length;

	char* source = &((char*)buffer)[length];
	*source = 0;

	for (--source; value; --source) {
		*source = Digits_[value % base];
		value /= base;
	}
	return true;
}

/**
 * @param buffer [out] buffer to write the result to
 * @param value [in] signed value to be converted
 * @param base [in] number base (2,...,16)
 * @return true if the number system is in the range (2,...,16), otherwise false
*/
bool StringUtils::GetString(char* buffer, ptrdiff_t value, size_t base) {
	if (value < 0) {
		buffer[0] = '-';
		value = -value;
		++buffer;
	}

	return GetString(buffer, (size_t)value, base);
}

/**
 * @param buffer [in, out] buffer in which to change the letter case
*/
void StringUtils::RaiseLetterCase(char* buffer) {
	for (size_t i = 0; buffer[i]; i++) if (buffer[i] >= 'a' && buffer[i] <= 'z') buffer[i] &= '_';
}

/**
 * @param buffer [in, out] buffer in which to change the letter case
*/
void StringUtils::LowerLetterCase(char* buffer) {
	for (size_t i = 0; buffer[i]; i++) if (buffer[i] >= 'A' && buffer[i] <= 'Z') buffer[i] |= ' ';
}

size_t StringUtils::GetLength(const char* str) {
	size_t length = 0;
	for (; str[length];) ++length;
	return length;
}

bool StringUtils::Compare(const char* first, const char* second) {
	for (size_t i = 0; ; i++) {
		if (first[i] != second[i]) return false;
		else if (!first[i]) break;
	}

	return true;
}

size_t StringUtils::FindFirstSubstr(const char* source, const char* substr, size_t offset) {
	size_t srcLength = GetLength(source);
	size_t subLength = GetLength(substr);

	if (offset + subLength > srcLength || offset == WRONG_INDEX) return WRONG_INDEX;
	srcLength -= subLength;
	bool equal;
	for (; offset <= srcLength; offset++) {
		equal = true;
		for (size_t i = 0; substr[i]; i++) if (source[offset + i] != substr[i]) {
			equal = false;
			break;
		}
		if (equal) return offset;
	}

	return WRONG_INDEX;
}

bool StringUtils::Substr(char* destination, const char* source, size_t offset, size_t length) {
	size_t strLength = GetLength(source);
	if (offset >= strLength) return false;

	if (!length) length = strLength - offset;
	else if (offset + length > strLength) length = strLength - offset;

	MemoryUtils::Copy(destination, &source[offset], length);
	destination[length] = 0;
	return true;
}