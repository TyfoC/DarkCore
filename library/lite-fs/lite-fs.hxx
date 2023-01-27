#pragma once
#ifndef LITE_FS_HEADER
#define LITE_FS_HEADER

#include "../storage-device/storage-device.hxx"
#include "../common/string.hxx"
#include "../common/list.hxx"

//	Functions not implemented yet
class LiteFS {
	public:
	static constexpr uint64_t FSHeaderLBA =					1;
	static constexpr uint64_t RootID =						0;
	static constexpr uint32_t CounterFieldValue =			0x55AAAA55;
	static constexpr size_t MaxEntryNameLength =			0x100;		//	with null-terminator
	static constexpr size_t MaxPartitionNameLength =		0x20;		//	with null-terminator
	static constexpr size_t MaxPartitionsCount =			0x3E;
	static constexpr size_t SignatureLength =				0x06;
	static const char Signature[];
	static const char RunConfigPath[];

	enum ObjectTypes {
		OBJECT_TYPE_FILE =									0x00,
		OBJECT_TYPE_DIRECTORY =								0x01,
		OBJECT_TYPE_SYMBOLIC_LINK =							0x02
	};

	enum ObjectAttributes {
		OBJECT_ATTRIBUTE_SYSTEM =							0x01,
		OBJECT_ATTRIBUTE_READ_ONLY =						0x02,
		OBJECT_ATTRIBUTE_HIDDEN =							0x04
	};

	typedef struct __attribute__((packed)) DataChunk {
		uint8_t				Data[0x1F8];
		int64_t				NextOffset;									//	0 if this was the last data chunk
	} DataChunk, *PDataChunk;

	typedef struct __attribute__((packed)) ObjectDescriptor {
		char				Name[MaxEntryNameLength];
		uint64_t			ID;
		uint64_t			ParentID;
		uint32_t			Type;										//	OBJECT_TYPE_*
		uint64_t			Attributes;									//	OBJECT_ATTRIBUTE_*
		uint64_t			UnalignedSize;								//	unaligned size for file / files count for directory
		uint64_t			TakenSectorsCount;							//	data sectors count for file / 0 for directory
		uint32_t			CreationDate;
		uint32_t			LastReadDate;
		uint32_t			LastWriteDate;
		uint32_t			CreationTime;
		uint32_t			LastReadTime;
		uint32_t			LastWriteTime;
		int64_t				FirstDataChunkOffset;
		uint8_t				Reserved[0xB0];
		uint32_t			CounterField;
	} ObjectDescriptor, *PObjectDescriptor;

	typedef struct __attribute__((packed)) PartitionHeader {
		char				Name[MaxPartitionNameLength];
		uint64_t			EntriesCount;
		uint64_t			UsedSectorsCount;
		uint64_t			MaxSectorsCount;							//	0 if not initialized
		uint8_t				Reserved[0x1C8];
	} PartitionHeader, *PPartitionHeader;

	typedef struct __attribute__((packed)) FSHeader {
		char				Signature[SignatureLength];					//	`LITEFS`
		uint16_t			Revision;									//	0
		uint64_t			CurrentPartitionsCount;
		int64_t				Partitions[MaxPartitionsCount];				//	LBA offsets
	} FSHeader, *PFSHeader;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {					//	only for usermode
		char				Name[MaxEntryNameLength];
		uint8_t				Type;
		uint64_t			Attributes;
		uint64_t			Size;
		uint32_t			CreationDate;
		uint32_t			LastReadDate;
		uint32_t			LastWriteDate;
		uint32_t			CreationTime;
		uint32_t			LastReadTime;
		uint32_t			LastWriteTime;
	} ObjectInformation, *PObjectInformation;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
		size_t				StorageDeviceIndex;
		size_t				PartitionIndex;
		uint64_t			DirectoryID;
		uint64_t			SectorOffset;
		uint64_t			ObjectsLeft;
	} DirectoryIterator, *PDirectoryIterator;

	static bool GetFSHeader(StorageDevice& storageDevice, PFSHeader fsHeader);

	static size_t GetPartitionIndex(StorageDevice& storageDevice, const String& partitionName);
	static bool GetPartitionHeader(StorageDevice& storageDevice, size_t partitionIndex, PPartitionHeader partHeader);
	static bool GetPartitionHeader(StorageDevice& storageDevice, const String& partitionName, PPartitionHeader partHeader);
	
	static size_t GetObjectDescriptorIndex(StorageDevice& storageDevice, size_t partitionIndex, uint64_t id);
	static size_t GetObjectDescriptorIndex(StorageDevice& storageDevice, size_t partitionIndex, const String& objectName, uint64_t parentID);
	static bool GetObjectDescriptor(StorageDevice& storageDevice, const String& objectPath, PObjectDescriptor objectDescriptor);

	static bool CreateDirectoryIterator(StorageDevice& storageDevice, const String& directoryPath, PDirectoryIterator directoryIterator);
	static bool GetNextObjectInformation(StorageDevice& storageDevice, PDirectoryIterator directoryIterator, PObjectInformation objectInformation);

	static bool ReadFile(StorageDevice& storageDevice, const String& filePath, void* buffer, uint64_t offset = 0, uint64_t bytesCount = 0);
};

#endif