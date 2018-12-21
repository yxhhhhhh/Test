/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_mc.h
	\brief		Motor control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_MC_H_
#define _CMD_MC_H_

#include <stdint.h>

int32_t cmd_motor_ctrl(int argc, char* argv[]);
void MC_TestThread(void const *argument);

#define CMD_TBL_MOTOR    CMD_TBL_ENTRY(          \
	"motor",		5,	NULL,       \
	"motor		- Enter Motor control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_mc_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_MOTOR_CTRL    CMD_TBL_ENTRY(          \
	"motor_ctrl",		10,	cmd_motor_ctrl,       \
	"motor_ctrl	- Motor control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
