#pragma once
#ifndef STORAGE_DEVICE_HXX
#define STORAGE_DEVICE_HXX

#include "../ide/ide.hxx"

class StorageDevice {
	public:
	static constexpr size_t MaxDevicesCount = 'Z' - 'A' + 1;

	enum Types {
		TYPE_UNKNOWN =							0x00,
		TYPE_IDE_CONTROLLER_CHANNEL_DEVICE =	0x01,
	};

	StorageDevice();
	StorageDevice(const IDEControllerChannel::Device& ideCCDev);
	~StorageDevice();

	StorageDevice& operator()(const IDEControllerChannel::Device& ideCCDev);

	size_t GetType();
	void* GetData();

	size_t GetSectorsCount();
	void GetModelName(void* buffer);

	bool ReadSectors(void* destination, size_t lbaOffset, size_t sectorsCount);
	bool WriteSectors(const void* source, size_t lbaOffset, size_t sectorsCount);

	static char GetLetterByIndex(size_t index);
	static size_t GetIndexByLetter(char letter);
	private:
	void*	Data_;
	size_t	Type_;
};

#endif