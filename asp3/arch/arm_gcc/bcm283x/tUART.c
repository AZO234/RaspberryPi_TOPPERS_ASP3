/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
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
 *  $Id: tUART.c 720 2016-09-23 00:00:00Z azo $
 */

/*
 *		FIFO内蔵シリアルコミュニケーションインタフェース用 簡易SIOドライバ
 */

#include <sil.h>
#include "tUART_tecsgen.h"
#include "bcm283x.h"

/*
 *  UART関連
 */

/*
 *  プリミティブな送信／受信関数
 */

/*
 *  受信バッファに文字があるか？
 */
Inline bool_t
uart_getready(CELLCB *p_cellcb)
{
#if BCM283X_USE_UART == 1
#ifdef BCM283X_UART_ENABLE_FIFO
	return((sil_rew_mem(BCM283X_AUX_MU_LSR_REG) & 0x01) != 0);
#else	/* BCM283X_UART_ENABLE_FIFO */
	return(1);
#endif	/* BCM283X_UART_ENABLE_FIFO */
#else	/* BCM283X_USE_UART */
#ifdef BCM283X_UART_ENABLE_FIFO
	return((sil_rew_mem(BCM283X_UART0_FR) & (1 << 4)) == 0);	/* Receive FIFO is not empty is true */
#else	/* BCM283X_UART_ENABLE_FIFO */
	return(1);
#endif	/* BCM283X_UART_ENABLE_FIFO */
#endif	/* BCM283X_USE_UART */
}

/*
 *  送信バッファに空きがあるか？
 */
Inline bool_t
uart_putready(CELLCB *p_cellcb)
{
#if BCM283X_USE_UART == 1
	if (VAR_initialized) {
		return((sil_rew_mem(BCM283X_AUX_MU_LSR_REG) & 0x20) != 0);
	} else {
		return(1);
	}
#else	/* BCM283X_USE_UART */
	return((sil_rew_mem(BCM283X_UART0_FR) & (1 << 5)) == 0);	/* Transmit FIFO is not full is true */
#endif	/* BCM283X_USE_UART */
}

/*
 *  受信した文字の取出し
 */
Inline char
uart_getchar(CELLCB *p_cellcb)
{
#if BCM283X_USE_UART == 1
	return((char)(sil_rew_mem(BCM283X_AUX_MU_IO_REG) & 0xFF));
#else	/* BCM283X_USE_UART */
	return((char)(sil_rew_mem(BCM283X_UART0_DR)) & 0xFF);	/* Read DR 0:7 */
#endif	/* BCM283X_USE_UART */
}

/*
 *  送信する文字の書込み
 */
void
uart_putchar(CELLCB *p_cellcb, char c)
{
#if BCM283X_USE_UART == 1
	sil_wrw_mem(BCM283X_AUX_MU_IO_REG, c);
#else	/* BCM283X_USE_UART */
	sil_wrw_mem(BCM283X_UART0_DR, c);		/* Write DR 0:7 */
#endif	/* BCM283X_USE_UART */
}

/*
 *  シリアルI/Oポートのオープン
 */
void
eSIOPort_open(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
#if BCM283X_USE_UART == 1
	uint_t	baud;
#else	/* BCM283X_USE_UART */
	uint_t	ibrd, fbrd;
#endif	/* BCM283X_USE_UART */

	if (VAR_initialized) {
		/*
		 *  既に初期化している場合は、二重に初期化しない．
		 */
		return;
	}

#if BCM283X_USE_UART == 1
	baud = (250000000 / (8 * ATTR_baudRate)) - 1;

	sil_wrw_mem(BCM283X_AUX_ENABLES, 1);		/* Enable UART1 */
	sil_wrw_mem(BCM283X_AUX_MU_IER_REG, 0);		/* Disable interrupt */
	sil_wrw_mem(BCM283X_AUX_MU_CNTL_REG, 0);	/* Disable Transmitter and Receiver */
	sil_wrw_mem(BCM283X_AUX_MU_LCR_REG, 3);		/* Works in 8-bit mode */
	sil_wrw_mem(BCM283X_AUX_MU_MCR_REG, 0);		/* Disable RTS */
#ifdef BCM283X_UART_ENABLE_FIFO
	sil_wrw_mem(BCM283X_AUX_MU_IIR_REG, 0xC6);	/* Enable FIFO, Clear FIFO */
#else	/* BCM283X_UART_ENABLE_FIFO */
	sil_wrw_mem(BCM283X_AUX_MU_IIR_REG, 0)	;	/* Disable FIFO */
#endif	/* BCM283X_UART_ENABLE_FIFO */
	sil_wrw_mem(BCM283X_AUX_MU_BAUD_REG, baud);	/* 115200 = system clock 250MHz / (8 * (baud + 1)), baud = 270 */
	sil_wrw_mem(BCM283X_AUX_MU_CNTL_REG, 3);	/* Enable Transmitter and Receiver */
#else	/* BCM283X_USE_UART */
	ibrd = 3000000 / (ATTR_baudRate * 16);
	fbrd = (3000000 - (ibrd * ATTR_baudRate * 16)) * 4 / ATTR_baudRate;

	sil_wrw_mem(BCM283X_UART0_CR, 0x0);			/* Disable UART0 */
	sil_wrw_mem(BCM283X_UART0_ICR, 0x7FF);			/* Interrupt clear */
	sil_wrw_mem(BCM283X_UART0_IBRD, ibrd);			/* 3000000(300MHz) / (115200 * 16) = 625/384 = 1 + 241/384 */
	sil_wrw_mem(BCM283X_UART0_FBRD, fbrd);			/* 241/384 * 64 = about 40 */
#ifdef BCM283X_UART_ENABLE_FIFO
	sil_wrw_mem(BCM283X_UART0_LCRH, 0x70);			/* FIFO Enable, 8 n 1 */
	sil_wrw_mem(BCM283X_UART0_IFLS, 0x0);			/* Receive interrupt FIFO 1/8, Transmit interrupt FIFO 1/8 */
	sil_wrw_mem(BCM283X_UART0_CR, 0x301);			/* Transmit Receive Enable UART0 */
	while((sil_rew_mem(BCM283X_UART0_FR) & 0x10) == 0) {	/* FIFO clear */
		sil_rew_mem(BCM283X_UART0_DR);
	}
#else	/* BCM283X_UART_ENABLE_FIFO */
	sil_wrw_mem(BCM283X_UART0_LCRH, 0x60);			/* FIFO Disable, 8 n 1 */
	sil_wrw_mem(BCM283X_UART0_CR, 0x301);			/* Transmit Receive Enable UART0 */
#endif	/* BCM283X_UART_ENABLE_FIFO */
#endif	/* BCM283X_USE_UART */

	VAR_initialized = true;
}

