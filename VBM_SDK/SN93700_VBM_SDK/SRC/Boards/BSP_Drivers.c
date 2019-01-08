/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_drivers.c
	\brief		VBM PU/BU Demo/EV driver
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/10/26
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if defined VBM_PU || defined VBM_BU

#include <stdio.h>
#include "BSP.h"
#include "bsp_config.h"
#include "UART.h"
#include "WDT.h"
#include "TIMER.h"
#include "CQ_API.h"
#include "RTC_API.h"
#include "CIPHER_API.h"
#include "DMAC_API.h"
#include "SD_API.h"
#include "APBC.h"
#include "APP_CFG.h"

#ifdef BSP_BOARD_VBMPU_DEMO
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	TIMER_Init();
	CIPHER_Init();
	CQ_Init();
    APBC_Init();
	SD_Init();
#if 1
	//! LED
	//GPIO->GPIO_OE3 	= 1;
	//GPIO->GPIO_O3 	= 0;
	//GPIO->GPIO_OE13	= 1;
	//GPIO->GPIO_OE0	= 0;
	GPIO->GPIO_OE1		= 1;

	//! BackLight Control
	PWM->PWM8_RATE  	= 120;
	PWM->PWM8_PERIOD 	= 100;
	PWM->PWM8_HIGH_CNT 	= 0; //80
	
	//! BL Enable
	//GPIO->GPIO_OE11 = 1;
	//PWM->PWM_EN8    = 1;

	//! Speaker
	GPIO->GPIO_OE2 = 1;

	//! AUDIO+
	GPIO->GPIO_OE9 = 0;
	
	//! AUDIO-	
	GPIO->GPIO_OE10 = 0;

	//! USB_DET	
	GPIO->GPIO_OE11 = 0;

	//! FCHG_ON
	GPIO->GPIO_OE12 = 1;

	//! CHG_ON
	GPIO->GPIO_OE13 = 1;

	//! CHG_FULL
	GPIO->GPIO_OE0	= 0;

#else //DEMO
	//! LED
	GPIO->GPIO_OE2 	= 1;
	GPIO->GPIO_O2  	= 1;
	GPIO->GPIO_OE3 	= 1;
	GPIO->GPIO_O3 	= 0;
	GPIO->GPIO_OE13	= 1;
	GPIO->GPIO_OE0	= 1;
	GPIO->GPIO_OE1	= 1;

	//! BL Control
	PWM->PWM3_RATE  	= 127;
	PWM->PWM3_PERIOD 	= 0xC00;
	PWM->PWM3_HIGH_CNT 	= 0xA00;
	//! BL Enable
	GPIO->GPIO_OE11 = 1;
	GPIO->GPIO_O11  = 1;
	PWM->PWM_EN3    = 1;

	//! Speaker
	GPIO->GPIO_OE12 = 1;

	//! LCD POWER	
	GPIO->GPIO_OE10 = 1;
	GPIO->GPIO_O10  = 0;
#endif
	
	//! RTC GPIO1
	RTC_SetGPO_1(1, RTC_PullDownDisable);	

#if Current_Test
	GPIO->GPIO_O12 = 1; // LCD Power
#endif	
	
	printd(DBG_CriticalLvl, "SONiX SN9370X High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMPU_EV
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	TIMER_Init();
	CIPHER_Init();
	CQ_Init();
    APBC_Init();
	SDIO_Init();

	//! BL	
	//GPIO->GPIO_OE10 = 1;
	//GPIO->GPIO_O10  = 1;

	printd(DBG_CriticalLvl, "SONiX SN9370X High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMBU_DEMO
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
#ifdef CFG_UART1_ENABLE
	UART_Init(UART_1, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
#endif
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	TIMER_Init();
	CIPHER_Init();
	CQ_Init();
	SD_Init();

	//! LED
	GPIO->GPIO_OE1 	= 1;
	GPIO->GPIO_O1	= 1;
//	GPIO->GPIO_OE2 	= 1;
//	GPIO->GPIO_O2	= 0;
	GPIO->GPIO_OE3 	= 1;
//	GPIO->GPIO_O3	= 0;

	//! Speaker
	GPIO->GPIO_OE2 	= 1;

	//! IR LED
	GPIO->GPIO_OE4 	= 1;

	//! RTC GPIO1
//	RTC_SetGPO_1(1, RTC_PullDownDisable);

        //IR-CUT
	GPIO->GPIO_OE7 	= 1;
	GPIO->GPIO_OE8 	= 1;

	printd(DBG_CriticalLvl, "SONiX SN9370X High Speed Mode Start!\n");
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMBU_EV
void BSP_DriversInit(void)
{
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	UART_Init(UART_2, UART_CLOCK_96M, BR_115200, UART_1STOPBIT, NULL);
	TIMER_Init();
	CIPHER_Init();
	CQ_Init();
	SDIO_Init();
	
	printd(DBG_CriticalLvl, "SONiX SN9370X High Speed Mode Start!\n");	
}
#endif
//------------------------------------------------------------------------------

#endif //! End #if defined VBM_PU || defined VBM_BU
