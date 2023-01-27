#pragma once
#ifndef GDT_HXX
#define GDT_HXX

#include "../common/inline-assembly.hxx"

class GDT {
	public:
	enum AccessBits {
		ACCESS_READ_WRITE =		0x02,
		ACCESS_CONFIRMING =		0x04,
		ACCESS_EXECUTABLE =		0x08,
		ACCESS_CODE_DATA =		0x10,
		ACCESS_USER =			0x60,
		ACCESS_PRESENT =		0x80
	};

	enum FlagsBits {
		FLAG_LONG_MODE_CODE =	0x02,
		FLAG_PROTECTED_MODE =	0x04,
		FLAG_GRANULARITY =		0x08
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Register {
		uint16_t	Size;
		size_t		Offset;
	} Register, *PRegister;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Entry {
		uint16_t	LimitLow;
		uint32_t	BaseLow		:24;
		uint8_t		Access;
		uint8_t		LimitHigh	:4;
		uint8_t		Flags		:4;
		uint8_t		BaseHigh;
	} Entry, *PEntry;

	static void Select(PEntry table, size_t countEntries);
	static Entry CreateEntry(size_t limit, size_t base, uint8_t access, uint8_t flags);
	private:
	static Register Register_;
};

#endif