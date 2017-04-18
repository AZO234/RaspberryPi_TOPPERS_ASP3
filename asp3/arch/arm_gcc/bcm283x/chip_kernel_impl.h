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
 *  $Id: chip_kernel_impl.h 720 2016-09-23 00:00:00Z azo $
 */

/*
 *		kernel_impl.hのチップ依存部（BCM283X用）
 *
 *  このヘッダファイルは，target_kernel_impl.h（または，そこからインク
 *  ルードされるファイル）のみからインクルードされる．他のファイルから
 *  直接インクルードしてはならない．
 */

#ifndef TOPPERS_CHIP_KERNEL_IMPL_H
#define TOPPERS_CHIP_KERNEL_IMPL_H

#define USE_INTINIB_TABLE

/*
 *  BCM283Xのハードウェア資源の定義
 */
#include <sil.h>
#include "bcm283x.h"
#include "arm.h"

/*
 *  ベクタールーチンをカーネルで持つかの定義
 */
#define VECTOR_KERNEL

/*
 *  ベクタールーチンを持たない場合のベクターアドレスの先頭番地
 */ 
#define VECTOR_START  0x00000000

/*
 * 割込み待ち命令
 */
#define ASM_TARGET_WAIT_INTERRUPT wfi

/*
 *  MMUの使用に関する設定
 */
#define USE_ARM_MMU

/*
 *  割込み番号の数，最小値と最大値
 */
#define TNUM_INTNO		BCM283X_TNUM_INHNO
#define TMIN_INTNO		UINT_C(0)
#define TMAX_INTNO		(TNUM_INTNO - 1)

/*
 *  割込みハンドラ番号の数
 */
#define TNUM_INHNO		TNUM_INTNO
#define TMIN_INHNO		UINT_C(0)
#define TMAX_INHNO		(TNUM_INHNO - 1)

/*
 *  割込み要求ラインのための標準的な初期化情報を生成する
 */
#define USE_INTINIB_TABLE

/*
 *  割込み要求ライン設定テーブルを生成する
 */
#define USE_INTCFG_TABLE

/*
 *  コアで共通な定義
 */
#include "core_kernel_impl.h"

/*
 *  IPMをimp_mask_tblのインデックスに変換するマクロ
 */
#define INDEX_IPM(ipm)  (TNUM_INTPRI + (ipm))

#ifndef TOPPERS_MACRO_ONLY

/*
 *  割込み番号の範囲の判定
 *
 *  ビットパターンを求めるのを容易にするために，8は欠番になっている．
 */
#define VALID_INTNO(intno) (TMIN_INTNO <= (intno) && (intno) <= TMAX_INTNO)
#define VALID_INTNO_DISINT(intno)	VALID_INTNO(intno)
#define VALID_INTNO_CFGINT(intno)	VALID_INTNO(intno)
#define VALID_INTNO_ATTISR(intno)   VALID_INTNO(intno)

/*
 * 各割込みの割込み要求禁止フラグの状態
 */
extern uint32_t idf[4];

/*
 *  割込み優先度マスク操作ライブラリ
 *
 *  AT91SAM7Sは割込み優先度マスクをIRC内でハードウェア的に持つが，
 *  ソフトウェア側から値を読み書きできないため，割込み要求禁止フラ
 *  グにより割込み優先度マスクを実現する
 */

/*
 *  現在の割込み優先度マスクの値
 */
extern PRI ipm;

/*
 *  割込み優先度マスク毎にセットする，割込み要求禁止フラグの値
 *  のテーブル
 */
extern const uint32_t ipm_mask_tbl[TNUM_INTPRI][4];

/*
 *  割込み属性が設定されているかを判別するための変数
 */
extern uint32_t	bitpat_cfgintb;
extern uint32_t	bitpat_cfgint1;
extern uint32_t	bitpat_cfgint2;
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
extern uint32_t	bitpat_cfgintc0;
#endif

#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
/*
 *  現在の割込みの有効フラグの状態
 *  割込みの有効状態を示すレジスタが無いのでこのフラグで表現する．
 */
extern volatile uint32_t bcm283x_enable_intflag;
#endif

/*
 *  チップ依存の初期化
 */
extern void chip_initialize(void);

/*
 *  チップ依存の終了処理
 */
extern void chip_terminate(void);

/*
 * 割込み要求のマスク
 */
