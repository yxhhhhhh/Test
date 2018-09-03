/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UART.h
	\brief		UART Header file
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UART_H_
#define _UART_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------

//Uart config bit definition
#define	RS232_TX_INT_EN_BIT		16
#define	RS232_RX_INT_EN_BIT		17
#define	RS232_TX_INT_CLR_BIT	18
#define	RS232_RX_INT_CLR_BIT	19
#define	FIFO_THD_MASK			0x1F
#define	FIFO_CNT_MASK			0x3F

#define UART_DEV						((UART1_Type*)ulUART_DevAddr[tUART_Dev])

typedef enum
{
	UART_1,
	UART_2,
	UART_NULL = 0xFF
} UART_DEVICE_t;

typedef enum
{
	UART_NOTREADY,
	UART_READY,
} UART_DEVSTS_t;

typedef enum
{
	UART_CLOCK_10M = 10000000,
	UART_CLOCK_96M = 96000000,
} UART_CLOCK_RATE_t;

typedef enum
{
	UART_TX = 1,
	UART_RX,
	UART_TXRX
} UART_DIRECTION_t;

//! Baud rate definition
typedef enum
{
	BR_921600 = 921600,
	BR_115200 = 115200,
	BR_57600 = 57600,
	BR_38400 = 38400,
	BR_28800 = 28800,
	BR_19200 = 19200,
	BR_14400 = 14400,
	BR_12800 = 12800,
	BR_11520 = 11520,
	BR_9600 = 9600,
	BR_7200 = 7200,
	BR_4800 = 4800,
} UART_BAUDRATE_t;

//! Stop bit defintion
typedef enum
{
	UART_1STOPBIT = 0,
	UART_2STOPBIT
} UART_STOPBIT_t;
 
//------------------------------------------------------------------------------
/*!
\brief UART initialization function, setting baudrate and stopbit.
\param tUART_Dev		Select UART base.
\param tClockRate		Clock source of RS232.
\par Note:
		0 : 10MHz
		1 : 96MHz
\param tBaudRate		Setting the RS232 baudrate.
\param ubStopbit		Setting the RS232 stop bit number, can be either\n
						RS232_1STOPBIT or RS232_2STOPBIT
\param pxRxSemaphore	Interrupt notify for RX mode
\return(no)
\par Example:
Initialize with 115200bps baudrate and 1 stop bit
\code
UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
\endcode
\sa			UART_Uninit();
*/
void UART_Init(UART_DEVICE_t tUART_Dev, UART_CLOCK_RATE_t tClockRate, UART_BAUDRATE_t tBaudRate, uint8_t ubStopbit, void* pxRxSemaphore);
//------------------------------------------------------------------------------
/*!
\brief UART un-initialization function
\param tUART_Dev		Select UART base.
\return(no)
\par Example:
Un-initial UART
\code
UART_Uninit(UART_1);
\endcode
\sa			UART_Init(...);
*/
void UART_Uninit(UART_DEVICE_t tUART_Dev);
//------------------------------------------------------------------------------
/*!
\brief Interrupt handler of UART1
\return(no)
*/
void UART1_INT_Handler(void);
//------------------------------------------------------------------------------
/*!
\brief Interrupt handler of UART2
\return(no)
*/
void UART2_INT_Handler(void);
//------------------------------------------------------------------------------
/*!
\brief Setup UART FIFO threshold
\param tUART_Dev	Select UART base.
\param ubTxTHR 		TX FIFO threshold
\par Note:
		The data can be write when TX data number <= ubTxTHR
\param ubRxTHR 		RX FIFO threshold
\par Note:
		The data can be read when RX data number >= ubRxTHR
\return(no)
*/
void UART_FIFO_Setup(UART_DEVICE_t tUART_Dev, uint8_t ubTxTHR, uint8_t ubRxTHR);
//------------------------------------------------------------------------------
/*!
\brief How many data stored in the RX FIFO.
\param tUART_Dev	Select UART base.
\return data number
*/
uint8_t ubUART_Get_RxFIFOCnt(UART_DEVICE_t tUART_Dev);
//------------------------------------------------------------------------------
/*!
\brief Enable interrupt of UART.
\param tUART_Dev	Select UART base.
\param tDir			UART1 direction.(TX or RX)
\return(no)
*/
void UART_INT_Enable(UART_DEVICE_t tUART_Dev, UART_DIRECTION_t tDir);
//------------------------------------------------------------------------------
/*!
\brief Disable interrupt of UART.
\param tUART_Dev	Select UART base.
\param tDir			UART1 direction.(TX or RX)
\return(no)
*/
void UART_INT_Disable(UART_DEVICE_t tUART_Dev, UART_DIRECTION_t tDir);
//------------------------------------------------------------------------------
/*!
\brief Clear flag of UART.
\param tUART_Dev	Select UART base.
\param tDir			UART direction.(TX or RX)
\return(no)
*/
void UART_INT_Clear(UART_DEVICE_t tUART_Dev, UART_DIRECTION_t tDir);
//------------------------------------------------------------------------------
/*!
\brief 	Get UART Version	
\return	Version
*/
uint16_t uwUART_GetVersion(void);
#endif
