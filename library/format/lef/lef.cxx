#include "lef.hxx"

//	curPath - physical offset!
Scheduler::Thread LEF::CreateThread(const uint8_t* data, uint16_t dataSeg, uint16_t codeSeg, const char* curPath, bool suspended) {
	PLEFHeader lefHeader = (PLEFHeader)data;
	// PSectionHeader sectionHeaders = (PSectionHeader)&data[lefHeader->SectionsOffset];

	Scheduler::Thread thread = Scheduler::CreateThread(
		suspended,
		lefHeader->BaseAddress + lefHeader->EntryPointOffset,
		lefHeader->StackSize,
		dataSeg, codeSeg, InlineAssembly::ReadFlagsRegister()
	);

	const size_t pagesCount = AlignUp(lefHeader->FileSize, PMM::FrameSize) / PMM::FrameSize;
	size_t destAddress = lefHeader->BaseAddress, sourceAddress = (size_t)data;
	for (size_t i = 0; i < pagesCount; i++) {
		VMM::AllocateVirtualMemoryToPage(
			thread.PagingDirectory,
			destAddress / PMM::FrameSize,
			PMM::FrameSize,
			VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE | VMM::PAGE_FLAG_USER
		);

		size_t sourcePhysAddr = VMM::GetPhysicalAddress(VMM::GetSelectedDirectory(), sourceAddress);
		size_t destPhysAddr = VMM::GetPhysicalAddress(thread.PagingDirectory, destAddress);

		bool pagingEnabled = VMM::VirtualMemoryEnabled();
		if (pagingEnabled) VMM::DisableVirtualMemory();

		MemoryUtils::Copy((void*)destPhysAddr, (void*)sourcePhysAddr, PMM::FrameSize);

		if (pagingEnabled) VMM::EnableVirtualMemory();

		sourceAddress += PMM::FrameSize;
		destAddress += PMM::FrameSize;
	}

	//	CAUTION/WARNING - STACK MUST BE KILLED
	if (thread.CurrentPathStringVirtualAddress && curPath) {
		size_t destPhysAddress = VMM::GetPhysicalAddress(thread.PagingDirectory, thread.CurrentPathStringVirtualAddress);
		VMM::DisableVirtualMemory();
		MemoryUtils::Copy((void*)destPhysAddress, curPath, StringUtils::GetLength(curPath) + 1);
		VMM::EnableVirtualMemory();
	}

	return thread;
}