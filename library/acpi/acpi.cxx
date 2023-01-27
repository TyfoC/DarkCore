#include "acpi.hxx"

bool ACPI::SCIEnabled_ = 0;
uint8_t ACPI::MajorVersion_;
uint32_t ACPI::SMICommand_;
uint8_t ACPI::Enable_;
uint8_t ACPI::Disable_;
uint32_t ACPI::PM1AControl_;
uint32_t ACPI::PM1BControl_;
uint8_t ACPI::PM1ControlLength_;
uint8_t ACPI::MinorVersion_;
uint16_t ACPI::BootArchitectureFlags_;
uint16_t ACPI::SleepTypeA_;
uint16_t ACPI::SleepTypeB_;
uint16_t ACPI::SleepEnable_;

/**
 * @return true if it was possible to get data and control over ACPI, otherwise false
*/
bool ACPI::Initialize() {
	PRSDP rsdp = (PRSDP)(*((puint16_t)0x0000040E) * 0x10 & 0x000FFFFF);
	size_t searchBound = (size_t)rsdp + 0x400;

	for (; (size_t)rsdp < searchBound;) {
		if (IsRSDPValid(rsdp)) break;
		rsdp = (PRSDP)((size_t)rsdp + RSDPSignatureSize);
	}

	if (!IsRSDPValid(rsdp)) {
		rsdp = (PRSDP)0x000E0000;
		searchBound = 0x00100000;

		for (; (size_t)rsdp < searchBound;) {
			if (IsRSDPValid(rsdp)) break;
			rsdp = (PRSDP)((size_t)rsdp + RSDPSignatureSize);
		}

		if (!IsRSDPValid(rsdp)) return false;
	}

	PRSDT rsdt;
	size_t sdtCount, ptrShift;
	if (!rsdp->Revision) {
		rsdt = (PRSDT)rsdp->RSDTAddress;
		sdtCount = rsdt->Header.Length - sizeof(SDTHeader);
		ptrShift = sizeof(uint32_t);
		if (!IsSDTValid(&rsdt->Header, SIGNATURE_RSDT)) return false;
	}
	else {
		if (rsdp->XSDTAddress >= 0x0000000100000000) {
			if (!rsdp->RSDTAddress) return false;
			rsdt = (PRSDT)rsdp->RSDTAddress;
			ptrShift = sizeof(uint32_t);
			if (!IsSDTValid(&rsdt->Header, SIGNATURE_RSDT)) return false;
		}
		else {
			rsdt = (PRSDT)((size_t)rsdp->XSDTAddress);
			ptrShift = sizeof(uint64_t);
			if (!IsSDTValid(&rsdt->Header, SIGNATURE_XSDT)) return false;
		}
	}

	void* sdtPointer = (void*)((size_t)rsdt + sizeof(SDTHeader)), *dsdt;
	PSDTHeader sdtHeader;
	PFADT fadt;
	for (size_t i = 0; i < sdtCount; i++, sdtPointer = (void*)((size_t)sdtPointer + ptrShift)) {
		if (ptrShift == sizeof(uint64_t) && *((puint64_t)sdtPointer) >= 0x0000000100000000) continue;
		
		sdtHeader = (PSDTHeader)*(psize_t)sdtPointer;

		if (sdtHeader->Signature == SIGNATURE_FADT && IsSDTValid(sdtHeader, SIGNATURE_FADT)) {
			fadt = (PFADT)sdtHeader;

			MajorVersion_ = fadt->Header.Revision;
			MinorVersion_ = fadt->MinorVersion;

			if (fadt->ExtendedDSDTAddress && fadt->ExtendedDSDTAddress < 0x0000000100000000) {
				dsdt = (void*)(size_t)(fadt->ExtendedDSDTAddress);
			}
			else dsdt = (void*)fadt->DSDTAddress;

			if (!IsSDTValid((PSDTHeader)dsdt, SIGNATURE_DSDT)) return false;

			puint8_t s5obj = (puint8_t)((size_t)dsdt + sizeof(SDTHeader));
			size_t dsdtLength = ((PSDTHeader)dsdt)->Length - sizeof(SDTHeader);
			
			while (dsdtLength--) {
				if (*((puint32_t)s5obj) == SIGNATURE_DSDT_S5) break;
				s5obj++;
			}

			if (!dsdtLength) return false;

			if ((*(s5obj - 1) == 0x08 || (*(s5obj - 2) == 0x08 && *(s5obj - 1) == '\\')) && *(s5obj + 4) == 0x12) {
				s5obj += 5;
				s5obj += ((*s5obj & 0xc0) >> 6) + 2;

				if (*s5obj == 0x0a) ++s5obj;
				SleepTypeA_ = *(s5obj) << 10;

				SMICommand_ = fadt->SMICommand;

				Enable_ = fadt->ACPIEnable;
				Disable_ = fadt->ACPIDisable;

				PM1AControl_ = fadt->PM1AControlBlock;
				PM1BControl_ = fadt->PM1BControlBlock;

				PM1ControlLength_ = fadt->PM1ControlLength;

				BootArchitectureFlags_ = fadt->BootArchitectureFlags;

				SleepEnable_ = 0x2000;
				SCIEnabled_ = true;

				// enabling acpi
				if (!(InlineAssembly::ReadPortWord((uint16_t)PM1AControl_) & SCIEnabled_)) {
					if (SMICommand_ && Enable_) {
						InlineAssembly::SendPortByte((uint16_t)SMICommand_, Enable_);
						size_t j = 0;
						for (; j < 300; j++) {
							if ((InlineAssembly::ReadPortWord((uint16_t)PM1AControl_) & SCIEnabled_) == 1) break;
							VirtualTimer::Sleep(10);
						}

						if (PM1BControl_) {
							for (; j < 300; j++) {
								if ((InlineAssembly::ReadPortWord((uint16_t)PM1BControl_) & SCIEnabled_) == 1) break;
								VirtualTimer::Sleep(10);
							}
						}

						if (j >= 300) return false;
					}
					else return false;
				}
			}
		}
	}

	return true;
}

