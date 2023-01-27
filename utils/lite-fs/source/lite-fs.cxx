#include "../include/lite-fs.hxx"

void LiteFS::PrintFileSystemObjects(const std::vector<FileSystemObject>& fsObjects) {
	for (const FileSystemObject& entry : fsObjects) {
		std::cout << "ID: " << entry.ID << std::endl;
		std::cout << "ParentID: " << entry.ParentID << std::endl;
		std::cout << "Name: " << entry.Name << std::endl;
		std::cout << "Parent: " << entry.ParentPath << std::endl;
		std::cout << "Is directory: " << entry.Directory << std::endl;
		if (!entry.Directory) {
			std::cout << "Size: " << entry.FileSize << std::endl;
		}
	}
}

std::vector<LiteFS::FileSystemObject> LiteFS::GetFileSystemObjects(const std::string& dirPath, uint64_t startID, uint64_t parentID) {
	std::vector<FileSystemObject> fsObjects;
	FileSystemObject fsObject = {};

	std::filesystem::path pathObj;
	std::string targetDirPath = dirPath;
	if (targetDirPath.back() != '/') targetDirPath += '/';

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(targetDirPath)) {
		pathObj = entry.path();

		fsObject.ID = startID;
		fsObject.ParentID = parentID;
		fsObject.Name = pathObj.filename();
		fsObject.ParentPath = pathObj.parent_path().generic_string();
		fsObjects.push_back(fsObject);

		if (entry.is_directory()) {
			fsObjects.back().Directory = true;
			std::vector<FileSystemObject> tmpFsObjects = GetFileSystemObjects(pathObj, startID + 1, fsObject.ID);
			for (const FileSystemObject& subEntry : tmpFsObjects) if (subEntry.ParentID == fsObjects.back().ID) ++fsObjects.back().FileSize;
			startID += fsObjects.back().FileSize + 1;
			fsObjects.insert(fsObjects.end(), tmpFsObjects.begin(), tmpFsObjects.end());
			tmpFsObjects.clear();
		}
		else {
			fsObject.Directory = false;

			std::ifstream input(pathObj.generic_string().c_str(), std::ios_base::binary);
			if (!input.is_open()) std::cout << "Warning: cannot read `" << pathObj.generic_string() << '`' << std::endl;
			else {
				std::stringstream ss;
				ss << input.rdbuf();
				fsObjects.back().FileSize = ss.str().length();
				fsObjects.back().FileData = new uint8_t[fsObjects.back().FileSize];
				memcpy(fsObjects.back().FileData, ss.str().c_str(), fsObjects.back().FileSize);
				input.close();
			}

			++startID;
		}

		fsObject.FileData = 0;
		fsObject.FileSize = 0;
		fsObject.Directory = false;
	}

	return fsObjects;
}

void LiteFS::FreeFileSystemObjects(std::vector<FileSystemObject>& fsObjects) {
	const size_t objsCount = fsObjects.size();
	for (size_t i = 0; i < objsCount; i++) {
		if (!fsObjects[i].Directory && fsObjects[i].FileSize && fsObjects[i].FileData) delete[] fsObjects[i].FileData;
	}
	fsObjects.clear();
}

