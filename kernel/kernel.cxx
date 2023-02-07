#include "kernel.hxx"

EXTERN_CXX size_t __FileBegin[];
EXTERN_CXX size_t __FileEnd[];

EXTERN_C void DarkCore(BootInformation::Structure bootInformationStructure) {
	BootInformation::Initialize(&bootInformationStructure);
	Terminal::Redraw(Terminal::GetColor());
	Terminal::PutString("Welcome to DarkCore, initializing drivers...\r\n");

	GDT::Select(Kernel::GlobalDT, DARK_CORE_GDT_ENTRIES_COUNT);
	IDT::Select(Kernel::InterruptDT, DARK_CORE_IDT_ENTRIES_COUNT);
	Exceptions::Initialize(Kernel::InterruptDT);
	VirtualIC::Initialize();
	HardwareIRQs::Initialize(Kernel::InterruptDT);
	VirtualTimer::Initialize();

	bool acpiControlOverriden = ACPI::Initialize();
	if (!acpiControlOverriden) Terminal::PrintFormat("%a0EWarning: cannot initialize ACPI, shutdown!\r\n");
	
	PMM::PRegionDescriptor regionDescriptors = (PMM::PRegionDescriptor)BootInformation::GetARDTableAddress();
	size_t regionsCount = BootInformation::GetARDTableEntriesCount();
	PMM::TweakRegionDescriptors(regionDescriptors, regionDescriptors, &regionsCount);
	PMM::SortRegionDescriptors(regionDescriptors, regionsCount);
	PMM::RemoveSimilarRegionDescriptors(regionDescriptors, &regionsCount);

	PMM::RegionDescriptor extraRegions[] = {
		{ 0x00000000, 0x00000500, PMM::REGION_RESERVED },	 										//	IVT & BDA
		{ (size_t)&__FileBegin, (size_t)&__FileEnd - (size_t)&__FileBegin, PMM::REGION_RESERVED },	//	kernel
		{ 0x00080000, 0x00080000, PMM::REGION_RESERVED },											//	video, EBDA, motherboard BIOS, ...
		{ 0x00F00000, 0x00100000, PMM::REGION_RESERVED }											//	ISA hole
	};

	const size_t extraRegionsCount = sizeof(extraRegions) / sizeof(PMM::RegionDescriptor);
	PMM::RemoveExtraRegions(regionDescriptors, extraRegions, &regionsCount, extraRegionsCount);
	PMM::Initialize(regionDescriptors, regionsCount);

	Kernel::VMMDirectory = VMM::CreateDirectory();
	VMM::SelectDirectory(Kernel::VMMDirectory);

	VMM::MapPages(
		Kernel::VMMDirectory,
		VMM::PageIndexFromAddress((size_t)&__FileBegin),
		(size_t)&__FileBegin,
		VMM::PagesCountFromBytesCount((size_t)&__FileEnd - (size_t)&__FileBegin),
		VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE | VMM::PAGE_FLAG_BUSY
	);

	VMM::MapPages(
		Kernel::VMMDirectory,
		VMM::PageIndexFromAddress(Terminal::BufferAddress),
		Terminal::BufferAddress,
		VMM::PagesCountFromBytesCount(Terminal::BufferSize),
		VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE | VMM::PAGE_FLAG_BUSY
	);

	VMM::EnableVirtualMemory();

	bool psControlOverriden = PS2Controller::Initialize();
	if (!psControlOverriden) Terminal::PrintFormat("%a0EWarning: cannot initialize PS/2 controller, access keyboard & mouse!\r\n");
	else PS2Keyboard::Initialize();

	StorageDevices::Initialize();

	if (BootInformation::GetConfigSpaceAccessMechanism() & BootInformation::ACCESS_MECHANISM_STANDARD) {
		PCI::CheckAllBuses([](uint8_t bus, uint8_t device, uint8_t function, uint8_t baseClass, uint8_t subClass) {
			if (baseClass == PCI::CLASS_MASS_STORAGE_CONTROLLER && subClass == PCI::SUB_CLASS_MASS_STORAGE_IDE_CONTROLLER) {
				uint16_t bar0 = (uint16_t)PCI::ReadLong(bus, device, function, 0x04);
				uint16_t bar1 = (uint16_t)PCI::ReadLong(bus, device, function, 0x05);
				uint16_t bar2 = (uint16_t)PCI::ReadLong(bus, device, function, 0x06);
				uint16_t bar3 = (uint16_t)PCI::ReadLong(bus, device, function, 0x07);
				uint16_t bar4 = (uint16_t)PCI::ReadLong(bus, device, function, 0x08);
				
				IDEControllerChannel* ideControllerSecondaryChannel = new IDEControllerChannel(
					bar2 ? bar2 & 0xFFFC : IDEControllerChannel::STANDARD_PORT_SECONDARY_BASE,
					bar3 ? bar3 & 0xFFFC : IDEControllerChannel::STANDARD_PORT_SECONDARY_CONTROL,
					(bar4 & 0xFFFC) + 8
				);
				
				IDEControllerChannel* ideControllerPrimaryChannel = new IDEControllerChannel(
					bar0 ? bar0 & 0xFFFC : IDEControllerChannel::STANDARD_PORT_PRIMARY_BASE,
					bar1 ? bar1 & 0xFFFC : IDEControllerChannel::STANDARD_PORT_PRIMARY_CONTROL,
					bar4 & 0xFFFC
				);

				for (size_t i = 0; i < ideControllerPrimaryChannel->GetDevicesCount(); i++) StorageDevices::Append(ideControllerPrimaryChannel->GetDevice(i));
				for (size_t i = 0; i < ideControllerSecondaryChannel->GetDevicesCount(); i++) StorageDevices::Append(ideControllerSecondaryChannel->GetDevice(i));
			}
		});
	}

	UserModeCalls::Initialize(Kernel::InterruptDT, 0x08);
	UserModeCalls::SetFunction(DARK_CORE_UMC_POWER_SERVICE, Kernel::UserModeCalls::PowerService);
	UserModeCalls::SetFunction(DARK_CORE_UMC_MEMORY_SERVICE, Kernel::UserModeCalls::MemoryService);
	UserModeCalls::SetFunction(DARK_CORE_UMC_TELETYPE_SERVICE, Kernel::UserModeCalls::TerminalService);
	UserModeCalls::SetFunction(DARK_CORE_UMC_KEYBOARD_SERVICE, Kernel::UserModeCalls::KeyboardService);
	UserModeCalls::SetFunction(DARK_CORE_UMC_FILESYSTEM_SERVICE, Kernel::UserModeCalls::FileSystemService);
	UserModeCalls::SetFunction(DARK_CORE_UMC_THREAD_SERVICE, Kernel::UserModeCalls::ThreadService);
	
	char* binaryObjectData;
	bool neededObjectDescriptorFound = false;
	LiteFS::ObjectDescriptor objectDescriptor;
	size_t storageDeviceIndex = 0;
	StorageDevice* storageDevice;
	for (storageDeviceIndex = 0; storageDeviceIndex < StorageDevices::GetCount(); storageDeviceIndex++) {
		storageDevice = (StorageDevice*)&StorageDevices::Get(storageDeviceIndex);
		if (LiteFS::GetObjectDescriptor(*storageDevice, LiteFS::RunConfigPath, &objectDescriptor)) {
			Terminal::PrintFormat("Found base partition on device %c/\r\n", StorageDevice::GetLetterByIndex(storageDeviceIndex));
			binaryObjectData = new char[(size_t)objectDescriptor.UnalignedSize];
			if (LiteFS::ReadFile(*storageDevice, LiteFS::RunConfigPath, binaryObjectData)) {
				neededObjectDescriptorFound = true;
				break;
			}
			if (neededObjectDescriptorFound) break;
			delete[] binaryObjectData;
		}
	}

	if (neededObjectDescriptorFound) {
		List<RPFParser::Element> rpfElements = RPFParser::Parse(binaryObjectData, (size_t)objectDescriptor.UnalignedSize);

		char* startProgramPath = 0;
		for (size_t j = 0; j < rpfElements.GetCount(); j++) {
			if (StringUtils::Compare(rpfElements[j].Key, "START_PROGRAM")) {
				startProgramPath = rpfElements[j].Value;
				Terminal::PrintFormat("Start program path: `%s`\r\n", startProgramPath);
			}
		}
		delete[] binaryObjectData;

		if (!startProgramPath || !LiteFS::GetObjectDescriptor(*storageDevice, startProgramPath, &objectDescriptor)) {
			Terminal::PrintFormat("%a0CError: cannot get info about important file!\r\n");
		}
		else {
			binaryObjectData = new char[objectDescriptor.UnalignedSize];
			if (!LiteFS::ReadFile(*storageDevice, startProgramPath, binaryObjectData)) {
				Terminal::PrintFormat("%a0CError: cannot load important file (size = %u bytes)!\r\n", objectDescriptor.UnalignedSize);
			}
			else {
				Scheduler::Thread supervisorThread = Scheduler::CreateThread(
					false,
					(size_t)Kernel::SupervisorTask,
					PMM::FrameSize,
					0x10, 0x08, InlineAssembly::ReadFlagsRegister()
				);

				VMM::MapPages(
					supervisorThread.PagingDirectory,
					(size_t)&__FileBegin / PMM::FrameSize,
					(size_t)&__FileBegin, AlignUp((size_t)&__FileEnd - (size_t)&__FileBegin, PMM::FrameSize) / PMM::FrameSize,
					VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
				);

				VMM::MapPages(
					supervisorThread.PagingDirectory,
					Terminal::BufferAddress / PMM::FrameSize,
					Terminal::BufferAddress, AlignUp(Terminal::BufferSize, PMM::FrameSize) / PMM::FrameSize,
					VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
				);

				Kernel::InitialPath[1] = StorageDevice::GetLetterByIndex(storageDeviceIndex);
				Scheduler::Thread shellThread = LEF::CreateThread((puint8_t)binaryObjectData, 0x10, 0x08, Kernel::InitialPath);

				VMM::MapPages(
					shellThread.PagingDirectory,
					(size_t)&__FileBegin / PMM::FrameSize,
					(size_t)&__FileBegin, AlignUp((size_t)&__FileEnd - (size_t)&__FileBegin, PMM::FrameSize) / PMM::FrameSize,
					VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
				);

				VMM::MapPages(
					shellThread.PagingDirectory,
					Terminal::BufferAddress / PMM::FrameSize,
					Terminal::BufferAddress, AlignUp(Terminal::BufferSize, PMM::FrameSize) / PMM::FrameSize,
					VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
				);

				Scheduler::AddThread(&supervisorThread);
				Scheduler::AddThread(&shellThread);

				delete[] binaryObjectData;

				Scheduler::Run(Kernel::InterruptDT, Kernel::VMMDirectory);
			}
		}

		RPFParser::FreeElements(rpfElements);
	}
	else Terminal::PutString("Cannot find any storage device with LiteFS!\r\n");
	
	__cxa_finalize(0);
	INSERT_ASSEMBLY("jmp .");
}

