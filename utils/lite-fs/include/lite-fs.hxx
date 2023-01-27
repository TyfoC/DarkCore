#pragma once
#ifndef LITE_FS_HXX
#define LITE_FS_HXX

#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>
#include <filesystem>

class LiteFS {
	public:
	static constexpr uint64_t RootID =						0;
	static constexpr uint32_t CounterFieldValue =			0x55AAAA55;
	static constexpr size_t MaxEntryNameLength =			0x100;		//	with null-terminator
	static constexpr size_t MaxPartitionNameLength =		0x20;		//	with null-terminator
	static constexpr size_t MaxPartitionsCount =			0x3E;
	static constexpr size_t SignatureLength =				0x06;
	static constexpr char Signature[6] =					{ 'L', 'I', 'T', 'E', 'F', 'S' };

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

	//	-----------------------------------------------------------

	typedef struct FileSystemObject {
		uint64_t			ID;
		uint64_t			ParentID;
		std::string			Name;
		std::string			ParentPath;
		uint8_t*			FileData;
		size_t				FileSize;									//	files count in directory
		bool				Directory;
	} FileSystemObject, *PFileSystemObject;

	static void PrintFileSystemObjects(const std::vector<FileSystemObject>& fsObjects);
	static std::vector<FileSystemObject> GetFileSystemObjects(const std::string& dirPath, uint64_t startID = 1, uint64_t parentID = 0);
	static void FreeFileSystemObjects(std::vector<FileSystemObject>& fsObjects);

	static void PrintPartition(const uint8_t* data);
	static uint8_t* GeneratePartition(const std::string& name, size_t* resultSize, const std::vector<FileSystemObject>& fsObjects, uint64_t attributes = OBJECT_ATTRIBUTE_SYSTEM);
	static size_t GetPartitionObjectsCount(const uint8_t* data);

	static FSHeader GenerateEmptyFSHeader();
	static bool AddFSHeaderPartition(FSHeader& fsHeader, int64_t partOffsetLBA);
	private:
	static size_t AlignUp(size_t value, size_t alignment);
	static uint8_t* ReallocateArray(uint8_t* memory, size_t newSize);
};

#endif