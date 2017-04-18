/*
 *  TOPPERS Software
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2016 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  $Id: chip_support_asm.S 718 2016-09-23 00:00:00Z azo $
 */

/*
 *		カーネルの割込みコントローラ依存部のアセンブリ言語部（BCM283X用）
 */

#define TOPPERS_MACRO_ONLY
#define TOPPERS_ASM_MACRO
#define UINT_C(val)			(val)		/* uint_t型の定数を作るマクロ */
#define ULONG_C(val)		(val)		/* ulong_t型の定数を作るマクロ */
#define CAST(type, val)		(val)		/* 型キャストを行うマクロ */
#include "kernel_impl.h"
#include "core_asm.inc"

/*
 *  割込みハンドラ呼出し前の割込みコントローラ操作
 *
 *  r4に割込み番号を返す．irc_end_intで用いる情報（割込み発生前の割込み
 *  優先度マスク）を，スタックの先頭に保存する．
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(irc_begin_int)
ALABEL(irc_begin_int)
	/*
	 *  割り込み要因の判定
    	 */
	ldr   r1, =AIC_BASIC
	ldr   r5, [r1]
	tst   r5, #0x200
	movne r4, #64
	ldrne r1, =AIC_PEND2
	ldrne r5, [r1]
	bne   bsc283x_irc_begin_int_1
	tst   r5, #0x100
	movne r4, #32
	ldrne r1, =AIC_PEND1
	ldrne r5, [r1]
	bne   bsc283x_irc_begin_int_1
	bic   r5, r5, #0x300
	mov   r4, #0
bsc283x_irc_begin_int_1:
	mov   r3, #1
bsc283x_irc_begin_int_2:
	ands  r1, r5, r3
	movne r3, #0xF			/* 要因番号が確定 */
	bne   bsc283x_irc_begin_int_9
	add   r4, r4, #1
	lsls  r3, r3, #1
	bne   bsc283x_irc_begin_int_2
bsc283x_irc_begin_int_9:
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	cmp   r3, #0xF			/* 要因番号が未確定かチェック */
	beq   bsc283x_irc_begin_int_8
	mov   r4, #96
	ldr   r1, =BCM283X_C0_IRQSOURCE
	ldr   r5, [r1]
	ldr   r1, =bcm283x_enable_intflag	/* 許可されている要因だけチェック */
	ldr   r3, [r1]
	and   r5, r5, r3
	mov   r3, #1
bsc283x_irc_begin_int_3:
	ands  r1, r5, r3
	movne r3, #0xF			/* 要因番号が確定 */
	bne   bsc283x_irc_begin_int_8
	add   r4, r4, #1
	lsls  r3, r3, #1
	bne   bsc283x_irc_begin_int_3
bsc283x_irc_begin_int_8:
#endif
	cmp   r3, #0xF			/* 要因番号が未確定かチェック */
	bne   undef_handler

	/*
	 *  割込み要因の割込み優先度を求め(モデル上の)割込み優先度マスクをセット
	 *  する．またその際，ハンドラ実行前の(モデル上の)割込み優先度マスクを
	 *  保存する．
	 */
	ldr   r0, =intinib_table	/* 割込み優先度を取得 */
	mov   r2, #0
bsc283x_intpri_loop_0:
	mov   r3, #0xC
	mul   r3, r2, r3
	ldr   r1, [r0, r3]
	cmp   r1, r4
	beq   bsc283x_intpri_loop_9
	add   r2, r2, #1
	ldr   r1, =_kernel_tnum_cfg_intno
	ldr   r1, [r1]
	cmp   r1, r2
	beq   undef_handler
	b     bsc283x_intpri_loop_0