void Kernel::SupervisorTask() {
	while (1);
}

void Kernel::UserModeCalls::PowerService(CPU::PISRData pointer) {
	if (pointer->CounterRegister == 0) ACPI::PowerOff();
	else if (pointer->CounterRegister == 1) PS2Controller::Reboot();
}

void Kernel::UserModeCalls::MemoryService(CPU::PISRData pointer) {
	VMM::Directory* currentVMMDirectory = VMM::GetSelectedDirectory();

	if (pointer->CounterRegister == 0) {
		pointer->AccumulatorRegister = (size_t)VMM::AllocateVirtualMemory(
			currentVMMDirectory, pointer->DataRegister, VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE
		);
	}
	else if (pointer->CounterRegister == 1) {
		pointer->AccumulatorRegister = (size_t)VMM::FreeVirtualMemory(currentVMMDirectory, (void*)pointer->DataRegister);
	}
}

void Kernel::UserModeCalls::TerminalService(CPU::PISRData pointer) {
	VMM::Directory* currentVMMDirectory = VMM::GetSelectedDirectory();

	if (pointer->CounterRegister == 0) Terminal::Redraw(Terminal::GetColor());
	else if (pointer->CounterRegister == 1) {
		if (pointer->DataRegister == 0) pointer->AccumulatorRegister = (size_t)Terminal::GetColumn();
		else if (pointer->DataRegister == 1) pointer->AccumulatorRegister = (size_t)Terminal::GetLine();
		else if (pointer->DataRegister == 2) pointer->AccumulatorRegister = (size_t)Terminal::GetColor();
	}
	else if (pointer->CounterRegister == 2) {
		if (pointer->DataRegister == 0) Terminal::SetColumn((uint8_t)pointer->BaseRegister);
		else if (pointer->DataRegister == 1) Terminal::SetLine((uint8_t)pointer->BaseRegister);
		else if (pointer->DataRegister == 2) Terminal::SetColor((uint8_t)pointer->BaseRegister);
	}
	else if (pointer->CounterRegister == 3) Terminal::PutChar((char)pointer->DataRegister);
	else if (pointer->CounterRegister == 4) {
		char* strPhysSource = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		Terminal::PutString(strPhysSource);
	}
	else if (pointer->CounterRegister == 5) {
		psize_t paramsAddress = (psize_t)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		char* strFormat = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, *paramsAddress);
		++paramsAddress;

		uint8_t terminalColor = Terminal::GetColor();
		for (size_t i = 0; strFormat[i];) {
			if (strFormat[i] == '%') {
				++i;
				if (strFormat[i] == 's') {
					Terminal::PutString((char*)VMM::GetPhysicalAddress(currentVMMDirectory, *paramsAddress));
					++paramsAddress;
					++i;
				}
				else {
					size_t tmpArgument = Terminal::PrintFormatTemplate(&strFormat[i - 1], *paramsAddress);
					if (tmpArgument) {
						if (strFormat[i] != 'a') ++paramsAddress;
						i += tmpArgument - 1;
					}
				}
			}
			else Terminal::PutChar(strFormat[i++]);
		}
		Terminal::SetColor(terminalColor);
	}
	else if (pointer->CounterRegister == 6) {
		void* data = (void*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		Terminal::PrintHexData(data, pointer->BaseRegister, pointer->SourceIndexRegister);
	}
}

