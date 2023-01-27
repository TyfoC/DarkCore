#pragma once
#ifndef CPU_HXX
#define CPU_HXX

#include "../common/typedefs.hxx"

class CPU {
	public:
	static constexpr size_t IDTMaximumLength =	0x100;
	static constexpr size_t ExceptionsCount = 	0x20;
	static constexpr size_t HardwareIRQsCount =	0x10;
	static constexpr size_t SoftwareIRQsCount =	IDTMaximumLength - ExceptionsCount - HardwareIRQsCount;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) ISRData {
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
		size_t	InterruptIndex;
		size_t	ErrorCode;
		size_t	InstructionPointer;
		size_t	CodeSegment;
		size_t	FlagsRegister;
	} ISRData, *PISRData;

	typedef void (*ISR)(PISRData pointer);
};

#endif