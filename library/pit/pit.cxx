#include "pit.hxx"

static size_t TicksCount_ = 0;

/**
 * PIT IRQ handler
 * @param [-] pointer to ISRData
*/
static void IRQHandler(CPU::PISRData pointer);
static void IRQHandler(CPU::PISRData pointer) {
	UNREFERENCED_PARAMETER(pointer);
	++TicksCount_;
}

void PIT::Initialize() {
	HardwareIRQs::SetRoutine(IRQIndex, IRQHandler);
	uint16_t divisor = HardwareFrequency / SoftwareFrequency;

	InlineAssembly::SendPortByte(PORT_COMMAND, MODE_SQUARE_WAVE | BYTE_ACCESS_BOTH);
	InlineAssembly::SendPortByte(PORT_FIRST_COUNTER, (uint8_t)(divisor & 0xFF));
	InlineAssembly::SendPortByte(PORT_FIRST_COUNTER, (uint8_t)((divisor >> 8) & 0xFF));

	TicksCount_ = 0;
	VirtualIC::UnmaskHardwareIRQ(IRQIndex);
}

/**
 * @param countTicks [in] count of PIT ticks to wait
*/
void PIT::Sleep(size_t countTicks) {
	countTicks += TicksCount_;
	while (TicksCount_ < countTicks);
}

/**
 * @return PIT ticks count
*/
size_t PIT::GetTicksCount() {
	return TicksCount_;
}