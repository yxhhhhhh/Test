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
#include <stdarg.h>
#include "CLI.h"
#include "UI_UART1.h"
#include "UI_CMD.h"

#ifdef CFG_UART1_ENABLE

osMessageQId tUART1_RecvQueue;
struct cmd_table *UART1_curr_lv;
#define UART1_MAX_CMD_LENGTH			48
#define UART1_RECV_QUEUE_SIZE			UART1_MAX_CMD_LENGTH

void UART1_PutChar(char ch)
{
	while(!UART1->TX_RDY);
	UART1->RS_DATA = ch;
}

void UART1_PutString(char *str)
{
	while(*str)
	{
		UART1_PutChar(*str++);
	}    
}

void UART1_printf(const char *fmt, ...)	/* variable arguments */
{
    va_list args;
    char printbuffer[128];

	va_start(args, fmt);
	vsprintf(printbuffer, fmt, args);
	va_end(args);

	UART1_PutString(printbuffer);
}

void UART1_RecvInit(void)
{
	UART1_curr_lv = UART1_cmd_main_tbl;
    /* Create and assert a queue for received characters */
	osMessageQDef(tUART1_RecvQueue, UART1_RECV_QUEUE_SIZE, sizeof(char));
	tUART1_RecvQueue = osMessageCreate(osMessageQ(tUART1_RecvQueue), NULL);
	
	osThreadDef(Uart1Thread, UART1_RecvThread, osPriorityBelowNormal, 1, 1280);
	osThreadCreate(osThread(Uart1Thread), NULL);
}

void UART1_rtoscli_recv(char ch)
{
	if((osMessagePut(tUART1_RecvQueue, (void*) &ch, 0)) != osOK) {
		printf("UART1 Q full\r\n");
	}
}

int32_t UART1_tok_cmd(char *argument, char *array_arg[16])
{
	int32_t num_arg = 0;
	char *delim=" ";

	array_arg[0] = strtok(argument,delim);
	
	for (num_arg = 1 ; num_arg < 16 ; num_arg++) {
		array_arg[num_arg] = strtok(NULL,delim);

		if(!array_arg[num_arg])
			break;
	}

	return num_arg;
}

int32_t UART1_parse_cmd(char *cmd)
{
	int32_t i = 0;
	int32_t ret = 0;
	int32_t found = 0;
	char *pargv[16];
	int32_t num_argc = 0;

	if (strlen(cmd) == 0)
		return 0;

	//memset(search_tmp, 0, 128);

	num_argc = UART1_tok_cmd(cmd, pargv);

	while (UART1_curr_lv[i].name != NULL) {
		if (UART1_curr_lv[i].len_name == strlen(pargv[0]))
			found = strncmp(UART1_curr_lv[i].name, pargv[0], UART1_curr_lv[i].len_name);
		else
			found = 1;	/* not found */

		if (!found) {
			if (UART1_curr_lv[i].Func != NULL) {				
				ret = UART1_curr_lv[i].Func(num_argc, pargv);
			}
			break;
		}
		i++;
	}

	if (found != 0) {
		ret = 0;
		printf("CMD not found\n");
	} else {
		ret = 1;
	}

	return ret;
}

void UART1_RecvThread(void const *argument)
{
	char ch;
	char cmdbuf[UART1_MAX_CMD_LENGTH];
	int32_t ulLoop = 0;
	
	for ( ; ; ) {
		/* The task is blocked until something appears in the queue */
		if (osMessageGet(tUART1_RecvQueue, (void*) &ch, osWaitForever)) {
    		if (ch == 0xd) { /* enter key */
    			if (ulLoop > 0) {
    				printf("\n");
    				cmdbuf[ulLoop] = 0x0;

    				UART1_parse_cmd(cmdbuf);
					memset(cmdbuf, 0, sizeof(cmdbuf));
    				ulLoop = 0;
    			}
    		} else {
				//printf("%c", ch);
				cmdbuf[ulLoop] = ch;
				ulLoop++;
				if (ulLoop >= UART1_MAX_CMD_LENGTH - 1) { // Over Max command length, Force excute command to avoid buffer overflow
					printf("\n");

    				UART1_parse_cmd(cmdbuf);
    				memset(cmdbuf, 0, sizeof(cmdbuf));
    				ulLoop = 0;
				}
    		}
		}
	}
}
#endif
