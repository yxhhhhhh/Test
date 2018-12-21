/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WDT.c
	\brief		Watch Dog Timer Function
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "WDT.h"
#include "INTC.h"

#define WDT_MAJORVER	0
#define WDT_MINORVER	3

uint32_t ulWDT_DeviceAddr[] = {WDRT_BASE, WDIT_BASE};
void *WDIT_Event;
void *WDIT_INTHandler;
WDIT_EVENT_METHOD_t WDIT_EventMethod;
//------------------------------------------------------------------------------
void WDT_RST_Enable(WDT_CLK_SRC_t Clock_Src, uint8_t ubTimeOut)
{
	uint32_t ulTimer_Cnt;

	WDT_TimerClr(WDT_RST);
	WDRT->WDOG_CTR     = 0;
	WDRT->WD_LENGTH    = 0xFF;
	ulTimer_Cnt		   = (Clock_Src == WDT_CLK_APBCLK)?((IP_CLK/GLB->APBC_RATE)*ubTimeOut):(EXT_CLK*ubTimeOut);
	WDRT->WDOG_LOAD    = ulTimer_Cnt;
	WDRT->WDOG_RESTART = 0x5AB9;
	WDRT->WDOG_CLOCK   = Clock_Src;
	WDRT->WDOG_RST_EN  = 1;
	WDRT->WDOG_EN      = 1;
}
//------------------------------------------------------------------------------
void WDT_INT_Enable(WDIT_SETUP_t WDITSetup, uint8_t ubTimeOut)
{
	uint32_t ulTimer_Cnt;

	WDT_TimerClr(WDT_INT);
	WDIT->WDOG_CTR     = 0;
	WDIT->WD_LENGTH    = 0xFF;

	ulTimer_Cnt 	   = (WDITSetup.tCLK == WDT_CLK_APBCLK)?((IP_CLK/GLB->APBC_RATE)*ubTimeOut):(EXT_CLK*ubTimeOut);
	WDIT->WDOG_LOAD    = ulTimer_Cnt;
	WDIT->WDOG_RESTART = 0x5AB9;
	WDIT->WDOG_CLOCK   = WDITSetup.tCLK;
#ifdef RTOS
	WDIT_EventMethod   = WDITSetup.tEM;
	WDIT_Event		   = WDITSetup.pvEvent;
	WDIT_INTHandler	   = WDITSetup.pvINTHandler;
#endif
	INTC_IrqSetup(INTC_WDOG_IRQ, INTC_EDGE_TRIG, (WDITSetup.pvINTHandler == NULL)?WDIT_INT_Handler:(INTC_IrqHandler)WDITSetup.pvINTHandler);
	INTC_IrqEnable(INTC_WDOG_IRQ);
	WDIT->WDOG_INTR_EN = 1;
	WDIT->WDOG_EN      = 1;
}
//------------------------------------------------------------------------------
void WDT_Disable(WDT_DEIVCE_t tDevice)
{
	WDT_DEV->WDOG_CTR = 0;
}
//------------------------------------------------------------------------------
void WDT_TimerClr(WDT_DEIVCE_t tDevice)
{
	WDT_DEV->CLR_WDOG_FLAG = 1;
	WDT_DEV->WDOG_RESTART  = 0x5AB9;
}
//------------------------------------------------------------------------------
void WDIT_INT_Handler(void)
{
#ifdef RTOS
	uint8_t ubEvent;
#endif
	INTC_IrqClear(INTC_WDOG_IRQ);
	WDT_TimerClr(WDT_INT);
#ifdef RTOS
	if(WDIT_EventMethod == WDIT_SEMAPHORE)
	{
		if(*(osSemaphoreId*)WDIT_Event != NULL)
			osSemaphoreRelease(*(osSemaphoreId*)WDIT_Event);
	}
	else if(WDIT_EventMethod == WDIT_QUEUE)
	{
		if(*(osMessageQId*)WDIT_Event != NULL)
			osMessagePut(*(osMessageQId*)WDIT_Event, &ubEvent, 0);
	}
#endif
}
//------------------------------------------------------------------------------
uint16_t uwWDT_GetVersion(void)
{
    return ((WDT_MAJORVER << 8) + WDT_MINORVER);
}
