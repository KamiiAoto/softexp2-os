.global inbyte

.text
.even

inbyte:
    movem.l %d1-%d3/%a0, -(%sp)
    /*作業領域の確保*/
    link.w  %a6, #-4

inbyte_loop:
    /*GETSTRING呼び出し準備*/
    lea     -4(%a6), %a0
    moveq  #1, %d0
    moveq  #0, %d1
    move.l  %a0, %d2
    moveq  #1, %d3
    trap #0
    cmp.l   #1, %d0
    bne     inbyte_loop

inbyte_end:
    move.b  (%a0),%d0
    unlk    %a6
    movem.l (%sp)+, %d1-%d3/%a0
    rts
