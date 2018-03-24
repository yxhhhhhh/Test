/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_sf.c
	\brief		SF control command line
	\author		Ocean
	\version	0.1
	\date		2017/10/13
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "BB_API.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_BB

static void bb_info_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : bb_info <OPT> <PerType> <Role>\n");
	printf(" OPT:\n");
	printf("	1: BB Get PER\n");
	printf("	2: Show all BB info\n");
	printf(" PerType:\n");
	printf("	0: BB_HEAD_PER\n");
	printf("	1: BB_VDO_PER\n");
	printf("	2: BB_ADO_PER\n");
	printf(" Role:\n");
	printf("	0: BB_GET_STA1_PER\n");
	printf("	1: BB_GET_STA2_PER\n");
	printf("	2: BB_GET_STA3_PER\n");
	printf("	3: BB_GET_STA4_PER\n");
	printf("	4: BB_GET_SLAVE_AP_PER\n");
	printf("	5: BB_GET_MASTER_AP_PER\n");
	printf(" Example 1: Get STA1 HEAD PER\n");
	printf("	bb_info 1 0 0\n");
	printf(" Example 2: Show all BB information\n");
	printf("	bb_info 2\n");
	printf("###################################\n");
}  

int32_t cmd_bb_info(int argc, char* argv[])
{
	uint8_t ubOpt = 0;
	uint8_t ubPerType = 0;
	uint8_t ubRole = 0;

	if (argc < 2) {
		bb_info_usage();
		return cliFAIL; 
	}
	
	ubOpt = strtoul(argv[1], NULL, 0);
	
	if (ubOpt == 1) {
		ubPerType = strtoul(argv[2], NULL, 0);
		ubRole = strtoul(argv[3], NULL, 0);
		
		printf("===================================\n");
		printf("Type:%d, Rule:%d, Per=%d\n", ubPerType, ubRole, 100 - ubBB_GetPer((PER_TYPE)ubPerType,(GET_PER_ROLE)ubRole));
		printf("===================================\n");
	} else if (ubOpt == 2) {
		printf("===================================\n");
		printf("Show all BB info\n");
		printf("STA1: H=%d, V=%d, A=%d\n", 
			100 - ubBB_GetPer(BB_HEAD_PER,BB_GET_STA1_PER), 
			100 - ubBB_GetPer(BB_VDO_PER,BB_GET_STA1_PER), 
			100 - ubBB_GetPer(BB_ADO_PER,BB_GET_STA1_PER));
		printf("STA2: H=%d, V=%d, A=%d\n", 
			100 - ubBB_GetPer(BB_HEAD_PER,BB_GET_STA2_PER), 
			100 - ubBB_GetPer(BB_VDO_PER,BB_GET_STA2_PER), 
			100 - ubBB_GetPer(BB_ADO_PER,BB_GET_STA2_PER));
		printf("STA3: H=%d, V=%d, A=%d\n", 
			100 - ubBB_GetPer(BB_HEAD_PER,BB_GET_STA3_PER), 
			100 - ubBB_GetPer(BB_VDO_PER,BB_GET_STA3_PER), 
			100 - ubBB_GetPer(BB_ADO_PER,BB_GET_STA3_PER));
		printf("STA4: H=%d, V=%d, A=%d\n", 
			100 - ubBB_GetPer(BB_HEAD_PER,BB_GET_STA4_PER), 
			100 - ubBB_GetPer(BB_VDO_PER,BB_GET_STA4_PER), 
			100 - ubBB_GetPer(BB_ADO_PER,BB_GET_STA4_PER));
		printf("===================================\n");
	} else {
		bb_info_usage();
		return cliFAIL; 
	}
	
	return cliPASS; 
}

#endif
