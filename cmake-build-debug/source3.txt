	.ORIG x3000 
A 	LEA R1, Y
	AND R0, R1, R2	;new
	AND R0, R1, #4	;new
	BRN AY		;new
	BRZ SUP		;new
	BRNP GORL	;new
	JMP R0		;new
	RET		;new
	JSR A		;new
W2	JSRR R0		;new
	LDB R0, R1, #12
	NOT R0;		;new
	RTI
	LSHF R4, R5, #6	;new
	RSHFL R7, R6, #2;new
	RSHFA R3, R2, #4;new
	STB R2, R5, #5	;new	
	TRAP x25	;new
	STW R4, R3, #12	;new
	XOR R0, R3, #3	;new
	XOR R0, R1, R3	;new
	NOT R3, R4	;new
       	LDW R1, R1, #0
       	LDW R1, R1, #0
AY      ADD R1, R1, R1
	ADD R1, R1, x-10
SUP     BRN A
GORL    HALT
Y 	.FILL #263 
	.FILL #13
	.FILL #6 
	.END