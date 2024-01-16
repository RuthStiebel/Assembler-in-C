.entry LENGTH           
.extern W    
MAIN: mov @r3 ,LENGTH    
LOOP: jmp L1    
;i am a note
mcro ruth   
    prn -5 
    bne W
    ;i am a note
    sub @r1, @r4
endmcro
ruth
bne L3
                            
 
  
  	

L1: inc K
;i am a note
.entry LOOP
jmp W
;i am a note    
END: stop
STR: .string    "abcdef" 
;i am a note
;i am a note
;i am a note
LENGTH: .data 6,-9,15 
K: .data 22
.extern L3
