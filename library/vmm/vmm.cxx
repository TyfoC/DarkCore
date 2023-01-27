#include "vmm.hxx"

void VMM::EnableVirtualMemory() {
	InlineAssembly::WriteControlRegister0(InlineAssembly::ReadControlRegister0() | InlineAssembly::REG_CR0_PAGING_ENABLE);
}

void VMM::DisableVirtualMemory() {
	InlineAssembly::WriteControlRegister0(InlineAssembly::ReadControlRegister0() & ~InlineAssembly::REG_CR0_PAGING_ENABLE);
}

bool VMM::VirtualMemoryEnabled() {
	return (InlineAssembly::ReadControlRegister0() & InlineAssembly::REG_CR0_PAGING_ENABLE) != 0;
}

void VMM::SelectDirectory(Directory* dir) {
	InlineAssembly::WriteControlRegister3((size_t)dir);
}

VMM::Directory* VMM::GetSelectedDirectory() {
	return (Directory*)InlineAssembly::ReadControlRegister3();
}

VMM::Directory* VMM::CreateDirectory() {
	Directory* dir = (Directory*)PMM::AllocatePhysicalMemory(sizeof(Directory));

	if (dir) {
		MemoryUtils::Fill(dir, 0, sizeof(Directory));
		if (!MapPages(dir, 0, (size_t)dir, PagesCountFromBytesCount(sizeof(Directory)), PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE)) {
			PMM::FreePhysicalMemory(dir);
			dir = (Directory*)0;
		}
	}

	return dir;
}

void VMM::FreeDirectory(Directory* dir) {
	FreePages(0, 0, PMM::FrameSize * PMM::FrameSize);
	FreeTables(dir, 0, TablesCount);
}

size_t VMM::CreateTables(Directory* dir, size_t tableIndex, size_t tablesCount, uint16_t flags) {
	if (tableIndex + tablesCount > TablesCount) return 0;

	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	void* data;
	size_t createdCount = 0;
	size_t curTableIndex = tableIndex;
	for (curTableIndex = tableIndex; curTableIndex < tableIndex + tablesCount; curTableIndex++) {
		if (!dir->Tables[curTableIndex].Allocated) {
			data = PMM::AllocatePhysicalMemory(sizeof(Table));
			if (!data) {
				if (vmEnabled) EnableVirtualMemory();
				return createdCount;
			}

			MemoryUtils::Fill(data, 0, sizeof(Table));
			dir->Tables[curTableIndex].Allocated = true;
			*((puint32_t)&dir->Tables[curTableIndex]) |= flags & 0xFFF;
			dir->Tables[curTableIndex].PhysicalPageIndex = PageIndexFromAddress((size_t)data) & 0xFFFFF;
			++createdCount;
		}
	}

	if (vmEnabled) EnableVirtualMemory();
	return createdCount;
}

bool VMM::FreeTables(Directory* dir, size_t tableIndex, size_t tablesCount) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	if (tableIndex + tablesCount > TablesCount) {
		if (vmEnabled) EnableVirtualMemory();
		return false;
	}

	for (size_t i = tableIndex; i < tableIndex + tablesCount; i++) {
		if (dir->Tables[i].Allocated) {
			PMM::FreePhysicalMemory((void*)AddressFromPageIndex(dir->Tables[i].PhysicalPageIndex));
			dir->Tables[i].Allocated = false;
		}
	}

	if (vmEnabled) EnableVirtualMemory();
	return true;
}

bool VMM::CreatePages(Directory* dir, size_t pageIndex, size_t pagesCount, uint16_t flags) {
	if (pageIndex + pagesCount > PMM::FrameSize * PagesCount) return false;

	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t tableIndex = pageIndex / PagesCount;
	pageIndex -= tableIndex * PagesCount;

	void* data;
	PTable table;
	for (; tableIndex < TablesCount && pagesCount; tableIndex++) {
		if (!dir->Tables[tableIndex].Allocated) {
			CreateTables(dir, tableIndex, 1, TABLE_FLAG_PRESENT | TABLE_FLAG_READ_WRITE);
		}

		table = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
		for (; pageIndex < PagesCount && pagesCount; pageIndex++, pagesCount--) {
			if (!table->Pages[pageIndex].Allocated) {
				data = PMM::AllocatePhysicalMemory(PMM::FrameSize);

				if (!data) {
					if (vmEnabled) EnableVirtualMemory();
					return false;
				}

				table->Pages[pageIndex].Allocated = true;
				table->Pages[pageIndex].PhysicalPageIndex = PageIndexFromAddress((size_t)data) & 0xFFFFF;
				*((puint32_t)&table->Pages[pageIndex]) |= flags & 0xFFF;
			}
		}

		pageIndex = 0;
	}

	if (vmEnabled) EnableVirtualMemory();
	return true;
}

