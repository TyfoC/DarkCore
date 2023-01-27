#include "pci.hxx"

uint32_t PCI::ReadLong(uint8_t bus, uint8_t device, uint8_t function, uint8_t longIndex) {
	uint32_t address = (uint32_t)(
		(uint32_t)0x80000000 |
		((uint32_t)bus << 16) |
		((uint32_t)device << 11) |
		((uint32_t)function << 8) |
		((uint32_t)longIndex << 2)
	);

	InlineAssembly::SendPortLong(PORT_CONFIG_ADDRESS, address);
	return InlineAssembly::ReadPortLong(PORT_CONFIG_DATA);
}

uint16_t PCI::ReadWord(uint8_t bus, uint8_t device, uint8_t function, uint8_t wordIndex) {
	uint32_t longValue = ReadLong(bus, device, function, (wordIndex & 0x7E) >> 1);
	return (uint16_t)((longValue >> ((wordIndex & 1) << 4)) & 0xFFFF);
}

uint8_t PCI::ReadByte(uint8_t bus, uint8_t device, uint8_t function, uint8_t byteIndex) {
	uint16_t wordValue = ReadWord(bus, device, function, (byteIndex & 0xFE) >> 1);
	return (uint8_t)((wordValue >> ((byteIndex & 1) << 3)) & 0xFFFF);
}

uint8_t PCI::GetHeaderType(uint8_t bus, uint8_t device, uint8_t function) {
	return (uint8_t)ReadWord(bus, device, function, WORD_INDEX_HEADER_TYPE_BUILT_IN_SELF_TEST) & 0xFF;
}

uint16_t PCI::GetVendorID(uint8_t bus, uint8_t device, uint8_t function) {
	return ReadWord(bus, device, function, WORD_INDEX_VENDOR_ID);
}

uint8_t PCI::GetClass(uint8_t bus, uint8_t device, uint8_t function) {
	return (uint8_t)(ReadWord(bus, device, function, WORD_INDEX_SUB_CLASS_CLASS) >> 8) & 0xFF;
}

uint8_t PCI::GetSubClass(uint8_t bus, uint8_t device, uint8_t function) {
	return (uint8_t)(ReadWord(bus, device, function, WORD_INDEX_SUB_CLASS_CLASS) & 0xFF);
}

void PCI::CheckFunction(OnDeviceFound handler, uint8_t bus, uint8_t device, uint8_t function) {
	uint8_t baseClass = GetClass(bus, device, function);
	uint8_t subClass = GetSubClass(bus, device, function);

	if ((baseClass == CLASS_BRIDGE) && (subClass == SUB_CLASS_BRIDGE_PCI_TO_PCI_1 || subClass == SUB_CLASS_BRIDGE_PCI_TO_PCI_2)) {
		CheckBus(handler, (uint8_t)((ReadWord(bus, device, function, 0x0C) >> 8) & 0xFF));
	}
	else handler(bus, device, function, baseClass, subClass);
}

void PCI::CheckDevice(OnDeviceFound handler, uint8_t bus, uint8_t device) {
	uint16_t vendorID = GetVendorID(bus, device, 0);
	if (vendorID == InvalidVendorID) return;
	CheckFunction(handler, bus, device, 0);

	uint8_t headerType = GetHeaderType(bus, device, 0);
	if (headerType & HEADER_MULTI_FUNCTIONAL) {
		for (uint8_t functionIndex = 1; functionIndex < 8; functionIndex++) {
			vendorID = GetVendorID(bus, device, functionIndex);
			if (vendorID != InvalidVendorID) {
				CheckFunction(handler, bus, device, functionIndex);
			}
		}
	}
}

void PCI::CheckBus(OnDeviceFound handler, uint8_t bus) {
	for (uint8_t deviceIndex = 0; deviceIndex < 32; deviceIndex++) {
		CheckDevice(handler, bus, deviceIndex);
	}
}

void PCI::CheckAllBuses(OnDeviceFound handler) {
	uint8_t headerType = GetHeaderType(0, 0, 0);
	if (!(headerType & HEADER_MULTI_FUNCTIONAL)) {
		CheckBus(handler, 0);
	}
	else {
		for (uint8_t busIndex = 0; busIndex < 8; busIndex++) {
			if (GetVendorID(0, 0, busIndex) != InvalidVendorID) break;
			CheckBus(handler, busIndex);
		}
	}
}