#include "gdt.hxx"

GDT::Register GDT::Register_;

/**
 * @param table [in] GDT
 * @param countEntries [in] GDT entries count
*/
void GDT::Select(
	PEntry table,
	size_t countEntries
) {
	Register_.Size = (uint16_t)(sizeof(Entry) * countEntries - 1);
	Register_.Offset = (size_t)table;
	InlineAssembly::LoadGDTRegister(&Register_);
}

/**
 * @param limit [in] global descriptor end address
 * @param base [in] global descriptor start address
 * @param access [in] GDT entry access field
 * @param flags [in] GDT entry flags field
 * @return GDT:::Entry
*/
GDT::Entry GDT::CreateEntry(
	size_t limit,
	size_t base,
	uint8_t access,
	uint8_t flags
) {
	Entry entry = {
		(uint16_t)(limit & 0xFFFF),
		(uint32_t)(base & 0xFFFFFF),
		access,
		(uint8_t)((limit >> 16) & 0x0F),
		(uint8_t)(flags & 0x0F),
		(uint8_t)((base >> 24) & 0xFF)
	};

	return entry;
}