#pragma once
#ifndef BOOT_INFORMATION_HXX
#define BOOT_INFORMATION_HXX

#include "typedefs.hxx"

class BootInformation {
	public:
	enum ConfigSpaceAccessMechanismType {			//	PCI configuration space access mechanism type
		ACCESS_MECHANISM_NONE =			0x00,
		ACCESS_MECHANISM_STANDARD =		0x01,
		ACCESS_MECHANISM_DEPRECATED =	0x02,
		ACCESS_MECHANISM_BOTH =			0x03
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Structure {
		size_t	ARDTableAddress;
		size_t	ARDTableEntriesCount;
		size_t	BootFlags;
	} Structure, *PStructure;

	static void Initialize(const PStructure pointer);
	static size_t GetARDTableAddress();
	static size_t GetARDTableEntriesCount();
	static size_t GetBootFlags();
	static uint8_t GetConfigSpaceAccessMechanism();
	private:
	static Structure Structure_;
};

#endif