void Kernel::UserModeCalls::KeyboardService(CPU::PISRData pointer) {
	VMM::Directory* currentVMMDirectory = VMM::GetSelectedDirectory();

	if (pointer->CounterRegister == 0) {
		char* buffer = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);

		Scheduler::DisableTaskSwitching();
		INSERT_ASSEMBLY("sti");
		Terminal::ReadString(buffer, pointer->BasePointer);
		INSERT_ASSEMBLY("cli");
		Scheduler::EnableTaskSwitching();
	}
	else if (pointer->CounterRegister == 1) {
		Scheduler::DisableTaskSwitching();
		INSERT_ASSEMBLY("sti");
		while (PS2Keyboard::GetInput().Released);
		INSERT_ASSEMBLY("cli");
		Scheduler::EnableTaskSwitching();
	}
}

void Kernel::UserModeCalls::FileSystemService(CPU::PISRData pointer) {
	VMM::Directory* currentVMMDirectory = VMM::GetSelectedDirectory();

	if (pointer->CounterRegister == 0) pointer->AccumulatorRegister = Scheduler::GetCurrentThread()->CurrentPathStringVirtualAddress;
	else if (pointer->CounterRegister == 1) {
		pointer->AccumulatorRegister = 1;
		char* currentPathStr = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, Scheduler::GetCurrentThread()->CurrentPathStringVirtualAddress);
		char* pathStr = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		String pathString = PerformPathString(currentPathStr, pathStr);

		size_t storageDeviceIndex = StorageDevice::GetIndexByLetter(pathString[1]);
		if (storageDeviceIndex != WRONG_INDEX && storageDeviceIndex < StorageDevices::GetCount()) {
			StorageDevice storageDevice = {};
			if (!CreatePhysicalStorageDevice(Kernel::VMMDirectory, StorageDevices::Get(storageDeviceIndex), storageDevice)) pointer->AccumulatorRegister = 0;

			if (pathString[1] && pointer->AccumulatorRegister) {
				Scheduler::DisableTaskSwitching();
				INSERT_ASSEMBLY("sti");

				size_t dividerOffset = pathString.FindFirstMatch("/", 1);
				size_t pathStringOffset = dividerOffset != WRONG_INDEX ? dividerOffset + 1 : pathString.GetLength();
				LiteFS::ObjectDescriptor objectDescriptor = {};

				bool result = LiteFS::GetObjectDescriptor(storageDevice, &pathString[pathStringOffset], &objectDescriptor);
				if (!result || objectDescriptor.Type != LiteFS::OBJECT_TYPE_DIRECTORY) pointer->AccumulatorRegister = 0;
				else {
					Scheduler::PThread currentThread = Scheduler::GetCurrentThread();
					size_t destPathVirtAddr = (size_t)VMM::AllocateVirtualMemory(currentVMMDirectory, pathString.GetLength(), VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE);
					if (!destPathVirtAddr) pointer->AccumulatorRegister = 0;
					else {
						size_t destPathPhysAddr = VMM::GetPhysicalAddress(currentVMMDirectory, destPathVirtAddr);
						MemoryUtils::Copy((void*)destPathPhysAddr, (void*)&pathString[0], pathString.GetLength() + 1);
						if (currentThread->CurrentPathStringVirtualAddress) VMM::FreeVirtualMemory(currentVMMDirectory, (void*)currentThread->CurrentPathStringVirtualAddress);
						currentThread->CurrentPathStringVirtualAddress = destPathVirtAddr;
					}
				}

				INSERT_ASSEMBLY("cli");
				Scheduler::EnableTaskSwitching();
			}

			FreePhysicalStorageDevice(storageDevice);
		}
		else pointer->AccumulatorRegister = 0;
	}
	else if (pointer->CounterRegister == 2) {		//	DataReg=dirPath|BaseReg=&directoryIterator
		char* pathStr = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		String pathString = PerformPathString((char*)VMM::GetPhysicalAddress(currentVMMDirectory, Scheduler::GetCurrentThread()->CurrentPathStringVirtualAddress), pathStr);

		size_t storageDeviceIndex = StorageDevice::GetIndexByLetter(pathString[1]);
		if (storageDeviceIndex != WRONG_INDEX && storageDeviceIndex < StorageDevices::GetCount()) {
			StorageDevice storageDevice = {};
			if (!CreatePhysicalStorageDevice(Kernel::VMMDirectory, StorageDevices::Get(storageDeviceIndex), storageDevice)) pointer->AccumulatorRegister = 0;

			if (pointer->AccumulatorRegister) {
				Scheduler::DisableTaskSwitching();
				INSERT_ASSEMBLY("sti");

				LiteFS::PDirectoryIterator directoryIterator = (LiteFS::PDirectoryIterator)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->BaseRegister);
				size_t dividerOffset = pathString.FindFirstMatch("/", 1);
				size_t pathStringOffset = dividerOffset != WRONG_INDEX ? dividerOffset + 1 : pathString.GetLength();
				pointer->AccumulatorRegister = LiteFS::CreateDirectoryIterator(storageDevice, &pathString[pathStringOffset], directoryIterator);
				directoryIterator->StorageDeviceIndex = storageDeviceIndex;

				INSERT_ASSEMBLY("cli");
				Scheduler::EnableTaskSwitching();
			}

			FreePhysicalStorageDevice(storageDevice);
		}
		else pointer->AccumulatorRegister = 0;
	}
	else if (pointer->CounterRegister == 3) {		//	DataReg=&directoryIterator|BaseReg=&objectInformation
		LiteFS::PDirectoryIterator directoryIterator = (LiteFS::PDirectoryIterator)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		LiteFS::PObjectInformation objectInformation = (LiteFS::PObjectInformation)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->BaseRegister);
		
		if (directoryIterator->StorageDeviceIndex != WRONG_INDEX && directoryIterator->StorageDeviceIndex < StorageDevices::GetCount()) {
			StorageDevice storageDevice = {};
			if (CreatePhysicalStorageDevice(Kernel::VMMDirectory, StorageDevices::Get(directoryIterator->StorageDeviceIndex), storageDevice)) {
				Scheduler::DisableTaskSwitching();
				INSERT_ASSEMBLY("sti");
				pointer->AccumulatorRegister = LiteFS::GetNextObjectInformation(storageDevice, directoryIterator, objectInformation) == true;
				INSERT_ASSEMBLY("cli");
				Scheduler::EnableTaskSwitching();
			}
			else pointer->AccumulatorRegister = 0;

			FreePhysicalStorageDevice(storageDevice);
		}
		else pointer->AccumulatorRegister = 0;
	}
	else if (pointer->CounterRegister == 4) {
		char* pathStr = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		String pathString = PerformPathString((char*)VMM::GetPhysicalAddress(currentVMMDirectory, Scheduler::GetCurrentThread()->CurrentPathStringVirtualAddress), pathStr);

		size_t storageDeviceIndex = StorageDevice::GetIndexByLetter(pathString[1]);
		if (storageDeviceIndex != WRONG_INDEX && storageDeviceIndex < StorageDevices::GetCount()) {
			StorageDevice storageDevice = {};
			if (!CreatePhysicalStorageDevice(Kernel::VMMDirectory, StorageDevices::Get(storageDeviceIndex), storageDevice)) pointer->AccumulatorRegister = 0;

			if (pointer->AccumulatorRegister) {
				Scheduler::DisableTaskSwitching();
				INSERT_ASSEMBLY("sti");

				LiteFS::PObjectInformation objectInformation = (LiteFS::PObjectInformation)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->BaseRegister);
				LiteFS::ObjectDescriptor objectDescriptor = {};
				size_t dividerOffset = pathString.FindFirstMatch("/", 1);
				size_t pathStringOffset = dividerOffset != WRONG_INDEX ? dividerOffset + 1 : pathString.GetLength();
				bool result = LiteFS::GetObjectDescriptor(storageDevice, &pathString[pathStringOffset], &objectDescriptor);
				if (!result) pointer->AccumulatorRegister = 0;
				else {
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
				}

				INSERT_ASSEMBLY("cli");
				Scheduler::EnableTaskSwitching();
			}

			FreePhysicalStorageDevice(storageDevice);
		}
		else pointer->AccumulatorRegister = 0;
	}
	else if (pointer->CounterRegister == 5) {
		char* pathStr = (char*)VMM::GetPhysicalAddress(currentVMMDirectory, pointer->DataRegister);
		String pathString = PerformPathString((char*)VMM::GetPhysicalAddress(currentVMMDirectory, Scheduler::GetCurrentThread()->CurrentPathStringVirtualAddress), pathStr);
		
		size_t storageDeviceIndex = StorageDevice::GetIndexByLetter(pathString[1]);
		if (storageDeviceIndex != WRONG_INDEX) {
			StorageDevice storageDevice = {};
			if (CreatePhysicalStorageDevice(Kernel::VMMDirectory, StorageDevices::Get(storageDeviceIndex), storageDevice)) {
				LiteFS::ObjectDescriptor fileDescriptor;
				Scheduler::DisableTaskSwitching();
				INSERT_ASSEMBLY("sti");

				size_t dividerOffset = pathString.FindFirstMatch("/", 1);
				size_t pathStringOffset = dividerOffset != WRONG_INDEX ? dividerOffset + 1 : pathString.GetLength();
				bool result = LiteFS::GetObjectDescriptor(storageDevice, &pathString[pathStringOffset], &fileDescriptor);

				INSERT_ASSEMBLY("cli");
				Scheduler::EnableTaskSwitching();

				if (result) {
					size_t bufferVirtAddr = (size_t)VMM::AllocateVirtualMemory(
						currentVMMDirectory,
						(size_t)fileDescriptor.UnalignedSize,
						VMM::PAGE_FLAG_PRESENT | VMM::PAGE_FLAG_READ_WRITE | VMM::PAGE_FLAG_USER
					);

					size_t bufferPhysAddr = VMM::GetPhysicalAddress(currentVMMDirectory, bufferVirtAddr);
					
					Scheduler::DisableTaskSwitching();
					INSERT_ASSEMBLY("sti");

					result = LiteFS::ReadFile(storageDevice, &pathString[pathStringOffset], (void*)bufferPhysAddr);
					
					INSERT_ASSEMBLY("cli");
					Scheduler::EnableTaskSwitching();

					if (result) {
						pointer->AccumulatorRegister = bufferVirtAddr;
						pointer->CounterRegister = (size_t)fileDescriptor.UnalignedSize;
					}
					else pointer->AccumulatorRegister = 0;
				}
				else pointer->AccumulatorRegister = 0;
			}
			else pointer->AccumulatorRegister = 0;
			
			FreePhysicalStorageDevice(storageDevice);
		}
		else pointer->AccumulatorRegister = 0;
	}
}

