	.ORIG #4096 
A 	LEA R1, Y
       	LDW R1, R1, #0
       	LDW R1, R1, #0
       	ADD R1, R1, R1
	ADD R1, R1, x-10
       	BRN A
       	HALT
Y 	.FILL #263 
	.FILL #13
	.FILL #6 
	.END