void LiteFS::PrintPartition(const uint8_t* data) {
	PPartitionHeader header = (PPartitionHeader)data;
	std::cout << "Partition name: " << &header->Name[0] << std::endl;
	std::cout << "Partition entries count: " << header->EntriesCount << std::endl;
	std::cout << "Partition used sectors count: " << header->UsedSectorsCount << std::endl;
	std::cout << "Partition max sectors count: " << header->MaxSectorsCount << std::endl << std::endl;

	PObjectDescriptor descriptor = (PObjectDescriptor)&data[sizeof(PartitionHeader)];
	for (size_t i = 0; i < header->EntriesCount; i++) {
		if (descriptor->CounterField == CounterFieldValue) {
			std::cout << "Name: " << &descriptor->Name[0] << std::endl;
			std::cout << "ID: " << descriptor->ID << std::endl;
			std::cout << "ParentID: " << descriptor->ParentID << std::endl;

			std::cout << "Attributes: ";
			if (descriptor->Attributes & OBJECT_ATTRIBUTE_SYSTEM) std::cout << "SYSTEM ";
			if (descriptor->Attributes & OBJECT_ATTRIBUTE_READ_ONLY) std::cout << "READ_ONLY ";
			if (descriptor->Attributes & OBJECT_ATTRIBUTE_HIDDEN) std::cout << "HIDDEN ";

			std::cout << std::endl << "Type: ";
			if (descriptor->Type == OBJECT_TYPE_FILE) {
				std::cout << "File" << std::endl;
				std::cout << "Size: " << descriptor->UnalignedSize << std::endl;
				std::cout << "Taken sectors count: " << descriptor->TakenSectorsCount << std::endl;
			}
			else if (descriptor->Type == OBJECT_TYPE_DIRECTORY) {
				std::cout << "Directory" << std::endl;
				std::cout << "Objects count: " << descriptor->UnalignedSize << std::endl;
			}
			else if (descriptor->Type == OBJECT_TYPE_SYMBOLIC_LINK) {
				std::cout << "Symbolic Link" << std::endl;
			}
			else std::cout << "Unknown" << std::endl;
		}
		else break;

		std::cout << std::endl;
		++descriptor;
	}
}

