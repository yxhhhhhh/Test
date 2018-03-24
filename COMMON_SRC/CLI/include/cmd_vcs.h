/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_vcs.h
	\brief		Version control command line header file
	\author		Hanyi Chiu
	\version	0.1
	\date		2017/07/23
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_VCS_H_
#define _CMD_VCS_H_

#include <stdint.h>

int32_t cmd_vcs_getVer(int argc, char* argv[]);

#define CMD_TBL_VCS CMD_TBL_ENTRY(											\
	"VCS",			3,      				cmd_vcs_getVer,					\
	"VCS         	- Show Version",		CFG_DEFAULT_CMD_LEVEL,			\
	NULL,			NULL													\
),

#endif
