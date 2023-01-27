#pragma once
#ifndef PS2_CONTROLLER_HXX
#define PS2_CONTROLLER_HXX

#include "../acpi/acpi.hxx"
#include "../cpu/hardware-irqs.hxx"

class PS2Controller {
	public:
	enum Ports {
		PORT_DATA =											0x60,
		PORT_STATUS =										0x64,
		PORT_COMMAND =										0x64
	};

	enum Statuses {
		STATUS_CAN_READ =									0x01,
		STATUS_CANNOT_WRITE =								0x02,
		STATUS_HARD_RESET =									0x04,
		STATUS_DATA_SENT_TO_DATA_PORT =						0x08,
		STATUS_KEYBOARD_PORT_UNLOCKED =						0x10,
		STATUS_RECEIVED_MOUSE_BYTE =						0x20,
		STATUS_TIME_OUT_ERROR =								0x40,
		STATUS_PARITY_ERROR =								0x80
	};

	enum Commands {
		COMMAND_READ_CONFIGURATION_BYTE =					0x20,
		COMMAND_WRITE_CONFIGURATION_BYTE =					0x60,
		COMMAND_LOCK_MOUSE_PORT =							0xA7,
		COMMAND_UNLOCK_MOUSE_PORT =							0xA8,
		COMMAND_TEST_MOUSE_PORT =							0xA9,
		COMMAND_TEST_CONTROLLER =							0xAA,
		COMMAND_TEST_KEYBOARD_PORT =						0xAB,
		COMMAND_LOCK_KEYBOARD_PORT =						0xAD,
		COMMAND_UNLOCK_KEYBOARD_PORT =						0xAE,
		COMMAND_READ_CONTROLLER_OUTPUT_PORT =				0xD0,
		COMMAND_WRITE_CONTROLLER_OUTPUT_PORT =				0xD1,
		COMMAND_WRITE_TO_MOUSE_PORT =						0xD4,
		COMMAND_REBOOT =									0xFE
	};

	enum ConfigurationBits {
		CONFIGURATION_KEYBOARD_PORT_IRQ_UNLOCKED =			0x01,
		CONFIGURATION_MOUSE_PORT_IRQ_UNLOCKED =				0x02,
		// HARD_RESET =										0x04,		<- Statuses
		// RESERVED =										0x08,
		CONFIGURATION_KEYBOARD_PORT_LOCKED =				0x10,
		CONFIGURATION_MOUSE_PORT_LOCKED =					0x20,
		CONFIGURATION_TRANSLATE_SCAN_CODES_TO_SET_1 =		0x40
		// RESERVED =										0x80
	};

	enum OutputBits {
		OUTPUT_SYSTEM_RUNNING =								0x01,
		OUTPUT_LOCK_A20_LINE =								0x02,
		OUTPUT_MOUSE_PORT_OUTPUT_DATA_LINE_STATE =			0x04,
		OUTPUT_MOUSE_PORT_OUTPUT_SYNC_LINE_STATE =			0x08,
		OUTPUT_KEYBOARD_PORT_IRQ_ENABLE =					0x10,
		OUTPUT_MOUSE_PORT_IRQ_ENABLE =						0x20,
		OUTPUT_KEYBOARD_PORT_OUTPUT_SYNC_LINE_STATE =		0x40,
		OUTPUT_KEYBOARD_PORT_OUTPUT_DATA_LINE_STATE =		0x80
	};

	enum Answers {
		ANSWER_CONTROLLER_TEST_PASSED =						0x55,
		ANSWER_PORT_TEST_PASSED =							0x00,
		ANSWER_RESET_PASSED =								0xFA,
		ANSWER_CONTROLLER_TEST_FAILED =						0xFC,
		ANSWER_RESET_FAILED =								0xFC
	};

	static constexpr size_t BootArchitectureFlagsBit8042 =	0x02;

	static bool Initialize();
	static void SendCommand(uint8_t command);
	static void SendData(uint8_t data);
	static uint8_t ReadAnswer();
	static bool KeyboardPortAvailable();
	static bool MousePortAvailable();
	static bool ReadyToSend();
	static bool ReadyToReceive();
	static void WaitUntilReadyToSend();
	static void WaitUntilReadyToReceive();
	static void LockInput();
	static void UnlockInput();
	static void FlushBuffer();
	static void Reboot();
	private:
	static bool KeyboardPortAvailable_;
	static bool MousePortAvailable_;
};

#endif