JMP main

counter: DW 0
offset: DW 12
cap: DW 37

idle_input:
    XOR r0, r0
	
	idle_input_loop_a:
	    MOV r1, [FFFE]
	    CMP r1, r0
		JEQ idle_input_loop_a_break
		JMP idle_input_loop_a
	idle_input_loop_a_break:
	
RET

main: 
	MOV r1, [offset]
	MOV [counter], r1

    main_loop_a:
		CAL idle_input
		MOV r1, [counter]
		ADD r1, 1
		MOV r2, [cap]
		CMP r1, r2
		JEQ main_loop_a_breal
		JMP main_loop_a
	main_loop_a_break:
	
HLT
	
		