/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UART.c
	\brief		UART Function
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CLI.h"
#include "UI_CMD.h"

#ifdef CFG_UART1_ENABLE

#define CMD_TBL_UART1_TEST_1	CMD_TBL_ENTRY(		\
	"uart1_test",		10,      cmd_uart1_test,	\
	"uart1 test cmd",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

struct cmd_table UART1_cmd_main_tbl[] = {
	CMD_TBL_UART1_TEST_1
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

int32_t cmd_uart1_test(int argc, char* argv[])
{                          
	if (argc < 3) {
	//	return cliFAIL; 
	}

	printf("argv1 = %s\n", argv[1]);
	printf("argv2 = %s\n", argv[2]);
	printf("argv3 = %s\n", argv[3]);

	return cliPASS;	
}
#endif
