	XOR	r1, r1
	XOR	r2, r2
	XOR	r3, r3
	WB2	r1, 0x0d5f
	WB1	r1, 0xfc7f
	WB2	r2, 0x1c60
	ST	r1, 100
	ST	r2, 104
	ADD	r3, 100
	DJMP	r3
