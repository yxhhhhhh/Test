/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_HS.c
	\brief		Application function (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.3
	\date		2017/11/27
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "APP_HS.h"
#include "APP_CFG.h"
#include "BUF.h"
#include "FWU_API.h"
#include "PROFILE_API.h"
#include "RTC_API.h"
#include "SF_API.h"
#include "MMU_API.h"
#include "DMAC_API.h"
#include "BB_API.h"
#include "VDO.h"
#include "ADO.h"
#include "RC.h"
#include "CLI.h"
#include "LCD.h"
#include "WDT.h"
#include "OSD.h"
#include "UI_VBMPU.h"
#include "TIMER.h"
#include "WDT.h"
#ifdef CFG_UART1_ENABLE
#include "UI_UART1.h"
#endif
//------------------------------------------------------------------------------
extern uint32_t Image$$RW_UNCACHED_HEAP$$Base;
extern uint32_t Image$$RW_IRAM3$$Base;

extern uint8_t ubDisplaymodeFlag;
extern uint8_t ubSetViewCam;

extern uint8_t ubFactorySettingFLag;
extern uint8_t ubClearOsdFlag_2;

extern uint8_t ubFactoryModeFLag;

uint8_t ubLinkonceflag = 0;
//------------------------------------------------------------------------------
const uint8_t ubAPP_SfWpGpioPin __attribute__((section(".ARM.__at_0x00005FF0"))) = SF_WP_GPIN;
static uint8_t osHeap[osHeapSize] __attribute__((aligned (8)));
osMessageQId APP_EventQueue;
pvAPP_StateCtrl APP_StateCtrlFunc;
static APP_StatusReport_t tAPP_StsReport;
osMutexId APP_UpdateMutex;
uint32_t ulAPP_WaitTickTime;
static APP_StaNumMap_t tAPP_STANumTable[CAM_4T] =
{
	[CAM1] = {KNL_STA1, PAIR_STA1, TWC_STA1},
	[CAM2] = {KNL_STA2, PAIR_STA2, TWC_STA2},
	[CAM3] = {KNL_STA3, PAIR_STA3, TWC_STA3},
	[CAM4] = {KNL_STA4, PAIR_STA4, TWC_STA4},
};
static APP_KNLInfo_t tAPP_KNLInfo;
#ifdef VBM_PU
static APP_DispLocMap_t tAPP_DispLocMap[] =
{
	//! Qual View
	[DISP_UPPER_LEFT]  = {KNL_DISP_LOCATION3},
	[DISP_UPPER_RIGHT] = {KNL_DISP_LOCATION1},
	[DISP_LOWER_LEFT]  = {KNL_DISP_LOCATION4},
	[DISP_LOWER_RIGHT] = {KNL_DISP_LOCATION2},
	//! Dual View
	[DISP_LEFT]  	   = {KNL_DISP_LOCATION2},
	[DISP_RIGHT]  	   = {KNL_DISP_LOCATION1},
	//! Single View
	[DISP_1T]		   = {KNL_DISP_LOCATION1},
};
APP_PairRoleInfo_t tAPP_PairRoleInfo;
#endif
static void APP_StartThread(void const *argument);
static void APP_WatchDogThread(void const *argument);

void RTC_PowerOff(void)
{
	printd(DBG_Debug1Lvl, "RTC_PowerOff Power OFF!\n");
	RTC_WriteUserRam(RECORD_PWRSTS_ADDR, PWRSTS_KEEP);
	RTC_PowerDisable();
	while(1);
}

uint8_t APP_GetBatteryValue(void)
{
	return 50;
}

uint8_t APP_CheckBootStatus(void)
{
	#ifdef VBM_PU
	
	#if 0
	#define CHECK_COUNT		10
	uint16_t checkCount = 0;
	printd(Apk_DebugLvl, "APP_CheckBootStatus USB: %d.\n", UI_GetUsbDet());

	while(1)
	{
		if(ubRTC_GetKey() == 1)
		{
			checkCount++;
			if(checkCount >= CHECK_COUNT)
			{
				break;
			}
		}
		else
		{
			if(checkCount >= CHECK_COUNT)
			{
				break;
			}
			else
			{
				checkCount = 0;
				RTC_PowerOff();
			}
		}
		
		TIMER_Delay_ms(200);
		WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	}
	printd(Apk_DebugLvl, "APP_CheckBootStatus USB: %d, checkCount: %d.\n", UI_GetUsbDet(), checkCount);
	#else
	#define CHECK_COUNT		10
	uint16_t checkCount = 0;
	printd(Apk_DebugLvl, "APP_CheckBootStatus USB: %d, ubRTC_GetKey(): %d.\n", UI_GetUsbDet(), ubRTC_GetKey());

	while(1)
	{
		if(UI_GetUsbDet() == 1) //Usb On
		{
			TIMER_Delay_ms(1000);
			break;
		}
		else
		{
			if(APP_GetBatteryValue() <= 10)
			{
				break;
			}
			
			if(ubRTC_GetKey() == 1)
			{
				checkCount++;
				if(checkCount >= CHECK_COUNT)
				{
					printd(Apk_DebugLvl, "break@@@\n");
					break;
				}
			}
			else
			{
				printd(Apk_DebugLvl, "PowerOff!!!\n");
				RTC_PowerOff();
			}
		}
		TIMER_Delay_ms(200);
		WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	}
	printd(Apk_DebugLvl, "APP_CheckBootStatus USB: %d, checkCount: %d.\n", UI_GetUsbDet(), checkCount);
	#endif
	
	//LCDBL_ENABLE(UI_ENABLE);
	#endif
}
//------------------------------------------------------------------------------
void APP_Init(void)
{
    osStatus APP_OsStatus;
    uint8_t* pUncachedHeap = (uint8_t*)&Image$$RW_UNCACHED_HEAP$$Base;
    uint32_t ulUncachedHeapSize = (uint32_t)&Image$$RW_IRAM3$$Base - (uint32_t)&Image$$RW_UNCACHED_HEAP$$Base;

    APP_OsStatus = osKernelInitialize(osHeap, osHeapSize, pUncachedHeap, ulUncachedHeapSize);
#ifdef OP_AP
	RTC_Init(RTC_TimerEnable);
#else
	RTC_Init(RTC_TimerDisable);
#endif

	#if 0 //def VBM_PU //20180330
	if(ubRTC_GetKey() == 0)
	{
		RTC_PowerOff();
	}
	#endif

	BSP_Init();
	APP_CheckBootStatus();
	//printd(DBG_InfoLvl, "%s\n", osKernelSystemId);	// Move to CLI VCS command
    if(APP_OsStatus != osOK)
    {
        printd(DBG_ErrorLvl, "RTOS initial fail\n");
    }

	if (ubAPP_SfWpGpioPin <= 13) {
		printd(DBG_InfoLvl, "SF_WP=GPIO%d\n",ubAPP_SfWpGpioPin);
	}
	SF_SetWpPin(ubAPP_SfWpGpioPin);
	SF_Init();
	PROF_Init();
	MMU_Init();
	TWC_Init();
	CLI_Init();
	
	#ifdef CFG_UART1_ENABLE
	UART1_RecvInit();
	#endif
	
	FWU_Init();
	
	UI_Init(&APP_EventQueue);
	
	tAPP_StsReport.tAPP_State  	= APP_POWER_OFF_STATE;
	ulAPP_WaitTickTime      	= 0;	//!< osWaitForever;
	APP_StateCtrlFunc 			= APP_StateFlowCtrl;
	osMutexDef(AppUpdate);
	APP_UpdateMutex 			= osMutexCreate(osMutex(AppUpdate));
	osMessageQDef(APP_EventQueue, APP_EVENTQUEUE_SZ, APP_EventMsg_t);
	APP_EventQueue = osMessageCreate(osMessageQ(APP_EventQueue), NULL);
    osThreadDef(APP_StartThread, APP_StartThread, THREAD_PRIO_APP_HANDLER, 1, THREAD_STACK_APP_HANDLER);
    if(osThreadCreate(osThread(APP_StartThread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_StartThread fail!\n");
		while(1);
	}
    osThreadDef(APP_WatchDogThread, APP_WatchDogThread, THREAD_PRIO_WDT_HANDLER, 1, 256);
    if(osThreadCreate(osThread(APP_WatchDogThread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_WatchDogThread fail!\n");
		while(1);
	}
	/*! Start the kernel.  From here on, only tasks and interrupts will run. */
	osKernelStart();

	/*! If all is well, the scheduler will now be running, and the following
	line will never be reached. */
	for( ;; );
}
//------------------------------------------------------------------------------
void APP_StartThread(void const *argument)
{
	osStatus osAPP_EventStauts;
	APP_EventMsg_t tAPP_EventMsg;

	while(1)
	{
		osAPP_EventStauts = osMessageGet(APP_EventQueue, &tAPP_EventMsg, ulAPP_WaitTickTime);
		if(osAPP_EventStauts == osEventTimeout)
			tAPP_EventMsg.ubAPP_Event = APP_REFRESH_EVENT;
		if(APP_StateCtrlFunc)
			APP_StateCtrlFunc(&tAPP_EventMsg);
	}
}
//------------------------------------------------------------------------------
void APP_WatchDogThread(void const *argument)
{
	while(1)
	{
		WDT_TimerClr(WDT_RST);
		osDelay(500);
	}
}
//------------------------------------------------------------------------------
void APP_PowerOnFunc(void)
{
	uint32_t ulBUF_StartAddr = 0;

	APP_LoadKNLSetupInfo();

#if USBD_ENABLE
	//! USB Device initialization
	USBD_Init(tAPP_KNLInfo.tUsbdClassMode);
#endif

	//! Firmware Upgrade Setup
	APP_FWUgradeSetup();

	//! System initialization
	DMAC_Init();
	PAIR_Init(&APP_EventQueue);

	ulBUF_StartAddr  = ulMMU_GetBufStartAddr();
	//! UI Buffer Setup
	ulBUF_StartAddr += ulUI_BufSetup(ulBUF_StartAddr);

	//! Kernel / Buffer initialization
	KNL_Init();
	BUF_Init(ulBUF_StartAddr);

	//! Kernel Parameter Setup
	APP_KNLParamSetup();

	//! Rate Control Preset Setup
	RC_PresetSetup(RC_QTY_AND_BW);

	//! Video / Audio initialization
	VDO_Init();
	ADO_Init();

	//! Kernel Buffer Calculate
	KNL_BufInit();

	//! UI Plug-in
	UI_PlugIn();

	//! Application Start
	APP_Start();
}
//------------------------------------------------------------------------------
void APP_StateFlowCtrl(APP_EventMsg_t *ptEventMsg)
{
	static APP_StateFunc_t tAppStateFunc[] =
	{
		[APP_POWER_OFF_STATE] 	= APP_PowerCtrlFunc,
		[APP_IDLE_STATE] 		= APP_IdleStateFunc,
		[APP_LINK_STATE] 		= APP_LinkStateFunc,
		[APP_LOSTLINK_STATE] 	= APP_LostLinkStateFunc,
		[APP_PAIRING_STATE] 	= APP_PairingStateFunc,
	};
	if(tAppStateFunc[tAPP_StsReport.tAPP_State].pvFuncPtr)
		tAppStateFunc[tAPP_StsReport.tAPP_State].pvFuncPtr(ptEventMsg);
}
//------------------------------------------------------------------------------
void APP_PowerCtrlFunc(APP_EventMsg_t *ptEventMsg)
{
	ulAPP_WaitTickTime = osWaitForever;
	switch(tAPP_StsReport.tAPP_State)
	{
		case APP_POWER_OFF_STATE:
			APP_PowerOnFunc();
			break;
		default:
			tAPP_StsReport.tAPP_State = APP_POWER_OFF_STATE;
			break;
	}
}
//------------------------------------------------------------------------------
void APP_IdleStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef VBM_PU
		case APP_UNBIND_BU_EVENT:
			APP_doUnbindBU(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LinkStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef VBM_PU
		case APP_UNBIND_BU_EVENT:
			APP_doUnbindBU(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
		case APP_ADOSRCSEL_EVENT:
		{
			KNL_ROLE tKNL_Role = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tKNL_StaNum;

			tAPP_KNLInfo.tAdoSrcRole = tKNL_Role;
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
			APP_UpdateKNLSetupInfo();
			break;
		}
		case APP_PTT_EVENT:
		{
			uint8_t ubAPP_PttFlag = ptEventMsg->ubAPP_Message[1];
			ADO_PttFuncPtr_t tAPP_PttFunc[] = {ADO_PTTStop, ADO_PTTStart};

			if(tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr)
				tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr();

			OSD_IMG_INFO tOsdImgInfo;
			if(ubDisplaymodeFlag == 1)
			{
				if(ubFactoryModeFLag == 0)
				{
					if(tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr == ADO_PTTStart)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TALKBACK, 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart = 259;
						tOsdImgInfo.uwYStart = 515;	
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					}
					else
					{
						tOsdImgInfo.uwHSize  = 250;
						tOsdImgInfo.uwVSize  = 250;
						tOsdImgInfo.uwXStart = 259;
						tOsdImgInfo.uwYStart = 515;
						OSD_EraserImg2(&tOsdImgInfo);
					}
				}
			}		
			break;
		}
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LostLinkStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef VBM_PU
		case APP_UNBIND_BU_EVENT:
			APP_doUnbindBU(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_PairingStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_PAIRING_STOP_EVENT:
			PAIR_Stop();
		case APP_PAIRING_FAIL_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_PAIRSTS_RPT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = rFAIL;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			printd(DBG_Debug1Lvl, (APP_PAIRING_STOP_EVENT == ptEventMsg->ubAPP_Event)?"Pairing_stop\r\n":"Pairing_fail\r\n");
			break;
		case APP_PAIRING_SUCCESS_EVENT:
		{
		#ifdef VBM_PU
			UI_CamNum_t tAdoSrcCamNum;
		#endif
			tAPP_StsReport.tAPP_ReportType = APP_PAIRSTS_RPT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = rSUCCESS;
		#ifdef VBM_PU
			APP_KNLRoleMap2CamNum(tAPP_KNLInfo.tAdoSrcRole, tAdoSrcCamNum);
			tAPP_StsReport.ubAPP_Report[1] = tAdoSrcCamNum;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			if((UI_CamNum_t)tAPP_StsReport.ubAPP_Report[1] != tAdoSrcCamNum)
			{
				tAdoSrcCamNum = (UI_CamNum_t)tAPP_StsReport.ubAPP_Report[1];
				tAPP_KNLInfo.tAdoSrcRole = APP_GetSTANumMappingTable(tAdoSrcCamNum)->tKNL_StaNum;
			}
			tAPP_KNLInfo.tBURoleInfo[tAPP_PairRoleInfo.tPairBURole].tKNL_DispLoc = tAPP_PairRoleInfo.tPairBUDispLoc;
			VDO_DisplayLocationSetup(tAPP_PairRoleInfo.tPairBURole, tAPP_PairRoleInfo.tPairBUDispLoc);
			VDO_SetPlayRole(tAPP_PairRoleInfo.tPairBURole); //设置当前匹配的摄像头.20180525
			VDO_UpdateDisplayParameter();
			if(APP_UNBIND_BU_EVENT == ptEventMsg->ubAPP_Message[2])
			{
				UI_CamNum_t tDelCam;

				tAPP_StsReport.tAPP_ReportType = APP_PAIRUDBU_PRT;
				tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
				APP_PairTagMap2CamNum((PAIR_TAG)ptEventMsg->ubAPP_Message[1], tDelCam);
				tAPP_StsReport.ubAPP_Report[0] = tDelCam;
				UI_UpdateAppStatus(&tAPP_StsReport);
				tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			}
		#endif
		#ifdef VBM_BU
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			tAPP_KNLInfo.tKNL_Role   = tAPP_STANumTable[PAIR_GetStaNumber()].tKNL_StaNum;
			tAPP_KNLInfo.tAdoSrcRole = tAPP_KNLInfo.tKNL_Role;
			VDO_KNLSysInfoSetup(tAPP_KNLInfo.tKNL_Role);
			ADO_KNLSysInfoSetup(tAPP_KNLInfo.tKNL_Role);
		#endif
			APP_UpdateKNLSetupInfo();
			break;
		}
		default:
			return;
	}
#ifdef VBM_PU
	KNL_ResetLcdChannel();
#endif
	VDO_Start();
	ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
}
//------------------------------------------------------------------------------
void APP_doPairingStart(void *pvPairInfo)
{
#ifdef VBM_PU
	uint8_t *pAPP_PairInfo 				= (uint8_t *)pvPairInfo;
	PAIR_TAG tPair_Tag 					= tAPP_STANumTable[pAPP_PairInfo[1]].tPAIR_StaNum;
	tAPP_PairRoleInfo.tPairBURole		= tAPP_STANumTable[pAPP_PairInfo[1]].tKNL_StaNum;
	tAPP_PairRoleInfo.tPairBUDispLoc	= tAPP_DispLocMap[pAPP_PairInfo[2]].tKNL_DispLocation;
#endif
#ifdef VBM_BU
	PAIR_TAG tPair_Tag = PAIR_AP_ASSIGN;
#endif
	VDO_Stop();
	ADO_Stop();
	PAIR_Start(tPair_Tag, APP_PAIRING_TIMEOUT);
}
//------------------------------------------------------------------------------
#ifdef VBM_PU
void APP_doUnbindBU(APP_EventMsg_t *ptEventMsg)
{
	PAIR_TAG tPair_Tag = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tPAIR_StaNum;
	KNL_ROLE tKNL_Role = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tKNL_StaNum;

	if((tPair_Tag > PAIR_STA4) || (tKNL_Role > KNL_STA4))
		return;
	PAIR_DeleteTxId(tPair_Tag);
	VDO_RemoveDataPath(tKNL_Role);
	ADO_RemoveDataPath(tKNL_Role);
	tAPP_KNLInfo.tBURoleInfo[tKNL_Role].tKNL_DispLoc = KNL_DISP_LOCATION_ERR;
	tAPP_KNLInfo.tAdoSrcRole = (tAPP_KNLInfo.tAdoSrcRole == tKNL_Role)?KNL_NONE:tAPP_KNLInfo.tAdoSrcRole;
	APP_UpdateKNLSetupInfo();
}
#endif
//------------------------------------------------------------------------------
uint8_t APP_UpdateLinkStatus(void)
{
#ifdef VBM_PU
	uint8_t ubAPP_Event = APP_LOSTLINK_EVENT;
	KNL_ROLE ubKNL_RoleNum;

	for(ubKNL_RoleNum = KNL_STA1; ubKNL_RoleNum <= KNL_STA4; ubKNL_RoleNum++)
	{
		tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum] 	 = rLOSTLINK;
		tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+4] = 0;
		tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+8] = 0;
		if(ubKNL_GetCommLinkStatus(ubKNL_RoleNum) == BB_LINK)
		{
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum]   = rLINK;
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+4] = KNL_GetPerValue(ubKNL_RoleNum);
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+8] = KNL_GetRssiValue(ubKNL_RoleNum);
			if(UI_GetCamViewPoolID() == ubKNL_RoleNum)
			{
				//printd(Apk_DebugLvl, "### LinkStatus ubKNL_RoleNum: %d, UI_GetCamViewPoolID(): %d.\n", ubKNL_RoleNum, UI_GetCamViewPoolID());
				ubAPP_Event = APP_LINK_EVENT; //当前是无信号状态,匹配成功以后状态未更新,一直无法进入LINK状态.20180519
			}
		}
	}
	if(ubAPP_Event == APP_LOSTLINK_EVENT)
		ubLinkonceflag = 0;

	if((ubAPP_Event == APP_LINK_EVENT)&&(ubLinkonceflag == 0))
	{
		if(LCD_JPEG_DISABLE == tLCD_GetJpegDecoderStatus())
		{
			UI_PowerOnSet();
			ubLinkonceflag = 1;
		}
	}
	
	return ubAPP_Event;
