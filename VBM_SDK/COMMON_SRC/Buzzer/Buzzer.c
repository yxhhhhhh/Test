/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		Buzzer.c
	\brief		Buzzer function
	\author		Ocean
	\version	0.2
	\date		2018/02/24
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include "Buzzer.h"
#include "ADO_API.h"

uint8_t ubBUZ_ToneGroupSize;
uint8_t ubBUZ_ChannelCnt;
uint8_t ubBUZ_PlayStatus = 0;
BUZ_ToneParam_t *pBUZ_ToneParam;

osThreadId BUZ_PlayId = NULL;
osThreadDef(BUZ_PlayThread, BUZ_PlayThread, osPriorityAboveNormal, 1, 512);
osMutexId BUZ_Mutex;
osMutexDef(BUZ_Mutex);

//------------------------------------------------------------------------------
void BUZ_PlayStart(uint8_t ubChCnt, uint8_t ubWeight, BUZ_ToneParam_t *pToneParam, uint8_t ubToneGroupSize) {
    if(BUZ_Mutex == NULL) {
        BUZ_Mutex = osMutexCreate(osMutex(BUZ_Mutex));
        if(BUZ_Mutex == NULL)
            printd(DBG_ErrorLvl, "Buzzer mutex creat fail!\n");
    }
	
	BUZ_WaitMutex;
	
	printd(DBG_InfoLvl, "BuzSize:%d\r\n",ubToneGroupSize);
	ubBUZ_ToneGroupSize = ubToneGroupSize;
	pBUZ_ToneParam = pToneParam;
	ubBUZ_ChannelCnt = ubChCnt;
	
	ADO->BUZ_EN = 0;

	ADO->BUZ_WEIGHT = (ubWeight > 8) ? 8 : ubWeight;
	ADO->AUD_WEIGHT = 8 - ADO->BUZ_WEIGHT;

	ADO->BUZ0_LEVEL = 0;
	ADO->BUZ0_LEV_TRG = 1;
	ADO->BUZ1_LEVEL = 0;
	ADO->BUZ1_LEV_TRG = 1;
	ADO->BUZ2_LEVEL = 0;
	ADO->BUZ2_LEV_TRG = 1;
	ADO->BUZ3_LEVEL = 0;
	ADO->BUZ3_LEV_TRG = 1;
	
	ADO->BUZ0_CNT = 0;
	ADO->BUZ1_CNT = 0;
	ADO->BUZ2_CNT = 0;
	ADO->BUZ3_CNT = 0;

	if(BUZ_PlayId == NULL) {
		BUZ_PlayId = osThreadCreate(osThread(BUZ_PlayThread), NULL);
		if(BUZ_PlayId == NULL) {
			printd(DBG_ErrorLvl, "Create BUZ_PlayId fail!\n");
		}
	} else {
		osThreadResume(BUZ_PlayId);
	}
}

void BUZ_PlayThread(void const *argument) {
    while(1) {		
		if( ADO_GetIpReadyStatus() == ADO_IP_READY ) {
			uint8_t ubLoop;
			
			ADO_SetDacAutoMute(ADO_OFF);					// Automute
			ADO->BUZ_EN = 1;
			
			for(ubLoop = 0; ubLoop < ubBUZ_ToneGroupSize; ubLoop++) {
				switch(ubBUZ_ChannelCnt) {
					case 4:
						ADO->BUZ3_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq3 * 32 * 128 / 16000);
						ADO->BUZ3_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel3;
						ADO->BUZ3_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex3;
						ADO->BUZ3_INDEX_TRG = 1;
						ADO->BUZ3_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume3;
						ADO->BUZ3_LEV_TRG = 1;
					case 3:
						ADO->BUZ2_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq2 * 32 * 128 / 16000);
						ADO->BUZ2_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel2;
						ADO->BUZ2_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex2;
						ADO->BUZ2_INDEX_TRG = 1;
						ADO->BUZ2_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume2;
						ADO->BUZ2_LEV_TRG = 1;
					case 2:
						ADO->BUZ1_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq1 * 32 * 128 / 16000);
						ADO->BUZ1_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel1;
						ADO->BUZ1_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex1;
						ADO->BUZ1_INDEX_TRG = 1;
						ADO->BUZ1_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume1;
						ADO->BUZ1_LEV_TRG = 1;
					case 1:
						ADO->BUZ0_CNT = round(pBUZ_ToneParam[ubLoop].BUZ_Freq0 * 32 * 128 / 16000);
						ADO->BUZ0_TAB_SEL = pBUZ_ToneParam[ubLoop].ubBUZ_TabSel0;
						ADO->BUZ0_LEV_INDEX = pBUZ_ToneParam[ubLoop].ubBUZ_LevIndex0;
						ADO->BUZ0_INDEX_TRG = 1;
						ADO->BUZ0_LEVEL = pBUZ_ToneParam[ubLoop].ubBUZ_Volume0;
						ADO->BUZ0_LEV_TRG = 1;
						break;
				}
				osDelay(pBUZ_ToneParam[ubLoop].uwDelay);
			}
			ADO->BUZ_EN = 0;
			ADO_SetDacAutoMute(ADO_ON); 					// Automute
			BUZ_ReleaseMutex;
			osThreadSuspend(NULL);
		} else {
			osDelay(20);
			printd(DBG_Debug1Lvl, "ADO IP NON READY\r\n");
		}
    }
}

