[BITS 32]
[GLOBAL _start]
[EXTERN main]

_start:
	CALL main
	MOV EAX, 0x00000002			;	exit thread
	INT 0xEC
	JMP $