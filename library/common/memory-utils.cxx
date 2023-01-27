#include "memory-utils.hxx"

/**
 * @param destination [out] destination buffer
 * @param source [in] source buffer
 * @param count [in] count of bytes to copy
*/
void MemoryUtils::Copy(void* destination, const void* source, size_t count) {
	size_t i = 0;
	if ((count & 0x00000007) == 0x00000004 || !(count & 0x00000007)) {
		puint32_t destination32 = (puint32_t)destination;
		puint32_t source32 = (puint32_t)source;

		for (; i < count >> 2; i++) destination32[i] = source32[i];
	}
	else if ((count & 0x00000003) == 0x00000002 || !(count & 0x00000003)) {
		puint16_t destination16 = (puint16_t)destination;
		puint16_t source16 = (puint16_t)source;

		for (; i < count >> 1; i++) destination16[i] = source16[i];
	}
	else {
		puint8_t destination8 = (puint8_t)destination;
		puint8_t source8 = (puint8_t)source;

		for (; i < count; i++) destination8[i] = source8[i];
	}
}

/**
 * @param destination [out] destination buffer
 * @param value [in] value to fill
 * @param count [in] count of bytes to fill
*/
void MemoryUtils::Fill(void* destination, uint8_t value, size_t count) {
	size_t i = 0;
	if ((count & 0x00000007) == 0x00000004 || !(count & 0x00000007)) {
		puint32_t destination32 = (puint32_t)destination;
		uint32_t value32 = (uint32_t)(((uint32_t)value << 24) | ((uint32_t)value << 16) | ((uint32_t)value << 8) | (uint32_t)value);

		for (; i < count >> 2; i++) destination32[i] = value32;
	}
	else if ((count & 0x00000003) == 0x00000002 || !(count & 0x00000003)) {
		puint16_t destination16 = (puint16_t)destination;
		uint16_t value16 = (uint16_t)(((uint16_t)value << 8) | (uint16_t)value);

		for (; i < count >> 1; i++) destination16[i] = value16;
	}
	else {
		puint8_t destination8 = (puint8_t)destination;

		for (; i < count; i++) destination8[i] = value;
	}
}

/**
 * @param first [in] first buffer
 * @param second [in] second buffer
 * @param count [in] count of bytes to compare
 * @return true if the data of the memory areas are equal, otherwise false
*/
bool MemoryUtils::Compare(const void* first, const void* second, size_t count) {
	size_t i = 0;
	if ((count & 0x00000007) == 0x00000004 || !(count & 0x00000007)) {
		puint32_t first32 = (puint32_t)first;
		puint32_t second32 = (puint32_t)second;

		for (; i < count >> 2; i++) if (first32[i] != second32[i]) return false;
	}
	else if ((count & 0x00000003) == 0x00000002 || !(count & 0x00000003)) {
		puint16_t first16 = (puint16_t)first;
		puint16_t second16 = (puint16_t)second;

		for (; i < count >> 1; i++) if (first16[i] != second16[i]) return false;
	}
	else {
		puint8_t first8 = (puint8_t)first;
		puint8_t second8 = (puint8_t)first;

		for (; i < count; i++) if (first8[i] != second8[i]) return false;
	}

	return true;
}