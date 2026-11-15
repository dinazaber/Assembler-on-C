;file ps.as  
;sample source code 
 
.entry NEXT 
.extern NEXT  garbage text
.extern wNumber  
.entry wNumber  
STR: .asciz "aBcd 
MAIN: add $3,$-52,$99 
LOOP: ori $9,-5
 la val1 
 jmp NEXT 
NXT: move $20,$4 , 5
LIST*: .db 6,-9 
 bgt $4,$2,END 5
 la K 
 sw $0,4,$10 
 bn $31,$9, LOOP 
 calle val1 
 jmp $4 
 la wNumber 
.extern val1  
 .dh 27056 
K: .dw 31,-12 
END: hlt         
.entry K
