.global outbyte

.text
.even

outbyte:
    movem.l %d1-%d3/%a0, -(%sp)

outbyte_loop:
    /*PUTSTRING呼び出し準備*/
    lea     23(%sp), %a0  | 7+4*4 = 直後のoffset + 4*(退避した本数)
    moveq  #2, %d0
    moveq  #0, %d1
    move.l  %a0, %d2
    moveq  #1, %d3
    trap #0
    cmp.l   #1, %d0
    bne     outbyte_loop

outbyte_end:
    movem.l (%sp)+, %d1-%d3/%a0
    rts