/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UART.c
	\brief		UART Function
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "UART.h"
#include "INTC.h"
#include "CLI.h"
#ifdef CFG_UART1_ENABLE
#include "UI_UART1.h"
#endif

#define UART_MAJORVER	0
#define UART_MINORVER	2
//------------------------------------------------------------------------------
uint32_t ulUART_DevAddr[] = {UART1_BASE, UART2_BASE};
INTC_IrqSrc_t iUART_IRQn[] = {INTC_UART1_IRQ, INTC_UART2_IRQ};
INTC_IrqHandler iUART_IrqHandler[] = {UART1_INT_Handler, UART2_INT_Handler};
uint8_t ubUART_Ready[] = {UART_NOTREADY, UART_NOTREADY};										//!< Ready after initialize
#ifdef RTOS
osSemaphoreId *pxUART_RxSemaphore[] = {NULL, NULL};
#endif
//------------------------------------------------------------------------------
void UART_Init(UART_DEVICE_t tUART_Dev, UART_CLOCK_RATE_t tClockRate, UART_BAUDRATE_t tBaudRate, uint8_t ubStopbit, void* pxRxSemaphore)
{
	UART_BAUDRATE_t tBaseTRX_Rate = BR_115200;
	float fClk_Src = tClockRate;
	float fBase_rate = 0;
	uint8_t ubBase_rate = 0;

	UART_INT_Disable(tUART_Dev, UART_TXRX);
	tBaseTRX_Rate 			= (tBaudRate >= BR_115200)?tBaudRate:BR_115200;
	GLB->UART_RATE 			= (tBaudRate >= (UART_CLOCK_10M / 16))?1:(tClockRate/UART_CLOCK_96M);
	fBase_rate 				= (fClk_Src/8/tBaseTRX_Rate)+0.5;
	ubBase_rate 			= floor(fBase_rate);
	UART_DEV->TRX_BASE 		= ubBase_rate;
	UART_DEV->RS_RATE 		= (tBaudRate >= BR_115200)?1:(BR_115200/tBaudRate);
	UART_DEV->TX_STOP_BIT 	= ubStopbit;
	UART_DEV->CPU_DATA_MODE = 0;
	UART_DEV->RX_FIFO_THD 	= 1;
	UART_DEV->TX_MODE 		= 1;
	UART_DEV->RX_MODE 		= 1;

#ifdef RTOS
	if(pxUART_RxSemaphore[tUART_Dev] == NULL)
	{
		pxUART_RxSemaphore[tUART_Dev] = pxRxSemaphore;
		INTC_IrqSetup(iUART_IRQn[tUART_Dev], INTC_LEVEL_TRIG, iUART_IrqHandler[tUART_Dev]);
		INTC_IrqEnable(iUART_IRQn[tUART_Dev]);
		UART_INT_Enable(tUART_Dev, UART_RX);
	}
#endif
	ubUART_Ready[tUART_Dev] = UART_READY;
}
//------------------------------------------------------------------------------
void UART_Uninit(UART_DEVICE_t tUART_Dev)
{
	ubUART_Ready[tUART_Dev] = UART_NOTREADY;
	UART_DEV->TX_MODE = 0;
	UART_DEV->RX_MODE = 0;
	//! Set TX to GPIO
	//! Set RX to GPIO
	INTC_IrqClear(iUART_IRQn[tUART_Dev]);
	INTC_IrqDisable(iUART_IRQn[tUART_Dev]);
	UART_INT_Disable(tUART_Dev, UART_TXRX);
#ifdef RTOS
	pxUART_RxSemaphore[tUART_Dev] = NULL;
#endif
}
//------------------------------------------------------------------------------
void UART1_INT_Handler(void)
{
#ifdef CFG_UART1_ENABLE
	uint8_t ch;
	while(UART1->RX_RDY)
	{
		ch = UART1->RS_DATA & 0x0FF;
		UART1_rtoscli_recv(ch);
	}
#endif
	UART1->RS232_RX_INT_CLR = 1;
	INTC_IrqClear(INTC_UART1_IRQ);
#ifdef RTOS
	/*! We have not woken a task at the start of the ISR. */
	if(pxUART_RxSemaphore[UART_1] != NULL)
		osSemaphoreRelease(*(osSemaphoreId*)pxUART_RxSemaphore[UART_1]);
#endif
}
//------------------------------------------------------------------------------
void UART2_INT_Handler(void)
{
	uint8_t ch;
	while(UART2->RX_RDY)
	{
		ch = UART2->RS_DATA & 0x0FF;
		CLI_rtoscli_recv(ch);
	}
	UART2->RS232_RX_INT_CLR = 1;
	INTC_IrqClear(INTC_UART2_IRQ);
#ifdef RTOS
		/*! We have not woken a task at the start of the ISR. */
	if(pxUART_RxSemaphore[UART_2] != NULL)
		osSemaphoreRelease(*(osSemaphoreId*)pxUART_RxSemaphore[UART_2]);
#endif
}
//------------------------------------------------------------------------------
void UART_FIFO_Setup(UART_DEVICE_t tUART_Dev, uint8_t ubTxTHR, uint8_t ubRxTHR)
{
	UART_DEV->TX_FIFO_THD = (ubTxTHR & FIFO_THD_MASK);
	UART_DEV->RX_FIFO_THD = (ubRxTHR & FIFO_THD_MASK);
}
//------------------------------------------------------------------------------
uint8_t ubUART_Get_RxFIFOCnt(UART_DEVICE_t tUART_Dev)
{
	return UART_DEV->RX_FIFO_CNT;
}
//------------------------------------------------------------------------------
void UART_INT_Enable(UART_DEVICE_t tUART_Dev, UART_DIRECTION_t tDir)
{
	uint32_t ulIntr_Value;

	ulIntr_Value = UART_DEV->UART_CTR;
	ulIntr_Value |= (tDir & 0x3) << RS232_TX_INT_EN_BIT;
	UART_DEV->UART_CTR = ulIntr_Value;
}
//------------------------------------------------------------------------------
void UART_INT_Disable(UART_DEVICE_t tUART_Dev, UART_DIRECTION_t tDir)
{
	uint32_t ulIntr_Value;

	ulIntr_Value = UART_DEV->UART_CTR;
	ulIntr_Value &= ~((tDir & 0x3) << RS232_TX_INT_EN_BIT);
	UART_DEV->UART_CTR = ulIntr_Value;
}
//------------------------------------------------------------------------------
void UART_INT_Clear(UART_DEVICE_t tUART_Dev, UART_DIRECTION_t tDir)
{
	uint32_t ulIntr_Value;

	ulIntr_Value = UART_DEV->UART_CTR;
	ulIntr_Value |= (tDir & 0x3) << RS232_TX_INT_CLR_BIT;
	UART_DEV->UART_CTR = ulIntr_Value;
}
//------------------------------------------------------------------------------
uint16_t uwUART_GetVersion(void)
{
    return ((UART_MAJORVER << 8) + UART_MINORVER);
}
