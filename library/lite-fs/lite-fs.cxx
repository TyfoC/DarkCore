#include "lite-fs.hxx"

//	#include "../terminal/terminal.hxx"

const char LiteFS::Signature[] = "LITEFS";
const char LiteFS::RunConfigPath[] = "/base/system/config/run-cfg.rpf";

bool LiteFS::GetFSHeader(StorageDevice& storageDevice, PFSHeader fsHeader) {
	MemoryUtils::Fill(fsHeader, 0, sizeof(FSHeader));
	if (FSHeaderLBA >= storageDevice.GetSectorsCount()) return false;
	else if (!storageDevice.ReadSectors(fsHeader, FSHeaderLBA, 1)) return false;
	if (!MemoryUtils::Compare(fsHeader->Signature, "LITEFS", SignatureLength)) return false;
	return true;
}

size_t LiteFS::GetPartitionIndex(StorageDevice& storageDevice, const String& partitionName) {
	FSHeader fsHeader = {};
	PartitionHeader partHeader = {};
	if (!GetFSHeader(storageDevice, &fsHeader)) return WRONG_INDEX;

	for (size_t i = 0; i < (size_t)fsHeader.CurrentPartitionsCount; i++) {
		if (!GetPartitionHeader(storageDevice, i, &partHeader)) break;
		if (partitionName == partHeader.Name) return i;
	}

	return WRONG_INDEX;
}

bool LiteFS::GetPartitionHeader(StorageDevice& storageDevice, size_t partitionIndex, PPartitionHeader partHeader) {
	FSHeader fsHeader = {};
	MemoryUtils::Fill(partHeader, 0, sizeof(PartitionHeader));
	size_t storageSectorsCount = storageDevice.GetSectorsCount();

	if (!GetFSHeader(storageDevice, &fsHeader) || fsHeader.CurrentPartitionsCount <= partitionIndex) return false;
	else if ((size_t)(FSHeaderLBA + fsHeader.Partitions[partitionIndex]) >= storageSectorsCount) return false;
	else if (!storageDevice.ReadSectors(partHeader, (size_t)(FSHeaderLBA + fsHeader.Partitions[partitionIndex]), 1)) return false;

	return true;
}

bool LiteFS::GetPartitionHeader(StorageDevice& storageDevice, const String& partitionName, PPartitionHeader partHeader) {
	FSHeader fsHeader = {};
	MemoryUtils::Fill(partHeader, 0, sizeof(PartitionHeader));
	if (!GetFSHeader(storageDevice, &fsHeader)) return WRONG_INDEX;

	for (size_t i = 0; i < (size_t)fsHeader.CurrentPartitionsCount; i++) {
		if (!GetPartitionHeader(storageDevice, i, partHeader)) break;
		if (partitionName == partHeader->Name) return true;
	}

	return false;
}

size_t LiteFS::GetObjectDescriptorIndex(StorageDevice& storageDevice, size_t partitionIndex, uint64_t id) {
	FSHeader fsHeader = {};
	PartitionHeader partHeader = {};
	ObjectDescriptor objectDescriptor = {};
	size_t storageSectorsCount = storageDevice.GetSectorsCount();

	if (!GetFSHeader(storageDevice, &fsHeader) || fsHeader.CurrentPartitionsCount <= partitionIndex) return WRONG_INDEX;

	size_t lba = (size_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex]);

	if ((uint64_t)(FSHeaderLBA + fsHeader.Partitions[partitionIndex]) >= storageSectorsCount) return WRONG_INDEX;
	else if (!storageDevice.ReadSectors(&partHeader, lba, 1)) return WRONG_INDEX;

	lba += 1;
	for (size_t i = 0; i < (size_t)partHeader.EntriesCount && (uint64_t)lba < storageSectorsCount; i++) {
		if (storageDevice.ReadSectors(&objectDescriptor, lba, 1) && objectDescriptor.ID == id) return i;
		++lba;
	}

	return WRONG_INDEX;
}

