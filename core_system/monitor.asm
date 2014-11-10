JMP main

char_x_ptr: DW 2000
char_y_ptr: DW 2001

putc:
PSH r1
PSH r2
PSH r3
PSH r4
PSH r5
PSH r6

MOV r5, r0
AND r5, FF00
CMP r5, 0
JEQ putc_return

'Set r1 = x and r6 = y
MOV r2, [char_x_ptr]
MOV r1, [r2]
MOV r2, [char_y_ptr]
MOV r6, [r2]

'Calculate the video memory cell
MOV r3, r6
MLT r3, 50 'y*width
ADD r3, r1 '+x
DIV r3, 2
ADD r3, FC00

MOV r5, r0
AND r5, FF00
CMP r5, 0A00
JEQ newline_1

MOV r2, r6
MLT r2, 50
ADD r2, r1
AND r2, 1
CMP r2, 1
JEQ char_pos_odd

	MOV r4, [r3]
	AND r4, 00FF

	MOV r5, r0
	AND r5, FF00
	ADD r4, r5
	MOV [r3], r4
	JMP endif

char_pos_odd:

	MOV r4, [r3]
	AND r4, FF00

	MOV r5, r0
	RSH r5, 8
	ADD r4, r5
	MOV [r3], r4

endif:
ADD r1, 1 'x++
CMP r1, 50
JLT no_y_inc
XOR r1, r1
ADD r6, 1 'y++
CMP r6, 19
JLT no_y_inc
XOR r6, r6
CAL clear

no_y_inc:
JMP do_char_2

newline_1:
XOR r1, r1 'x = 0
ADD r6, 1  'y++
CMP r6, 19
JLT do_char_2
XOR r6, r6
CAL clear

do_char_2:
'Stash the current X and Y
MOV r2, [char_x_ptr]
MOV [r2], r1
MOV r2, [char_y_ptr]
MOV [r2], r6

MOV r5, r0
AND r5, 00FF
CMP r5, 0
JEQ putc_return

MOV r5, r0
AND r5, 00FF
CMP r5, 00A0
JEQ newline_2

'Calculate the video memory cell
MOV r3, r6
MLT r3, 50 'y*width
ADD r3, r1 '+x
DIV r3, 2
ADD r3, FC00

MOV r2, r6
MLT r2, 50
ADD r2, r1
AND r2, 1
CMP r2, 1
JEQ char_pos_odd_2

	MOV r4, [r3]
	AND r4, 00FF

	MOV r5, r0
	LSH r5, 8
	ADD r4, r5
	MOV [r3], r4
	JMP endif_2

char_pos_odd_2:

	MOV r4, [r3]
	AND r4, FF00

	MOV r5, r0
	AND r5, 00FF
	ADD r4, r5
	MOV [r3], r4

endif_2:
ADD r1, 1 'x++
CMP r1, 50
JLT no_y_inc_2
XOR r1, r1
ADD r6, 1 'y++
CMP r6, 19
JLT no_y_inc_2
XOR r6, r6
CAL clear

no_y_inc_2:
JMP putc_return

newline_2:
XOR r1, r1 'x = 0
ADD r6, 1  'y++
CMP r6, 19
JLT putc_return
XOR r6, r6
CAL clear

putc_return:
MOV r2, [char_x_ptr]
MOV [r2], r1
MOV r2, [char_y_ptr]
MOV [r2], r6
POP r6
POP r5
POP r4
POP r3
POP r2
POP r1
RET

prints:
PSH r0
PSH r1
PSH r2
MOV r1, r0
prints_top:
	MOV r2, [r1]
	CMP r2, 0
	JEQ prints_exit
	MOV r0, r2
	CAL putc
	MOV r0, r2
	AND r0, 00FF
	CMP r0, 0
	JEQ prints_exit
	ADD r1, 1
	JMP prints_top
prints_exit:
	POP r2
	POP r1
	POP r0
	RET

getc:
	MOV r0, [FFFE]
	CMP r0, 0
	JEQ getc
	MOV r0, [FFFF]
	RET

clear:
PSH r0
PSH r1
PSH r2
PSH r3
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
	POP r3
	POP r2
	POP r1
	POP r0
	RET

