MAIN: addi $4,15,$15

mcro RUN_PROCESS

  lw $8,0,$10
  ;comment
  bne $8,$0,AFTER

mcroend

STEP1: sub $5,$12,$4

RUN_PROCESS

AFTER: or $10,$5,$3

  beq $10,$0,FINISH

  la TOTAL

  sw $10,0,$11

  jmp STEP1

FINISH: hlt

MSG: .asciz "XyZ1"

ARR: .dh 1024,-4050

  .db 12,-1

TOTAL: .dw 100,-500