bool VMM::FreePages(Directory* dir, size_t pageIndex, size_t pagesCount) {
	if (pageIndex + pagesCount > PMM::FrameSize * PagesCount) return false;

	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t tableIndex = pageIndex / PagesCount;
	pageIndex -= tableIndex * PagesCount;

	PTable table;
	for (; tableIndex < TablesCount && pagesCount; tableIndex++) {
		table = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
		for (; pageIndex < PagesCount && pagesCount; pageIndex++, pagesCount--) {
			if (table->Pages[pageIndex].Allocated) PMM::FreePhysicalMemory((void*)AddressFromPageIndex(table->Pages[pageIndex].PhysicalPageIndex));
			*((puint32_t)&table->Pages[pageIndex]) = 0;
		}

		pageIndex = 0;
	}

	if (vmEnabled) EnableVirtualMemory();
	return true;
}

bool VMM::MapPages(Directory* dir, size_t pageIndex, size_t physAddress, size_t countPages, uint16_t flags) {
	if (pageIndex > PMM::FrameSize * PMM::FrameSize) return false;

	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t tableIndex = pageIndex / PagesCount;
	size_t relativePageIndex = pageIndex - tableIndex * PagesCount;
	size_t physIndex = PageIndexFromAddress(physAddress);

	PTable table;
	for (; tableIndex < TablesCount && countPages; tableIndex++) {
		if (!dir->Tables[tableIndex].Allocated) {
			CreateTables(dir, tableIndex, 1, TABLE_FLAG_PRESENT | TABLE_FLAG_READ_WRITE);
		}

		table = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
		for (; relativePageIndex < PagesCount && countPages; relativePageIndex++, countPages--) {
			table->Pages[relativePageIndex].Busy = true;
			table->Pages[relativePageIndex].PhysicalPageIndex = physIndex & 0xFFFFF;
			*((puint32_t)&table->Pages[relativePageIndex]) |= flags & 0xFFF;
			physIndex++;
		}

		relativePageIndex = 0;
	}

	if (vmEnabled) EnableVirtualMemory();
	return true;
}

bool VMM::UnmapPages(Directory* dir, size_t pageIndex, size_t countPages) {
	if (pageIndex > PMM::FrameSize * PMM::FrameSize) return false;

	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t tableIndex = pageIndex / PagesCount;
	size_t relativePageIndex = pageIndex - tableIndex * PagesCount;

	PTable table;
	for (; tableIndex < TablesCount && countPages; tableIndex++) {
		if (!dir->Tables[tableIndex].Allocated) continue;
		table = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);

		for (; relativePageIndex < PagesCount && countPages; relativePageIndex++, countPages--) {
			table->Pages[relativePageIndex].Busy = false;
			table->Pages[relativePageIndex].Closing = false;
		}

		relativePageIndex = 0;
	}

	if (vmEnabled) EnableVirtualMemory();
	return true;
}

size_t VMM::FindFreePagesBlockPageIndex(Directory* dir, size_t pagesCount) {
	if (pagesCount > PMM::FrameSize * PMM::FrameSize) return WRONG_INDEX;
	
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	bool freePages;
	size_t i, j, tableIndex, pageIndex;
	size_t tmpPagesCount;

	PTable entries;
	for (i = 0; i < TablesCount; i++) {
		for (j = 0; j < PagesCount; j++) {
			freePages = true;
			tmpPagesCount = pagesCount;
			for (tableIndex = i; tableIndex < TablesCount && tmpPagesCount; tableIndex++) {
				if (dir->Tables[tableIndex].Allocated) {
					entries = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
					for (pageIndex = j; pageIndex < PagesCount && tmpPagesCount; pageIndex++, tmpPagesCount--) {
						if (entries->Pages[pageIndex].Busy) {
							freePages = false;
							break;
						}
					}
				}
				else {
					if (tmpPagesCount > PagesCount) tmpPagesCount -= PagesCount;
					else tmpPagesCount = 0;
				}

				if (!freePages || !tmpPagesCount) break;
			}

			if (freePages && !tmpPagesCount) {
				if (vmEnabled) EnableVirtualMemory();
				return i * PagesCount + j;
			}
		}
	}

	if (vmEnabled) EnableVirtualMemory();
	return WRONG_INDEX;
}

