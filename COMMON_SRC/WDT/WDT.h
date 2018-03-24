/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WDT.h
	\brief		Watch Dog Timer Header File
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _WDT_H_
#define _WDT_H_

#include <stdint.h>

#define WDT_DEV								((WDRT_Type*)ulWDT_DeviceAddr[tDevice])

#define IP_CLK								480000000								
#define EXT_CLK								32768

typedef enum
{
	WDT_FAIL,
	WDT_PASS,
} WDT_RESULT;

typedef enum
{
	WDT_RST,								//!< Watch Dog RST Timer(reset)
	WDT_INT,								//!< Watch Dog INT Timer(interrupt)
} WDT_DEIVCE_t;

typedef enum
{
	WDT_CLK_APBCLK = 0,
	WDT_CLK_EXTCLK,
} WDT_CLK_SRC_t;

typedef enum
{
	WDIT_SEMAPHORE,							//!< Event triggered by RTOS semaphore
	WDIT_QUEUE,								//!< Event triggered by RTOS Queue
	WDIT_EM_NULL
} WDIT_EVENT_METHOD_t;

typedef struct
{
	WDT_CLK_SRC_t 	 	tCLK;
	WDIT_EVENT_METHOD_t	tEM;
	void*			 	pvEvent;
	void*				pvINTHandler;
} WDIT_SETUP_t;

//------------------------------------------------------------------------------
/*!
\brief Enable watch dog reset timer
\param Clock_Src 		Clock source of watch dog timer.
\par Note:
		0 : APB clock
		1 : External clock (32768Hz)
\param ubTimeOut			Watch dog timeout (unit: second)
\return(no)
*/
void WDT_RST_Enable(WDT_CLK_SRC_t Clock_Src, uint8_t ubTimeOut);
//------------------------------------------------------------------------------
/*!
\brief Enable watch dog interrupt timer
\param WDITSetup 		Setup parameter of watch dog interrupt timer.
\par Note:
		1. tCLK	  	  : Clock source (APB clock or 32768Hz).				
		2. tEM	  	  : Semaphore or queue.
		3. pvEvent	  : Semaphore or Queue handler.
\param ubTimeOut		Watch dog timeout (unit: second)
\return(no)
*/
void WDT_INT_Enable(WDIT_SETUP_t WDITSetup, uint8_t ubTimeOut);
//------------------------------------------------------------------------------
/*!
\brief Disable watch dog
\param tDevice			Select reset or interrupt timer base
\return(no)
*/
void WDT_Disable(WDT_DEIVCE_t tDevice);
//------------------------------------------------------------------------------
/*!
\brief Clear watch dog timer flag
\param tDevice			Select reset or interrupt timer base
\return(no)
*/
void WDT_TimerClr(WDT_DEIVCE_t tDevice);
//------------------------------------------------------------------------------
/*!
\brief Interrupt handler of Watch dog.
\par Note:
	1. Clear flag of watch dog interrupt timer.
	2. Event notify.
\return(no)
*/
void WDIT_INT_Handler(void);
//------------------------------------------------------------------------------
/*!
\brief 	Get Watch dog Version	
\return	Version
*/
uint16_t uwWDT_GetVersion(void);
#endif
