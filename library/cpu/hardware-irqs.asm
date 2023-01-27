[BITS 32]
[EXTERN GlobalHardwareInterruptsRoutine]

%MACRO HardwareInterruptStub	1
[GLOBAL HardwareInterruptStub%+%1]
HardwareInterruptStub%+%1:
	PUSH 0
	PUSH BYTE %1+32
	JMP GlobalHardwareInterruptsRoutineStub
%ENDMACRO

HardwareInterruptStub	0
HardwareInterruptStub	1
HardwareInterruptStub	2
HardwareInterruptStub	3
HardwareInterruptStub	4
HardwareInterruptStub	5
HardwareInterruptStub	6
HardwareInterruptStub	7
HardwareInterruptStub	8
HardwareInterruptStub	9
HardwareInterruptStub	10
HardwareInterruptStub	11
HardwareInterruptStub	12
HardwareInterruptStub	13
HardwareInterruptStub	14
HardwareInterruptStub	15

GlobalHardwareInterruptsRoutineStub:
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
	CALL GlobalHardwareInterruptsRoutine
	POP EAX
	POP GS
	POP FS
	POP ES
	POP DS
	POPA
	ADD ESP, 8
	IRET