size_t LiteFS::GetObjectDescriptorIndex(StorageDevice& storageDevice, size_t partitionIndex, const String& objectName, uint64_t parentID) {
	FSHeader fsHeader = {};
	PartitionHeader partHeader = {};
	ObjectDescriptor objectDescriptor = {};
	size_t storageSectorsCount = storageDevice.GetSectorsCount();

	if (!GetFSHeader(storageDevice, &fsHeader) || fsHeader.CurrentPartitionsCount <= partitionIndex) return WRONG_INDEX;

	size_t lba = (size_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex]);

	if ((uint64_t)(FSHeaderLBA + fsHeader.Partitions[partitionIndex]) >= storageSectorsCount) return WRONG_INDEX;
	else if (!storageDevice.ReadSectors(&partHeader, lba, 1)) return WRONG_INDEX;

	lba += 1;
	for (size_t i = 0; i < partHeader.EntriesCount && (uint64_t)lba < storageSectorsCount; i++) {
		if (storageDevice.ReadSectors(&objectDescriptor, lba, 1) && objectDescriptor.ParentID == parentID) {
			if (objectName == objectDescriptor.Name) return i;
		}
		++lba;
	}

	return WRONG_INDEX;
}

bool LiteFS::GetObjectDescriptor(StorageDevice& storageDevice, const String& objectPath, PObjectDescriptor objectDescriptor) {
	FSHeader fsHeader = {};
	size_t partitionIndex = WRONG_INDEX, descriptorsOffset;
	PartitionHeader partitionHeader = {};
	if (!GetFSHeader(storageDevice, &fsHeader)) return false;

	List<String::Part> pathParts = objectPath.Split("/");
	size_t pathPartsCount = pathParts.GetCount();

	if (!pathPartsCount) {
		objectDescriptor->ID = objectDescriptor->ParentID = WRONG_INDEX;
		objectDescriptor->Attributes = OBJECT_ATTRIBUTE_SYSTEM;
		objectDescriptor->UnalignedSize = objectDescriptor->TakenSectorsCount = 0;
		objectDescriptor->Type = OBJECT_TYPE_DIRECTORY;
		objectDescriptor->CounterField = CounterFieldValue;

		for (size_t i = 0; i < fsHeader.CurrentPartitionsCount; i++) {
			if (!GetPartitionHeader(storageDevice, i, &partitionHeader)) continue;
			
			++objectDescriptor->UnalignedSize;
			objectDescriptor->TakenSectorsCount += partitionHeader.UsedSectorsCount;
		}

		return true;
	}

	for (size_t i = 0; i < fsHeader.CurrentPartitionsCount; i++) {
		if (!GetPartitionHeader(storageDevice, i, &partitionHeader)) break;
		else if (objectPath.GetSubString(pathParts[0].StartOffset, pathParts[0].Length) == partitionHeader.Name) {
			partitionIndex = i;
			break;
		}
	}

	if (partitionIndex == WRONG_INDEX) return false;

	if (pathPartsCount == 1) {
		MemoryUtils::Copy(objectDescriptor->Name, partitionHeader.Name, StringUtils::GetLength(partitionHeader.Name) + 1);
		objectDescriptor->ID = 0;
		objectDescriptor->ParentID = WRONG_INDEX;
		objectDescriptor->Type = OBJECT_TYPE_DIRECTORY;
		objectDescriptor->Attributes = OBJECT_ATTRIBUTE_SYSTEM;
		objectDescriptor->UnalignedSize = 0;
		objectDescriptor->TakenSectorsCount = partitionHeader.UsedSectorsCount;
		objectDescriptor->CreationDate = objectDescriptor->CreationTime = 0;
		objectDescriptor->LastReadDate = objectDescriptor->LastReadTime = 0;
		objectDescriptor->LastWriteDate = objectDescriptor->LastWriteTime = 0;
		objectDescriptor->CounterField = CounterFieldValue;

		ObjectDescriptor tmpObjectDescriptor = {};
		size_t objectDescriptorOffset = (size_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex] + 1);
		for (size_t i = 0; i < partitionHeader.EntriesCount; i++) {
			if (!storageDevice.ReadSectors(&tmpObjectDescriptor, objectDescriptorOffset + i, 1)) continue;
			if (!tmpObjectDescriptor.ParentID) ++objectDescriptor->UnalignedSize;
		}

		return true;
	}

	String pathPart;
	uint64_t parentID = 0;
	bool objectFound = false;
	descriptorsOffset = (size_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex]) + 1;
	for (size_t i = 1; i < pathPartsCount - 1; i++) {
		pathPart = objectPath.GetSubString(pathParts[i].StartOffset, pathParts[i].Length);

		for (size_t j = 0; j < partitionHeader.EntriesCount; j++) {
			if (!storageDevice.ReadSectors(objectDescriptor, descriptorsOffset + j, 1)) continue;
			if (objectDescriptor->ParentID == parentID && pathPart == objectDescriptor->Name) {
				objectFound = true;
				parentID = objectDescriptor->ID;
				break;
			}
		}

		if (!objectFound) return false;
	}

	pathPart = objectPath.GetSubString(pathParts[pathPartsCount - 1].StartOffset, pathParts[pathPartsCount - 1].Length);
	for (size_t i = 0; i < partitionHeader.EntriesCount; i++) {
		if (!storageDevice.ReadSectors(objectDescriptor, descriptorsOffset + i, 1)) continue;
		if (objectDescriptor->ParentID == parentID && pathPart == objectDescriptor->Name) return true;
	}

	return false;
}

