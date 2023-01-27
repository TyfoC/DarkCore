#include "ide.hxx"

IDEControllerChannel::IDEControllerChannel(uint16_t basePort, uint16_t controlPort, uint16_t busMasterIDEPort) {
	BasePort_ = basePort;
	ControlPort_ = controlPort;
	BusMasterIDEPort_ = busMasterIDEPort;
	InterruptDisabled_ = true;

	DevicesCount_ = 0;
	MemoryUtils::Fill(IdentifyBuffer_, 0, IdentifyBufferSize);

	Write(REGISTER_CONTROL, InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0);
	for (size_t i = 0; i < Device::DevicesPerChannel; i++) {
		Device tmpDevice((uint8_t)i, this);
		if (tmpDevice.Exist()) {
			Devices_[DevicesCount_] = tmpDevice;
			DevicesCount_++;
		}
	}
}

uint8_t IDEControllerChannel::Read(uint8_t reg) {
	uint8_t result = 0;
	if (reg > 0x07 && reg < 0x0C) Write(REGISTER_CONTROL, 0x80 | (InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0));
	
	if (reg < 0x08) result = InlineAssembly::ReadPortByte(BasePort_ + reg);
	else if (reg < 0x0C) result = InlineAssembly::ReadPortByte((uint16_t)(BasePort_ + reg - 0x06));
	else if (reg < 0x0E) result = InlineAssembly::ReadPortByte((uint16_t)(ControlPort_ + reg - 0x0A));
	else if (reg < 0x16) result = InlineAssembly::ReadPortByte((uint16_t)(BusMasterIDEPort_ + reg - 0x0E));

	if (reg > 0x07 && reg < 0x0C) InlineAssembly::SendPortByte(REGISTER_CONTROL, InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0);
	return result;
}

void IDEControllerChannel::Write(uint8_t reg, uint8_t data) {
	if (reg > 0x07 && reg < 0x0C) Write(REGISTER_CONTROL, 0x80 | (InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0));
	
	if (reg < 0x08) InlineAssembly::SendPortByte(BasePort_ + reg, data);
	else if (reg < 0x0C) InlineAssembly::SendPortByte((uint16_t)(BasePort_ + reg - 0x06), data);
	else if (reg < 0x0E) InlineAssembly::SendPortByte((uint16_t)(ControlPort_ + reg - 0x0A), data);
	else if (reg < 0x16) InlineAssembly::SendPortByte((uint16_t)(BusMasterIDEPort_ + reg - 0x0E), data);

	if (reg > 0x07 && reg < 0x0C) InlineAssembly::SendPortByte(REGISTER_CONTROL, InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0);
}

void IDEControllerChannel::ReadBuffer(uint8_t reg, void* buffer, size_t longsCount) {
	puint32_t buffer32 = (puint32_t)buffer;
	if (reg > 0x07 && reg < 0x0C) Write(REGISTER_CONTROL, 0x80 | (InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0));
	
	if (reg < 0x08) {
		for (size_t i = 0; i < longsCount; i++) {
			*buffer32 = InlineAssembly::ReadPortLong(BasePort_ + reg);
			buffer32++;
		}
	}
	else if (reg < 0x0C) {
		for (size_t i = 0; i < longsCount; i++) {
			*buffer32 = InlineAssembly::ReadPortLong((uint16_t)(BasePort_ + reg - 0x06));
			buffer32++;
		}
	}
	else if (reg < 0x0E) {
		for (size_t i = 0; i < longsCount; i++) {
			*buffer32 = InlineAssembly::ReadPortLong((uint16_t)(ControlPort_ + reg - 0x0A));
			buffer32++;
		}
	}
	else if (reg < 0x16) {
		for (size_t i = 0; i < longsCount; i++) {
			*buffer32 = InlineAssembly::ReadPortLong((uint16_t)(BusMasterIDEPort_ + reg - 0x0E));
			buffer32++;
		}
	}

	if (reg > 0x07 && reg < 0x0C) InlineAssembly::SendPortByte(REGISTER_CONTROL, InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0);
}

