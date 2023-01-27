#include "virtual-ic.hxx"

VirtualIC::Function VirtualIC::MaskHardwareIRQ_;
VirtualIC::Function VirtualIC::UnmaskHardwareIRQ_;
VirtualIC::Function VirtualIC::EndOfInterrupt_;

void VirtualIC::Initialize() {
	PIC::Remap(32, 40);
	PIC::MaskEachHardwareIRQ();
	MaskHardwareIRQ_ = PIC::MaskHardwareIRQ;
	UnmaskHardwareIRQ_ = PIC::UnmaskHardwareIRQ;
	EndOfInterrupt_ = PIC::EndOfInterrupt;
}

void VirtualIC::MaskHardwareIRQ(size_t index) {
	MaskHardwareIRQ_(index);
}

void VirtualIC::UnmaskHardwareIRQ(size_t index) {
	UnmaskHardwareIRQ_(index);
}

void VirtualIC::EndOfInterrupt(size_t index) {
	EndOfInterrupt_(index);
}

void VirtualIC::SetFunctionMaskHardwareIRQ(Function function) {
	MaskHardwareIRQ_ = function;
}

void VirtualIC::SetFunctionUnmaskHardwareIRQ(Function function) {
	UnmaskHardwareIRQ_ = function;
}

void VirtualIC::SetFunctionEndOfInterrupt(Function function) {
	EndOfInterrupt_ = function;
}

VirtualIC::Function VirtualIC::GetFunctionMaskHardwareIRQ() {
	return MaskHardwareIRQ_;
}

VirtualIC::Function VirtualIC::GetFunctionUnmaskHardwareIRQ() {
	return UnmaskHardwareIRQ_;
}

VirtualIC::Function VirtualIC::GetFunctionEndOfInterrupt() {
	return EndOfInterrupt_;
}