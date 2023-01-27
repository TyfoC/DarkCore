[BITS 32]
[EXTERN UserModeCallHandler]
[EXTERN VMMConvertVirtualAddressToPhysical]	;	VMMConvertVirtualAddressToPhysical(VMM::Directory* dir, size_t virtualAddress)
[GLOBAL UserModeInterruptCallHandler]

%DEFINE STACK_BUFFER_SIZE						0x100

UserModeInterruptCallHandlerVirtualESP:			DD 0x00000000
UserModeInterruptCallHandlerVirtualEBP:			DD 0x00000000
UserModeInterruptCallHandlerPhysicalESP:		DD 0x00000000
UserModeInterruptCallHandlerPhysicalEBP:		DD 0x00000000
UserModeInterruptCallHandlerRegisterBuffer:		DD 0x00000000					;	one-register buffer

ALIGN(4)
UserModeInterruptCallHandlerStackTop:
	times STACK_BUFFER_SIZE	DB 0x00	;	stack buffer for VMMConvertVirtualAddressToPhysical
UserModeInterruptCallHandlerStackBottom:
UserModeInterruptCallHandler:
	CLI														;	(1)		disable hardware IRQs

	MOV [UserModeInterruptCallHandlerRegisterBuffer], EAX	;	(2)		save EAX

	MOV [UserModeInterruptCallHandlerVirtualESP], ESP		;	(3)		save virtual ESP, EBP
	MOV [UserModeInterruptCallHandlerVirtualEBP], EBP

	MOV ESP, UserModeInterruptCallHandlerStackBottom - 4	;	(4)		load temp stack
	MOV EBP, ESP

	PUSHA
	PUSH DWORD [UserModeInterruptCallHandlerVirtualESP]		;	(5)		save physical ESP, EBP
	MOV EAX, CR3
	PUSH EAX
	CALL VMMConvertVirtualAddressToPhysical
	MOV [UserModeInterruptCallHandlerPhysicalESP], EAX
	POP EAX
	POP EAX
	PUSH DWORD [UserModeInterruptCallHandlerVirtualEBP]
	MOV EAX, CR3
	PUSH EAX
	CALL VMMConvertVirtualAddressToPhysical
	MOV [UserModeInterruptCallHandlerPhysicalEBP], EAX
	POP EAX
	POP EAX
	POPA

	MOV EAX, CR0											;	(6)		disable VM
	AND EAX, 0x7FFFFFFF
	MOV CR0, EAX

	MOV ESP, [UserModeInterruptCallHandlerPhysicalESP]		;	(7)		load physical ESP, EBP
	MOV EBP, [UserModeInterruptCallHandlerPhysicalESP]

	MOV EAX, [UserModeInterruptCallHandlerRegisterBuffer]	;	(8)		restore EAX

	PUSH 0													;	(9)		push registers
	PUSH DWORD 0xEC
	PUSHA
	PUSH DS
	PUSH ES
	PUSH FS
	PUSH GS
	MOV AX, 0x10
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX
	MOV EAX, ESP
	PUSH ESP

	CALL UserModeCallHandler								;	(10)	call UserModeCallHandler & do something

	POP EAX													;	(11)	pop registers
	POP GS
	POP FS
	POP ES
	POP DS
	POPA

	ADD ESP, 8												;	(12)	pop ErrorCode, InterruptIndex

	MOV ESP, [UserModeInterruptCallHandlerVirtualESP]		;	(13)	load virtual ESP, EBP
	MOV EBP, [UserModeInterruptCallHandlerVirtualEBP]

	MOV [UserModeInterruptCallHandlerRegisterBuffer], EAX	;	(14)		save EAX

	MOV EAX, CR0											;	(15)	enable VM
	OR EAX, 0x80000000
	MOV CR0, EAX

	MOV EAX, [UserModeInterruptCallHandlerRegisterBuffer]	;	(16)		restore EAX

	STI														;	(17)	enable hardware IRQs

	IRET													;	(18)	return