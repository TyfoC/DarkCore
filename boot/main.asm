[BITS 16]
[ORG 0x0600]

%DEFINE KernelAddress		0x00020000
%DEFINE MemoryTableAddress	DiskBuffer

Start:
	MOV AX, SI
	AND AL, 0x3F
	MOV [KernelSectorIndex], AL
	MOV [KernelSizeInSectors], EDI

	MOV AX, 0x0003
	INT 0x10

	MOV AX, 0x1003
	MOV BL, 0x00
	INT 0x10

	MOV BL, 0x0B
	MOV SI, MessageLoadingKernel
	CALL Print

	CALL GetLineA20Status
	JNZ LineA20Enabled

	MOV SI, MessageLineA20NotEnabled
	CALL Print

	CALL EnableLineA20

	MOV SI, MessageCantEnableLineA20
	JZ Error
LineA20Enabled:
	MOV SI, MessageLineA20Enabled
	CALL Print

	CALL SwitchUnrealBigMode

	MOV SI, MessageUnrealBigModeEnabled
	CALL Print

	MOV BP, Start
	MOV SP, Start

	CALL SelectDisk

	MOV CX, 0x0000
	MOV CL, [KernelSectorIndex]
	MOV DH, 0x00
	MOV EAX, [KernelSizeInSectors]
	MOV EDI, KernelAddress
	CALL ReadDisk

	MOV SI, MessageKernelLoaded
	CALL Print

	MOV DI, MemoryTableAddress
	CALL GetMemoryMap

	PUSH EAX
	CALL GetPciAccessMethods
	MOV EAX, [PciAccessMethodsValue]
	AND EAX, 0x00000003
	MOV [BootFlags], EAX
	POP EAX

	MOV WORD [MemoryTableLength], CX
	AND DWORD [MemoryTableLength], 0x0000FFFF

	MOV SI, MessageMemoryMapReceived
	CALL Print

	MOV AX, 0x000A
	MOV DX, 0x03D4
	OUT DX, AX
	MOV AX, 0x0020
	MOV DX, 0x03D5
	OUT DX, AX

	MOV EDI, LoadKernel
	JMP SwitchProtectedMode

[BITS 32]
LoadKernel:
	MOV EBP, [KernelAddress + 4]			;	Stack top
	MOV ESP, EBP
	PUSH DWORD [BootFlags]
	PUSH DWORD [MemoryTableLength]
	PUSH MemoryTableAddress
	JMP DWORD [KernelAddress]				;	Entry point
[BITS 16]

Error:
	MOV BL, 0x0C
	CALL Print
	JMP $

MessageLoadingKernel:			DB "[MESSAGE] Loading Kernel:\r\n    ", 0
MessageLineA20Enabled:			DB "[MESSAGE] Line A20 enabled\r\n    ", 0
MessageLineA20NotEnabled:		DB "[MESSAGE] Line A20 not enabled, trying to enable it...\r\n    ", 0
MessageCantEnableLineA20:		DB "[ERROR] Line A20 cannot be enabled!\r\n    ", 0
MessageUnrealBigModeEnabled:	DB "[MESSAGE] Unreal Big Mode enabled\r\n    ", 0
MessageMemoryMapReceived:		DB "[MESSAGE] Memory Map Received\r\n    ", 0
MessageKernelLoaded:			DB "[MESSAGE] Kernel loaded into memory\r\n    ", 0

KernelSectorIndex:			DB 0x00
KernelSizeInSectors:		DD 0x00000000
MemoryTableLength:			DD 0x00000000
BootFlags:					DD 0x00000000

%INCLUDE "video.asm"
%INCLUDE "cpu.asm"
%INCLUDE "memory.asm"
%INCLUDE "disk.asm"
%INCLUDE "pci.asm"

TIMES 0x0800 - $ + $$ DB 0x00

DiskBuffer: