/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_isp.h
	\brief		ISP control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/13
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_ISP_H_
#define _CMD_ISP_H_

#include <stdint.h>


int32_t cmd_isp_ctrl(int argc, char* argv[]);

#define CMD_TBL_ISP		CMD_TBL_ENTRY(		\
	"isp",	3,      NULL,			\
	"isp		- Enter isp control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_isp_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_ISP_CTRL	CMD_TBL_ENTRY(		\
	"isp_ctrl",		8,      cmd_isp_ctrl,			\
	"isp_ctrl	- isp control", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
