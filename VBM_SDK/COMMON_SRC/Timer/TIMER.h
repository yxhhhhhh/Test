/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TIMER.h
	\brief		TIMER Header File
	\author		Hanyi Chiu
	\version	0.5
	\date		2018/08/27
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _TIMER_H_
#define _TIMER_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------
#define TIMER_TOTAL_NUM					6

//! TIMER APP EVENT definition
#define TIMER1_1_EVENT					0x71
#define TIMER1_2_EVENT					0x72
#define TIMER1_3_EVENT					0x73
#define TIMER2_1_EVENT					0x74
#define TIMER2_2_EVENT					0x75
#define TIMER2_3_EVENT					0x76
#define TIMER_DEFU_EVENT				0x77

#define TIMER_DV_EMPTY					0				//!< Device is empty
#define TIMER_DV_OCCUPY					1				//!< Device is occupied

#define TIMER1_FLAG_MASK				0x7
#define TIMER2_FLAG_MASK				0x38
#define TIMER3_FLAG_MASK				0x1C0

#define TIMER_DEV						((Timer1_Type*)ulTimer_DeviceAddr[tDevice])

typedef void(*pvTimerEvent_Cb)(uint8_t);

typedef enum
{
	TIMER_ERR,							//!< TIMER OK state
	TIMER_OK							//!< TIMER Error state
} TIMER_RESULT;

typedef enum
{
	TIMER1_1,							//!< 32bit Timer1
	TIMER1_2,							//!< 32bit Timer1
	TIMER1_3,							//!< 32bit Timer1
	TIMER2_1,							//!< 32bit Timer2
	TIMER2_2,							//!< 32bit Timer2
	TIMER2_3,							//!< 32bit Timer2
	TIMER_NULL = 0xFF
} TIMER_DEVICE_t;

typedef enum
{
	TIMER_SEMAPHORE,					//!< Event triggered by RTOS semaphore
	TIMER_QUEUE,						//!< Event triggered by RTOS Queue
	TIMER_CB,							//!< Event triggered by Callback function
	TIMER_EM_NULL
} TIMER_EVENT_METHOD_t;

typedef enum
{
	TIMER_CLK_APBCLK,					//!< Timer clock source is APB clock
	TIMER_CLK_EXTCLK,					//!< Timer clock source is external clock(10MHz)
} TIMER_CLK_SRC_t;

typedef enum
{
	TIMER_OF_DISABLE,					//!< Disable Timer overflow or underflow interrupt
	TIMER_OF_ENABLE,					//!< Enable Timer overflow or underflow interrupt
} TIMER_OF_t;

typedef enum
{
	TIMER_DOWN_CNT,						//!< Timer is down count
	TIMER_UP_CNT,						//!< Timer is up count
} TIMER_CNT_DIRECT_t;

typedef enum
{
	TIMER_DELAY_US,
	TIMER_DELAY_MS,
} TIMER_DELAYUNIT_t;

typedef struct
{
	uint32_t 		 		ulTmCounter;
	uint32_t 		 		ulTmLoad;
	uint32_t 		 		ulTmMatch1;
	uint32_t 		 		ulTmMatch2;
	TIMER_CLK_SRC_t 		tCLK;
	TIMER_OF_t 		 		tOF;
	TIMER_CNT_DIRECT_t		tDIR;
	TIMER_EVENT_METHOD_t	tEM;
	void*			 		pvEvent;
} TIMER_SETUP_t;
//------------------------------------------------------------------------------
/*!
\brief Timer initialize
\return(no)
*/
void TIMER_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Start function of Timer
\param tDevice 			Select timer base.
\param TimerSetup		Setup timer parameters.
\par Note:
	1. ulTmCounter  : Counter value.
	2. ulTmLoad	  	: Auto reload value.
	3. ulTmMatch1	: Match value. 
	4. ulTmMatch2	: Match value. 
	5. tCLK	  	  	: Clock source (APB clock or 10MHz clock). 
	6. tOF	  	  	: Enable or disable timer overflow or underflow interrupt.
	7. tDIR	  	  	: Up or down count.
	8. tEM	  	  	: Semaphore or queue.
	9. pvEvent	  	: Semaphore or Queue handler.
\return Setup result of timer start
*/
TIMER_RESULT TIMER_Start(TIMER_DEVICE_t tDevice, TIMER_SETUP_t TimerSetup);
//------------------------------------------------------------------------------
/*!
\brief Stop function of Timer
\param tDevice 			Select timer base.
\return Setup result of timer stop
*/
TIMER_RESULT TIMER_Stop(TIMER_DEVICE_t tDevice);
//------------------------------------------------------------------------------
/*!
\brief Timer Delay function
\param uwUs				Delay Time.
\par Note:
	Delay unit : microseconds.
\return(no)
*/
void TIMER_Delay_us(uint16_t uwUs);
//------------------------------------------------------------------------------
/*!
\brief Timer Delay function
\param uwMs				Delay Time.
\par Note:
	Delay unit : milliseconds.
\return(no)
*/
void TIMER_Delay_ms(uint16_t uwMs);
//------------------------------------------------------------------------------
/*!
\brief Interrupt handler of Timer1
\par Note:
	1. Clear timer1 flag.
	2. Event notify.
\return(no)
*/
void TIMER1_INT_Handler(void);
//------------------------------------------------------------------------------
/*!
\brief Interrupt handler of Timer2
\par Note:
	1. Clear timer2 flag.
	2. Event notify.
\return(no)
*/
void TIMER2_INT_Handler(void);
//------------------------------------------------------------------------------
/*!
\brief 	Get TIMER Version	
\return	Version
*/
uint16_t uwTIMER_GetVersion(void);
#endif