Inline void
bcm283x_disable_int(uint32_t no, uint32_t mask)
{
	switch(no) {
	case 0: 
		sil_wrw_mem((void *)(AIC_DISABLE_BASIC), mask);
		break;
	case 1: 
		sil_wrw_mem((void *)(AIC_DISABLE_IRQ1), mask);
		break;
	case 2: 
		sil_wrw_mem((void *)(AIC_DISABLE_IRQ2), mask);
		break;
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	case 3: 
		bcm283x_enable_intflag &= ~(mask);
		break;
#endif
	}
}

/*
 * 割込み要求のマスクの解除
 */
Inline void
bcm283x_enable_int(uint32_t no, uint32_t mask)
{
	switch(no) {
	case 0: 
		sil_wrw_mem((void *)(AIC_ENABLE_BASIC), mask);
		break;
	case 1: 
		sil_wrw_mem((void *)(AIC_ENABLE_IRQ1), mask);
		break;
	case 2: 
		sil_wrw_mem((void *)(AIC_ENABLE_IRQ2), mask);
		break;
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	case 3: 
		bcm283x_enable_intflag |= mask;
		break;
#endif
	}
}

/*
 * 割込み要求のクリア
 */
Inline void
bcm283x_clear_int(uint32_t no, uint32_t mask)
{
}

/*
 * 割込み要求のチェック
 */
Inline bool_t
bcm283x_probe_int(uint32_t no, uint32_t mask)
{
	switch(no) {
	case 0: 
		return((sil_rew_mem((void *)(AIC_BASIC)) & mask) == mask);
	case 1: 
		return((sil_rew_mem((void *)(AIC_PEND1)) & mask) == mask);
	case 2: 
		return((sil_rew_mem((void *)(AIC_PEND2)) & mask) == mask);
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	case 3: 
		return(0);
#endif
	}
}

/*
 *  割込み優先度マスクの設定
 */
Inline void
x_set_ipm(PRI intpri)
{
    uint32_t ipm_maskb = ipm_mask_tbl[INDEX_IPM(intpri)][0];
    uint32_t ipm_mask1 = ipm_mask_tbl[INDEX_IPM(intpri)][1];
    uint32_t ipm_mask2 = ipm_mask_tbl[INDEX_IPM(intpri)][2];
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
    uint32_t ipm_maskc0 = ipm_mask_tbl[INDEX_IPM(intpri)][3];
#endif

    /*
     *  AT91SAM7Sの割込みコントローラはイネーブルレジスタと
     *  クリアーレジスタがあるため，一旦全ての割込みを禁止してから，
     *  特定の割込みのみ許可する必要がある
     */
    /* 全割込み禁止 */
    bcm283x_disable_int(0,~0U);
    bcm283x_disable_int(1,~0U);
    bcm283x_disable_int(2,~0U);
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
    bcm283x_disable_int(3,~0U);
#endif

    /* マスク指定されていない割込みのみ許可 */
    bcm283x_enable_int(0,~(ipm_maskb|idf[0]));
    bcm283x_enable_int(1,~(ipm_mask1|idf[1]));
    bcm283x_enable_int(2,~(ipm_mask2|idf[2]));
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
    bcm283x_enable_int(3,~(ipm_maskc0|idf[3]));
#endif

    ipm = intpri;
}

#define t_set_ipm(intpri) x_set_ipm(intpri)

/*
 *  割込み優先度マスクの参照
 */
Inline PRI
x_get_ipm(void)
{
    return(ipm);
}

#define t_get_ipm() x_get_ipm()

/*
 * （モデル上の）割込み要求禁止フラグのセット
 *
 *  指定された，割込み番号の割込み要求禁止フラグのセットして，割込みを
 *  禁止する．また，（モデル上の）割込み要求禁止フラグを管理するidfの対
 *  応するビットををセットする．
 *  割込み要求をマスクする機能をサポートしていない場合には，falseを返す
 */
Inline bool_t
x_disable_int(INTNO intno)
{
	if(intno >= 0 && intno < 32) {
		if ((bitpat_cfgintb & INTNO_BITPAT((intno))) == 0U) {
		        return(false);
		}
		bcm283x_disable_int(0,INTNO_BITPAT((intno)));
		idf[0] |= INTNO_BITPAT((intno));
	} else if(intno >= 32 && intno < 64) {
		if ((bitpat_cfgint1 & INTNO_BITPAT((intno - 32))) == 0U) {
		        return(false);
		}
		bcm283x_disable_int(1,INTNO_BITPAT((intno - 32)));
		idf[1] |= INTNO_BITPAT((intno - 32));
	} else if(intno >= 64 && intno < 96) {
		if ((bitpat_cfgint1 & INTNO_BITPAT((intno - 64))) == 0U) {
		        return(false);
		}
		bcm283x_disable_int(2,INTNO_BITPAT((intno - 64)));
		idf[2] |= INTNO_BITPAT((intno - 64));
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	} else if(intno >= 96 && intno < 128) {
		if ((bitpat_cfgint1 & INTNO_BITPAT((intno - 96))) == 0U) {
		        return(false);
		}
		bcm283x_disable_int(3,INTNO_BITPAT((intno - 96)));
		idf[3] |= INTNO_BITPAT((intno - 96));
#endif
	}
	return(true);
}

