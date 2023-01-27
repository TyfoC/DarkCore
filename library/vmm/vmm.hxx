#pragma once
#ifndef VMM_HXX
#define VMM_HXX

#include "../common/inline-assembly.hxx"
#include "../pmm/pmm.hxx"

class VMM {
	public:
	static constexpr size_t TablesCount =	0x400;
	static constexpr size_t PagesCount =	0x400;

	enum PageFlags {
		PAGE_FLAG_PRESENT =							0x0001,
		PAGE_FLAG_READ_WRITE =						0x0002,
		PAGE_FLAG_USER =							0x0004,
		PAGE_FLAG_WRITE_THROUGH =					0x0008,
		PAGE_FLAG_DISABLE_CACHING =					0x0010,
		PAGE_FLAG_DIRTY =							0x0040,
		PAGE_FLAG_PAGE_ATTRIBUTE_TABLE =			0x0080,
		PAGE_FLAG_GLOBAL =							0x0100,

		PAGE_FLAG_BUSY =							0x0200,
		PAGE_FLAG_BUSY_GROUP_CLOSER =				0x0400,
		PAGE_FLAG_ALLOCATED =						0x0800
	};

	enum TableFlags {
		TABLE_FLAG_PRESENT =						0x0001,
		TABLE_FLAG_READ_WRITE =						0x0002,
		TABLE_FLAG_USER =							0x0004,
		TABLE_FLAG_WRITE_THROUGH =					0x0008,
		TABLE_FLAG_DISABLE_CACHING =				0x0010,

		TABLE_FLAG_ALLOCATED =						0x0800		//	unused, but still exist
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) DirectoryEntry {
		uint8_t		Present				:1;
		uint8_t		ReadWrite			:1;
		uint8_t		User				:1;
		uint8_t		WriteThrough		:1;
		uint8_t		DisableCaching		:1;
		uint8_t		Accessed			:1;
		uint8_t		Reserved1			:1;
		uint8_t		PageSizeExtension	:1;		//	unused
		uint8_t		Reserved2			:1;
		uint8_t		Reserved3			:1;
		uint8_t		Reserved4			:1;
		uint8_t		Allocated			:1;
		uint32_t	PhysicalPageIndex	:20;
	} DirectoryEntry, *PDirectoryEntry;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) TableEntry {
		uint8_t		Present				:1;
		uint8_t		ReadWrite			:1;
		uint8_t		User				:1;
		uint8_t		WriteThrough		:1;
		uint8_t		DisableCaching		:1;
		uint8_t		Accessed			:1;
		uint8_t		Reserved1			:1;
		uint8_t		PageSizeExtension	:1;		//	unused
		uint8_t		Reserved2			:1;
		uint8_t		Busy				:1;
		uint8_t		Closing				:1;
		uint8_t		Allocated			:1;
		uint32_t	PhysicalPageIndex	:20;
	} TableEntry, *PTableEntry;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Table {
		TableEntry		Pages[PagesCount];
	} Table, *PTable;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Directory {
		DirectoryEntry	Tables[TablesCount];
	} Directory, *PDirectory;

	static void EnableVirtualMemory();
	static void DisableVirtualMemory();
	static bool VirtualMemoryEnabled();

	static void SelectDirectory(Directory* dir);
	static Directory* GetSelectedDirectory();
	static Directory* CreateDirectory();
	static void FreeDirectory(Directory* dir);

	static size_t CreateTables(Directory* dir, size_t tableIndex, size_t tablesCount, uint16_t flags);
	static bool FreeTables(Directory* dir, size_t tableIndex, size_t tablesCount);

	static bool CreatePages(Directory* dir, size_t pageIndex, size_t pagesCount, uint16_t flags);
	static bool FreePages(Directory* dir, size_t pageIndex, size_t pagesCount);

	static bool MapPages(Directory* dir, size_t pageIndex, size_t physAddress, size_t countPages, uint16_t flags);
	static bool UnmapPages(Directory* dir, size_t pageIndex, size_t countPages);

	static size_t FindFreePagesBlockPageIndex(Directory* dir, size_t pagesCount);

	static size_t PageIndexFromAddress(size_t address);
	static size_t PagesCountFromBytesCount(size_t bytesCount);
	static size_t TableIndexFromAddress(size_t address);

	static size_t AddressFromPageIndex(size_t pageIndex);

	static void* AllocateVirtualMemory(Directory* dir, size_t bytesCount, uint16_t flags);
	static bool AllocateVirtualMemoryToPage(Directory* dir, size_t pageIndex, size_t bytesCount, uint16_t flags);
	static void* ReallocateVirtualMemory(Directory* dir, void* memory, size_t bytesCount, uint16_t flags);
	static bool ReallocateVirtualMemoryToPage(Directory* dir, void* memory, size_t pageIndex, size_t bytesCount, uint16_t flags);
	static bool FreeVirtualMemory(Directory* dir, void* memory);

	static size_t GetPhysicalAddress(Directory* dir, size_t virtAddress);
};

#endif