#include "idt.hxx"

IDT::Register IDT::Register_;

/**
 * @param table [in] IDT
 * @param countEntries [in] IDT entries count
*/
void IDT::Select(PEntry table, size_t countEntries) {
	Register_.Size = (uint16_t)(sizeof(Entry) * countEntries - 1);
	Register_.Offset = (size_t)table;
	InlineAssembly::LoadIDTRegister(&Register_);
}

/**
 * @param offset [in] ISR stub offset
 * @param segmentSelector [in] code segment selector
 * @param gateType [in] IDT entry gate type
 * @param flags [in] IDT entry flags field
 * @return IDT::Entry
*/
IDT::Entry IDT::CreateEntry(size_t offset, uint16_t segmentSelector, uint8_t gateType, uint8_t flags) {
	Entry entry = {
		(uint16_t)(offset & 0xFFFF),
		segmentSelector,
		0,
		(uint8_t)(gateType & 0x0F),
		(uint8_t)(flags & 0x0E),
		(uint16_t)((offset >> 16) & 0xFFFF)
	};

	return entry;
}