/**
 * @return true if it was possible to power off, otherwise false
*/
bool ACPI::PowerOff() {
	if (SCIEnabled_ && InlineAssembly::ReadPortWord((uint16_t)PM1AControl_) & SCIEnabled_) {
		InlineAssembly::SendPortWord((uint16_t)PM1AControl_, SleepTypeA_ | SleepEnable_);
		if (PM1BControl_) InlineAssembly::SendPortWord((uint16_t)PM1BControl_, SleepTypeB_ | SleepEnable_);
		return true;
	}
	
	return false;
}

/**
 * @param rsdp [in] pointer to RSDP
 * @return true if RSDP is valid, otherwise false
*/
bool ACPI::IsRSDPValid(const PRSDP rsdp) {
	if (rsdp->Signature != RSDPSignature) return false;

	size_t checkLength;
	if (!rsdp->Revision) checkLength = (size_t)(&((PRSDP)0)->Length);
	else checkLength = rsdp->Length;

	uint8_t value = 0;
	for (size_t i = 0; i < checkLength; i++) value += ((puint8_t)rsdp)[i];

	return !value;
}

/**
 * @param sdt [in] pointer to SDT
 * @param signature [in] putative SDT signature
 * @return true if SDT is valid, otherwise false
*/
bool ACPI::IsSDTValid(const void* sdt, uint32_t signature) {
	if (((PSDTHeader)sdt)->Signature != signature) return false;

	uint8_t value = 0;
	for (size_t i = 0; i < ((PSDTHeader)sdt)->Length; i++) value += ((puint8_t)sdt)[i];

	return !value;
}

/**
 * @param sdt [in] pointer to SDT
 * @param signature [in] putative SDT signature
 * @return true if it was possible to establish control, otherwise false
*/
bool ACPI::Initialized() {
	return SCIEnabled_;
}

/**
 * @return major version of ACPI, if it was possible to establish control, otherwise UndefinedVersion
*/
uint8_t ACPI::GetMajorVersion() {
	if (!SCIEnabled_) return UndefinedVersion;
	return MajorVersion_;
}

/**
 * @return minor version of ACPI, if it was possible to establish control, otherwise UndefinedVersion
*/
uint8_t ACPI::GetMinorVersion() {
	if (!SCIEnabled_) return UndefinedVersion;
	return MinorVersion_ & 0x0F;
}

/**
 * @return Boot Architecture Flags (FADT) or 0
*/
uint16_t ACPI::GetBootArchitectureFlags() {
	if (!SCIEnabled_) return 0;
	return BootArchitectureFlags_;
}