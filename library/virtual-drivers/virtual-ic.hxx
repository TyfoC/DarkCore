#pragma once
#ifndef VIRTUAL_IC_HXX
#define VIRTUAL_IC_HXX

#include "../pic/pic.hxx"

//	virtual interrupt controller
class VirtualIC {
	public:
	typedef void (*Function)(size_t index);

	static void Initialize();
	static void MaskHardwareIRQ(size_t index);
	static void UnmaskHardwareIRQ(size_t index);
	static void EndOfInterrupt(size_t index);
	static void SetFunctionMaskHardwareIRQ(Function function);
	static void SetFunctionUnmaskHardwareIRQ(Function function);
	static void SetFunctionEndOfInterrupt(Function function);
	static Function GetFunctionMaskHardwareIRQ();
	static Function GetFunctionUnmaskHardwareIRQ();
	static Function GetFunctionEndOfInterrupt();
	private:
	static Function MaskHardwareIRQ_;
	static Function UnmaskHardwareIRQ_;
	static Function EndOfInterrupt_;
};

#endif