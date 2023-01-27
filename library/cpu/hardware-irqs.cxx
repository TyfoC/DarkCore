#include "hardware-irqs.hxx"

CPU::ISR HardwareIRQs::Routines_[CPU::HardwareIRQsCount];

EXTERN_C {
	void HardwareInterruptStub0();
	void HardwareInterruptStub1();
	void HardwareInterruptStub2();
	void HardwareInterruptStub3();
	void HardwareInterruptStub4();
	void HardwareInterruptStub5();
	void HardwareInterruptStub6();
	void HardwareInterruptStub7();
	void HardwareInterruptStub8();
	void HardwareInterruptStub9();
	void HardwareInterruptStub10();
	void HardwareInterruptStub11();
	void HardwareInterruptStub12();
	void HardwareInterruptStub13();
	void HardwareInterruptStub14();
	void HardwareInterruptStub15();
}

/**
 * CPU global IRQs handler
 * @param pointer [in, out] pointer to CPU::ISRData
*/
EXTERN_C void GlobalHardwareInterruptsRoutine(CPU::PISRData pointer);
EXTERN_C void GlobalHardwareInterruptsRoutine(CPU::PISRData pointer) {
	size_t index = pointer->InterruptIndex - 32;
	if (index < CPU::HardwareIRQsCount) {
		CPU::ISR routine = HardwareIRQs::GetRoutine(index);
		if (routine) routine(pointer);
	}
	VirtualIC::EndOfInterrupt(index);
}

/**
 * @param table [out] IDT
*/
void HardwareIRQs::Initialize(IDT::PEntry table) {
	table[32] = IDT::CreateEntry((size_t)HardwareInterruptStub0, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[33] = IDT::CreateEntry((size_t)HardwareInterruptStub1, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[34] = IDT::CreateEntry((size_t)HardwareInterruptStub2, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[35] = IDT::CreateEntry((size_t)HardwareInterruptStub3, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[36] = IDT::CreateEntry((size_t)HardwareInterruptStub4, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[37] = IDT::CreateEntry((size_t)HardwareInterruptStub5, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[38] = IDT::CreateEntry((size_t)HardwareInterruptStub6, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[39] = IDT::CreateEntry((size_t)HardwareInterruptStub7, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[40] = IDT::CreateEntry((size_t)HardwareInterruptStub8, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[41] = IDT::CreateEntry((size_t)HardwareInterruptStub9, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[42] = IDT::CreateEntry((size_t)HardwareInterruptStub10, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[43] = IDT::CreateEntry((size_t)HardwareInterruptStub11, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[44] = IDT::CreateEntry((size_t)HardwareInterruptStub12, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[45] = IDT::CreateEntry((size_t)HardwareInterruptStub13, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[46] = IDT::CreateEntry((size_t)HardwareInterruptStub14, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[47] = IDT::CreateEntry((size_t)HardwareInterruptStub15, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	InlineAssembly::EnableIRQs();
}

/**
 * @param index [in] IRQ index
 * @param routine [in] CPU::ISR
*/
void HardwareIRQs::SetRoutine(size_t index, CPU::ISR routine) {
	Routines_[index] = routine;
}

/**
 * @param index [in] IRQ index
 * @return CPU::ISR
*/
CPU::ISR HardwareIRQs::GetRoutine(size_t index) {
	return Routines_[index];
}