bsc283x_intpri_loop_9:
	add   r3, r3, #8
	ldr   r1, [r0, r3]		/* r1<-割込み優先度   */
	ldr   r0, =ipm			/* 割込み発生前の割込み優先度マスクをスタックに保存 */
	ldr   r2, [r0]              
	stmfd sp!,{r2}
	str   r1, [r0]			/* (モデル上の)割込み優先度マスクをセット */

	/*
	 * (モデル上の)割込み優先度マスクの設定
	 * 
	 * 受け付けた割込みの割込み優先度に設定された割込み要求禁止フラグの
	 * テーブルの値と(モデル上の)各割込みの割込み要求禁止フラグの状態を
	 * 保持した変数の値とのORをIRCの割込み要求禁止フラグにセットする．
	 */
	mvn   r1, r1			/* 要素番号となるように反転+1 */
	add   r1, r1, #1
	lsl   r1, r1, #4		/* x16 */
	ldr   r0, =ipm_mask_tbl		/* (モデル上)の割込み優先度を実現するための */
	add   r0, r0, r1
	ldr   r1, =idf			/* 各割込みの割込み要求禁止フラグの状態を取得 */
	ldr   r2, [r0]  		/* 割込み要求禁止フラグ(BASIC)を取得 */
	ldr   r3, [r1]
	orr   r2, r2, r3		/* マスク指定されていない割込みの許可(BASIC) */
	ldr   r3, =AIC_DISABLE_BASIC
	str   r2, [r3]
	ldr   r2, [r0, #4]		/* 割込み要求禁止フラグ(IRQ1)を取得 */
	ldr   r3, [r1, #4]
	orr   r2, r2, r3		/* マスク指定されていない割込みの許可(IRQ1) */
	ldr   r3, =AIC_DISABLE_IRQ1
	str   r2, [r3]
	ldr   r2, [r0, #8]		/* 割込み要求禁止フラグ(IRQ2)を取得 */
	ldr   r3, [r1, #8]
	orr   r2, r2, r3		/* マスク指定されていない割込みの許可(IRQ2) */
	ldr   r3, =AIC_DISABLE_IRQ2
	str   r2, [r3]
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	ldr   r2, [r0, #12]		/* 割込み要求禁止フラグ(Core0)を取得 */
	ldr   r3, [r1, #12]
	orr   r2, r2, r3		/* マスク指定されていない割込みの許可(Core0) */
	ldr   r3, =bcm283x_enable_intflag
	ldr   r0, [r3]
	bic   r2, r2, r0
	str   r2, [r3]
#endif
#if defined(TOPPERS_USE_BCM2837)
	ldr r3, =AIC_PEND1		/* CPUが速すぎるので、AICが落ち着くのを待つためのレジスタ読み */
	ldr r3, [r3]
#endif
	bx    lr

/*
 *  割込みハンドラ呼出し後の割込みコントローラ操作
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(irc_end_int)
ALABEL(irc_end_int)
	/*
	 * 割込み優先度マスクを元に戻す
	 */
	/*
	 * 各割込みの割込み要求禁止フラグの状態と(モデル上)の割込み優先度のを実現する
	 * ための割込み要求禁止フラグの状態のORをとり，それの否定を求めることにより，
	 * IRCの割込み許可レジスタへの設定値を生成し設定する．
	 */
	ldmfd sp!, {r1}             /* 元の割込み優先度マスクを取得 */
	ldr   r0, =ipm              /* 割込み優先度マスクを復帰     */
	str   r1, [r0]
	mvn   r1, r1			/* 要素番号となるように反転+1 */
	add   r1, r1, #1
	lsl   r1, r1, #4
	ldr   r0, =ipm_mask_tbl     /* (モデル上)の割込み優先度のを実現するための */
	add   r0, r0, r1
	ldr   r1, =idf              /* 各割込みの割込み要求禁止フラグの状態を取得 */
	ldr   r2, [r0]          /* 割込み要求禁止フラグ(BASIC)を取得 */
	ldr   r3, [r1]
	orr   r2, r2, r3            /* マスク指定されていない割込みの許可 */
	mvn   r2, r2
	ldr   r3, =AIC_ENABLE_BASIC
	str   r2, [r3]
	ldr   r2, [r0, #4]          /* 割込み要求禁止フラグ(IRQ1)を取得 */
	ldr   r3, [r1, #4]
	orr   r2, r2, r3            /* マスク指定されていない割込みの許可 */
	mvn   r2, r2
	ldr   r3, =AIC_ENABLE_IRQ1
	str   r2, [r3]
	ldr   r2, [r0, #8]          /* 割込み要求禁止フラグ(IRQ2)を取得 */
	ldr   r3, [r1, #8]
	orr   r2, r2, r3            /* マスク指定されていない割込みの許可 */
	mvn   r2, r2
	ldr   r3, =AIC_ENABLE_IRQ2
	str   r2, [r3]
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	ldr   r2, [r0, #12]         /* 割込み要求禁止フラグ(Core0)を取得 */
	ldr   r3, [r1, #12]
	orr   r2, r2, r3            /* マスク指定されていない割込みの許可 */
	ldr   r3, =bcm283x_enable_intflag
	str   r2, [r3]
#endif
	bx    lr


	AGLOBAL(azo_ledon)
ALABEL(azo_ledon)
	ldr r1, =0x3F200008
	ldr r0, [r1]
	bic r0, #0x38
	orr r0, #0x08
	str r0, [r1]
	ldr r1, =0x3F20001C
	mov r0, #0x200000
	str r0, [r1]
	bx lr

	AGLOBAL(azo_ledoff)
ALABEL(azo_ledoff)
	ldr r1, =0x3F200028
	mov r0, #0x200000
	str r0, [r1]
	bx lr

