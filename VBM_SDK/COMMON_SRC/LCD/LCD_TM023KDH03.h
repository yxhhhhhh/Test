/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TM023KDH03.h
	\brief		LCD TM023KDH03 Header
	\author		Pierce
	\version	0.1
	\date		2017/03/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TM023KDH03_H_
#define _LCD_TM023KDH03_H_
#include "_510PF.h"
//------------------------------------------------------------------------------
#define TM023KDH03_RSTIO			GPIO->GPIO_OE0
#define TM023KDH03_CSIO				GPIO->GPIO_OE1
#define TM023KDH03_RSTO				GPIO->GPIO_O0
#define TM023KDH03_CSO				GPIO->GPIO_O1

#define TM023KDH03_CMD(Cmd)			LCD->LCD_CMD = Cmd;		
#define TM023KDH03_DATA(Data)		LCD->LCD_DATA = Data;	
void LCD_TM023KDH03_Init(uint8_t ubValue);
void LCD_TM023KDH03_CPU(uint8_t ubMode);
#endif