uint8_t IDEControllerChannel::Polling(bool fullCheck) {
	for (size_t i = 0; i < 4; i++) Read(REGISTER_ALTERNATIVE_STATUS);		//	400ns
	while (Read(REGISTER_STATUS) & STATUS_BUSY);

	if (fullCheck) {
		uint8_t statusValue = Read(REGISTER_STATUS);

		if (statusValue & STATUS_ERROR) return POLLING_STATUS_ERROR;
		else if (statusValue & STATUS_DRIVE_WRITE_FAULT) return POLLING_STATUS_DRIVE_WRITE_FAULT;
		else if (!(statusValue & STATUS_DRIVE_READY)) return POLLING_STATUS_DATA_REQUEST_NOT_READY;
	}

	return POLLING_STATUS_SUCCESS;
}

size_t IDEControllerChannel::GetDevicesCount() {
	return DevicesCount_;
}

IDEControllerChannel::Device& IDEControllerChannel::GetDevice(size_t deviceType) {
	return Devices_[deviceType];
}

IDEControllerChannel::Device::Device() {
	Type_ = 0;
	Channel_ = 0;
}

IDEControllerChannel::Device::Device(uint8_t type, const IDEControllerChannel* channel) {
	Exist_ = false;
	if (!channel) return;

	Type_ = type;
	Channel_ = (IDEControllerChannel*)channel;
	MemoryUtils::Fill(ModelName_, 0, MaxModelNameLength);

	Channel_->Write(REGISTER_DEVICE, 0xA0 | (type == TYPE_SLAVE ? REGISTER_DEVICE_BIT_SLAVE : 0));
	VirtualTimer::Sleep(1);
	Channel_->Write(REGISTER_COMMAND, COMMAND_IDENTIFY);
	VirtualTimer::Sleep(1);
	if (Channel_->Read(REGISTER_STATUS) == STATUS_NO_DRIVE) return;

	uint8_t statusValue = 0, errorValue = 0, interfaceType = INTERFACE_TYPE_ATA, ch, cl;
	while (true) {
		statusValue = Channel_->Read(REGISTER_STATUS);
		if (statusValue & STATUS_ERROR) {
			errorValue = STATUS_ERROR;
			break;
		}
		else if (!(statusValue & STATUS_ERROR) && (statusValue & STATUS_DATA_REQUEST_READY)) break;
	}

	if (errorValue) {
		ch = Channel_->Read(REGISTER_LBA_1);
		cl = Channel_->Read(REGISTER_LBA_2);

		if ((ch == 0xEB && cl == 0x14) || (ch == 0x96 && cl == 0x69)) interfaceType = INTERFACE_TYPE_ATAPI;
		else return;

		Channel_->Write(REGISTER_COMMAND, COMMAND_IDENTIFY_PACKET);
		VirtualTimer::Sleep(1);
	}

	Channel_->ReadBuffer(REGISTER_DATA, Channel_->IdentifyBuffer_, 128);

	Signature_ = *((puint16_t)(Channel_->IdentifyBuffer_ + IDENTIFY_BUFFER_OFFSET_DEVICE_TYPE));
	Capatibilities_ = *((puint16_t)(Channel_->IdentifyBuffer_ + IDENTIFY_BUFFER_OFFSET_CAPABILITIES));
	CommandSets_ = *((puint16_t)(Channel_->IdentifyBuffer_ + IDENTIFY_BUFFER_OFFSET_COMMAND_SETS));
	SectorsCount_ = *((puint32_t)(Channel_->IdentifyBuffer_ + (CommandSets_ & 0x4000000 ? IDENTIFY_BUFFER_OFFSET_MAX_LBA_EXTENDED : IDENTIFY_BUFFER_OFFSET_MAX_LBA)));

	for (size_t k = 0; k < MaxModelNameLength - 1; k += 2) {
		ModelName_[k] = Channel_->IdentifyBuffer_[IDENTIFY_BUFFER_OFFSET_MODEL_NAME + k + 1];
		ModelName_[k + 1] = Channel_->IdentifyBuffer_[IDENTIFY_BUFFER_OFFSET_MODEL_NAME + k];
	}

	ModelName_[MaxModelNameLength - 1] = 0;
	InterfaceType_ = interfaceType;
	Exist_ = true;
}