#endif
#ifdef VBM_BU
	return (ubKNL_GetCommLinkStatus(KNL_MASTER_AP) == BB_LINK)?APP_LINK_EVENT:APP_LOSTLINK_EVENT;
#endif
}
//------------------------------------------------------------------------------
APP_StaNumMap_t *APP_GetSTANumMappingTable(UI_CamNum_t tCamNum)
{
	return &tAPP_STANumTable[tCamNum];
}
//------------------------------------------------------------------------------
void APP_LoadKNLSetupInfo(void)
{
	uint32_t ulAPP_KNLInfoSFAddr = pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);

	tAPP_KNLInfo.tAdoSrcRole = KNL_NONE;
	osMutexWait(APP_UpdateMutex, osWaitForever);
	SF_Read(ulAPP_KNLInfoSFAddr, sizeof(APP_KNLInfo_t), (uint8_t *)&tAPP_KNLInfo);
	osMutexRelease(APP_UpdateMutex);
	printd(DBG_InfoLvl, "KNL TAG:%s\n",tAPP_KNLInfo.cbKNL_InfoTag);
	printd(DBG_InfoLvl, "KNL VER:%s\n",tAPP_KNLInfo.cbKNL_FwVersion);
#ifdef OP_AP
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag, SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) != 0)
	|| (strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) != 0)) {
#else 
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) != 0)
	|| (strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) != 0)) {
#endif
		printd(DBG_ErrorLvl, "TAG no match, Reset KNL\n");
		tAPP_KNLInfo.tUsbdClassMode = USBD_MSC_MODE;
		tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
		return;
	}
	if(tAPP_KNLInfo.tUsbdClassMode >= USBD_UNKNOWN_MODE)
		tAPP_KNLInfo.tUsbdClassMode = USBD_MSC_MODE;
	if(tAPP_KNLInfo.tTuningMode > APP_TUNINGMODE_ON)
		tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
