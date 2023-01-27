%IFNDEF CPU_INC
%DEFINE CPU_INC

STRUC GlobalDescriptorTableEntry_t
	.LimitLow:			RESW 0x01
	.AddressLow:		RESB 0x03
	.AccessByte:		RESB 0x01
	.LimitHighFlags:	RESB 0x01
	.AddressHigh:		RESB 0x01
ENDSTRUC

STRUC GlobalDescriptorTablePointer_t
	.Limit:				RESW 0x01
	.Address:			RESD 0x01
ENDSTRUC

;	Function to get line a20 status
;	Output:
;		ZF - enabled/disabled
GetLineA20Status:
	PUSH AX
	PUSH SI
	PUSH DI
	PUSH DS
	PUSH ES
	MOV AX, 0x0000
	MOV DS, AX
	NOT AX
	MOV ES, AX
	MOV SI, 0x0500
	MOV DI, 0x0510

	MOV AH, [ES:DI]
	MOV AL, [DS:SI]

	MOV BYTE [ES:DI], 0xAA
	MOV BYTE [DS:SI], 0x55

	CMP BYTE [ES:DI], 0x55

	MOV BYTE [ES:DI], AH
	MOV BYTE [DS:SI], AL

	MOV AH, 0
	JE GetLineA20StatusDone

	MOV AH, 1
GetLineA20StatusDone:
	TEST AH, AH
	POP ES
	POP DS
	POP DI
	POP SI
	POP AX
	RET

;	Function to enable line a20
;	Output:
;		ZF - enabled/disabled
EnableLineA20:
	PUSH AX
	PUSH BX
	MOV AX, 0x2403
	INT 0x15
	JC EnableLineA20Error
	TEST AH, AH
	JNZ EnableLineA20Error
	MOV AX, 0x2402
	INT 0x15
	JC EnableLineA20Error
	TEST AH, AH
	JNZ EnableLineA20Error
	CMP AL, 1
	JE EnableLineA20Done
	MOV AX, 0x2401
	INT 0x15
	JC EnableLineA20Error
	TEST AH, AH
	JNZ EnableLineA20Error
	MOV AL, 0x01
	JMP EnableLineA20Done
EnableLineA20Error:
	MOV AL, 0x00
EnableLineA20Done:
	TEST AL, AL
	POP BX
	POP AX
	RET

;	Function to switch unreal big mode
SwitchUnrealBigMode:
	CLI
	PUSH AX
	PUSH BX
	PUSH DS
	LGDT [GdtUnrealBigModePointer]
	MOV EAX, CR0
	OR AL, 0x01
	MOV CR0, EAX
	jmp $+2
	MOV BX, 0x0008
	MOV DS, BX
	AND AL, 0xFE
	MOV CR0, EAX
	POP DS
	POP BX
	POP AX
	STI
	RET

;	Function to switch protected mode
SwitchProtectedMode:
	MOV [SwitchProtectedModeAddress], EDI
	CLI
	lgdt [GdtProtectedModePointer]
	MOV EAX, CR0
	OR AL, 0x01
	MOV CR0, EAX
	JMP 0x0008:SwitchProtectedModeInitializeRegisters
SwitchProtectedModeInitializeRegisters:
[BITS 32]
	MOV AX, 0x0010
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	JMP [SwitchProtectedModeAddress]
[BITS 16]
SwitchProtectedModeAddress:		DD 0x00000000

ALIGN(0x0010)

