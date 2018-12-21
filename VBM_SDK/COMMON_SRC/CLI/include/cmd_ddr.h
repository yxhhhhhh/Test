/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_ddr.h
	\brief		DDR Memory Configuration command line header file
	\author		Ocean
	\version	0.1
	\date		2017/12/27
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_DDR_H_
#define _CMD_DDR_H_

#include <stdint.h>

int32_t cmd_ddr_getMemoryConf(int argc, char* argv[]);

#define CMD_TBL_DDR CMD_TBL_ENTRY(											\
	"ddr",			3,      				NULL,							\
	"ddr         	- Enter DRAM control",		CFG_DEFAULT_CMD_LEVEL,			\
	cmd_ddr_tbl,	cmd_main_tbl													\
),

#define CMD_TBL_DDR_GCONF CMD_TBL_ENTRY(         									\
	"ddr_gconf",	9,								cmd_ddr_getMemoryConf,       	\
	"ddr_gconf		- Get memory configuration",	CFG_DEFAULT_CMD_LEVEL,			\
	NULL,			NULL			\
),

#endif