size_t VMM::PageIndexFromAddress(size_t address) {
	return (address & 0xFFFFF000) / PMM::FrameSize;
}

size_t VMM::PagesCountFromBytesCount(size_t bytesCount) {
	return AlignUp(bytesCount, PMM::FrameSize) / PMM::FrameSize;
}

size_t VMM::TableIndexFromAddress(size_t address) {
	return address / (PMM::FrameSize * PagesCount);
}

size_t VMM::AddressFromPageIndex(size_t pageIndex) {
	return pageIndex * PMM::FrameSize;
}

void* VMM::AllocateVirtualMemory(Directory* dir, size_t bytesCount, uint16_t flags) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t pagesCount = AlignUp(bytesCount, PMM::FrameSize) / PMM::FrameSize;
	size_t freePagesBlockIndex = FindFreePagesBlockPageIndex(dir, pagesCount);
	if (freePagesBlockIndex == WRONG_INDEX) {
		if (vmEnabled) EnableVirtualMemory();
		return (void*)WRONG_INDEX;
	}

	size_t tableIndex = freePagesBlockIndex / PagesCount;
	size_t pageIndex = freePagesBlockIndex - tableIndex * PagesCount;
	size_t i = tableIndex, j = pageIndex;

	void* data;
	PTable table;
	size_t tmpPagesCount = pagesCount, firstUnallocatedTableIndex = WRONG_INDEX;
	for (; i < TablesCount && tmpPagesCount; i++) {
		if (!dir->Tables[i].Allocated) {
			CreateTables(dir, i, 1, TABLE_FLAG_PRESENT | TABLE_FLAG_READ_WRITE);
			if (firstUnallocatedTableIndex == WRONG_INDEX) firstUnallocatedTableIndex = i;
		}

		table = (PTable)AddressFromPageIndex(dir->Tables[i].PhysicalPageIndex);
		for (; j < PagesCount && tmpPagesCount; j++, tmpPagesCount--) {
			data = PMM::AllocatePhysicalMemory(PMM::FrameSize);

			if (!data) {
				FreePages(dir, freePagesBlockIndex, pagesCount);
				for(; tableIndex <= i; tableIndex++) FreeTables(dir, tableIndex, 1);
				if (vmEnabled) EnableVirtualMemory();
				return (void*)WRONG_INDEX;
			}
			
			table->Pages[j].PhysicalPageIndex = PageIndexFromAddress((size_t)data) & 0xFFFFF;
			table->Pages[j].Allocated = table->Pages[j].Busy = true;
			*((puint32_t)(&table->Pages[j])) |= flags & 0xFFF;
		}

		if (!tmpPagesCount) break;
		j = 0;
	}

	table->Pages[j - 1].Closing = true;
	void* result = (void*)AddressFromPageIndex(freePagesBlockIndex);
	if (vmEnabled) EnableVirtualMemory();
	return result;
}

bool VMM::AllocateVirtualMemoryToPage(Directory* dir, size_t pageIndex, size_t bytesCount, uint16_t flags) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t tableIndex = pageIndex / PagesCount;
	size_t relPageIndex = pageIndex - tableIndex * PagesCount;
	size_t pagesCount = AlignUp(bytesCount, PMM::FrameSize) / PMM::FrameSize;

	PTable table;
	bool freePages = true;
	size_t tmpPagesCount = pagesCount;
	for (; tableIndex < TablesCount && tmpPagesCount; tableIndex++) {
		if (!dir->Tables[tableIndex].Allocated) CreateTables(dir, tableIndex, 1, TABLE_FLAG_PRESENT | TABLE_FLAG_READ_WRITE);
		table = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
		for (; relPageIndex < PagesCount && tmpPagesCount; relPageIndex++, tmpPagesCount--) {
			if (table->Pages[relPageIndex].Busy) {
				freePages = false;
				break;
			}
		}

		if (!tmpPagesCount) break;
		relPageIndex = 0;
	}

	if (!freePages || tmpPagesCount) {
		if (vmEnabled) EnableVirtualMemory();
		return false;
	}
	bool result = CreatePages(dir, pageIndex, pagesCount, flags | PAGE_FLAG_BUSY);
	if (result) table->Pages[relPageIndex - 1].Closing = true;
	if (vmEnabled) EnableVirtualMemory();
	return result;
}

