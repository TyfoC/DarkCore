#include "ps2-keyboard.hxx"

uint8_t PS2Keyboard::Buffer_[PS2Keyboard::MaxBufferLength];
size_t PS2Keyboard::BufferLength_ = 0;
size_t PS2Keyboard::CurrentScanCodeMapIndex_ = PS2Keyboard::SCAN_CODE_MAP_INDEX_NO_SHIFT_NO_CAPS;
bool PS2Keyboard::ShiftPressed_ = false;
bool PS2Keyboard::CapsLockPressed_ = false;

bool PS2Keyboard::Initialize() {
	if (!PS2Controller::KeyboardPortAvailable()) return false;
	PS2Controller::FlushBuffer();
	PS2Controller::SendData(COMMAND_ENABLE_SCANNING);
	
	uint8_t answer;
	size_t resendTimes = ResendTimesCount;
	do {
		answer = PS2Controller::ReadAnswer();
		if (answer == ANSWER_RESEND) {
			if (resendTimes) --resendTimes;
			else return false;
		}
		else if (answer != ANSWER_ACK) return false;
	} while (answer != ANSWER_ACK);

	HardwareIRQs::SetRoutine(IRQIndex, IRQHandler);
	VirtualIC::UnmaskHardwareIRQ(IRQIndex);

	PS2Controller::SendCommand(PS2Controller::COMMAND_READ_CONFIGURATION_BYTE);
	uint8_t controllerConfiguration = PS2Controller::ReadAnswer();
	PS2Controller::SendCommand(PS2Controller::COMMAND_WRITE_CONFIGURATION_BYTE);
	PS2Controller::SendData(controllerConfiguration | PS2Controller::CONFIGURATION_KEYBOARD_PORT_IRQ_UNLOCKED);

	if (!Reset()) return false;

	if (!PS2Keyboard::SetScanCodeSet(PS2Keyboard::SCAN_CODE_SET_2)) return false;
	return true;
}

uint8_t PS2Keyboard::ReadBufferFirstByte() {
	while (!BufferLength_);

	uint8_t value = Buffer_[0];
	for (size_t i = 0; i < BufferLength_; i++) Buffer_[i] = Buffer_[i + 1];
	BufferLength_ -= 1;
	return value;
}

uint8_t PS2Keyboard::ReadBufferLastByte() {
	while (!BufferLength_);

	uint8_t value = Buffer_[--BufferLength_];
	return value;
}

bool PS2Keyboard::SendByte(uint8_t value) {
	PS2Controller::WaitUntilReadyToSend();
	size_t timesCount = ResendTimesCount;
	uint8_t answer;

	PS2Controller::SendData(value);

	do {
		answer = ReadBufferLastByte();
		if (answer == ANSWER_RESEND) {
			if (timesCount) {
				PS2Controller::SendData(value);
				--timesCount;
			}
			else return false;
		}
		else if (answer != ANSWER_ACK) return false;
	} while (answer != ANSWER_ACK);

	return true;
}

size_t PS2Keyboard::GetBufferLength() {
	return BufferLength_;
}

bool PS2Keyboard::Echo() {
	if (!SendByte(COMMAND_ECHO)) return false;
	return ReadBufferLastByte() == ANSWER_ECHO;
}

uint8_t PS2Keyboard::GetScanCodeSet() {
	uint8_t scanCodeSet;

	if (!SendByte(COMMAND_GET_SET_SCAN_CODE_SET)) scanCodeSet = SCAN_CODE_SET_UNKNOWN;
	else {
		if (!SendByte(SUB_COMMAND_GET_CURRENT_SCAN_CODE_SET)) scanCodeSet = SCAN_CODE_SET_UNKNOWN;
		else {
			scanCodeSet = ReadBufferLastByte();

			//	real hardware answers
			if (scanCodeSet == 0x43) scanCodeSet = SCAN_CODE_SET_1;
			else if (scanCodeSet == 0x41) scanCodeSet = SCAN_CODE_SET_2;
			else if (scanCodeSet == 0x3F) scanCodeSet = SCAN_CODE_SET_3;
			else if (!(scanCodeSet >= 1 && scanCodeSet <= 3)) scanCodeSet = SCAN_CODE_SET_UNKNOWN;		//	qemu 0xF0 answers
		}
	}

	return scanCodeSet;
}

bool PS2Keyboard::SetScanCodeSet(size_t index) {
	if (!index || index > 3) return false;
	else if (!SendByte(COMMAND_GET_SET_SCAN_CODE_SET)) return false;

	return SendByte((uint8_t)index);
}

