[BITS 16]
[ORG 0x7C00]

STRUC PartitionTableEntry_t
	.Flags:											RESB 0x01
	.FirstHead:										RESB 0x01
	.FirstSectorCylinder:							RESW 0x01
	.SystemIdentifier:								RESB 0x01
	.LastHead:										RESB 0x01
	.LastSectorCylinder:							RESW 0x01
	.RelativeSector:								RESD 0x01
	.SectorsCount:									RESD 0x01
ENDSTRUC

%MACRO CreatePartitionTableEntry	1
PartitionTableEntry%+%1:	ISTRUC PartitionTableEntry_t
	AT PartitionTableEntry_t.Flags,					DB 0x00
	AT PartitionTableEntry_t.FirstHead,				DB 0x00
	AT PartitionTableEntry_t.FirstSectorCylinder,	DW 0x00
	AT PartitionTableEntry_t.SystemIdentifier,		DB 0x00
	AT PartitionTableEntry_t.LastHead,				DB 0x00
	AT PartitionTableEntry_t.LastSectorCylinder,	DW 0x00
	AT PartitionTableEntry_t.RelativeSector,		DD 0x00000000
	AT PartitionTableEntry_t.SectorsCount,			DD 0x00000000
IEND
%ENDMACRO

Start:
	JMP 0x0000:LoadRegisters
LoadRegisters:
	MOV AX, 0x0000
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV SS, AX
	MOV BP, Start
	MOV SP, Start

	STI
	CLD

	MOV SI, (((KernelStart - Start + 0x01FF) & ~0x01FF) / 0x0200) + 1
	MOV EDI, (((KernelEnd - KernelStart + 0x01FF) & ~0x01FF) / 0x0200)

	MOV AX, 0x0200 + (((MainEnd - MainStart + 0x01FF) & ~0x01FF) / 0x0200)
	MOV BX, 0x0600
	MOV CX, 0x0003
	MOV DH, 0x00
	INT 0x13
	JNC 0x0600
	INT 0x19
	JMP $

TIMES 0x01BE - $ + $$ DB 0

CreatePartitionTableEntry	1
CreatePartitionTableEntry	2
CreatePartitionTableEntry	3
CreatePartitionTableEntry	4

DW 0xAA55

FSHeader:
	.Signature:					DB	"LITEFS"
	.Revision:					DW	0x0000
	.CurrentPartitionsCount:	DQ	0x0000000000000001
	.Partitions:
		DQ		(((PartitionHeader - FSHeader + 511) & ~511) / 512)
		TIMES	0x1E8 DB 0

MainStart:
INCBIN "main.bin"
MainEnd:

KernelStart:
INCBIN "kernel.bin"
KernelEnd:

PartitionHeader:
INCBIN "lite-fs-image.pbd"