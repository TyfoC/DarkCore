#pragma once
#ifndef EXCEPTIONS_HXX
#define EXCEPTIONS_HXX

#include "cpu.hxx"
#include "idt.hxx"
#include "terminal/terminal.hxx"

class Exceptions {
	public:
	static void Initialize(IDT::PEntry table);
	static void SetRoutine(size_t index, CPU::ISR routine);
	static CPU::ISR GetRoutine(size_t index);
	private:
	static CPU::ISR Routines_[CPU::ExceptionsCount];
};

#endif