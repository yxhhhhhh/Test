/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BSP.c
	\brief		Board support package
	\author		Hanyi Chiu
	\version	0.1
	\date		2017/05/03
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "BSP.h"

void BSP_Init(void)
{
	BSP_INIT_BOARD();
	BSP_INIT_DRIVERS();
}