GdtUnrealBigModeStart:
	GdtUnrealBigModeNullDescriptor:	ISTRUC GlobalDescriptorTableEntry_t			;	null-descriptor
		AT GlobalDescriptorTableEntry_t.LimitLow,		DW 0x0000
		AT GlobalDescriptorTableEntry_t.AddressLow,		DB 0x00, 0x00, 0x00
		AT GlobalDescriptorTableEntry_t.AccessByte,		DB 0x00
		AT GlobalDescriptorTableEntry_t.LimitHighFlags,	DB 0x00
		AT GlobalDescriptorTableEntry_t.AddressHigh,	DB 0x00
	IEND
	GdtUnrealBigModeDataDescriptor:	ISTRUC GlobalDescriptorTableEntry_t			;	data segment descriptor
		AT GlobalDescriptorTableEntry_t.LimitLow,		DW 0xFFFF
		AT GlobalDescriptorTableEntry_t.AddressLow,		DB 0x00, 0x00, 0x00
		AT GlobalDescriptorTableEntry_t.AccessByte,		DB 0x92					;	(RW | S | P) or (Read/Write | Code/Data segment | Present)
		AT GlobalDescriptorTableEntry_t.LimitHighFlags,	DB 0xCF					;	Flags - 0xC: (DB | G) or (32-bit PM segment | Limit is in 4KiB blocks)
		AT GlobalDescriptorTableEntry_t.AddressHigh,	DB 0x00
	IEND
GdtUnrealBigModeEnd:

ALIGN(0x0010)

GdtProtectedModeStart:
	GdtProtectedModeNullDescriptor:	ISTRUC GlobalDescriptorTableEntry_t			;	null-descriptor
		AT GlobalDescriptorTableEntry_t.LimitLow,		DW 0x0000
		AT GlobalDescriptorTableEntry_t.AddressLow,		DB 0x00, 0x00, 0x00
		AT GlobalDescriptorTableEntry_t.AccessByte,		DB 0x00
		AT GlobalDescriptorTableEntry_t.LimitHighFlags,	DB 0x00
		AT GlobalDescriptorTableEntry_t.AddressHigh,	DB 0x00
	IEND
	GdtProtectedModeCodeDescriptor:	ISTRUC GlobalDescriptorTableEntry_t			;	code segment descriptor
		AT GlobalDescriptorTableEntry_t.LimitLow,		DW 0xFFFF
		AT GlobalDescriptorTableEntry_t.AddressLow,		DB 0x00, 0x00, 0x00
		AT GlobalDescriptorTableEntry_t.AccessByte,		DB 0x9A					;	(RW | E | S | P) or (Read/Write | Code Segment | Code/Data segment | Present)
		AT GlobalDescriptorTableEntry_t.LimitHighFlags,	DB 0xCF					;	Flags - 0xC: (DB | G) or (32-bit PM segment | Limit is in 4KiB blocks)
		AT GlobalDescriptorTableEntry_t.AddressHigh,	DB 0x00
	IEND
	GdtProtectedModeDataDescriptor:	ISTRUC GlobalDescriptorTableEntry_t			;	data segment descriptor
		AT GlobalDescriptorTableEntry_t.LimitLow,		DW 0xFFFF
		AT GlobalDescriptorTableEntry_t.AddressLow,		DB 0x00, 0x00, 0x00
		AT GlobalDescriptorTableEntry_t.AccessByte,		DB 0x92					;	(RW | S | P) or (Read/Write | Code/Data segment | Present)
		AT GlobalDescriptorTableEntry_t.LimitHighFlags,	DB 0xCF					;	Flags - 0xC: (DB | G) or (32-bit PM segment | Limit is in 4KiB blocks)
		AT GlobalDescriptorTableEntry_t.AddressHigh,	DB 0x00
	IEND
GdtProtectedModeEnd:

ALIGN(0x0010)

GdtUnrealBigModePointer:	ISTRUC GlobalDescriptorTablePointer_t
	AT GlobalDescriptorTablePointer_t.Limit,		DW GdtUnrealBigModeEnd - GdtUnrealBigModeStart - 1
	AT GlobalDescriptorTablePointer_t.Address,		DD GdtUnrealBigModeStart
IEND

ALIGN(0x0010)

GdtProtectedModePointer:	ISTRUC GlobalDescriptorTablePointer_t
	AT GlobalDescriptorTablePointer_t.Limit,		DW GdtProtectedModeEnd - GdtProtectedModeStart - 1
	AT GlobalDescriptorTablePointer_t.Address,		DD GdtProtectedModeStart
IEND

%ENDIF