uint8_t* LiteFS::GeneratePartition(const std::string& name, size_t* resultSize, const std::vector<FileSystemObject>& fsObjects, uint64_t attributes) {
	*resultSize = sizeof(PartitionHeader);
	uint8_t* data = ReallocateArray(0, *resultSize);
	const size_t descriptorsCount = fsObjects.size();

	size_t partNameLen = name.length();
	PPartitionHeader partHeader = (PPartitionHeader)data;
	memset(partHeader, 0, sizeof(PartitionHeader));
	if (partNameLen >= MaxPartitionNameLength) {
		std::cout << "Warning: partition name will be shortened (max length: " << (MaxPartitionNameLength - 1) << "):" << std::endl;
		std::cout << '`' << name << '`' << std::endl;
		memcpy(&partHeader->Name[0], name.c_str(), MaxPartitionNameLength);
		partHeader->Name[MaxPartitionNameLength] = 0;
	}
	else {
		memset(&partHeader->Name[0], 0, MaxPartitionNameLength);
		memcpy(&partHeader->Name[0], name.c_str(), partNameLen);
		partHeader->Name[partNameLen] = 0;
	}

	partHeader->EntriesCount = descriptorsCount;
	partHeader->MaxSectorsCount = partHeader->UsedSectorsCount = 0;

	const size_t descriptorsAlignedSize = AlignUp(descriptorsCount * sizeof(ObjectDescriptor), 0x200);
	data = ReallocateArray(data, sizeof(PartitionHeader) + descriptorsAlignedSize);

	size_t entryNameLen, dataChunkOffset = sizeof(PartitionHeader) + descriptorsAlignedSize, descriptorOffset = sizeof(PartitionHeader);
	PObjectDescriptor descriptor = (PObjectDescriptor)&data[descriptorOffset];
	for (const FileSystemObject& entry : fsObjects) {
		entryNameLen = entry.Name.length();

		if (entryNameLen >= MaxEntryNameLength) {
			std::cout << "Warning: name will be shortened (max length: " << (MaxEntryNameLength - 1) << "):" << std::endl;
			std::cout << '`' << entry.Name << '`' << std::endl;

			memcpy(&descriptor->Name[0], entry.Name.c_str(), MaxEntryNameLength - 1);
			descriptor->Name[MaxEntryNameLength - 1] = 0;
		}
		else {
			memset(&descriptor->Name[0], 0, MaxEntryNameLength);
			memcpy(&descriptor->Name[0], entry.Name.c_str(), entryNameLen);
			descriptor->Name[entryNameLen] = 0;
		}

		descriptor->ID = entry.ID;
		descriptor->ParentID = entry.ParentID;
		descriptor->Type = entry.Directory ? OBJECT_TYPE_DIRECTORY : OBJECT_TYPE_FILE;
		descriptor->Attributes = attributes;
		descriptor->UnalignedSize = entry.FileSize;
		descriptor->CounterField = CounterFieldValue;

		descriptor->CreationDate = 0;
		descriptor->CreationTime = 0;
		descriptor->LastReadDate = 0;
		descriptor->LastReadTime = 0;
		descriptor->LastWriteDate = 0;
		descriptor->LastWriteTime = 0;

		memset(descriptor->Reserved, 0, sizeof(descriptor->Reserved));

		if (entry.Directory) {
			descriptor->TakenSectorsCount = 0;
			descriptor->FirstDataChunkOffset = 0;
		}
		else {
			descriptor->TakenSectorsCount = AlignUp(descriptor->UnalignedSize + sizeof(DataChunk::NextOffset), sizeof(DataChunk::Data)) / sizeof(DataChunk::Data);
			descriptor->FirstDataChunkOffset = (descriptorsAlignedSize - ((size_t)descriptor - (size_t)data)) / 0x200 + (dataChunkOffset - descriptorsAlignedSize) / 0x200;

			data = ReallocateArray(data, dataChunkOffset + descriptor->TakenSectorsCount * sizeof(DataChunk));
			descriptor = (PObjectDescriptor)&data[descriptorOffset];

			PDataChunk dataChunk = (PDataChunk)&data[dataChunkOffset];
			size_t unalignedFileSize = descriptor->UnalignedSize, fileDataPointer = 0;
			for (size_t i = 0; i < descriptor->TakenSectorsCount; i++) {
				if (unalignedFileSize >= sizeof(DataChunk::Data)) {
					memcpy(&dataChunk->Data[0], &entry.FileData[fileDataPointer], sizeof(DataChunk::Data));
					dataChunk->NextOffset = 1;
					fileDataPointer += sizeof(DataChunk::Data);
					unalignedFileSize -= sizeof(DataChunk::Data);
				}
				else {
					memset(&dataChunk->Data[0], 0, sizeof(DataChunk::Data));
					memcpy(&dataChunk->Data[0], &entry.FileData[fileDataPointer], unalignedFileSize);
					unalignedFileSize = fileDataPointer = dataChunk->NextOffset = 0;
				}

				dataChunkOffset += sizeof(DataChunk);
				++dataChunk;
			}
		}
		
		descriptorOffset += sizeof(ObjectDescriptor);
		descriptor = (PObjectDescriptor)&data[descriptorOffset];
	}

	if (dataChunkOffset >= 0x200) partHeader->UsedSectorsCount = (dataChunkOffset - 0x200) / 0x200;
	*resultSize = dataChunkOffset;
	return data;
}

size_t LiteFS::GetPartitionObjectsCount(const uint8_t* data) {
	PPartitionHeader header = (PPartitionHeader)data;
	return header->EntriesCount;
}

LiteFS::FSHeader LiteFS::GenerateEmptyFSHeader() {
	FSHeader header = {
		{ Signature[0], Signature[1], Signature[2], Signature[3], Signature[4], Signature[5] },
		0, 0, {}
	};
	return header;
}

bool LiteFS::AddFSHeaderPartition(FSHeader& fsHeader, int64_t partOffsetLBA) {
	if (fsHeader.CurrentPartitionsCount == MaxPartitionsCount) return false;

	fsHeader.Partitions[fsHeader.CurrentPartitionsCount] = partOffsetLBA;
	++fsHeader.CurrentPartitionsCount;

	return true;
}

size_t LiteFS::AlignUp(size_t value, size_t alignment) {
	return value - (value % alignment) + alignment;
}

uint8_t* LiteFS::ReallocateArray(uint8_t* memory, size_t newSize) {
	uint8_t* newMemory = new uint8_t[newSize];
	memset(newMemory, 0, newSize);

	if (memory) {
		memcpy(newMemory, memory, newSize);
		delete[] memory;
	}

	return newMemory;
}