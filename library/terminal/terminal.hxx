#pragma once
#ifndef TERMINAL_H
#define TERMINAL_H

#include "../common/memory-utils.hxx"
#include "../common/string-utils.hxx"
#include "../ps2/ps2-keyboard.hxx"

class Terminal {
	public:
	enum Colors {
		COLOR_BLACK =								0x00,
		COLOR_BLUE =								0x01,
		COLOR_GREEN =								0x02,
		COLOR_CYAN =								0x03,
		COLOR_RED =									0x04,
		COLOR_MAGENTA =								0x05,
		COLOR_BROWN =								0x06,
		COLOR_LIGHT_GRAY =							0x07,
		COLOR_DARK_GRAY =							0x08,
		COLOR_LIGHT_BLUE =							0x09,
		COLOR_LIGHT_GREEN =							0x0A,
		COLOR_LIGHT_CYAN =							0x0B,
		COLOR_LIGHT_RED =							0x0C,
		COLOR_LIGHT_MAGENTA =						0x0D,
		COLOR_YELLOW =								0x0E,
		COLOR_WHITE =								0x0F
	};

	enum Symbols {
		SYMBOL_TABULATION =							'\t',
		SYMBOL_BACKSPACE =							'\b',
		SYMBOL_NEW_LINE =							'\n',
		SYMBOL_CARRET =								'\r'
	};

	static constexpr size_t BufferAddress =			0x000B8000;
	static constexpr size_t BufferSize =			0xFA0;
	static constexpr size_t LineSize =				0xA0;
	static constexpr size_t Width =					0x50;
	static constexpr size_t Height =				0x19;
	static constexpr size_t TabulationLength =		0x04;

	static void Redraw(uint8_t color);
	static void RaiseLastLine();
	static void LowerLastLine();
	static uint8_t GetColumn();
	static uint8_t GetLine();
	static uint8_t GetColor();
	static bool SetColumn(uint8_t index);
	static bool SetLine(uint8_t index);
	static void SetColor(uint8_t color);
	static void PutChar(char character);
	static void PutString(const char* source);
	static void PrintFormat(const char* format, ...);
	static size_t PrintFormatTemplate(const char* format, ...);
	static void PrintHexData(const void* hexData, size_t bytesCount, size_t valuesPerLine);
	static uint8_t ReadChar();
	static void ReadString(char* buffer, size_t maxLength = 0);
	private:
	static puint8_t Buffer_;
	static uint16_t Position_;
	static uint8_t Color_;
	static char NumberConversionBuffer_[65];
};

#endif