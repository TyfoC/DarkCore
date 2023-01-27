#include "storage-device.hxx"

StorageDevice::StorageDevice() {
	Type_ = TYPE_UNKNOWN;
	Data_ = 0;
}

StorageDevice::StorageDevice(const IDEControllerChannel::Device& ideCCDev) {
	Type_ = TYPE_IDE_CONTROLLER_CHANNEL_DEVICE;
	Data_ = (void*)&ideCCDev;
}

StorageDevice::~StorageDevice() {
	Type_ = TYPE_UNKNOWN;
}

StorageDevice& StorageDevice::operator()(const IDEControllerChannel::Device& ideCCDev) {
	Type_ = TYPE_IDE_CONTROLLER_CHANNEL_DEVICE;
	Data_ = (void*)&ideCCDev;
	return *this;
}

size_t StorageDevice::GetType() {
	return Type_;
}

void* StorageDevice::GetData() {
	return Data_;
}

size_t StorageDevice::GetSectorsCount() {
	if (Type_ == TYPE_IDE_CONTROLLER_CHANNEL_DEVICE) return ((IDEControllerChannel::Device*)Data_)->GetSectorsCount();
	return 0;
}

void StorageDevice::GetModelName(void* buffer) {
	if (Type_ == TYPE_IDE_CONTROLLER_CHANNEL_DEVICE) ((IDEControllerChannel::Device*)Data_)->GetModelName((char*)buffer);
}

bool StorageDevice::ReadSectors(void* destination, size_t lbaOffset, size_t sectorsCount) {
	if (Type_ == TYPE_IDE_CONTROLLER_CHANNEL_DEVICE) {
		return ((IDEControllerChannel::Device*)Data_)->ReadSectors(destination, lbaOffset, sectorsCount) == IDEControllerChannel::Device::TRANSFER_STATUS_SUCCESS;
	}
	return false;
}

bool StorageDevice::WriteSectors(const void* source, size_t lbaOffset, size_t sectorsCount) {
	if (Type_ == TYPE_IDE_CONTROLLER_CHANNEL_DEVICE) {
		return ((IDEControllerChannel::Device*)Data_)->WriteSectors(source, lbaOffset, sectorsCount) == IDEControllerChannel::Device::TRANSFER_STATUS_SUCCESS;
	}
	return false;
}

char StorageDevice::GetLetterByIndex(size_t index) {
	if (index >= MaxDevicesCount) return (char)0;
	return 'A' + (char)index;
}

size_t StorageDevice::GetIndexByLetter(char letter) {
	if (letter < 'A' && letter > 'Z') return WRONG_INDEX;
	return (size_t)(letter - 'A');
}