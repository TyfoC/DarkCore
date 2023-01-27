#pragma once
#ifndef STORAGE_DEVICES_HXX
#define STORAGE_DEVICES_HXX

#include "storage-device.hxx"
#include "../common/list.hxx"

class StorageDevices {
	public:
	static void Initialize();
	static size_t GetCount();

	static bool Append(const IDEControllerChannel::Device& ideCCDevice);
	static StorageDevice& Get(size_t index);

	static StorageDevice* GetArray();
	static List<IDEControllerChannel::Device>* GetIDECCDevicesList();
	private:
	static List<IDEControllerChannel::Device>*	IDECCDevices_;
	static StorageDevice						Devices_[];
	static size_t								DevicesCount_;
};

#endif