; no entery no external
MAIN: mov @r3, @r4
 add -3, @r2
LOOP: jmp W
 prn +5
W: sub @r1, @r4
 inc @r1
 cmp @r1, @r2
 bne LOOP
END: rts
