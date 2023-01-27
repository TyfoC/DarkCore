[BITS 32]
[EXTERN DarkCore]
[GLOBAL DarkCoreEntry]

DarkCoreEntry:
	CALL DarkCore
	JMP $