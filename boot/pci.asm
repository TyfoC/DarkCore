%IFNDEF PCI_INC
%DEFINE PCI_INC

;	Function to check access mechanism to PCI
;	Output:
;		AL[0:1]/PciAccessMethodsValue[0:1] - configuration space access mechanism #1 supports
;		AL[1:2]/PciAccessMethodsValue[1:2] - configuration space access mechanism #2 supports
;	
GetPciAccessMethods:
	MOV BYTE [PciAccessMethodsValue], 0x00
	PUSHAD
	PUSHFD
	MOV AX, 0xB101
	INT 0x1A
	JC GetPciAccessMethodsNoMethods
	TEST AH, AH
	JNZ GetPciAccessMethodsNoMethods
GetPciAccessMethodsEnd:
	POPFD
	MOV AH, AL
	AND AH, 0x01
	TEST AH, AH
	JZ GetPciAccessMethodsEndNo2
	OR BYTE [PciAccessMethodsValue], 0x01
GetPciAccessMethodsEndNo2:
	AND AL, 0x02
	TEST AL, AL
	JZ GetPciAccessMethodsRealEnd
	OR BYTE [PciAccessMethodsValue], 0x02
GetPciAccessMethodsRealEnd:
	POPAD
	MOV AL, BYTE [PciAccessMethodsValue]
	RET
GetPciAccessMethodsNoMethods:
	POPFD
	POPAD
	MOV AL, 0x00
	RET
PciAccessMethodsValue:	DB 0x00

%ENDIF