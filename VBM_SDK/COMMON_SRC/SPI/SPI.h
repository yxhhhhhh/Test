/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SPI.h
	\brief		SPI header file
	\author		Nick Huang
	\version	0.3
	\date		2017/10/26
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SPI_H_
#define _SPI_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------
typedef enum
{
    SPI_SLAVE = 1,
    SPI_MASTER = 3
} SPI_MODE_t;

typedef void (*SPI_SlaveHook) (uint8_t ubDataLen);
typedef void (*SPI_DmaEndHook) (void);

typedef struct
{
    uint8_t         ubSPI_CPOL:1;                       //!< SPI Clock Polarity
    uint8_t         ubSPI_CPHA:1;                       //!< SPI Clock Phase
    SPI_MODE_t      tSPI_Mode;                          //!< Master/Slave Mode
    uint16_t        uwClkDiv;                           //!< SPI Clock Divider, SPI_CLK = PCLK / (2* (uwSPI_ClkDiv+1))
    SPI_SlaveHook   pfSlaveHook;
    SPI_DmaEndHook  pfDmaEndHook;
} SPI_Setup_t;

typedef enum
{
    SPI_WaitReady,
    SPI_DontWait
} SPI_WaitMode_t;
//------------------------------------------------------------------------------
uint16_t uwSPI_GetVersion(void);
void SPI_Init(SPI_Setup_t* setup);
//void SPI_CpuRW(uint8_t* pbTxData, uint8_t* pbRxData, uint8_t ubDataLen, uint8_t ubDataWidth);
void SPI_DmaRW(uint8_t* pbTxData, uint8_t* pbRxData, uint32_t ulDataLen, uint8_t ubDataWidth, SPI_WaitMode_t tWaitMode);
//------------------------------------------------------------------------------
#endif
