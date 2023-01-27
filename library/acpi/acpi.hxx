#pragma once
#ifndef ACPI_H
#define ACPI_H

#include "../common/memory-utils.hxx"
#include "../virtual-drivers/virtual-timer.hxx"

class ACPI {
	public:
	enum DefaultSignatures {
		SIGNATURE_RSDT =		0x54445352,
		SIGNATURE_XSDT =		0x54445358,
		SIGNATURE_FADT =		0x50434146,
		SIGNATURE_DSDT =		0x54445344,
		SIGNATURE_DSDT_S5 =		0x5F35535F
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		uint64_t		Signature;
		uint8_t			Checksum;
		char			OEMID[6];
		uint8_t			Revision;
		uint32_t		RSDTAddress;

		//	since ACPI 2.0
		uint32_t		Length;
		uint64_t		XSDTAddress;
		uint8_t			ExtendedChecksum;
		uint32_t		Reserved			:24;
	} RSDP, *PRSDP;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		uint32_t		Signature;
		uint32_t		Length;
		uint8_t			Revision;
		uint8_t			Checksum;
		char			OEMID[6];
		char			OEMTableID[8];
		uint32_t		OEMRevision;
		uint32_t		CreatorID;
		uint32_t		CreatorRevision;
	} SDTHeader, *PSDTHeader;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		SDTHeader		Header;
		uint32_t		Table[];
	} RSDT, *PRSDT;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		SDTHeader		Header;
		uint64_t		Table[];
	} XSDT, *PXSDT;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		uint8_t			AddressSpace;
		uint8_t			BitWidth;
		uint8_t			BitOffset;
		uint8_t			AccessSize;
		uint64_t		Address;
	} GenericAddress, *PGenericAddress;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		SDTHeader		Header;
		uint32_t		FirmwareControl;
		uint32_t		DSDTAddress;
		uint8_t			Reserved1;
		uint8_t			PreferredPowerManagementProfile;
		uint16_t		SCIInterrupt;
		uint32_t		SMICommand;
		uint8_t			ACPIEnable;
		uint8_t			ACPIDisable;
		uint8_t			S4BIOSReq;
		uint8_t			PStateControl;
		uint32_t		PM1AEventBlock;
		uint32_t		PM1BEventBlock;
		uint32_t		PM1AControlBlock;
		uint32_t		PM1BControlBlock;
		uint32_t		PM2ControlBlock;
		uint32_t		PMTimerBlock;
		uint32_t		GPE0Block;
		uint32_t		GPE1Block;
		uint8_t			PM1EventLength;
		uint8_t			PM1ControlLength;
		uint8_t			PM2ControlLength;
		uint8_t			PMTimerLength;
		uint8_t			GPE0Length;
		uint8_t			GPE1Length;
		uint8_t			GPE1Base;
		uint8_t			CStateControl;
		uint16_t		WorstC2Latency;
		uint16_t		WorstC3Latency;
		uint16_t		FlushSize;
		uint16_t		FlushStride;
		uint8_t			DutyOffset;
		uint8_t			DutyWidth;
		uint8_t			DayAlarm;
		uint8_t			MonthAlarm;
		uint8_t			Century;
		uint16_t		BootArchitectureFlags;
		uint8_t			Reserved2;
		uint32_t		Flags;
		GenericAddress	ResetRegister;
		uint8_t			ResetValue;
		uint16_t		Reserved3;
		uint8_t			MinorVersion;						//	ACPI minor version, major located in Header.Revision
		uint64_t		ExtendedFirmwareControl;
		uint64_t		ExtendedDSDTAddress;
		GenericAddress	ExtendedPM1AEventBlock;
		GenericAddress	ExtendedPM1BEventBlock;
		GenericAddress	ExtendedPM1AControlBlock;
		GenericAddress	ExtendedPM1BControlBlock;
		GenericAddress	ExtendedPM2ControlBlock;
		GenericAddress	ExtendedPMTimerBlock;
		GenericAddress	ExtendedGPE0Block;
		GenericAddress	ExtendedGPE1Block;
	} FADT, *PFADT;

	static constexpr uint64_t RSDPSignature =	0x2052545020445352;
	static constexpr size_t RSDPSignatureSize =	sizeof(RSDPSignature);
	static constexpr size_t UndefinedVersion =	0xFF;

	static bool Initialize();
	static bool Initialized();
	static bool PowerOff();
	static bool IsRSDPValid(const PRSDP rsdp);
	static bool IsSDTValid(const void* sdt,uint32_t signature);
	static uint8_t GetMajorVersion();
	static uint8_t GetMinorVersion();
	static uint16_t GetBootArchitectureFlags();
	private:
	static bool SCIEnabled_;
	static uint8_t MajorVersion_;
	static uint32_t SMICommand_;
	static uint8_t Enable_;
	static uint8_t Disable_;
	static uint32_t PM1AControl_;
	static uint32_t PM1BControl_;
	static uint8_t PM1ControlLength_;
	static uint8_t MinorVersion_;
	static uint16_t BootArchitectureFlags_;
	static uint16_t SleepTypeA_;
	static uint16_t SleepTypeB_;
	static uint16_t SleepEnable_;
};

#endif