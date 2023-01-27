#pragma once
#ifndef IDE_CONTROLLER_HXX
#define IDE_CONTROLLER_HXX

#include "../common/memory-utils.hxx"
#include "../pci/pci.hxx"
#include "../virtual-drivers/virtual-timer.hxx"

class IDEControllerChannel {
	public:
	enum Types {
		TYPE_PRIMARY =									0x00,
		TYPE_SECONDARY =								0x01
	};

	enum StandardPorts {
		STANDARD_PORT_PRIMARY_BASE =					0x1F0,
		STANDARD_PORT_SECONDARY_BASE =					0x170,
		STANDARD_PORT_PRIMARY_CONTROL =					0x1F6,
		STANDARD_PORT_SECONDARY_CONTROL =				0x176,
	};

	enum Registers {
		REGISTER_DATA =									0x00,
		REGISTER_SECTORS_COUNT_0 =						0x02,
		REGISTER_LBA_0 =								0x03,
		REGISTER_LBA_1 =								0x04,
		REGISTER_LBA_2 =								0x05,
		REGISTER_DEVICE =								0x06,
		REGISTER_COMMAND =								0x07,
		REGISTER_STATUS =								0x07,
		REGISTER_SECTORS_COUNT_1 =						0x08,
		REGISTER_LBA_3 =								0x09,
		REGISTER_LBA_4 = 								0x0A,
		REGISTER_LBA_5 =								0x0B,
		REGISTER_CONTROL =								0x0C,
		REGISTER_ALTERNATIVE_STATUS =					0x0C
	};

	enum RegisterDeviceBits {
		REGISTER_DEVICE_BIT_SLAVE =						0x10,	
	};

	enum RegisterControlBits {
		REGISTER_CONTROL_BIT_IRQ_DISABLED =				0x02,	
	};

	enum Statuses {
		STATUS_NO_DRIVE =								0x00,
		STATUS_ERROR =									0x01,
		STATUS_INDEX =									0x02,
		STATUS_CORRECT_DATA =							0x04,
		STATUS_DATA_REQUEST_READY =						0x08,
		STATUS_DRIVE_SEEK_DONE =						0x10,
		STATUS_DRIVE_WRITE_FAULT =						0x20,
		STATUS_DRIVE_READY =							0x40,
		STATUS_BUSY =									0x80
	};

	enum PollingStatuses {
		POLLING_STATUS_SUCCESS =						0x00,
		POLLING_STATUS_DRIVE_WRITE_FAULT =				0x01,
		POLLING_STATUS_ERROR =							0x02,
		POLLING_STATUS_DATA_REQUEST_NOT_READY =			0x03
	};

	class Device {
		public:
		enum Types {
			TYPE_MASTER =								0x00,
			TYPE_SLAVE =								0x01
		};

		enum InterfaceTypes {
			INTERFACE_TYPE_ATA =						0x00,
			INTERFACE_TYPE_ATAPI =						0x01
		};

		enum Commands {
			COMMAND_READ_PIO =							0x20,
			COMMAND_READ_PIO_EXTENDED =					0x24,
			COMMAND_WRITE_PIO =							0x30,
			COMMAND_WRITE_PIO_EXTENDED =				0x34,
			COMMAND_IDENTIFY_PACKET =					0xA1,
			COMMAND_CACHE_FLUSH =						0xE7,
			COMMAND_CACHE_FLUSH_EXTENDED =				0xEA,
			COMMAND_IDENTIFY =							0xEC
		};

		enum IdentifyBufferOffsets {
			IDENTIFY_BUFFER_OFFSET_DEVICE_TYPE =			0x00,
			IDENTIFY_BUFFER_OFFSET_CYLINDERS =				0x02,
			IDENTIFY_BUFFER_OFFSET_HEADS =					0x06,
			IDENTIFY_BUFFER_OFFSET_SECTORS =				0x0C,
			IDENTIFY_BUFFER_OFFSET_SERIAL =					0x14,
			IDENTIFY_BUFFER_OFFSET_MODEL_NAME =				0x36,
			IDENTIFY_BUFFER_OFFSET_CAPABILITIES =			0x62,
			IDENTIFY_BUFFER_OFFSET_FIELD_VALID =			0x6A,
			IDENTIFY_BUFFER_OFFSET_MAX_LBA =				0x78,
			IDENTIFY_BUFFER_OFFSET_COMMAND_SETS =			0xA4,
			IDENTIFY_BUFFER_OFFSET_MAX_LBA_EXTENDED =		0xC8
		};

		enum TranferModes {
			TRANSFER_MODE_CHS =								0x00,
			TRANSFER_MODE_LBA28 =							0x01,
			TRANSFER_MODE_LBA48 =							0x02
		};

		enum TransferStatuses {
			TRANSFER_STATUS_SUCCESS =						0x00,
			TRANSFER_STATUS_DEVICE_NOT_EXIST =				0x01,
			TRANSFER_STATUS_LBA48_UNSUPPORTED =				0x02,
			TRANSFER_STATUS_UNKNOWN_MODE =					0x03,
		};

		static constexpr size_t DevicesPerChannel =			0x02;
		static constexpr size_t MaxModelNameLength =		41;
		static constexpr size_t MaxSectorsPerOperation =	0xFF;
		static constexpr size_t ShiftPerMaxSectors =		0x1FE00;		//	0x200 * MaxSectorsPerOperation
		
		Device();
		Device(uint8_t type, const IDEControllerChannel* channel);
		bool Exist();

		uint8_t NativeReadSectors(void* destination, uint32_t lba, uint8_t sectorsCount);
		uint8_t NativeWriteSectors(const void* source, uint32_t lba, uint8_t sectorsCount);
		uint8_t ReadSectors(void* destination, uint32_t lba, size_t sectorsCount);
		uint8_t WriteSectors(const void* source, uint32_t lba, size_t sectorsCount);

		uint8_t GetType();
		uint8_t GetInterfaceType();
		uint16_t GetSignature();
		uint16_t GetCapatibilities();
		uint32_t GetCommandSets();
		uint32_t GetSectorsCount();
		void GetModelName(char* buffer);

		IDEControllerChannel* GetChannel();
		void SetChannel(IDEControllerChannel* channel);
		private:
		uint8_t Type_;
		IDEControllerChannel* Channel_;
		uint8_t InterfaceType_;
		uint16_t Signature_;
		uint16_t Capatibilities_;
		uint32_t CommandSets_;
		uint32_t SectorsCount_;
		char ModelName_[MaxModelNameLength];
		bool Exist_;
	};

	static constexpr size_t ChannelsPerController =		0x02;
	static constexpr size_t IdentifyBufferSize =		0x800;

	IDEControllerChannel(uint16_t basePort, uint16_t controlPort, uint16_t busMasterIDEPort);
	uint8_t Read(uint8_t reg);
	void Write(uint8_t reg, uint8_t data);
	void ReadBuffer(uint8_t reg, void* buffer, size_t longsCount);
	uint8_t Polling(bool fullCheck);

	size_t GetDevicesCount();
	Device& GetDevice(size_t deviceType);
	private:
	uint16_t BasePort_;
	uint16_t ControlPort_;
	uint16_t BusMasterIDEPort_;
	Device Devices_[Device::DevicesPerChannel];
	size_t DevicesCount_;
	bool InterruptDisabled_;
	uint8_t IdentifyBuffer_[IdentifyBufferSize];
};

#endif