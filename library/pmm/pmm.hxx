#pragma once
#ifndef PMM_HXX
#define PMM_HXX

#include "../common/memory-utils.hxx"

class PMM {
	public:
	enum RegionTypes {
		REGION_USABLE =			0x01,
		REGION_RESERVED =		0x02,
		REGION_RECLAIMABLE =	0x03,
	};

	enum RegionOverlapTypes {
		OVERLAP_NONE =			0x00,
		OVERLAP_START =			0x01,
		OVERLAP_END =			0x02,
		OVERLAP_INSIDE =		0x03,
		OVERLAP_OUTSIDE =		0x04
	};

	enum FrameFlagsBits {
		FRAME_BUSY =			0x01
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) RegionDescriptor {
		uint64_t	Address;
		uint64_t	Length;
		uint32_t	Type;
	} RegionDescriptor, *PRegionDescriptor;

	typedef struct FrameDescriptor {
		size_t		BufferAddress;
		size_t		BusyCount;
		size_t		Flags;
	} FrameDescriptor, *PFrameDescriptor;

	static constexpr size_t FrameSize = 0x1000;

	static void Initialize(PRegionDescriptor descriptors, size_t count);
	static void* AllocatePhysicalMemory(size_t count);
	static void* ReallocatePhysicalMemory(void* memory, size_t count);
	static size_t FreePhysicalMemory(void* memory);
	static void TweakRegionDescriptors(PRegionDescriptor destination, const PRegionDescriptor source, psize_t countRegions);
	static void SortRegionDescriptors(PRegionDescriptor descriptors, size_t count);
	static void RemoveSimilarRegionDescriptors(PRegionDescriptor descriptors, psize_t count);
	static void RemoveExtraRegions(PRegionDescriptor descriptors, PRegionDescriptor extra, psize_t countRegions, size_t countExtra);
	static uint8_t GetRegionOverlapType(RegionDescriptor region, RegionDescriptor overlappingRegion);
	static size_t GetRegionFramesCount(RegionDescriptor region);
	static RegionDescriptor GetRegionDescriptor(size_t index);
	static size_t GetRegionsCount();
	private:
	static PRegionDescriptor Descriptors_;
	static size_t DescriptorsCount_;
};

#endif