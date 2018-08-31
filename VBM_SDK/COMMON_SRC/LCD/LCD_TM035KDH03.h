/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TM035KDH03.h
	\brief		LCD TM035KDH03 Funcation Header
	\author		Pierce
	\version	0.3
	\date		2017/03/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TM035KDH03_H_
#define _LCD_TM035KDH03_H_
#include <stdbool.h>
#include "_510PF.h"
//------------------------------------------------------------------------------
#define TM035KDH03_CSIO				GPIO->GPIO_OE0
#define TM035KDH03_SDAIO			GPIO->GPIO_OE2
#define TM035KDH03_SCLIO			GPIO->GPIO_OE1
#define TM035KDH03_CS				GPIO->GPIO_O0
#define TM035KDH03_SDAO				GPIO->GPIO_O2
#define TM035KDH03_SCL				GPIO->GPIO_O1
#define TM035KDH03_SDAI				GPIO->GPIO_I2

#define LCD_TM035KDH03_ADDR_SFT		(5)
#define LCD_TM035KDH03_ADDR_MAX		(6)
#define LCD_TM035KDH03_ADDR_SB		(10)
#define LCD_TM035KDH03_DATA_SFT		(7)
#define LCD_TM035KDH03_DATA_MAX		(8)
#define LCD_TM035KDH03_DATA_MASK	(0xFF)
#define LCD_TM035KDH03_SETTING_SFT	(15)
#define LCD_TM035KDH03_SETTING_MAX	(16)
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_24RgbHv (void);
bool bLCD_TM035KDH03_24RgbDe (void);
bool bLCD_TM035KDH03_8RgbHv (void);
bool bLCD_TM035KDH03_IturBt601 (void);
bool bLCD_TM035KDH03_IturBt656 (void);
#endif
