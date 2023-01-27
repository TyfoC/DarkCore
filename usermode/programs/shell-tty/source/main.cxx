#include <umi-functions/umi-functions.hxx>
#include <common/string-utils.hxx>

using PowerManagement = UMIFunctions::PowerManagement;
using Memory = UMIFunctions::Memory;
using Terminal = UMIFunctions::Terminal;
using Keyboard = UMIFunctions::Keyboard;
using FileSystem = UMIFunctions::FileSystem;

#define UTILITY_VERSION_MAJOR			0
#define UTILITY_VERSION_MINOR			1

#define COMMAND_INDEX_HELP				0
#define COMMAND_INDEX_CLEAR				1
#define COMMAND_INDEX_ECHO				2
#define COMMAND_INDEX_TEST_ALLOCATION	3
#define COMMAND_INDEX_SHUTDOWN			4
#define COMMAND_INDEX_REBOOT			5
#define COMMAND_INDEX_GET_CURRENT_PATH	6
#define COMMAND_INDEX_SET_CURRENT_PATH	7
#define COMMAND_INDEX_GET_DIR_OBJECTS	8
#define COMMAND_INDEX_GET_OBJECT_INFO	9
#define COMMAND_INDEX_READ_TEXT_FILE	10
#define COMMAND_INDEX_READ_HEX_FILE		11

char UserInputBuffer[0x200];
const char* Commands[] = {
	"help", "clear", "echo",
	"vmtest",
	"shutdown", "reboot",
	"path", "cd", "ls", "info", "txtread", "hexread"
};

void PrintHelp() {
	Terminal::PutString("Supported commands:\r\n");
	Terminal::PrintFormat("\t%s - get info about commands\r\n", Commands[COMMAND_INDEX_HELP]);
	Terminal::PrintFormat("\t%s - clear terminal\r\n", Commands[COMMAND_INDEX_CLEAR]);
	Terminal::PrintFormat("\t%s <text> - echo some text\r\n", Commands[COMMAND_INDEX_ECHO]);
	Terminal::PrintFormat("\t%s - test VMM memory allocation\r\n", Commands[COMMAND_INDEX_TEST_ALLOCATION]);
	Terminal::PrintFormat("\t%s - shutdown\r\n", Commands[COMMAND_INDEX_SHUTDOWN]);
	Terminal::PrintFormat("\t%s - reboot\r\n", Commands[COMMAND_INDEX_REBOOT]);
	Terminal::PrintFormat("\t%s - get current path\r\n", Commands[COMMAND_INDEX_GET_CURRENT_PATH]);
	Terminal::PrintFormat("\t%s <path> - set current path\r\n", Commands[COMMAND_INDEX_SET_CURRENT_PATH]);
	Terminal::PrintFormat("\t%s - show objects in current directory\r\n", Commands[COMMAND_INDEX_GET_DIR_OBJECTS]);
	Terminal::PrintFormat("\t%s <path> - show file system object information\r\n", Commands[COMMAND_INDEX_GET_OBJECT_INFO]);
	Terminal::PrintFormat("\t%s <path> - show txt file content\r\n", Commands[COMMAND_INDEX_READ_TEXT_FILE]);
	Terminal::PrintFormat("\t%s <path> - show hex file content\r\n", Commands[COMMAND_INDEX_READ_HEX_FILE]);
}

