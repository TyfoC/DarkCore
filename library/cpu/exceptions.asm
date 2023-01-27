[BITS 32]
[EXTERN GlobalExceptionsRoutine]

%MACRO ExceptionAsErrorStub	1
[GLOBAL ExceptionStub%+%1]
ExceptionStub%+%1:
	PUSH BYTE %1
	JMP GlobalExceptionsRoutineStub
%ENDMACRO
%MACRO ExceptionAsWarningStub	1
[GLOBAL ExceptionStub%+%1]
ExceptionStub%+%1:
	PUSH 0
	PUSH BYTE %1
	JMP GlobalExceptionsRoutineStub
%ENDMACRO

ExceptionAsWarningStub	0
ExceptionAsWarningStub	1
ExceptionAsWarningStub	2
ExceptionAsWarningStub	3
ExceptionAsWarningStub	4
ExceptionAsWarningStub	5
ExceptionAsWarningStub	6
ExceptionAsWarningStub	7
ExceptionAsErrorStub	8
ExceptionAsWarningStub	9
ExceptionAsErrorStub	10
ExceptionAsErrorStub	11
ExceptionAsErrorStub	12
ExceptionAsErrorStub	13
ExceptionAsErrorStub	14
ExceptionAsWarningStub	15
ExceptionAsWarningStub	16
ExceptionAsErrorStub	17
ExceptionAsWarningStub	18
ExceptionAsWarningStub	19
ExceptionAsWarningStub	20
ExceptionAsErrorStub	21
ExceptionAsWarningStub	22
ExceptionAsWarningStub	23
ExceptionAsWarningStub	24
ExceptionAsWarningStub	25
ExceptionAsWarningStub	26
ExceptionAsWarningStub	27
ExceptionAsWarningStub	28
ExceptionAsErrorStub	29
ExceptionAsErrorStub	30
ExceptionAsWarningStub	31

GlobalExceptionsRoutineStub:
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
	CALL GlobalExceptionsRoutine
	POP EAX
	POP GS
	POP FS
	POP ES
	POP DS
	POPA
	ADD ESP, 8
	IRET