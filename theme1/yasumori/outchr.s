.equ SYSCALL_NUM_PUTSTRING, 2

.global outbyte

.text
.even

outbyte:
	link.w	%a6, #0                 | スタックフレーム設定 (%a6基準で引数を参照しやすい)
	movem.l	%d1-%d3/%a0, -(%sp)     | 使用するレジスタを退避 (%d0は退避不要)

outbyte_loop:
	move.l	#SYSCALL_NUM_PUTSTRING, %d0
	move.l	#0, %d1                 | ch = 0 (シリアルポート0)
	lea.l	11(%a6), %a0            | 引数 (32bit符号拡張済) の下位1バイトアドレス
	move.l	%a0, %d2                | p = 出力文字のアドレス
	move.l	#1, %d3                 | size = 1 
	trap	#0                      | PUTSTRING呼び出し
	cmpi.l	#1, %d0                 | 1文字キューに入れたか確認
	bne	outbyte_loop            | 失敗 (0) ならリトライ

	movem.l	(%sp)+, %d1-%d3/%a0     | レジスタを復元
	unlk	%a6                     | スタックフレーム解放
	rts
