/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_lcd.h
	\brief		LCD control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_LCD_H_
#define _CMD_LCD_H_

#include <stdint.h>

int32_t cmd_lcd_ctrl(int argc, char* argv[]);

#define CMD_TBL_LCD    CMD_TBL_ENTRY(          \
	"lcd",		3,	NULL,       \
	"lcd		- Enter LCD control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_lcd_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_LCD_CTRL    CMD_TBL_ENTRY(          \
	"lcd_ctrl",		8,	cmd_lcd_ctrl,       \
	"lcd_ctrl	- LCD register R/W",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
