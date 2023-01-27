#pragma once
#ifndef PS2_KEYBOARD_HXX
#define PS2_KEYBOARD_HXX

#include "ps2-controller.hxx"

class PS2Keyboard {
	public:
	enum Commands {
		COMMAND_ECHO =							0xEE,
		COMMAND_GET_SET_SCAN_CODE_SET =			0xF0,
		COMMAND_IDENTIFY =						0xF2,
		COMMAND_ENABLE_SCANNING =				0xF4,
		COMMAND_DISABLE_SCANNING =				0xF5,
		COMMAND_RESEND_LAST_BYTE =				0xFE,
		COMMAND_RESET =							0xFF
	};

	enum SubCommands {
		SUB_COMMAND_GET_CURRENT_SCAN_CODE_SET =	0x00,
		SUB_COMMAND_SET_SCAN_CODE_SET_1 =		0x01,
		SUB_COMMAND_SET_SCAN_CODE_SET_2 =		0x02,
		SUB_COMMAND_SET_SCAN_CODE_SET_3 =		0x03
	};

	enum Answers {
		ANSWER_RESET_DONE =						0xAA,
		ANSWER_ECHO =							0xEE,
		ANSWER_ACK =							0xFA,
		ANSWER_RESEND =							0xFE
	};

	enum Types {
		TYPE_AT =								0x00,
		TYPE_MF2 =								0x01,
		TYPE_MF2_WITH_TRANSLATION_ENABLED =		0x02,
		TYPE_UNKNOWN =							0xFF
	};

	enum ScanCodeSets {
		SCAN_CODE_SET_1 =						0x01,
		SCAN_CODE_SET_2 =						0x02,
		SCAN_CODE_SET_3 =						0x03,
		SCAN_CODE_SET_UNKNOWN =					0xFF
	};

