/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APBC.h
	\brief		APB Controller header file
	\author		Nick Huang
	\version	0.2
	\date		2017/10/19
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APBC_H_
#define _APBC_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------
typedef enum
{
    APBC_DMA_32BIT,
    APBC_DMA_16BIT,
    APBC_DMA_8BIT
} APBC_DMA_WIDTH_t;

enum
{
    APBC_SSP1_TX = 1,
    APBC_SSP1_RX,
    APBC_UART1_TX,
    APBC_UART1_RX,
    APBC_UART2_TX,
    APBC_UART2_RX
};

typedef enum
{
    APBC_NONBURST,
    APBC_BURST
} APBC_BURST_t;

typedef enum
{
    APBC_FINISH,
    APBC_ERROR
} APBC_INT_t;

typedef void (*APBC_IsrHandler) (APBC_INT_t tEvent);

#define APBC_CHA_ENABLE         (APBC->CHA_EN = 1)
#define APBC_CHB_ENABLE         (APBC->CHB_EN = 1)
#define APBC_CHC_ENABLE         (APBC->CHC_EN = 1)
#define APBC_CHD_ENABLE         (APBC->CHD_EN = 1)
#define APBC_CHA_DISABLE        (APBC->CHA_EN = 0)
#define APBC_CHB_DISABLE        (APBC->CHB_EN = 0)
#define APBC_CHC_DISABLE        (APBC->CHC_EN = 0)
#define APBC_CHD_DISABLE        (APBC->CHD_EN = 0)
//------------------------------------------------------------------------------
void APBC_Init(void);
uint16_t uwAPBC_GetVersion(void);
void APBC_ChA_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR);
void APBC_ChB_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR);
void APBC_ChC_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR);
void APBC_ChD_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR);
//------------------------------------------------------------------------------
#endif