/*
 *  シリアルI/Oポートのクローズ
 */
void
eSIOPort_close(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);

#if BCM283X_USE_UART == 1
	sil_wrw_mem(BCM283X_AUX_ENABLES, 0);		/* Disable UART1 */
	sil_wrw_mem(BCM283X_AUX_MU_IER_REG, 0);		/* Disable interrupt */
	sil_wrw_mem(BCM283X_AUX_MU_CNTL_REG, 0);	/* Disable Transmitter and Receiver */
#else	/* BCM283X_USE_UART */
	sil_wrw_mem(BCM283X_UART0_CR, 0x0);		/* Disable UART0 */
#endif	/* BCM283X_USE_UART */
}

/*
 *  シリアルI/Oポートへの文字送信
 */
bool_t
eSIOPort_putChar(CELLIDX idx, char c)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);

	if (uart_putready(p_cellcb)){
		uart_putchar(p_cellcb, c);
		return(true);
	}
	return(false);
}

/*
 *  シリアルI/Oポートからの文字受信
 */
int_t
eSIOPort_getChar(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);
	char	c;

	if (uart_getready(p_cellcb)) {
		return((int_t)(uint8_t) uart_getchar(p_cellcb));
	}
	return(-1);
}

/*
 *  シリアルI/Oポートからのコールバックの許可
 */
void
eSIOPort_enableCBR(CELLIDX idx, uint_t cbrtn)
{
	CELLCB		*p_cellcb = GET_CELLCB(idx);
	uint32_t	imsc;

#if BCM283X_USE_UART == 1
	imsc = sil_rew_mem(BCM283X_AUX_MU_IER_REG);
#else	/* BCM283X_USE_UART */
	imsc = sil_rew_mem(BCM283X_UART0_IMSC);
#endif	/* BCM283X_USE_UART */
	switch (cbrtn) {
	case SIOSendReady:
#if BCM283X_USE_UART == 1
//		imsc |= 0x2;
#else	/* BCM283X_USE_UART */
//		imsc |= 0x20;
#endif	/* BCM283X_USE_UART */
		break;
	case SIOReceiveReady:
#if BCM283X_USE_UART == 1
		imsc |= 0x5;
#else	/* BCM283X_USE_UART */
		imsc |= 0x10;
#endif	/* BCM283X_USE_UART */
		break;
	}
#if BCM283X_USE_UART == 1
	sil_wrw_mem(BCM283X_AUX_MU_IER_REG, imsc);
#else	/* BCM283X_USE_UART */
	sil_wrw_mem(BCM283X_UART0_IMSC, imsc);
#endif	/* BCM283X_USE_UART */
}

/*
 *  シリアルI/Oポートからのコールバックの禁止
 */
void
eSIOPort_disableCBR(CELLIDX idx, uint_t cbrtn)
{
	CELLCB		*p_cellcb = GET_CELLCB(idx);
	uint32_t	imsc;

#if BCM283X_USE_UART == 1
	imsc = sil_rew_mem(BCM283X_AUX_MU_IER_REG);
#else	/* BCM283X_USE_UART */
	imsc = sil_rew_mem(BCM283X_UART0_IMSC);
#endif	/* BCM283X_USE_UART */
	switch (cbrtn) {
	case SIOSendReady:
#if BCM283X_USE_UART == 1
		imsc &= ~(0x2);
#else	/* BCM283X_USE_UART */
		imsc &= ~(0x20);
#endif	/* BCM283X_USE_UART */
		break;
	case SIOReceiveReady:
#if BCM283X_USE_UART == 1
		imsc &= ~(0x5);
#else	/* BCM283X_USE_UART */
		imsc &= ~(0x10);
#endif	/* BCM283X_USE_UART */
		break;
	}
#if BCM283X_USE_UART == 1
	sil_wrw_mem(BCM283X_AUX_MU_IER_REG, imsc);
#else	/* BCM283X_USE_UART */
	sil_wrw_mem(BCM283X_UART0_IMSC, imsc);
#endif	/* BCM283X_USE_UART */
}

/*
 *  シリアルI/Oポートに対する受信割込み処理
 */
void
eiISR_main(CELLIDX idx)
{
	CELLCB	*p_cellcb = GET_CELLCB(idx);

	if (uart_getready(p_cellcb)) {
		/*
		 *  受信通知コールバックルーチンを呼び出す．
		 */
		ciSIOCBR_readyReceive();
	}
	if (uart_putready(p_cellcb)) {
		/*
		 *  送信可能コールバックルーチンを呼び出す．
		 */
//		ciSIOCBR_readySend();
	}
}


