#pragma once
#ifndef VIRTUAL_TIMER_HXX
#define VIRTUAL_TIMER_HXX

#include "../pit/pit.hxx"

class VirtualTimer {
	public:
	typedef void (*SleepFunction)(size_t countTicks);
	typedef size_t (*GetTicksCountFunction)();

	static void Initialize();
	static void Sleep(size_t ticksCount);
	static size_t GetTicksCount();
	static void SetFunctionSleep(SleepFunction function);
	static void SetFunctionGetTicksCount(GetTicksCountFunction function);
	static SleepFunction GetFunctionSleep();
	static GetTicksCountFunction GetFunctionGetTicksCount();
	private:
	static SleepFunction SleepFunction_;
	static GetTicksCountFunction GetTicksCountFunction_;
};

#endif