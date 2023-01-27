#pragma once
#ifndef PIT_H
#define PIT_H

#include "../common/inline-assembly.hxx"
#include "../cpu/hardware-irqs.hxx"
#include "../virtual-drivers/virtual-ic.hxx"

class PIT {
	public:
	enum Ports {
		PORT_FIRST_COUNTER =	0x40,
		PORT_COMMAND =		0x43
	};

	enum ByteAccessSizeTypes {
		BYTE_ACCESS_LOW =			0x10,
		BYTE_ACCESS_HIGH =			0x20,
		BYTE_ACCESS_BOTH =			BYTE_ACCESS_LOW | BYTE_ACCESS_HIGH
	};

	enum Modes {
		MODE_SQUARE_WAVE =	0x06
	};

	static constexpr size_t IRQIndex = 0;
	static constexpr size_t HardwareFrequency = 3579545 / 3;
	static constexpr size_t SoftwareFrequency = 1000;

	static void Initialize();
	static void Sleep(size_t countTicks);
	static size_t GetTicksCount();
};

#endif