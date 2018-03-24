/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_vcs.c
	\brief		Version control command line
	\author		Hanyi Chiu
	\version	0.1
	\date		2017/07/23
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "cmd.h"
#include "VCS.h"
#include "CLI.h"
#ifdef CONFIG_CLI_CMD_VCS
//------------------------------------------------------------------------------
int32_t cmd_vcs_getVer(int argc, char* argv[])
{
	if((argc < 2) || (argc > 2))
		return cliFAIL;	

	uwVCS_LibraryVersionList(argv[1]);

	return cliPASS;	
}
#endif