void Kernel::UserModeCalls::ThreadService(CPU::PISRData pointer) {
	if (pointer->CounterRegister == 0) pointer->AccumulatorRegister = Scheduler::GetCurrentThread()->WorkedTicksCount;
}

bool Kernel::UserModeCalls::CreatePhysicalStorageDevice(VMM::PDirectory vmmDirectory, StorageDevice& virtualStorageDevice, StorageDevice& resultPhysicalStorageDevice) {
	void* data = (void*)VMM::GetPhysicalAddress(vmmDirectory, (size_t)virtualStorageDevice.GetData());
	IDEControllerChannel::Device* ideCCDevice;
	IDEControllerChannel* ideCChannel;
	size_t ideCChannelAddress;

	switch (virtualStorageDevice.GetType()) {
		case StorageDevice::TYPE_IDE_CONTROLLER_CHANNEL_DEVICE:
			ideCCDevice = (IDEControllerChannel::Device*)PMM::AllocatePhysicalMemory(sizeof(IDEControllerChannel::Device));
			if (!ideCCDevice) return false;
			MemoryUtils::Copy(ideCCDevice, data, sizeof(IDEControllerChannel::Device));
			
			ideCChannel = (IDEControllerChannel*)PMM::AllocatePhysicalMemory(sizeof(IDEControllerChannel));
			if (!ideCChannel) return false;

			ideCChannelAddress = VMM::GetPhysicalAddress(vmmDirectory, (size_t)ideCCDevice->GetChannel());
			MemoryUtils::Copy(ideCChannel, (void*)ideCChannelAddress, sizeof(IDEControllerChannel));
			ideCCDevice->SetChannel(ideCChannel);

			resultPhysicalStorageDevice = StorageDevice(*ideCCDevice);
			break;
		default:
			return false;
	};

	return true;
}

