#pragma once
#ifndef UMI_FUNCTIONS_HXX		//	usermode interrupt functions
#define UMI_FUNCTIONS_HXX

#include "../common/string-utils.hxx"

#define UMI_KEYBOARD_MAX_INPUT_BUFFER_LENGTH		0x200

class UMIFunctions {
	public:
	//	0
	class Power {
		public:
		enum Functions {
			FUNCTION_SHUTDOWN =	0x00,
			FUNCTION_REBOOT =	0x01,
		};

		static void Shutdown();
		static void Reboot();
	};

	//	1
	class Memory {
		public:
		enum Functions {
			FUNCTION_ALLOCATE =	0x00,
			FUNCTION_FREE =		0x01
		};

		static void* Allocate(size_t count);
		static bool Free(void* memory);
	};

	//	2
	class Terminal {
		public:
		enum Functions {
			FUNCTION_REDRAW =			0x00,
			FUNCTION_GET_OPTION =		0x01,
			FUNCTION_SET_OPTION =		0x02,
			FUNCTION_PUT_CHAR =			0x03,
			FUNCTION_PUT_STRING =		0x04,
			FUNCTION_PRINT_FORMAT =		0x05,
			FUNCTION_PRINT_HEX_DATA =	0x06
		};

		enum OptionIndexes {
			OPTION_COLUMN =				0x00,
			OPTION_LINE =				0x01,
			OPTION_COLOR =				0x02
		};

		static void Redraw();										//	CX=0
		static size_t GetOption(size_t optionIndex);				//	CX=1|DX=optionIndex
		static void SetOption(size_t optionIndex, size_t value);	//	CX=2|DX=optionIndex|BX=value
		static void PutChar(char character);						//	CX=3|DX=character
		static void PutString(const char* source);					//	CX=4|DX=sourceAddress
		static void PrintFormat(const char* format, ...);			//	CX=5|DX=format & argumentsAddress
		static void PrintHexData(const puint8_t data, size_t bytesCount, size_t valuesPerLine = 10);
	};
	
	//	3
	class Keyboard {
		public:
		enum Functions {
			FUNCTION_READ_INPUT =	0x00,
			FUNCTION_WAIT_INPUT =	0x01
		};
		
		static void ReadInput(char* buffer, size_t bufferLength = UMI_KEYBOARD_MAX_INPUT_BUFFER_LENGTH);
		static void WaitInput();
	};

	//	4
	class FileSystem {
		public:
		static constexpr size_t MaxEntryNameLength =		0x100;

		enum Functions {
			FUNCTION_GET_CURRENT_PATH =						0x00,
			FUNCTION_SET_CURRENT_PATH =						0x01,
			FUNCTION_CREATE_DIRECTORY_ITERATOR =			0x02,
			FUNCTION_GET_NEXT_DIRECTORY_ITERATOR_OBJECT =	0x03,
			FUNCTION_GET_OBJECT_INFORMATION =				0x04,
			FUNCTION_READ_FILE =							0x05
		};

		enum ObjectTypes {
			OBJECT_TYPE_FILE =								0x00,
			OBJECT_TYPE_DIRECTORY =							0x01,
			OBJECT_TYPE_SYMBOLIC_LINK =						0x02
		};

		enum ObjectAttributes {
			OBJECT_ATTRIBUTE_SYSTEM =							0x01,
			OBJECT_ATTRIBUTE_READ_ONLY =						0x02,
			OBJECT_ATTRIBUTE_HIDDEN =							0x04
		};

		typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
			char		Name[MaxEntryNameLength];
			uint8_t		Type;
			uint64_t	Attributes;
			uint64_t	Size;
			uint32_t	CreationDate;
			uint32_t	LastReadDate;
			uint32_t	LastWriteDate;
			uint32_t	CreationTime;
			uint32_t	LastReadTime;
			uint32_t	LastWriteTime;
		} ObjectInformation, *PObjectInformation;

		typedef struct DEFINE_SPECIAL(PACKED_DEFINITION) {
			size_t				StorageDeviceIndex;
			size_t				PartitionIndex;
			uint64_t			DirectoryID;
			uint64_t			SectorOffset;
			uint64_t			ObjectsLeft;
		} DirectoryIterator, *PDirectoryIterator;

		static char* GetCurrentPath();
		static bool SetCurrentPath(const char* currentPath);
		
		static bool CreateDirectoryIterator(const char* directoryPath, DirectoryIterator& directoryIterator);
		static bool GetNextDirectoryIteratorObject(DirectoryIterator& directoryIterator, ObjectInformation& objectInformation);

		static ObjectInformation GetObjectInformation(const char* objectPath);
		static puint8_t ReadFile(const char* filePath, size_t* readedBytesCount);
	};

	//	5
	class Thread {
		public:
		enum Functions {
			FUNCTION_GET_WORKED_TICKS_COUNT =	0
		};

		static size_t GetWorkedTicksCount();
	};

	enum ServiceIndexes {
		SERVICE_POWER =					0x00,
		SERVICE_MEMORY =				0x01,
		SERVICE_TELETYPE =				0x02,
		SERVICE_KEYBOARD =				0x03,
		SERVICE_FILESYSTEM =			0x04,
		SERVICE_THREAD =				0x05
	};
};

#endif