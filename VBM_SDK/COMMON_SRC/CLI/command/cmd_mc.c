/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_mc.c
	\brief		Motor control command line
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "MC_API.H"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_MC

uint8_t ubMC_ID = 0;
uint8_t ubMC_Direction[2];
uint8_t ubMC_Step[2];
MC_Setup_t MC_TestSetting;

osThreadId MC_TestId[2] = {NULL};
osThreadDef(MC_TestThread, MC_TestThread, osPriorityAboveNormal, 1, 512);

static void motor_ctrl_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: motor_ctrl <OPT> <ID> ...\n");
	printf(" OPT:\n");
	printf("	1: MC setup\n");
	printf("	2: MC start continue mode, manual stop\n");
	printf("	3: MC start counter mode wait ready, auto stop\n");
	printf("	4: MC start counter mode dont wait ready, auto stop\n");
	printf("	5: MC stop\n");
	printf(" ID:\n");
	printf("	0: Motor 0\n");
	printf("	1: Motor 1\n");
	printf(" Usage 1: motor_ctrl <OPT> <ID> <ClockDivider> <ClockPerPeriod> <HighPeriod> <PeriodPerStep> <Inv>\n");
	printf(" Example: MC setup motor 1\n");
	printf("          ClockDivider = 12\n");
	printf("          ClockPerPeriod = 200\n");
	printf("          HighPeriod = 20\n");
	printf("          PeriodPerStep = 10\n");
	printf("          Inv = MC_NormalWaveForm\n");
	printf("	motor_ctrl 1 1 12 200 20 10 0\n");
	printf(" Usage 2: motor_ctrl <OPT> <ID> <Direction>\n");
	printf(" Direction:\n");
	printf("	0: Counterclockwise\n");
	printf("	1: Clockwise\n");
	printf(" Example: MC0 start clockwise continue\n");
	printf("	motor_ctrl 2 0 1\n");
	printf(" Usage 3: motor_ctrl <OPT> <ID> <Direction> <Step>\n");
	printf(" Example: MC1 start counter mode wait ready, 8 steps, Counterclockwise\n");
	printf("	motor_ctrl 3 1 0 8\n");
	printf(" Usage 4: motor_ctrl <OPT> <ID> <Direction> <Step>\n");
	printf(" Example: MC0 start counter mode dont wait ready, 8 steps, clockwise\n");
	printf("	motor_ctrl 4 0 1 8\n");
	printf(" Example: MC0 stop\n");
	printf("	motor_ctrl 5 0\n");
	printf("###################################\n");
}  

//------------------------------------------------------------------------------
void MC_TestFinishIoHook(MC_No_t MC_No)
{
	printf("MC_TestFinishIoHook\n");

	if(MC_TestId[MC_No] == NULL) {
		MC_TestId[MC_No] = osThreadCreate(osThread(MC_TestThread), &MC_No);
		if(MC_TestId[MC_No] == NULL) {
			printd(DBG_ErrorLvl, "Create MC_TestId[%d] fail!\n", MC_No);
		}
	} else {
		osThreadResume(MC_TestId[MC_No]);
	}
}
//------------------------------------------------------------------------------
void MC_TestThread(void const *argument)
{
    MC_No_t MC_No;

    MC_No = *((MC_No_t*)argument);
    while(1) {
        if(MC_No == MC_0) {
			osDelay(2000);
			MC_Start(MC_No, ubMC_Step[MC_No], (MC_CW_t)ubMC_Direction[MC_No], MC_DontWait);
        } else if(MC_No == MC_1) {
			osDelay(2000);
			MC_Start(MC_No, ubMC_Step[MC_No], (MC_CW_t)ubMC_Direction[MC_No], MC_DontWait);
        }

        osThreadSuspend(NULL);
    }
}