bool LiteFS::CreateDirectoryIterator(StorageDevice& storageDevice, const String& directoryPath, PDirectoryIterator directoryIterator) {
	FSHeader fsHeader = {};
	size_t descriptorOffset;
	size_t partitionIndex = WRONG_INDEX;
	PartitionHeader partitionHeader = {};
	ObjectDescriptor objectDescriptor = {};
	if (!GetFSHeader(storageDevice, &fsHeader)) return false;

	List<String::Part> pathParts = directoryPath.Split("/");
	size_t pathPartsCount = pathParts.GetCount();

	if (!pathPartsCount) {
		directoryIterator->PartitionIndex = WRONG_INDEX;
		directoryIterator->DirectoryID = WRONG_INDEX;
		directoryIterator->SectorOffset = FSHeaderLBA;
		directoryIterator->ObjectsLeft = 0;					//	fsHeader.CurrentPartitionsCount;
		return true;
	}

	for (size_t i = 0; i < fsHeader.CurrentPartitionsCount; i++) {
		if (!GetPartitionHeader(storageDevice, i, &partitionHeader)) break;
		else if (directoryPath.GetSubString(pathParts[0].StartOffset, pathParts[0].Length) == partitionHeader.Name) {
			partitionIndex = i;
			break;
		}
	}

	if (partitionIndex == WRONG_INDEX) return false;
	
	if (pathPartsCount == 1) {

		directoryIterator->PartitionIndex = partitionIndex;
		directoryIterator->DirectoryID = 0;
		directoryIterator->SectorOffset = (uint64_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex]);
		directoryIterator->ObjectsLeft = 0;

		descriptorOffset = (size_t)directoryIterator->SectorOffset + 1;
		for (size_t i = 0; i < partitionHeader.EntriesCount; i++) {
			if (!storageDevice.ReadSectors(&objectDescriptor, descriptorOffset + i, 1)) continue;
			if (!objectDescriptor.ParentID) ++directoryIterator->ObjectsLeft;
		}

		return true;
	}

	String pathPart;
	uint64_t parentID = 0;
	bool objectFound = false;
	descriptorOffset = (size_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex]) + 1;
	for (size_t i = 1; i < pathPartsCount - 1; i++) {
		pathPart = directoryPath.GetSubString(pathParts[i].StartOffset, pathParts[i].Length);

		for (size_t j = 0; j < partitionHeader.EntriesCount; j++) {
			if (!storageDevice.ReadSectors(&objectDescriptor, descriptorOffset + j, 1)) continue;
			if (objectDescriptor.ParentID == parentID && pathPart == objectDescriptor.Name) {
				objectFound = true;
				parentID = objectDescriptor.ID;
				break;
			}
		}

		if (!objectFound) return false;
	}

	pathPart = directoryPath.GetSubString(pathParts[pathPartsCount - 1].StartOffset, pathParts[pathPartsCount - 1].Length);
	for (size_t i = 0; i < partitionHeader.EntriesCount; i++) {
		if (!storageDevice.ReadSectors(&objectDescriptor, descriptorOffset + i, 1)) continue;
		if (objectDescriptor.ParentID == parentID && objectDescriptor.Type == OBJECT_TYPE_DIRECTORY && pathPart == objectDescriptor.Name) {
			directoryIterator->PartitionIndex = partitionIndex;
			directoryIterator->DirectoryID = objectDescriptor.ID;
			directoryIterator->SectorOffset = descriptorOffset + i;
			directoryIterator->ObjectsLeft = objectDescriptor.UnalignedSize;
			return true;
		}
	}

	return false;
}

