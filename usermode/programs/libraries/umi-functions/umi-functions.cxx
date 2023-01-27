#include "umi-functions.hxx"

void UMIFunctions::PowerManagement::Shutdown() {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_POWER_MANAGEMENT), "c"(FUNCTION_SHUTDOWN));
}

void UMIFunctions::PowerManagement::Reboot() {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_POWER_MANAGEMENT), "c"(FUNCTION_REBOOT));
}

void* UMIFunctions::Memory::Allocate(size_t count) {
	size_t result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(SERVICE_MEMORY), "c"(FUNCTION_ALLOCATE), "d"(count));
	return (void*)result;
}

bool UMIFunctions::Memory::Free(void* memory) {
	size_t result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(SERVICE_MEMORY), "c"(FUNCTION_FREE), "d"(memory));
	return result == 1;
}

void UMIFunctions::Terminal::Redraw() {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_TELETYPE), "c"(FUNCTION_REDRAW));
}

size_t UMIFunctions::Terminal::GetOption(size_t optionIndex) {
	size_t result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(SERVICE_TELETYPE), "c"(FUNCTION_GET_OPTION), "d"(optionIndex));
	return result;
}

void UMIFunctions::Terminal::SetOption(size_t optionIndex, size_t value) {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_TELETYPE), "c"(FUNCTION_SET_OPTION), "d"(optionIndex), "b"(value));
}

void UMIFunctions::Terminal::PutChar(char character) {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_TELETYPE), "c"(FUNCTION_PUT_CHAR), "d"(character));
}

void UMIFunctions::Terminal::PutString(const char* source) {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_TELETYPE), "c"(FUNCTION_PUT_STRING), "d"(source));
}

void UMIFunctions::Terminal::PrintFormat(const char* format, ...) {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_TELETYPE), "c"(FUNCTION_PRINT_FORMAT), "d"((size_t)&format));
}

void UMIFunctions::Terminal::PrintHexData(const puint8_t data, size_t bytesCount, size_t valuesPerLine) {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_TELETYPE), "c"(FUNCTION_PRINT_HEX_DATA), "d"(data), "b"(bytesCount), "S"(valuesPerLine));
}

void UMIFunctions::Keyboard::ReadInput(char* buffer, size_t bufferLength) {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_KEYBOARD), "c"(FUNCTION_READ_INPUT), "d"(buffer), "b"(bufferLength));
}

void UMIFunctions::Keyboard::WaitInput() {
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_KEYBOARD), "c"(FUNCTION_WAIT_INPUT));
}

char* UMIFunctions::FileSystem::GetCurrentPath() {
	size_t bufferAddress;
	__asm__ __volatile__("int $0xEC":"=a"(bufferAddress):"a"(SERVICE_FILESYSTEM), "c"(FUNCTION_GET_CURRENT_PATH));
	size_t strLen = StringUtils::GetLength((char*)bufferAddress);
	char* newBuffer = new char[strLen + 1];
	MemoryUtils::Copy(newBuffer, (void*)bufferAddress, strLen + 1);
	return (char*)newBuffer;
}

bool UMIFunctions::FileSystem::SetCurrentPath(const char* currentPath) {
	size_t result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(SERVICE_FILESYSTEM), "c"(FUNCTION_SET_CURRENT_PATH), "d"(currentPath));
	return result == 1;
}

bool UMIFunctions::FileSystem::CreateDirectoryIterator(const char* directoryPath, DirectoryIterator& directoryIterator) {
	size_t result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(SERVICE_FILESYSTEM), "c"(FUNCTION_CREATE_DIRECTORY_ITERATOR), "d"(directoryPath), "b"((size_t)&directoryIterator));
	return result == 1;
}

bool UMIFunctions::FileSystem::GetNextDirectoryIteratorObject(DirectoryIterator& directoryIterator, ObjectInformation& objectInformation) {
	size_t result;
	__asm__ __volatile__(
		"int $0xEC":
		"=a"(result):
		"a"(SERVICE_FILESYSTEM), "c"(FUNCTION_GET_NEXT_DIRECTORY_ITERATOR_OBJECT), "d"((size_t)&directoryIterator), "b"((size_t)&objectInformation)
	);
	return result == 1;
}

UMIFunctions::FileSystem::ObjectInformation UMIFunctions::FileSystem::GetObjectInformation(const char* objectPath) {
	ObjectInformation result;
	__asm__ __volatile__("int $0xEC"::"a"(SERVICE_FILESYSTEM), "c"(FUNCTION_GET_OBJECT_INFORMATION), "d"(objectPath), "b"((size_t)&result));
	return result;
}

puint8_t UMIFunctions::FileSystem::ReadFile(const char* filePath, size_t* readedBytesCount) {
	size_t fileContentAddress, fileContentSize;
	__asm__ __volatile__("int $0xEC":"=a"(fileContentAddress), "=c"(fileContentSize):"a"(SERVICE_FILESYSTEM), "c"(FUNCTION_READ_FILE), "d"((size_t)filePath));
	*readedBytesCount = fileContentSize;
	return (puint8_t)fileContentAddress;
}

#ifdef __cplusplus
void* operator new(size_t bytesCount) {
	void* result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(UMIFunctions::SERVICE_MEMORY), "c"(0), "d"(bytesCount));
	return result;
}
 
void* operator new[](size_t bytesCount) {
	void* result;
	__asm__ __volatile__("int $0xEC":"=a"(result):"a"(UMIFunctions::SERVICE_MEMORY), "c"(0), "d"(bytesCount));
	return result;
}
 
void operator delete(void* memory) {
	__asm__ __volatile__("int $0xEC"::"a"(UMIFunctions::SERVICE_MEMORY), "c"(1), "d"(memory));
}
 
void operator delete[](void* memory) {
	__asm__ __volatile__("int $0xEC"::"a"(UMIFunctions::SERVICE_MEMORY), "c"(1), "d"(memory));
}

void operator delete(void* memory, size_t) {
	__asm__ __volatile__("int $0xEC"::"a"(UMIFunctions::SERVICE_MEMORY), "c"(1), "d"(memory));
}

void operator delete [](void* memory, size_t) {
	__asm__ __volatile__("int $0xEC"::"a"(UMIFunctions::SERVICE_MEMORY), "c"(1), "d"(memory));
}
#endif