	enum ScanCodes {
		SCAN_CODE_UNKNOWN =						0x00,
		SCAN_CODE_F9 =							0x01,
		SCAN_CODE_F5 =							0x03,
		SCAN_CODE_F3 =							0x04,
		SCAN_CODE_F1 =							0x05,
		SCAN_CODE_F2 =							0x06,
		SCAN_CODE_F12 =							0x07,
		SCAN_CODE_F10 =							0x09,
		SCAN_CODE_F8 =							0x0A,
		SCAN_CODE_F6 =							0x0B,
		SCAN_CODE_F4 =							0x0C,
		SCAN_CODE_TABULATION =					0x0D,
		SCAN_CODE_BACK_TICK =					0x0E,
		SCAN_CODE_LEFT_ALT =					0x11,
		SCAN_CODE_LEFT_SHIFT =					0x12,
		SCAN_CODE_LEFT_CONTROL =				0x14,
		SCAN_CODE_Q =							0x15,
		SCAN_CODE_1 =							0x16,
		SCAN_CODE_Z =							0x1A,
		SCAN_CODE_S =							0x1B,
		SCAN_CODE_A =							0x1C,
		SCAN_CODE_W =							0x1D,
		SCAN_CODE_2 =							0x1E,
		SCAN_CODE_C =							0x21,
		SCAN_CODE_X =							0x22,
		SCAN_CODE_D =							0x23,
		SCAN_CODE_E =							0x24,
		SCAN_CODE_4 =							0x25,
		SCAN_CODE_3 =							0x26,
		SCAN_CODE_SPACE =						0x29,
		SCAN_CODE_V =							0x2A,
		SCAN_CODE_F =							0x2B,
		SCAN_CODE_T =							0x2C,
		SCAN_CODE_R =							0x2D,
		SCAN_CODE_5 =							0x2E,
		SCAN_CODE_N =							0x31,
		SCAN_CODE_B =							0x32,
		SCAN_CODE_H =							0x33,
		SCAN_CODE_G =							0x34,
		SCAN_CODE_Y =							0x35,
		SCAN_CODE_6 =							0x36,
		SCAN_CODE_M =							0x3A,
		SCAN_CODE_J =							0x3B,
		SCAN_CODE_U =							0x3C,
		SCAN_CODE_7 =							0x3D,
		SCAN_CODE_8 =							0x3E,
		SCAN_CODE_COMMA =						0x41,
		SCAN_CODE_K =							0x42,
		SCAN_CODE_I =							0x43,
		SCAN_CODE_O =							0x44,
		SCAN_CODE_0 =							0x45,
		SCAN_CODE_9 =							0x46,
		SCAN_CODE_DOT =							0x49,
		SCAN_CODE_FORWARD_SLASH =				0x4A,
		SCAN_CODE_L =							0x4B,
		SCAN_CODE_SEMICOLON =					0x4C,
		SCAN_CODE_P =							0x4D,
		SCAN_CODE_MINUS =						0x4E,
		SCAN_CODE_SIGNLE_QUOTE =				0x52,
		SCAN_CODE_LEFT_SQUARE_BRACKET =			0x54,
		SCAN_CODE_EQUALS =						0x55,
		SCAN_CODE_CAPS_LOCK =					0x58,
		SCAN_CODE_RIGHT_SHIFT =					0x59,
		SCAN_CODE_ENTER =						0x5A,
		SCAN_CODE_RIGHT_SQUARE_BRACKET =		0x5B,
		SCAN_CODE_BACKWARD_SLASH =				0x5D,
		SCAN_CODE_BACKSPACE =					0x66,
		SCAN_CODE_KEYPAD_1 =					0x69,
		SCAN_CODE_KEYPAD_4 =					0x6B,
		SCAN_CODE_KEYPAD_7 =					0x6C,
		SCAN_CODE_KEYPAD_0 =					0x70,
		SCAN_CODE_KEYPAD_DOT =					0x71,
		SCAN_CODE_KEYPAD_2 =					0x72,
		SCAN_CODE_KEYPAD_5 =					0x73,
		SCAN_CODE_KEYPAD_6 =					0x74,
		SCAN_CODE_KEYPAD_8 =					0x75,
		SCAN_CODE_ESCAPE =						0x76,
		SCAN_CODE_NUM_LOCK =					0x77,
		SCAN_CODE_F11 =							0x78,
		SCAN_CODE_KEYPAD_PLUS =					0x79,
		SCAN_CODE_KEYPAD_3 =					0x7A,
		SCAN_CODE_KEYPAD_MINUS =				0x7B,
		SCAN_CODE_KEYPAD_MULTIPLY =				0x7C,
		SCAN_CODE_KEYPAD_9 =					0x7D,
		SCAN_CODE_SCROLL_LOCK =					0x83,
		SCAN_CODE_EXTENDED =					0xE0,
		SCAN_CODE_RELEASED =					0xF0,
		SCAN_CODE_RIGHT_ALT =					0xE011,
		SCAN_CODE_RIGHT_CONTROL =				0xE014,
		SCAN_CODE_LEFT_GUI =					0xE01F,
		SCAN_CODE_VOLUME_DOWN =					0xE021,
		SCAN_CODE_MUTE =						0xE023,
		SCAN_CODE_RIGHT_GUI =					0xE027,
		SCAN_CODE_APPS =						0xE02F,
		SCAN_CODE_VOLUME_UP =					0xE032,
		SCAN_CODE_PLAY_PAUSE =					0xE034,
		SCAN_CODE_ACPI_POWER =					0xE037,
		SCAN_CODE_STOP =						0xE03B,
		SCAN_CODE_ACPI_SLEEP =					0xE03F,
		SCAN_CODE_KEYPAD_FORWARD_SLASH =		0xE04A,
		SCAN_CODE_KEYPAD_ENTER =				0xE05A,
		SCAN_CODE_ACPI_WAKE =					0xE05E,
		SCAN_CODE_END =							0xE069,
		SCAN_CODE_CURSOR_LEFT =					0xE06B,
		SCAN_CODE_HOME =						0xE06C,
		SCAN_CODE_INSERT =						0xE070,
		SCAN_CODE_DELETE =						0xE071,
		SCAN_CODE_CURSOR_DOWN =					0xE072,
		SCAN_CODE_CURSOR_RIGHT =				0xE074,
		SCAN_CODE_CURSOR_UP =					0xE075,
		SCAN_CODE_PAGE_DOWN =					0xE07A,
		SCAN_CODE_PAGE_UP =						0xE07D
	};

	enum ScanCodeMapIndexes {
		SCAN_CODE_MAP_INDEX_NO_SHIFT_NO_CAPS =	0x00,
		SCAN_CODE_MAP_INDEX_SHIFT_NO_CAPS =		0x01,
		SCAN_CODE_MAP_INDEX_NO_SHIFT_CAPS =		0x02,
		SCAN_CODE_MAP_INDEX_SHIFT_CAPS =		0x03
	};

	typedef struct InputResult {
		size_t	ScanCode;
		bool	Released;
		size_t	ScanCodeMapIndex;
	} InputResult, *PInputResult;

	static constexpr const char ScanCodeMapNoShiftNoCapsLock[0x80] = {
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'\t',	'`',	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'q',	'1',	0x00,
		0x00,	0x00,	'z',	's',	'a',	'w',	'2',	0x00,
		0x00,	'c',	'x',	'd',	'e',	'4',	'3',	0x00,
		0x00,	' ',	'v',	'f',	't',	'r',	'5',	0x00,
		0x00,	'n',	'b',	'h',	'g',	'y',	'6',	0x00,
		0x00,	0x00,	'm',	'j',	'u',	'7',	'8',	0x00,
		0x00,	',',	'k',	'i',	'o',	'0',	'9',	0x00,
		0x00,	'.',	'/',	'l',	';',	'p',	'-',	0x00,
		0x00,	0x00,	'\'',	0x00,	'[',	'=',	0x00,	0x00,
		0x00,	0x00,	'\n',	']',	0x00,	'\\',	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	'\b',	0x00,
		0x00,	'1',	0x00,	'4',	'7',	0x00,	0x00,	0x00,
		'0',	'.',	'2',	'5',	'6',	'8',	0x00,	0x00,
		0x00,	'+',	'3',	'-',	'*',	'9',	0x00,	0x00
	};

