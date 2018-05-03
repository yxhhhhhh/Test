/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_config.h
	\brief		BSP Config header file
	\author		Hanyi Chiu
	\version	0.1
	\date		2017/05/03
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_CONFIG_H_
#define _BSP_CONFIG_H_

#ifdef VBM_PU
#ifdef  _510PF_EVB_
#define BSP_BOARD_VBMPU_EV
#else
#define BSP_BOARD_VBMPU_DEMO
/*
#define LCDBL_ENABLE(en)												\
									{									\
										GPIO->GPIO_OE11 = en;			\
										GPIO->GPIO_O11 	= en; 			\
										PWM->PWM_EN8 	= en;			\
									}
									*/
#define LCDBL_ENABLE(en)			{PWM->PWM_EN8 	= en;}
#define LCD_BACKLIGHT_CTRL(LvL)		(PWM->PWM8_HIGH_CNT = LvL)
#define SPEAKER_EN(en)				(GPIO->GPIO_O2 = en)
//#define 	POWER_LED_IO				(GPIO->GPIO_O2)
//#define 	POWER_LED_IO_ENABLE			(GPIO->GPIO_OE2)
//#define	SIGNAL_LED_IO				(GPIO->GPIO_O3)
//#define	SIGNAL_LED_IO_ENABLE		(GPIO->GPIO_OE3)
#define	LCD_PWR_ENABLE				(GPIO->GPIO_O10 = 0)	//output low
#define	LCD_PWR_DISABLE				(GPIO->GPIO_O10 = 1)	//output high

#endif
#endif

#ifdef VBM_BU
#ifdef  _510PF_EVB_
#define BSP_BOARD_VBMBU_EV
#else
#define BSP_BOARD_VBMBU_DEMO
#define SPEAKER_EN(en)				(GPIO->GPIO_O2 = en)
//#define	PAIRING_LED_IO				(GPIO->GPIO_O2)
//#define	PAIRING_LED_IO_ENABLE		(GPIO->GPIO_OE2)
//#define 	POWER_LED_IO				(GPIO->GPIO_O1)
//#define 	POWER_LED_IO_ENABLE			(GPIO->GPIO_OE1)
//#define	SIGNAL_LED_IO				(GPIO->GPIO_O3)
//#define	SIGNAL_LED_IO_ENABLE		(GPIO->GPIO_OE3)
#endif
#endif

#endif
