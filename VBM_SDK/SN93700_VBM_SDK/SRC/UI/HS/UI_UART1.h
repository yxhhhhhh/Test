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
#ifndef _UART1_H_
#define _UART1_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------
#ifdef CFG_UART1_ENABLE
void UART1_PutChar(char ch);
void UART1_PutString(char *str);
void UART1_printf(const char *fmt, ...);
void UART1_RecvInit(void);
void UART1_rtoscli_recv(char ch);
void UART1_RecvThread(void const *argument);
int32_t UART1_parse_cmd(char *cmd);
#endif

#endif
