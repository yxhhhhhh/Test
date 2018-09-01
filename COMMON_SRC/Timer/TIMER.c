/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TIMER.c
	\brief		TIMER Function
	\author		Hanyi Chiu
	\version	0.5
	\date		2018/08/27
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "TIMER.h"
#include "INTC.h"

#define TIMER_MAJORVER	0
#define TIMER_MINORVER	5

//------------------------------------------------------------------------------
uint32_t ulTimer_DeviceAddr[] = {Timer1_BASE, Timer1_BASE, Timer1_BASE,
								 Timer2_BASE, Timer2_BASE, Timer2_BASE};
INTC_IrqSrc_t iTimer_IRQn[] = {INTC_TIMER1_IRQ, INTC_TIMER1_IRQ, INTC_TIMER1_IRQ,
							   INTC_TIMER2_IRQ, INTC_TIMER2_IRQ, INTC_TIMER2_IRQ};
INTC_IrqHandler iTimer_IrqHandler[] = {TIMER1_INT_Handler, TIMER1_INT_Handler, TIMER1_INT_Handler,
									   TIMER2_INT_Handler, TIMER2_INT_Handler, TIMER2_INT_Handler};
void* Timer_Event[TIMER_TOTAL_NUM];
TIMER_EVENT_METHOD_t Timer_EventMethod[TIMER_TOTAL_NUM];
osMutexId TIMER22_delayMutex;
//------------------------------------------------------------------------------
/*!	\file TIMER.c
Timer FlowChart:
	\dot
	digraph timer_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		TIMER_Init ->
		TIMER_Start ->
		TIMER_Stop;
	}
	\enddot
*/
//------------------------------------------------------------------------------
void TIMER_Init(void)
{
	uint8_t ubIdx;

	//! Clear Interrupt flag
	INTC_IrqClear(INTC_TIMER1_IRQ);
	INTC_IrqClear(INTC_TIMER2_IRQ);
	for(ubIdx = 0; ubIdx < TIMER_TOTAL_NUM; ubIdx++)
	{
		TIMER_Stop((TIMER_DEVICE_t)(TIMER1_1 + ubIdx));
		INTC_IrqSetup(iTimer_IRQn[(TIMER_DEVICE_t)(TIMER1_1 + ubIdx)], INTC_EDGE_TRIG, iTimer_IrqHandler[(TIMER_DEVICE_t)(TIMER1_1 + ubIdx)]);
		INTC_IrqEnable(iTimer_IRQn[(TIMER_DEVICE_t)(TIMER1_1 + ubIdx)]);
	}
	osMutexDef(TIMER22_delayMutex);
	TIMER22_delayMutex = osMutexCreate(osMutex(TIMER22_delayMutex));
}
//------------------------------------------------------------------------------
TIMER_RESULT TIMER_Start(TIMER_DEVICE_t tDevice, TIMER_SETUP_t TimerSetup)
{
	Timer_EventMethod[tDevice] = TimerSetup.tEM;
	Timer_Event[tDevice]  	   = TimerSetup.pvEvent;
	switch(tDevice)
	{
		case TIMER1_1:
		case TIMER2_1:
			TIMER_DEV->TM1_COUNTER 		= TimerSetup.ulTmCounter&0xFFFFFFFF;
			TIMER_DEV->TM1_LOAD 		= TimerSetup.ulTmLoad&0xFFFFFFFF;
			TIMER_DEV->TM1_MATCH1 		= TimerSetup.ulTmMatch1&0xFFFFFFFF;
			TIMER_DEV->TM1_MATCH2		= TimerSetup.ulTmMatch2&0xFFFFFFFF;
			TIMER_DEV->TM1_CLOCK 		= TimerSetup.tCLK;
			TIMER_DEV->TM1_OF_EN 		= TimerSetup.tOF;
			TIMER_DEV->TM1_UPDOWN 		= TimerSetup.tDIR;
			TIMER_DEV->TM1_MATCH1_MSK	= 0;
			TIMER_DEV->TM1_MATCH2_MSK	= 0;
			TIMER_DEV->TM1_OF_MSK		= 0;
			TIMER_DEV->TM1_EN 			= 1;
			break;
		case TIMER1_2:
		case TIMER2_2:	
			TIMER_DEV->TM2_COUNTER 		= TimerSetup.ulTmCounter&0xFFFFFFFF;
			TIMER_DEV->TM2_LOAD 		= TimerSetup.ulTmLoad&0xFFFFFFFF;
			TIMER_DEV->TM2_MATCH1 		= TimerSetup.ulTmMatch1&0xFFFFFFFF;
			TIMER_DEV->TM2_MATCH2		= TimerSetup.ulTmMatch2&0xFFFFFFFF;
			TIMER_DEV->TM2_CLOCK 		= TimerSetup.tCLK;
			TIMER_DEV->TM2_OF_EN 		= TimerSetup.tOF;
			TIMER_DEV->TM2_UPDOWN 		= TimerSetup.tDIR;
			TIMER_DEV->TM2_MATCH1_MSK	= 0;
			TIMER_DEV->TM2_MATCH2_MSK	= 0;
			TIMER_DEV->TM2_OF_MSK		= 0;
			TIMER_DEV->TM2_EN 			= 1;
			break;
		case TIMER1_3:
		case TIMER2_3:
			TIMER_DEV->TM3_COUNTER 		= TimerSetup.ulTmCounter&0xFFFFFFFF;
			TIMER_DEV->TM3_LOAD 		= TimerSetup.ulTmLoad&0xFFFFFFFF;
			TIMER_DEV->TM3_MATCH1 		= TimerSetup.ulTmMatch1&0xFFFFFFFF;
			TIMER_DEV->TM3_MATCH2		= TimerSetup.ulTmMatch2&0xFFFFFFFF;
			TIMER_DEV->TM3_CLOCK 		= TimerSetup.tCLK;
			TIMER_DEV->TM3_OF_EN 		= TimerSetup.tOF;
			TIMER_DEV->TM3_UPDOWN 		= TimerSetup.tDIR;
			TIMER_DEV->TM3_MATCH1_MSK	= 0;
			TIMER_DEV->TM3_MATCH2_MSK	= 0;
			TIMER_DEV->TM3_OF_MSK		= 0;
			TIMER_DEV->TM3_EN 			= 1;
			break;
		default:
			break;
	}
	return TIMER_OK;
}
//------------------------------------------------------------------------------
TIMER_RESULT TIMER_Stop(TIMER_DEVICE_t tDevice)
{
	switch(tDevice)
	{
		case TIMER1_1:
		case TIMER2_1:
			TIMER_DEV->TM1_EN 				= 0;
			TIMER_DEV->TM1_OF_EN 			= TIMER_OF_DISABLE;
			TIMER_DEV->TM1_MATCH1_MSK		= 1;
			TIMER_DEV->TM1_MATCH2_MSK		= 1;
			TIMER_DEV->TM1_OF_MSK			= 1;
			TIMER_DEV->TM1_COUNTER 			= 0;
			TIMER_DEV->TM1_LOAD 			= 0;
			TIMER_DEV->TM1_MATCH1 			= 0xFFFFFFFF;
			TIMER_DEV->TM1_MATCH2			= 0xFFFFFFFF;
			TIMER_DEV->CLR_TM1_OF_FLAG		= 1;
			TIMER_DEV->CLR_TM1_MATCH1_FLAG  = 1;
			TIMER_DEV->CLR_TM1_MATCH2_FLAG  = 1;
			break;
		case TIMER1_2:
		case TIMER2_2:
			TIMER_DEV->TM2_EN 				= 0;
			TIMER_DEV->TM2_OF_EN 			= TIMER_OF_DISABLE;
			TIMER_DEV->TM2_MATCH1_MSK		= 1;
			TIMER_DEV->TM2_MATCH2_MSK		= 1;
			TIMER_DEV->TM2_OF_MSK			= 1;
			TIMER_DEV->TM2_COUNTER 			= 0;
			TIMER_DEV->TM2_LOAD 			= 0;
			TIMER_DEV->TM2_MATCH1 			= 0xFFFFFFFF;
			TIMER_DEV->TM2_MATCH2			= 0xFFFFFFFF;
			TIMER_DEV->CLR_TM2_OF_FLAG		= 1;
			TIMER_DEV->CLR_TM2_MATCH1_FLAG  = 1;
			TIMER_DEV->CLR_TM2_MATCH2_FLAG  = 1;
			break;
		case TIMER1_3:
		case TIMER2_3:
			TIMER_DEV->TM3_EN 				= 0;
			TIMER_DEV->TM3_OF_EN 			= TIMER_OF_DISABLE;
			TIMER_DEV->TM3_MATCH1_MSK		= 1;
			TIMER_DEV->TM3_MATCH2_MSK		= 1;
			TIMER_DEV->TM3_OF_MSK			= 1;
			TIMER_DEV->TM3_COUNTER 			= 0;
			TIMER_DEV->TM3_LOAD 			= 0;
			TIMER_DEV->TM3_MATCH1 			= 0xFFFFFFFF;
			TIMER_DEV->TM3_MATCH2			= 0xFFFFFFFF;
			TIMER_DEV->CLR_TM3_OF_FLAG		= 1;
			TIMER_DEV->CLR_TM3_MATCH1_FLAG  = 1;
			TIMER_DEV->CLR_TM3_MATCH2_FLAG  = 1;
			break;
		default:
			break;
	}
	return TIMER_OK;
}
//------------------------------------------------------------------------------
void TIMER_Delay_us(uint16_t uwUs)
{
	osMutexWait(TIMER22_delayMutex, osWaitForever);	
	Timer2->TM2_CLOCK 	= TIMER_CLK_EXTCLK;
	Timer2->TM2_LOAD 	= 10 * uwUs;							//!< Donw count from TM3_LOAD @ 10MHz clock
	Timer2->TM2_COUNTER = Timer2->TM2_LOAD;
	Timer2->TM2_MATCH1 	= Timer2->TM2_LOAD + 1;					//!< never reach to this match value
	Timer2->TM2_MATCH2 	= Timer2->TM2_LOAD + 1;					//!< never reach to this match value

	Timer2->TM2_UPDOWN 	= TIMER_DOWN_CNT;						//!< Down count
	Timer2->TM2_OF_EN 	= TIMER_OF_ENABLE;						//!< Enable underflow interrupt11
	Timer2->TM2_OF_MSK 	= 1;
	Timer2->TM2_EN 		= 1;
	while(!Timer2->TM2_OF_FLAG);
	Timer2->TM2_EN 		= 0;
	Timer2->CLR_TM2_OF_FLAG = 1;
	osMutexRelease(TIMER22_delayMutex);
}
//------------------------------------------------------------------------------
void TIMER_Delay_ms(uint16_t uwMs)
{
	osMutexWait(TIMER22_delayMutex, osWaitForever);	
	Timer2->TM2_CLOCK 	= TIMER_CLK_EXTCLK;
	Timer2->TM2_LOAD 	= 10000 * uwMs;							//!< Donw count from TM3_LOAD @ 10MHz clock
	Timer2->TM2_COUNTER = Timer2->TM2_LOAD;
	Timer2->TM2_MATCH1 	= Timer2->TM2_LOAD + 1;					//!< never reach to this match value
	Timer2->TM2_MATCH2 	= Timer2->TM2_LOAD + 1;					//!< never reach to this match value

	Timer2->TM2_UPDOWN 	= TIMER_DOWN_CNT;						//!< Down count
	Timer2->TM2_OF_EN 	= TIMER_OF_ENABLE;						//!< Enable underflow interrupt11
	Timer2->TM2_OF_MSK 	= 1;
	Timer2->TM2_EN 		= 1;
	while(!Timer2->TM2_OF_FLAG);
	Timer2->TM2_EN 		= 0;
	Timer2->CLR_TM2_OF_FLAG = 1;
	osMutexRelease(TIMER22_delayMutex);
}
//------------------------------------------------------------------------------
void TIMER1_INT_Handler(void)
{	
	uint32_t TmFlag = (Timer1->TIMER_FLAG & 0x1FF);
	uint32_t TmMask = (Timer1->TIMER_MASK & 0x1FF);
	uint8_t ubEvent;
	TIMER_DEVICE_t tDev;

	while(1)
	{
		tDev = ((TmFlag & TIMER1_FLAG_MASK) && (!(TmMask & TIMER1_FLAG_MASK)))?TIMER1_1:
			   ((TmFlag & TIMER2_FLAG_MASK) && (!(TmMask & TIMER2_FLAG_MASK)))?TIMER1_2:
			   ((TmFlag & TIMER3_FLAG_MASK) && (!(TmMask & TIMER3_FLAG_MASK)))?TIMER1_3:TIMER_NULL;
		TmFlag &= ~((tDev == TIMER1_1)?TIMER1_FLAG_MASK:(tDev == TIMER1_2)?TIMER2_FLAG_MASK:(tDev == TIMER1_3)?TIMER3_FLAG_MASK:TmFlag);
		if(TIMER_NULL == tDev)
			break;
#ifdef RTOS
		if(Timer_EventMethod[tDev] == TIMER_SEMAPHORE)
		{
			if(*(osSemaphoreId*)Timer_Event[tDev] != NULL)
				osSemaphoreRelease(*(osSemaphoreId*)Timer_Event[tDev]);
		}
		else if(Timer_EventMethod[tDev] == TIMER_QUEUE)
		{
			ubEvent =(tDev == TIMER1_1)?TIMER1_1_EVENT:(tDev == TIMER1_2)?TIMER1_2_EVENT:(tDev == TIMER1_3)?TIMER1_3_EVENT:TIMER_DEFU_EVENT;
			if(*(osMessageQId*)Timer_Event[tDev] != NULL)
				osMessagePut(*(osMessageQId*)Timer_Event[tDev], &ubEvent, 0);
		}
		else if(Timer_EventMethod[tDev] == TIMER_CB)
#else
		if(Timer_EventMethod[tDev] == TIMER_CB)
#endif
		{
			pvTimerEvent_Cb Event_CB;

			ubEvent =(tDev == TIMER1_1)?TIMER1_1_EVENT:(tDev == TIMER1_2)?TIMER1_2_EVENT:(tDev == TIMER1_3)?TIMER1_3_EVENT:TIMER_DEFU_EVENT;
			Event_CB = (pvTimerEvent_Cb)Timer_Event[tDev];
			if(Event_CB != NULL)
				Event_CB(ubEvent);
		}
	}
	TmFlag = (Timer1->TIMER_FLAG & 0x1FF);
	Timer1->TIMER_FLAG = (TmFlag << 9) & 0x3FE00;
	INTC_IrqClear(INTC_TIMER1_IRQ);	
}
//------------------------------------------------------------------------------
void TIMER2_INT_Handler(void)
{
	uint32_t TmFlag = (Timer2->TIMER_FLAG & 0x1FF);
	uint32_t TmMask = (Timer2->TIMER_MASK & 0x1FF);
	uint8_t ubEvent;
	TIMER_DEVICE_t tDev;

	while(1)
	{
		tDev = ((TmFlag & TIMER1_FLAG_MASK) && (!(TmMask & TIMER1_FLAG_MASK)))?TIMER2_1:
			   ((TmFlag & TIMER2_FLAG_MASK) && (!(TmMask & TIMER2_FLAG_MASK)))?TIMER2_2:
			   ((TmFlag & TIMER3_FLAG_MASK) && (!(TmMask & TIMER3_FLAG_MASK)))?TIMER2_3:TIMER_NULL;
		TmFlag &= ~((tDev == TIMER2_1)?TIMER1_FLAG_MASK:(tDev == TIMER2_2)?TIMER2_FLAG_MASK:(tDev == TIMER2_3)?TIMER3_FLAG_MASK:TmFlag);
		if(TIMER_NULL == tDev)
			break;
#ifdef RTOS
		if(Timer_EventMethod[tDev] == TIMER_SEMAPHORE)
		{
			if(*(osSemaphoreId*)Timer_Event[tDev] != NULL)
				osSemaphoreRelease(*(osSemaphoreId*)Timer_Event[tDev]);
		}
		else if(Timer_EventMethod[tDev] == TIMER_QUEUE)
		{
			ubEvent =(tDev == TIMER2_1)?TIMER2_1_EVENT:(tDev == TIMER2_2)?TIMER2_2_EVENT:(tDev == TIMER2_3)?TIMER2_3_EVENT:TIMER_DEFU_EVENT;
			if(*(osMessageQId*)Timer_Event[tDev] != NULL)
				osMessagePut(*(osMessageQId*)Timer_Event[tDev], &ubEvent, 0);
		}
		else if(Timer_EventMethod[tDev] == TIMER_CB)
#else
		if(Timer_EventMethod[tDev] == TIMER_CB)
#endif
		{
			pvTimerEvent_Cb Event_CB;

			ubEvent =(tDev == TIMER2_1)?TIMER2_1_EVENT:(tDev == TIMER2_2)?TIMER2_2_EVENT:(tDev == TIMER2_3)?TIMER2_3_EVENT:TIMER_DEFU_EVENT;
			Event_CB = (pvTimerEvent_Cb)Timer_Event[tDev];
			if(Event_CB != NULL)
				Event_CB(ubEvent);
		}
	}
	TmFlag = (Timer2->TIMER_FLAG & 0x1FF);
	Timer2->TIMER_FLAG = (TmFlag << 9) & 0x3FE00;
	INTC_IrqClear(INTC_TIMER2_IRQ);
}
//------------------------------------------------------------------------------
uint16_t uwTIMER_GetVersion(void)
{
    return ((TIMER_MAJORVER << 8) + TIMER_MINORVER);
}
