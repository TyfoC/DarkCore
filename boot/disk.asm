%IFNDEF DISK_INC
%DEFINE DISK_INC

;	Function to select disk
;	Input:
;		DL - index
SelectDisk:
	MOV [DiskIndex], DL
	PUSH AX
	PUSH CX
	PUSH DX
	PUSH BX
	PUSH DI
	PUSH ES
	
	MOV AH, 0x08
	PUSH 0x0000
	PUSH 0x0000
	POP ES
	POP DI
	INT 0x13

	MOV [LastHeadIndex], DH
	MOV [LastSectorIndex], CL
	AND BYTE [LastSectorIndex], 0x3F
	MOV [LastCylinderIndex], CH
	MOV [LastCylinderIndex + 1], CL
	SHR BYTE [LastCylinderIndex + 1], 0x06
	
	POP ES
	POP DI
	POP BX
	POP DX
	POP CX
	POP AX
	RET

;	Function to read disk
;	Input:
;		DH - head index
;		CL[0:5] - sector index
;		CH, CL[6:7] - cylinder index
;		EAX - count of sectors to read
;		EDI - destination buffer
ReadDisk:
	PUSHAD
	PUSH ES
	PUSH 0x0000
	POP ES

	PUSH ECX
	PUSH EDX
	MOV EDX, 0x00000000
	MOV CL, [LastSectorIndex]
	AND ECX, 0x0000003F
	DIV ECX
	MOV [ReadDiskIntPartsCount], EAX
	MOV [ReadDiskLastPartSize], DL
	POP EDX
	POP ECX

	ReadDiskLoop:
		CMP DWORD [ReadDiskIntPartsCount], 0x00000000
		JE ReadDiskLoopDone

		MOV AH, 0x02
		MOV AL, [LastSectorIndex]
		MOV DL, [DiskIndex]
		MOV BX, DiskBuffer
		INT 0x13
		JC ReadDiskError

		PUSH CX
		PUSH DX

		MOV AX, 0x0200
		MOV CL, [LastSectorIndex]
		AND CX, 0x003F
		MUL CX
		MOV DX, 0x0000
		SHR AX, 2
		MOV CX, AX

		MOV SI, BX
		AND ESI, 0x0000FFFF
		ReadDiskLoopCopy:
			MOV EAX, [ESI]
			MOV [EDI], EAX
			ADD ESI, 0x04
			ADD EDI, 0x04
			LOOP ReadDiskLoopCopy

		POP DX
		POP CX

		CMP BYTE [LastHeadIndex], DH
		JLE ReadDiskChsIncCylinder
		INC DH
		JMP ReadDiskChsIncremented
ReadDiskChsIncCylinder:
		MOV BH, CL
		AND BH, 0x3F
		MOV AH, CL
		SHR AH, 0x06
		MOV AL, CH
		INC AX
		MOV CH, AL
		MOV CL, AH
		SHL CL, 0x06
		OR CL, BH
		MOV DH, 0x00
ReadDiskChsIncremented:
		DEC DWORD [ReadDiskIntPartsCount]
		JMP ReadDiskLoop
ReadDiskLoopDone:
	MOV AL, [ReadDiskLastPartSize]
	TEST AL, AL
	JZ ReadDiskDone
	MOV AH, 0x02
	MOV AL, [ReadDiskLastPartSize]
	MOV DL, [DiskIndex]
	MOV BX, DiskBuffer
	INT 0x13
	JC ReadDiskError

	MOV AX, 0x0200
	MOV CL, [ReadDiskLastPartSize]
	MUL CX
	MOV DX, 0x0000
	SHR AX, 2
	MOV CX, AX
	MOV SI, BX
	AND ESI, 0x0000FFFF
	ReadDiskLoopDoneCopy:
		MOV EAX, [ESI]
		MOV [EDI], EAX
		ADD ESI, 0x04
		ADD EDI, 0x04
		LOOP ReadDiskLoopDoneCopy
	JMP ReadDiskDone
ReadDiskError:
	STC
ReadDiskDone:
	POP ES
	POPAD
	RET

ReadDiskIntPartsCount:	DD 0x00
ReadDiskLastPartSize:	DB 0x00

DiskIndex:				DB 0x00
LastSectorIndex:		DB 0x00
LastHeadIndex:			DB 0x00
LastCylinderIndex:		DW 0x0000

%ENDIF