int32_t cmd_motor_ctrl(int argc, char* argv[]) {
	uint8_t OPT = 0;

	if (argc < 2) {
		motor_ctrl_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);
	ubMC_ID = strtoul(argv[2], NULL, 0);

	if (OPT == 1) {	
		//MC setup
		printf("===================================\n");
		printf("MC setup\n");
		if (ubMC_ID == 0) {
			GLB->PADIO0 = 1;
			GLB->PADIO1 = 1;
			GLB->PADIO2 = 1;
			GLB->PADIO3 = 1;

			MC_TestSetting.ubMC_ClockDivider	= strtoul(argv[3], NULL, 0);
			MC_TestSetting.ubMC_ClockPerPeriod = strtoul(argv[4], NULL, 0);
			MC_TestSetting.ubMC_HighPeriod 	= strtoul(argv[5], NULL, 0);
			MC_TestSetting.ubMC_PeriodPerStep	= strtoul(argv[6], NULL, 0);
			MC_TestSetting.tMC_Inv 			= (MC_Inverse_t)strtoul(argv[7], NULL, 0);
			MC_TestSetting.pfMC_FinishHook 	= MC_TestFinishIoHook;

			printf("MC0 %d %d %d %d %d\n", 
				MC_TestSetting.ubMC_ClockDivider, 
				MC_TestSetting.ubMC_ClockPerPeriod,
				MC_TestSetting.ubMC_HighPeriod,
				MC_TestSetting.ubMC_PeriodPerStep,
				MC_TestSetting.tMC_Inv);
			
			if (tMC_Setup(MC_0, &MC_TestSetting) != MC_SUCCESS) {
				printf("MC_0 setup fail\n");
			}
		} else if (ubMC_ID == 1) {
			GLB->PADIO4 = 1;
			GLB->PADIO5 = 1;
			GLB->PADIO6 = 1;
			GLB->PADIO7 = 1;
			
			MC_TestSetting.ubMC_ClockDivider	= strtoul(argv[3], NULL, 0);
			MC_TestSetting.ubMC_ClockPerPeriod = strtoul(argv[4], NULL, 0);
			MC_TestSetting.ubMC_HighPeriod 	= strtoul(argv[5], NULL, 0);
			MC_TestSetting.ubMC_PeriodPerStep	= strtoul(argv[6], NULL, 0);
			MC_TestSetting.tMC_Inv 			= (MC_Inverse_t)strtoul(argv[7], NULL, 0);
			MC_TestSetting.pfMC_FinishHook 	= MC_TestFinishIoHook;
			
			printf("MC1 %d %d %d %d %d\n", 
				MC_TestSetting.ubMC_ClockDivider, 
				MC_TestSetting.ubMC_ClockPerPeriod,
				MC_TestSetting.ubMC_HighPeriod,
				MC_TestSetting.ubMC_PeriodPerStep,
				MC_TestSetting.tMC_Inv);

			if (tMC_Setup(MC_1, &MC_TestSetting) != MC_SUCCESS) {
				printf("MC_1 setup fail\n");
			}
		}
		printf("===================================\n");
	} else if (OPT == 2) {
		//MC start continue mode, manual stop
		if (ubMC_ID == 0) {
			ubMC_Direction[ubMC_ID] = strtoul(argv[3], NULL, 0);
			
			printf("===================================\n");
			if (ubMC_Direction[ubMC_ID] == MC_Clockwise) {
				printf("MC0 clockwise continue\n");
			} else {
				printf("MC0 Counterclockwise continue\n");
			}
			printf("===================================\n");
			MC_Start(MC_0, 0, (MC_CW_t)ubMC_Direction[ubMC_ID], MC_DontWait);
		} else if (ubMC_ID == 1) {
			ubMC_Direction[ubMC_ID] = strtoul(argv[3], NULL, 0);
			
			printf("===================================\n");
			if (ubMC_Direction[ubMC_ID] == MC_Clockwise) {
				printf("MC1 clockwise continue\n");
			} else {
				printf("MC1 Counterclockwise continue\n");
			}
			printf("===================================\n");
			MC_Start(MC_1, 0, (MC_CW_t)ubMC_Direction[ubMC_ID], MC_DontWait);
		} else {
			motor_ctrl_usage();
			return cliFAIL; 
		}
	} else if (OPT == 3) {
		//MC start counter mode wait ready, auto stop
		if (ubMC_ID == 0) {
			ubMC_Direction[ubMC_ID] = strtoul(argv[3], NULL, 0);
			ubMC_Step[ubMC_ID] = strtoul(argv[4], NULL, 0);
			
			if (ubMC_Step[ubMC_ID] == 0) {
				ubMC_Step[ubMC_ID] = 1;
			}
			
			printf("===================================\n");
			if (ubMC_Direction[ubMC_ID] == MC_Clockwise) {
				printf("MC0 clockwise wait %d counter\n", ubMC_Step[ubMC_ID]);
			} else {
				printf("MC0 Counterclockwise wait %d counter\n", ubMC_Step[ubMC_ID]);
			}
			printf("===================================\n");
			MC_Start(MC_0, ubMC_Step[ubMC_ID], (MC_CW_t)ubMC_Direction[ubMC_ID], MC_WaitReady);
		} else if (ubMC_ID == 1) {
			ubMC_Direction[ubMC_ID] = strtoul(argv[3], NULL, 0);
			ubMC_Step[ubMC_ID] = strtoul(argv[4], NULL, 0);

			if (ubMC_Step[ubMC_ID] == 0) {
				ubMC_Step[ubMC_ID] = 1;
			}

			printf("===================================\n");
			if (ubMC_Direction[ubMC_ID] == MC_Clockwise) {
				printf("MC1 clockwise wait %d counter\n", ubMC_Step[ubMC_ID]);
			} else {
				printf("MC1 Counterclockwise wait %d counter\n", ubMC_Step[ubMC_ID]);
			}
			printf("===================================\n");
			MC_Start(MC_1, ubMC_Step[ubMC_ID], (MC_CW_t)ubMC_Direction[ubMC_ID], MC_WaitReady);
		} else {
			motor_ctrl_usage();
			return cliFAIL; 
		}
	} else if (OPT == 4) {
		//MC start counter mode dont wait ready, auto stop
		if (ubMC_ID == 0) {
			ubMC_Direction[ubMC_ID] = strtoul(argv[3], NULL, 0);
			ubMC_Step[ubMC_ID] = strtoul(argv[4], NULL, 0);
			
			if (ubMC_Step[ubMC_ID] == 0) {
				ubMC_Step[ubMC_ID] = 1;
			}

			printf("===================================\n");
			if (ubMC_Direction[ubMC_ID] == MC_Clockwise) {
				printf("MC0 clockwise dont wait %d counter\n", ubMC_Step[ubMC_ID]);
			} else {
				printf("MC0 Counterclockwise dont wait %d counter\n", ubMC_Step[ubMC_ID]);
			}
			printf("===================================\n");
			MC_Start(MC_0, ubMC_Step[ubMC_ID], (MC_CW_t)ubMC_Direction[ubMC_ID], MC_DontWait);
		} else if (ubMC_ID == 1) {
			ubMC_Direction[ubMC_ID] = strtoul(argv[3], NULL, 0);
			ubMC_Step[ubMC_ID] = strtoul(argv[4], NULL, 0);

			if (ubMC_Step[ubMC_ID] == 0) {
				ubMC_Step[ubMC_ID] = 1;
			}

			printf("===================================\n");
			if (ubMC_Direction[ubMC_ID] == MC_Clockwise) {
				printf("MC1 clockwise dont wait %d counter\n", ubMC_Step[ubMC_ID]);
			} else {
				printf("MC1 Counterclockwise dont wait %d counter\n", ubMC_Step[ubMC_ID]);
			}
			printf("===================================\n");
			MC_Start(MC_1, ubMC_Step[ubMC_ID], (MC_CW_t)ubMC_Direction[ubMC_ID], MC_DontWait);
		} else {
			motor_ctrl_usage();
			return cliFAIL; 
		}
	} else if (OPT == 5) {
		//MC stop
		if (ubMC_ID == 0) {
			printf("===================================\n");
			printf("MC0 Stop\n");
			printf("===================================\n");
            MC_Stop(MC_0);
		} else if (ubMC_ID == 1) {
			printf("===================================\n");
			printf("MC1 Stop\n");
			printf("===================================\n");
            MC_Stop(MC_1);
		} else {
			motor_ctrl_usage();
			return cliFAIL; 
		}

	} else {
		motor_ctrl_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

#endif
