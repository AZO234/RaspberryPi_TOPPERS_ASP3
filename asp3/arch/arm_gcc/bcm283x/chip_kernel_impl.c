/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2016 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
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
 *  $Id: chip_kernel_impl.c 720 2016-09-23 00:00:00Z azo $
 */

/*
 *		カーネルのチップ依存部（BCM283X用）
 */

#include "kernel_impl.h"
#include "interrupt.h"
#include "bcm283x.h"

#ifdef USE_ARM_MMU
/*
 *  MMUへの設定属性（第1レベルディスクリプタ）
 */
#define MMU_ATTR_RAM	(ARM_MMU_DSCR1_SHARED|ARMV6_MMU_DSCR1_APX0 \
							|ARM_MMU_DSCR1_TEX001|ARM_MMU_DSCR1_AP11 \
							|ARM_MMU_DSCR1_CB10)
#define MMU_ATTR_IODEV	(ARM_MMU_DSCR1_SHARED|ARMV6_MMU_DSCR1_APX0 \
							|ARM_MMU_DSCR1_TEX000|ARM_MMU_DSCR1_AP11 \
							|ARM_MMU_DSCR1_CB00|ARMV6_MMU_DSCR1_NOEXEC)

/*
 *  MMUの設定情報（メモリエリアの情報）
 */
ARM_MMU_CONFIG arm_memory_area[] = {
	{ SDRAM_ADDR, SDRAM_ADDR, SDRAM_SIZE, MMU_ATTR_RAM },
	{ IO1_ADDR, IO1_ADDR, IO1_SIZE, MMU_ATTR_IODEV },
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	{ IO2_ADDR, IO2_ADDR, IO2_SIZE, MMU_ATTR_IODEV },
#endif
};

/*
 *  MMUの設定情報の数（メモリエリアの数）
 */
const uint_t arm_tnum_memory_area
					= sizeof(arm_memory_area) / sizeof(ARM_MMU_CONFIG);
#endif	/* USE_ARM_MMU */

/*
 * 各割込みの割込み要求禁止フラグの状態
 */
uint32_t idf[4];

/*
 * 現在の割込み優先度マスクの値
 */
PRI ipm;

/*
 *  割込み属性が設定されているかを判別するための変数
 */
uint32_t bitpat_cfgintb;
uint32_t bitpat_cfgint1;
uint32_t bitpat_cfgint2;
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
uint32_t bitpat_cfgintc0;
#endif

#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
/*
 *  現在の割込みの有効フラグの状態
 *  割込みの有効状態を示すレジスタが無いのでこのフラグで表現する．
 */
volatile uint32_t bcm283x_enable_intflag;
#endif

/*
 *  チップ依存の初期化
 */
void
chip_initialize(void)
{
#if defined(TOPPERS_USE_BCM2835)
	/*
	 *  ベクタの初期化
	 */
	volatile uint32_t *sp, *dp, index;
	sp = (uint32_t*)0x8000;
	dp = (uint32_t*)0x0000;
	for(index = 0; index < 16; index++) {
		dp[index] = sp[index];
	}
#endif

	/*
	 *  キャッシュをディスエーブル
	 */
	arm_disable_cache();

	/*
	 * 各割込みの割込み要求禁止フラグ全禁止
	 */
	idf[0] = ~0U;
	idf[1] = ~0U;
	idf[2] = ~0U;
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	idf[3] = ~0U;
#endif

	/*
	 * 割込み優先度マスクの値は最低（全割り込み許可）
	 */ 
	ipm = TMAX_INTPRI;

	/*
	 * 全ての割込みをマスク
	 */ 
	bcm283x_disable_int(0,~0U);
	bcm283x_disable_int(1,~0U);
	bcm283x_disable_int(2,~0U);
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	bcm283x_disable_int(3,~0U);
#endif

	/*
	 * 全ての割込み要因をクリア
	 */
	bcm283x_clear_int(0,~0U);
	bcm283x_clear_int(1,~0U);
	bcm283x_clear_int(2,~0U);
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	bcm283x_clear_int(3,~0U);
#endif

	/*
	 *  割込み属性が設定されているかを判別するための変数を初期化する．
	 */
	bitpat_cfgintb = 0U;
	bitpat_cfgint1 = 0U;
	bitpat_cfgint2 = 0U;
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	bitpat_cfgintc0 = 0U;
#endif

#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	/*
	 *  割込み有効フラグを初期化する．
	 */
	bcm283x_enable_intflag = 0U;
#endif

	/*
	 *  コア依存の初期化
	 */
	core_initialize();

	/*
	 *  キャッシュをイネーブル
	 */
	arm_enable_cache();

	/*
	 *  分岐予測の無効化とイネーブル
	 */
	arm_invalidate_bp();
	arm_enable_bp();
}