bool IDEControllerChannel::Device::Exist() {
	return Exist_;
}

uint8_t IDEControllerChannel::Device::NativeReadSectors(void* destination, uint32_t lba, uint8_t sectorsCount) {
	if (!Exist_) return TRANSFER_STATUS_DEVICE_NOT_EXIST;

	uint8_t transferMode, command;
	uint8_t lbaInputOutput[6];
	uint16_t cylinderIndex, i, j;
	uint8_t headIndex, sectorNumber, errorValue;

	Channel_->Write(REGISTER_CONTROL, Channel_->InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0);

	if (lba >= 0x10000000) {
		if (!(Capatibilities_ & 0x200)) return TRANSFER_STATUS_LBA48_UNSUPPORTED;
		transferMode = TRANSFER_MODE_LBA48;
		lbaInputOutput[0] = lba & 0xFF;
		lbaInputOutput[1] = (uint8_t)((lba & 0xFF00) >> 8);
		lbaInputOutput[2] = (uint8_t)((lba & 0xFF0000) >> 16);
		lbaInputOutput[3] = (uint8_t)((lba & 0xFF000000) >> 24);
		lbaInputOutput[4] = lbaInputOutput[5] = headIndex = 0;
	}
	else if (Capatibilities_ & 0x200) {
		transferMode = TRANSFER_MODE_LBA28;
		lbaInputOutput[0] = lba & 0xFF;
		lbaInputOutput[1] = (uint8_t)((lba & 0xFF00) >> 8);
		lbaInputOutput[2] = (uint8_t)((lba & 0xFF0000) >> 16);
		lbaInputOutput[3] = lbaInputOutput[4] = lbaInputOutput[5] = 0;
		headIndex = (lba & 0x0F000000) >> 24;
	}
	else {
		transferMode = TRANSFER_MODE_CHS;
		sectorNumber = (uint8_t)((lba % 63) + 1);
		cylinderIndex = (uint16_t)((lba + 1 - sectorNumber) / (16 * 63));
		lbaInputOutput[0] = sectorNumber;
		lbaInputOutput[1] = (uint8_t)(cylinderIndex & 0xFF);
		lbaInputOutput[2] = (uint8_t)((cylinderIndex >> 8) & 0xFF);
		lbaInputOutput[3] = lbaInputOutput[4] = lbaInputOutput[5] = 0;
		headIndex = (uint8_t)((lba + 1 - sectorNumber) % (16 * 63) / 63);
	}

	while (Channel_->Read(REGISTER_STATUS) & STATUS_BUSY);

	uint8_t hardDriveSelectValue = transferMode == TRANSFER_MODE_CHS ? 0xA0 : 0xE0;
	Channel_->Write(REGISTER_DEVICE, hardDriveSelectValue | ((Type_ == TYPE_SLAVE) << 4) | headIndex);
	
	if (transferMode == TRANSFER_MODE_LBA48) {
		Channel_->Write(REGISTER_SECTORS_COUNT_1, 0);
		Channel_->Write(REGISTER_LBA_3, lbaInputOutput[3]);
		Channel_->Write(REGISTER_LBA_4, lbaInputOutput[4]);
		Channel_->Write(REGISTER_LBA_5, lbaInputOutput[5]);
	}

	Channel_->Write(REGISTER_SECTORS_COUNT_0, sectorsCount);
	Channel_->Write(REGISTER_LBA_0, lbaInputOutput[0]);
	Channel_->Write(REGISTER_LBA_1, lbaInputOutput[1]);
	Channel_->Write(REGISTER_LBA_2, lbaInputOutput[2]);

	if (transferMode == TRANSFER_MODE_CHS || transferMode == TRANSFER_MODE_LBA28) command = COMMAND_READ_PIO;
	else if (transferMode == TRANSFER_MODE_LBA48) command = COMMAND_READ_PIO_EXTENDED;
	else return TRANSFER_STATUS_UNKNOWN_MODE;

	Channel_->Write(REGISTER_COMMAND, command);

	puint16_t buffer16 = (puint16_t)destination;

	for (j = 0; j < sectorsCount; j++) {
		for (i = 0; i < 256; i++) {
			errorValue = Channel_->Polling(true);
			if (errorValue) return errorValue;

			*buffer16 = InlineAssembly::ReadPortWord(Channel_->BasePort_);
			++buffer16;
		}
	}

	return TRANSFER_STATUS_SUCCESS;
}

