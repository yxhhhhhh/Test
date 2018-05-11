/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		system__510PF.c
	\brief		ARM926EJ-S Device Peripheral Access Layer Source File for 510PF
	\author		Nick Huang
	\version	1.1
	\date		2017/10/11
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "_510PF.h"
#include "RTC_API.h"
//------------------------------------------------------------------------------
uint32_t SystemCoreClock;														//!< Variable to hold the system core clock value
int32_t lSYS_DebugLvl = DBG_ErrorLvl;                                          //!< System Debug Level
uint32_t ulSYS_Func1State;
uint32_t ulSYS_Func2State;
//------------------------------------------------------------------------------
void SystemStartup(void)
{
}
//------------------------------------------------------------------------------
void SystemInit(void)
{
//	SystemCoreClockUpdate();
	GLB->REG_CK_MODE = 0;														//!< Save power consumption mode
	GLB->PLL300_DIV  = 28;
	GLB->AHB1_RATE 	 = 3;
	GLB->AHB2_RATE 	 = 15;
	GLB->AHB3_RATE   = 10;
	GLB->APBC_RATE   = 10;
	lSYS_DebugLvl    = DBG_ErrorLvl;
}
//------------------------------------------------------------------------------
void SystemReset(void)
{
}
//------------------------------------------------------------------------------
void SystemCoreClockUpdate(void)
{
}
//------------------------------------------------------------------------------
void SYS_SetCoreClock(SYS_CoreClkSel CoreClkSel)
{
	GLB->CPU_RATE = CoreClkSel;
}
//------------------------------------------------------------------------------
void SYS_SetState(SYS_STATE State)
{
	uint8_t ubTemp;

	ubTemp = wRTC_ReadSysRam(SYS_STATE_IMG_ADDR);
	ubTemp &= 0x0F;
	ubTemp |= State << 4;
	RTC_WriteSysRam(SYS_STATE_IMG_ADDR, ubTemp);
}
//------------------------------------------------------------------------------
SYS_STATE SYS_GetState(void)
{
	uint8_t ubTemp;

	ubTemp = wRTC_ReadSysRam(SYS_STATE_IMG_ADDR);
	return (SYS_STATE)(ubTemp >> 4);
}
//------------------------------------------------------------------------------
void SYS_SetPowerStates(SYS_PowerState_t tPowerState)
{
	switch(tPowerState)
	{
		case SYS_PS0:
			SYS_SetCoreClock(CPU_CLK_FS);
			GLB->AHB1_RATE = 3;
			GLB->AHB2_RATE = 15;
			GLB->AHB3_RATE = 10;
			GLB->APBC_RATE = 10;
			GLB->REG_0x001C = ulSYS_Func1State;
			GLB->REG_0x0020 = ulSYS_Func2State;
			break;
		case SYS_PS1:
			ulSYS_Func1State   = GLB->REG_0x001C;
			ulSYS_Func2State   = GLB->REG_0x0020;
			GLB->MT1_FUNC_DIS  = 1;
			GLB->MT2_FUNC_DIS  = 1;
			GLB->ISP_FUNC_DIS 	  = 1;
			GLB->ISP_MD_FUNC_DIS  = 1;
			GLB->ISP_DIS_FUNC_DIS = 1;
			GLB->ISP_YUV_PATH1_FUNC_DIS = 1;
			GLB->ISP_YUV_PATH2_FUNC_DIS = 1;
			GLB->ISP_YUV_PATH3_FUNC_DIS = 1;
			GLB->IMG_FUNC_DIS  = 1;
			GLB->H264_FUNC_DIS = 1;
			GLB->MIPI_FUNC_DIS = 1;
			GLB->ENC_FUNC_DIS  = 1;
			GLB->FAT_FUNC_DIS  = 1;
			GLB->MAC_FUNC_DIS  = 1;
			GLB->MT1_FUNC_DIS  = 1;
			GLB->MT2_FUNC_DIS  = 1;
			GLB->UH1_FUNC_DIS  = 1;
			GLB->UH2_FUNC_DIS  = 1;
			GLB->UD_FUNC_DIS   = 1;
			USB_PHY1->PHY_PWR  = 0;
			USB_PHY2->PHY_PWR  = 0;
			GLB->SDIO1_FUNC_DIS = 1;
			GLB->SDIO2_FUNC_DIS = 1;
			GLB->SDIO3_FUNC_DIS = 1;
			SYS_SetCoreClock(CPU_CLK_DIV1);
			GLB->AHB1_RATE = 8;
			GLB->AHB2_RATE = 15;
			GLB->AHB3_RATE = 15;
			GLB->APBC_RATE = 15;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void SYS_SetPrintLevel(SYS_PrintLevel_t tLevel)
{
	printf("===================================\n");
	printf("Old lv=%d\n",lSYS_DebugLvl);
	lSYS_DebugLvl = tLevel;
	printf("New lv=%d\n",lSYS_DebugLvl);
	printf("===================================\n");
}

uint16_t uwSYS_GetOsVersion(void)
{
    return osCMSIS_KERNEL;
}

//------------------------------------------------------------------------------