bool Kernel::UserModeCalls::FreePhysicalStorageDevice(StorageDevice& physicalStorageDevice) {
	void* data = physicalStorageDevice.GetData();
	switch(physicalStorageDevice.GetType()) {
		case StorageDevice::TYPE_IDE_CONTROLLER_CHANNEL_DEVICE:
			PMM::FreePhysicalMemory(((IDEControllerChannel::Device*)data)->GetChannel());
			PMM::FreePhysicalMemory(data);
			break;
		default:
			return false;
	}

	return true;
}

String Kernel::UserModeCalls::PerformPathString(const char* currentPath, const char* rawPath) {
	String pathString(rawPath);

	if (pathString[0] != '/') {
		String fullPath(currentPath);
		fullPath += "/";
		fullPath += pathString;
		pathString = fullPath;
	}

	List<String::Part> pathParts = pathString.Split("/");
	size_t count = pathParts.GetCount();
	for (size_t i = 0; i < count; i++) {
		if (pathParts[i].Length == 1 && pathString[pathParts[i].StartOffset] == '.' && i) {
			if (i == count - 1) pathString = pathString.GetSubString(0, pathParts[i].StartOffset - 1);
			else pathString = pathString.Erase(pathParts[i].StartOffset - 1, pathParts[i].Length + 1);
			pathParts = pathString.Split("/");
			count = pathParts.GetCount();
			i = (size_t)-1;
		}
	}

	for (size_t i = 0; i < count; i++) {
		if (pathParts[i].Length == 2 && MemoryUtils::Compare(&pathString[pathParts[i].StartOffset], "..", 2) && i >= 2) {
			size_t prevPos = pathString.FindLastMatch("/", pathParts[i].StartOffset - 2);
			pathString = pathString.Erase(prevPos + 1, pathParts[i - 1].Length + pathParts[i].Length + 1);
			pathParts = pathString.Split("/");
			count = pathParts.GetCount();
			i = (size_t)-1;
		}
	}

	while (pathString[pathString.GetLength() - 1] == '/') pathString = pathString.GetSubString(0, pathString.GetLength() - 1);
	return pathString;
}