'This function takes a two-char hext string in r0 and replaces
'that string with the byte value it represents
hex2byte:

	PSH r1 	'r1 = the working value
	XOR r1, r1

  'Move the string into r2 and reduce it to the LSB
	PSH r2
	MOV r2, r0
	AND r2, 00FF

	'Test to find r2 in 0-9
	'Is r2 less than '0'?
	CMP r2, 0030
	JLT h2b_fail 'Fail if it is
	'Is r2 greater than '9'?
	CMP r2, 0039
	JGT lsb_in_letters
	'r2 is between '0' and '9', so we can subtract '0' and add the result to the total
	SUB r2, 0030
	JMP do_msb

	lsb_in_letters:
	'Test to find r2 in a-f
	'Is r2 less than 'a'?
	CMP r2, 0061
	JLT h2b_fail 'Fail if it is
	'Is r2 greater than 'f'?
	CMP r2, 0066
	JGT h2b_fail 'Fail if it is
	'r2 is between 'a' and 'f', so we can subtract ('a' + 0xa) and add the result to the total
	SUB r2, 0061
	ADD r2, 000A

	do_msb:
	MOV r1, r2 'Put LSB into the accumulator

	'Move the string into r2 and reduce it to the MSB
	MOV r2, r0
	RSH r2, 8

	'The below tests are pretty much identical to the ones above,
	'so this could really stand to be refactored
	'Test to find r2 in 0-9
	'Is r2 less than '0'?
	CMP r2, 0030
	JLT h2b_fail 'Fail if it is
	'Is r2 greater than '9'?
	CMP r2, 0039
	JGT msb_in_letters
	'r2 is between '0' and '9', so we can subtract '0' and add the result to the total
	SUB r2, 0030
	JMP h2b_success

	msb_in_letters:
	'Test to find r2 in a-f
	'Is r2 less than 'a'?
	CMP r2, 0061
	JLT h2b_fail 'Fail if it is
	'Is r2 greater than 'f'?
	CMP r2, 0066
	JGT h2b_fail 'Fail if it is
	'r2 is between 'a' and 'f', so we can subtract ('a' + 0xa) and add the result to the total
	SUB r2, 0061
	ADD r2, 000A

	h2b_success:
		LSH r2, 4
		ADD r1, r2
		MOV r0, r1
		JMP h2b_return

	h2b_fail:
		XOR r0, r0

	h2b_return:
		POP r2
		POP r1
		RET

'Does the opposite of hex2byte
byte2hex:

	'r1 will be the working reg
	'r2 will be the output reg
	PSH r1
	PSH r2

	'Convert low nybble to char
	MOV r1, r0
	AND r1, 000F
	CMP r1, 0009
	JGT lsn_in_letters
	ADD r1, 0030
	JMP do_msn
	lsn_in_letters:
	ADD r1, 0057

	do_msn:
	MOV r2, r1

	'Convert high nybble to char
	MOV r1, r0
	AND r1, 00F0
	RSH r1, 4
	CMP r1, 0009
	JGT msn_in_letters
	ADD r1, 0030
	JMP b2h_return
	msn_in_letters:
	ADD r1, 0057

	b2h_return:
	LSH r1, 8
	ADD r2, r1
	MOV r0, r2
	POP r2
	POP r1
	RET

'output the word given in r0 in hex format
print_word:

	PSH r1
	MOV r1, r0
	RSH r0, 8
	CAL byte2hex
	CAL putc
	MOV r0, r1
	CAL byte2hex
	CAL putc
	POP r1
	RET

prompt:
DW 3E20
DW 0000

bad_cmd_msg:
DW 0A20
DW 6261
DW 6420
DW 636F
DW 6D6D
DW 616E
DW 642E
DW 0A00

byte_buf_ptr: DW 2002

main:
    LSP 1000

    CAL clear

main_loop:
    MOV r0, prompt
    CAL prints

type_top:
		CAL getc
		CAL putc
		AND r0, FF00 'Yeah, we'll just fuckin' ignore the next key in the buffer
		CMP r0, 7200 'r\0'
		JEQ read_address
		CMP r0, 7700 'w\0'
		JEQ write_address
		CMP r0, 7100 'q\0'
		JEQ quit
		CMP r0, 6A00 'j\0'
		JEQ jump
		MOV r0, bad_cmd_msg
		CAL prints
		JMP main_loop

		read_address:
		MOV r0, 0A00
		CAL putc
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		MOV r3, r0
		LSH r3, 8
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		ADD r3, r0
		MOV r0, 0A00
		CAL putc
		MOV r0, [r3]
		CAL print_word
		MOV r0, 0A00
		CAL putc
		JMP main_loop



		write_address:
		MOV r0, 0A00
		CAL putc
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		MOV r3, r0
		LSH r3, 8
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		ADD r3, r0
		MOV r0, 0A00
		CAL putc

		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		MOV r4, r0
		LSH r4, 8
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		ADD r4, r0
		MOV r0, 0A00
		CAL putc

		MOV [r3], r4
		JMP main_loop

		jump:
		MOV r0, 0A00
		CAL putc
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		MOV r3, r0
		LSH r3, 8
		CAL getc
		CAL putc
		MOV r1, r0
		AND r1, FF00
		CAL getc
		CAL putc
		MOV r2, r0
		RSH r2, 8
		ADD r1, r2
		MOV r0, r1
		CAL hex2byte
		ADD r3, r0
		MOV r0, 0A00
		CAL putc
		MOV [2004], 0C00
		MOV [2005], r3
		jmpline: JMP 2004

		quit:
    	HLT
