/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_bb.h
	\brief		BB control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/13
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_BB_H_
#define _CMD_BB_H_

#include <stdint.h>

int32_t cmd_bb_info(int argc, char* argv[]);

#define CMD_TBL_BB		CMD_TBL_ENTRY(		\
	"bb",	2,      NULL,			\
	"bb		- Enter bb control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_bb_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_BB_INFO	CMD_TBL_ENTRY(		\
	"bb_info",		7,      cmd_bb_info,			\
	"bb_info		- Show BB information", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
