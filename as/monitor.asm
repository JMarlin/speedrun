main:
    XOR r0, r0

    loop_a:
        MOV r1, [FFFE]
        CMP r0, r1
        JEQ break_a
        JMP loop_a

break_a:
    MOV r1, [FFFF]
    MOV [FFFF], r1
    HLT
 
