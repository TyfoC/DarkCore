#pragma once
#ifndef PCI_HXX
#define PCI_HXX

#include "../common/inline-assembly.hxx"

class PCI {
	public:
	enum Ports {
		PORT_CONFIG_ADDRESS =							0xCF8,
		PORT_CONFIG_DATA =								0xCFC
	};

	enum HeaderTypes {
		HEADER_SINGLE_FUNCTIONAL =						0x00,
		HEADER_MULTI_FUNCTIONAL =						0x80
	};

	enum ClassCodes {
		CLASS_UNCLASSIFIED =							0x00,
		CLASS_MASS_STORAGE_CONTROLLER =					0x01,
		CLASS_NETWORK_CONTROLLER =						0x02,
		CLASS_DISPLAY_CONTROLLER =						0x03,
		CLASS_MULTIMEDIA_CONTROLLER =					0x04,
		CLASS_MEMORY_CONTROLLER =						0x05,
		CLASS_BRIDGE =									0x06,
		CLASS_SIMPLE_COMMUNICATION_CONTROLLER =			0x07,
		CLASS_BASE_SYSTEM_PERIPHERAL =					0x08,
		CLASS_INPUT_DEVICE_CONTROLLER =					0x09,
		CLASS_DOCKING_STATION =							0x0A,
		CLASS_PROCESSOR =								0x0B,
		CLASS_SERIAL_BUS_CONTROLLER =					0x0C,
		CLASS_WIRELESS_CONTROLLER =						0x0D,
		CLASS_INTELLIGENT_CONTROLLER =					0x0E,
		CLASS_SATELLITE_COMUNNICATION_CONTROLLER =		0x0F,
		CLASS_ENCRYPTION_CONTROLLER =					0x10,
		CLASS_SIGNAL_PROCESSING_CONTROLLER =			0x11,
		CLASS_PROCESSIN_ACCELERATOR =					0x12,
		CLASS_NON_ESSENTIAL_INSTRUMENTATION =			0x13,
		CLASS_COPROCESSOR =								0x40,
		CLASS_UNASSIGNED =								0xFF
	};

	enum SubClassCodes {
		SUB_CLASS_MASS_STORAGE_SCSI_BUS_CONTROLLER =	0x00,
		SUB_CLASS_MASS_STORAGE_IDE_CONTROLLER =			0x01,
		SUB_CLASS_MASS_STORAGE_FLOPPY_DISK_CONTROLLER =	0x02,
		SUB_CLASS_MASS_STORAGE_IPI_BUS_CONTROLLER =		0x03,
		SUB_CLASS_MASS_STORAGE_RAID_CONTROLLER =		0x04,
		SUB_CLASS_MASS_STORAGE_PATA_CONTROLLER =		0x05,
		SUB_CLASS_MASS_STORAGE_SATA_CONTROLLER =		0x06,
		SUB_CLASS_MASS_STORAGE_SASCSI_CONTROLLER =		0x07,
		SUB_CLASS_MASS_STORAGE_NVM_CONTROLLER =			0x08,
		SUB_CLASS_MASS_STORAGE_OTHER_CONTROLLER =		0x80,

		SUB_CLASS_BRIDGE_PCI_TO_PCI_1 =					0x04,
		SUB_CLASS_BRIDGE_PCI_TO_PCI_2 =					0x09
	};

