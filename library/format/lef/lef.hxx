#pragma once
#ifndef LEF_HXX
#define LEF_HXX

#include "../../scheduler/scheduler.hxx"
#include "../../common/string-utils.hxx"

class LEF {
	public:
	enum SectionAttributes {
		SECTION_ATTRIBUTE_READ_ONLY =							0x01
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) LEFHeader {
		uint32_t	Signature;										//	`LEF0`
		uint32_t	StackSize;
		uint32_t	BaseAddress;
		uint32_t	FileSize;
		uint32_t	EntryPointOffset;
		uint32_t	SectionsCount;
		uint32_t	SectionsOffset;
	} LEFHeader, *PLEFHeader;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) SectionHeader {
		uint32_t	SectionAttributes;
		uint32_t	SectionDataOffset;
		uint32_t	SectionDataSize;
	} SectionHeader, *PSectionHeader;

	static Scheduler::Thread CreateThread(const uint8_t* data, uint16_t dataSeg, uint16_t codeSeg, const char* curPath, bool suspended = false);
};

#endif