	static constexpr const char ScanCodeMapShiftNoCapsLock[0x80] = {
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'\t',	'~',	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'Q',	'!',	0x00,
		0x00,	0x00,	'Z',	'S',	'A',	'W',	'@',	0x00,
		0x00,	'C',	'X',	'D',	'E',	'$',	'#',	0x00,
		0x00,	' ',	'V',	'F',	'T',	'R',	'%',	0x00,
		0x00,	'N',	'B',	'H',	'G',	'Y',	'^',	0x00,
		0x00,	0x00,	'M',	'J',	'U',	'&',	'*',	0x00,
		0x00,	'<',	'K',	'I',	'O',	')',	'(',	0x00,
		0x00,	'.',	'/',	'L',	';',	'p',	'-',	0x00,
		0x00,	0x00,	'\"',	0x00,	'{',	'+',	0x00,	0x00,
		0x00,	0x00,	'\n',	'}',	0x00,	'|',	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	'\b',	0x00,
		0x00,	'1',	0x00,	'4',	'7',	0x00,	0x00,	0x00,
		'0',	'.',	'2',	'5',	'6',	'8',	0x00,	0x00,
		0x00,	'+',	'3',	'-',	'*',	'9',	0x00,	0x00
	};

	static constexpr const char ScanCodeMapNoShiftCapsLock[0x80] = {
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'\t',	'`',	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'Q',	'1',	0x00,
		0x00,	0x00,	'Z',	'S',	'A',	'W',	'2',	0x00,
		0x00,	'C',	'X',	'D',	'E',	'4',	'3',	0x00,
		0x00,	' ',	'V',	'F',	'T',	'R',	'5',	0x00,
		0x00,	'N',	'B',	'H',	'G',	'Y',	'6',	0x00,
		0x00,	0x00,	'M',	'J',	'U',	'7',	'8',	0x00,
		0x00,	',',	'K',	'I',	'O',	'0',	'9',	0x00,
		0x00,	'.',	'/',	'L',	';',	'P',	'-',	0x00,
		0x00,	0x00,	'\'',	0x00,	'[',	'=',	0x00,	0x00,
		0x00,	0x00,	'\n',	']',	0x00,	'\\',	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	'\b',	0x00,
		0x00,	'1',	0x00,	'4',	'7',	0x00,	0x00,	0x00,
		'0',	'.',	'2',	'5',	'6',	'8',	0x00,	0x00,
		0x00,	'+',	'3',	'-',	'*',	'9',	0x00,	0x00
	};

	static constexpr const char ScanCodeMapShiftCapsLock[0x80] = {
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'\t',	'~',	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	'q',	'!',	0x00,
		0x00,	0x00,	'z',	's',	'a',	'w',	'@',	0x00,
		0x00,	'c',	'x',	'd',	'e',	'$',	'#',	0x00,
		0x00,	' ',	'v',	'f',	't',	'r',	'%',	0x00,
		0x00,	'n',	'b',	'h',	'g',	'y',	'^',	0x00,
		0x00,	0x00,	'm',	'j',	'u',	'&',	'*',	0x00,
		0x00,	'<',	'k',	'i',	'o',	')',	'(',	0x00,
		0x00,	'>',	'?',	'l',	':',	'p',	'_',	0x00,
		0x00,	0x00,	'\"',	0x00,	'{',	'+',	0x00,	0x00,
		0x00,	0x00,	'\n',	'}',	0x00,	'|',	0x00,	0x00,
		0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	'\b',	0x00,
		0x00,	'1',	0x00,	'4',	'7',	0x00,	0x00,	0x00,
		'0',	'.',	'2',	'5',	'6',	'8',	0x00,	0x00,
		0x00,	'+',	'3',	'-',	'*',	'9',	0x00,	0x00
	};

	static constexpr size_t IRQIndex =			0x01;
	static constexpr size_t MaxBufferLength =	0x80;
	static constexpr size_t ResendTimesCount =	0x05;

	static bool Initialize();
	static uint8_t GetScanCodeSet();
	static bool SetScanCodeSet(size_t index);
	static uint8_t ReadBufferFirstByte();
	static uint8_t ReadBufferLastByte();
	static bool SendByte(uint8_t value);
	static size_t GetBufferLength();

	static bool Echo();
	static uint8_t GetType();
	static bool LockInput();
	static bool UnlockInput();
	static bool Reset();

	static InputResult GetInput();
	static char* GetScanCodeMapByIndex(size_t index);
	static char ReadChar();
	private:
	static void IRQHandler(CPU::PISRData pointer);
	static uint8_t Buffer_[MaxBufferLength];
	static size_t BufferLength_;
	static size_t CurrentScanCodeMapIndex_;
	static bool ShiftPressed_;
	static bool CapsLockPressed_;
};

#endif