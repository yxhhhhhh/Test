/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		Buzzer.h
	\brief		Buzzer header file
	\author		Ocean
	\version	0.2
	\date		2018/02/24
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BUZZER_H_
#define _BUZZER_H_
#include <stdint.h>

#define L_DO_1	261.626
#define L_RE_2	293.665
#define L_MI_3	329.628
#define L_FA_4	349.228
#define L_SOL_5	391.995
#define L_LA_6	440
#define L_SI_7	493.883

#define M_DO_1	523.251
#define	M_RE_2	587.330
#define	M_MI_3	659.255
#define	M_FA_4	698.456
#define	M_SOL_5	783.991
#define	M_LA_6	880
#define	M_SI_7	987.767

#define	H_DO_1	1046.502
#define H_RE_2	1174.659
#define	H_MI_3	1318.510
#define	H_FA_4	1396.913
#define	H_SOL_5	1567.982
#define	H_LA_6	1760
#define	H_SI_7	1975.533

#define	ZERO_0	0

typedef struct {
	uint16_t uwDelay;
	
	float BUZ_Freq0;
	uint8_t ubBUZ_TabSel0;
	uint8_t ubBUZ_LevIndex0;
	uint8_t ubBUZ_Volume0;	//max:63
	
	float BUZ_Freq1;
	uint8_t ubBUZ_TabSel1;
	uint8_t ubBUZ_LevIndex1;
	uint8_t ubBUZ_Volume1;	//max:63
	
	float BUZ_Freq2;
	uint8_t ubBUZ_TabSel2;
	uint8_t ubBUZ_LevIndex2;
	uint8_t ubBUZ_Volume2;	//max:63
	
	float BUZ_Freq3;
	uint8_t ubBUZ_TabSel3;
	uint8_t ubBUZ_LevIndex3;
	uint8_t ubBUZ_Volume3;	//max:63
}BUZ_ToneParam_t;

#define BUZ_WaitMutex                        \
if(BUZ_Mutex != NULL)                        \
    osMutexWait(BUZ_Mutex, osWaitForever);

#define BUZ_ReleaseMutex                     \
if(BUZ_Mutex != NULL)                        \
    osMutexRelease(BUZ_Mutex);

void BUZ_PlayStart(uint8_t ubChCnt, uint8_t ubWeight, BUZ_ToneParam_t *pToneParam, uint8_t ubToneGroupSize);
void BUZ_PlayStop(void);
void BUZ_PlayThread(void const *argument);
void BUZ_PlayPowerOnSound(void);
void BUZ_PlayPowerOffSound(void);
void BUZ_PlaySingleSound(void);
void BUZ_PlayLowBatSound(void);
void BUZ_PlayTestSound(void);
void BUZ_PlayTempAlarmSound(void);

#endif
