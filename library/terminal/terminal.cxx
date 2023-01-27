#include "terminal.hxx"

puint8_t Terminal::Buffer_ = (puint8_t)Terminal::BufferAddress;
uint16_t Terminal::Position_ = 0;
uint8_t Terminal::Color_ = Terminal::COLOR_LIGHT_GRAY;
char Terminal::NumberConversionBuffer_[65];

/**
 * @param color [in] color to redraw screen
*/
void Terminal::Redraw(
	uint8_t color
) {
	for (size_t i = 0; i < BufferSize;) {
		Buffer_[i++] = ' ';
		Buffer_[i++] = color;
	}
}

void Terminal::RaiseLastLine() {
	MemoryUtils::Copy(Buffer_, (void*)(BufferAddress + LineSize), BufferSize - LineSize);
	MemoryUtils::Fill((void*)(BufferAddress + BufferSize - LineSize), 0, LineSize);
}

void Terminal::LowerLastLine() {
	size_t sourceOffset = BufferSize - (LineSize << 1);
	size_t destinationOffset = BufferSize - LineSize;

	while (destinationOffset) {
		MemoryUtils::Copy(&Buffer_[destinationOffset], &Buffer_[sourceOffset], LineSize);
		sourceOffset -= LineSize;
		destinationOffset -= LineSize;
	}

	MemoryUtils::Fill(&Buffer_[destinationOffset], 0, LineSize);
}

/**
 * @return terminal cursor column
*/
uint8_t Terminal::GetColumn() {
	return (uint8_t)((Position_ % LineSize) >> 1);
}

/**
 * @return terminal cursor line
*/
uint8_t Terminal::GetLine() {
	return (uint8_t)(Position_ / LineSize);
}

/**
 * @return terminal selected font color
*/
uint8_t Terminal::GetColor() {
	return Color_;
}

/**
 * @param index [in] column index
 * @return true if the column < Width, otherwise false
*/
bool Terminal::SetColumn(
	uint8_t index
) {
	if (index < Width) {
		Position_ = (uint16_t)(Position_ - (Position_ % LineSize) + (index << 1));
		return true;
	}

	return false;
}

/**
 * @param index [in] line index
 * @return true if the line < Height, otherwise false
*/
bool Terminal::SetLine(
	uint8_t index
) {
	if (index < Height) {
		Position_ = (uint16_t)((Position_ % LineSize) + index * LineSize);
		return true;
	}

	return false;
}

/**
 * @param color [in] terminal font color
*/
void Terminal::SetColor(
	uint8_t color
) {
	Color_ = color;
}

/**
 * @param character character to put
*/
void Terminal::PutChar(
	char character
) {
	if (character == SYMBOL_TABULATION) {
		Position_ += (uint16_t)((TabulationLength - (((Position_ % LineSize) >> 1) % TabulationLength)) << 1);
		if (Position_ >= BufferSize) {
			RaiseLastLine();
			Position_ = (Height - 1) * LineSize;
		}
	}
	else if (character == SYMBOL_NEW_LINE) {
		Position_ += LineSize;
		if (Position_ >= BufferSize) {
			RaiseLastLine();
			Position_ = (Height - 1) * LineSize;
		}
	}
	else if (character == SYMBOL_CARRET) Position_ = (uint16_t)(Position_ - (Position_ % LineSize));
	else if (character == SYMBOL_BACKSPACE) {
		if (Position_) Position_ -= 2;
	}
	else {
		Buffer_[Position_] = (uint8_t)character;
		Buffer_[Position_ + 1] = Color_;
		if (Position_ < BufferSize) Position_ += 2;
		else {
			RaiseLastLine();
			Position_ = (Height - 1) * LineSize;
		}
	}
}

/**
 * @param string string to put
*/
void Terminal::PutString(
	const char* source
) {
	for (size_t i = 0; source[i]; i++) PutChar(source[i]);
}

/**
 * @param format the format of the string to be output
 * @param __VA_ARGS__ fomrat arguments
*/
void Terminal::PrintFormat(
	const char* format,
	...
) {
	psize_t argument = (psize_t)(size_t)&format;
	const uint8_t currentColor = Color_;
	++argument;

	for (size_t i = 0; format[i]; i++) {
		if (format[i] == '%') {
			i += 1;
			if (format[i] == '%') PutChar('%');
			else if (format[i] == 'a') {
				i += 1;
				if (format[i] >= '0' && format[i] <= '9') Color_ = (format[i] - '0') & 0x0F;
				else if (format[i] >= 'a' && format[i] <= 'f') Color_ = (format[i] - 'a' + 10) & 0x0F;
				else if (format[i] >= 'A' && format[i] <= 'F') Color_ = (format[i] - 'A' + 10) & 0x0F;
				Color_ <<= 4;
				i += 1;
				if (format[i] >= '0' && format[i] <= '9') Color_ |= (format[i] - '0') & 0x0F;
				else if (format[i] >= 'a' && format[i] <= 'f') Color_ |= (format[i] - 'a' + 10) & 0x0F;
				else if (format[i] >= 'A' && format[i] <= 'F') Color_ |= (format[i] - 'A' + 10) & 0x0F;
			}
			else if (format[i] == 'A') {
				Color_ = (uint8_t)*argument;
				++argument;
			}
			else if (format[i] == 'c') {
				PutChar((char)*argument);
				++argument;
			}
			else if (format[i] == 's') {
				PutString((char*)*argument);
				++argument;
			}
			else if (format[i] == 'd') {
				StringUtils::GetString(NumberConversionBuffer_, (ptrdiff_t)*argument, 10);
				PutString(NumberConversionBuffer_);
				++argument;
			}
			else if (format[i] == 'u') {
				StringUtils::GetString(NumberConversionBuffer_, (size_t)*argument, 10);
				PutString(NumberConversionBuffer_);
				++argument;
			}
			else if (format[i] == 'x') {
				StringUtils::GetString(NumberConversionBuffer_, (size_t)*argument, 16);
				PutString(NumberConversionBuffer_);
				++argument;

			}
			else if (format[i] == 'o') {
				StringUtils::GetString(NumberConversionBuffer_, (size_t)*argument, 8);
				PutString(NumberConversionBuffer_);
				++argument;
			}
			else if (format[i] == 'b') {
				StringUtils::GetString(NumberConversionBuffer_, (size_t)*argument, 2);
				PutString(NumberConversionBuffer_);
				++argument;
			}
		}
		else PutChar(format[i]);
	}

	Color_ = currentColor;
}