/* 
 * (モデル上の)割り要求禁止フラグの解除
 *
 * 指定された，割込み番号の割込み要求禁止フラグのクリアして，割込みを
 * 許可する．また，（モデル上の）割込み要求禁止フラグを管理するidfの対
 * 応するビットををクリアする．
 * 割込み要求をマスクする機能をサポートしていない場合には，falseを返す
 */
Inline bool_t
x_enable_int(INTNO intno)
{
    uint32_t ipm_maskb = ipm_mask_tbl[INDEX_IPM(ipm)][0];
    uint32_t ipm_mask1 = ipm_mask_tbl[INDEX_IPM(ipm)][1];
    uint32_t ipm_mask2 = ipm_mask_tbl[INDEX_IPM(ipm)][2];
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
    uint32_t ipm_maskc0 = ipm_mask_tbl[INDEX_IPM(ipm)][3];
#endif

	if(intno >= 0 && intno < 32) {
		idf[0] &= ~INTNO_BITPAT((intno));
	} else if(intno >= 32 && intno < 64) {
		idf[1] &= ~INTNO_BITPAT((intno - 32));
	} else if(intno >= 64 && intno < 96) {
		idf[2] &= ~INTNO_BITPAT((intno - 64));
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	} else if(intno >= 96 && intno < 128) {
		idf[3] &= ~INTNO_BITPAT((intno - 96));
#endif
	}

    /* 全割込み禁止 */
    bcm283x_disable_int(0,~0U);    
    bcm283x_disable_int(1,~0U);    
    bcm283x_disable_int(2,~0U);    
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
    bcm283x_disable_int(3,~0U);    
#endif
    /* マスク指定されていない割込みのみ許可 */
    bcm283x_enable_int(0,~(ipm_maskb|idf[0]));
    bcm283x_enable_int(1,~(ipm_mask1|idf[1]));
    bcm283x_enable_int(2,~(ipm_mask2|idf[2]));
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
    bcm283x_enable_int(3,~(ipm_maskc0|idf[3]));
#endif

    return(true);
}

/*
 * 割込み要求のクリア
 */
Inline void
x_clear_int(INTNO intno)
{
	if(intno >= 0 && intno < 32) {
		bcm283x_clear_int(0, INTNO_BITPAT((intno)));
	} else if(intno >= 32 && intno < 64) {
		bcm283x_clear_int(1, INTNO_BITPAT((intno - 32)));
	} else if(intno >= 64 && intno < 96) {
		bcm283x_clear_int(2, INTNO_BITPAT((intno - 64)));
#if defined(TOPPERS_USE_BCM2836) || defined(TOPPERS_USE_BCM2837)
	} else if(intno >= 96 && intno < 128) {
		bcm283x_clear_int(3, INTNO_BITPAT((intno - 96)));
#endif
	}
}

/*
 *  割込み要求禁止フラグのセット
 *
 *  intnoで指定された割込み要求ラインに対する割込み要求禁止フラグのセッ
 *  トし，割込みを禁止する．割込み属性が設定されていない割込み要求ライ
 *  ンが指定された場合には，falseを返す．
 */
Inline bool_t
disable_int(INTNO intno)
{
	if (intcfg_table[intno] == 0U) {
		return(false);
	}
	x_disable_int(intno);
	return(true);
}

/* 
 *  割込み要求禁止フラグのクリア
 *
 *  intnoで指定された割込み要求ラインに対する割込み要求禁止フラグのクリ
 *  アし，割込みを許可する．割込み属性が設定されていない割込み要求ライ
 *  ンが指定された場合には，falseを返す．
 */
Inline bool_t
enable_int(INTNO intno)
{
	if (intcfg_table[intno] == 0U) {
		return(false);
	}
	x_enable_int(intno);
	return(true);
}

/*
 *  割込み要求のクリア
 */
Inline void
clear_int(INTNO intno)
{
	x_clear_int(intno);
}

/*
 *  割込み要求のチェック
 */
Inline bool_t
probe_int(INTNO intno)
{
	return(bcm283x_probe_pending(intno));
}

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_CHIP_KERNEL_IMPL_H */