#if ((!USBD_ENABLE) || (defined(VBM_PU)))
	tAPP_KNLInfo.tUsbdClassMode = USBD_MSC_MODE;
	tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
#endif
}
//------------------------------------------------------------------------------
void APP_UpdateKNLSetupInfo(void)
{
	uint32_t ulAPP_KNLInfoSFAddr;

	osMutexWait(APP_UpdateMutex, osWaitForever);
	ulAPP_KNLInfoSFAddr 		= pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);
#ifdef OP_AP
	memcpy(tAPP_KNLInfo.cbKNL_InfoTag, SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1);
#else
	memcpy(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1);
#endif
	memcpy(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1);
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulAPP_KNLInfoSFAddr, pSF_Info->ulSecSize);
	SF_Write(ulAPP_KNLInfoSFAddr, sizeof(APP_KNLInfo_t), (uint8_t *)&tAPP_KNLInfo);	
	SF_EnableWrProtect();
	osMutexRelease(APP_UpdateMutex);
}
//------------------------------------------------------------------------------
void APP_KNLParamSetup(void)
{
#ifdef VBM_PU
	KNL_ROLE tKNL_BURole;
	UI_CamNum_t tCamNum;

	KNL_SetRole((tAPP_KNLInfo.tKNL_Role = KNL_MASTER_AP));
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag, SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) == 0)
	&& (strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) == 0)) {
		for(tCamNum = CAM1; tCamNum < DISPLAY_MODE; tCamNum++)
		{
			tKNL_BURole = tAPP_STANumTable[tCamNum].tKNL_StaNum;
			VDO_DisplayLocationSetup(tKNL_BURole, tAPP_KNLInfo.tBURoleInfo[tKNL_BURole].tKNL_DispLoc);
		}
		if(tAPP_KNLInfo.tAdoSrcRole > KNL_STA4)
			tAPP_KNLInfo.tAdoSrcRole = KNL_STA1;
	}