bool LiteFS::GetNextObjectInformation(StorageDevice& storageDevice, PDirectoryIterator directoryIterator, PObjectInformation objectInformation) {
	if (!directoryIterator->ObjectsLeft && directoryIterator->PartitionIndex != WRONG_INDEX) return false;

	FSHeader fsHeader = {};
	PartitionHeader partitionHeader = {};
	if (!GetFSHeader(storageDevice, &fsHeader)) return false;

	if (directoryIterator->PartitionIndex == WRONG_INDEX) {
		if (directoryIterator->DirectoryID != WRONG_INDEX || directoryIterator->ObjectsLeft >= fsHeader.CurrentPartitionsCount) return false;
		if (!GetPartitionHeader(storageDevice, (size_t)directoryIterator->ObjectsLeft, &partitionHeader)) return false;

		MemoryUtils::Copy(objectInformation->Name, partitionHeader.Name, StringUtils::GetLength(partitionHeader.Name) + 1);
		objectInformation->Type = OBJECT_TYPE_DIRECTORY;
		objectInformation->Attributes = OBJECT_ATTRIBUTE_SYSTEM;
		objectInformation->Size = fsHeader.CurrentPartitionsCount;
		objectInformation->CreationDate = objectInformation->CreationTime = 0;
		objectInformation->LastReadDate = objectInformation->LastReadTime = 0;
		objectInformation->LastWriteDate = objectInformation->LastWriteTime = 0;

		++directoryIterator->ObjectsLeft;
		return true;
	}

	if (!GetPartitionHeader(storageDevice, directoryIterator->PartitionIndex, &partitionHeader)) return false;

	ObjectDescriptor objectDescriptor = {};
	size_t descriptorOffset = (size_t)directoryIterator->SectorOffset + 1;
	for (size_t i = 0; i < partitionHeader.EntriesCount; i++) {
		if (!storageDevice.ReadSectors(&objectDescriptor, descriptorOffset + i, 1)) continue;
		if (objectDescriptor.ParentID == directoryIterator->DirectoryID) {
			MemoryUtils::Copy(objectInformation->Name, objectDescriptor.Name, StringUtils::GetLength(objectDescriptor.Name) + 1);
			objectInformation->Type = (uint8_t)objectDescriptor.Type;
			objectInformation->Attributes = objectDescriptor.Attributes;
			objectInformation->Size = objectDescriptor.UnalignedSize;
			objectInformation->CreationDate = objectDescriptor.CreationDate;
			objectInformation->LastReadDate = objectDescriptor.LastReadDate;
			objectInformation->LastWriteDate = objectDescriptor.LastWriteDate;
			objectInformation->CreationTime = objectDescriptor.CreationTime;
			objectInformation->LastReadTime = objectDescriptor.LastReadTime;
			objectInformation->LastWriteTime = objectDescriptor.LastWriteTime;

			directoryIterator->SectorOffset = descriptorOffset + i;
			--directoryIterator->ObjectsLeft;
			return true;
		}
	}

	return false;
}

