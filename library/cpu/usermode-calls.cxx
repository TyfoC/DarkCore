#include "usermode-calls.hxx"

UserModeCalls::Function UserModeCalls::Functions_[MaxFunctionsCount] = {};
size_t UserModeCalls::FunctionsCount_ = 0;

EXTERN_C void UserModeInterruptCallHandler();

void UserModeCalls::Initialize(IDT::PEntry table, uint16_t userModeSegmentSelector) {
	table[InterruptIndex] = IDT::CreateEntry(
		(size_t)UserModeInterruptCallHandler,
		userModeSegmentSelector, IDT::GATE_INT32, IDT::FLAG_PRESENT | IDT::FLAG_USER
	);
}

bool UserModeCalls::SetFunction(size_t accumulatorRegister, CPU::ISR handler) {
	if (FunctionsCount_ >= MaxFunctionsCount) return false;

	Functions_[FunctionsCount_].AccumulatorRegisterValue = accumulatorRegister;
	Functions_[FunctionsCount_].Handler = handler;

	++FunctionsCount_;
	return true;
}

CPU::ISR UserModeCalls::GetFunction(size_t accumulatorRegister) {
	for (size_t i = 0; i < FunctionsCount_; i++) {
		if (Functions_[i].AccumulatorRegisterValue == accumulatorRegister) {
			return Functions_[i].Handler;
		}
	}

	return (CPU::ISR)0;
}

EXTERN_C void UserModeCallHandler(CPU::PISRData pointer);
EXTERN_C void UserModeCallHandler(CPU::PISRData pointer) {
	CPU::ISR routine = UserModeCalls::GetFunction(pointer->AccumulatorRegister);
	if (routine) routine(pointer);
}