/*
 *  チップ依存部の終了処理
 */
void
chip_terminate(void)
{
	extern void    software_term_hook(void);
	void (*volatile fp)(void) = software_term_hook;

	/*
	 *  software_term_hookへのポインタを，一旦volatile指定のあるfpに代
	 *  入してから使うのは，0との比較が最適化で削除されないようにするた
	 *  めである．
	 */
	if (fp != 0) {
		(*fp)();
	}

	/*
	 *  すべての割込みをマスクする．
	 */
	bcm283x_disable_int(0,~0U);
	bcm283x_disable_int(1,~0U);
	bcm283x_disable_int(2,~0U);
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	bcm283x_disable_int(3,~0U);
#endif

	/*
	 *  コア依存の終了処理
	 */
	core_terminate();
}

/*
 *  割込み要求ラインの属性の設定
 *
 *  ASPカーネルでの利用を想定して，パラメータエラーはアサーションでチェッ
 *  クしている．
 */
Inline void
config_int(INTNO intno, ATR intatr, PRI intpri)
{
	INTINIB	*p_intinib;

	assert(VALID_INTNO(intno));
	assert(TMIN_INTPRI <= intpri && intpri <= TMAX_INTPRI);

	/*
	 *  割込みを禁止
	 *
	 *  割込みを受け付けたまま，レベルトリガ／エッジトリガの設定や，割
	 *  込み優先度の設定を行うのは危険なため，割込み属性にかかわらず，
	 *  一旦マスクする．
	 */
	disable_int(intno);

	/*
	 *  割込みをコンフィギュレーション
	 */
	p_intinib = &(intinib_table[intno]);
	p_intinib->intatr = intatr;
	p_intinib->intpri = intpri;

	/*
	 * 割込みを許可
	 */
	if ((intatr & TA_ENAINT) != 0U) {
		enable_int(intno);
	}
}

/*
 *  割込み管理機能の初期化
 */
void
initialize_interrupt(void)
{
	uint_t			i;
	const INTINIB	*p_intinib;

	for (i = 0; i < tnum_cfg_intno; i++) {
		p_intinib = &(intinib_table[i]);
		config_int(p_intinib->intno, p_intinib->intatr, p_intinib->intpri);
	}
}

/*
 *  割込み要求ラインの属性の設定
 *
 *  ASPカーネルでの利用を想定して，パラメータエラーはアサーションでチェッ
 *  クしている．FI4カーネルに利用する場合には，エラーを返すようにすべき
 *  であろう．
 *
 */
void
x_config_int(INTNO intno, ATR intatr, PRI intpri)
{
	assert(VALID_INTNO(intno));
	assert(TMIN_INTPRI <= intpri && intpri <= TMAX_INTPRI);

	if(intno >= 0 && intno < 32) {
		/*
		 *  割込み属性が設定されているかを判別するための変数の設定
		 */
		bitpat_cfgintb |= INTNO_BITPAT(intno);
	} else if(intno >= 32 && intno < 64) {
		bitpat_cfgint1 |= INTNO_BITPAT(intno - 32);
	} else if(intno >= 64 && intno < 96) {
		bitpat_cfgint2 |= INTNO_BITPAT(intno - 64);
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	} else if(intno >= 96 && intno <= 127) {
		bitpat_cfgintc0 |= INTNO_BITPAT(intno - 96);
#endif
	}
    
	/* 
	 * いったん割込みを禁止する
	 */    
	x_disable_int(intno);

	if ((intatr & TA_ENAINT) != 0U){
		(void)x_enable_int(intno);
	}
}