bool LiteFS::ReadFile(StorageDevice& storageDevice, const String& path, void* buffer, uint64_t offset, uint64_t bytesCount) {
	FSHeader fsHeader = {};
	size_t partitionIndex = WRONG_INDEX, descriptorsOffset;
	PartitionHeader partitionHeader = {};
	if (!GetFSHeader(storageDevice, &fsHeader)) return false;

	List<String::Part> pathParts = path.Split("/");
	size_t pathPartsCount = pathParts.GetCount();

	if (pathPartsCount < 2) return false;

	for (size_t i = 0; i < fsHeader.CurrentPartitionsCount; i++) {
		if (!GetPartitionHeader(storageDevice, i, &partitionHeader)) break;
		else if (path.GetSubString(pathParts[0].StartOffset, pathParts[0].Length) == partitionHeader.Name) {
			partitionIndex = i;
			break;
		}
	}

	if (partitionIndex == WRONG_INDEX) return false;

	String pathPart;
	uint64_t parentID = 0;
	bool objectFound = false;
	ObjectDescriptor objectDescriptor = {};
	descriptorsOffset = (size_t)((int64_t)FSHeaderLBA + fsHeader.Partitions[partitionIndex]) + 1;
	for (size_t i = 1; i < pathPartsCount - 1; i++) {
		pathPart = path.GetSubString(pathParts[i].StartOffset, pathParts[i].Length);

		for (size_t j = 0; j < partitionHeader.EntriesCount; j++) {
			if (!storageDevice.ReadSectors(&objectDescriptor, descriptorsOffset + j, 1)) continue;
			if (objectDescriptor.ParentID == parentID && pathPart == objectDescriptor.Name) {
				objectFound = true;
				parentID = objectDescriptor.ID;
				break;
			}
		}

		if (!objectFound) return false;
	}

	objectFound = false;
	pathPart = path.GetSubString(pathParts[pathPartsCount - 1].StartOffset, pathParts[pathPartsCount - 1].Length);
	for (size_t i = 0; i < partitionHeader.EntriesCount; i++) {
		if (!storageDevice.ReadSectors(&objectDescriptor, descriptorsOffset + i, 1)) continue;
		if (objectDescriptor.ParentID == parentID && pathPart == objectDescriptor.Name) {
			descriptorsOffset += i;
			objectFound = true;
			break;
		}
	}

	if (!objectFound) return false;

	if (!bytesCount) bytesCount = objectDescriptor.UnalignedSize;
	if (offset + bytesCount > objectDescriptor.UnalignedSize) bytesCount = objectDescriptor.UnalignedSize - offset;

	size_t startDataChunkIndex = (size_t)offset / sizeof(DataChunk::Data);
	if (startDataChunkIndex >= objectDescriptor.TakenSectorsCount) return false;
	offset -= startDataChunkIndex * sizeof(DataChunk::Data);

	DataChunk dataChunk;
	int64_t dataChunkLBA = descriptorsOffset + objectDescriptor.FirstDataChunkOffset;
	for (size_t i = 0; i < startDataChunkIndex; i++) {
		if (!storageDevice.ReadSectors(&dataChunk, (size_t)dataChunkLBA, 1)) return false;
		else dataChunkLBA += dataChunk.NextOffset;
	}

	puint8_t buffer8 = (puint8_t)buffer;
	size_t lenValue, bufferOffset = 0;
	while (bytesCount) {
		if (!storageDevice.ReadSectors(&dataChunk, (size_t)dataChunkLBA, 1)) return false;
		lenValue = sizeof(DataChunk::Data) - (size_t)offset;

		MemoryUtils::Copy(&buffer8[bufferOffset], dataChunk.Data, lenValue);
		bufferOffset += lenValue;

		if (bytesCount > lenValue) bytesCount -= lenValue;
		else bytesCount = 0;
		dataChunkLBA += dataChunk.NextOffset;
		offset = 0;
	};

	return true;
}