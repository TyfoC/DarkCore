%IFNDEF VIDEO_INC
%DEFINE VIDEO_INC

;	Function to print string from real mode
;	Input:
;		DS:SI - string
;		BL - color
Print:
	PUSH AX
	PUSH CX
	PUSH DX
	PUSH SI
	PUSH DS
	PrintLoop:
		LODSB
		TEST AL, AL
		JZ PrintEnd
		CMP AL, '\'
		JE PrintSpecial
		MOV AH, 0x09
		CALL GetCurrentVideoPage
		MOV CX, 0x01
		INT 0x10

		CALL GetVgaCursorPosition
		CMP DL, 0x4f
		JGE PrintIncrementLine
		INC DL
		CALL SetVgaCursorPosition

		JMP PrintLoop
PrintIncrementLine:
	CALL IncermentVgaCursorColumn
	JMP PrintLoop
PrintSpecial:
	LODSB
	CMP AL, 'r'
	JE PrintSpecialCarRet
	CMP AL, 'n'
	JE PrintSpecialNewLine
	TEST AL, AL
	JZ PrintEnd
	JMP PrintLoop
PrintSpecialCarRet:
	CALL GetVgaCursorPosition
	MOV DL, 0x00
	CALL SetVgaCursorPosition
	JMP PrintLoop
PrintSpecialNewLine:
	CALL IncrementVgaCursorLine
	JMP PrintLoop
PrintEnd:
	POP DS
	POP SI
	POP DX
	POP CX
	POP AX
	RET

;	Function to get current video page
;	Output:
;		BH - current video page
GetCurrentVideoPage:
	PUSH AX
	MOV AH, 0x0F
	INT 0x10
	POP AX
	RET

;	Function to get vga cursor position
;	Output:
;		DH - line (from 0)
;		DL - column (from 0)
GetVgaCursorPosition:
	PUSH AX
	PUSH BX
	MOV AH, 0x03
	CALL GetCurrentVideoPage
	INT 0x10
	POP BX
	POP AX
	RET

;	Function to set vga cursor position
;	Input:
;		DH - line
;		DL - column
SetVgaCursorPosition:
	PUSH AX
	PUSH BX
	CALL GetCurrentVideoPage
	MOV AH, 0x02
	INT 0x10
	POP BX
	POP AX
	RET

;	Function to increment vga cursor column
IncermentVgaCursorColumn:
	PUSH DX
	CALL GetVgaCursorPosition
	CMP DL, 0x4f
	JGE IncermentVgaCursorColumnLine
	INC DL
	CALL SetVgaCursorPosition
	JMP IncermentVgaCursorColumnEnd
IncermentVgaCursorColumnLine:
	CMP DH, 0x18
	JGE IncermentVgaCursorColumnShiftLine
	INC DH
	MOV DL, 0x00
	CALL SetVgaCursorPosition
	JMP IncermentVgaCursorColumnEnd
IncermentVgaCursorColumnShiftLine:
	PUSH AX
	PUSH CX
	PUSH BX
	MOV AX, 0x0601
	MOV BH, 0x07
	MOV CX, 0x0000
	MOV DX, 0x184f
	INT 0x10
	POP BX
	POP CX
	POP AX
IncermentVgaCursorColumnEnd:
	POP DX
	RET

;	Function to increment vga cursor line
IncrementVgaCursorLine:
	PUSH DX
	CALL GetVgaCursorPosition
	CMP DH, 0x18
	JGE IncrementVgaCursorLineShift
	INC DH
	JMP IncrementVgaCursorLineEnd
IncrementVgaCursorLineShift:
	PUSH AX
	PUSH CX
	PUSH BX
	MOV AX, 0x0601
	MOV BH, 0x07
	MOV CX, 0x0000
	MOV DX, 0x184f
	INT 0x10
	POP BX
	POP CX
	POP AX
	MOV DL, 0x00
IncrementVgaCursorLineEnd:
	CALL SetVgaCursorPosition
	POP DX
	RET

%ENDIF