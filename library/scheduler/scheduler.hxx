#pragma once
#ifndef SCHEDULER_HXX
#define SCHEDULER_HXX

#include "../cpu/hardware-irqs.hxx"
#include "../virtual-drivers/virtual-timer.hxx"
#include "../virtual-drivers/virtual-ic.hxx"
#include "../vmm/vmm.hxx"

class Scheduler {
	public:
	static constexpr size_t DirectoryVirtualAddress =	0;
	static constexpr size_t CountAllocate =				10;
	static constexpr size_t MaxPathLength =				0x1000;
	enum States {
		STATE_RUNNING =					0x00,
		STATE_READY_TO_RUN =			0x01,
		STATE_SLEEPING =				0x02,
		STATE_SUSPENDED =				0x03		//	not used, but still exist
	};

	enum TicksCounts {
		TICKS_COUNT_KERNEL_PRIORITY =	600,
		TICKS_COUNT_USER_PRIORITY =		100
	};

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) ThreadRegisters {
		size_t	GSegment;
		size_t	FSegment;
		size_t	ExtraSegment;
		size_t	DataSegment;
		size_t	DestinationIndexRegister;
		size_t	SourceIndexRegister;
		size_t	BasePointer;
		size_t	StackPointer;
		size_t	BaseRegister;
		size_t	DataRegister;
		size_t	CounterRegister;
		size_t	AccumulatorRegister;
		size_t	InstructionPointer;
		size_t	CodeSegment;
		size_t	FlagsRegister;
	} ThreadRegisters, *PThreadRegisters;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Thread {
		size_t			State;
		size_t			SleepTicksCount;
		size_t			TicksBeforeSwitchCount;
		size_t			WorkedTicksCount;
		VMM::PDirectory	PagingDirectory;
		ThreadRegisters	Registers;
		Thread*			Next;
		size_t			CurrentPathStringVirtualAddress;
	} Thread, *PThread;

	static void Run(IDT::PEntry table, VMM::PDirectory kernelDirectory);
	static Thread CreateThread(bool suspended, size_t entryPoint, size_t stackSize, uint16_t dataSeg, uint16_t codeSeg, size_t flagsRegister);
	static void AddThread(const PThread thread);
	static void Sleep(size_t ticksCount);
	static size_t GetTicksCount();
	static void SwitchThreadNext();
	static PThread GetCurrentThread();
	static void RemoveCurrentThread();

	static bool TaskSwitchingEnabled();
	static void DisableTaskSwitching();
	static void EnableTaskSwitching();
};

#endif