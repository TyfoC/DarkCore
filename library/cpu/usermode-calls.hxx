#pragma once
#ifndef USERMODE_CALLS_HXX
#define USERMODE_CALLS_HXX

#include "cpu.hxx"
#include "idt.hxx"

/*
	UMCs calls WORK WITH VM DISABLED!!!
*/
class UserModeCalls {
	public:
	static constexpr size_t InterruptIndex = 0xEC;
	static constexpr size_t MaxFunctionsCount = 100;

	typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) Function {
		size_t		AccumulatorRegisterValue;
		CPU::ISR	Handler;
	} Function, *PFunction;

	static void Initialize(IDT::PEntry table, uint16_t userModeSegmentSelector);
	static bool SetFunction(size_t accumulatorRegister, CPU::ISR handler);
	static CPU::ISR GetFunction(size_t accumulatorRegister);
	private:
	static Function Functions_[MaxFunctionsCount];
	static size_t FunctionsCount_;
};

#endif