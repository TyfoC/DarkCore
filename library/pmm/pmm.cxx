#include "pmm.hxx"

PMM::PRegionDescriptor PMM::Descriptors_;
size_t PMM::DescriptorsCount_ = 0;

/**
 * @param descriptors [in, out] region descriptors
 * @param count [in] region descriptors count
*/
void PMM::Initialize(PRegionDescriptor descriptors, size_t count) {
	Descriptors_ = descriptors;

	RegionDescriptor extraDescriptor = {
		AlignUp((size_t)descriptors, FrameSize),
		AlignUp((size_t)descriptors + (count + 2) * sizeof(RegionDescriptor), FrameSize) - (size_t)descriptors,
		REGION_RESERVED
	};

	RemoveExtraRegions(descriptors, &extraDescriptor, &count, 1);

	for (size_t i = 0; i < count; i++) {
		descriptors[i].Address = AlignUp((size_t)descriptors[i].Address, FrameSize);
		descriptors[i].Length = AlignDown((size_t)descriptors[i].Length, FrameSize);
	}

	for (size_t i = 0; i < count; i++) {
		if (!GetRegionFramesCount(descriptors[i])) {
			MemoryUtils::Copy(&descriptors[i], &descriptors[i + 1], (count - i - 1) * sizeof(RegionDescriptor));
			count -= 1;
			continue;
		}
	}

	DescriptorsCount_ = count;

	PFrameDescriptor frameDescriptor;
	size_t framesCount, frameAddress, j;
	for (size_t i = 0; i < count; i++) {
		if (Descriptors_[i].Type == REGION_USABLE || Descriptors_[i].Type == REGION_RECLAIMABLE) {
			frameDescriptor = (PFrameDescriptor)(size_t)Descriptors_[i].Address;
			framesCount = GetRegionFramesCount(Descriptors_[i]);
			frameAddress = (size_t)frameDescriptor + AlignUp(framesCount * sizeof(FrameDescriptor), FrameSize);
			for (j = 0; j < framesCount; j++) {
				frameDescriptor->BufferAddress = frameAddress;
				frameDescriptor->BusyCount = 0;
				frameDescriptor->Flags = 0;
				++frameDescriptor;
				frameAddress += FrameSize;
			}
		}
	}
}

/**
 * @param count [in] bytes count
 * @return allocated buffer pointer
*/
void* PMM::AllocatePhysicalMemory(size_t count) {
	PFrameDescriptor frameDescriptors;
	size_t framesCount, j, k;
	bool framesFree;
	count = AlignUp(count, FrameSize) / FrameSize;

	for (size_t i = 0; i < DescriptorsCount_; i++) {
		if (Descriptors_[i].Type == REGION_USABLE || Descriptors_[i].Type == REGION_RECLAIMABLE) {
			frameDescriptors = (PFrameDescriptor)(size_t)Descriptors_[i].Address;
			framesCount = GetRegionFramesCount(Descriptors_[i]);

			if (count <= framesCount) {
				for (j = 0; j + count <= framesCount; j++) {
					framesFree = true;

					for (k = j; k < j + count; k++) {
						if (frameDescriptors[k].Flags & FRAME_BUSY) {
							framesFree = false;
							break;
						}
					}

					if (framesFree) {
						frameDescriptors[j].BusyCount = count;
						for (k = j; k < j + count; k++)frameDescriptors[k].Flags |= FRAME_BUSY;
						return (void*)frameDescriptors[j].BufferAddress;
					}
				}
			}
		}
	}

	return 0;
}

/**
 * @param memory [in] allocated buffer
 * @param count [in] region descriptors count
 * @return reallocated buffer pointer
*/
void* PMM::ReallocatePhysicalMemory(void* memory, size_t count) {
	puint8_t reallocated = (puint8_t)AllocatePhysicalMemory(count);
	if (!reallocated) return 0;

	MemoryUtils::Copy(reallocated, memory, count);

	if (!FreePhysicalMemory(memory)) {
		FreePhysicalMemory(reallocated);
		return 0;
	}

	return reallocated;
}

/**
 * @param memory [in] allocated buffer
 * @return count of freed pages
*/
size_t PMM::FreePhysicalMemory(void* memory) {
	PFrameDescriptor frameDescriptors;
	size_t framesCount, j, k;

	for (size_t i = 0; i < DescriptorsCount_; i++) {
		if (Descriptors_[i].Type == REGION_USABLE || Descriptors_[i].Type == REGION_RECLAIMABLE) {
			frameDescriptors = (PFrameDescriptor)(size_t)Descriptors_[i].Address;
			framesCount = GetRegionFramesCount(Descriptors_[i]);

			for (j = 0; j < framesCount; j++) {
				if (frameDescriptors[j].BufferAddress == (size_t)memory) {
					framesCount = frameDescriptors[j].BusyCount;
					frameDescriptors[j].BusyCount = 0;

					for (k = j; k < j + framesCount; k++) frameDescriptors[k].Flags &= ~(size_t)FRAME_BUSY;

					return framesCount;
				}
			}
		}
	}

	return 0;
}

/**
 * @param destination [out] tweaked region descriptors table
 * @param source [in] raw region descriptors table
 * @param count [in, out] raw regions count & tweaked region descriptors table entries count
*/
void PMM::TweakRegionDescriptors(PRegionDescriptor destination, const PRegionDescriptor source, psize_t countRegions) {
	size_t rawCount = *countRegions;
	*countRegions = 0;

	for (size_t i = 0; i < rawCount; i++) {
		if (source[i].Address < 0x100000000 && source[i].Address + source[i].Length <= 0x100000000) {
			destination[*countRegions] = source[i];
			*countRegions += 1;
		}
	}
}

