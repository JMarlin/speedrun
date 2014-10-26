JMP main

string:
DW 4865
DW 0000

clear:
    MOV r0, 0000
	MOV r1, FC00
	XOR r2, r2
	
	clear_top:
	    MOV r3, r1
		ADD r3, r2
	    MOV [r3], r0
	    ADD r2, 1
	    CMP r2, 3E7
	    JGT clear_bottom
	    JMP clear_top
	clear_bottom:

	RET
	
main:
    LSP 1000
 
    CAL clear
 
    MOV r0, [string]
    CAL print

    HLT
    

