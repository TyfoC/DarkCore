#pragma once
#ifndef KERNEL_H
#define KERNEL_H

#include <common/boot-information.hxx>
#include <terminal/terminal.hxx>

#include <cpu/gdt.hxx>
#include <cpu/idt.hxx>
#include <cpu/exceptions.hxx>
#include <cpu/hardware-irqs.hxx>
#include <cpu/usermode-calls.hxx>

#include <virtual-drivers/virtual-timer.hxx>
#include <acpi/acpi.hxx>

#include <pmm/pmm.hxx>
#include <vmm/vmm.hxx>

#include <scheduler/scheduler.hxx>

#include <ps2/ps2-controller.hxx>
#include <ps2/ps2-keyboard.hxx>

#include <pci/pci.hxx>
#include <ide/ide.hxx>
#include <storage-device/storage-devices.hxx>

#include <lite-fs/lite-fs.hxx>
#include <common/rpf-parser.hxx>
#include <format/lef/lef.hxx>

#define DARK_CORE_GDT_ENTRIES_COUNT								0x05
#define DARK_CORE_IDT_ENTRIES_COUNT								0x100

//	user mode interrupt (0xEC)
#define DARK_CORE_UMC_POWER_SERVICE								0
#define DARK_CORE_UMC_MEMORY_SERVICE							1
#define DARK_CORE_UMC_TELETYPE_SERVICE							2
#define DARK_CORE_UMC_KEYBOARD_SERVICE							3
#define DARK_CORE_UMC_FILESYSTEM_SERVICE						4
#define DARK_CORE_UMC_THREAD_SERVICE							5

EXTERN_C void DarkCore(BootInformation::Structure bootInformationStructure);

class Kernel {
	public:
	class UserModeCalls {
		public:
		static void PowerService(CPU::PISRData pointer);
		static void MemoryService(CPU::PISRData pointer);
		static void TerminalService(CPU::PISRData pointer);
		static void KeyboardService(CPU::PISRData pointer);
		static void FileSystemService(CPU::PISRData pointer);
		static void ThreadService(CPU::PISRData pointer);

		static bool CreatePhysicalStorageDevice(VMM::PDirectory vmmDirectory, StorageDevice& virtualStorageDevice, StorageDevice& resultPhysicalStorageDevice);
		static bool FreePhysicalStorageDevice(StorageDevice& physicalStorageDevice);

		static String PerformPathString(const char* currentPath, const char* rawPath);
	};

	static void SupervisorTask();

	static char InitialPath[];
	static VMM::PDirectory VMMDirectory;
	static GDT::Entry GlobalDT[];
	static IDT::Entry InterruptDT[];
};

#endif