cursor_offset: DW 0
video_base: DW FC00

putc:

	  PSH r1
    PSH r2
    PSH r3
    PSH r4
    PSH r5
    PSH r6

	MOV r2, [cursor_offset]
	XOR r6, r6

	MOV r1, r0
	AND r1, FF00
	CMP r1, r6
	JEQ putc_exit

	MOV r4, r2
	RSH r4, 1
	ADD r4, [video_base]
	MOV r5, [r4]

	MOV r4, FF

	MOV r3, 1
	AND r3, r2
	CMP r3, r6
	JEQ putc_even
	RSH r1, 8
	LSH r4, 8
	putc_even:
	AND r5, r4
	OR r5, r1

	ADD r2, 1

	putc_ptb:
	MOV r1, r0
	AND r1, FF

	CMP r1, r6
	JEQ putc_exit

	MOV r4, FF00

	MOV r3, 1
	AND r3, r2
	CMP r3, r6
	JGT putc_odd
	LSH r1, 8
	RSH r4, 8
	putc_odd:
	AND r5, r4
	OR r5, r1

	ADD r2, 1

	putc_exit:
		MOV r4, [cursor_offset]
		RSH r4, 1
		ADD r4, [video_base]
		MOV [r4], 5353
		MOV [cursor_offset], r2

		POP r6
		POP r5
		POP r4
		POP r3
		POP r2
		POP r1

RET

print:

    PSH r1
    PSH r2
    PSH r3
    PSH r4
    PSH r5
    PSH r6

    XOR r1, r1

    print_while_a:
        MOV r2, r0
        ADD r2, r1
        MOV r3, [r2]

        CMP r3, 0
        JEQ print_wend_a

		PSH r0
	    MOV r0, r3
		CAL putc
		POP r0
        ADD r1, 1

	    LSH r3, 8
        CMP r3, 0
        JEQ print_wend_a

        JMP print_while_a
    print_wend_a:

    POP r6
    POP r5
    POP r4
    POP r3
    POP r2
    POP r1

RET