EXTERN_C void main() {
	Terminal::PrintFormat("Welcome to DarkCore Shell v%u.%u\r\n", UTILITY_VERSION_MAJOR, UTILITY_VERSION_MINOR);
	while (1) {
		Terminal::PutString("# ");
		Keyboard::ReadInput(UserInputBuffer, 0x200 - 1);

		if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_HELP])) PrintHelp();
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_CLEAR])) {
			Terminal::Redraw();
			Terminal::SetOption(Terminal::OPTION_COLUMN, 0);
			Terminal::SetOption(Terminal::OPTION_LINE, 0);
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_ECHO])) {
			size_t firstCharPos = 0;
			while (UserInputBuffer[firstCharPos] != ' ' && UserInputBuffer[firstCharPos] != '\t' && UserInputBuffer[firstCharPos]) ++firstCharPos;
			while ((UserInputBuffer[firstCharPos] == ' ' || UserInputBuffer[firstCharPos] == '\t') && UserInputBuffer[firstCharPos]) ++firstCharPos;
			Terminal::PutString(&UserInputBuffer[firstCharPos]);
			Terminal::PutString("\r\n");
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_TEST_ALLOCATION])) {
			puint8_t data = new uint8_t[0x2000];
			Terminal::PrintFormat("Memory allocated: %s\r\n", (size_t)data ? "yes" : "no");
			Terminal::PrintFormat("Memory freed: %s\r\n", Memory::Free(data) ? "yes" : "no");
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_SHUTDOWN])) PowerManagement::Shutdown();
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_REBOOT])) PowerManagement::Reboot();
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_GET_CURRENT_PATH])) {
			char* currentPath = FileSystem::GetCurrentPath();
			Terminal::PrintFormat("Current path: `%s`\r\n", currentPath);
			delete[] currentPath;
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_SET_CURRENT_PATH])) {
			size_t firstCharPos = 0;
			while (UserInputBuffer[firstCharPos] != ' ' && UserInputBuffer[firstCharPos] != '\t' && UserInputBuffer[firstCharPos]) ++firstCharPos;
			while ((UserInputBuffer[firstCharPos] == ' ' || UserInputBuffer[firstCharPos] == '\t') && UserInputBuffer[firstCharPos]) ++firstCharPos;
			if (UserInputBuffer[firstCharPos]) {
				if (!FileSystem::SetCurrentPath(&UserInputBuffer[firstCharPos])) Terminal::PrintFormat("%a0CError: invalid directory path!\r\n");
			}
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_GET_DIR_OBJECTS])) {
			char* currentPath = FileSystem::GetCurrentPath();
			FileSystem::DirectoryIterator directoryIterator = {};
			if (!FileSystem::CreateDirectoryIterator(currentPath, directoryIterator)) Terminal::PutString("Error: cannot create directory iterator!\r\n");
			else {
				FileSystem::ObjectInformation objectInformation = {};
				while (FileSystem::GetNextDirectoryIteratorObject(directoryIterator, objectInformation)) {
					Terminal::PutString(objectInformation.Name);
					if (objectInformation.Type == FileSystem::OBJECT_TYPE_FILE) Terminal::PrintFormat(" - file (%u bytes)", (size_t)objectInformation.Size);
					else if (objectInformation.Type == FileSystem::OBJECT_TYPE_DIRECTORY) Terminal::PrintFormat(" - directory (%u elements)", (size_t)objectInformation.Size);
					else if (objectInformation.Type == FileSystem::OBJECT_TYPE_SYMBOLIC_LINK) Terminal::PutString(" - symbolic link");

					if (objectInformation.Attributes) {
						Terminal::PutString(" [");
						if (objectInformation.Attributes & FileSystem::OBJECT_ATTRIBUTE_SYSTEM) Terminal::PutString("SYSTEM ");
						if (objectInformation.Attributes & FileSystem::OBJECT_ATTRIBUTE_READ_ONLY) Terminal::PutString("READ_ONLY ");
						if (objectInformation.Attributes & FileSystem::OBJECT_ATTRIBUTE_HIDDEN) Terminal::PutString("HIDDEN ");
						Terminal::PutString("\b]");
					}

					Terminal::PutString("\r\n");
				}
			}

			delete[] currentPath;
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_GET_OBJECT_INFO])) {
			size_t firstCharPos = 0;
			while (UserInputBuffer[firstCharPos] != ' ' && UserInputBuffer[firstCharPos] != '\t' && UserInputBuffer[firstCharPos]) ++firstCharPos;
			while ((UserInputBuffer[firstCharPos] == ' ' || UserInputBuffer[firstCharPos] == '\t') && UserInputBuffer[firstCharPos]) ++firstCharPos;
			if (UserInputBuffer[firstCharPos]) {
				FileSystem::ObjectInformation objectInformation = FileSystem::GetObjectInformation(&UserInputBuffer[firstCharPos]);

				if (objectInformation.Name[0]) {
					Terminal::PrintFormat("Name: `%s`\r\n", objectInformation.Name);
					Terminal::PrintFormat("Type: %s\r\n", objectInformation.Type == FileSystem::OBJECT_TYPE_FILE ? "file" : "directory");

					if (objectInformation.Attributes) {
						Terminal::PrintFormat("Attributes: ");
						Terminal::PutString(" [");
						if (objectInformation.Attributes & FileSystem::OBJECT_ATTRIBUTE_SYSTEM) Terminal::PutString("SYSTEM ");
						if (objectInformation.Attributes & FileSystem::OBJECT_ATTRIBUTE_READ_ONLY) Terminal::PutString("READ_ONLY ");
						if (objectInformation.Attributes & FileSystem::OBJECT_ATTRIBUTE_HIDDEN) Terminal::PutString("HIDDEN ");
						Terminal::PutString("\b]");
					}

					Terminal::PutString("\r\n");

					if (objectInformation.Type == FileSystem::OBJECT_TYPE_FILE) Terminal::PrintFormat("Size: %u bytes\r\n", objectInformation.Size);
					else Terminal::PrintFormat("Files & Folders Count: %u\r\n", objectInformation.Size);
					Terminal::PrintFormat("Creation Date & Time: %x %x\r\n", objectInformation.CreationDate, objectInformation.CreationTime);
					Terminal::PrintFormat("Last Read Date & Time: %x %x\r\n", objectInformation.LastReadDate, objectInformation.LastReadTime);
					Terminal::PrintFormat("Last Write Date & Time: %x %x\r\n", objectInformation.LastWriteDate, objectInformation.LastWriteTime);
				}
				else Terminal::PrintFormat("%a0CError: cannot get information about %a0E`%s`%a0C!\r\n", &UserInputBuffer[firstCharPos]);

				MemoryUtils::Fill(&objectInformation, 0, sizeof(FileSystem::ObjectInformation));
			}
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_READ_TEXT_FILE])) {
			size_t firstCharPos = 0;
			while (UserInputBuffer[firstCharPos] != ' ' && UserInputBuffer[firstCharPos] != '\t' && UserInputBuffer[firstCharPos]) ++firstCharPos;
			while ((UserInputBuffer[firstCharPos] == ' ' || UserInputBuffer[firstCharPos] == '\t') && UserInputBuffer[firstCharPos]) ++firstCharPos;
			if (UserInputBuffer[firstCharPos]) {
				size_t fileContentLength;
				puint8_t buffer = FileSystem::ReadFile(&UserInputBuffer[firstCharPos], &fileContentLength);
				if (!buffer)  Terminal::PrintFormat("%a0CError: cannot read file!\r\n");
				else {
					for (size_t i = 0; i < fileContentLength; i++) Terminal::PutChar(buffer[i]);
					delete[] buffer;
					Terminal::PutString("\r\n");
				}
			}
		}
		else if (!StringUtils::FindFirstSubstr(UserInputBuffer, Commands[COMMAND_INDEX_READ_HEX_FILE])) {
			size_t firstCharPos = 0;
			while (UserInputBuffer[firstCharPos] != ' ' && UserInputBuffer[firstCharPos] != '\t' && UserInputBuffer[firstCharPos]) ++firstCharPos;
			while ((UserInputBuffer[firstCharPos] == ' ' || UserInputBuffer[firstCharPos] == '\t') && UserInputBuffer[firstCharPos]) ++firstCharPos;
			if (UserInputBuffer[firstCharPos]) {
				size_t fileContentLength;
				puint8_t buffer = FileSystem::ReadFile(&UserInputBuffer[firstCharPos], &fileContentLength);
				if (!buffer)  Terminal::PrintFormat("%a0CError: cannot read file!\r\n");
				else {
					Terminal::PrintHexData(buffer, fileContentLength);
					delete[] buffer;
					Terminal::PutString("\r\n");
				}
			}
		}
		else if (UserInputBuffer[0]) Terminal::PrintFormat(
			"%a0CError: unknown command: `%s` %a07(Type `%a0A%s`%a07 to get info)%a0C!\r\n",
			UserInputBuffer, Commands[COMMAND_INDEX_HELP]
		);
	}
}