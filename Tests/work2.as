;ext, ent, macro
K:		.data 22
MAIN:	mov @r7, W
		add @r2,STR
S2:		.string	 "12345"

XYZ: 	jmp W
		mcro m1
		inc K
; This is comment!!!
		mov S1 ,	@r3	
		endmcro
		prn 	-50

		sub @r1	, @r4
		m1
		bne L3
END:	stop
STR:	.string "abcdef"
LENGTH:	.data 6,-512,511
S1:		.string 	"ab"
.entry K

.entry XYZ
.extern L3
.extern W
