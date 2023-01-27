#include "storage-devices.hxx"

List<IDEControllerChannel::Device>*	StorageDevices::IDECCDevices_ = 0;
StorageDevice						StorageDevices::Devices_[StorageDevice::MaxDevicesCount];
size_t								StorageDevices::DevicesCount_ = 0;

void StorageDevices::Initialize() {
	IDECCDevices_ = new List<IDEControllerChannel::Device>();
}

size_t StorageDevices::GetCount() {
	return DevicesCount_;
}

bool StorageDevices::Append(const IDEControllerChannel::Device& ideCCDevice) {
	if (DevicesCount_ >= StorageDevice::MaxDevicesCount) return false;

	IDECCDevices_->Append(ideCCDevice);
	Devices_[DevicesCount_] = StorageDevice(IDECCDevices_->GetLastElement());

	++DevicesCount_;
	return true;
}

StorageDevice& StorageDevices::Get(size_t index) {
	return Devices_[index];
}

StorageDevice* StorageDevices::GetArray() {
	return &Devices_[0];
}

List<IDEControllerChannel::Device>* StorageDevices::GetIDECCDevicesList() {
	return IDECCDevices_;
}