/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		main.c
	\brief		main function
	\author		Nick Huang
	\version	1.0
	\date		2018/02/04
	\copyright	Copyright (C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "_510PF.h"
//------------------------------------------------------------------------------
int main(void)
{
	SystemInit();
	APP_Init();
}
//------------------------------------------------------------------------------
void osMallocFail(void)
{
	printf("Malloc Fail\n");
	for( ;; );
}
//------------------------------------------------------------------------------
void osStackOverflow(osThreadId thread_id, signed char* thread_name)
{
	printf("Stack Overflow ID:0x%08x Name:\"%s\"\n", (uint32_t)thread_id, thread_name);
	for( ;; );
}
//------------------------------------------------------------------------------
void osIdleHook(void)
{
}
//------------------------------------------------------------------------------
void osThreadStackWarningHook(osThreadId thread_id, signed char* thread_name, int remained_stack_sz)
{
	printf("Stack Warning ID:0x%08x Name:\"%s\" Remained Stack Size: %d\n", (uint32_t)thread_id, thread_name, remained_stack_sz);
}
//------------------------------------------------------------------------------
