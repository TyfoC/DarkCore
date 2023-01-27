#pragma once
#ifndef IDT_HXX
#define IDT_HXX

#include "../common/inline-assembly.hxx"

class IDT {
	public:
	enum GateTypes {
		GATE_TASK =		0x05,
		GATE_INT16 =	0x06,
		GATE_TRAP16 =	0x07,
		GATE_INT32 =	0x0E,
		GATE_TRAP32 =	0x0F
	};

	enum FlagsBits {
		FLAG_USER =		0x06,
		FLAG_PRESENT =	0x08
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Register {
		uint16_t	Size;
		size_t		Offset;
	} Register, *PRegister;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Entry {
		uint16_t	OffsetLow;
		uint16_t	SegmentSelector;
		uint8_t		Reserved0;
		uint8_t		GateType			:4;
		uint8_t		Flags				:4;
		uint16_t	OffsetHigh;
	} Entry, *PEntry;

	static void Select(PEntry table, size_t countEntries);
	static Entry CreateEntry(size_t offset, uint16_t segmentSelector, uint8_t gateType, uint8_t flags);
	private:
	static Register Register_;
};

#endif