uint8_t PS2Keyboard::GetType() {
	uint8_t value;

	if (!LockInput()) value = TYPE_UNKNOWN;
	else if (!SendByte(COMMAND_IDENTIFY)) value = TYPE_AT;
	else if (ReadBufferLastByte() != 0xAB) value = TYPE_UNKNOWN;		//	0xAB - MF2 prefix
	else {
		value = ReadBufferLastByte();
		if (value == 0x83) value = TYPE_MF2;
		else if (value == 0x41 || value == 0xC1) value = TYPE_MF2_WITH_TRANSLATION_ENABLED;
		else value = TYPE_UNKNOWN;
	}

	UnlockInput();
	return value;
}

bool PS2Keyboard::LockInput() {
	return SendByte(COMMAND_DISABLE_SCANNING);
}

bool PS2Keyboard::UnlockInput() {
	return SendByte(COMMAND_ENABLE_SCANNING);
}

bool PS2Keyboard::Reset() {
	if (!SendByte(COMMAND_RESET)) return false;
	return ReadBufferLastByte() == ANSWER_RESET_DONE;
}

PS2Keyboard::InputResult PS2Keyboard::GetInput() {
	InputResult result = { 0, false, 0 };
	uint8_t value = 0;
	
	do {
		if (BufferLength_ > 0xFFFFFF) while (BufferLength_) ReadBufferLastByte();
		else {
			value = ReadBufferLastByte();
			if (value == SCAN_CODE_EXTENDED) result.ScanCode = SCAN_CODE_EXTENDED << 8;
			else if (value == SCAN_CODE_RELEASED) result.Released = true;
			else {
				result.ScanCode = value;
				break;
			}

			value = ReadBufferLastByte();
			if (value == SCAN_CODE_RELEASED) result.Released = true;
			else {
				result.ScanCode |= value;
				break;
			}

			value = ReadBufferLastByte();
			result.ScanCode |= value;
			break;
		}
	} while (true);

	bool shiftAction = result.ScanCode == SCAN_CODE_LEFT_SHIFT || result.ScanCode == SCAN_CODE_RIGHT_SHIFT;
	bool capsLockAction = result.ScanCode == SCAN_CODE_CAPS_LOCK && !result.Released;

	if (shiftAction || capsLockAction) {
		if (shiftAction) {
			if (result.Released) ShiftPressed_ = false;
			else ShiftPressed_ = true;
		}
		else CapsLockPressed_ = !CapsLockPressed_;

		if (!ShiftPressed_ && !CapsLockPressed_) CurrentScanCodeMapIndex_ = SCAN_CODE_MAP_INDEX_NO_SHIFT_NO_CAPS;
		else if (ShiftPressed_ && !CapsLockPressed_) CurrentScanCodeMapIndex_ = SCAN_CODE_MAP_INDEX_SHIFT_NO_CAPS;
		else if (!ShiftPressed_ && CapsLockPressed_) {
			CurrentScanCodeMapIndex_ = SCAN_CODE_MAP_INDEX_NO_SHIFT_CAPS;
		}
		else CurrentScanCodeMapIndex_ = SCAN_CODE_MAP_INDEX_SHIFT_CAPS;
	}

	result.ScanCodeMapIndex = CurrentScanCodeMapIndex_;
	return result;
}

char* PS2Keyboard::GetScanCodeMapByIndex(size_t index) {
	if (index == SCAN_CODE_MAP_INDEX_NO_SHIFT_NO_CAPS) return (char*)&ScanCodeMapNoShiftNoCapsLock[0];
	else if (index == SCAN_CODE_MAP_INDEX_SHIFT_NO_CAPS) return (char*)&ScanCodeMapShiftNoCapsLock[0];
	else if (index == SCAN_CODE_MAP_INDEX_NO_SHIFT_CAPS) return (char*)&ScanCodeMapNoShiftCapsLock[0];
	else if (index == SCAN_CODE_MAP_INDEX_SHIFT_CAPS) return (char*)&ScanCodeMapShiftCapsLock[0];

	return 0;
}

char PS2Keyboard::ReadChar() {
	InputResult result;
	char* scanCodeMap;

	do {
		result = GetInput();
		scanCodeMap = GetScanCodeMapByIndex(result.ScanCodeMapIndex);
	} while (result.Released || result.ScanCode == SCAN_CODE_CAPS_LOCK || result.ScanCode >= 0x80 || !scanCodeMap[result.ScanCode]);

	return scanCodeMap[result.ScanCode];
}

void PS2Keyboard::IRQHandler(CPU::PISRData pointer) {
	UNREFERENCED_PARAMETER(pointer);
	if (BufferLength_ > MaxBufferLength) BufferLength_ = MaxBufferLength;

	for (size_t i = BufferLength_; i > 0; i--) Buffer_[i] = Buffer_[i - 1];
	Buffer_[0] = InlineAssembly::ReadPortByte(PS2Controller::PORT_DATA);
	++BufferLength_;
}