ENTRY(_start)
OUTPUT_ARCH(i386)
OUTPUT_FORMAT(binary)
SECTIONS {
	BaseAddress = 0x400000;
	. = BaseAddress;

	.LEFHeader : {
		LONG(0x3046454C);					/*Signature:	LEF0 - for i386*/
		LONG(0x8000);						/*Stack size:	32K*/
		LONG(BaseAddress);					/*Placement address*/
		LONG(EndAddress - BaseAddress);		/*File size*/
		LONG(_start - BaseAddress);			/*Entry point offset*/
		LONG(0x00000004);					/*Sections count*/
		LONG(SectionHeaders - BaseAddress);	/*Section headers offset*/
	}

	. = ALIGN(0x200);

	SectionHeaders = .;
	.LEFSectionHeaders : {
		/*====================================
			Section Header Format:

			Attributes				[uint32_t]
				ReadOnly - 0x01
			Offset					[uint32_t]
			Size					[uint32_t]
		====================================*/

		/*.text:*/
		LONG(0x00000001);
		LONG(TextStart - BaseAddress);
		LONG(TextEnd - TextStart);

		/*.data:*/
		LONG(0x00000000);
		LONG(DataStart - BaseAddress);
		LONG(DataEnd - DataStart);

		/*.rodata:*/
		LONG(0x00000000);
		LONG(RodataStart - BaseAddress);
		LONG(RodataEnd - RodataStart);

		/*.bss:*/
		LONG(0x00000000);
		LONG(BssStart - BaseAddress);
		LONG(BssEnd - BssStart);
	}

	. = ALIGN(0x200);

	TextStart = .;
	.text : {
		*(.text)
	}
	TextEnd = .;

	. = ALIGN(0x200);

	DataStart = .;
	.data : {
		*(.text)
	}
	DataEnd = .;

	. = ALIGN(0x200);

	RodataStart = .;
	.rodata : {
		*(.text)
	}
	RodataEnd = .;

	. = ALIGN(0x200);

	BssStart = .;
	.bss : {
		*(.text)
	}
	BssEnd = .;
	EndAddress = .;

	. = ALIGN(0x200);
}