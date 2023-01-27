%IFNDEF MEMORY_INC
%DEFINE MEMORY_INC

STRUC MemoryMapEntry_t
	.Address:		RESQ 0x01
	.BytesCount:	RESQ 0x01
	.Type:			RESD 0x01
	.Attributes:	RESD 0x01
ENDSTRUC

STRUC PerformedMemoryMapEntry_t
	.Address:		RESQ 0x01
	.BytesCount:	RESQ 0x01
	.Type:			RESD 0x01
ENDSTRUC

;	Function to get available memory size in first MB of memory through int 0x12
;	Output:
;		EAX - count of available memory in first MB of memory in bytes
GetMemorySizeBeforeMb:
	CLC
	MOV EAX, 0x00000000
	INT 0x12
	SHL EAX, 0x0A
	AND EAX, 0x07FFFF
	RET

;	Function to get available memory size between 1MB and 16MB
;	Output:
;		EAX - count of available memory between 1MB and 16MB in bytes
GetMemorySizeBetween1MbAnd16Mb:
	CLC
	MOV CX, 0x0000
	MOV AX, 0xE801
	INT 0x15
	JC GetMemorySizeBetween1MbAnd16MbError
	CMP AH, 0x86
	JE GetMemorySizeBetween1MbAnd16MbError
	CMP AH, 0x80
	JE GetMemorySizeBetween1MbAnd16MbError
	JCXZ GetMemorySizeBetween1MbAnd16MbDone
	MOV AX, CX
	JMP GetMemorySizeBetween1MbAnd16MbDone
GetMemorySizeBetween1MbAnd16MbError:
	STC
GetMemorySizeBetween1MbAnd16MbDone:
	SHL EAX, 0x10
	AND EAX, 0x00FF0000
	RET

;	Function to get normal memory map
;	Input:
;		ES:DI - table address
;	Output:
;		ES:DI - filled table
;		CX - entries count
GetGreatMemoryMap:
	PUSHAD
	PUSH ES
	MOV DWORD [GetGreatMemoryMapCount], 0x0000
	PUSH ES
	POP WORD [GetGreatMemoryMapEs]
	PUSH DI
	POP WORD [GetGreatMemoryMapDi]

	CLC
	PUSH 0x0000
	POP ES
	MOV DI, MemoryMapEntryBuffer
	MOV EAX, 0x0000E820
	MOV ECX, 0x00000018
	MOV EDX, 0x534D4150
	MOV EBX, 0x00000000
	INT 0x15
	JC GetGreatMemoryMapError
	CMP EAX, 0x534D4150
	JNE GetGreatMemoryMapError

	MOV EBX, 0x00000000
	GetGreatMemoryMapLoop:
		MOV EAX, 0x0000E820
		MOV ECX, 0x00000018
		MOV EDX, 0x534D4150
		INT 0x15

		PUSHF

		CMP CL, 0x18
		JNE GetGreatMemoryMapFixed
		TEST DWORD [ES:DI + MemoryMapEntry_t.Attributes], 0x00000001
		JNZ GetGreatMemoryMapFixed
		MOV DWORD [ES:DI + MemoryMapEntry_t.Type], 0x00000002
	GetGreatMemoryMapFixed:
		INC WORD [GetGreatMemoryMapCount]

		PUSH ES
		PUSH DI
		PUSH WORD [GetGreatMemoryMapEs]
		PUSH WORD [GetGreatMemoryMapDi]
		POP DI
		POP ES
		MOV EAX, [MemoryMapEntryBuffer]
		MOV [ES:DI], EAX
		MOV EAX, [MemoryMapEntryBuffer + 4]
		MOV [ES:DI + 4], EAX
		MOV EAX, [MemoryMapEntryBuffer + 8]
		MOV [ES:DI + 8], EAX
		MOV EAX, [MemoryMapEntryBuffer + 12]
		MOV [ES:DI + 12], EAX
		MOV EAX, [MemoryMapEntryBuffer + 16]
		MOV [ES:DI + 16], EAX
		ADD WORD [GetGreatMemoryMapDi], 0x0014
		POP DI
		POP ES
		
		POPF
		JC GetGreatMemoryMapEnd
		TEST EBX, EBX
		JZ GetGreatMemoryMapEnd

		MOV EAX, 0x0000E820
		MOV ECX, 0x00000018
		MOV EDX, 0x534D4150
		JMP GetGreatMemoryMapLoop
GetGreatMemoryMapError:
	STC
	POP ES
	POPAD
	MOV CX, 0x0000
	RET
GetGreatMemoryMapEnd:
	POP ES
	POPAD
	MOV CX, [GetGreatMemoryMapCount]
	RET
GetGreatMemoryMapCount:	DW 0x0000
GetGreatMemoryMapEs:	DW 0x0000
GetGreatMemoryMapDi:	DW 0x0000

;	Function to get full memory map through all functions
;	Input:
;		ES:DI - table address
;	Output:
;		CX - entries count
GetMemoryMap:
	CALL GetGreatMemoryMap
	JCXZ GetMemoryMapOldMethod
	JNC GetMemoryMapEnd
GetMemoryMapOldMethod:
	MOV CX, 0x0000
	CALL GetMemorySizeBeforeMb
	JC GetMemoryMapOldMethodNext
	INC CX
	MOV DWORD [ES:DI], 0x00000500
	MOV DWORD [ES:DI + 4], 0
	MOV DWORD [ES:DI + 8], EAX
	MOV DWORD [ES:DI + 12], 0
	MOV DWORD [ES:DI + 16], 0x00000001
	ADD DI, 0x0014
GetMemoryMapOldMethodNext:
	CALL GetMemorySizeBetween1MbAnd16Mb
	JC GetMemoryMapEnd
	INC CX
	MOV DWORD [ES:DI], 0x00100000
	MOV DWORD [ES:DI + 4], 0
	MOV DWORD [ES:DI + 8], EAX
	MOV DWORD [ES:DI + 12], 0
	MOV DWORD [ES:DI + 16], 0x00000001
	SUB DI, 0x0014
GetMemoryMapEnd:
	RET


ALIGN(0x10)

MemoryMapEntryBuffer:	ISTRUC MemoryMapEntry_t
	AT MemoryMapEntry_t.Address,		DQ 0x0000000000000000
	AT MemoryMapEntry_t.BytesCount,		DQ 0x0000000000000000
	AT MemoryMapEntry_t.Type,			DD 0x00000000
	AT MemoryMapEntry_t.Attributes,		DD 0x00000000
IEND

%ENDIF