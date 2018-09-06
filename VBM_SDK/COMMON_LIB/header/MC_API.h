/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		MC_API.h
	\brief		Motor control header file
	\author		Nick Huang
	\version	0.2
	\date		2017/10/19
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _MC_API_H_
#define _MC_API_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------
typedef enum
{
	MC_0,
	MC_1
} MC_No_t;

typedef enum
{
    MC_SUCCESS,
    MC_FAIL
} MC_Status_t;

typedef enum
{
    MC_NormalWaveForm,
    MC_InversedWaveForm
} MC_Inverse_t;

typedef enum
{
    MC_Counterclockwise,
    MC_Clockwise
} MC_CW_t;

typedef enum
{
    MC_WaitReady,
    MC_DontWait
} MC_WaitMode_t;

typedef struct
{
    uint8_t ubMC_ClockDivider:4;                    //!< Clock_base = 12MHz / ubMC_ClockDivider
    uint8_t ubMC_ClockPerPeriod;                    //!< Base_time = 1 / (Clock_base * ubMC_ClockPerPeriod)
    uint8_t ubMC_HighPeriod;						//!< High_pulse_time = ubMC_HighPeriod * Base_time
    uint8_t ubMC_PeriodPerStep;						//!< Step_time = ubMC_PeriodPerStep * Base_time
    MC_Inverse_t tMC_Inv;							//!< Inverse output timing
//    void (*pfMC_StartIoHook) (void);
//    void (*pfMC_StopIoHook) (void);
    void (*pfMC_FinishHook)(MC_No_t MC_No);
} MC_Setup_t;
//------------------------------------------------------------------------------
uint16_t uwMC_GetVersion(void);
MC_Status_t tMC_Setup(MC_No_t MC_No, MC_Setup_t* pMC_Setup);
void MC_Start(MC_No_t MC_No, uint8_t ubStep, MC_CW_t tMC_CW, MC_WaitMode_t tWaitMode);
void MC_Stop(MC_No_t MC_No);
//------------------------------------------------------------------------------

#endif
