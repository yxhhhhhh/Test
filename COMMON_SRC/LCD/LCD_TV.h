/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TV.h
	\brief		LCD TV Funcation Header
	\author		Pierce
	\version	0.1
	\date		2017/03/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TV_H_
#define _LCD_TV_H_
#include <stdint.h>
#include "_510PF.h"
//------------------------------------------------------------------------------
void LCD_TV_Init (uint8_t ubMode, uint8_t ubProg);
#endif
