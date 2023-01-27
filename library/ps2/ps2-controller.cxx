#include "ps2-controller.hxx"

bool PS2Controller::KeyboardPortAvailable_ = false;
bool PS2Controller::MousePortAvailable_ = false;

bool PS2Controller::Initialize() {
	if (ACPI::Initialized() && ACPI::GetMajorVersion() > 1) {
		if (!(ACPI::GetBootArchitectureFlags() & BootArchitectureFlagsBit8042)) return false;
	}

	LockInput();
	FlushBuffer();

	SendCommand(COMMAND_READ_CONFIGURATION_BYTE);
	uint8_t configuration = ReadAnswer();
	configuration &= (uint8_t)~(
		CONFIGURATION_KEYBOARD_PORT_IRQ_UNLOCKED |
		CONFIGURATION_MOUSE_PORT_IRQ_UNLOCKED |
		CONFIGURATION_TRANSLATE_SCAN_CODES_TO_SET_1
	);

	SendCommand(COMMAND_WRITE_CONFIGURATION_BYTE);
	SendData(configuration);

	bool singleChannel = !(configuration & CONFIGURATION_MOUSE_PORT_LOCKED);

	SendCommand(COMMAND_TEST_CONTROLLER);
	uint8_t result = ReadAnswer();
	SendCommand(COMMAND_WRITE_CONFIGURATION_BYTE);						//	for compatibility
	SendData(configuration);

	if (result != ANSWER_CONTROLLER_TEST_PASSED) return false;

	if (!singleChannel) {
		SendCommand(COMMAND_UNLOCK_MOUSE_PORT);
		SendCommand(COMMAND_READ_CONFIGURATION_BYTE);
		configuration = ReadAnswer();
		singleChannel = configuration & CONFIGURATION_MOUSE_PORT_LOCKED;
		if (!singleChannel) SendCommand(COMMAND_LOCK_MOUSE_PORT);
	}

	SendCommand(COMMAND_TEST_KEYBOARD_PORT);
	KeyboardPortAvailable_ = ReadAnswer() == ANSWER_PORT_TEST_PASSED;
	if (!singleChannel) {
		SendCommand(COMMAND_TEST_MOUSE_PORT);
		MousePortAvailable_ = ReadAnswer() == ANSWER_PORT_TEST_PASSED;
	}

	UnlockInput();

	return true;
}

void PS2Controller::SendCommand(uint8_t command) {
	InlineAssembly::SendPortByte(PORT_COMMAND, command);
}

void PS2Controller::SendData(uint8_t data) {
	WaitUntilReadyToSend();
	InlineAssembly::SendPortByte(PORT_DATA, data);
}

uint8_t PS2Controller::ReadAnswer() {
	WaitUntilReadyToReceive();
	return InlineAssembly::ReadPortByte(PORT_DATA);
}

bool PS2Controller::KeyboardPortAvailable() {
	return KeyboardPortAvailable_;
}

bool PS2Controller::MousePortAvailable() {
	return MousePortAvailable_;
}

bool PS2Controller::ReadyToSend() {
	return !(InlineAssembly::ReadPortByte(PORT_STATUS) & STATUS_CANNOT_WRITE);
}

bool PS2Controller::ReadyToReceive() {
	return InlineAssembly::ReadPortByte(PORT_STATUS) & STATUS_CAN_READ;
}

void PS2Controller::WaitUntilReadyToSend() {
	while (!ReadyToSend());
}

void PS2Controller::WaitUntilReadyToReceive() {
	while (!ReadyToReceive());
}

void PS2Controller::LockInput() {
	SendCommand(COMMAND_LOCK_KEYBOARD_PORT);
	SendCommand(COMMAND_LOCK_MOUSE_PORT);
}

void PS2Controller::UnlockInput() {
	SendCommand(COMMAND_UNLOCK_KEYBOARD_PORT);
	SendCommand(COMMAND_UNLOCK_MOUSE_PORT);
}

void PS2Controller::FlushBuffer() {
	while (ReadyToReceive()) ReadAnswer();
}

void PS2Controller::Reboot() {
	InlineAssembly::DisableIRQs();
	FlushBuffer();
	WaitUntilReadyToSend();
	SendCommand(COMMAND_REBOOT);
	INSERT_ASSEMBLY("hlt");
}