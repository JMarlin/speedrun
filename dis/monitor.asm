XOR r0, r0
MOV r1, [FFFE]
CMP r0, r1
JEQ 0008
JMP 0001
MOV r1, [FFFF]
MOV [FFFF], r1
HLT
JMP 0001