/**
 * @param descriptors [in, out] region descriptors
 * @param count [in] region descriptors count
*/
void PMM::SortRegionDescriptors(PRegionDescriptor descriptors, size_t count) {
	if (!count) return;

	RegionDescriptor tmp;
	for (size_t i = 0; i < count - 1; i++) {
		for (size_t j = 0; j < count - i - 1; j++) {
			if (descriptors[j].Address > descriptors[j + 1].Address) {
				tmp = descriptors[j];
				descriptors[j] = descriptors[j + 1];
				descriptors[j + 1] = tmp;
			}
		}
	}
}

/**
 * @param descriptors [in, out] region descriptors
 * @param count [in, out] region descriptors count & new region descriptors count
*/
void PMM::RemoveSimilarRegionDescriptors(PRegionDescriptor descriptors, psize_t count) {
	if (!*count) return;

	for (size_t i = 0; i < *count - 1; i++) {
		if (descriptors[i].Address == descriptors[i + 1].Address) {
			if (descriptors[i].Type == descriptors[i + 1].Type && descriptors[i].Length < descriptors[i + 1].Length)
				descriptors[i].Length = descriptors[i + 1].Length;
			else if (descriptors[i].Type == REGION_USABLE || descriptors[i].Type == REGION_RECLAIMABLE)
				descriptors[i] = descriptors[i + 1];
			MemoryUtils::Copy(&descriptors[i + 1], &descriptors[i + 2], (*count - i - 2) * sizeof(RegionDescriptor));
			*count -= 1;
		}
	}
}

/**
 * @param descriptors [in, out] region descriptors
 * @param extra [in] extra region descriptors
 * @param countRegions [in, out] region descriptors count & new region descriptors count
 * @param countExtra [in] extra region descriptors count
*/
void PMM::RemoveExtraRegions(PRegionDescriptor descriptors, PRegionDescriptor extra, psize_t countRegions, size_t countExtra) {
	uint64_t myStart, myLen, myEnd, entryStart, entryLen, entryEnd;
	uint8_t overlapType;

	for (size_t i = 0; i < *countRegions; i++) {
		if (descriptors[i].Type == REGION_USABLE || descriptors[i].Type == REGION_RECLAIMABLE) {
			for (size_t j = 0; j < countExtra; j++) {
				if (extra[j].Length) {
					overlapType = GetRegionOverlapType(descriptors[i], extra[j]);

					myStart = descriptors[i].Address;
					myLen = descriptors[i].Length;
					myEnd = myStart + myLen;

					entryStart = extra[j].Address;
					entryLen = extra[j].Length;
					entryEnd = entryStart + entryLen;

					if (overlapType == OVERLAP_START) {
						descriptors[i].Length -= entryEnd - myStart;
						descriptors[i].Address = entryEnd;
					}
					else if (overlapType == OVERLAP_END) {
						descriptors[i].Length = entryStart - myStart;
					}
					else if (overlapType == OVERLAP_INSIDE) {
						for (size_t k = *countRegions; k > i; k--) descriptors[k] = descriptors[k - 1];
						descriptors[i + 1].Address = entryEnd;
						descriptors[i + 1].Length = myEnd - entryEnd;
						descriptors[i + 1].Type = descriptors[i].Type;
						descriptors[i].Length = entryStart - myStart;

						*countRegions = *countRegions + 1;
					}
					else if (overlapType == OVERLAP_OUTSIDE) {
						descriptors[i].Type = REGION_RESERVED;
					}
				}
			}
		}
	}
}

/**
 * @param region [in] target region descriptor
 * @param overlappingRegion [in] a region that overlaps another region
 * @return *
*/
uint8_t PMM::GetRegionOverlapType(RegionDescriptor region, RegionDescriptor overlappingRegion) {
	uint64_t overlapEnd = (uint64_t)(overlappingRegion.Address + overlappingRegion.Length);
	uint64_t regionEnd = (uint64_t)(region.Address + region.Length);

	if (overlappingRegion.Address <= region.Address && overlapEnd > (uint64_t)region.Address && overlapEnd < regionEnd) return OVERLAP_START;
	else if ((uint64_t)overlappingRegion.Address < regionEnd && overlappingRegion.Address > region.Address && overlapEnd >= regionEnd) return OVERLAP_END;
	else if (overlappingRegion.Address >= region.Address && overlapEnd <= regionEnd) return OVERLAP_INSIDE;
	else if (overlappingRegion.Address <= region.Address && overlapEnd >= regionEnd) return OVERLAP_OUTSIDE;

	return OVERLAP_NONE;
}

/**
 * @param region [in] region descriptor
 * @return region max frames count
*/
size_t PMM::GetRegionFramesCount(RegionDescriptor region) {
	if (region.Length < AlignUp(FrameSize + sizeof(FrameDescriptor), FrameSize)) return 0;
	return AlignDown((size_t)region.Length, FrameSize) / (FrameSize + sizeof(FrameDescriptor)) - 1;
}

PMM::RegionDescriptor PMM::GetRegionDescriptor(size_t index) {
	return Descriptors_[index];
}

size_t PMM::GetRegionsCount() {
	return DescriptorsCount_;
}

/* #ifndef VMM_HXX
void* operator new(size_t bytesCount) {
	return PMM::AllocatePhysicalMemory(bytesCount);
}
 
void* operator new[](size_t bytesCount) {
	return PMM::AllocatePhysicalMemory(bytesCount);
}
 
void operator delete(void* memory) {
	PMM::FreePhysicalMemory(memory);
}
 
void operator delete[](void* memory) {
	PMM::FreePhysicalMemory(memory);
}

void operator delete(void* memory, size_t) {
	PMM::FreePhysicalMemory(memory);
}

void operator delete [](void* memory, size_t) {
	PMM::FreePhysicalMemory(memory);
}
#endif */