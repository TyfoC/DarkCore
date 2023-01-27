#pragma once
#ifndef HARDWARE_INTERRUPTS_HXX
#define HARDWARE_INTERRUPTS_HXX

#include "cpu.hxx"
#include "idt.hxx"
#include "../virtual-drivers/virtual-ic.hxx"

class HardwareIRQs {
	public:
	static void Initialize(IDT::PEntry table);
	static void SetRoutine(size_t index, CPU::ISR routine);
	static CPU::ISR GetRoutine(size_t index);
	private:
	static CPU::ISR Routines_[CPU::HardwareIRQsCount];
};

#endif