uint8_t IDEControllerChannel::Device::NativeWriteSectors(const void* source, uint32_t lba, uint8_t sectorsCount) {
	if (!Exist_) return TRANSFER_STATUS_DEVICE_NOT_EXIST;

	uint8_t transferMode, command;
	uint8_t lbaInputOutput[6];
	uint16_t cylinderIndex, i, j;
	uint8_t headIndex, sectorNumber;

	Channel_->Write(REGISTER_CONTROL, Channel_->InterruptDisabled_ ? REGISTER_CONTROL_BIT_IRQ_DISABLED : 0);

	if (lba >= 0x10000000) {
		if (!(Capatibilities_ & 0x200)) return TRANSFER_STATUS_LBA48_UNSUPPORTED;
		transferMode = TRANSFER_MODE_LBA48;
		lbaInputOutput[0] = lba & 0xFF;
		lbaInputOutput[1] = (uint8_t)((lba & 0xFF00) >> 8);
		lbaInputOutput[2] = (uint8_t)((lba & 0xFF0000) >> 16);
		lbaInputOutput[3] = (uint8_t)((lba & 0xFF000000) >> 24);
		lbaInputOutput[4] = lbaInputOutput[5] = headIndex = 0;
	}
	else if (Capatibilities_ & 0x200) {
		transferMode = TRANSFER_MODE_LBA28;
		lbaInputOutput[0] = lba & 0xFF;
		lbaInputOutput[1] = (uint8_t)((lba & 0xFF00) >> 8);
		lbaInputOutput[2] = (uint8_t)((lba & 0xFF0000) >> 16);
		lbaInputOutput[3] = lbaInputOutput[4] = lbaInputOutput[5] = 0;
		headIndex = (lba & 0x0F000000) >> 24;
	}
	else {
		transferMode = TRANSFER_MODE_CHS;
		sectorNumber = (uint8_t)((lba % 63) + 1);
		cylinderIndex = (uint16_t)((lba + 1 - sectorNumber) / (16 * 63));
		lbaInputOutput[0] = sectorNumber;
		lbaInputOutput[1] = (uint8_t)(cylinderIndex & 0xFF);
		lbaInputOutput[2] = (uint8_t)((cylinderIndex >> 8) & 0xFF);
		lbaInputOutput[3] = lbaInputOutput[4] = lbaInputOutput[5] = 0;
		headIndex = (uint8_t)((lba + 1 - sectorNumber) % (16 * 63) / 63);
	}

	while (Channel_->Read(REGISTER_STATUS) & STATUS_BUSY);

	uint8_t hardDriveSelectValue = transferMode == TRANSFER_MODE_CHS ? 0xA0 : 0xE0;
	Channel_->Write(REGISTER_DEVICE, hardDriveSelectValue | ((Type_ == TYPE_SLAVE) << 4) | headIndex);
	
	if (transferMode == TRANSFER_MODE_LBA48) {
		Channel_->Write(REGISTER_SECTORS_COUNT_1, 0);
		Channel_->Write(REGISTER_LBA_3, lbaInputOutput[3]);
		Channel_->Write(REGISTER_LBA_4, lbaInputOutput[4]);
		Channel_->Write(REGISTER_LBA_5, lbaInputOutput[5]);
	}

	Channel_->Write(REGISTER_SECTORS_COUNT_0, sectorsCount);
	Channel_->Write(REGISTER_LBA_0, lbaInputOutput[0]);
	Channel_->Write(REGISTER_LBA_1, lbaInputOutput[1]);
	Channel_->Write(REGISTER_LBA_2, lbaInputOutput[2]);

	if (transferMode == TRANSFER_MODE_CHS || transferMode == TRANSFER_MODE_LBA28) command = COMMAND_WRITE_PIO;
	else if (transferMode == TRANSFER_MODE_LBA48) command = COMMAND_WRITE_PIO_EXTENDED;
	else return TRANSFER_STATUS_UNKNOWN_MODE;

	Channel_->Write(REGISTER_COMMAND, command);

	puint16_t buffer16 = (puint16_t)source;

	for (j = 0; j < sectorsCount; j++) {
		for (i = 0; i < 256; i++) {
			Channel_->Polling(false);
			InlineAssembly::SendPortWord(Channel_->BasePort_, *buffer16);
			++buffer16;
		}
	}

	Channel_->Polling(false);
	return TRANSFER_STATUS_SUCCESS;
}