char Kernel::InitialPath[] = "/ /base";
VMM::PDirectory Kernel::VMMDirectory = 0;

GDT::Entry DEFINE_SPECIAL(ALIGNED_DEFINITION(16)) Kernel::GlobalDT[DARK_CORE_GDT_ENTRIES_COUNT] = {
	{ 0, 0, 0, 0, 0, 0 },
	{
		0xFFFF, 0,
		GDT::ACCESS_READ_WRITE | GDT::ACCESS_EXECUTABLE | GDT::ACCESS_CODE_DATA | GDT::ACCESS_PRESENT, 0x0F,
		GDT::FLAG_PROTECTED_MODE | GDT::FLAG_GRANULARITY, 0
	},
	{
		0xFFFF, 0,
		GDT::ACCESS_READ_WRITE | GDT::ACCESS_CODE_DATA | GDT::ACCESS_PRESENT, 0x0F,
		GDT::FLAG_PROTECTED_MODE | GDT::FLAG_GRANULARITY, 0
	},
	{
		0xFFFF, 0,
		GDT::ACCESS_READ_WRITE | GDT::ACCESS_EXECUTABLE | GDT::ACCESS_CODE_DATA | GDT::ACCESS_USER | GDT::ACCESS_PRESENT, 0x0F,
		GDT::FLAG_PROTECTED_MODE | GDT::FLAG_GRANULARITY, 0
	},
	{
		0xFFFF, 0,
		GDT::ACCESS_READ_WRITE | GDT::ACCESS_CODE_DATA | GDT::ACCESS_USER | GDT::ACCESS_PRESENT, 0x0F,
		GDT::FLAG_PROTECTED_MODE | GDT::FLAG_GRANULARITY, 0
	}
};

IDT::Entry DEFINE_SPECIAL(ALIGNED_DEFINITION(16)) Kernel::InterruptDT[DARK_CORE_IDT_ENTRIES_COUNT];