/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UART.h
	\brief		UART Header file
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_CMD_H_
#define _UI_CMD_H_
#include "CLI.h"
//------------------------------------------------------------------------------
#ifdef CFG_UART1_ENABLE

int32_t cmd_uart1_test(int argc, char* argv[]);

extern struct cmd_table UART1_cmd_main_tbl[];

#endif
//------------------------------------------------------------------------------
#endif