#endif
#ifdef VBM_BU
	if(USBD_UVC_MODE == tAPP_KNLInfo.tUsbdClassMode)
	{
		APP_TuningFuncPtr_t tAPP_TuningFunc[] = {[APP_TUNINGMODE_OFF] = KNL_TurnOffTuningTool,
												 [APP_TUNINGMODE_ON]  = KNL_TurnOnTuningTool};
		tAPP_TuningFunc[tAPP_KNLInfo.tTuningMode].pvAPP_TuningFunc();
	}
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) == 0)
	&& (strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) == 0)) {
		tAPP_KNLInfo.tKNL_Role   = (tAPP_KNLInfo.tKNL_Role <= KNL_STA4)?tAPP_KNLInfo.tKNL_Role:KNL_NONE;
	} else {
		tAPP_KNLInfo.tKNL_Role   = KNL_NONE;
	}
	tAPP_KNLInfo.tAdoSrcRole = tAPP_KNLInfo.tKNL_Role;
	KNL_SetRole(tAPP_KNLInfo.tKNL_Role);
#endif
	tAPP_KNLInfo.tKNL_OpMode = (DISPLAY_MODE ==	DISPLAY_4T1R)?KNL_OPMODE_VBM_4T:
	                           (DISPLAY_MODE ==	DISPLAY_2T1R)?KNL_OPMODE_VBM_2T:
							   (DISPLAY_MODE ==	DISPLAY_1T1R)?KNL_OPMODE_VBM_1T:KNL_OPMODE_VBM_4T;
	KNL_SetOpMode(tAPP_KNLInfo.tKNL_OpMode);
}
//------------------------------------------------------------------------------
void APP_FWUgradeSetup(void)
{
	FWU_MODE_t tAPP_FwuMode = FWU_USBDMSC;
	char *pFW_Ver = SN937XX_FW_VERSION, *p, *q;
	FWU_MSCParam_t tAPP_FWUParam = {{0},{0},{0}};

	strncpy(tAPP_FWUParam.cVolumeLable, SN937XX_VOLUME_LABLE, sizeof(tAPP_FWUParam.cVolumeLable));
	for(p=pFW_Ver; (q=strchr(p, '.'))!=NULL; p=q+1)
		p = q;
	strncpy(tAPP_FWUParam.cFileName, pFW_Ver, (p - pFW_Ver - 1));
	strncpy(tAPP_FWUParam.cFileNameExt, p, (strlen(pFW_Ver) - (p - pFW_Ver)));
#if !USBD_ENABLE
	if(FWU_USBDMSC == tAPP_FwuMode)
		tAPP_FwuMode = FWU_DISABLE;
#endif
	FWU_Setup(tAPP_FwuMode, &tAPP_FWUParam);
	FWU_Enable();
}
//------------------------------------------------------------------------------
#ifdef VBM_PU
void APP_SwitchViewTypeExec(APP_EventMsg_t *ptEventMsg)
{
	KNL_ROLE tKNL_Role[3];
	KNL_DISP_TYPE tKNL_DispType;
	UI_CamViewType_t tAPP_CamView;
	uint8_t i;

	tAPP_CamView  = (UI_CamViewType_t)ptEventMsg->ubAPP_Message[1];
	tKNL_DispType = ((tAPP_CamView == SINGLE_VIEW) || (tAPP_CamView == SCAN_VIEW))?KNL_DISP_SINGLE:
					 (tAPP_CamView == DUAL_VIEW)?KNL_DISP_DUAL_C:KNL_DISP_QUAD;
	for(i = 0; i < 2; i++)
		tKNL_Role[i] = tAPP_STANumTable[ptEventMsg->ubAPP_Message[2+i]].tKNL_StaNum;
	tAPP_StsReport.tAPP_ReportType = APP_VWMODESTS_RPT;
	tAPP_StsReport.ubAPP_Report[0] = tAPP_CamView;
	UI_UpdateAppStatus(&tAPP_StsReport);
	tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;

	if ((tAPP_CamView == SINGLE_VIEW) || (tAPP_CamView == SCAN_VIEW)) {
		if(tAPP_KNLInfo.tAdoSrcRole != tAPP_STANumTable[tKNL_Role[0]].tKNL_StaNum)
			APP_UpdateKNLSetupInfo();
		tAPP_KNLInfo.tAdoSrcRole = tAPP_STANumTable[tKNL_Role[0]].tKNL_StaNum;
		ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
	}
	VDO_SwitchDisplayType(tKNL_DispType, tKNL_Role);
}
//------------------------------------------------------------------------------
void APP_LcdDisplayOff(void)
{
	LCD_Suspend();
	LCD_Stop();
	GLB->LCD_FUNC_DIS  = 1;
	LCD_PWR_DISABLE;
}
//------------------------------------------------------------------------------
void APP_LcdDisplayOn(void)
{
	SSP->SSP_GPIO_MODE = 0; //0:Normal SSP Mode 
	osDelay(20);			//???
	LCD_PWR_ENABLE;
	osDelay(200);

	GLB->LCD_FUNC_DIS  = 0;
	LCD_Init(LCD_LCD_PANEL);
	LCD_SetGammaLevel(4);
	KNL_VdoDisplayParamUpdate();
	LCD_Start();
	LCDBL_ENABLE(UI_ENABLE);
}
#endif
//------------------------------------------------------------------------------
void APP_PowerSaveExec(APP_EventMsg_t *ptEventMsg)
{
	UI_PowerSaveMode_t tAPP_PsMode = (UI_PowerSaveMode_t)ptEventMsg->ubAPP_Message[1];
#ifdef VBM_PU
	UI_CamNum_t tPsCamNum          = (UI_CamNum_t)ptEventMsg->ubAPP_Message[3];
	KNL_ROLE tKNL_Role;
#endif
	switch(tAPP_PsMode)
	{
		case PS_VOX_MODE:
		{
			VDO_PsFuncPtr_t tAPP_VoxFunc[] = {VDO_Start, VDO_Stop};
		#ifdef VBM_PU
			APP_ActFuncPtr_t tAPP_LcdFunc[] = {APP_LcdDisplayOn, APP_LcdDisplayOff};	//! {LCD_Resume, LCD_Suspend};
			SYS_PowerState_t tAPP_PsState[]	= {SYS_PS0, SYS_PS1};
		#endif
			uint8_t ubAPP_PsFlag = ptEventMsg->ubAPP_Message[2];

			if(tAPP_VoxFunc[ubAPP_PsFlag].VDO_tPsFunPtr)
				tAPP_VoxFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
		#ifdef VBM_PU
			SYS_SetPowerStates(tAPP_PsState[ubAPP_PsFlag]);
			//SIGNAL_LED_IO_ENABLE = (!ubAPP_PsFlag)?TRUE:FALSE; 
			tAPP_StsReport.tAPP_ReportType = APP_VOXMODESTS_RPT;
			tAPP_StsReport.ubAPP_Report[0] = ubAPP_PsFlag;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			if(tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr)
				tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr();
		#endif
			break;
		}
		case PS_ECO_MODE:
		#ifdef VBM_BU
			VDO_ChangePlayState(tAPP_KNLInfo.tKNL_Role, VDO_STOP);
			KNL_EnableWORFunc();
		#endif
		#ifdef VBM_PU
		{
			VDO_PlayState_t tAPP_VdoPlySte = (TRUE == ptEventMsg->ubAPP_Message[4])?VDO_START:VDO_STOP;

			tKNL_Role = tAPP_STANumTable[tPsCamNum].tKNL_StaNum;
			VDO_ChangePlayState(tKNL_Role, tAPP_VdoPlySte);
			KNL_WakeupDevice(tKNL_Role, ptEventMsg->ubAPP_Message[2]);
		}
		#endif
			break;
		case PS_WOR_MODE:
		#ifdef VBM_BU
		{
			VDO_PsFuncPtr_t tAPP_WorFunc[] = {VDO_Stop, VDO_Start};
			uint8_t ubAPP_PsFlag = ptEventMsg->ubAPP_Message[2];
			uint8_t ubAPP_VdoActFlag = ptEventMsg->ubAPP_Message[4];

			if((TRUE == ubAPP_VdoActFlag) &&
			   (tAPP_WorFunc[ubAPP_PsFlag].VDO_tPsFunPtr))
				tAPP_WorFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
			KNL_WakeupDevice(KNL_MASTER_AP, ptEventMsg->ubAPP_Message[3]);
		}
		#endif
		#ifdef VBM_PU
			KNL_EnableWORFunc();
		#endif
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_SetTuningToolMode(APP_TuningMode_t tTuningMode)
{
	tAPP_KNLInfo.tTuningMode = tTuningMode;
	switch(tTuningMode)
	{
		case APP_TUNINGMODE_ON:
			tAPP_KNLInfo.tUsbdClassMode = USBD_UVC_MODE;
			KNL_TurnOnTuningTool();
			break;
		case APP_TUNINGMODE_OFF:
			tAPP_KNLInfo.tUsbdClassMode = USBD_MSC_MODE;
			KNL_TurnOffTuningTool();
			break;
		default:
			return;
	}
	APP_UpdateKNLSetupInfo();
}
//------------------------------------------------------------------------------
APP_TuningMode_t APP_GetTuningToolMode(void)
{
	return tAPP_KNLInfo.tTuningMode;
}
//------------------------------------------------------------------------------
void APP_Start(void)
{
	//! Kernel Setup
	KNL_BlockInit();
	
	//! Video Start
	#ifdef VBM_BU	
	VDO_Start();
	#endif
	
	#ifdef VBM_PU	
	if(ubFactorySettingFLag == 0)	
		VDO_Start();
	else
		VDO_Stop();
	#endif
	
	//! Audio Start
	ADO_Start(tAPP_KNLInfo.tAdoSrcRole);

	//! Two way command Start
	TWC_Start();

	tAPP_StsReport.tAPP_State = APP_IDLE_STATE;
	UI_UpdateAppStatus(&tAPP_StsReport);
#ifdef VBM_PU
	UI_SwitchCameraSource();
#endif
}
//------------------------------------------------------------------------------
