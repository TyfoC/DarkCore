#include "exceptions.hxx"

CPU::ISR Exceptions::Routines_[CPU::ExceptionsCount];

EXTERN_C {
	void ExceptionStub0();
	void ExceptionStub1();
	void ExceptionStub2();
	void ExceptionStub3();
	void ExceptionStub4();
	void ExceptionStub5();
	void ExceptionStub6();
	void ExceptionStub7();
	void ExceptionStub8();
	void ExceptionStub9();
	void ExceptionStub10();
	void ExceptionStub11();
	void ExceptionStub12();
	void ExceptionStub13();
	void ExceptionStub14();
	void ExceptionStub15();
	void ExceptionStub16();
	void ExceptionStub17();
	void ExceptionStub18();
	void ExceptionStub19();
	void ExceptionStub20();
	void ExceptionStub21();
	void ExceptionStub22();
	void ExceptionStub23();
	void ExceptionStub24();
	void ExceptionStub25();
	void ExceptionStub26();
	void ExceptionStub27();
	void ExceptionStub28();
	void ExceptionStub29();
	void ExceptionStub30();
	void ExceptionStub31();
}

/**
 * CPU global exception handler
 * @param pointer [in, out] pointer to ISRData
*/
EXTERN_C void GlobalExceptionsRoutine(CPU::PISRData pointer);
EXTERN_C void GlobalExceptionsRoutine(CPU::PISRData pointer) {
	if (pointer->InterruptIndex < CPU::ExceptionsCount){
		CPU::ISR routine = Exceptions::GetRoutine(pointer->InterruptIndex);
		if (routine) routine(pointer);
		else {
			Terminal::PrintFormat("%a0CError: unhandled exception #%u (error code - 0x%x)!\r\n", pointer->InterruptIndex, pointer->ErrorCode);
			while (1);
		}
	}
}

/**
 * @param table [out] IDT
*/
void Exceptions::Initialize(IDT::PEntry table) {
	table[0] = IDT::CreateEntry((size_t)ExceptionStub0, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[1] = IDT::CreateEntry((size_t)ExceptionStub1, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[2] = IDT::CreateEntry((size_t)ExceptionStub2, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[3] = IDT::CreateEntry((size_t)ExceptionStub3, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[4] = IDT::CreateEntry((size_t)ExceptionStub4, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[5] = IDT::CreateEntry((size_t)ExceptionStub5, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[6] = IDT::CreateEntry((size_t)ExceptionStub6, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[7] = IDT::CreateEntry((size_t)ExceptionStub7, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[8] = IDT::CreateEntry((size_t)ExceptionStub8, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[9] = IDT::CreateEntry((size_t)ExceptionStub9, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[10] = IDT::CreateEntry((size_t)ExceptionStub10, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[11] = IDT::CreateEntry((size_t)ExceptionStub11, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[12] = IDT::CreateEntry((size_t)ExceptionStub12, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[13] = IDT::CreateEntry((size_t)ExceptionStub13, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[14] = IDT::CreateEntry((size_t)ExceptionStub14, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[15] = IDT::CreateEntry((size_t)ExceptionStub15, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[16] = IDT::CreateEntry((size_t)ExceptionStub16, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[17] = IDT::CreateEntry((size_t)ExceptionStub17, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[18] = IDT::CreateEntry((size_t)ExceptionStub18, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[19] = IDT::CreateEntry((size_t)ExceptionStub19, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[20] = IDT::CreateEntry((size_t)ExceptionStub20, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[21] = IDT::CreateEntry((size_t)ExceptionStub21, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[22] = IDT::CreateEntry((size_t)ExceptionStub22, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[23] = IDT::CreateEntry((size_t)ExceptionStub23, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[24] = IDT::CreateEntry((size_t)ExceptionStub24, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[25] = IDT::CreateEntry((size_t)ExceptionStub25, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[26] = IDT::CreateEntry((size_t)ExceptionStub26, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[27] = IDT::CreateEntry((size_t)ExceptionStub27, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[28] = IDT::CreateEntry((size_t)ExceptionStub28, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[29] = IDT::CreateEntry((size_t)ExceptionStub29, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[30] = IDT::CreateEntry((size_t)ExceptionStub30, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
	table[31] = IDT::CreateEntry((size_t)ExceptionStub31, InlineAssembly::ReadCodeSegment(), IDT::GATE_INT32, IDT::FLAG_PRESENT);
}

/**
 * @param index [in] exception index (0,...,31)
 * @param routine [in] ISR
*/
void Exceptions::SetRoutine(size_t index, CPU::ISR routine) {
	Routines_[index] = routine;
}

/**
 * @param index [in] exception index (0,...,31)
 * @return ISR
*/
CPU::ISR Exceptions::GetRoutine(size_t index) {
	return Routines_[index];
}