void BUZ_PlayStop(void) {
	ubBUZ_ToneGroupSize = 0;
}

void BUZ_PlayPowerOnSound(void) {   
	static BUZ_ToneParam_t BUZ_PowerOnSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{50,	M_DO_1, 3,		0,		63},
		{50,	M_RE_2, 3,		0,		63},
		{50,	M_MI_3, 3,		0,		63},
		{50,	M_FA_4, 3,		0,		63},
		{50,	M_SOL_5,3,		0,		63},
		{50,	M_LA_6, 3,		0,		63},
		{50,	M_SI_7, 3,		0,		63},
		{50,	H_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, BUZ_PowerOnSound, sizeof(BUZ_PowerOnSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlayPowerOffSound(void) {   
	static BUZ_ToneParam_t BUZ_PowerOffSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{50,	H_DO_1, 3,		0,		63},
		{50,	M_SI_7, 3,		0,		63},
		{50,	M_LA_6, 3,		0,		63},
		{50,	M_SOL_5,3,		0,		63},
		{50,	M_FA_4, 3,		0,		63},
		{50,	M_MI_3, 3,		0,		63},
		{50,	M_RE_2, 3,		0,		63},
		{50,	M_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, BUZ_PowerOffSound, sizeof(BUZ_PowerOffSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlaySingleSound(void) {   
	static BUZ_ToneParam_t BUZ_SingleSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{30,	H_DO_1, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, BUZ_SingleSound, sizeof(BUZ_SingleSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlayLowBatSound(void) {   
	static BUZ_ToneParam_t BUZ_LowBatSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{30,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{20,	ZERO_0, 3,		0,		63},
		{30,	M_DO_1, 3,		0,		63},
		{50,	M_DO_1, 3,		15,		63},
		{150,	ZERO_0,	3,		0,		63},
		{30,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{20,	ZERO_0, 3,		0,		63},
		{30,	M_DO_1, 3,		0,		63},
		{50,	M_DO_1, 3,		15,		63},
		{150,	ZERO_0, 3,		0,		63},
		{30,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{20,	ZERO_0, 3,		0,		63},
		{30,	M_DO_1, 3,		0,		63},
		{50,	M_DO_1, 3,		15,		63},
		{150,	ZERO_0, 3,		0,		63},
	};
	
	BUZ_PlayStart(1, 8, BUZ_LowBatSound, sizeof(BUZ_LowBatSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlayAlarmSound(void) {   
	static BUZ_ToneParam_t BUZ_AlarmSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0
		{50,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
		{150,	ZERO_0, 3,		0,		63},
		{50,	H_DO_1, 3,		0,		63},
		{50,	H_DO_1, 3,		15,		63},
	};

	BUZ_PlayStart(1, 8, BUZ_AlarmSound, sizeof(BUZ_AlarmSound) / sizeof(BUZ_ToneParam_t));
}

void BUZ_PlayTestSound(void) {   
	#define BUZ_SMOOTH	1
	static BUZ_ToneParam_t BUZ_TestSound[] = {
	//	delay	Freq_0	Tab_0	LvIdx_0	Vol_0	Freq_1	Tab_1	LvIdx_1	Vol_1	Freq_2	Tab_2	LvIdx_2	Vol_2	Freq_3	Tab_3	LvIdx_3	Vol_3
		{200,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 1,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 2,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{200,	M_DO_1, 3,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{200,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1,	0,		0,		24, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 0,		0,		16, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{200,	M_DO_1, 0,		0,		8, 		ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		1,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		2,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		3,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1,	0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		4,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		5,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		6,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1,	0,		7,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		8,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#if BUZ_SMOOTH
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
#endif
		{200,	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		9,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		10,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		11,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		12,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		13,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		14,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},

		{500,	M_DO_1, 0,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
		{500,	M_DO_1, 0,		15,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32, 	ZERO_0, 3,		0,		32},
	};
	
	BUZ_PlayStart(4, 8, BUZ_TestSound, sizeof(BUZ_TestSound) / sizeof(BUZ_ToneParam_t));
}

