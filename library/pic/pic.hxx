#pragma once
#ifndef PIC_HXX
#define PIC_HXX

#include "../common/inline-assembly.hxx"

class PIC {
	public:
	enum Ports {
		PORT_MASTER_COMMAND =		0x20,
		PORT_MASTER_DATA =			0x21,
		PORT_SLAVE_COMMAND =		0xA0,
		PORT_SLAVE_DATA =			0xA1
	};

	enum Commands {
		COMMAND_ICW1_ICW4 =			0x01,
		COMMAND_ICW4_UP_MODE =		0x01,
		COMMAND_ICW1_INITIALIZE =	0x10,
		COMMAND_END_OF_INTERRUPT =	0x20
	};

	static void Remap(size_t masterBase, size_t slaveBase);
	static void MaskHardwareIRQ(size_t index);
	static void UnmaskHardwareIRQ(size_t index);

	static inline void MaskEachHardwareIRQ() {
		InlineAssembly::InlineAssembly::SendPortByte(PORT_MASTER_DATA, 0xFF);
		InlineAssembly::InlineAssembly::SendPortByte(PORT_SLAVE_DATA, 0xFF);
	}

	static inline void UnmaskEachHardwareIRQ() {
		InlineAssembly::InlineAssembly::SendPortByte(PORT_MASTER_DATA, 0);
		InlineAssembly::InlineAssembly::SendPortByte(PORT_SLAVE_DATA, 0);
	}

	/**
	 * @param index [in] IRQ index
	*/
	static inline void EndOfInterrupt(size_t index) {
		InlineAssembly::InlineAssembly::SendPortByte(PORT_MASTER_COMMAND, COMMAND_END_OF_INTERRUPT);
		if (index >= 8) InlineAssembly::InlineAssembly::SendPortByte(PORT_SLAVE_COMMAND, COMMAND_END_OF_INTERRUPT);
	}
};

#endif