void* VMM::ReallocateVirtualMemory(Directory* dir, void* memory, size_t bytesCount, uint16_t flags) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	void* newMemory = AllocateVirtualMemory(dir, bytesCount, flags);
	if (!newMemory) return (void*)WRONG_INDEX;
	MemoryUtils::Copy(newMemory, memory, bytesCount);
	if (!FreeVirtualMemory(dir, memory)) {
		FreeVirtualMemory(dir, newMemory);
		if (vmEnabled) EnableVirtualMemory();
		return (void*)WRONG_INDEX;
	}

	if (vmEnabled) EnableVirtualMemory();
	return newMemory;
}

bool VMM::ReallocateVirtualMemoryToPage(Directory* dir, void* memory, size_t pageIndex, size_t bytesCount, uint16_t flags) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	void* newMemory = (void*)AddressFromPageIndex(pageIndex);
	if (!AllocateVirtualMemoryToPage(dir, pageIndex, bytesCount, flags)) {
		if (vmEnabled) EnableVirtualMemory();
		return false;
	}
	size_t newPhysAddress = GetPhysicalAddress(dir, (size_t)newMemory);
	size_t physAddress = GetPhysicalAddress(dir, (size_t)memory);
	MemoryUtils::Copy((void*)newPhysAddress, (void*)physAddress, bytesCount);
	if (!FreeVirtualMemory(dir, memory)) {
		FreeVirtualMemory(dir, newMemory);
		if (vmEnabled) EnableVirtualMemory();
		return false;
	}

	if (vmEnabled) EnableVirtualMemory();
	return true;
}

bool VMM::FreeVirtualMemory(Directory* dir, void* memory) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t pageIndex = PageIndexFromAddress((size_t)memory);
	size_t remPagesCount = 0;

	size_t tableIndex = pageIndex / PagesCount;
	size_t relPageIndex = pageIndex - tableIndex * PagesCount;

	PTable table;
	bool closed = false;
	for (; tableIndex < TablesCount; tableIndex++) {
		if (dir->Tables[tableIndex].Allocated) {
			table = (PTable)AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
			for (; relPageIndex < PagesCount; relPageIndex++, remPagesCount++) {
				if (table->Pages[relPageIndex].Closing) {
					++remPagesCount;
					closed = true;
					break;
				}
			}
		}
		else break;

		if (closed) break;
		relPageIndex = 0;
	}

	if (!closed) {
		if (vmEnabled) EnableVirtualMemory();
		return false;
	}
	bool result = FreePages(dir, pageIndex, remPagesCount);
	if (vmEnabled) EnableVirtualMemory();
	return result;
}

size_t VMM::GetPhysicalAddress(Directory* dir, size_t virtAddress) {
	bool vmEnabled = VirtualMemoryEnabled();
	if (vmEnabled) DisableVirtualMemory();

	size_t absoluteVirtPageIndex = virtAddress / PMM::FrameSize;
	size_t tableIndex = absoluteVirtPageIndex / PagesCount;
	size_t pageIndex = absoluteVirtPageIndex - tableIndex * PagesCount;

	size_t result = AddressFromPageIndex(dir->Tables[tableIndex].PhysicalPageIndex);
	result = AddressFromPageIndex(((PTable)result)->Pages[pageIndex].PhysicalPageIndex);\

	if (vmEnabled) EnableVirtualMemory();
	return result + (virtAddress % PMM::FrameSize);
}

/*
	Only needed for ASM calls (But can be used as VMM::GetPhysicalAddress)!
*/
EXTERN_C size_t VMMConvertVirtualAddressToPhysical(VMM::Directory* dir, size_t virtualAddress);
EXTERN_C size_t VMMConvertVirtualAddressToPhysical(VMM::Directory* dir, size_t virtualAddress) {
	return VMM::GetPhysicalAddress(dir, virtualAddress);
}

void* operator new(size_t bytesCount) {
	if (!VMM::VirtualMemoryEnabled()) return PMM::AllocatePhysicalMemory(bytesCount);
	return VMM::AllocateVirtualMemory(VMM::GetSelectedDirectory(), bytesCount, VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE);
}
 
void* operator new[](size_t bytesCount) {
	return operator new(bytesCount);
}
 
void operator delete(void* memory) {
	if (!VMM::VirtualMemoryEnabled()) PMM::FreePhysicalMemory(memory);
	else VMM::FreeVirtualMemory(VMM::GetSelectedDirectory(), memory);
}
 
void operator delete[](void* memory) {
	return operator delete(memory);
}

void operator delete(void* memory, size_t) {
	return operator delete(memory);
}

void operator delete [](void* memory, size_t) {
	return operator delete(memory);
}