uint8_t IDEControllerChannel::Device::ReadSectors(void* destination, uint32_t lba, size_t sectorsCount) {
	size_t fullTimesCount = sectorsCount / MaxSectorsPerOperation;
	uint8_t lastTimesCount = (uint8_t)(sectorsCount - fullTimesCount * MaxSectorsPerOperation);

	puint8_t buffer = (puint8_t)destination;
	uint8_t status;
	for (size_t i = 0; i < fullTimesCount; i++) {
		status = NativeReadSectors(buffer, lba, MaxSectorsPerOperation);
		if (status != TRANSFER_STATUS_SUCCESS) return status;
		lba += MaxSectorsPerOperation;
		buffer = (puint8_t)((size_t)buffer + ShiftPerMaxSectors);
	}

	if (lastTimesCount) return NativeReadSectors(buffer, lba, lastTimesCount);
	else return TRANSFER_STATUS_SUCCESS;
}

uint8_t IDEControllerChannel::Device::WriteSectors(const void* source, uint32_t lba, size_t sectorsCount) {
	size_t fullTimesCount = sectorsCount / MaxSectorsPerOperation;
	uint8_t lastTimesCount = (uint8_t)(sectorsCount - fullTimesCount * MaxSectorsPerOperation);

	puint8_t buffer = (puint8_t)source;
	uint8_t status;
	for (size_t i = 0; i < fullTimesCount; i++) {
		status = NativeWriteSectors(buffer, lba, MaxSectorsPerOperation);
		if (status != TRANSFER_STATUS_SUCCESS) return status;
		lba += MaxSectorsPerOperation;
		buffer = (puint8_t)((size_t)buffer + ShiftPerMaxSectors);
	}

	if (lastTimesCount) return NativeWriteSectors(buffer, lba, lastTimesCount);
	else return TRANSFER_STATUS_SUCCESS;
}

uint8_t IDEControllerChannel::Device::GetType() {
	return Type_;
}

uint8_t IDEControllerChannel::Device::GetInterfaceType() {
	return InterfaceType_;
}

uint16_t IDEControllerChannel::Device::GetSignature() {
	return Signature_;
}

uint16_t IDEControllerChannel::Device::GetCapatibilities() {
	return Capatibilities_;
}

uint32_t IDEControllerChannel::Device::GetCommandSets() {
	return CommandSets_;
}

uint32_t IDEControllerChannel::Device::GetSectorsCount() {
	return SectorsCount_;
}

void IDEControllerChannel::Device::GetModelName(char* buffer) {
	MemoryUtils::Copy(buffer, ModelName_, MaxModelNameLength);
}

IDEControllerChannel* IDEControllerChannel::Device::GetChannel() {
	return Channel_;
}

void IDEControllerChannel::Device::SetChannel(IDEControllerChannel* channel) {
	Channel_ = channel;
}