	enum WordIndexes {
		WORD_INDEX_VENDOR_ID =							0x00,
		WORD_INDEX_DEVICE_ID =							0x01,
		WORD_INDEX_COMMAND =							0x02,
		WORD_INDEX_STATUS =								0x03,
		WORD_INDEX_REVESION_ID_PROG_IF =				0x04,
		WORD_INDEX_SUB_CLASS_CLASS =					0x05,
		WORD_INDEX_CACHE_LINE_SIZE_LATENCY_TIMER =		0x06,
		WORD_INDEX_HEADER_TYPE_BUILT_IN_SELF_TEST =		0x07
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Header {
		uint16_t		VendorID;
		uint16_t		DeviceID;
		uint16_t		Command;
		uint16_t		Status;
		uint8_t			RevisionID;
		uint8_t			ProgrammingInterface;
		uint8_t			SubClass;
		uint8_t			Class;
		uint8_t			CacheLineSize;
		uint8_t			LatencyTimer;
		uint8_t			HeaderType;
		uint8_t			BuiltInSelfTest;
	} Header, *PHeader;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Standard {
		Header			CommonHeader;
		uint32_t		BaseAddress0;
		uint32_t		BaseAddress1;
		uint32_t		BaseAddress2;
		uint32_t		BaseAddress3;
		uint32_t		BaseAddress4;
		uint32_t		BaseAddress5;
		uint32_t		CardBusCISPointer;
		uint16_t		SubSystemVendorID;
		uint16_t		SubSystemID;
		uint32_t		ExpnsionROMBaseAddress;
		uint32_t		Reserved1				:24;
		uint8_t			CapabilitiesPointer;
		uint32_t		Reserved2;
		uint8_t			InterruptLine;
		uint8_t			InterruptPIN;
		uint8_t			MinGrant;
		uint8_t			MaxLatency;
	} StandardDevice, *PStandardDevice;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) PCIToPCIBridge {
		Header			CommonHeader;
		uint32_t		BaseAddress0;
		uint32_t		BaseAddress1;
		uint8_t			PrimaryBusNumber;
		uint8_t			SecondaryBusNumber;
		uint8_t			SubordinateBusNumber;
		uint8_t			SecondaryLatencyTimer;
		uint8_t			InputOutputBaseLow;
		uint8_t			InputOutputLimitLow;
		uint16_t		SecondaryStatus;
		uint16_t		MemoryBase;
		uint16_t		MemoryLimit;
		uint16_t		PrefetchableMemoryBaseLow;
		uint16_t		PrefetchableMemoryLimitLow;
		uint32_t		PrefetchableMemoryBaseHigh;
		uint32_t		PrefetchableMemoryLimitHigh;
		uint16_t		InputOutputBaseHigh;
		uint16_t		InputOutputLimitHigh;
		uint8_t			CapabilityPointer;
		uint32_t		Reserved					:24;
		uint32_t		ExpansionROMBaseAddress;
		uint8_t			InterruptLine;
		uint8_t			InterruptPIN;
		uint16_t		BridgeControl;
	} PCIToPCIBridge, *PPCIToPCIBridge;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) PCIToCardBusBridge {
		Header			CommonHeader;
		uint32_t		CardBusSoketBaseAddress;
		uint8_t			CapabilitiesListOffset;
		uint8_t			Reserved;
		uint16_t		SecondaryStatus;
		uint8_t			PCIBusNumber;
		uint8_t			CardBusBusNumber;
		uint8_t			SubordinateBusNumber;
		uint8_t			CardBusLatencyTimer;
		uint32_t		MemoryBaseAddress0;
		uint32_t		MemoryLimit0;
		uint32_t		MemoryBaseAddress1;
		uint32_t		MemoryLimit1;
		uint32_t		InputOutputBaseAddress0;
		uint32_t		InputOutputLimit0;
		uint32_t		InputOutputBaseAddres1;
		uint32_t		InputOutputLimit1;
		uint8_t			InterruptLine;
		uint8_t			InterruptPIN;
		uint16_t		BridgeControl;
		uint16_t		SubSystemDeviceID;
		uint16_t		SubSystemVendorID;
		uint32_t		PCCardLegacyModeBaseAddress;
	} PCIToCardBusBridge, *PPCIToCardBusBridge;

	typedef void (*OnDeviceFound)(uint8_t bus, uint8_t device, uint8_t function, uint8_t baseClass, uint8_t subClass);

	static constexpr uint16_t InvalidVendorID =		0xFFFF;

	static uint32_t ReadLong(uint8_t bus, uint8_t device, uint8_t function, uint8_t longIndex);
	static uint16_t ReadWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t wordIndex);
	static uint8_t ReadByte(uint8_t bus, uint8_t device, uint8_t function, uint8_t byteIndex);
	static uint8_t GetHeaderType(uint8_t bus, uint8_t device, uint8_t function);
	static uint16_t GetVendorID(uint8_t bus, uint8_t device, uint8_t function);
	static uint8_t GetClass(uint8_t bus, uint8_t device, uint8_t function);
	static uint8_t GetSubClass(uint8_t bus, uint8_t device, uint8_t function);
	static void CheckFunction(OnDeviceFound handler, uint8_t bus, uint8_t device, uint8_t function);
	static void CheckDevice(OnDeviceFound handler, uint8_t bus, uint8_t device);
	static void CheckBus(OnDeviceFound handler, uint8_t bus);
	static void CheckAllBuses(OnDeviceFound handler);
};

#endif