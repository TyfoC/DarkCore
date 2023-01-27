#include "boot-information.hxx"

BootInformation::Structure BootInformation::Structure_;
/**
 * @param pointer [in] pointer to boot information data
 * @return BootInformation::ConfigSpaceAccessMechanismType
*/
void BootInformation::Initialize(const PStructure pointer) {
	Structure_ = *pointer;
}

size_t BootInformation::GetARDTableAddress() {
	return Structure_.ARDTableAddress;
}

size_t BootInformation::GetARDTableEntriesCount() {
	return Structure_.ARDTableEntriesCount;
}

size_t BootInformation::GetBootFlags() {
	return Structure_.BootFlags;
}

/**
 * @return BootInformation::ConfigSpaceAccessMechanismType
*/
uint8_t BootInformation::GetConfigSpaceAccessMechanism() {
	return Structure_.BootFlags & ACCESS_MECHANISM_BOTH;
}