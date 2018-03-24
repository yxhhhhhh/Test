/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_sf.h
	\brief		SF control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/13
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_SF_H_
#define _CMD_SF_H_

#include <stdint.h>

int32_t cmd_sf_info(int argc, char* argv[]);
int32_t cmd_sf_ctrl(int argc, char* argv[]);

#define CMD_TBL_SF		CMD_TBL_ENTRY(		\
	"sf",	2,      NULL,			\
	"sf		- Enter sf control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_sf_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_SF_INFO	CMD_TBL_ENTRY(		\
	"sf_info",		7,      cmd_sf_info,			\
	"sf_info		- show sf information", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SF_CTRL	CMD_TBL_ENTRY(		\
	"sf_ctrl",		7,      cmd_sf_ctrl,			\
	"sf_ctrl		- sf control", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
