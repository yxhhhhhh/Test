/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_system.h
	\brief		System control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_SYSTEM_H_
#define _CMD_SYSTEM_H_

#include <stdint.h>

int32_t cmd_system_date(int argc, char* argv[]);
int32_t cmd_system_phymem_rw(int argc, char* argv[]);
int32_t cmd_system_fps(int argc, char* argv[]);
int32_t cmd_system_padio(int argc, char* argv[]);
int32_t cmd_system_wdt(int argc, char* argv[]);

#define CMD_TBL_SYSTEM    CMD_TBL_ENTRY(          \
	"system",		6,	NULL,       \
	"system		- Enter System control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_system_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_SYS_DATE    CMD_TBL_ENTRY(          \
	"date",		4,	cmd_system_date,       \
	"date		- Get/Set date time",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_PHYMEM_RW    CMD_TBL_ENTRY(          \
	"phymem_rw",		9,	cmd_system_phymem_rw,       \
	"phymem_rw	- read/write ASIC register",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_FPS    CMD_TBL_ENTRY(          \
	"fps",		3,	cmd_system_fps,       \
	"fps		- Get current frame rate",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_PADIO    CMD_TBL_ENTRY(          \
	"padio",	5,	cmd_system_padio,       \
	"padio		- Get PADIO Config",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_WDT    CMD_TBL_ENTRY(          \
	"wdt",	3,	cmd_system_wdt,       \
	"wdt		- Watch Dog Timer Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