/*
	@return If template valid - length, otherwise - zero
*/
size_t Terminal::PrintFormatTemplate(const char* format, ...) {
	size_t templateLength = 0;
	if (format[0] != '%') return templateLength;
	++templateLength;
	char* ptr = (char*)&format[templateLength];
	size_t argument = *(psize_t)((size_t)&format + sizeof(size_t));

	if (!*ptr) {
		PutChar('%');
		return templateLength;
	}

	if (*ptr == '%') PutChar('%');
	else if (*ptr == 'a') {
		++templateLength;
		++ptr;
		if (!*ptr) return 0;
		if (*ptr >= '0' && *ptr <= '9') Color_ = (*ptr - '0') & 0x0F;
		else if (*ptr >= 'a' && *ptr <= 'f') Color_ = (*ptr - 'a' + 10) & 0x0F;
		else if (*ptr >= 'A' && *ptr <= 'F') Color_ = (*ptr - 'A' + 10) & 0x0F;
		Color_ <<= 4;
		++templateLength;
		++ptr;
		if (!*ptr) return 0;
		if (*ptr >= '0' && *ptr <= '9') Color_ |= (*ptr - '0') & 0x0F;
		else if (*ptr >= 'a' && *ptr <= 'f') Color_ |= (*ptr - 'a' + 10) & 0x0F;
		else if (*ptr >= 'A' && *ptr <= 'F') Color_ |= (*ptr - 'A' + 10) & 0x0F;
	}
	else if (*ptr == 'A') {
		Color_ = (uint8_t)argument;
	}
	else if (*ptr == 'c') {
		PutChar((char)argument);
	}
	else if (*ptr == 's') {
		PutString((char*)argument);
	}
	else if (*ptr == 'd') {
		StringUtils::GetString(NumberConversionBuffer_, (ptrdiff_t)argument, 10);
		PutString(NumberConversionBuffer_);
	}
	else if (*ptr == 'u') {
		StringUtils::GetString(NumberConversionBuffer_, (size_t)argument, 10);
		PutString(NumberConversionBuffer_);
	}
	else if (*ptr == 'x') {
		StringUtils::GetString(NumberConversionBuffer_, (size_t)argument, 16);
		PutString(NumberConversionBuffer_);
	}
	else if (*ptr == 'o') {
		StringUtils::GetString(NumberConversionBuffer_, (size_t)argument, 8);
		PutString(NumberConversionBuffer_);
	}
	else if (*ptr == 'b') {
		StringUtils::GetString(NumberConversionBuffer_, (size_t)argument, 2);
		PutString(NumberConversionBuffer_);
	}

	return templateLength + 1;
}

void Terminal::PrintHexData(const void* hexData, size_t bytesCount, size_t valuesPerLine) {
	puint8_t buffer = (puint8_t)hexData;
	for (; bytesCount;) {
		for (size_t j = 0; j < valuesPerLine && bytesCount; j++) {
			if (*buffer < 0x10) PutChar('0');
			PrintFormat("%x", *buffer);
			PutChar('\t');
			--bytesCount;
			++buffer;
		}
		if (bytesCount) PutString("\r\n");
	}
}

/**
 * PS/2 Keyboard MUST BE INITIALIZED BEFORE CALL!
*/
uint8_t Terminal::ReadChar() {
	char character = 0x00, tmp;

	do {
		tmp = PS2Keyboard::ReadChar();
		if (tmp == SYMBOL_NEW_LINE && character) break;
		else if (tmp == SYMBOL_BACKSPACE) {
			if (Position_) {
				Position_ -= 2;
				PutChar(' ');
				Position_ -= 2;
			}
			character = 0x00;
		}
		else if (!character) {
			character = tmp;
			PutChar(character);
		}
	} while (true);

	PutString("\r\n");
	return character;
}

/**
 * PS/2 Keyboard MUST BE INITIALIZED BEFORE CALL!
*/
void Terminal::ReadString(char* buffer, size_t maxLength) {
	if (!maxLength) maxLength = WRONG_INDEX;
	ptrdiff_t length = 0;
	char character = 0x00;

	while (true) {
		character = PS2Keyboard::ReadChar();
		if (character == SYMBOL_BACKSPACE && length) {
			buffer[length--] = ' ';
			if (Position_) {
				Position_ -= 2;
				PutChar(' ');
				Position_ -= 2;
			}
		}
		else if (character == SYMBOL_NEW_LINE) {
			buffer[length] = 0;
			break;
		}
		else if (character != SYMBOL_BACKSPACE && (size_t)length < maxLength) {
			buffer[length++] = character;
			PutChar(character);
		}
	}

	PutString("\r\n");
}