.equ SYSCALL_NUM_GETSTRING, 1

.global inbyte

.text
.even

inbyte:
	link.w	%a6, #-4                | スタック上に作業領域を確保 
	movem.l	%d1-%d3/%a0, -(%sp)     | 使用するレジスタを退避

inbyte_loop:
	move.l	#SYSCALL_NUM_GETSTRING, %d0
	move.l	#0, %d1                 | ch = 0 (シリアルポート0)
	lea.l	-1(%a6), %a0            | 1文字格納用のスタック領域アドレス
	move.l	%a0, %d2                | p = 書き込み先アドレス
	move.l	#1, %d3                 | size = 1 (1文字読み込み)
	trap	#0                      | GETSTRING 呼び出し
	cmpi.l	#1, %d0                 | 1文字取得できたか確認
	bne	inbyte_loop             | 取得できなかった (0) ならリトライ

	clr.l	%d0                     | 上位バイトのゴミを消去
	move.b	-1(%a6), %d0            | 取得した1文字を戻り値 (%d0) にセット

	movem.l	(%sp)+, %d1-%d3/%a0     | レジスタを復元
	unlk	%a6                     | スタックフレームを解放
	rts
