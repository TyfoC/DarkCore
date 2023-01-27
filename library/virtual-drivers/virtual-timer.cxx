#include "virtual-timer.hxx"

VirtualTimer::SleepFunction VirtualTimer::SleepFunction_;
VirtualTimer::GetTicksCountFunction VirtualTimer::GetTicksCountFunction_;

void VirtualTimer::Initialize() {
	PIT::Initialize();
	SleepFunction_ = PIT::Sleep;
	GetTicksCountFunction_ = PIT::GetTicksCount;
}

void VirtualTimer::Sleep(size_t ticksCount) {
	ticksCount += GetTicksCountFunction_();
	while (GetTicksCountFunction_() < ticksCount);
}

size_t VirtualTimer::GetTicksCount() {
	return GetTicksCountFunction_();
}

void VirtualTimer::SetFunctionSleep(
	SleepFunction function
) {
	SleepFunction_ = function;
}

void VirtualTimer::SetFunctionGetTicksCount(GetTicksCountFunction function) {
	GetTicksCountFunction_ = function;
}

VirtualTimer::SleepFunction VirtualTimer::GetFunctionSleep() {
	return SleepFunction_;
}

VirtualTimer::GetTicksCountFunction VirtualTimer::GetFunctionGetTicksCount() {
	return GetTicksCountFunction_;
}