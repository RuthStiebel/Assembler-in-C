;regisert overflow 12 out of 11 
.entry ADD
ADD: add 4,@r1
DEC: .string "abcdefg"
A: .data 1

mcro PRINT
prn @r1
endmcro

;one regiseter 
not @r12 
clr @r5
dec @r1
dec A

;two regiseters 
cmp @r12 ,@r6 
add 3,@r3 
sub 2,@r4
lea ADD ,@r1
PRINT

mcro SET
red @r1
endmcro

TEST: add 1, @r1

test: add 2,@r2

