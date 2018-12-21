/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_ado.c
	\brief		Audio control command line
	\author		Ocean
	\version	0.1
	\date		2017/10/30
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "ADO_API.h"
#include "Buzzer.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_ADO

static void ado_wav_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : wav <OPT> <Index>\n");
	printf(" OPT:\n");
	printf("	0: wav play once\n");
	printf("	1: wav stop\n");
	printf("	2: wav play continue\n");
	printf("	3: wav stop & start play once\n");
	printf("	4: wav get play state\n");
	printf(" Index: wav index\n");
	printf(" Example 1: start play wav index 0 once\n");
	printf("	wav 0 0\n");
	printf(" Example 2: stop play wav\n");
	printf("	wav 1\n");
	printf(" Example 3: play wav index 1 continue\n");
	printf("	wav 2 1\n");
	printf(" Example 4: stop current wav play and play wav index 1\n");
	printf("	wav 3 1\n");
	printf(" Example 5: get play state\n");
	printf("	wav 4\n");
	printf("###################################\n");
}  

int32_t cmd_ado_wav(int argc, char* argv[])
{
	uint8_t OPT, ubIndex;
	
	if (argc < 2) {
		ado_wav_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);
	ubIndex = strtoul(argv[2], NULL, 0);
	if (OPT == 0) {
		ADO_WavPlay(ubIndex);
		printf("===================================\n");
		printf("wav play start once:%d\n",ubIndex);
		printf("===================================\n");		
	} else if (OPT == 1) {
		ADO_WavStop();
		printf("===================================\n");
		printf("wav play stop\n");
		printf("===================================\n");		
	} else if (OPT == 2) {
		ADO_WavRepeat(ubIndex);
		printf("===================================\n");
		printf("wav play start continue:%d\n",ubIndex);
		printf("===================================\n");		
	} else if (OPT == 3) {
		ADO_WavStop();
		ADO_WavPlay(ubIndex);
		printf("===================================\n");
		printf("wav play stop & start:%d\n",ubIndex);
		printf("===================================\n");		
	} else if (OPT == 4) {
		printf("===================================\n");
		printf("wav get play state:%d\n",tADO_GetWavState());
		printf("===================================\n");		
	} else {
		ado_wav_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

static void ado_buzzer_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : buzzer <OPT> <Index>\n");
	printf(" OPT:\n");
	printf("	0: buzzer play\n");
	printf("	1: buzzer stop\n");
	printf(" Index: sound index\n");
	printf(" Example 1: buzzer play power on sound\n");
	printf("	buzzer 0 1\n");
	printf(" Example 2: buzzer play stop\n");
	printf("	buzzer 1\n");
	printf("###################################\n");
}  

int32_t cmd_ado_buzzer(int argc, char* argv[])
{
	uint8_t OPT, ubIndex;
	
	if (argc < 2) {
		ado_buzzer_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);
	ubIndex = strtoul(argv[2], NULL, 0);
		
	if (OPT == 0) {
		printf("===================================\n");
		printf("buzzer play start:%d\n",ubIndex);
		printf("===================================\n");		
		if (ubIndex == 1) {
			BUZ_PlayPowerOnSound();
		}
		if (ubIndex == 2) {
			BUZ_PlayPowerOffSound();
		}
		if (ubIndex == 3) {
			BUZ_PlaySingleSound();
		}
		if (ubIndex == 4) {
			BUZ_PlayLowBatSound();
		}
		if (ubIndex == 5) {
			BUZ_PlayTestSound();
		}
	} else if (OPT == 1) {
		printf("===================================\n");
		printf("buzzer play stop\n");
		BUZ_PlayStop();
		printf("===================================\n");		
	} else {
		ado_buzzer_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}



#endif
