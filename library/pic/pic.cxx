#include "pic.hxx"

/**
 * @param masterBase [in] PIC master IRQs base
 * @param slaveBase [in] PIC slave IRQs base
*/
void PIC::Remap(size_t masterBase, size_t slaveBase) {
	uint8_t masterData = InlineAssembly::ReadPortByte(PORT_MASTER_DATA);
	uint8_t slaveData = InlineAssembly::ReadPortByte(PORT_SLAVE_DATA);

	InlineAssembly::SendPortByte(PORT_MASTER_COMMAND, COMMAND_ICW1_INITIALIZE | COMMAND_ICW1_ICW4);
	InlineAssembly::SendPortByte(PORT_SLAVE_COMMAND, COMMAND_ICW1_INITIALIZE | COMMAND_ICW1_ICW4);
	InlineAssembly::SendPortByte(PORT_MASTER_DATA, (uint8_t)masterBase);
	InlineAssembly::SendPortByte(PORT_SLAVE_DATA, (uint8_t)slaveBase);
	InlineAssembly::SendPortByte(PORT_MASTER_DATA, 4);
	InlineAssembly::SendPortByte(PORT_SLAVE_DATA, 2);
	InlineAssembly::SendPortByte(PORT_MASTER_DATA, COMMAND_ICW4_UP_MODE);
	InlineAssembly::SendPortByte(PORT_SLAVE_DATA, COMMAND_ICW4_UP_MODE);

	InlineAssembly::SendPortByte(PORT_MASTER_DATA, masterData);
	InlineAssembly::SendPortByte(PORT_SLAVE_DATA, slaveData);
}

/**
 * @param index [in] IRQ index
*/
void PIC::MaskHardwareIRQ(size_t index) {
	uint16_t port;

	if (index < 8) port = PORT_MASTER_DATA;
	else {
		port = PORT_SLAVE_DATA;
		index -= 8;
	}

	InlineAssembly::SendPortByte(port, (uint8_t)(InlineAssembly::ReadPortByte(port) | (1 << index)));
}

/**
 * @param index [in] IRQ index
*/
void PIC::UnmaskHardwareIRQ(size_t index) {
	uint16_t port;

	if (index < 8) port = PORT_MASTER_DATA;
	else {
		port = PORT_SLAVE_DATA;
		index -= 8;
	}

	InlineAssembly::SendPortByte(port, (uint8_t)(InlineAssembly::ReadPortByte(port) & ~(1 << index)));
}