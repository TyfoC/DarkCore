ENTRY(DarkCoreEntry)
OUTPUT_ARCH(i386)
OUTPUT_FORMAT(binary)
SECTIONS {
	__FileBegin = 0x00020000;
	. = __FileBegin;

	.Header : {
		LONG(DarkCoreEntry);
		LONG(__StackTop);
	}

	. = ALIGN(0x00000200);

	__StackBottom = .;
	.Stack : {
		. = . + 32K;
	}
	__StackTop = .;

	.Code : {
		*(.text)
	}

	. = ALIGN(0x00000200);

	.Data : {
		*(.data)
	}

	. = ALIGN(0x00000200);

	.Bss : {
		*(.bss)
	}

	. = ALIGN(0x00000200);

	.Rodata : {
		*(.rodata)
	}

	. = ALIGN(0x00000200);

	__FileEnd = .;
}