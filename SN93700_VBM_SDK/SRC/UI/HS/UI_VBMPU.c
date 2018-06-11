/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_VBMPU.c
	\brief		User Interface of VBM Parent Unit (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.7
	\date		2017/11/30
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include "UI_VBMPU.h"
#include "SF_API.h"
#include "EN_API.h"
#include "TIMER.h"
#include "LCD.h"
#include "Buzzer.h"
#include "FWU_API.h"
#include "VDO.h"
#include "SADC.h"
#include "WDT.h"


#define osUI_SIGNALS	0x66

#define UI_TEST_MODE	0

/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
UI_KeyEventMap_t UiKeyEventMap[] =
{
	{NULL,				0,			NULL,						NULL},
	{AKEY_MENU, 		20,			UI_MenuKey, 				NULL},
	{AKEY_MENU, 		0,			UI_MenuKey,					NULL},
	{AKEY_UP, 			0,			UI_UpArrowKey,				NULL},
	{AKEY_DOWN, 		0,			UI_DownArrowKey,			NULL},
	{AKEY_DOWN, 		50,			NULL,						NULL},
	{AKEY_LEFT, 		0,			UI_LeftArrowKey,			NULL},
	{AKEY_LEFT, 		30,			NULL,						NULL},
	{AKEY_RIGHT, 		0,			UI_RightArrowKey,			NULL},
	{AKEY_RIGHT, 		30,			NULL,						NULL},
	{AKEY_ENTER, 		0,			UI_EnterKey,				NULL},
	{AKEY_ENTER, 		20,			NULL, 						NULL},
	{AKEY_PS,			0,			NULL,						NULL},
	{AKEY_PS,			20,			NULL,						NULL},
	{AKEY_PTT,			0,			UI_PushTalkKey,				NULL},
	{PKEY_ID0,			0,			UI_PowerKeyShort,			NULL},
	{PKEY_ID0, 			20,			UI_PowerKey,				NULL},
	{GKEY_ID0,			0,			UI_VolUpKey,			NULL},
	{GKEY_ID1, 			0,			UI_VolDownKey,				NULL},	
};
static UI_State_t tUI_State;
static APP_State_t tUI_SyncAppState;
static UI_BUStatus_t tUI_CamStatus[CAM_4T];
static UI_PUSetting_t tUI_PuSetting;
static UI_MenuFuncPtr_t tUI_StateMap2MenuFunc[UI_STATE_MAX] =
{
	[UI_DISPLAY_STATE]			= UI_DisplayArrowKeyFunc,
	[UI_MAINMENU_STATE] 		= UI_Menu,
	[UI_SUBMENU_STATE]  		= UI_SubMenu,
	[UI_SUBSUBMENU_STATE]  		= UI_SubSubMenu,
	[UI_SUBSUBSUBMENU_STATE]  	= UI_SubSubSubMenu,
	[UI_CAM_SEL_STATE]			= UI_CameraSelection,
	[UI_SET_VDOMODE_STATE]		= UI_ChangeVideoMode,
	[UI_SET_ADOSRC_STATE]		= UI_ChangeAudioSource,
	[UI_SET_PUPSMODE_STATE] 	= UI_PuPowerSaveModeSelection,
	[UI_SET_BUECOMODE_STATE] 	= UI_BuPowerSaveModeSelection,
	[UI_CAMSETTINGMENU_STATE]	= UI_CameraSettingMenu,
	[UI_SET_CAMCOLOR_STATE] 	= UI_CameraColorSetting,
	[UI_DPTZ_CONTROL_STATE]		= UI_DPTZ_Control,
	[UI_MD_WINDOW_STATE]		= UI_MD_Window,
	[UI_PAIRING_STATE]			= UI_PairingControl,
	[UI_DUALVIEW_CAMSEL_STATE]  = UI_CameraSelection4DualView,
	[UI_ENGMODE_STATE]			= UI_EngModeCtrl,
};
static UI_MenuItem_t tUI_MenuItem;
static UI_SubMenuItem_t tUI_SubMenuItem[MENUITEM_MAX] =
{
	{ADJUST_BR_ITEM,	BRIGHTNESS_MAX	},
	{PAIRCAM_ITEM,		PAIRITEM_MAX	},
	{RECSELCAM_ITEM,	RECITEM_MAX		},
	{PHOTOSELCAM_ITEM, 	PHOTOITEM_MAX	},
	{NULL},
	{CAMSSELCAM_ITEM,	CAMSITEM_MAX	},
	{NIGHTMODE_ITEM, 	SETTINGITEM_MAX	},
};
UI_ReportFuncPtr_t tUiReportMap2Func[] =
{
	[UI_UPDATE_BUSTS] 			= UI_UpdateBuStatus,
	[UI_VOX_TRIG]				= UI_VoxTrigger,
	[UI_MD_TRIG]				= UI_MDTrigger,
	[UI_VOICE_TRIG]				= UI_VoiceTrigger,
	[UI_VOICE_CHECK]			= UI_GetVolData,
	[UI_TEMP_CHECK]				= UI_GetTempData,
};

//ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n36DB, R2R_VOL_n23p5DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};
//ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};
//ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n39p1DB, R2R_VOL_n36DB, R2R_VOL_n29p8DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};
ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n39p1DB, R2R_VOL_n29p8DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};

//uint32_t ulUI_BLTable[] = {0x100, 0x118, 0x200, 0x400, 0x600, 0x700, 0x800, 0x0900, 0xA00};
uint32_t ulUI_BLTable[] = {0, 4, 16, 28, 40, 52, 64, 76, 88};


static UI_SubMenuCamNum_t tCamSelect;
static UI_CamViewSelect_t tCamViewSel;
static UI_CamViewSelect_t tCamPreViewSel;
static UI_PairingInfo_t tPairInfo;
static UI_ThreadNotify_t tosUI_Notify;
static UI_DPTZParam_t tUI_DptzParam;
static UI_CamNum_t tUI_BuEcoCamNum;
static UI_CamNum_t tUI_CamNumSel;
static uint8_t ubUI_PttStartFlag;
static uint8_t ubUI_ResetPeriodFlag;
static uint8_t ubUI_FastStateFlag;
static uint8_t ubUI_ShowTimeFlag;
static uint8_t ubUI_ScanStartFlag;
static uint32_t ulUI_MonitorPsFlag[CAM_4T];
static uint8_t ubUI_DualViewExFlag;
static uint8_t ubUI_StopUpdateStsBarFlag;
static uint32_t ulUI_LogoIndex;
uint8_t *pUI_BuConnectFlag[CAM_STSMAX];
osMutexId UI_PUMutex;

uint8_t ubViewTypeFlags;
uint8_t ubClearOsdFlag = 0;
uint8_t ubClearOsdCnt= 0;

//uint8_t ubCurrentMenuItemIdx = 0;
//uint8_t ubNextSubMenuItemIdx = 0;
uint8_t ubSubMenuItemFlag = 0;
uint8_t ubSubMenuItemPreFlag = 0;

uint8_t brightness_temp =1;	
uint8_t ubAutoBrightnessFlag =0;	
uint8_t ubAutoBrightnessValue = 4;	
uint8_t ubSleepModeFlag = 1;	
uint8_t ubZoomFlag = 0;
uint8_t ubLangageFlag = 0;
uint8_t ubFlickerFlag = 0;
uint8_t ubDefualtFlag = 0;
uint8_t ubTempunitFlag = 0;
uint8_t ubNightmodeFlag[4] = {0,0,0,0};

uint8_t ubTimeHour = 6;
uint8_t ubTimeMin = 22;
uint8_t ubTimeAMPM = 0;

uint8_t ubPairDisplayCnt = 0;
uint8_t ubPairDisplayTime = 60;

uint8_t ubSubSubMenuItemFlag = 0;
uint8_t ubSubSubMenuItemPreFlag = 0;

uint8_t ubSubSubMenuRealItem = 0;

uint8_t ubAlarmSubSubItemNum[5] = {4,4,3,5,3};
uint8_t ubHighTempC[5] = {0,24,27,30,32};
uint8_t ubHighTempF[5] = {0,75,80,85,90};
uint8_t ubLowTempC[5] = {0,13,16,18,21};
uint8_t ubLowTempF[5] = {0,55,60,65,70};
uint8_t ubAlertTime[4] = {5,10,20,30};
uint8_t ubAlarmIconFlag = 0;

uint8_t ubSubSubSubMenuItemFlag = 0;
uint8_t ubSubSubSubMenuItemPreFlag = 0;

uint8_t ubSubSubSubMenuRealItem = 0;

uint8_t ubClearMenuFlag = 0;
uint8_t ubClearMenuCnt = 0;
uint8_t ubClearMenuCntMax = 50;

uint8_t ubFactorySettingFlag = 0;
uint8_t ubFS_MenuItem = 0;
uint8_t ubFS_Timeitem = 0;

uint8_t ubGetVoiceTemp = 0;

uint8_t ubCamPairFlag[4] = {0,0,0,0};
uint8_t ubPairSelCam = 0;
uint8_t ubCamFullFlag = 0;
uint8_t ubNoAddCamFlag = 0;
//uint16_t ubScanTime = 5000;

uint8_t ubEnterTimeMenuFlag = 0;
uint8_t ubDisplaymodeFlag = 0;

uint8_t ubGetTempData = 25;
uint8_t ubRealTemp = 25;
uint8_t ubHighAlarmOn = 0;
uint8_t ubLowAlarmOn = 0;
uint8_t ubHighAlarmTriggerFlag = 0;
uint8_t ubLowAlarmTriggerFlag = 0;
uint16_t ubAlarmIdleCnt = 0;
uint8_t ubTempAlarmcheck = 0;
uint8_t ubAlarmClearFlag = 0;
uint8_t ubAlarmDisplayFlag = 0;
uint8_t ubAlertCnt = 0;

uint8_t ubAlarmMaxSoundOn = 0;
uint8_t ubAlarmSoundPickupOn = 0;
uint8_t ubAlarmSoundShowFlag = 0;
uint16_t ubAlarmSoundIdleCnt = 0;
uint8_t ubAlarmSoundDisplay = 0;
uint8_t ubAlarmSoundTriggerFlag = 0;

uint8_t ubDelCamitem = 0;
uint8_t ubCameraScanTime[5] = {0, 5, 10, 15, 30};

//20180409
uint8_t ubPlayAlarmCount = 0;
uint8_t ubTempAlarmState = TEMP_ALARM_IDLE;
uint16_t ubTempAlarmTriggerCount = 0;

uint8_t ubPickupAlarmState = PICKUP_ALARM_IDLE;
uint16_t ubPickupAlarmTriggerCount = 0;

uint8_t ubDisplayPTN = 0;
uint8_t ubMotor0State = MC_LEFT_RIGHT_OFF;
uint8_t ubMotor1State = MC_UP_DOWN_OFF;
uint8_t ubMC0OnCount = 0;
uint8_t ubMC1OnCount = 0;
uint8_t ubPairOK_SwitchCam = 0;
uint8_t ubSetViewCam = 0;

uint8_t ubGetBatValue = 0;
extern uint8_t ubStartUpState;

uint8_t ubFactorySettingFLag ;
uint8_t ubClearOsdFlag_2 =0;
uint8_t ubFactoryModeFLag ;

uint8_t ubGetIR1Temp = 0;
uint8_t ubGetIR2Temp = 0;

//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;
	OSD_IMG_INFO tOsdImgInfo;

	KEY_Event_t *ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);

	if(ubStartUpState)
		return;

	if(tUI_PuSetting.ubDefualtFlag == TRUE) //恢复出厂设置只有上下左右,Enter,PowerKey键有用
	{
		if((ptKeyEvent->ubKeyID < AKEY_UP) || (ptKeyEvent->ubKeyID > AKEY_ENTER))
		{
			if(ptKeyEvent->ubKeyID != PKEY_ID0)
				return;
		}
	}

	if(UI_AutoLcdResetSleepTime(ptKeyEvent->ubKeyAction) == 1) //20180319
		return;
	
	if(ubFactoryModeFLag == 1)
	{
		if(ptKeyEvent->ubKeyAction == KEY_DOWN_ACT)
		{
			UI_FactorymodeKeyDisplay(ptKeyEvent->ubKeyID);
		}		
		if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
		{
			tOsdImgInfo.uwHSize  = 76;
			tOsdImgInfo.uwVSize  = 300;
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
		}
	}
	
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((UI_DISPLAY_STATE == tUI_State) &&
		   ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
		    (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
		{
			if(UI_GetAlarmStatus())
				return;
			
			//UI_DPTZ_KeyRelease(ptKeyEvent->ubKeyID);
			UI_MotorControl((ptKeyEvent->ubKeyID > AKEY_DOWN)?1:0);
			return;
		}
		if(((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt)) ||
		   (ptKeyEvent->ubKeyID == AKEY_PTT))
		{
			if(ptKeyEvent->ubKeyID == AKEY_PTT)
				ubUI_PttStartFlag = FALSE;
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
			{
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr(); //按键弹起
			}
		}
		ubUI_KeyEventIdx = 0;
		return;
	}
	
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		
		if((UI_DISPLAY_STATE == tUI_State) &&
		   ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
		    (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
		{
			if(UI_DPTZ_KeyPress(ptKeyEvent->ubKeyID, uwIdx) == rUI_FAIL)
				continue;
			
			if(UiKeyEventMap[uwIdx].KeyEventFuncPtr)
			{
				if(ptKeyEvent->ubKeyAction == KEY_DOWN_ACT)
				{
					UiKeyEventMap[uwIdx].KeyEventFuncPtr(); //短按
				}
			}	
			ubUI_KeyEventIdx = 0;
			break;
		}
		
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{
			ubUI_KeyEventIdx = uwIdx;
			if(((ptKeyEvent->uwKeyCnt) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)) ||
			   (ptKeyEvent->ubKeyID == AKEY_PTT))
			{
				if(ptKeyEvent->ubKeyID == AKEY_PTT)
					ubUI_PttStartFlag = TRUE;
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr(); //长按
				ubUI_KeyEventIdx = (ptKeyEvent->ubKeyID == AKEY_PTT)?ubUI_KeyEventIdx:0;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_OnInitDialog(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i = 0;

	OSD_LogoJpeg(OSDLOGO_BOOT);
	//OSD_LogoJpeg(OSDLOGO_LOGOFINISH);
	
	for(i = 0; i < 27; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO1+i, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 234;
		tOsdImgInfo.uwYStart = 418;
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}	

	/*
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO62, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);	
	*/
	//OSD_LogoJpeg(OSDLOGO_LOGOFINISH);
	
	GPIO->GPIO_O1 	= 0;
	GPIO->GPIO_O0 	= 0;
	//GPIO->GPIO_O13 	= 0;
	BUZ_PlayPowerOnSound();

	ubStartUpState = 0;
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osMutexDef(UI_PUMutex);
	UI_PUMutex 				  = osMutexCreate(osMutex(UI_PUMutex));
	tosUI_Notify.thread_id	  = NULL;
	tosUI_Notify.iSignals	  = 0;
	ubUI_ResetPeriodFlag	  = FALSE;
	ubUI_FastStateFlag		  = FALSE;
	ubUI_ShowTimeFlag		  = FALSE;
	ubUI_PttStartFlag		  = TRUE;
	ubUI_ScanStartFlag    	  = FALSE;
	ubUI_DualViewExFlag		  = FALSE;
	ubUI_StopUpdateStsBarFlag = FALSE;
	tUI_State 		 		  = UI_DISPLAY_STATE;
	tUI_SyncAppState		  = APP_STATE_NULL;
	ulUI_LogoIndex			  = OSDLOGO_BOOT;

	printf("UI_StateReset###\n");
	if (wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) == RTC_WATCHDOG_CHK_TAG) {
		printd(DBG_ErrorLvl, "Watch Dog Rst\n");
		tCamViewSel.tCamViewType  = (UI_CamViewType_t)wRTC_ReadUserRam(RTC_RECORD_VIEW_MODE_ADDR);
		tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)(wRTC_ReadUserRam(RTC_RECORD_VIEW_CAM_ADDR) >> 4);
		tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)(wRTC_ReadUserRam(RTC_RECORD_VIEW_CAM_ADDR) & 0x0f);
		if (tCamViewSel.tCamViewType == QUAL_VIEW) {
			KNL_SetDispType(KNL_DISP_QUAD); 
		} else if (tCamViewSel.tCamViewType == DUAL_VIEW) {
			KNL_SetDispType(KNL_DISP_DUAL_C); 
		} else {
			KNL_SetDispType(KNL_DISP_SINGLE); 
		}
	} else {
		tCamViewSel.tCamViewType  = (VDO_DISP_TYPE == KNL_DISP_QUAD)?QUAL_VIEW:
									(VDO_DISP_TYPE == KNL_DISP_DUAL_C)?DUAL_VIEW:
									(VDO_DISP_SCAN == TRUE)?SCAN_VIEW:SINGLE_VIEW;
		tCamViewSel.tCamViewPool[0] = (VDO_DISP_TYPE == KNL_DISP_QUAD)?CAM_4T:CAM1;
		tCamViewSel.tCamViewPool[1] = (VDO_DISP_TYPE == KNL_DISP_DUAL_C)?CAM2:NO_CAM;
		KNL_SetDispType(VDO_DISP_TYPE); 
	}
	tUI_MenuItem.ubItemIdx 	  = BRIGHT_ITEM;
	tUI_MenuItem.ubItemPreIdx = BRIGHT_ITEM;

	if (tCamViewSel.tCamViewType == SCAN_VIEW) {
		UI_EnableScanMode();
	} else {
		UI_DisableScanMode();
	}
	UI_ResetSubMenuInfo();
	UI_ResetSubSubMenuInfo();
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvBUResponse, UI_RecvBURequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
	UI_LoadDevStatusInfo();
	ubSetViewCam = tCamViewSel.tCamViewPool[0];
	
	if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
	{
		printd(DBG_ErrorLvl, "Calendar base setting fail!\n");
		//return;
	}
	if ((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) != RTC_PWRSTS_KEEP_TAG)
	&& (wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) != RTC_WATCHDOG_CHK_TAG)) {
		RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
	}
	
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_WATCHDOG_CHK_TAG);
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	static uint8_t ubUI_PuStartUpFlag = FALSE;

	osMutexWait(UI_PUMutex, osWaitForever);
	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result  = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];
			UI_CamNum_t tAppAdoSrcNum = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[1];

			UI_ReportPairingResult(tPair_Result);
			if((rUI_SUCCESS == tPair_Result) &&
			   (tUI_PuSetting.tAdoSrcCamNum != tAppAdoSrcNum))
				pAppStsRpt->ubAPP_Report[1] = tUI_PuSetting.tAdoSrcCamNum;
			break;
		}
		case APP_LINKSTS_RPT:
			UI_ReportBuConnectionStatus(pAppStsRpt->ubAPP_Report);
			break;
		case APP_VWMODESTS_RPT:
			//tUI_PuSetting.ubScanModeEn = pAppStsRpt->ubAPP_Report[0];
			if (pAppStsRpt->ubAPP_Report[0] != SCAN_VIEW)
			{
				UI_DisableScanMode();
			}
			break;
		case APP_VOXMODESTS_RPT:
			ubUI_StopUpdateStsBarFlag = pAppStsRpt->ubAPP_Report[0];
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			break;
		default:
			break;
	}
	if(pAppStsRpt->tAPP_State == APP_LINK_STATE)
	{
		if(tUI_SyncAppState != pAppStsRpt->tAPP_State)
			UI_RemoveLostLinkLogo();
		ubUI_ResetPeriodFlag = TRUE;
		if(FALSE == ubUI_PuStartUpFlag)
		{
			if(PS_VOX_MODE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode)
				UI_EnableVox();
			ubUI_PuStartUpFlag = TRUE;
		}
		/*
		if((tCamViewSel.tCamViewType == SCAN_VIEW) && (FALSE == ubUI_ScanStartFlag))
			UI_EnableScanMode();
		*/
		if((FALSE == ubUI_ScanStartFlag) && (tUI_PuSetting.ubScanTime > 0))
			UI_EnableScanMode();
	}
	tUI_PuSetting.IconSts.ubClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	osMutexRelease(UI_PUMutex);
}

//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};
	uint8_t ubUI_SendMsg2AppFlag = FALSE;
	OSD_IMG_INFO tOsdImgInfo;
	
	osMutexWait(UI_PUMutex, osWaitForever);
	UI_CLEAR_THREADCNT(tUI_PuSetting.IconSts.ubClearThdCntFlag, *pThreadCnt);

	//printf("UI_UpdateStatus tUI_SyncAppState: %d.\n", tUI_SyncAppState);
	printf("UI_UpdateStatus GPIO->GPIO_I0: %d, GPIO->GPIO_I11: %d.\n", GPIO->GPIO_I0, GPIO->GPIO_I11);
	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
	switch(tUI_SyncAppState)
	{
		case APP_IDLE_STATE:
			ubUI_SendMsg2AppFlag = ((tUI_State == UI_DISPLAY_STATE)||(tUI_State == UI_SUBSUBMENU_STATE))?TRUE:FALSE;
			break;
		case APP_LOSTLINK_STATE:
			UI_DisableScanMode();
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{
				UI_ShowLostLinkLogo(pThreadCnt);
				(*pThreadCnt)++;
				ubUI_SendMsg2AppFlag = TRUE;
				goto END_UPDATESTS;
			}
			else
			{
				if((ubFactorySettingFlag == 0)&&tUI_State == UI_DISPLAY_STATE)
				{
					tOsdImgInfo.uwHSize  = 1280;
					tOsdImgInfo.uwVSize  = 720;
					tOsdImgInfo.uwXStart = 48;
					tOsdImgInfo.uwYStart = 0;
					OSD_EraserImg1(&tOsdImgInfo);
					
					OSD_LogoJpeg(OSDLOGO_BOOT);
					/*
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 0;
					tOsdImgInfo.uwYStart =0;	
					tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
					*/
						
					UI_FS_LangageMenuDisplay(ubLangageFlag);

					ubFS_MenuItem = 0;		
					
					ubFactorySettingFlag = 1;					
				}
				ubUI_SendMsg2AppFlag = TRUE;
				goto END_UPDATESTS;				
			}
			break;
		case APP_LINK_STATE:
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{				
				//if(((*pThreadCnt)%5) == 0)
				{
					UI_TempAlarmCheck();
					UI_PickupAlarmCheck();
				}
				
				//if(((*pThreadCnt)%2) == 0)
				{
					UI_MotorStateCheck();
				}
			
				UI_RedrawStatusBar(pThreadCnt);
				(*pThreadCnt)++;
				ubUI_SendMsg2AppFlag = TRUE;
				goto END_UPDATESTS;
			}
			else
			{
				if((ubFactorySettingFlag == 0)&&tUI_State == UI_DISPLAY_STATE)
				{
					tOsdImgInfo.uwHSize  = 1280;
					tOsdImgInfo.uwVSize  = 720;
					tOsdImgInfo.uwXStart = 48;
					tOsdImgInfo.uwYStart = 0;
					OSD_EraserImg1(&tOsdImgInfo);
					
					
					OSD_LogoJpeg(OSDLOGO_BOOT);
					/*
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 0;
					tOsdImgInfo.uwYStart =0;	
					tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);	
					*/
						
					UI_FS_LangageMenuDisplay(ubLangageFlag);

					ubFS_MenuItem = 0;		
					
					ubFactorySettingFlag = 1;					
				}
				ubUI_SendMsg2AppFlag = TRUE;
				goto END_UPDATESTS;
			}
			break;
		case APP_PAIRING_STATE:
			UI_DrawPairingStatusIcon();
			osMutexRelease(UI_PUMutex);
			return;
		default:
			break;
	}
	*pThreadCnt 	    					= 0;
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
END_UPDATESTS:
	//UI_UpdateBriLvlIcon();
	UI_UpdateVolLvlIcon();
	if(((*pThreadCnt)%10) == 0)
	{
		UI_GetBatLevel();
	}
	tUI_PuSetting.IconSts.ubRdPairIconFlag 	= FALSE;
	if(ubUI_SendMsg2AppFlag == TRUE)
	{
		tUI_GetLinkStsMsg.ubAPP_Event 		= APP_LINKSTATUS_REPORT_EVENT;
		UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	}
	osMutexRelease(UI_PUMutex);
}
//------------------------------------------------------------------------------
extern uint8_t ubStartUpState;
void UI_PuInit(void)
{
	ubStartUpState = 0;

	UI_Zoom_SetScaleParam(tUI_PuSetting.ubZoomScale);
	UI_AutoLcdSetSleepTime(tUI_PuSetting.ubSleepMode);
	if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
		ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
	else
    	ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
	
	LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_PuSetting.BriLvL.tBL_UpdateLvL]);

	if((TRUE == tUI_PuSetting.ubScanModeEn) && (tUI_PuSetting.ubDefualtFlag == FALSE))
	{
		UI_EnableScanMode();
	}

	GPIO->GPIO_O12 = 0;
	//GPIO->GPIO_O13 = 1;
	
	printf("UI_PuInit ok.\n");
}
//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
		case GKEY_EVENT:			
			osMutexWait(UI_PUMutex, osWaitForever);
			UI_KeyEventExec(ptEventPtr->pvEvent);
			osMutexRelease(UI_PUMutex);
			break;
		case SCANMODE_EVENT:
			UI_ScanModeExec();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PowerKeyShort(void) //20180319
{
	if(ubFactoryModeFLag == 1)return;

	if(PWM->PWM_EN8 == 0)
	{
		LCDBL_ENABLE(UI_ENABLE);
	}
	else
	{
		LCDBL_ENABLE(UI_DISABLE);
	}
	printf("UI_PowerKeyShort###\n");
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	printf("UI_PowerKey###\n");
	if(ubStartUpState)
		return;
	
	BUZ_PlayPowerOffSound();
	UI_UpdateDevStatusInfo(); //20180321
	osDelay(600);			//wait buzzer play finish
	LCDBL_ENABLE(UI_DISABLE);
	//POWER_LED_IO  = 0;
	//SIGNAL_LED_IO = 0;
	RTC_WriteUserRam(RECORD_PWRSTS_ADDR, PWRSTS_KEEP);
	printf("UI_PowerKey Power OFF!\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------
void UI_MenuKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
			OSD_IMG_INFO tOsdInfo;
			
			if(ubFactoryModeFLag == 1)
				return;

			if(UI_CheckStopAlarm() == 1)
					return;
			
			tOsdInfo.uwHSize  = 672;
			tOsdInfo.uwVSize  = 1280;
			tOsdInfo.uwXStart = 48;
			tOsdInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdInfo);

			if(ubAutoBrightnessFlag == 0)
			{
				ubSubMenuItemFlag = 0;
				ubSubMenuItemPreFlag = 1;
			}
			else
			{
				ubSubMenuItemFlag = 1;
				ubSubMenuItemPreFlag = 0;
			}
			brightness_temp = tUI_PuSetting.BriLvL.tBL_UpdateLvL;
			
			tUI_State = UI_MAINMENU_STATE;
			tUI_MenuItem.ubItemPreIdx = BRIGHT_ITEM;
			tUI_MenuItem.ubItemIdx 	  = BRIGHT_ITEM;
			UI_DrawMenuPage();
			break;
		case UI_MAINMENU_STATE:
		{
			UI_UpdateDevStatusInfo();	
			OSD_IMG_INFO tOsdInfo;

			tOsdInfo.uwHSize  = 672;
			tOsdInfo.uwVSize  = 1280;
			tOsdInfo.uwXStart = 48;
			tOsdInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdInfo);
			
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
			if(APP_LOSTLINK_STATE == tUI_SyncAppState)
			{
				tLCD_JpegDecodeDisable();
				OSD_LogoJpeg(OSDLOGO_LOSTLINK);

				if(ubNoAddCamFlag == 1)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (23*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
					tOsdInfo.uwXStart = 170;
					tOsdInfo.uwYStart = 432;	
					tOSD_Img2(&tOsdInfo, OSD_QUEUE);	

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (23*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
					tOsdInfo.uwXStart = 502;
					tOsdInfo.uwYStart = 360;	
					tOSD_Img2(&tOsdInfo, OSD_UPDATE);
					printf("UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL1 + (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdInfo);
					tOsdInfo.uwXStart= 160;
					tOsdInfo.uwYStart =328;	
					tOSD_Img2(&tOsdInfo, OSD_QUEUE);	

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL2+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdInfo);
					tOsdInfo.uwXStart= 480;
					tOsdInfo.uwYStart =304-104;	
					tOSD_Img2(&tOsdInfo, OSD_UPDATE);
					printf("UI_MenuKey OSD2IMG_MENU_NOSIGNAL1.\n");
				}
			}
			tUI_State = UI_DISPLAY_STATE;
			break;
		}

/*		
		case UI_SUBMENU_STATE:
		{
			tUI_State = UI_MAINMENU_STATE;
			UI_ResetSubMenuInfo();
			if(TRUE == ubUI_FastStateFlag)
			{
				OSD_IMG_INFO tOsdInfo;

				ubUI_FastStateFlag = FALSE;
				UI_ClearBuConnectStatusFlag();
				tOsdInfo.uwHSize  = uwOSD_GetHSize();
				tOsdInfo.uwVSize  = uwOSD_GetVSize();
				tOsdInfo.uwXStart = 0;
				tOsdInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdInfo);
				tUI_State = UI_DISPLAY_STATE;
				break;
			}
			UI_DrawMenuPage();
			break;
		}
*/	
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:			
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_ENGMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(EXIT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_UpArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_ENGMODE_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(UP_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DownArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_ENGMODE_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(DOWN_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_LeftArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(LEFT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RightArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(RIGHT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_EnterKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_PUPSMODE_STATE:
		case UI_SET_BUECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_ENGMODE_STATE:			
		case UI_PAIRING_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(ENTER_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu1Key(void)
{
	if(tUI_State != UI_DISPLAY_STATE)
	{
		UI_MenuKey();
		return;
	}
	if(APP_LOSTLINK_STATE == tUI_SyncAppState)
		return;
	if(DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
	{
		UI_CameraSelectionKey();
		return;
	}
	UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu2Key(void)
{
	if((APP_LOSTLINK_STATE == tUI_SyncAppState) || (SINGLE_VIEW != tCamViewSel.tCamViewType))
		return;

	UI_DrawCameraSettingMenu(UI_CAMFUNC_SETUP);
}
//------------------------------------------------------------------------------
void UI_ShowColorSettingValue(uint8_t ubValue)
{
	ubValue =ubValue;
}
//------------------------------------------------------------------------------
void UI_DrawColorSettingMenu(void)
{
	
}
//------------------------------------------------------------------------------
void UI_DrawMDSettingScreen(void)
{

}
void UI_MD_Window(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_CameraColorSetting(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_DrawRecordSubMenuPage(void)
{

}
//------------------------------------------------------------------------------
void UI_DrawPhotoSubMenuPage(void)
{

}
//------------------------------------------------------------------------------
void UI_DrawPowerSaveSubMenuPage(void)
{

}


//------------------------------------------------------------------------------
void UI_DrawCameraSettingMenu(UI_CameraSettingMenu_t tCamSetMenu)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(tCamSetMenu)
	{
		case UI_CAMISP_SETUP:
			tUI_State = UI_SUBMENU_STATE;
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			tOsdImgInfo.uwHSize  = 100;
			tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
			OSD_EraserImg1(&tOsdImgInfo);
			OSD_Weight(OSD_WEIGHT_7DIV8);
			tUI_MenuItem.ubItemIdx = BRIGHT_ITEM;
			UI_DrawSubMenuPage(BRIGHT_ITEM);
			ubUI_FastStateFlag = TRUE;
			break;
		case UI_CAMFUNC_SETUP:
		{
			OSD_IMG_INFO tCamSetOsdImgInfo[6];

			tUI_State = UI_CAMSETTINGMENU_STATE;
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSCOLORMENUNOR_ITEM, 6, &tCamSetOsdImgInfo[0]) != OSD_OK)
			{
				printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
				return;
			}
			tOSD_Img2(&tCamSetOsdImgInfo[1], OSD_QUEUE);
			tOSD_Img2(&tCamSetOsdImgInfo[2], OSD_QUEUE);
			tOSD_Img2(&tCamSetOsdImgInfo[4], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_SUBMENUICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			OSD_Weight(OSD_WEIGHT_8DIV8);
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu(UI_ArrowKey_t tArrowKey)
{
	static UI_CameraSettingItem_t tUI_CamSetItem = UI_COLOR_ITEM;
	UI_CameraSettingItem_t tUI_PrevCamSetItem = UI_DPTZ_ITEM;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwOsdImgIdx[3] = {OSD2IMG_CAMSCOLORMENUNOR_ITEM, OSD2IMG_CAMSDPTZMENUNOR_ITEM, OSD2IMG_CAMSMDV2MENUNOR_ITEM};

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_COLOR_ITEM == tUI_CamSetItem)
				return;
			tUI_PrevCamSetItem = tUI_CamSetItem;
			--tUI_CamSetItem;
			break;
		case RIGHT_ARROW:
			if(UI_MD_ITEM == tUI_CamSetItem)
				return;
			tUI_PrevCamSetItem = tUI_CamSetItem;
			++tUI_CamSetItem;
			break;
		case ENTER_ARROW:
		case EXIT_ARROW:
			if(ENTER_ARROW == tArrowKey)
			{
				if(UI_DPTZ_ITEM == tUI_CamSetItem)
				{
					uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
					uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
					uint8_t ubArrowNum;

					tUI_DptzParam.tScaleParam							 = UI_SCALEUP_2X;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize/tUI_DptzParam.tScaleParam;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize/tUI_DptzParam.tScaleParam;
					tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize	   	 = ulLcd_HSize;
					tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize	   	 = ulLcd_VSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
					tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
					/*
					for(ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_DPTZUPARROWNOR_ICON+(ubArrowNum*2)), 1, &tOsdImgInfo);
						tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					}
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DPTZZOOMMSG_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					*/
					tUI_CamSetItem = UI_COLOR_ITEM;
					tUI_State = UI_DPTZ_CONTROL_STATE;
					return;
				}
			}
			tUI_CamSetItem = UI_COLOR_ITEM;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
}

//------------------------------------------------------------------------------
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwUI_ArrowOsdImgIdx[] = {[AKEY_UP]    = OSD2IMG_DPTZUPARROWHL_ICON,
							          [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWHL_ICON,
									  [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWHL_ICON,
		                              [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWHL_ICON};

	if(ubKeyID == UiKeyEventMap[ubKeyMapIdx].ubKeyID)
	{
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
		//tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		return rUI_SUCCESS;
	}
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_DPTZ_KeyRelease(uint8_t ubKeyID)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwUI_ArrowOsdImgIdx[] = {[AKEY_UP]    = OSD2IMG_DPTZUPARROWNOR_ICON,
							          [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWNOR_ICON,
									  [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWNOR_ICON,
		                              [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWNOR_ICON};

	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey)
{
#define PT_STEP		10
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubArrowNum;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart == 0)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart -= PT_STEP;
			break;
		case DOWN_ARROW:
			if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart + 
			    tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart += PT_STEP;
			break;
		case LEFT_ARROW:
			if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + 
			    tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart += PT_STEP;
			break;
		case RIGHT_ARROW:
			if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart == 0)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart -= PT_STEP;
			break;
		case ENTER_ARROW:
			if(tUI_DptzParam.tScaleParam == UI_SCALEUP_2X)
			{
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize /= UI_SCALEUP_2X;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize /= UI_SCALEUP_2X;
				tUI_DptzParam.tScaleParam = UI_SCALEUP_4X;
			}
			else if(tUI_DptzParam.tScaleParam == UI_SCALEUP_4X)
			{
				uint16_t uwCropHsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize*UI_SCALEUP_2X;
				uint16_t uwCropVsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize*UI_SCALEUP_2X;
				uint16_t uwPrevCropHsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize;
				uint16_t uwPrevCropVsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize;
				if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart < ((uwCropHsize - uwPrevCropHsize)/2))
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
				if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + ((uwCropVsize - uwPrevCropVsize)/2)) >= uwCropVsize)
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = uwCropVsize;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize = uwCropHsize;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize = uwCropVsize;
				tUI_DptzParam.tScaleParam = UI_SCALEUP_2X;
			}
			break;
		case EXIT_ARROW:
		{
			UI_ClearStatusBarOsdIcon();
			for(ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
			{
				if(ubArrowNum < 2)
				{
					tOsdImgInfo.uwXStart = 300;
					tOsdImgInfo.uwYStart = 40 + (ubArrowNum * 1080);
				}
				else
				{
					tOsdImgInfo.uwXStart = 40 + ((ubArrowNum - 2) * 570);
					tOsdImgInfo.uwYStart = 590;
				}
				tOsdImgInfo.uwHSize  = 100;
				tOsdImgInfo.uwVSize  = 110;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 20;
			tOsdImgInfo.uwHSize  = 50;
			tOsdImgInfo.uwVSize  = 125;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_DptzParam.tScaleParam						   = UI_SCALEUP_2X;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = 0;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize  = uwLCD_GetLcdHoSize();
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize  = uwLCD_GetLcdVoSize();
			tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
			tUI_State = UI_DISPLAY_STATE;
			return;
		}
		default:
			return;
	}
	tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_CameraSelectionKey(void)
{
#define QUAL_TYPE_ITEM	6
#define SCAN_TYPE_ITEM	5
#define DUAL_TYPE_ITEM	4
	OSD_IMG_INFO tOsdImgInfo[(OSD2IMG_SELCAM2TDISABLE_ICON-OSD2IMG_SELCAM1ONLINE_ICON)+1] = {0};
	uint16_t uwDisplayImgIdx = 0, uwStartIdx;
	uint16_t uwXOffset  = 0;
	uint16_t uwYOffset  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?230:0;
	uint8_t ubSelItem   = tCamViewSel.tCamViewPool[0];
	UI_CamNum_t tCamNum;
	static uint8_t ubUI_UpdateCamSelFlag = FALSE;
	
	if(tUI_State != UI_DISPLAY_STATE)
	{
		UI_MenuKey();
		return;
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAM1ONLINE_ICON, (OSD2IMG_SELCAM2TDISABLE_ICON-OSD2IMG_SELCAM1ONLINE_ICON)+1, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(FALSE == ubUI_UpdateCamSelFlag)
	{
		tUI_CamNumSel = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(UI_CamNum_t)QUAL_TYPE_ITEM:(UI_CamNum_t)tUI_PuSetting.ubTotalBuNum;
		ubUI_UpdateCamSelFlag = TRUE;
	}
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		uwDisplayImgIdx = (((tCamNum == CAM4)?(tCamNum+1):tCamNum)*2) +
		                  (((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) || (tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE))?1:0);
		tOsdImgInfo[uwDisplayImgIdx].uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	}
	if (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) {
		OSD_IMG_INFO tOsdImg4TInfo[4];

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SEL4TDUALONLINE_ICON, 4, &tOsdImg4TInfo[0]);
		uwStartIdx = OSD2IMG_SELCAM4TENABLE_ICON;
		uwDisplayImgIdx = (uwStartIdx - OSD2IMG_SELCAM1ONLINE_ICON);
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		tOSD_Img2(&tOsdImg4TInfo[((tUI_PuSetting.ubPairedBuNum > 1)?0:1)], OSD_QUEUE);
		tOSD_Img2(&tOsdImg4TInfo[((tUI_PuSetting.ubPairedBuNum > 1)?2:3)], OSD_QUEUE);
		ubSelItem = (tCamViewSel.tCamViewType == QUAL_VIEW)?QUAL_TYPE_ITEM:
		            (tCamViewSel.tCamViewType == DUAL_VIEW)?DUAL_TYPE_ITEM:
					(tCamViewSel.tCamViewType == SCAN_VIEW)?SCAN_TYPE_ITEM:tCamViewSel.tCamViewPool[0];
		tUI_CamNumSel = (UI_CamNum_t)ubSelItem;
	} else if(DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) {
		uwStartIdx = OSD2IMG_SELCAM2TENABLE_ICON;
		uwDisplayImgIdx = (uwStartIdx - OSD2IMG_SELCAM1ONLINE_ICON);
		tOsdImgInfo[uwDisplayImgIdx].uwYStart -= 80;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		ubSelItem = (tCamViewSel.tCamViewType == DUAL_VIEW)?CAM_2T:tCamViewSel.tCamViewPool[0];
		tUI_CamNumSel = (UI_CamNum_t)ubSelItem;
	} else {
		ubSelItem	= tCamViewSel.tCamViewPool[0];
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo[0]);
	tOsdImgInfo[0].uwXStart += uwXOffset;
	tOsdImgInfo[0].uwYStart -= ((ubSelItem*111) + uwYOffset);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	tUI_State = UI_CAM_SEL_STATE;
}
//------------------------------------------------------------------------------
void UI_CameraSelection(UI_ArrowKey_t tArrowKey)
{
	UI_CamNum_t tPreCamNum = tUI_CamNumSel;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwXOffset  = 0;
	uint16_t uwYOffset  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?230:0;

	if(DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum)
		return;

	tPreCamNum = tUI_CamNumSel;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			tUI_CamNumSel = UI_ChangeSelectCamNum4UiMenu(&tPreCamNum, &tArrowKey);
			if(tUI_CamNumSel == NO_CAM)
			{
				tUI_CamNumSel = tPreCamNum;
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXOffset;
			tOsdImgInfo.uwYStart -= ((tPreCamNum*111) + uwYOffset);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXOffset;
			tOsdImgInfo.uwYStart -= ((tUI_CamNumSel*111) + uwYOffset);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case DOWN_ARROW:
			if ((SINGLE_VIEW == tCamViewSel.tCamViewType) || (SCAN_VIEW == tCamViewSel.tCamViewType))
			{
				UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			}
			else
			{
				tOsdImgInfo.uwXStart  = 100;
				tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?400:0;
				tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?300:255;
				tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?500:uwLCD_GetLcdVoSize();
				OSD_EraserImg2(&tOsdImgInfo);
				UI_ChangeAudioSourceKey();
			}
			break;
		case ENTER_ARROW:
		{
			UI_Result_t tUI_ChkResult = rUI_SUCCESS;

			if((tUI_CamNumSel == SCAN_TYPE_ITEM) && (tCamViewSel.tCamViewType == SCAN_VIEW))			   
				goto EXIT_CAMSELECT_MENU;
			tCamViewSel.tCamViewPool[0]    = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == DUAL_TYPE_ITEM)?tCamViewSel.tCamViewPool[0]:tUI_CamNumSel:
											 (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == CAM_2T)?CAM1:tUI_CamNumSel:
											 CAM1;
			tCamViewSel.tCamViewPool[1]    = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel != DUAL_TYPE_ITEM)?NO_CAM:tCamViewSel.tCamViewPool[1]:
										     (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == CAM_2T)?CAM2:NO_CAM:
											 NO_CAM;
			tCamViewSel.tCamViewType   = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == QUAL_TYPE_ITEM)?QUAL_VIEW:
																					  (tUI_CamNumSel == DUAL_TYPE_ITEM)?tCamPreViewSel.tCamViewType:
																					  (tUI_CamNumSel == SCAN_TYPE_ITEM)?SCAN_VIEW:SINGLE_VIEW:
									     (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?(tUI_CamNumSel == CAM_2T)?DUAL_VIEW:SINGLE_VIEW:
										 SINGLE_VIEW;
			//tUI_PuSetting.ubScanModeEn = (tUI_CamNumSel == SCAN_TYPE_ITEM)?TRUE:FALSE;
			//tUI_PuSetting.ubDualModeEn = (tUI_CamNumSel == DUAL_TYPE_ITEM)?TRUE:FALSE;
			if(tCamViewSel.tCamViewType == SCAN_VIEW)
			{
				tCamViewSel.tCamViewPool[0]  = CAM1;
				//tCamViewSel.tCamViewType = SCAN_VIEW;
				tUI_ChkResult = UI_CheckCameraSource4SV();
			}
			if (tUI_CamNumSel == DUAL_TYPE_ITEM)
			{
				UI_CameraSelection4DualView(ENTER_ARROW);
				break;
			}
			else
				ubUI_DualViewExFlag = FALSE;
			if(rUI_SUCCESS == tUI_ChkResult)
				UI_SwitchCameraSource();
		}
		case EXIT_ARROW:
EXIT_CAMSELECT_MENU:
			if(ENTER_ARROW == tArrowKey)
			{
				tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
				UI_ClearStatusBarOsdIcon();
			}
			UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = 0;
			tOsdImgInfo.uwHSize   = 255;
			tOsdImgInfo.uwVSize   = uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_DISPLAY_STATE;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_ChangeAudioSourceKey(void)
{
	if((APP_LOSTLINK_STATE == tUI_SyncAppState) || (tUI_State != UI_CAM_SEL_STATE) ||
	   (DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum))
		return;

	tUI_State = UI_SET_ADOSRC_STATE;
}
//------------------------------------------------------------------------------
void UI_ChangeAudioSource(UI_ArrowKey_t tArrowKey)
{
	static UI_CamNum_t tAdoCamNumSel;
	static uint8_t ubUI_UpdateAdoCamSelFlag = FALSE;
//	UI_CamNum_t tPreAdoCamNum = tAdoCamNumSel;

	if(FALSE == ubUI_UpdateAdoCamSelFlag)
	{
		tAdoCamNumSel = tUI_PuSetting.tAdoSrcCamNum;
		ubUI_UpdateAdoCamSelFlag = TRUE;
	}
//	tPreAdoCamNum = tAdoCamNumSel;
	switch(tArrowKey)
	{
		case DOWN_ARROW:
			if(SINGLE_VIEW != tCamViewSel.tCamViewType)
				UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			return;
		case LEFT_ARROW:
			if(tAdoCamNumSel == CAM1)
				return;
			--tAdoCamNumSel;
			break;
		case RIGHT_ARROW:
			if((tAdoCamNumSel + 1) >= tUI_PuSetting.ubTotalBuNum)
				return;
			++tAdoCamNumSel;
			break;
		case ENTER_ARROW:
		{
			APP_EventMsg_t tUI_SwitchAdoSrcMsg = {0};

			tUI_PuSetting.tAdoSrcCamNum			 = tAdoCamNumSel;
			tUI_SwitchAdoSrcMsg.ubAPP_Event 	 = APP_ADOSRCSEL_EVENT;
			tUI_SwitchAdoSrcMsg.ubAPP_Message[0] = 1;		//! Message Length
			tUI_SwitchAdoSrcMsg.ubAPP_Message[1] = tUI_PuSetting.tAdoSrcCamNum;
			UI_UpdateDevStatusInfo();
			UI_SendMessageToAPP(&tUI_SwitchAdoSrcMsg);
		}
		case EXIT_ARROW:
			if(ENTER_ARROW == tArrowKey)
				UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();
			ubUI_UpdateAdoCamSelFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
}
//------------------------------------------------------------------------------
#define UI_MENUICON_NUM		7
#define UI_WRICON_OFFSET	2
//------------------------------------------------------------------------------
void UI_ChangeVideoMode(UI_ArrowKey_t tArrowKey)
{
	static UI_CamVdoMode_t tCamVdoMode = RECLOOP_MODE;
//	UI_CamVdoMode_t tPreCamVdoMode = RECLOOP_MODE;
	static uint8_t ubUI_UpdateVdoModeFlag = FALSE;
	//uint16_t uwVdoModeOsdImg[VDOMODE_MAX] = {OSD2IMG_RECLOOPMODENOR_ICON, 	 OSD2IMG_RECMANUMODENOR_ICON,
       //                                        OSD2IMG_PHOTOSHOOTMODENOR_ICON, OSD2IMG_RECPHOTOMODENOR_ICON};
//	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_UpdateVdoModeFlag)
	{
		tCamVdoMode = tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamVdoMode;
		ubUI_UpdateVdoModeFlag = TRUE;
	}
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tCamVdoMode == RECLOOP_MODE)
				return;
//			tPreCamVdoMode = tCamVdoMode;
//			tCamVdoMode--;
			break;
		case RIGHT_ARROW:
			if(tCamVdoMode == RECPHOTO_MODE)
				return;
//			tPreCamVdoMode = tCamVdoMode;
//			tCamVdoMode++;
			break;
		case ENTER_ARROW:
			tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamVdoMode = tCamVdoMode;
		case EXIT_ARROW:
			//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECPHOTOMODEHL_ICON, 1, &tOsdImgInfo);
			//tOsdImgInfo.uwHSize = 150;
			//tOsdImgInfo.uwVSize = 650;
			//OSD_EraserImg2(&tOsdImgInfo);
			ubUI_UpdateVdoModeFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			return;
		default:
			return;
	}
	//tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwVdoModeOsdImg[tPreCamVdoMode]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	//OSD_EraserImg2(&tOsdImgInfo);
	//UI_DrawHLandNormalIcon(uwVdoModeOsdImg[tPreCamVdoMode], (uwVdoModeOsdImg[tCamVdoMode]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_PuPowerSaveKey(void)
{
//	OSD_IMG_INFO tOsdImgInfo[4];

	if((APP_LOSTLINK_STATE == tUI_SyncAppState) || (tUI_State != UI_DISPLAY_STATE) ||
	   (DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum))
		return;
	/*
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOXOPT_ICON, 4, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
	*/
	tUI_State = UI_SET_PUPSMODE_STATE;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupBuEcoMode(UI_CamNum_t tECO_CamNum)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0]  = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1]  = PS_ECO_MODE;
	if((POWER_NORMAL_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode) &&
	   (FALSE == ulUI_MonitorPsFlag[tECO_CamNum]))
	{
		UI_PUReqCmd_t tPsCmd;

		tPsCmd.tDS_CamNum 				= tECO_CamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_ECOMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_ECO_MODE;
		tPsCmd.ubCmd_Len  				= 3;		
		if(UI_SendRequestToBU(osThreadGetId(), &tPsCmd) == rUI_SUCCESS)
		{
			tUI_CamStatus[tECO_CamNum].tCamPsMode = PS_ECO_MODE;
			UI_UpdateDevStatusInfo();
			tUI_PsMessage.ubAPP_Message[2]  = FALSE;
			tUI_PsMessage.ubAPP_Message[3]  = tECO_CamNum;
			tUI_PsMessage.ubAPP_Message[4]  = FALSE;
			UI_SendMessageToAPP(&tUI_PsMessage);
		}
		else
		{
			printd(DBG_ErrorLvl, "ECO Setting Fail !\n");
			return rUI_FAIL;
		}
	}
	else if(PS_ECO_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode)
	{
		tUI_PsMessage.ubAPP_Message[2]  = TRUE;
		tUI_PsMessage.ubAPP_Message[3]  = tECO_CamNum;
		tUI_PsMessage.ubAPP_Message[4]  = TRUE;
		UI_SendMessageToAPP(&tUI_PsMessage);
	}
	
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
void UI_BuPowerSaveKey(void)
{
	UI_CamNum_t tCamNum;

	if((tCamViewSel.tCamViewPool[0] <= CAM4) &&
	   (PS_VOX_MODE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode))
	{
		UI_DisableVox();
		return;
	}

	if((tUI_State != UI_DISPLAY_STATE) ||
	   (TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag))
		return;

	if((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
	   (SINGLE_VIEW == tCamViewSel.tCamViewType))
	{
		UI_SetupBuEcoMode(tCamViewSel.tCamViewPool[0]);
		return;
	}
	tUI_BuEcoCamNum = NO_CAM;
	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) ||
		   ((tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE) && (PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)))
		{
			
		}
		else
		{
			if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			{
			}
			tUI_BuEcoCamNum = (NO_CAM == tUI_BuEcoCamNum)?tCamNum:tUI_BuEcoCamNum;
		}
	}

	tUI_State = UI_SET_BUECOMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_PuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	static UI_PowerSaveMode_t tUI_PsMode = PS_VOX_MODE;	
//	UI_PowerSaveMode_t tUI_PrePsMode = PS_WOR_MODE;	
	UI_PUReqCmd_t tPsCmd;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(PS_VOX_MODE == tUI_PsMode)
				return;
			tUI_PsMode    = PS_VOX_MODE;
//			tUI_PrePsMode = PS_WOR_MODE;
			break;
		case RIGHT_ARROW:
			if(PS_WOR_MODE == tUI_PsMode)
				return;
			tUI_PsMode    = PS_WOR_MODE;
			//tUI_PrePsMode = PS_VOX_MODE;
			break;
		case ENTER_ARROW:
			if(PS_VOX_MODE == tUI_PsMode)
			{
				tPsCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
				tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
				tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_VOX_MODE;
				tPsCmd.ubCmd_Len  				= 3;
				if(UI_SendRequestToBU(osThreadGetId(), &tPsCmd) != rUI_SUCCESS)
				{
					printd(DBG_ErrorLvl, "VOX Notify Fail !\n");
					return;
				}
				UI_EnableVox();
			}
		case EXIT_ARROW:
			UI_ClearBuConnectStatusFlag();
			tOsdImgInfo.uwXStart = 153;
			tOsdImgInfo.uwYStart = 380;
			tOsdImgInfo.uwHSize  = 210;
			tOsdImgInfo.uwVSize  = 420;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_PsMode = PS_VOX_MODE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
}
//------------------------------------------------------------------------------
void UI_BuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	UI_CamNum_t tEcoPrevCamNum = tUI_BuEcoCamNum;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			tUI_BuEcoCamNum = UI_ChangeSelectCamNum4UiMenu(&tEcoPrevCamNum, &tArrowKey);
			if(tUI_BuEcoCamNum >= tUI_PuSetting.ubTotalBuNum)
			{
				tUI_BuEcoCamNum = tEcoPrevCamNum;
				return;
			}
			break;
		case ENTER_ARROW:
			if(UI_SetupBuEcoMode(tUI_BuEcoCamNum) == rUI_FAIL)
				return;
		case EXIT_ARROW:
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
				UI_ClearBuConnectStatusFlag();
			if(ENTER_ARROW == tArrowKey)
				osDelay(800);
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
}
//------------------------------------------------------------------------------
void UI_PushTalkKey(void)
{
	OSD_IMG_INFO tOsdInfo;

	if(ubFactoryModeFLag == 1)return;

	printf("UI_PushTalkKey#####\n");
	if(((ubTempAlarmState > TEMP_ALARM_IDLE)&&(ubTempAlarmState < TEMP_ALARM_OFF)) ||  
		((ubPickupAlarmState > PICKUP_ALARM_IDLE)&&(ubPickupAlarmState < PICKUP_ALARM_OFF)) || 
		(tUI_SyncAppState != APP_LINK_STATE))
	{
		ubDisplaymodeFlag = 0;
		return;
	}

	tOsdInfo.uwHSize  = 672;
	tOsdInfo.uwVSize  = 1280;
	tOsdInfo.uwXStart = 48;
	tOsdInfo.uwYStart = 0;
	OSD_EraserImg2(&tOsdInfo);
	
	ubDisplaymodeFlag = 1;
	tUI_State = UI_DISPLAY_STATE;
	
	APP_EventMsg_t tUI_PttMessage = {0};

	tUI_PttMessage.ubAPP_Event 	 	= APP_PTT_EVENT;
	tUI_PttMessage.ubAPP_Message[0] = 1;		//! Message Length
	tUI_PttMessage.ubAPP_Message[1] = ubUI_PttStartFlag;
	UI_SendMessageToAPP(&tUI_PttMessage);
	SPEAKER_EN(((TRUE == ubUI_PttStartFlag)?UI_DISABLE:UI_ENABLE));
}
//------------------------------------------------------------------------------
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
//	uint16_t uwOsdImgIdx = 0;
	uint8_t *pT_Num[3] = {(uint8_t *)(&ubTimeHour),
						(uint8_t *)(&ubTimeMin), (uint8_t *)(&ubTimeAMPM)
						};
	uint8_t ubT_MaxNum[3] = {12, 59, 1};
	uint8_t ubT_MinNum[3] = {1,  0,  0};	
	UI_CamNum_t tSelCam;
	UI_PUReqCmd_t tPsCmd;

	if(ubFactoryModeFLag == 1)
		return;

	if((tArrowKey == ENTER_ARROW)&&(tUI_PuSetting.ubDefualtFlag == FALSE))
	{
		if(TRUE == tUI_PuSetting.ubScanModeEn)
			return;
				
		tSelCam = tCamViewSel.tCamViewPool[0] ;

		if(tSelCam < (ubPairSelCam - 1))
		{
			tSelCam = tSelCam + 1;

		}
		else if(tSelCam ==(ubPairSelCam - 1))
		{
			tSelCam = 0;
		}
				
					
		if((tUI_CamStatus[tSelCam].ulCAM_ID != INVALID_ID) &&
			 //(tUI_CamStatus[tSelCam].tCamConnSts == CAM_ONLINE) &&
			 (tSelCam != tCamViewSel.tCamViewPool[0]))
		{
							
			tCamViewSel.tCamViewType	= SINGLE_VIEW;
			tCamViewSel.tCamViewPool[0] 	= tSelCam;
			tUI_PuSetting.tAdoSrcCamNum = tSelCam;
			UI_SwitchCameraSource();
			UI_ClearBuConnectStatusFlag();

			ubSetViewCam = tCamViewSel.tCamViewPool[0];
						
			UI_UpdateDevStatusInfo();
		}
							
	}
	
	if(tUI_PuSetting.ubDefualtFlag == FALSE)
	{
		if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
			return;
	}

	if((tArrowKey >= UP_ARROW) && (tArrowKey <= RIGHT_ARROW) && (UI_GetAlarmStatus()))
		return;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		{
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{
				/*
				tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_PTZ_SETTING;
				tPsCmd.ubCmd[UI_SETTING_DATA]   = PTZ_UP;
				tPsCmd.ubCmd_Len  				= 3;		
				if(UI_SendRequestToBU(osThreadGetId(), &tPsCmd) == rUI_SUCCESS)
				{

				}
				*/

				#if 0 //test
				UI_TestCmd(1, 0);
				#endif
				
				UI_MotorControl(MC_LEFT_ON);
				break;
			}
			else
			{
				if(ubFS_MenuItem == 1)
				{
					if(ubFS_Timeitem >=1)
					{
						ubFS_Timeitem  -=  1;
					}	
					else
					{
						ubFS_Timeitem = 0 ;
					}		
					UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
				}	
				else
				{
						
				}	
				break;
			}
		}
		case RIGHT_ARROW:
		{	
			if(tUI_PuSetting.ubDefualtFlag == TRUE)
			{
				if(ubFS_MenuItem == 0)
				{
					ubFS_MenuItem = 1;
					tOsdImgInfo.uwHSize  = 1280;
					tOsdImgInfo.uwVSize  = 720;
					tOsdImgInfo.uwXStart = 48;
					tOsdImgInfo.uwYStart = 0;
					OSD_EraserImg1(&tOsdImgInfo);					
					//tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
					//tOsdImgInfo.uwXStart= 0;
					//tOsdImgInfo.uwYStart =0;	
					//tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);	

					UI_FS_SetTimeMenuDisplay(0);

					ubFS_Timeitem = 0;
				}	
				else if(ubFS_MenuItem == 1)
				{
					if(ubFS_Timeitem < 4)
					{
						ubFS_Timeitem += 1;
					}	
					else
					{
						ubFS_Timeitem = 4;
					}	
					UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
				}
			}
			else
			{
				#if 0 //test
				UI_TestCmd(0, 0);
				#endif
				
				UI_MotorControl(MC_RIGHT_ON);
			}
			break;			
		}
		case UP_ARROW:
		{
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{	
				#if 0
				if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL >= VOL_LVL8)
	  				break;
				
	 			tUI_PuSetting.VolLvL.tVOL_UpdateLvL++;
				UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
				#endif
				
				UI_MotorControl(MC_UP_ON);
				break;
			}
			else
			{
				if(ubFS_MenuItem == 0)
				{
					if(ubLangageFlag >=1)
					{
						ubLangageFlag -= 1;
					}	
					else
					{
						ubLangageFlag = 0 ;
					}
					UI_FS_LangageMenuDisplay(ubLangageFlag);
				}
				else
				{
					if((NULL == pT_Num[ubFS_Timeitem]) || (*pT_Num[ubFS_Timeitem] == ubT_MinNum[ubFS_Timeitem]))
							return;
					(*pT_Num[ubFS_Timeitem])--;
						
					UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
				}
				break;
			}
		}
		case DOWN_ARROW:
		{
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{	
				#if 0
				if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL <= VOL_LVL0)
	  				break;
			
	 			tUI_PuSetting.VolLvL.tVOL_UpdateLvL--;
				UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
				#endif
				
				UI_MotorControl(MC_DOWN_ON);
				break;
			}
			else
			{
				if(ubFS_MenuItem == 0)
				{
					if(ubLangageFlag < 3)
					{
						ubLangageFlag += 1;
					}	
					else
					{
						ubLangageFlag = 3;
					}	
					UI_FS_LangageMenuDisplay(ubLangageFlag);
				}
				else
				{
					if((NULL == pT_Num[ubFS_Timeitem]) || (*pT_Num[ubFS_Timeitem] >= ubT_MaxNum[ubFS_Timeitem]))
						return;
					(*pT_Num[ubFS_Timeitem])++;
						
					UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
				}
				break;
			}
		}

		case ENTER_ARROW:
		{
			if(UI_CheckStopAlarm() == 1)
					return;
			
			if(tUI_PuSetting.ubDefualtFlag == TRUE)
			{
				if(ubFS_MenuItem == 0)
				{
					ubFS_MenuItem = 1;

					tOsdImgInfo.uwHSize  = 1280;
					tOsdImgInfo.uwVSize  = 720;
					tOsdImgInfo.uwXStart = 48;
					tOsdImgInfo.uwYStart = 0;
					OSD_EraserImg1(&tOsdImgInfo);
					//tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
					//tOsdImgInfo.uwXStart= 0;
					//tOsdImgInfo.uwYStart =0;	
					//tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);	
					tUI_PuSetting.ubLangageFlag = ubLangageFlag;
					
					UI_FS_SetTimeMenuDisplay(0);

					ubFS_Timeitem = 0;
				}
				else
				{
					if(ubFS_Timeitem == 3)
					{
						tOsdImgInfo.uwHSize  = 1280;
						tOsdImgInfo.uwVSize  = 720;
						tOsdImgInfo.uwXStart = 48;
						tOsdImgInfo.uwYStart = 0;
						OSD_EraserImg1(&tOsdImgInfo);					
						//tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
						//tOsdImgInfo.uwXStart= 0;
						//tOsdImgInfo.uwYStart =0;	
						//tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);	
						ubFS_Timeitem =0;
						UI_FS_LangageMenuDisplay(ubLangageFlag);

						ubFS_MenuItem = 0;
					}
					else if(ubFS_Timeitem == 4)
					{				
						tOsdImgInfo.uwHSize  = 720;
						tOsdImgInfo.uwVSize  = 1280;
						tOsdImgInfo.uwXStart = 48;
						tOsdImgInfo.uwYStart = 0;
						OSD_EraserImg1(&tOsdImgInfo);

						VDO_Start();

						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart = 0;
						tOsdImgInfo.uwYStart = 0;
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

						UI_LoadDevStatusInfo();
						tUI_PuSetting.ubDefualtFlag = FALSE;
						UI_TimeSetSystemTime();
						tUI_PuSetting.ubLangageFlag = ubLangageFlag;
						UI_UpdateDevStatusInfo();
						UI_PuInit();
					}
				}
			}
			else
			{
				//if(UI_CheckStopAlarm() == 1)
					//return;
			}
			break;
		}
		
		default:
			return;
	}
	//tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
	//tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------
void UI_VolUpKey(void)
{
	if(tUI_State != UI_DISPLAY_STATE)
		return;
	
	if(tUI_PuSetting.ubDefualtFlag == FALSE)
	{
		if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
			return;
	}
	
	if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL > VOL_LVL8)
	  	return;
				
	if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL8)
	{

	}
	else
	{
	 	tUI_PuSetting.VolLvL.tVOL_UpdateLvL++;
	}
	
	UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
}

void UI_VolDownKey(void)
{
	if(tUI_State != UI_DISPLAY_STATE)
		return;
	
	if(tUI_PuSetting.ubDefualtFlag == FALSE)
	{
		if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
			return;
	}
	
	if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL < VOL_LVL0)
	  return;

	if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
	{

	}
	else
	{
	 	tUI_PuSetting.VolLvL.tVOL_UpdateLvL--;
	}
	UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
}
//------------------------------------------------------------------------------
uint8_t UI_GetAlarmStatus(void)
{
	if((ubTempAlarmState == HIGH_TEMP_ALARM_ON) || (ubTempAlarmState == HIGH_TEMP_ALARM_ING))
		return 1;

	if((ubTempAlarmState == LOW_TEMP_ALARM_ON) || (ubTempAlarmState == LOW_TEMP_ALARM_ING))
		return 2;

	if((ubPickupAlarmState == PICKUP_ALARM_ON) || (ubPickupAlarmState == PICKUP_ALARM_ING))
		return 3;
	
	return 0;
}
//------------------------------------------------------------------------------
uint8_t UI_CheckStopAlarm(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	tOsdImgInfo.uwHSize  = 387;
	tOsdImgInfo.uwVSize  = 717;
	tOsdImgInfo.uwXStart = 167;
	tOsdImgInfo.uwYStart = 282;
	
	if((ubTempAlarmState == HIGH_TEMP_ALARM_ON) || ((ubTempAlarmState == HIGH_TEMP_ALARM_ING)))
	{
		OSD_EraserImg2(&tOsdImgInfo);	
		ubTempAlarmState = TEMP_ALARM_OFF;
		return 1;
	}

	if((ubTempAlarmState == LOW_TEMP_ALARM_ON) || (ubTempAlarmState == LOW_TEMP_ALARM_ING))
	{
		OSD_EraserImg2(&tOsdImgInfo);	
		ubTempAlarmState = TEMP_ALARM_OFF;
		return 1;
	}

	if((ubPickupAlarmState == PICKUP_ALARM_ON) || (ubPickupAlarmState == PICKUP_ALARM_ING))
	{
		OSD_EraserImg2(&tOsdImgInfo);	
		ubPickupAlarmState = PICKUP_ALARM_OFF;
		return 1;
	}

	return 0;
}
//------------------------------------------------------------------------------
void UI_DrawMenuPage(void)
{
	uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {OSD2IMG_MENU_BRIGHTNESS,      OSD2IMG_MENU_LCD_AUTO, OSD2IMG_MENU_ALARM,
											  OSD2IMG_MENU_TIME,    OSD2IMG_MENU_ZOOM,  OSD2IMG_MENU_CAMERA,
											  OSD2IMG_MENU_SETTING};
//	uint16_t uwDisplayImgIdx = 0;
	uint8_t i;
	//OSD_IMG_INFO tOsdImgInfo[UI_MENUICON_NUM*4];
	OSD_IMG_INFO tOsdImgInfo;

	UI_ClearBuConnectStatusFlag();

/*
	uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += UI_ICON_HIGHLIGHT;

	for(i = 0; i < UI_MENUICON_NUM; i++)
	{		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS+(i*3), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 48+(i*96);
		tOsdImgInfo.uwYStart =1174;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS_S+(tUI_MenuItem.ubItemIdx*3), 1 , &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(tUI_MenuItem.ubItemIdx*96);
	tOsdImgInfo.uwYStart =1174;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	

	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
*/
	UI_DrawSelectMenuPage();

	OSD_Weight(OSD_WEIGHT_7DIV8);
}
//------------------------------------------------------------------------------
void UI_Menu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {OSD2IMG_MENU_BRIGHTNESS,      OSD2IMG_MENU_LCD_AUTO, OSD2IMG_MENU_ALARM,
											  OSD2IMG_MENU_TIME,    OSD2IMG_MENU_ZOOM,  OSD2IMG_MENU_CAMERA,
											  OSD2IMG_MENU_SETTING};
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_MenuItem.ubItemIdx == BRIGHT_ITEM)
			{
				tUI_MenuItem.ubItemPreIdx = BRIGHT_ITEM;
				tUI_MenuItem.ubItemIdx = SETTING_ITEM;
			}
			else
			{
				tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
				tUI_MenuItem.ubItemIdx 	 -= AUTOLCD_ITEM;
			}	
			break;
		case DOWN_ARROW:
			if(tUI_MenuItem.ubItemIdx == SETTING_ITEM)
			{
				tUI_MenuItem.ubItemPreIdx = SETTING_ITEM;
				tUI_MenuItem.ubItemIdx = BRIGHT_ITEM;
			}
			else
			{
				tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
				tUI_MenuItem.ubItemIdx 	 += AUTOLCD_ITEM;
			}	
			break;
		case LEFT_ARROW:
			/*
			if((tUI_MenuItem.ubItemIdx == BRIGHT_ITEM) ||
			   (tUI_MenuItem.ubItemIdx == ZOOM_ITEM))
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx--;
			*/
			break;
		case RIGHT_ARROW:
			/*
			if((tUI_MenuItem.ubItemIdx == TEMP_ITEM) ||
			   (tUI_MenuItem.ubItemIdx == SETTING_ITEM))
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx++;
			*/
			UI_SubMenu(RIGHT_ARROW);
			return;
		case ENTER_ARROW:
			//! Check tUI_MenuItem.ubItemIdx
			//! Draw Sub menu page (if record and photo page, check select camera number
			UI_SubMenu(ENTER_ARROW);
			return;

		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemPreIdx], 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart= 48+(tUI_MenuItem.ubItemPreIdx*96);
	tOsdImgInfo.uwYStart =1174;		
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	//! Draw highlight icon
	uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += 2;//UI_ICON_HIGHLIGHT;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemIdx], 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart= 48+(tUI_MenuItem.ubItemIdx*96);
	tOsdImgInfo.uwYStart =1174;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	UI_DrawSelectMenuPage();
}


void UI_DrawSelectMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < UI_MENUICON_NUM; i++)
	{		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS_NO_S+(i*3), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 48+(i*96);
		tOsdImgInfo.uwYStart =1174;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	switch(tUI_MenuItem.ubItemIdx)
	{
		case BRIGHT_ITEM:
			UI_DrawBrightnessSubMenuPage_NoSel();
		break;	

		case AUTOLCD_ITEM:
			UI_DrawAutoLcdSubMenuPage_NoSel();
		break;	

		case ALARM_ITEM:
			UI_DrawAlarmSubMenuPage_NoSel();
		break;			

		case TIME_ITEM:
			UI_DrawTimeSubMenuPage_NoSel();
		break;	

		case ZOOM_ITEM:
			UI_DrawZoomSubMenuPage_NoSel();
		break;	

		case CAMERAS_ITEM:
			UI_DrawCamsSubMenuPage_NoSel();
		break;		

		case SETTING_ITEM:
			UI_DrawSettingSubMenuPage_NoSel(1);
		break;			
	}

/*
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =313;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
*/		
}

void UI_DrawBrightnessSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_TITLE+ (2*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 84;
	tOsdImgInfo.uwYStart = 886;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_MAX, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 137;
	tOsdImgInfo.uwYStart = 938;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	for(i = 0; i < brightness_temp; i++)
	{		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_GET, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 565-(48*i);
		tOsdImgInfo.uwYStart = 947;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	for(i = brightness_temp; i < 8; i++)
	{		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_BLANK, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 565-(48*i);
		tOsdImgInfo.uwYStart = 947;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_MIN, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 610;
	tOsdImgInfo.uwYStart = 938;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawBrightnessSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_SEL, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 565-(48*(brightness_temp-1));
	tOsdImgInfo.uwYStart = 947;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

//add by wjb
void UI_ShowSysVolume(uint8_t value)
{
	uint8_t i;
	OSD_IMG_INFO tOsdImgInfo;
	ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n39p1DB, R2R_VOL_n36DB, R2R_VOL_n29p8DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};

	printf("UI_ShowSysVolume tVOL_UpdateLvL: %d.\n", value);
	tUI_PuSetting.VolLvL.ubVOL_UpdateCnt = UI_UPDATEVOLLVL_PERIOD*5; 
	
	if(tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
		ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
	else
    	ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
	tUI_PuSetting.VolLvL.tVOL_UpdateLvL = value;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_VOL_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 48;
	tOsdImgInfo.uwYStart = 1119;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_VOLUP, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 128;
	tOsdImgInfo.uwYStart = 1166;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	for(i = 0; i < 8; i++)
	{
		if(i < (value - 1))
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_VOLMAX, 1, &tOsdImgInfo);
		else if(i == (value - 1))
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_VOL, 1, &tOsdImgInfo);
		else
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_VOLMIN, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 536 - 48*i;
		tOsdImgInfo.uwYStart = 1166;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_VOLDOWN, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 602;
	tOsdImgInfo.uwYStart = 1166;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	UI_UpdateDevStatusInfo();
}

void UI_AutoBrightnessAdjust(void) //20180324
{
	int i;
	uint16_t LightSenseVal = 0;
	LightSenseAdjust_t LightSenseArray[] = 
	{	
		{0,   5,   7},
		{6,  10,   6},
		{11, 15,   5},
		{16, 20,   4},
		{21, 30,   3},
		{31, 40,   2},
		{41, 50,   1},
		{51, 70,   0}
	};
	
	if(tUI_PuSetting.ubAutoBrightness)
	{
		LightSenseVal = uwSADC_GetReport(SADC_CH2);
		for(i = 0; i < 7; i++)
		{
			if((LightSenseVal >= LightSenseArray[i].ubMin) && (LightSenseVal <= LightSenseArray[i].ubMax))
			{
				//printf("LightSenseVal: %d, value(%d, %d).\r\n", LightSenseVal, ubAutoBrightnessValue, LightSenseArray[i].ubValue);
				if(ubAutoBrightnessValue != LightSenseArray[i].ubValue)
				{
					LCD_BACKLIGHT_CTRL(ulUI_BLTable[LightSenseArray[i].ubValue]);
					ubAutoBrightnessValue = LightSenseArray[i].ubValue;
					//printf("UI_AutoBrightnessAdjust: %d#\r\n",ubAutoBrightnessValue);
				}
				break;
			}
		}
	}
}

void UI_BrightnessSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Brightness sub menu page
		//ubCurrentMenuItemIdx = 0;
		//ubNextSubMenuItemIdx = 0;
			
		UI_DrawSubMenuPage(BRIGHT_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	//printf("tArrowKey  %d \n",tArrowKey); 	

	switch(tArrowKey)
	{
		case UP_ARROW:
		    if(ubSubMenuItemFlag == 0)
		    {		   	
			 	if(brightness_temp < 8)
					brightness_temp += 1;
				else
					brightness_temp = 8;
			
				UI_BrightnessDisplay(brightness_temp);
		    }	
			break;	

		case DOWN_ARROW:
		    if(ubSubMenuItemFlag == 0)
		    {
				if(brightness_temp >=2)
					brightness_temp -= 1;
				else
					brightness_temp = 1 ;
			
				UI_BrightnessDisplay(brightness_temp);
		    }	
			break;	

		case LEFT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();
			break;
			
		case RIGHT_ARROW:
			break;

		case ENTER_ARROW:
			break;	

		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();			
		break;
		
	}
}
void UI_BrightnessDisplay(uint8_t value)
{
	uint8_t i;
	OSD_IMG_INFO tOsdImgInfo;
	
	for(i = 0; i < value; i++)
	{		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_GET, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 565-(48*i);
		tOsdImgInfo.uwYStart = 947;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	for(i = value; i < 8; i++)
	{		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_BLANK, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 565-(48*i);
		tOsdImgInfo.uwYStart = 947;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BRI_SEL, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 565-(48*(value-1));
	tOsdImgInfo.uwYStart = 947;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

	tUI_PuSetting.BriLvL.tBL_UpdateLvL = value;
	printf("UI_BrightnessDisplay tBL_UpdateLvL: %d.\n", tUI_PuSetting.BriLvL.tBL_UpdateLvL);
	LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_PuSetting.BriLvL.tBL_UpdateLvL]);
	UI_UpdateDevStatusInfo();
}

void UI_AutoBriDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;

	if(ubSubMenuItemFlag == 1)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_AUTOBRI_NOSEL_S+(value*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 216;
		tOsdImgInfo.uwYStart =848;	
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_AUTOBRI_NOSEL+(value*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 216;
		tOsdImgInfo.uwYStart =848;	
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
	}	
}

void UI_DrawAutoLcdSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 4; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF+(i*2)+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 233+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	if(tUI_PuSetting.ubLangageFlag  != 3)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
		tOsdImgInfo.uwYStart = 1104;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
		tOsdImgInfo.uwYStart = 1154;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LCD_AUTO_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+96;
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawAutoLcdSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF_S+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 233;
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	if(tUI_PuSetting.ubLangageFlag  != 3)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
		tOsdImgInfo.uwYStart = 1104;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
		tOsdImgInfo.uwYStart = 1154;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LCD_AUTO, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+96;
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	
}

void UI_AutoLcdSetSleepTimerEvent(void)
{
	printf("UI_AutoLcdSetSleepTimerEvent###\n");
	LCDBL_ENABLE(UI_DISABLE);
}

void UI_AutoLcdSetSleepTime(uint8_t SleepMode)
{
	uint8_t SleepTime[4] = {0, 1, 3, 5};

	printf("UI_AutoLcdSetSleepTime SleepMode: %d.\n", SleepMode);
	if(SleepMode > 4)
		return;
	
	tUI_PuSetting.ubSleepMode = SleepMode;
	UI_UpdateDevStatusInfo();

	#if 0
	if(SleepMode == 0)
		UI_TimerEventStop();
	else
		UI_TimerEventStart(SleepTime[SleepMode]*1000*60, UI_AutoLcdSetSleepTimerEvent);
	#else
	if(SleepMode == 0)
		UI_TimerDeviceEventStop(TIMER1_2);
	else
		UI_TimerDeviceEventStart(TIMER1_2, SleepTime[SleepMode]*1000*60, UI_AutoLcdSetSleepTimerEvent);
	#endif
}

uint8_t UI_AutoLcdResetSleepTime(uint8_t KeyAction) //20180319
{
	int ret = 0;

	if(PWM->PWM_EN8 == 0)
		ret = 1;
	
	if(KeyAction == KEY_UP_ACT)
	{
		LCDBL_ENABLE(UI_ENABLE);
		UI_AutoLcdSetSleepTime(tUI_PuSetting.ubSleepMode);
	}
	else if(KeyAction == KEY_DOWN_ACT)
	{
		UI_TimerDeviceEventStop(TIMER1_2);
	}

	return ret;
}

void UI_AutoLcdSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubSleepModeFlag = 0;
		ubSubMenuItemFlag = 0;
		ubSubMenuItemPreFlag = 1;	
		
		UI_DrawSubMenuPage(AUTOLCD_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSleepModeFlag >=1)
				ubSleepModeFlag -= 1;
			else
				ubSleepModeFlag = 0 ;	
			UI_AutoLcdSubMenuDisplay(ubSleepModeFlag);
		break;	

		case DOWN_ARROW:
			 if(ubSleepModeFlag < 3)
				ubSleepModeFlag += 1;
			else
				ubSleepModeFlag = 3;
			UI_AutoLcdSubMenuDisplay(ubSleepModeFlag);
		break;	

		case RIGHT_ARROW:
		break;

		case ENTER_ARROW:
			UI_AutoLcdSetSleepTime(ubSleepModeFlag);
			UI_AutoLcdSubMenuDisplay(ubSleepModeFlag);
		break;	

		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();			
		break;
		
	}
}

void UI_AutoLcdSubMenuDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	
	for(i = 0; i < 4; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF+(i*2)+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 233+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF_S+(ubSleepModeFlag*2)+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 233+(ubSleepModeFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	if(tUI_PuSetting.ubLangageFlag  != 3)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
		tOsdImgInfo.uwYStart = 1104;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
		tOsdImgInfo.uwYStart = 1154;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LCD_AUTO, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+96;
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawAlarmSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 309+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALARM_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*2);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
void UI_DrawAlarmSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 309;
	tOsdImgInfo.uwYStart = 729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALARM, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 48+(96*2);
	tOsdImgInfo.uwYStart = 1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_AlarmSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubSubMenuItemFlag = 0;
		ubSubMenuItemPreFlag = 1;	
		
		UI_DrawSubMenuPage(ALARM_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubMenuItemFlag >=1)
			{
				ubSubMenuItemPreFlag = ubSubMenuItemFlag;
				ubSubMenuItemFlag -= 1;
			}	
			else
			{
				break;
			}	
			UI_AlarmmenuDisplay();
		break;	

		case DOWN_ARROW:
			 if(ubSubMenuItemFlag < 1)
			 {
			 	ubSubMenuItemPreFlag = ubSubMenuItemFlag;
				ubSubMenuItemFlag += 1;
			 }	
			else
			{
				break;
			}	
			UI_AlarmmenuDisplay();
		break;	
	
		case RIGHT_ARROW:
		case ENTER_ARROW:
			ubSubSubMenuItemFlag = 0;
			ubSubSubMenuItemPreFlag = 1;	
			
			tUI_State = UI_SUBSUBMENU_STATE;
			UI_DrawSubSubMenu_Alarm();				
		break;	

		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();			
		break;
		
	}
}	

void UI_AlarmmenuDisplay(void)
{
	int i;
	OSD_IMG_INFO tOsdImgInfo;

	for(i = 0; i < 2; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 309+(i*76);
		tOsdImgInfo.uwYStart = 729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT_S+(ubSubMenuItemFlag*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 309+(ubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart = 729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALARM, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 48+(96*2);
	tOsdImgInfo.uwYStart = 1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawSubSubMenu_Alarm(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
//--------------------------------------------------------------------------------
	if(ubSubMenuItemFlag == 0)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 270;
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 346;
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	


		if(tUI_PuSetting.ubHighTempSetting != 0)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 270;
			tOsdImgInfo.uwYStart =400;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
		else
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 270;
			tOsdImgInfo.uwYStart =400;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}

		if(tUI_PuSetting.ubLangageFlag  != 2)
		{
			if(tUI_PuSetting.ubLowTempSetting != 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 346;
				tOsdImgInfo.uwYStart =431;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 346;
				tOsdImgInfo.uwYStart =431;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
		}
		else
		{
			if(tUI_PuSetting.ubLowTempSetting != 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 346;
				tOsdImgInfo.uwYStart =386;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 346;
				tOsdImgInfo.uwYStart =386;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
		}
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 422;
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 309;
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 306+76;
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
	}
}

void UITempSubmenuDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	switch(ubSubSubMenuItemFlag)
	{
		case 0:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 270;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 346;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			if(tUI_PuSetting.ubHighTempSetting != 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 270;
				tOsdImgInfo.uwYStart =400;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 270;
				tOsdImgInfo.uwYStart =400;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}

			if(tUI_PuSetting.ubLangageFlag  != 2)
			{
				if(tUI_PuSetting.ubLowTempSetting != 0)
				{			
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =431;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =431;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			else
			{
				if(tUI_PuSetting.ubLowTempSetting != 0)
				{			
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =386;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =386;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 422;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
		break;

		case 1:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 270;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 346;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			if(tUI_PuSetting.ubHighTempSetting != 0)
			{			
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 270;
				tOsdImgInfo.uwYStart =427;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 270;
				tOsdImgInfo.uwYStart =427;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}

			if(tUI_PuSetting.ubLangageFlag  != 2)
			{
				if(tUI_PuSetting.ubLowTempSetting != 0)
				{			
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0_S+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =400;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =400;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			else
			{
				if(tUI_PuSetting.ubLowTempSetting != 0)
				{			
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0_S+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =365;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =365;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}			
			}
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 422;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;

		case 2:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 270;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 346;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			if(tUI_PuSetting.ubHighTempSetting != 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 270;
				tOsdImgInfo.uwYStart =427;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 270;
				tOsdImgInfo.uwYStart =427;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);			
			}

			if(tUI_PuSetting.ubLangageFlag  != 2)
			{
				if(tUI_PuSetting.ubLowTempSetting != 0)
				{			
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =431;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =431;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			else
			{
				if(tUI_PuSetting.ubLowTempSetting != 0)
				{			
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =386;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 346;
					tOsdImgInfo.uwYStart =386;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S_S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 422;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;		
	}
}
void UISoundSubmenuDisplay(void)	
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(ubSubSubMenuItemFlag)
	{
		case 0:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 309;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 306+76;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
		break;

		case 1:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF+(tUI_PuSetting.ubSoundLevelSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 309;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT5S_S+(tUI_PuSetting.ubSoundAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 306+76;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;	
	}
}
void UI_AlarmSubSubMenuUpKey(uint8_t SubMenuItem)
{
	switch(SubMenuItem)
	{
		case 0:
			if(ubSubSubMenuItemFlag >=1)
			{
				ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
				ubSubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubSubMenuItemFlag = 0 ;
				ubSubSubMenuItemPreFlag =  1;
			}
			UITempSubmenuDisplay();						
		break;


		case 1:
			if(ubSubSubMenuItemFlag >=1)
			{
				ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
				ubSubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubSubMenuItemFlag = 0 ;
				ubSubSubMenuItemPreFlag =  1;
			}
			UISoundSubmenuDisplay();
		break;	
	}
}
void UI_AlarmSubSubMenuDownKey(uint8_t SubMenuItem)
{
	switch(SubMenuItem)
	{
		case 0:
			 if(ubSubSubMenuItemFlag < 2)
			 {
			 	ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
				ubSubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubSubMenuItemPreFlag =  2;
				ubSubSubMenuItemFlag = 0;
			}	
			UITempSubmenuDisplay();						
		break;


		case 1:
			 if(ubSubSubMenuItemFlag < 1)
			 {
			 	ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
				ubSubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubSubMenuItemPreFlag =  1;
				ubSubSubMenuItemFlag = 0;
			}	
			UISoundSubmenuDisplay();
		break;	
	}
}
void UI_AlarmSubSubMenuEnterKey(uint8_t SubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;

	uint8_t item_temp;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =0;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	
	switch(SubMenuItem)
	{
		case 0:
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			item_temp = (ubSubMenuItemFlag*3)+ubSubSubMenuItemFlag;
			InitAlarmSubSubSubmenu(item_temp);
			UITempSubSubSubmenuDisplay(ubSubSubMenuItemFlag);						
		break;


		case 1:
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			InitAlarmSubSubSubmenu(item_temp);
			UISoundSubSubSubmenuDisplay(ubSubSubMenuItemFlag);	
		break;	
	}
}

void InitAlarmSubSubSubmenu(uint8_t SubMenuItem)
{
	switch(SubMenuItem)
	{
		case 0:
			ubSubSubSubMenuItemFlag = 0;
			ubSubSubSubMenuItemPreFlag = 1;
			ubSubSubSubMenuRealItem = 0;			
		break;	

		case 1:
			ubSubSubSubMenuItemFlag = 0;
			ubSubSubSubMenuItemPreFlag = 1;
		break;
		
		case 2:
			ubSubSubSubMenuItemFlag = 0;
			ubSubSubSubMenuItemPreFlag = 1;
		break;

		case 3:
			ubSubSubSubMenuItemFlag = 0;
			ubSubSubSubMenuItemPreFlag = 1;
		break;

		case 4:
			ubSubSubSubMenuItemFlag = 0;
			ubSubSubSubMenuItemPreFlag = 1;
		break;		
	}
}

void UITempSubSubSubmenuDisplay(uint8_t SubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	switch(SubMenuItem)
	{
		case 0:	
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 194;
			tOsdImgInfo.uwYStart =0;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			for(i = 1; i < 5; i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_C0+((i-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(i*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

			if(ubSubSubSubMenuItemFlag == 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_C0_S+((ubSubSubSubMenuItemFlag-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 227 +(tUI_PuSetting.ubHighTempSetting*76);
			tOsdImgInfo.uwYStart =188;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
		break;

		case 1:		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 194;
			tOsdImgInfo.uwYStart =0;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			
			for(i = 1; i < 5; i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_LOWTEMP_C0+((i-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(i*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

			if(ubSubSubSubMenuItemFlag == 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SUBMENU_LOWTEMP_C0_S+((ubSubSubSubMenuItemFlag-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 227 +(tUI_PuSetting.ubLowTempSetting*76);
			tOsdImgInfo.uwYStart =188;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;	

		case 2:
			for(i = 0; i < 4; i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 237+(i*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

	
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S_S+(ubSubSubSubMenuItemFlag*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 237+(ubSubSubSubMenuItemFlag*76);
			tOsdImgInfo.uwYStart =0;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	


			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 264 + (tUI_PuSetting.ubTempAlertSetting*76);
			tOsdImgInfo.uwYStart =195;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;
	}
}

void UISoundSubSubSubmenuDisplay(uint8_t SubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	switch(SubMenuItem)
	{
		case 0:		
			for(i = 0; i < 6; i++)
			{	
				if(i == 0)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_OFF+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 159+(i*76);
					tOsdImgInfo.uwYStart =0;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_LVL1+((i-1)*2), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 159+(i*76);
					tOsdImgInfo.uwYStart =0;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
				}
			}

			if(ubSubSubSubMenuItemFlag == 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 159+(ubSubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_LVL1_S+((ubSubSubSubMenuItemFlag-1)*2), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 159+(ubSubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 191+(tUI_PuSetting.ubSoundLevelSetting*76);
			tOsdImgInfo.uwYStart =233;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;

		case 1:		
			for(i = 0; i < 4; i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 237+(i*76);
				tOsdImgInfo.uwYStart =0;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}

	
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S_S+(ubSubSubSubMenuItemFlag*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 237+(ubSubSubSubMenuItemFlag*76);
			tOsdImgInfo.uwYStart =0;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	


			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 264 + (tUI_PuSetting.ubSoundAlertSetting*76);
			tOsdImgInfo.uwYStart =195;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	
	}
}

void UI_AlarmSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	switch(tArrowKey)
	{
		case UP_ARROW:
			UI_AlarmSubSubMenuUpKey(ubSubMenuItemFlag);
		break;	

		case DOWN_ARROW:
			UI_AlarmSubSubMenuDownKey(ubSubMenuItemFlag);
		break;	

		case RIGHT_ARROW:
		case ENTER_ARROW:
			UI_AlarmSubSubMenuEnterKey(ubSubMenuItemFlag);			
		break;	
		
		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_SUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 729;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);			
		break;
	}
}


void UI_AlarmSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t item_temp;

	item_temp = (ubSubMenuItemFlag*3)+ubSubSubMenuItemFlag;
	
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubSubSubMenuItemFlag >=1)
			{
				ubSubSubSubMenuItemPreFlag =  ubSubSubSubMenuItemFlag;
				ubSubSubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubSubSubMenuItemFlag = 0 ;
				ubSubSubSubMenuItemPreFlag =  1;
			}

			if(item_temp >= 3)
				UISoundSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
			else
				UITempSubSubSubmenuDisplay(ubSubSubMenuItemFlag);	
		break;	

		case DOWN_ARROW:
			 if(ubSubSubSubMenuItemFlag < ubAlarmSubSubItemNum[item_temp] )
			 {
			 	ubSubSubSubMenuItemPreFlag =  ubSubSubSubMenuItemFlag;
				ubSubSubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubSubSubMenuItemPreFlag =  0;
				ubSubSubSubMenuItemFlag = ubAlarmSubSubItemNum[item_temp];
			}

			if(item_temp >= 3)
				UISoundSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
			else
				UITempSubSubSubmenuDisplay(ubSubSubMenuItemFlag);				
		break;	

		case RIGHT_ARROW:
		case ENTER_ARROW:
			UI_SetAlarm(item_temp);
			
			if((tUI_PuSetting.ubHighTempSetting == 0)&&(tUI_PuSetting.ubLowTempSetting == 0)
				&&(tUI_PuSetting.ubSoundLevelSetting == 0))
				ubAlarmIconFlag = 0;
			else
				ubAlarmIconFlag = 1;

			UI_UpdateDevStatusInfo();
			
			tUI_State = UI_SUBSUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 284;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);		

			if(item_temp >= 3)
				UISoundSubmenuDisplay();
			else
				UITempSubmenuDisplay();	
			
		break;	
		
		case LEFT_ARROW:
		case EXIT_ARROW:	
			tUI_State = UI_SUBSUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 284;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);			
		break;
	}

}

void UI_SetAlarm(uint8_t SubMenuItem)
{
	switch(SubMenuItem)
	{
		case 0:
			tUI_PuSetting.ubHighTempSetting 		= ubSubSubSubMenuItemFlag;			
		break;	

		case 1:
			tUI_PuSetting.ubLowTempSetting 		= ubSubSubSubMenuItemFlag;
		break;	

		case 2:
			tUI_PuSetting.ubTempAlertSetting 		= ubSubSubSubMenuItemFlag;
		break;	

		case 3:
			tUI_PuSetting.ubSoundLevelSetting 		= ubSubSubSubMenuItemFlag;
		break;	

		case 4:
			tUI_PuSetting.ubSoundAlertSetting 		= ubSubSubSubMenuItemFlag;	
		break;

		default:
			break;
		
	}
	UI_UpdateDevStatusInfo();
}

void UI_AlarmTriggerDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_ALARM_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 167;
	tOsdImgInfo.uwYStart =282;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	
	switch(value)
	{
		case 0:	//high temp
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_TITLE+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167+263;
			tOsdImgInfo.uwYStart =282+157;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167+79;
			tOsdImgInfo.uwYStart =282+301;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		break;	

		case 1:	// low temp
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_TITLE+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167+263;
			tOsdImgInfo.uwYStart =282+157;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167+79;
			tOsdImgInfo.uwYStart =282+301;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		break;

		case 2:   // sound
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_ALARMON_TITLE+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167+263;
			tOsdImgInfo.uwYStart =282+157;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_ALARMON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167+78;
			tOsdImgInfo.uwYStart =282+284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);							
		break;
	}
}

void UI_CheckTempAlarm(void)
{
	uint8_t ubHightempMax,ubLowtempMin;
	OSD_IMG_INFO tOsdImgInfo;

	if(ubAlarmSoundShowFlag == 1) 
		return;

	if(tUI_State == UI_DISPLAY_STATE)
	{
		
		if(tUI_PuSetting.ubTempunitFlag == 1)
		{
			ubHightempMax = ubHighTempC[tUI_PuSetting.ubHighTempSetting];
			ubLowtempMin = ubLowTempC[tUI_PuSetting.ubHighTempSetting];
		}
		else
		{
			ubHightempMax = ubHighTempF[tUI_PuSetting.ubHighTempSetting];
			ubLowtempMin = ubLowTempF[tUI_PuSetting.ubHighTempSetting];
		}
		
		if(tUI_PuSetting.ubHighTempSetting != 0)
		{
			if(ubRealTemp >= ubHightempMax)	
			{
				if(ubTempAlarmcheck == 0)
					ubHighAlarmOn =1;
			}
			else 
			{
				ubHighAlarmOn =0;
			}
		}
		else
		{
			ubHighAlarmOn =0;
			ubHighAlarmTriggerFlag =0;
			ubAlarmClearFlag = 0;
			
			if(tUI_PuSetting.ubLowTempSetting  == 0)
				ubAlarmDisplayFlag =0;		
		}

		//printf("RTC: %d \n",RTC->RTC_TIMER0);

		if(tUI_PuSetting.ubLowTempSetting != 0)
		{
			if(ubRealTemp <= ubLowtempMin)
			{
			   if(ubTempAlarmcheck == 0)
				ubLowAlarmOn = 1;
			}
			else
			{
				ubLowAlarmOn = 0;
			}
		}
		else
		{
			ubLowAlarmOn =0;
			ubLowAlarmTriggerFlag =0;
			ubAlarmClearFlag = 0;

			if(tUI_PuSetting.ubHighTempSetting  == 0)		
				ubAlarmDisplayFlag =0;
		}

		if((ubHighAlarmOn == 1)&&(ubHighAlarmTriggerFlag == 0)&&(ubAlarmDisplayFlag == 0))
		{
			ubTempAlarmcheck = 1;
			
			if(tUI_State == UI_DISPLAY_STATE)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 167;
				tOsdImgInfo.uwYStart =282;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp/10)*2), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 340;
				tOsdImgInfo.uwYStart =600;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 340;
				tOsdImgInfo.uwYStart =600-24;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

				if(tUI_PuSetting.ubTempunitFlag == 1)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_C, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 334;
					tOsdImgInfo.uwYStart =600-72;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);			
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_F, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 334;
					tOsdImgInfo.uwYStart =600-72;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
				}
				ubAlertCnt =0;
				ubAlarmDisplayFlag =1;
			
			}
		}

		if((ubLowAlarmOn == 1)&&(ubLowAlarmTriggerFlag == 0)&&(ubAlarmDisplayFlag == 0))
		{
			ubTempAlarmcheck = 1;
		
			if(tUI_State == UI_DISPLAY_STATE)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 167;
				tOsdImgInfo.uwYStart =282;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_0+((ubRealTemp/10)*2), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 340;
				tOsdImgInfo.uwYStart =600;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_0+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 340;
				tOsdImgInfo.uwYStart =600-24;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

				if(tUI_PuSetting.ubTempunitFlag == 1)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_C, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 334;
					tOsdImgInfo.uwYStart =600-72;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);			
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_F, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 334;
					tOsdImgInfo.uwYStart =600-72;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
				}
				ubAlertCnt = 0;
				ubAlarmDisplayFlag = 1;
			
			}
		}	

		if(ubAlarmDisplayFlag == 1)
		{
			UI_SetAlert(ubAlertTime[tUI_PuSetting.ubTempAlertSetting]);
		}

		if((ubHighAlarmOn == 0)&&(ubLowAlarmOn == 0)&&(ubAlarmClearFlag == 0))
		{
			if(ubAlarmSoundPickupOn == 0)
			{
				if(tUI_State == UI_DISPLAY_STATE)
				{
					tOsdImgInfo.uwHSize  = 387;
					tOsdImgInfo.uwVSize  = 717;
					tOsdImgInfo.uwXStart = 167;
					tOsdImgInfo.uwYStart = 282;
					OSD_EraserImg2(&tOsdImgInfo);	
				}
			}
			ubAlarmClearFlag = 1;
		}
	}
	
	if(ubHighAlarmTriggerFlag == 1) //ubTempAlarmcheck == 1
	{
		//printf("ubAlarmIdleCnt %d \n",ubAlarmIdleCnt);
		if(ubAlarmIdleCnt < 300) //180
		{
			ubAlarmIdleCnt++;
		}
		else
		{
			ubTempAlarmcheck = 0;	
			ubAlarmIdleCnt =0;
			ubLowAlarmTriggerFlag = 0;
			ubHighAlarmTriggerFlag = 0;
			ubAlarmDisplayFlag = 0;
			ubAlertCnt =0;
		}
	}	
}

void UI_CheckSoundAlarm(void)
{
	uint8_t ubBuMaxSoundLevel;
	OSD_IMG_INFO tOsdImgInfo;

	if(ubAlarmDisplayFlag == 1)
		return;

	if(tUI_State == UI_DISPLAY_STATE)
	{
		if(tUI_PuSetting.ubSoundLevelSetting != 0)
		{
			if(ubGetVoiceTemp >= tUI_PuSetting.ubSoundLevelSetting)	
			{
				if(ubAlarmSoundDisplay == 0)
					ubAlarmSoundPickupOn = 1;
			}
			else 
			{
				ubAlarmSoundPickupOn = 0;
			}
		}
		else
		{
			ubAlarmSoundPickupOn = 0;	
		}

		if((ubAlarmSoundPickupOn == 1)&&(ubAlarmSoundTriggerFlag == 0)&&(ubAlarmSoundDisplay == 0))
		{		
			if(ubAlarmDisplayFlag == 0)
			{
				if(tUI_State == UI_DISPLAY_STATE)
				{
					if(ubAlarmSoundShowFlag == 0)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_ALARMON+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart = 167;
						tOsdImgInfo.uwYStart = 282;	
						tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
						ubAlarmSoundShowFlag = 1;
						ubAlertCnt =0;
					}
				}
				ubAlarmSoundDisplay =1;
				printf("UI_CheckTempAlarm Sound ubAlarmDisplayFlag = 1 @@@@@@\n");
			}	
		}

		if(ubAlarmSoundDisplay == 1)
		{
			UI_SetAlert(ubAlertTime[tUI_PuSetting.ubTempAlertSetting]);
		}

		/*
		if((ubAlarmSoundPickupOn == 0)&&(ubAlarmDisplayFlag == 0))
		{
			if(tUI_State == UI_DISPLAY_STATE)
			{
				if(ubAlarmSoundShowFlag == 1)
				{
					tOsdImgInfo.uwHSize  = 387;
					tOsdImgInfo.uwVSize  = 717;
					tOsdImgInfo.uwXStart = 167;
					tOsdImgInfo.uwYStart = 282;
					OSD_EraserImg2(&tOsdImgInfo);	
				}
			}
			ubAlarmSoundShowFlag = 0;
		}
		*/
	}
	
	if(ubAlarmSoundTriggerFlag == 1) //ubAlarmSoundDisplay == 1
	{
		printf("ubAlarmSoundIdleCnt %d \n",ubAlarmSoundIdleCnt);
		if(ubAlarmSoundIdleCnt < 300) //180
		{
			ubAlarmSoundIdleCnt++;
		}
		else
		{
			ubAlarmSoundDisplay = 0;	
			ubAlarmSoundIdleCnt =0;
			ubAlarmSoundTriggerFlag =0;
			ubAlertCnt =0;
			ubAlarmSoundShowFlag = 0;
		}		
	}
}

void UI_SetAlert(uint8_t time)
{
	if(++ubAlertCnt > (time * 5))
	{
		ubAlertCnt = 100;
		ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
	}
	else
	{
		//printf("ubAlertCnt %d \n",ubAlertCnt);
		ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
		ADO_SetDacR2RVol(R2R_VOL_n5p6DB);		
		if((ubAlertCnt%3) == 0)
		{
			BUZ_PlayLowBatSound();
			printf("UI_SetAlert BUZ_PlaySingleSound###\n");
		}
	}
} 

/*********************************wjb@apical********************************/
void UI_ShowAlarm(uint8_t type)
{
	OSD_IMG_INFO tOsdImgInfo;

	printf("UI_ShowAlarm type: %d.\n", type);
	switch(type)
	{
		case 0:
			UI_AlarmTriggerDisplay(0);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 340;
			tOsdImgInfo.uwYStart = 600;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 340;
			tOsdImgInfo.uwYStart =600-24;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_F - 2*tUI_PuSetting.ubTempunitFlag, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 334;
			tOsdImgInfo.uwYStart = 600-72;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			break;

		case 1:
			UI_AlarmTriggerDisplay(1);

			/*
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 167;
			tOsdImgInfo.uwYStart = 282;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			*/

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_0+((ubRealTemp/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 340;
			tOsdImgInfo.uwYStart = 600;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_0+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 340;
			tOsdImgInfo.uwYStart =600-24;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_F - 2*tUI_PuSetting.ubTempunitFlag, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 334;
			tOsdImgInfo.uwYStart =600-72;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			break;

		case 2:
			UI_AlarmTriggerDisplay(2);
			break;
	}

	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_PlayAlarmSound(uint8_t type)
{
	uint8_t ubAlertSetting = 0;
	switch(type)
	{
		case 0:
		case 1:
			ubAlertSetting = tUI_PuSetting.ubTempAlertSetting;
			break;

		case 2:
			ubAlertSetting = tUI_PuSetting.ubSoundAlertSetting;
			break;

		default:
			break;
	}
	
	if(++ubPlayAlarmCount > (ubAlertTime[ubAlertSetting] * 5))
	{
		ubPlayAlarmCount = 200;
		ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
	}
	else
	{
		ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
		ADO_SetDacR2RVol(R2R_VOL_n5p6DB);		
		if((ubPlayAlarmCount%3) == 0)
		{
			printf("UI_PlayAlarmSound BUZ_PlaySingleSound###\n");
			BUZ_PlayLowBatSound();
		}
	}
}

void UI_TempAlarmCheck(void)
{
	uint8_t ubHightempMax,ubLowtempMin;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	if(ubFactoryModeFLag == 1)
		return;

	if(tUI_State != UI_DISPLAY_STATE)
	{
		if(ubTempAlarmState == TEMP_ALARM_IDLE)
			return;

		ubTempAlarmState = TEMP_ALARM_OFF;
	}

	if(ubTempAlarmState == TEMP_ALARM_IDLE)
	{
		if(tUI_PuSetting.ubTempunitFlag == 1)
		{
			ubHightempMax = ubHighTempC[tUI_PuSetting.ubHighTempSetting];
			ubLowtempMin = ubLowTempC[tUI_PuSetting.ubLowTempSetting];
		}
		else
		{
			ubHightempMax = ubHighTempF[tUI_PuSetting.ubHighTempSetting];
			ubLowtempMin = ubLowTempF[tUI_PuSetting.ubLowTempSetting];
		}
		
		if(tUI_PuSetting.ubHighTempSetting != 0)
		{
			if(ubRealTemp >= ubHightempMax)	
			{
				if((ubPickupAlarmState == PICKUP_ALARM_IDLE) || (ubPickupAlarmState == PICKUP_ALARM_OFF))
				{
					ubTempAlarmState = HIGH_TEMP_ALARM_ON;
				}
			}
		}

		if(tUI_PuSetting.ubLowTempSetting != 0)
		{
			if(ubRealTemp <= ubLowtempMin)	
			{
				if((ubPickupAlarmState == PICKUP_ALARM_IDLE) || (ubPickupAlarmState == PICKUP_ALARM_OFF))
				{
					ubTempAlarmState = LOW_TEMP_ALARM_ON;
				}
			}
		}
	}
	else if(ubTempAlarmState == HIGH_TEMP_ALARM_ON)
	{
		UI_ShowAlarm(0);
		ubPlayAlarmCount = 0;
		ubTempAlarmState = HIGH_TEMP_ALARM_ING;
	}
	else if(ubTempAlarmState == HIGH_TEMP_ALARM_ING)
	{
		UI_PlayAlarmSound(0);
	}
	else if(ubTempAlarmState == LOW_TEMP_ALARM_ON)
	{
		UI_ShowAlarm(1);
		ubTempAlarmState = LOW_TEMP_ALARM_ING;
	}
	else if(ubTempAlarmState == LOW_TEMP_ALARM_ING)
	{
		UI_PlayAlarmSound(1);
	}
	else if(ubTempAlarmState == TEMP_ALARM_OFF)
	{
		//printf("TEMP: (%d) ++++++\n", ubTempAlarmTriggerCount);
		if(ubTempAlarmTriggerCount < ALARM_INTERVAL) //60s
		{
			ubTempAlarmTriggerCount++;
		}
		else
		{
			ubTempAlarmState = TEMP_ALARM_IDLE;
			ubTempAlarmTriggerCount = 0;
		}	
	}
}

void UI_PickupAlarmCheck(void)
{
	uint8_t ubHightempMax, ubLowtempMin;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	if(tUI_State != UI_DISPLAY_STATE)
	{
		if(ubPickupAlarmState == PICKUP_ALARM_IDLE)
			return;

		ubPickupAlarmState = PICKUP_ALARM_OFF;
	}

	if(ubPickupAlarmState == PICKUP_ALARM_IDLE)
	{
		if(tUI_PuSetting.ubSoundLevelSetting != 0)
		{
			if(ubGetVoiceTemp >= tUI_PuSetting.ubSoundLevelSetting)	
			{
				if((ubTempAlarmState == TEMP_ALARM_IDLE) || (ubTempAlarmState == TEMP_ALARM_OFF))
				{
					ubPickupAlarmState = PICKUP_ALARM_ON;
				}
			}
		}
	}
	else if(ubPickupAlarmState == PICKUP_ALARM_ON)
	{
		UI_ShowAlarm(2);
		ubPlayAlarmCount = 0;
		ubPickupAlarmState = PICKUP_ALARM_ING;
	}
	else if(ubPickupAlarmState == PICKUP_ALARM_ING)
	{
		UI_PlayAlarmSound(2);
	}
	else if(ubPickupAlarmState == PICKUP_ALARM_OFF)
	{
		//printf("PICKUP: (%d) ------\n", ubPickupAlarmTriggerCount);
		if(ubPickupAlarmTriggerCount < ALARM_INTERVAL) //60s
		{
			ubPickupAlarmTriggerCount++;
		}
		else
		{
			ubPickupAlarmState = PICKUP_ALARM_IDLE;
			ubPickupAlarmTriggerCount = 0;
		}
	}
}
/*********************************wjb@apical********************************/

void UI_DrawTimeSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB_TIME+ (1*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 295;
	tOsdImgInfo.uwYStart =880;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 337;
	tOsdImgInfo.uwYStart =781;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 383;
	tOsdImgInfo.uwYStart =1020;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 467;
	tOsdImgInfo.uwYStart =1020;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	//-----------------------------------------------
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 415;
	tOsdImgInfo.uwYStart =852;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	//-------------------------------------------------------------------
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 415;
	tOsdImgInfo.uwYStart =930;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 415;
	tOsdImgInfo.uwYStart =950;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 415;
	tOsdImgInfo.uwYStart =985;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	//-----------------------------------------------------------------------
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 415;
	tOsdImgInfo.uwYStart =1018;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 415;
	tOsdImgInfo.uwYStart =1038;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*3);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawTimeSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	UI_DrawSelectMenuPage();

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB_TIME+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 295;
	tOsdImgInfo.uwYStart =880;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

	UI_TimeSubMenuDisplay(ubSubMenuItemFlag);	

//----------------------------------------------------------------------------
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*3);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_TimeSetSystemTime(void) //add by wjb
{
	tUI_PuSetting.tSysCalendar.uwYear = 2018;
	tUI_PuSetting.tSysCalendar.ubMonth = 1;
	tUI_PuSetting.tSysCalendar.ubDate = 1;

	if(ubTimeAMPM)
		tUI_PuSetting.tSysCalendar.ubHour = ubTimeHour + 12;
	else
		tUI_PuSetting.tSysCalendar.ubHour = ubTimeHour;
	
	tUI_PuSetting.tSysCalendar.ubMin = ubTimeMin;
	tUI_PuSetting.tSysCalendar.ubSec = 0;
	
	if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
		printd(DBG_ErrorLvl, "Calendar base setting fail !\n");
	else
		RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
}

void UI_TimeShowSystemTime(uint8_t type)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	RTC_GetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));

	if((type == 1) || (ubTimeHour != (tUI_PuSetting.tSysCalendar.ubHour%12)) || (ubTimeMin != tUI_PuSetting.tSysCalendar.ubMin)
		|| (ubTimeAMPM^(tUI_PuSetting.tSysCalendar.ubHour >= 12?1:0)))
	{
		if(tUI_PuSetting.tSysCalendar.ubHour >= 12)
		{
			ubTimeAMPM = 1;
	 		ubTimeHour = tUI_PuSetting.tSysCalendar.ubHour - 12;
		}
		else
		{
			ubTimeAMPM = 0;
			ubTimeHour = tUI_PuSetting.tSysCalendar.ubHour;
		}
		ubTimeMin = tUI_PuSetting.tSysCalendar.ubMin;

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeHour/10), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 221;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeHour%10), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 203;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_TIMESIGN, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 190;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeMin/10), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 174;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeMin%10), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 155;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_TIMEAM + ubTimeAMPM, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 98;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
}

void UI_TimeSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubT_MaxNum[3] = {12, 59, 1};
	uint8_t ubT_MinNum[3] = {1,  0,  0};	
	uint8_t *pT_Num[3] = {(uint8_t *)(&ubTimeHour),
						(uint8_t *)(&ubTimeMin), (uint8_t *)(&ubTimeAMPM)
						};


	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubSubMenuItemFlag = 0;
		ubSubMenuItemPreFlag = 1;	

		if(tUI_PuSetting.tSysCalendar.ubHour >= 12)
		{
			ubTimeAMPM = 1;
			if(tUI_PuSetting.tSysCalendar.ubHour == 12)
				ubTimeHour = 12;
			else
	 			ubTimeHour = tUI_PuSetting.tSysCalendar.ubHour - 12;
		}
		else
		{
			ubTimeAMPM = 0;

			if(tUI_PuSetting.tSysCalendar.ubHour == 0)
				ubTimeHour = 12;
			else
				ubTimeHour = tUI_PuSetting.tSysCalendar.ubHour;
		}
		ubTimeMin = tUI_PuSetting.tSysCalendar.ubMin;


		//printf("ubTimeHour  %d\n",ubTimeHour);
		ubEnterTimeMenuFlag = 1;
		
		UI_DrawSubMenuPage(TIME_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	switch(tArrowKey)
	{
		case UP_ARROW:
			if((NULL == pT_Num[ubSubMenuItemFlag]))
					return;
			if(*pT_Num[ubSubMenuItemFlag] == ubT_MinNum[ubSubMenuItemFlag])
				*pT_Num[ubSubMenuItemFlag] = ubT_MaxNum[ubSubMenuItemFlag];
			else
				(*pT_Num[ubSubMenuItemFlag])--;
				
			UI_TimeSubMenuDisplay(ubSubMenuItemFlag);	
			return;
			
		case DOWN_ARROW:
			if((NULL == pT_Num[ubSubMenuItemFlag]))
				return;
			if(*pT_Num[ubSubMenuItemFlag] == ubT_MaxNum[ubSubMenuItemFlag])
				*pT_Num[ubSubMenuItemFlag] = ubT_MinNum[ubSubMenuItemFlag];
			else
				(*pT_Num[ubSubMenuItemFlag])++;
				
			UI_TimeSubMenuDisplay(ubSubMenuItemFlag);
			return;

		case LEFT_ARROW:
			if(ubSubMenuItemFlag >=1)
			{
				ubSubMenuItemFlag -= 1;
			}	
			else
			{
				ubSubMenuItemFlag = 0;
				tUI_State = UI_MAINMENU_STATE;
				ubEnterTimeMenuFlag = 0;
				UI_DrawMenuPage();
				return;
			}	
			UI_TimeSubMenuDisplay(ubSubMenuItemFlag);
		break;	

		case RIGHT_ARROW:
			 if(ubSubMenuItemFlag < 2)
			 {
				ubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubMenuItemFlag = 2;
			}
			UI_TimeSubMenuDisplay(ubSubMenuItemFlag);		
		break;

		case ENTER_ARROW:
			UI_TimeSetSystemTime();
			UI_TimeShowSystemTime(1);
		break;
			
		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;

			ubEnterTimeMenuFlag = 0;
			
			UI_DrawMenuPage();			
		break;
	}
}	


void UI_TimeSubMenuDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;


	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB_TIME+ (1*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 295;
	tOsdImgInfo.uwYStart =880;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	
	switch(value)
	{
		case 0:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 378;
			tOsdImgInfo.uwYStart =845;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
					
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 383;
			tOsdImgInfo.uwYStart =1020;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 467;
			tOsdImgInfo.uwYStart =1020;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =852;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =930;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =950;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =985;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =1018;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =1038;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		break;	

		case 1:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 378;
			tOsdImgInfo.uwYStart =845;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 383;
			tOsdImgInfo.uwYStart =947;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 467;
			tOsdImgInfo.uwYStart =947;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =852;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =930+10;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =950+10;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =985+20;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =1018+10;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =1038+10;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		break;

		case 2:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 378;
			tOsdImgInfo.uwYStart =845;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 383;
			tOsdImgInfo.uwYStart =865;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 467;
			tOsdImgInfo.uwYStart =865;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

		//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_AM_S+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =852;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =930+20;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =950+20;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =985+20;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =1018+10;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 415;
			tOsdImgInfo.uwYStart =1038+10;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		break;	
	}
}	

void UI_Zoom_SetScaleParam(uint8_t tZoomScale)
{
	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();

	printf("UI_Zoom_SetScaleParam tZoomScale: %d, tUI_PuSetting.ubZoomScale: %d.\r\n", tZoomScale, tUI_PuSetting.ubZoomScale);

	if(tZoomScale == 0) //off
	{
		tUI_DptzParam.tScaleParam							 = UI_SCALEUP_2X;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize*2/tUI_DptzParam.tScaleParam;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize*2/tUI_DptzParam.tScaleParam;
		tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize	   	 = ulLcd_HSize;
		tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize	   	 = ulLcd_VSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
	}
	else if(tZoomScale == 1) //1.5X
	{
		tUI_DptzParam.tScaleParam							 = UI_SCALEUP_3X;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize*2/tUI_DptzParam.tScaleParam;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize*2/tUI_DptzParam.tScaleParam;
		tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize	   	 = ulLcd_HSize;
		tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize	   	 = ulLcd_VSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
	}
	else if(tZoomScale == 2) //2X
	{
		tUI_DptzParam.tScaleParam							 = UI_SCALEUP_4X;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize*2/tUI_DptzParam.tScaleParam;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize*2/tUI_DptzParam.tScaleParam;
		tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize	   	 = ulLcd_HSize;
		tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize	   	 = ulLcd_VSize;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
		tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
	}
			
	tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
}

void UI_DrawZoomSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 3; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF+(i*2)+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 270+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
	tOsdImgInfo.uwYStart = 1066;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ZOOM_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*4);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
}

void UI_DrawZoomSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF_S+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 270;
	tOsdImgInfo.uwYStart =729;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
	tOsdImgInfo.uwYStart = 1066;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}	
void UI_ZoomSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubSubMenuItemFlag = 0;
		ubSubMenuItemPreFlag = 1;	
		
		UI_DrawSubMenuPage(ZOOM_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubMenuItemFlag >=1)
			{
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				ubSubMenuItemFlag  -=  1;
			}	
			else
			{
				break;
			}	
			UI_ZoomDisplay();
		break;	

		case DOWN_ARROW:
			if(ubSubMenuItemFlag < 2)
			{
			 	ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				ubSubMenuItemFlag += 1;
			}	
			else
			{
				break;
			}	
			UI_ZoomDisplay();
		break;	


		case RIGHT_ARROW:
	
		break;

		case ENTER_ARROW:
			tUI_PuSetting.ubZoomScale = ubSubMenuItemFlag;
			UI_Zoom_SetScaleParam(tUI_PuSetting.ubZoomScale);
			UI_ZoomDisplay();
			UI_UpdateDevStatusInfo();
		break;	

		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();			
		break;
		
	}
}	

void UI_ZoomDisplay(void)
{
	int i;
	OSD_IMG_INFO tOsdImgInfo;

	for(i = 0; i < 3; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF+(i*2)+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 270+(i*76);
		tOsdImgInfo.uwYStart = 729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF_S+(ubSubMenuItemFlag*2)+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 270+(ubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart = 729;		
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
	tOsdImgInfo.uwYStart = 1066;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_ZOOM, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*4);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
}

//------------------------------------------
void UI_DrawCamsSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 3; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD+(i*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 270+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAMERA_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*5);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
}
void UI_DrawCamsSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD_S+(ubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 270+(ubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAMERA, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*5);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_CamSubSubmenuDisplay(uint8_t SubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i, j;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	switch(SubMenuItem)
	{
		case 1:
			for(i = 0; i < 4; i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_NOPAIR+ubCamPairFlag[i]+(i*3)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 233+(i*76);
				tOsdImgInfo.uwYStart =284;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}	

			if(ubNoAddCamFlag == 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_PAIR_S+(ubSubSubMenuItemFlag*3)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 233+(ubSubSubMenuItemFlag*76);
				tOsdImgInfo.uwYStart =284;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			}
			
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	

		case 2:
			for(i = 0; i < 5; i++)
			{	
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SCAN_OFF+(i*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 194+(i*76);
				tOsdImgInfo.uwYStart =284;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}	

			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SCAN_OFF_S+(ubSubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 194+(ubSubSubMenuItemFlag*76);
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
			

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 225 + (tUI_PuSetting.ubScanTime*76);
			tOsdImgInfo.uwYStart =572;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
		break;			
	}
}

void UI_DeletemenuDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =0;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_MENUDEL+(ubSubSubSubMenuItemPreFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 309+(ubSubSubSubMenuItemPreFlag*76);
	tOsdImgInfo.uwYStart =0;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	


	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_MENUDEL_S+(ubSubSubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 309+(ubSubSubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =0;		
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
}

void UI_GetPairCamInfo(void)
{
	uint8_t i;
		
	for(i = 0; i < 4 ; i++)
	{
		printf("tUI_CamStatus[%d].ulCAM_ID  %lx \n",i,tUI_CamStatus[i].ulCAM_ID);
		if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
		{
			ubCamPairFlag[i] = 1; 
		}
		else
		{
			ubCamPairFlag[i] = 0;
			tUI_PuSetting.NightmodeFlag = tUI_PuSetting.NightmodeFlag & (~(0x01<< i)) ;
		}
		printf("ubCamPairFlag[%d]= %d \n",i,ubCamPairFlag[i] );
	}


	for(i = 0; i < 4 ; i++)
	{
		if(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
		{
			ubPairSelCam = i;
			break;
		}	
	}

	if((ubCamPairFlag[0] == 1)&&(ubCamPairFlag[1] == 1)
		&&(ubCamPairFlag[2] == 1)&&(ubCamPairFlag[3] == 1))
		ubCamFullFlag = 1;
	else
		ubCamFullFlag = 0;			

	if((ubCamPairFlag[0] == 0)&&(ubCamPairFlag[1] == 0)
		&&(ubCamPairFlag[2] == 0)&&(ubCamPairFlag[3] == 0))
		ubNoAddCamFlag = 1;
	else
		ubNoAddCamFlag = 0;
}

//------------------------------------------------------------------------------
void UI_CameraSettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{		
	OSD_IMG_INFO tOsdImgInfo;
	APP_EventMsg_t tUI_PairMessage = {0};
	uint8_t i;
	
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubSubMenuItemFlag = 0;
		ubSubMenuItemPreFlag = 1;	

		UI_GetPairCamInfo();

		if(tUI_PuSetting.ubTotalBuNum == 0)
			ubDelCamitem =0;
		else
			ubDelCamitem = tUI_PuSetting.ubTotalBuNum - 1;

		UI_DrawSubMenuPage(CAMERAS_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	if(tUI_SyncAppState == APP_PAIRING_STATE) //if(tPairInfo.ubDrawFlag == TRUE)
	{
		if((tArrowKey == UP_ARROW) || (tArrowKey == DOWN_ARROW) || (tArrowKey == RIGHT_ARROW) || (tArrowKey == ENTER_ARROW))
		{
			return;
		}
		else if((tArrowKey == EXIT_ARROW) || (tArrowKey == LEFT_ARROW))
		{
			UI_PairingControl(tArrowKey);
			tUI_SyncAppState = APP_LOSTLINK_STATE;
			return;
		}
	}

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubMenuItemFlag >=1)
			{
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				ubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubMenuItemFlag = 0 ;
				ubSubMenuItemPreFlag =  1;
			}	
			UI_CamSubmenuDisplay();
		break;	

		case DOWN_ARROW:
			 if(ubSubMenuItemFlag < 2)
			 {
			 	ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				ubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubMenuItemPreFlag =  1;
				ubSubMenuItemFlag = 2;
			}	
			UI_CamSubmenuDisplay();
		break;	

		case RIGHT_ARROW:
		case ENTER_ARROW:
			if(ubSubMenuItemFlag == 0)
			{
				for(i = 0; i < 4 ; i++)
				{
					if(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
					{
						ubPairSelCam = i;
						break;
					}	
				}
				//printf("ubPairSelCam %d \n",i,ubPairSelCam);

				if(ubCamFullFlag == 0)
				{
					ubPairDisplayTime = 60; //10
					tPairInfo.tPairSelCam = ubPairSelCam;
					tPairInfo.tDispLocation = ubPairSelCam;
					
					tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
					tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
					tUI_PairMessage.ubAPP_Message[1] = ubPairSelCam;//CAM1;
					tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
					tUI_PairMessage.ubAPP_Message[2] = ubPairSelCam;					
					UI_SendMessageToAPP(&tUI_PairMessage);
					tPairInfo.ubDrawFlag 			 = TRUE;
					UI_DisableScanMode();

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 48;
					tOsdImgInfo.uwYStart =284;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIRING+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 200;
					tOsdImgInfo.uwYStart =284;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_PAIR_SEC+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 145;
					tOsdImgInfo.uwYStart =450;	
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 48;
					tOsdImgInfo.uwYStart =284;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIR_FULL+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 200;
					tOsdImgInfo.uwYStart =284;	
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
				}
			}
			else
			{			
				tUI_State = UI_SUBSUBMENU_STATE;
				
				if(ubSubMenuItemFlag == 2)
				{
					ubSubSubMenuItemFlag = 0; //tUI_PuSetting.ubScanModeEn
					ubUI_ScanStartFlag = FALSE;
					ubSubSubMenuItemPreFlag = 1;
				}
				else
				{
					ubSubSubMenuItemFlag = 0;
					ubSubSubMenuItemPreFlag = 1;	
				}
				UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);		
			}
		break;	

		case LEFT_ARROW:
		case EXIT_ARROW:
			if(tUI_SyncAppState != APP_PAIRING_STATE)
			{
				tUI_State = UI_MAINMENU_STATE;
				UI_DrawMenuPage();
			}		
		break;
	}
}

//------------------------------------------------------------------------------
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	int i = 0, j = 0;
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubMenu_MaxNum[3] = {3,ubDelCamitem,4};

	if(ubSubMenuItemFlag == 0)
	{
		if((tArrowKey == UP_ARROW) || (tArrowKey == DOWN_ARROW) || (tArrowKey == RIGHT_ARROW) || (tArrowKey == ENTER_ARROW))
		{
			return;
		}
	}
	
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubMenuItemFlag == 1)
			{
				if(ubSubSubMenuItemFlag == 0)
					return;
				
				for(i = (ubSubSubMenuItemFlag - 1); i >= 0; i--)
				{
					if(ubCamPairFlag[i] == 1)
					{
						ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
						ubSubSubMenuItemFlag 	= i;
						break;
					}
				}

			}
			else
			{
				if(ubSubSubMenuItemFlag >= 1)
				{
					ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
					ubSubSubMenuItemFlag -= 1;
				}	
				else
				{
					ubSubSubMenuItemFlag = 0;
					ubSubSubMenuItemPreFlag = 1;
				}
			}
			UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);			
		break;	

		case DOWN_ARROW:
			if(ubSubMenuItemFlag == 1)
			{
				if(ubSubSubMenuItemFlag >= 3)
					return;

				for(i = (ubSubSubMenuItemFlag + 1); i < 4; i++)
				{
					if(ubCamPairFlag[i] == 1)
					{
						ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
						ubSubSubMenuItemFlag 	= i;
						break;
					}
				}
			}
			else
			{
				if(ubSubSubMenuItemFlag < ubMenu_MaxNum[ubSubMenuItemFlag])
				{
				 	ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
					ubSubSubMenuItemFlag += 1;
				}	
				else
				{
					ubSubSubMenuItemPreFlag =  0;
					ubSubSubMenuItemFlag = ubMenu_MaxNum[ubSubMenuItemFlag];
				}
			}
			UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);			
		break;	

		case RIGHT_ARROW:
		case ENTER_ARROW:
			if(ubSubMenuItemFlag == 1)
			{
				i = ubSubSubMenuItemFlag;
				for(j = 0; j < 4; j++)
				{
					if(ubCamPairFlag[i] == 1)
					{
						ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
						ubSubSubMenuItemFlag 	= i;
						break;
					}
					i++;
					if(i > 3) i = 0;
				}

				if((ubCamPairFlag[0] == 0) && (ubCamPairFlag[1] == 0)&& (ubCamPairFlag[2] == 0)&& (ubCamPairFlag[3] == 0))
					break;

				ubSubSubSubMenuItemFlag = 0;
				ubSubSubSubMenuItemPreFlag = 1;

				tUI_State = UI_SUBSUBSUBMENU_STATE;	
				UI_DeletemenuDisplay();
			}
			else
			{
				UI_SetScanMenu(ubSubSubMenuItemFlag);			
			}
		break;	
		
		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_SUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 729;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);			
		break;
	}

}

void UI_CamDeleteCamera(uint8_t type, uint8_t CameraId)
{
	APP_EventMsg_t tUI_UnindBuMsg = {0};

	if(type)
	{
		for(int i = 0; i < 4; i++)
		{
			if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
			{
				printf("UI_CamDeleteCamera CameraID: %d.\n");
				tUI_CamStatus[i].ulCAM_ID 	= INVALID_ID;
				tUI_CamStatus[i].tCamConnSts = CAM_OFFLINE;
				tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
				tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == i)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
				UI_ResetDevSetting(i);
				UI_UpdateDevStatusInfo();
				tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_BU_EVENT;
				tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
				tUI_UnindBuMsg.ubAPP_Message[1] = i;
				UI_SendMessageToAPP(&tUI_UnindBuMsg);
				ubCamPairFlag[CameraId] = 0;
				tUI_PuSetting.NightmodeFlag = tUI_PuSetting.NightmodeFlag & (~(0x01<< i));
				osDelay(100);
			}
		}
	}
	else
	{
		tUI_CamStatus[CameraId].ulCAM_ID 	= INVALID_ID;
		tUI_CamStatus[CameraId].tCamConnSts = CAM_OFFLINE;
		tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
		tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == CameraId)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
		UI_ResetDevSetting(CameraId);
		UI_UpdateDevStatusInfo();
		tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_BU_EVENT;
		tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
		tUI_UnindBuMsg.ubAPP_Message[1] = CameraId;
		UI_SendMessageToAPP(&tUI_UnindBuMsg);
		ubCamPairFlag[CameraId] = 0;
		tUI_PuSetting.NightmodeFlag = tUI_PuSetting.NightmodeFlag & (~(0x01<< CameraId));
	}
}

void UI_CamSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	int i = 0;
	OSD_IMG_INFO tOsdImgInfo;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubSubSubMenuItemFlag >=1)
			{
				ubSubSubSubMenuItemPreFlag =  ubSubSubSubMenuItemFlag;
				ubSubSubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubSubSubMenuItemFlag = 0 ;
				ubSubSubSubMenuItemPreFlag =  1;
			}
				UI_DeletemenuDisplay();
		break;	

		case DOWN_ARROW:
			 if(ubSubSubSubMenuItemFlag <1 )
			 {
			 	ubSubSubSubMenuItemPreFlag =  ubSubSubSubMenuItemFlag;
				ubSubSubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubSubSubMenuItemPreFlag =  0;
				ubSubSubSubMenuItemFlag = 1;
			}
				UI_DeletemenuDisplay();			
		break;	

		case RIGHT_ARROW:
		case ENTER_ARROW:
			if(ubSubSubSubMenuItemFlag == 0) // delete	
			{
				APP_EventMsg_t tUI_UnindBuMsg = {0};
	
				tUI_CamStatus[ubSubSubMenuItemFlag].ulCAM_ID 	= INVALID_ID;
				tUI_CamStatus[ubSubSubMenuItemFlag].tCamConnSts = CAM_OFFLINE;
				tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
				tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == ubSubSubMenuItemFlag)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
				UI_ResetDevSetting(ubSubSubMenuItemFlag);
				UI_UpdateDevStatusInfo();
				tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_BU_EVENT;
				tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
				tUI_UnindBuMsg.ubAPP_Message[1] = ubSubSubMenuItemFlag;
				UI_SendMessageToAPP(&tUI_UnindBuMsg);
				ubCamPairFlag[ubSubSubMenuItemFlag] = 0; //20180324
				tUI_PuSetting.NightmodeFlag = tUI_PuSetting.NightmodeFlag & (~(0x01<< ubSubSubMenuItemFlag)) ;
			}
			tUI_State = UI_SUBSUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 284;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);						
			UI_GetPairCamInfo();

			for(i = 0; i < 4 ; i++)
			{
				if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
				{
					tCamViewSel.tCamViewPool[0] = i;
					ubSubSubMenuItemFlag = i;
					break;
				}					
			}
			
			UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);
			
			if(ubNoAddCamFlag == 1)
				tCamViewSel.tCamViewPool[0] = CAM1;
					
			tCamViewSel.tCamViewType	= SINGLE_VIEW;
			tUI_PuSetting.tAdoSrcCamNum = tCamViewSel.tCamViewPool[0];
			UI_SwitchCameraSource();
			UI_ClearBuConnectStatusFlag();
			ubSetViewCam = tCamViewSel.tCamViewPool[0];						
			UI_UpdateDevStatusInfo();
		break;	
		
		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_SUBSUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 284;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);			
		break;
	}

}
void UI_SetScanMenu(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	switch(value)
	{
		case 0:
			tUI_PuSetting.ubScanTime = 0;			
			tUI_PuSetting.ubScanModeEn = FALSE;		
		break;	

		case 1:
			tUI_PuSetting.ubScanTime = 1;
			tUI_PuSetting.ubScanModeEn = TRUE;	
			ubUI_ScanStartFlag = FALSE;
		break;	

		case 2:
			tUI_PuSetting.ubScanTime = 2;	
			tUI_PuSetting.ubScanModeEn = TRUE;	
			ubUI_ScanStartFlag = FALSE;
		break;	

		case 3:
			tUI_PuSetting.ubScanTime = 3;		
			tUI_PuSetting.ubScanModeEn = TRUE;	
			ubUI_ScanStartFlag = FALSE;
		break;	

		case 4:
			tUI_PuSetting.ubScanTime = 4;	
			tUI_PuSetting.ubScanModeEn = TRUE;
			ubUI_ScanStartFlag = FALSE;
		break;			
	}

	tUI_State = UI_SUBMENU_STATE;		
	tOsdImgInfo.uwHSize  = 672;
	tOsdImgInfo.uwVSize  = 729;
	tOsdImgInfo.uwXStart = 48;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg2(&tOsdImgInfo);

	UI_UpdateDevStatusInfo();

	printf("UI_SetScanMenu ubScanTime: %d.\n", tUI_PuSetting.ubScanTime);
	if(tUI_PuSetting.ubScanTime == 0)
		UI_DisableScanMode();
	else
		UI_EnableScanMode();
}
//------------------------------------------------------------------------------
//#define PAIRING_ICON_NUM	2
void UI_DrawPairingStatusIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	if(ubPairDisplayCnt < 3)
	{
		ubPairDisplayCnt++;
	}
	else
	{
		ubPairDisplayCnt = 0;
		
		if(ubPairDisplayTime == 0)
			ubPairDisplayTime = 0;
		else
			ubPairDisplayTime -= 1;
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIRING+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 200;
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_PAIR_0+(ubPairDisplayTime%10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 144;
	tOsdImgInfo.uwYStart =520; //488
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_PAIR_0+(ubPairDisplayTime/10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 144;
	tOsdImgInfo.uwYStart =541;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
}

void UI_CamSubmenuDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD+(ubSubMenuItemPreFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 270+(ubSubMenuItemPreFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	


	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD_S+(ubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 270+(ubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAMERA, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*5);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

//------------------------------------------------------------------------------
void UI_ReportPairingResult(UI_Result_t tResult)
{
	OSD_IMG_INFO tOsdImgInfo;	
	APP_EventMsg_t tUI_UnindBuMsg = {0};
	UI_CamNum_t tCamNum;

	tPairInfo.ubDrawFlag = FALSE;

	switch(tResult)
	{
		case rUI_SUCCESS:
			tUI_PuSetting.ubPairedBuNum += (tUI_PuSetting.ubPairedBuNum >= tUI_PuSetting.ubTotalBuNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
			for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
			{
				if(tCamNum == tPairInfo.tPairSelCam)
					continue;
				if((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
				   (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
				{
					tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_BU_EVENT;
					tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
					tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
					UI_SendMessageToAPP(&tUI_UnindBuMsg);
					tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
					tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;

					UI_ResetDevSetting(tCamNum);
				}
			}
			if((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
			   (NO_CAM == tUI_PuSetting.tAdoSrcCamNum) ||
			   (INVALID_ID == tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].ulCAM_ID))
				tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;

			ubPairDisplayTime = 60; //10
	
			UI_ResetDevSetting(tPairInfo.tPairSelCam);
			UI_UpdateDevStatusInfo();

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 48;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIR_OK+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 235;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			

			UI_GetPairCamInfo();

			if(tCamViewSel.tCamViewPool[0] != tPairInfo.tPairSelCam)
			{
				ubPairOK_SwitchCam = 1;
				tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam; //20180608
			}
			/*
			if(tCamViewSel.tCamViewPool[0] != tPairInfo.tPairSelCam)
			{
				tCamViewSel.tCamViewType	= SINGLE_VIEW;
				tCamViewSel.tCamViewPool[0] 	= tPairInfo.tPairSelCam;
				tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
				UI_SwitchCameraSource();
			}	
			*/
			break;
		case rUI_FAIL:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 48;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIR_FAIL+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 234;
			tOsdImgInfo.uwYStart =284;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			
			break;
		default:
			break;
	}
	tUI_State = UI_SUBSUBMENU_STATE;
}

void UI_DrawSettingSubMenuPage_NoSel(uint8_t type)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 7; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 118+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SETTING_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*6);
	tOsdImgInfo.uwYStart =1173;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE+type);	
}

void UI_DrawSettingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 118+(0*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SETTING, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*6);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_SettingSubmenuDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+(ubSubMenuItemPreFlag*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 118+(ubSubMenuItemPreFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+(ubSubMenuItemFlag*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 118+(ubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SETTING, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*6);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		ubSubMenuItemFlag = 0;
		ubSubMenuItemPreFlag = 1;	
		
		UI_DrawSubMenuPage(SETTING_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}
	
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubMenuItemFlag >=1)
			{
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				ubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubMenuItemFlag = 0 ;
				ubSubMenuItemPreFlag =  1;
			}	
			UI_SettingSubmenuDisplay();
		break;	

		case DOWN_ARROW:
			 if(ubSubMenuItemFlag < 6)
			 {
			 	ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				ubSubMenuItemFlag += 1;
			 }	
			else
			{
				ubSubMenuItemPreFlag =  5;
				ubSubMenuItemFlag = 6;
			}	
			UI_SettingSubmenuDisplay();
		break;	


		case RIGHT_ARROW:
		case ENTER_ARROW:
			tUI_State = UI_SUBSUBMENU_STATE;
			ubSubSubMenuItemFlag = 0;
			ubSubSubMenuItemPreFlag = 1;				
			UI_DrawSettingSubSubMenuPage(ubSubMenuItemFlag);
		break;	
		
		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();			
		break;
		
	}
}

void UI_DrawSettingSubSubMenuPage(uint8_t SubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	switch(SubMenuItem)
	{
		case NIGHTMODE_ITEM:
			for(i = 0; i < 4; i++)
			{
				if(ubCamPairFlag[i])
				{
					ubSubSubMenuItemFlag 	= i;
					break;
				}
			}
			if((i == 3)&&(ubCamPairFlag[3] == 0))
				break;
			
			UI_NightModeDisplay(ubSubSubMenuItemFlag);				
			break;
	
		case LANGUAGESET_ITEM:				
			ubSubSubMenuRealItem = 0;
			UI_LangageDisplay(ubSubSubMenuItemFlag);
			break;

		case FLICKER_ITEM:
			UI_FlickerDisplay(ubSubSubMenuItemFlag);
			break;
			
		case DEFAULT_ITEM:
			UI_DefualtDisplay(ubSubSubMenuItemFlag);
			break;
			
		case TEMPUNIT_ITEM:
			UI_TempUnitDisplay(ubSubSubMenuItemFlag);
			break;

		case PRODUCT_INFO_ITEM:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_INFO_INFO+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 290;
			tOsdImgInfo.uwYStart =377;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
			break;
			
		case CONTACT_ITEM:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CONTACT_INFO, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 139;
			tOsdImgInfo.uwYStart =325;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);			
			break;

		default:
			break;
	}
	
}

void UI_NightModeDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i, j;	

	for(i = 0; i < 4; i++)
	{	
		if((tUI_PuSetting.NightmodeFlag >> i) & (0x01) == 1)
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_ON+(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		else
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_OFF_NOPAIR+(i*3)+ubCamPairFlag[i]+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 233+(i*76);
		tOsdImgInfo.uwYStart = 284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	i = value;
	for(j = 0; j < 4; j++)
	{
		if(ubCamPairFlag[i] == 1)
		{
			if((tUI_PuSetting.NightmodeFlag >> i) & (0x01) == 1)
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_ON+(i*2)+ubCamPairFlag[i]+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			else
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_OFF+(i*3)+ubCamPairFlag[i]+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 233+(i*76);
			tOsdImgInfo.uwYStart = 284;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			break;
		}
		i++;
		if(i > 3) 
			i = 0;
	}
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_LangageDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;	

	for(i = 0; i < 4; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LANGAGE_0+(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 232+(i*76);
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_LANGAGE_0_S+(value*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 232+(value*76);
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 260 + (tUI_PuSetting.ubLangageFlag*76);
	tOsdImgInfo.uwYStart = 565;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
	
}

void UI_FlickerDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FLICKER_INFO+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 240;
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FLICKER_50+(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 392+(i*76);
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FLICKER_50_S+(value*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 392+(value*76);
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 422 +(tUI_PuSetting.ubFlickerFlag*76);
	tOsdImgInfo.uwYStart = 596;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DefualtDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_DEFUALT_INFO+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 240;
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NO-(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 392+(i*76);
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NO_S-(value*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 392+(value*76);
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_TempUnitDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;	

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMP_F+(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 308+(i*76);
		tOsdImgInfo.uwYStart =284;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMP_F_S+(value*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 308+(value*76);
	tOsdImgInfo.uwYStart =284;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 338 +(tUI_PuSetting.ubTempunitFlag*76);
	tOsdImgInfo.uwYStart = 658;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_NightModeSubSubSubmenuDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;	
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBSUBMENU_BG, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48;
	tOsdImgInfo.uwYStart =0;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NIG_OFF+(i*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 309+(i*76);
		tOsdImgInfo.uwYStart =0;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NIG_OFF_S+(ubSubSubSubMenuItemFlag*2)+ (66*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 309+(ubSubSubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =0;		
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 338+((tUI_PuSetting.NightmodeFlag>>ubSubSubMenuItemFlag)*76);
	tOsdImgInfo.uwYStart =167;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
	
}

//------------------------------------------------------------------------------
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(ubSubMenuItemFlag)
	{
		case NIGHTMODE_ITEM:
			break;
	
		case LANGUAGESET_ITEM:	
			ubLangageFlag = ubSubSubMenuItemFlag;
			break;

		case FLICKER_ITEM:
			ubFlickerFlag = ubSubSubMenuItemFlag;
			break;
			
		case DEFAULT_ITEM:
			ubDefualtFlag = ubSubSubMenuItemFlag;
			break;
			
		case TEMPUNIT_ITEM:
			ubTempunitFlag = ubSubSubMenuItemFlag;
			break;

		case PRODUCT_INFO_ITEM:
			break;
			
		case CONTACT_ITEM:
			break;

		default:
			break;
	}

	
	switch(tArrowKey)
	{
		case UP_ARROW:
			UI_SettingSubSubMenuUpKey(ubSubMenuItemFlag);			
		break;	

		case DOWN_ARROW:
			UI_SettingSubSubMenuDownKey(ubSubMenuItemFlag);		
		break;	

		case ENTER_ARROW:
			UI_SettingSubSubMenuEnterKey(ubSubMenuItemFlag);				
		break;	
		
		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_SUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 729;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);			
		break;
	}

}

void UI_SettingSubSubMenuUpKey(uint8_t SubMenuItem)
{
	int i = 0;
	
	switch(SubMenuItem)
	{
		case NIGHTMODE_ITEM:
			for(i = ubSubSubMenuItemFlag; i < 4; i++)
			{
				if(ubCamPairFlag[i])
				{
					ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
					ubSubSubMenuItemFlag 	= i;
					break;
				}
			}
			if((i == 3)&&(ubCamPairFlag[3] == 0))
				break;
			
			if(ubSubSubMenuItemFlag <= 0)
					break;
	
			ubSubSubMenuItemFlag -= 1;
			UI_NightModeDisplay(ubSubSubMenuItemFlag);	
			break;
	
		case LANGUAGESET_ITEM:	
			if(ubSubSubMenuItemFlag < 1)
				break;
			ubSubSubMenuItemFlag -= 1;
			
			ubLangageFlag = ubSubSubMenuItemFlag;
			UI_LangageDisplay(ubLangageFlag);
			break;

		case FLICKER_ITEM:
			if(ubSubSubMenuItemFlag == 0)
				break;
			ubSubSubMenuItemFlag = 0;
			
			ubFlickerFlag = ubSubSubMenuItemFlag;
			UI_FlickerDisplay(ubFlickerFlag);
			break;
			
		case DEFAULT_ITEM:
			if(ubSubSubMenuItemFlag == 0)
				break;
			ubSubSubMenuItemFlag = 0;
			
			ubDefualtFlag = ubSubSubMenuItemFlag;		
			UI_DefualtDisplay(ubDefualtFlag);	
			break;
			
		case TEMPUNIT_ITEM:
			if(ubSubSubMenuItemFlag == 0)
				break;
			ubSubSubMenuItemFlag = 0;
			
			ubTempunitFlag = ubSubSubMenuItemFlag;		
			UI_TempUnitDisplay(ubTempunitFlag);	
			break;

		case PRODUCT_INFO_ITEM:
			break;
			
		case CONTACT_ITEM:
			break;

		default:
			break;
	}
	
}
void UI_SettingSubSubMenuDownKey(uint8_t SubMenuItem)
{
	int i = 0;
	
	switch(SubMenuItem)
	{
		case NIGHTMODE_ITEM:
			for(i = ubSubSubMenuItemFlag; i < 4; i++)
			{
				if(ubCamPairFlag[i])
				{
					ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
					ubSubSubMenuItemFlag 	= i;
					break;
				}
			}
			if((i == 3)&&(ubCamPairFlag[3] == 0))
				break;
			
			if(ubSubSubMenuItemFlag >= 3)
				break;
			
			ubSubSubMenuItemFlag += 1;	
			UI_NightModeDisplay(ubSubSubMenuItemFlag);
			break;
	
		case LANGUAGESET_ITEM:	
			if(ubSubSubMenuItemFlag >= 3)
				break;
			ubSubSubMenuItemFlag += 1;

			ubLangageFlag = ubSubSubMenuItemFlag;
			UI_LangageDisplay(ubLangageFlag);
			break;

		case FLICKER_ITEM:
			if(ubSubSubMenuItemFlag == 1)
				break;
			ubSubSubMenuItemFlag = 1;	

			ubFlickerFlag = ubSubSubMenuItemFlag;
			UI_FlickerDisplay(ubFlickerFlag);
			break;
		case DEFAULT_ITEM:
			if(ubSubSubMenuItemFlag == 1)
				break;
			ubSubSubMenuItemFlag = 1;	

			ubDefualtFlag = ubSubSubMenuItemFlag;
			UI_DefualtDisplay(ubDefualtFlag);
			break;
			
		case TEMPUNIT_ITEM:
			if(ubSubSubMenuItemFlag == 1)
				break;
			ubSubSubMenuItemFlag = 1;	

			ubTempunitFlag = ubSubSubMenuItemFlag;
			UI_TempUnitDisplay(ubTempunitFlag);
			break;

		case PRODUCT_INFO_ITEM:
			break;
			
		case CONTACT_ITEM:
			break;

		default:
			break;
	}
}
void UI_SettingSubSubMenuEnterKey(uint8_t SubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tSelCamNum; 

	tSelCamNum = tCamViewSel.tCamViewPool[0] ;
	
	switch(SubMenuItem)
	{
		case NIGHTMODE_ITEM:
			tUI_State = UI_SUBSUBSUBMENU_STATE;
			ubSubSubSubMenuItemFlag =0;
			ubSubSubSubMenuRealItem =0;
			UI_NightModeSubSubSubmenuDisplay(ubSubSubSubMenuRealItem);
			break;
	
		case LANGUAGESET_ITEM:	
			tUI_PuSetting.ubLangageFlag = ubLangageFlag;
			UI_LangageDisplay(ubLangageFlag);
			UI_DrawSettingSubMenuPage_NoSel(0);
			UI_SettingSubmenuDisplay();
			UI_UpdateDevStatusInfo();
			break;

		case FLICKER_ITEM:
			if(ubFlickerFlag == 0)
			{			
				tUI_CamStatus[tSelCamNum].tCamFlicker = CAMFLICKER_50HZ;
			}
			else
			{
				tUI_CamStatus[tSelCamNum].tCamFlicker = CAMFLICKER_60HZ;					
			}
			tUI_PuSetting.ubFlickerFlag = ubFlickerFlag; //ubFlickerFlag;
			UI_FlickerDisplay(ubFlickerFlag);
			UI_UpdateDevStatusInfo();
			break;
			
		case DEFAULT_ITEM:
			if(ubDefualtFlag == 1)
			{
				tUI_PuSetting.ubDefualtFlag = TRUE;

				tUI_PuSetting.ubZoomScale				= 0;
				tUI_PuSetting.VolLvL.tVOL_UpdateLvL		= VOL_LVL6;
				//tUI_PuSetting.NightmodeFlag				= 0x00; //
				tUI_PuSetting.ubHighTempSetting 		= 0;
				tUI_PuSetting.ubLowTempSetting 			= 0;
				tUI_PuSetting.ubTempAlertSetting 		= 1;
				tUI_PuSetting.ubSoundLevelSetting 		= 0;
				tUI_PuSetting.ubSoundAlertSetting 		= 1;	
				tUI_PuSetting.ubTempunitFlag 			= 0;
				tUI_PuSetting.ubFlickerFlag 			= 1;
				tUI_PuSetting.ubSleepMode				= 1;
				tUI_PuSetting.BriLvL.tBL_UpdateLvL		= BL_LVL8;
				tUI_PuSetting.ubScanTime 				= 1;
				tUI_PuSetting.ubScanModeEn 				= TRUE;

				tUI_PuSetting.ubFeatCode0			= 0x00;
				tUI_PuSetting.ubFeatCode1			= 0x00;
				tUI_PuSetting.ubFeatCode2			= 0x00;	

				UI_CamDeleteCamera(1, 0); //tUI_PuSetting.NightmodeFlag on
				printf("defulat ~~~\n");			
				UI_UpdateDevStatusInfo();	
				ubFactorySettingFlag = 0;

				tOsdImgInfo.uwHSize  = 720;
				tOsdImgInfo.uwVSize  = 1280;
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg2(&tOsdImgInfo); //复位重启以后OSD可能会有残留

				WDT_Disable(WDT_RST);
				WDT_RST_Enable(WDT_CLK_EXTCLK, 1);//reboot
				while(1);
			}
			break;
			
		case TEMPUNIT_ITEM:
			tUI_PuSetting.ubTempunitFlag  = ubTempunitFlag;
			ubRealTemp = tUI_PuSetting.ubTempunitFlag?ubRealTemp:UI_TempCToF(ubRealTemp); //20180323
			UI_UpdateDevStatusInfo();	
			UI_TempUnitDisplay(ubTempunitFlag);
			break;

		case PRODUCT_INFO_ITEM:
			break;
			
		case CONTACT_ITEM:
			break;

		default:
			break;
	}
	/*
	tUI_State = UI_SUBMENU_STATE;		
	tOsdImgInfo.uwHSize  = 672;
	tOsdImgInfo.uwVSize  = 729;
	tOsdImgInfo.uwXStart = 48;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg2(&tOsdImgInfo);
	*/	
}

void UI_NightModeSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubSubSubSubMenuItemFlag >=1)
			{
				ubSubSubSubMenuItemPreFlag =  ubSubSubSubMenuItemFlag;
				ubSubSubSubMenuItemFlag  -=  1;
			}	
			else
			{
				ubSubSubSubMenuItemFlag = 0 ;
				ubSubSubSubMenuItemPreFlag =  1;
			}
			UI_NightModeSubSubSubmenuDisplay(ubSubSubSubMenuRealItem);
		break;	

		case DOWN_ARROW:
			if(ubSubSubSubMenuItemFlag < 1 )
			{
			 	ubSubSubSubMenuItemPreFlag =  ubSubSubSubMenuItemFlag;
				ubSubSubSubMenuItemFlag += 1;
			}	
			else
			{
				ubSubSubSubMenuItemPreFlag =  0;
				ubSubSubSubMenuItemFlag = 1;
			}
			UI_NightModeSubSubSubmenuDisplay(ubSubSubSubMenuRealItem);
		break;	

		case RIGHT_ARROW:
			break;
			
		case ENTER_ARROW:
			//tUI_PuSetting.NightmodeFlag = tUI_PuSetting.NightmodeFlag | (0x01<< ubSubSubMenuItemFlag);
			tUI_PuSetting.NightmodeFlag &=(~(1<<ubSubSubMenuItemFlag));
			tUI_PuSetting.NightmodeFlag |=(ubSubSubSubMenuItemFlag<<ubSubSubMenuItemFlag);
			UI_UpdateDevStatusInfo();
			UI_NightModeDisplay(ubSubSubMenuItemFlag);
		
		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_SUBSUBMENU_STATE;		
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 284;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg2(&tOsdImgInfo);			
		break;
	}
}
//------------------------------------------------------------------------------
void UI_FS_LangageMenuDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LANGAGE+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 227;
	tOsdImgInfo.uwYStart = 570;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

	for(i =0 ; i < 4 ; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_ENGLISH+(i*2)+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 227+(i*96);
		tOsdImgInfo.uwYStart = 260;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_ENGLISH_S+(value*2)+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 227+(value*96);
	tOsdImgInfo.uwYStart = 260;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	

}

void UI_FS_SetTimeMenuDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 216;
	tOsdImgInfo.uwYStart =694;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_BACK2+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 434;
	tOsdImgInfo.uwYStart =679;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_FINISH2+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 434;
	tOsdImgInfo.uwYStart =217;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
	switch(value)
	{
		case 0:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 149;
			tOsdImgInfo.uwYStart =339;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIMEUPAROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167;
			tOsdImgInfo.uwYStart =632;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIMEDOWNAROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 288;
			tOsdImgInfo.uwYStart =632;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =371;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =479;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =513;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =574;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =612;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =646;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
		break;

		case 1:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 149;
			tOsdImgInfo.uwYStart =339;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIMEUPAROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167;
			tOsdImgInfo.uwYStart =497;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIMEDOWNAROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 288;
			tOsdImgInfo.uwYStart =497;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =371;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =479;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =513;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =574;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =612;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =646;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;

		case 2:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 149;
			tOsdImgInfo.uwYStart =339;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIMEUPAROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 167;
			tOsdImgInfo.uwYStart =394;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIMEDOWNAROW, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 288;
			tOsdImgInfo.uwYStart =394;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_AM_S+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =371;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =479;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =513;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =574;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =612;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =646;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;	

		case 3:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 149;
			tOsdImgInfo.uwYStart =339;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
		//-----------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =371;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		//-------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =479;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =513;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =574;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		//-----------------------------------------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =612;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 216;
			tOsdImgInfo.uwYStart =646;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_BACK1+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 434;
			tOsdImgInfo.uwYStart =679;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;	

		case 4:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_FINISH1+ (23*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 434;
			tOsdImgInfo.uwYStart =217;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
		break;			
	}
	OSD_Weight(OSD_WEIGHT_8DIV8);
}

void UI_GetVolData(UI_CamNum_t tCamNum, void *pvTrig)
{
	uint8_t *pvdata = (uint8_t *)pvTrig;

	
	ubGetVoiceTemp = pvdata[0];
	ubGetIR1Temp =  pvdata[1];
	ubGetIR2Temp =  pvdata[2];

	//printf("ubGetVoiceTemp %d \n",ubGetVoiceTemp);
	//printf("ubGetIR1Temp %d \n",ubGetIR1Temp);
	//printf("ubGetIR2Temp %d \n",ubGetIR2Temp);
	
	if((tUI_PuSetting.ubDefualtFlag == FALSE)&&(ubClearOsdFlag == 1))
		UI_VolBarDisplay(ubGetVoiceTemp);
}


uint8_t UI_TempCToF(uint8_t cTemp)
{
	uint8_t fTemp = 0;

	fTemp = cTemp*18/10 + (((cTemp*18%10) >= 5)?1:0) + 32;

	//printf("UI_TempCToF cTemp: %d, fTemp: %d.\r\n", cTemp, fTemp);
	return fTemp;
}

void UI_GetTempData(UI_CamNum_t tCamNum, void *pvTrig) //20180322
{
	uint8_t *pvdata = (uint8_t *)pvTrig;
	
	ubRealTemp = tUI_PuSetting.ubTempunitFlag?pvdata[0]:UI_TempCToF(pvdata[0]);

	if(ubRealTemp > 99)
		ubRealTemp = 99;
	//printf("UI_GetTempData ubRealTemp: %d, ubTempunitFlag: %d. \n",ubRealTemp, tUI_PuSetting.ubTempunitFlag);
	if((tUI_PuSetting.ubDefualtFlag == FALSE)&&(ubClearOsdFlag == 1))
	{
		UI_TempBarDisplay(ubRealTemp);
	}
}

void UI_TempBarDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	if(value > 99)
		value = 99;

	if(value/10)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(value/10), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 920;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 920;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 +(value%10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 904;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_VolBarDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;
	
	if(value != 0)
	{
		for(i = 1; i< 6; i++)
		{
			if(i <= value)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_VOL1, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 832 - 30*(i-1);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_VOL0, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 832 - 30*(i-1);
			}
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
	}
	else
	{
		for(i = 0; i< 5; i++)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_VOL0, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 832 - 30*i;

			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
	}
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_EnableMotor(uint8_t value)
{
	UI_PUReqCmd_t tUI_motorReqCmd;

	printf("UI_EnableMotor value: %d.\n", value);
	tUI_motorReqCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
	tUI_motorReqCmd.ubCmd[UI_TWC_TYPE]	  	= UI_SETTING;
	tUI_motorReqCmd.ubCmd[UI_SETTING_ITEM] 	= UI_MOTOR_SETTING;
	tUI_motorReqCmd.ubCmd[UI_SETTING_DATA] 	= value;
	tUI_motorReqCmd.ubCmd_Len  			  	= 3;
	if(UI_SendRequestToBU(osThreadGetId(), &tUI_motorReqCmd) != rUI_SUCCESS)
	{
		printd(DBG_ErrorLvl, "UI_EnableMotor Fail!\n");
	}
}

void UI_CamvLDCModeCmd(uint8_t value)
{
	UI_PUReqCmd_t tUI_CamvLDCModeCmd;

	printf("UI_CamvLDCModeCmd value: %d.\n", value);
	tUI_CamvLDCModeCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
	tUI_CamvLDCModeCmd.ubCmd[UI_TWC_TYPE]	  	= UI_SETTING;
	tUI_CamvLDCModeCmd.ubCmd[UI_SETTING_ITEM] 	= UI_IMGPROC_SETTING;
	tUI_CamvLDCModeCmd.ubCmd[UI_SETTING_DATA] 	= UI_IMGvLDC_SETTING;
	tUI_CamvLDCModeCmd.ubCmd[UI_SETTING_DATA+1] = value;
	tUI_CamvLDCModeCmd.ubCmd_Len  			  	= 4;
	if(UI_SendRequestToBU(osThreadGetId(), &tUI_CamvLDCModeCmd) != rUI_SUCCESS)
	{
		printd(DBG_ErrorLvl, "UI_CamvLDCModeCmd Fail!\n");
	}
}

void UI_TestCmd(uint8_t Value1, uint8_t Value2)
{
	UI_PUReqCmd_t tUI_TestCmd;

	printf("UI_TestCmd (%d, %d), tCamViewPool[0]: %d.\n", Value1, Value2, tCamViewSel.tCamViewPool[0]);
	tUI_TestCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
	tUI_TestCmd.ubCmd[UI_TWC_TYPE]	  	= UI_SETTING;
	tUI_TestCmd.ubCmd[UI_SETTING_ITEM] 	= UI_TEST_SETTING;
	tUI_TestCmd.ubCmd[UI_SETTING_DATA] 	= Value1;
	tUI_TestCmd.ubCmd[UI_SETTING_DATA+1] = Value2;
	tUI_TestCmd.ubCmd_Len  			  	= 4;
	if(UI_SendRequestToBU(osThreadGetId(), &tUI_TestCmd) != rUI_SUCCESS)
	{
		printd(DBG_ErrorLvl, "tUI_TestCmd Fail!\n");
		printf("tUI_TestCmd Fail!\n");
	}
}

void UI_MotorDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;

	printf("UI_MotorDisplay value: 0x%x.\n", value);

	if((value != MC_UP_DOWN_OFF) && (value != MC_LEFT_RIGHT_OFF))
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_MOVE, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 259;
		tOsdImgInfo.uwYStart = 515;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	switch(value)
	{
		case MC_UP_ON:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_UP, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 191;
			tOsdImgInfo.uwYStart = 576;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor1State = MC_UP_ON;
			break;
			
		case MC_UP_TOP:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_UP_T, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 191;
			tOsdImgInfo.uwYStart = 576;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor1State = MC_UP_TOP;
			break;
			
		case MC_DOWN_ON:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_DOWN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 516;
			tOsdImgInfo.uwYStart = 576;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor1State = MC_DOWN_ON;
			break;
			
		case MC_DOWN_TOP:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_DOWN_T, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 516;
			tOsdImgInfo.uwYStart = 576;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor1State = MC_DOWN_TOP;
			break;

		case MC_UP_DOWN_OFF:
			tOsdImgInfo.uwHSize  = 386;
			tOsdImgInfo.uwVSize  = 389;
			tOsdImgInfo.uwXStart = 191;
			tOsdImgInfo.uwYStart = 447;
			OSD_EraserImg2(&tOsdImgInfo);
			ubMotor1State = MC_UP_DOWN_OFF;
			break;
			
		case MC_LEFT_ON:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_LEFT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 322;
			tOsdImgInfo.uwYStart = 771;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor0State = MC_LEFT_ON;
			break;
			
		case MC_LEFT_TOP:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_LEFT_T, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 322;
			tOsdImgInfo.uwYStart = 771;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor0State = MC_LEFT_TOP;
			break;
		
		case MC_RIGHT_ON:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_RIGHT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 322;
			tOsdImgInfo.uwYStart = 447;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor0State = MC_RIGHT_ON;
			break;
			
		case MC_RIGHT_TOP:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_RIGHT_T, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 322;
			tOsdImgInfo.uwYStart = 447;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubMotor0State = MC_RIGHT_TOP;
			break;
			
		case MC_LEFT_RIGHT_OFF:
			tOsdImgInfo.uwHSize  = 386;
			tOsdImgInfo.uwVSize  = 389;
			tOsdImgInfo.uwXStart = 191;
			tOsdImgInfo.uwYStart = 447;
			OSD_EraserImg2(&tOsdImgInfo);
			ubMotor0State = MC_LEFT_RIGHT_OFF;
			break;
			
		default:
			break;
	}
}

void UI_MotorControl(uint8_t Value)
{
	OSD_IMG_INFO tOsdImgInfo;
			
	if(tUI_PuSetting.ubDefualtFlag == TRUE)
		return;

	if(tUI_State != UI_DISPLAY_STATE)
		return;
	
	if(tUI_SyncAppState != APP_LINK_STATE)
	{
		if(ubMotor0State != MC_LEFT_RIGHT_OFF)
			UI_MotorDisplay(ubMotor0State);
		
		if(ubMotor1State != MC_UP_DOWN_OFF)
			UI_MotorDisplay(ubMotor1State);
		
		printf("UI_MotorControl NOT LINK.\n");
		return;
	}

	if(((ubMotor0State>>4)&&(Value>>4)) || (Value == ubMotor0State))
		return;
	
	if(((ubMotor1State>>4)&&(Value>>4)) || (Value == ubMotor1State))
		return;

	printf("UI_MotorControl Value: (0x%x) #####\n", Value);
	UI_EnableMotor((Value&0x01)?(Value>>4):0);
	UI_MotorDisplay(Value);
}

void UI_MotorStateCheck(void)
{
	#define MC_MAX_CNT	30 //10s /40
	
	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	if(tUI_PuSetting.ubDefualtFlag == TRUE)
		return;

	//printf("### UP_DOWN: (0x%x, %d), LEFT_RIGHT: (0x%x, %d).\n", ubMotor1State, ubMC1OnCount, ubMotor0State, ubMC0OnCount);
	if(ubMotor1State == MC_UP_ON)
	{
		ubMC1OnCount++;
		if(ubMC1OnCount >= MC_MAX_CNT)
		{
			UI_MotorDisplay(MC_UP_TOP);
			UI_EnableMotor(0);
		}
		else
		{
			UI_EnableMotor(1);
		}
	}
	else if(ubMotor1State == MC_UP_TOP)
	{
		ubMC1OnCount = 0;
	}
	else if(ubMotor1State == MC_DOWN_ON)
	{
		ubMC1OnCount++;
		if(ubMC1OnCount >= MC_MAX_CNT)
		{
			UI_MotorDisplay(MC_DOWN_TOP);
			UI_EnableMotor(0);
		}
		else
		{
			UI_EnableMotor(2);
		}
	}
	else if(ubMotor1State == MC_DOWN_TOP)
	{
		ubMC1OnCount = 0;
	}
	else if(ubMotor1State == MC_UP_DOWN_OFF)
	{
		ubMC1OnCount = 0;
	}

	
	if(ubMotor0State == MC_LEFT_ON)
	{
		ubMC0OnCount++;
		if(ubMC0OnCount >= MC_MAX_CNT)
		{
			UI_MotorDisplay(MC_LEFT_TOP);
			UI_EnableMotor(0);
		}
		else
		{
			UI_EnableMotor(3);
		}
	}
	else if(ubMotor0State == MC_LEFT_TOP)
	{
		ubMC0OnCount = 0;
	}
	else if(ubMotor0State == MC_RIGHT_ON)
	{
		ubMC0OnCount++;
		if(ubMC0OnCount >= MC_MAX_CNT)
		{
			UI_MotorDisplay(MC_RIGHT_TOP);
			UI_EnableMotor(0);
		}
		else
		{
			UI_EnableMotor(4);
		}
	}
	else if(ubMotor0State == MC_RIGHT_TOP)
	{
		ubMC0OnCount = 0;
	}
	else if(ubMotor0State == MC_LEFT_RIGHT_OFF)
	{
		ubMC0OnCount = 0;
	}
}

void UI_GetBatLevel(void)
{
	ubGetBatValue  = uwSADC_GetReport(SADC_CH3);
	printf("UI_GetBatLevel ubGetBatValue: 0x%x.\n", ubGetBatValue);
	#if UI_TEST_MODE
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubGetBatValue/1000), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 410;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubGetBatValue/100), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 390;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubGetBatValue/10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 370;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubGetBatValue%10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 350;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	#endif
}

void UI_PTNDisplay(uint8_t value)
{
	OSD_IMG_INFO tOsdImgInfo;

	if(value == 0)
		ubDisplayPTN = 0;
	
	if(ubDisplayPTN == 1)
		return;

	if(tUI_SyncAppState != APP_LINK_STATE)
		return;

	UI_EnableMotor(value);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_MOVE, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 259;
	tOsdImgInfo.uwYStart = 515;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	switch(value)
	{
		case 0:  //clean
			tOsdImgInfo.uwHSize  = 386;
			tOsdImgInfo.uwVSize  = 389;
			tOsdImgInfo.uwXStart = 191;
			tOsdImgInfo.uwYStart = 447;
			OSD_EraserImg2(&tOsdImgInfo);	
		break;

		case 1:   // up
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_UP, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 191;
			tOsdImgInfo.uwYStart = 576;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubDisplayPTN = 1;
		break;

		case 2:   //down
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_DOWN, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 516;
			tOsdImgInfo.uwYStart = 576;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubDisplayPTN = 1;
		break;

		case 3:   //left
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_LEFT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 322;
			tOsdImgInfo.uwYStart = 771;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubDisplayPTN = 1;
		break;

		case 4:   //right
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_PTZ_RIGHT, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 322;
			tOsdImgInfo.uwYStart = 447;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubDisplayPTN = 1;
		break;
	}
	
}

//------------------------------------------------------------------------------
#define CAMS_SET_ITEM_OFFSET	50

void UI_DrawPairingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMHL_ITEM, OSD2IMG_DELCAMNOR_ITEM};
	uint8_t i;
	for(i = 0; i < PAIRITEM_MAX; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);		
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIR_SUBMENUICON, 1, &tOsdImgInfo);	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem)
{
	static UI_DrawSubMenuFuncPtr_t DrawSubMenuFunc[MENUITEM_MAX] = 
	{
		UI_DrawBrightnessSubMenuPage,
		UI_DrawAutoLcdSubMenuPage,
		UI_DrawAlarmSubMenuPage,
		UI_DrawTimeSubMenuPage,
		UI_DrawZoomSubMenuPage,
		UI_DrawCamsSubMenuPage,
		UI_DrawSettingSubMenuPage
	};
//	OSD_IMG_INFO tOsdImgInfo;

	if(NULL == DrawSubMenuFunc[MenuItem].pvFuncPtr)
		return;
	/*
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	*/
	DrawSubMenuFunc[MenuItem].pvFuncPtr();
	tUI_State = UI_SUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_SubMenu(UI_ArrowKey_t tArrowKey)
{	
	static UI_MenuFuncPtr_t SubMenuFunc[MENUITEM_MAX] =
	{
		UI_BrightnessSubMenuPage,
		UI_AutoLcdSubMenuPage,
		UI_AlarmSubMenuPage,
		UI_TimeSubMenuPage,
		UI_ZoomSubMenuPage,
		UI_CameraSettingSubMenuPage,
		UI_SettingSubMenuPage,
		
	};
	if(SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}

//------------------------------------------------------------------------------
void UI_SubSubMenu(UI_ArrowKey_t tArrowKey)
{
	static UI_MenuFuncPtr_t SubSubMenuFunc[MENUITEM_MAX] =
	{
		NULL,
		NULL,
		UI_AlarmSubSubMenuPage,
		NULL,
		NULL,
		UI_CameraSettingSubSubMenuPage,
		UI_SettingSubSubMenuPage
	};
	if(SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_SubSubSubMenu(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_MenuItem.ubItemIdx)
	{
		//case PAIRING_ITEM:
		//	UI_PairingSubSubSubMenuPage(tArrowKey);
		//	break;
		case ALARM_ITEM:
				UI_AlarmSubSubSubMenuPage(tArrowKey);
			break;

		case CAMERAS_ITEM:
				UI_CamSubSubSubMenuPage(tArrowKey);
			break;
		
		case SETTING_ITEM:			
				UI_NightModeSubSubSubMenuPage(tArrowKey);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		//UI_DrawSubMenuPage(PAIRING_ITEM);
		return;
	}
	//tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[PAIRING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMNOR_ITEM, OSD2IMG_DELCAMNOR_ITEM};
			//uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemPreIdx = 0;
			//uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
			uint8_t ubSubMenuItemIdx  = 0;
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			break;
		}
		case DRAW_MENUPAGE:
		{
			//! Draw sub sub menu page
			OSD_IMG_INFO tOsdImgInfo, tCamRdyOsdImgInfo = {0};
			uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
			uint8_t ubBuNum = tUI_PuSetting.ubTotalBuNum, i, ubUI_FirstHL = FALSE;
			//uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
			uint8_t ubSubMenuItemIdx = 0;
			uint16_t uwIconOffset = (ubSubMenuItemIdx == DELCAM_ITEM)?80:0;
			uint16_t uwRDY_XStart = 0, uwOsdImgIdx;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tCamRdyOsdImgInfo);
			uwRDY_XStart = tCamRdyOsdImgInfo.uwXStart;
			if(ubSubMenuItemIdx == PAIRCAM_ITEM)
			{
				uint16_t uwItemOffset = ubBuNum * 55;				
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
				OSD_EraserImg2(&tOsdImgInfo);
				tOsdImgInfo.uwXStart += uwItemOffset;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				for(i = CAM1; i < ubBuNum; i++)
				{
					if(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
					{
						if(FALSE == ubUI_FirstHL)
						{
							uwSubMenuItemOsdImg[i] += UI_ICON_HIGHLIGHT;
							ubUI_FirstHL = TRUE;
						}
						tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					}
					else
					{
						uwOsdImgIdx = uwSubMenuItemOsdImg[i]+UI_ICON_READY;
						if(FALSE == ubUI_FirstHL)
						{
							uwOsdImgIdx  = uwSubMenuItemOsdImg[i]+UI_ICON_HIGHLIGHT;
							ubUI_FirstHL = TRUE;
						}
						tCamRdyOsdImgInfo.uwXStart = uwRDY_XStart + (i*55);
						tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);
						tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
					}
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			if(ubSubMenuItemIdx == DELCAM_ITEM)
			{
				for(i = CAM1; i < ubBuNum; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubMenuItemOsdImg[i]+((i == CAM1)?UI_ICON_HIGHLIGHT:(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)?UI_ICON_NORMAL:UI_ICON_READY)), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart += uwIconOffset;
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
					{
						tCamRdyOsdImgInfo.uwXStart = uwIconOffset + uwRDY_XStart + (i*55);
						tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);						
					}
				}
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIROPTMASK_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingUpdateSubSubMenuItemIndex(UI_SubMenuItem_t *ptSubSubMenuItem, uint8_t *Update_Flag)
{
	ptSubSubMenuItem->ubFirstItem 			 = CAM1;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx = CAM1;
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_PairingDrawSubSbuMenuItem(UI_PairSubMenuItemList_t *ptSubMenuItem, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
	uint16_t uwIconOffset = (*ptSubMenuItem == DELCAM_ITEM)?80:0;
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx    = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptSubMenuItem)
	{
		case PAIRCAM_ITEM:
		case DELCAM_ITEM:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx]+((tUI_CamStatus[ubSubSubMenuItemPreIdx].ulCAM_ID != INVALID_ID)?UI_ICON_READY:UI_ICON_NORMAL)), 1, &tOsdImgInfo);								
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT, 1, &tOsdImgInfo);											
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_PairSubSubMenuItem_t tPairSubSubMenuItem =
	{
		{
		   { 0, CAM_4T, { 0, 0 } },
		   { 0, CAM_4T, { 0, 0 } },
		},
	};
	static uint8_t ubUI_PairStsUpdateFlag = FALSE;
	//UI_PairSubMenuItemList_t tSubMenuItem = (UI_PairSubMenuItemList_t)tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_PairSubMenuItemList_t tSubMenuItem = 0;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo;

	if((FALSE == ubUI_PairStsUpdateFlag) && (tSubMenuItem == PAIRCAM_ITEM))
		UI_PairingUpdateSubSubMenuItemIndex(&tPairSubSubMenuItem.tPairS[tSubMenuItem], &ubUI_PairStsUpdateFlag);
	tPairSubSubMenuItem.tPairS[tSubMenuItem].ubItemCount = tUI_PuSetting.ubTotalBuNum;
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_PairingDrawSubSbuMenuItem(&tSubMenuItem, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
			break;
		case EXECUTE_MENUFUNC:
		{
			if(tSubMenuItem == PAIRCAM_ITEM)
			{
				if(tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)
				{
					APP_EventMsg_t tUI_PairMessage = {0};

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRINGSINGLE_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
					tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
					tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM1;
					tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
					UI_SendMessageToAPP(&tUI_PairMessage);
					tPairInfo.ubDrawFlag 			 = TRUE;
					UI_DisableScanMode();
					tUI_State = UI_PAIRING_STATE;
				}
				else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
				{
					tPairInfo.tDispLocation = DISP_LEFT;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLEFTWINDOW_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
					tUI_State = UI_SUBSUBSUBMENU_STATE;
				}
				else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
				{
					tPairInfo.tDispLocation = DISP_UPPER_LEFT;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
					tUI_State = UI_SUBSUBSUBMENU_STATE;
				}
			}
			if(tSubMenuItem == DELCAM_ITEM)
			{
				APP_EventMsg_t tUI_UnindBuMsg = {0};
				OSD_IMG_INFO tDelOsdInfo;
				uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1HL_ICON, OSD2IMG_PAIRCAM2HL_ICON, OSD2IMG_PAIRCAM3HL_ICON, OSD2IMG_PAIRCAM4HL_ICON};
				uint16_t uwIconOffset = 80;
				UI_CamNum_t tUI_DelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;

				if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
					break;
				tUI_CamStatus[tUI_DelCam].ulCAM_ID 	= INVALID_ID;
				tUI_CamStatus[tUI_DelCam].tCamConnSts = CAM_OFFLINE;
				tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
				tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == tUI_DelCam)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[tUI_DelCam], 1, &tOsdImgInfo);
				tDelOsdInfo.uwHSize  = 50;
				tDelOsdInfo.uwVSize  = 80;
				tDelOsdInfo.uwXStart = tOsdImgInfo.uwXStart + uwIconOffset;
				tDelOsdInfo.uwYStart = tOsdImgInfo.uwYStart - 80;
				OSD_EraserImg2(&tDelOsdInfo);
				tOsdImgInfo.uwXStart += uwIconOffset;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				UI_ResetDevSetting(tUI_DelCam);
				UI_UpdateDevStatusInfo();
				tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_BU_EVENT;
				tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
				tUI_UnindBuMsg.ubAPP_Message[1] = tUI_DelCam;
				UI_SendMessageToAPP(&tUI_UnindBuMsg);
			}
			break;
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON, 1, &tOsdImgInfo);
			if(tSubMenuItem == PAIRCAM_ITEM)
			{
				tOsdImgInfo.uwHSize = 335;
				tOsdImgInfo.uwVSize = 450;
				tOsdImgInfo.uwYStart -= 80;				
				OSD_EraserImg2(&tOsdImgInfo);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);			
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			if(tSubMenuItem == DELCAM_ITEM)
			{
				tOsdImgInfo.uwHSize = 335;
				tOsdImgInfo.uwVSize = 330;
				tOsdImgInfo.uwXStart += 80;
				tOsdImgInfo.uwYStart -= 80;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			memset(&tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
			ubUI_PairStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_UPPER_RIGHT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_LOWER_LEFT)?DISP_UPPER_LEFT:DISP_UPPER_RIGHT;
				break;
			}
			return;
		case DOWN_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_LOWER_LEFT:DISP_LOWER_RIGHT;
				break;
			}
			return;
		case LEFT_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_LEFT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_RIGHT)?DISP_UPPER_LEFT:DISP_LOWER_LEFT;
			}
			else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
			{
				if(tPairInfo.tDispLocation == DISP_LEFT)
					return;
				tPairInfo.tDispLocation = DISP_LEFT;
			}
			break;
		case RIGHT_ARROW:
			if(tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
					return;
				tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_UPPER_RIGHT:DISP_LOWER_RIGHT;
			}
			else if(tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
			{
				if(tPairInfo.tDispLocation == DISP_RIGHT)
					return;
				tPairInfo.tDispLocation = DISP_RIGHT;
			}
			break;
		case ENTER_ARROW:
		{
			APP_EventMsg_t tUI_PairMessage = {0};
			uint16_t uwIconXoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?88:0:
									 (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?40:0;
			uint16_t uwIconYoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?130:0:
									 (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?(tPairInfo.tDispLocation == DISP_RIGHT)?130:0:0;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRINGMULTI_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwIconXoffset;
			tOsdImgInfo.uwYStart -= uwIconYoffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
			tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam;
			tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation;
			UI_SendMessageToAPP(&tUI_PairMessage);
			tPairInfo.ubDrawFlag 			 = TRUE;
			UI_DisableScanMode();
			tUI_State = UI_PAIRING_STATE;
			return;
		}
		case EXIT_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);			
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_SUBSUBMENU_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON+tPairInfo.tDispLocation, 1, &tOsdImgInfo);	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

//------------------------------------------------------------------------------
void UI_RecordSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_RecordUpdateSubSubMenuItemIndex(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	ptSubSubMenuItem = ptSubSubMenuItem;
	tSubMenuItem = tSubMenuItem;
	Update_Flag = Update_Flag;
}
//------------------------------------------------------------------------------
void UI_RecordDrawSubSubMenuItem(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem)
{
	ptSubSubMenuItem = ptSubSubMenuItem;
	tSubMenuItem =tSubMenuItem;
}
//------------------------------------------------------------------------------
void UI_RecordSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_PhotoSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_PhotoUpdateSubSubMenuItemIndex(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	ptSubSubMenuItem =ptSubSubMenuItem;
	tSubMenuItem = tSubMenuItem;
	Update_Flag = Update_Flag;
}
//------------------------------------------------------------------------------
void UI_PhotoDrawSubSubMenuItem(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem)
{
	ptSubSubMenuItem =ptSubSubMenuItem;
	tSubMenuItem = tSubMenuItem;
}
//------------------------------------------------------------------------------
void UI_PhotoSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	tArrowKey = tArrowKey;
}
//------------------------------------------------------------------------------
void UI_PowerSaveSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_DrawSysDateTime(UI_CalendarItem_t tShowItem, UI_IconType_t tIconType, RTC_Calendar_t *ptSysCalendar)
{
	static OSD_IMG_INFO tNumOsdImgArrayInfo[22] = {0};
	static uint8_t ubUI_RdNumOsdImgFlag = FALSE;
	uint8_t ubDateOffset[2] = {0}, i;

	if(FALSE == ubUI_RdNumOsdImgFlag)
	{
		uint16_t uwSubSubMenuItemOsdImg[22] = {OSD2IMG_NUMBER0NOR_ICON, OSD2IMG_NUMBER0HL_ICON,
											   OSD2IMG_NUMBER1NOR_ICON, OSD2IMG_NUMBER1HL_ICON,
											   OSD2IMG_NUMBER2NOR_ICON, OSD2IMG_NUMBER2HL_ICON,
											   OSD2IMG_NUMBER3NOR_ICON, OSD2IMG_NUMBER3HL_ICON,
											   OSD2IMG_NUMBER4NOR_ICON, OSD2IMG_NUMBER4HL_ICON,
											   OSD2IMG_NUMBER5NOR_ICON, OSD2IMG_NUMBER5HL_ICON,
											   OSD2IMG_NUMBER6NOR_ICON, OSD2IMG_NUMBER6HL_ICON,
											   OSD2IMG_NUMBER7NOR_ICON, OSD2IMG_NUMBER7HL_ICON,
											   OSD2IMG_NUMBER8NOR_ICON, OSD2IMG_NUMBER8HL_ICON,
											   OSD2IMG_NUMBER9NOR_ICON, OSD2IMG_NUMBER9HL_ICON,
											   OSD2IMG_COLONNOR_ICON, OSD2IMG_DIVISIONNOR_ICON};
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[21], 1, &tNumOsdImgArrayInfo[21]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[0], 21, &tNumOsdImgArrayInfo[0]);
		ubUI_RdNumOsdImgFlag = TRUE;
	}
	switch(tShowItem)
	{
		case ALLCALE_ITEM:
		case YEAR_ITEM:
			tNumOsdImgArrayInfo[4+tIconType].uwXStart = 170;
			tNumOsdImgArrayInfo[4+tIconType].uwYStart = 390;
			tOSD_Img2(&tNumOsdImgArrayInfo[4+tIconType], OSD_QUEUE);
			tNumOsdImgArrayInfo[tIconType].uwXStart = 170;
			tNumOsdImgArrayInfo[tIconType].uwYStart = 360;
			tOSD_Img2(&tNumOsdImgArrayInfo[tIconType], OSD_QUEUE);
			ubDateOffset[0] = (ptSysCalendar->uwYear - 2000) / 10;
			ubDateOffset[1] = (ptSysCalendar->uwYear - 2000) - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 330 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case MONTH_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubMonth / 10;
			ubDateOffset[1] = ptSysCalendar->ubMonth - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 248 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case DATE_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubDate / 10;
			ubDateOffset[1] = ptSysCalendar->ubDate - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 166 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case HOUR_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubHour / 10;
			ubDateOffset[1] = ptSysCalendar->ubHour - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 390 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case MIN_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubMin / 10;
			ubDateOffset[1] = ptSysCalendar->ubMin - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 306 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case SEC_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubSec / 10;
			ubDateOffset[1] = ptSysCalendar->ubSec - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 222 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			break;
		default:
			return;
	}
	if(tShowItem == ALLCALE_ITEM)
	{
		tNumOsdImgArrayInfo[21].uwXStart = 170;
		tNumOsdImgArrayInfo[21].uwYStart = 278;
		tOSD_Img2(&tNumOsdImgArrayInfo[21], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[21].uwXStart = 170;
		tNumOsdImgArrayInfo[21].uwYStart = 196;
		tOSD_Img2(&tNumOsdImgArrayInfo[21], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[20].uwXStart = 220;
		tNumOsdImgArrayInfo[20].uwYStart = 336;
		tOSD_Img2(&tNumOsdImgArrayInfo[20], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[20].uwXStart = 220;
		tNumOsdImgArrayInfo[20].uwYStart = 252;
		tOSD_Img2(&tNumOsdImgArrayInfo[20], OSD_UPDATE);
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_SettingUpdateSubSubMenuItemIndex(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	switch(*tSubMenuItem)
	{

	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_SettingDrawSubSubMenuItem(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem)
{
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{

	}
}

//------------------------------------------------------------------------------
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubSubItem_t tSysDtSubSubSubItem = {YEAR_ITEM, YEAR_ITEM};
	uint16_t *pDT_YearNum = (uint16_t *)&tUI_PuSetting.tSysCalendar.uwYear;
	uint8_t *pDT_Num[ALLCALE_ITEM] = {NULL, (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubMonth),
									  (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubDate), (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubHour),
									  (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubMin), (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubSec)};
	uint16_t uwDT_MaxYearNum = 2098, uwDT_MinYearNum = 2015;
	uint8_t ubDT_MaxNum[ALLCALE_ITEM] = {0, 12, 31, 23, 59, 59};
	uint8_t ubDT_MinNum[ALLCALE_ITEM] = {0,  1,  1,  0,  0,  0};
	uint8_t ubDT_UpdateItem = tSysDtSubSubSubItem.ubItemIdx;
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubDT_UpdateItem == YEAR_ITEM)
			{
				if((NULL == pDT_YearNum) || (*pDT_YearNum >= uwDT_MaxYearNum))
					return;
				(*pDT_YearNum)++;
			}
			else
			{
				if((NULL == pDT_Num[ubDT_UpdateItem]) || (*pDT_Num[ubDT_UpdateItem] >= ubDT_MaxNum[ubDT_UpdateItem]))
					return;
				(*pDT_Num[ubDT_UpdateItem])++;
			}
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
			return;
		case DOWN_ARROW:
			if(ubDT_UpdateItem == YEAR_ITEM)
			{
				if((NULL == pDT_YearNum) || (*pDT_YearNum == uwDT_MinYearNum))
					return;
				(*pDT_YearNum)--;
			}
			else
			{
				if((NULL == pDT_Num[ubDT_UpdateItem]) || (*pDT_Num[ubDT_UpdateItem] == ubDT_MinNum[ubDT_UpdateItem]))
					return;
				(*pDT_Num[ubDT_UpdateItem])--;
			}
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
			return;
		case ENTER_ARROW:
		case LEFT_ARROW:
			if((tSysDtSubSubSubItem.ubItemIdx == YEAR_ITEM) || (ENTER_ARROW == tArrowKey))
			{
				UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
				tSysDtSubSubSubItem.ubItemPreIdx = YEAR_ITEM;
				tSysDtSubSubSubItem.ubItemIdx    = YEAR_ITEM;
				if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
					printd(DBG_ErrorLvl, "Calendar base setting fail !\n");
				else
					RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
				UI_UpdateDevStatusInfo();
				tUI_State = UI_SUBSUBMENU_STATE;
				return;
			}
			tSysDtSubSubSubItem.ubItemPreIdx = tSysDtSubSubSubItem.ubItemIdx;
			tSysDtSubSubSubItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if(tSysDtSubSubSubItem.ubItemIdx == SEC_ITEM)
				return;
			tSysDtSubSubSubItem.ubItemPreIdx = tSysDtSubSubSubItem.ubItemIdx;
			tSysDtSubSubSubItem.ubItemIdx++;
			break;
		default:
			return;
	}
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx,    UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemPreIdx, UI_ICON_NORMAL,    (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
}
//------------------------------------------------------------------------------
void UI_ResetSubMenuInfo(void)
{
	uint8_t i;

	for(i = 0; i < MENUITEM_MAX; i++)
		memset(&tUI_SubMenuItem[i].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
}
//------------------------------------------------------------------------------
void UI_ResetSubSubMenuInfo(void)
{
}
//------------------------------------------------------------------------------
UI_CamNum_t UI_ChangeSelectCamNum4UiMenu(UI_CamNum_t *tCurrentCamNum, UI_ArrowKey_t *ptArrowKey)
{
	UI_CamNum_t tChangeCamNum = NO_CAM;
	UI_CamNum_t tCamNum 	  = (UI_CamNum_t)*tCurrentCamNum;
	UI_CamNum_t tMaxCamNum 	  = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?CAM_4T:
	                            (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?CAM_2T:CAM_4T;

	if(tCamNum > ((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?QUAL_TYPE_ITEM:tMaxCamNum))
		return NO_CAM;
	switch(*ptArrowKey)
	{
		case LEFT_ARROW:
			if(tCamNum == CAM1)
				return NO_CAM;
			if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
			{
				if((tUI_PuSetting.ubPairedBuNum > 1) && (tCamNum > DUAL_TYPE_ITEM))
					return (UI_CamNum_t)(tCamNum-1);
				if(tCamNum == DUAL_TYPE_ITEM)
					tCamNum = CAM_4T;
			}
			for(;tCamNum > CAM1; tCamNum--)
			{
				if((tUI_CamStatus[tCamNum-1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[tCamNum-1].tCamConnSts == CAM_ONLINE))
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum-1);
					break;
				}
			}
			break;
		case RIGHT_ARROW:
			if((tCamNum+1) >= tMaxCamNum)
			{
				if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
					return ((tCamNum+1) > QUAL_TYPE_ITEM)?NO_CAM:(tUI_PuSetting.ubPairedBuNum > 1)?((UI_CamNum_t)(tCamNum+1)):(UI_CamNum_t)QUAL_TYPE_ITEM;
				else
					return tMaxCamNum;
			}
			tChangeCamNum = tMaxCamNum;
			for(;tCamNum < tMaxCamNum; tCamNum++)
			{
				if((tUI_CamStatus[tCamNum+1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[tCamNum+1].tCamConnSts == CAM_ONLINE))
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum+1);
					break;
				}
			}
			if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tChangeCamNum == tMaxCamNum))
				tChangeCamNum = (tUI_PuSetting.ubPairedBuNum > 1)?(UI_CamNum_t)DUAL_TYPE_ITEM:(UI_CamNum_t)QUAL_TYPE_ITEM;
			break;
		default:
			break;
	}
	return tChangeCamNum;
}
//------------------------------------------------------------------------------
void UI_DrawHLandNormalIcon(uint16_t uwNormalOsdImgIdx, uint16_t uwHighLigthOsdImgIdx)
{
	OSD_IMG_INFO tOsdImgInfo;

	//! Draw normal item
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNormalOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	//! Draw highlight item
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwHighLigthOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu)
{
	uint8_t ubCurrentSubMenuItemIdx = ptSubMenu->tSubMenuInfo.ubItemIdx;
	uint8_t ubNextSubMenuItemIdx = 0;

	switch(*ptArrowKey)
	{
		case UP_ARROW:
			if(ubCurrentSubMenuItemIdx <= ptSubMenu->ubFirstItem)
				return NOT_ACTION;
			ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx - 1;
			break;
		case DOWN_ARROW:
			if((ubCurrentSubMenuItemIdx+1) >= ptSubMenu->ubItemCount)
				return NOT_ACTION;
			ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx + 1;
			break;

		case RIGHT_ARROW:			
		case ENTER_ARROW:
			return DRAW_MENUPAGE;
			
		case LEFT_ARROW:			
		case EXIT_ARROW:
			return EXIT_MENUFUNC;
			
		default:
			return NOT_ACTION;
	}
	ptSubMenu->tSubMenuInfo.ubItemPreIdx = ubCurrentSubMenuItemIdx;
	ptSubMenu->tSubMenuInfo.ubItemIdx    = ubNextSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	uint8_t ubCurrentSubSubMenuItemIdx = 0;
	uint8_t ubNextSubSubMenuItemIdx = 0;

	ubCurrentSubSubMenuItemIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptArrowKey)
	{
		case UP_ARROW:
			if(ubCurrentSubSubMenuItemIdx <= ptSubSubMenuItem->ubFirstItem)
				return NOT_ACTION;
			ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx - 1;
			break;
		case DOWN_ARROW:
			if((ubCurrentSubSubMenuItemIdx+1) >= ptSubSubMenuItem->ubItemCount)
				return NOT_ACTION;
			ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx + 1;
			break;
		case ENTER_ARROW:
			return EXECUTE_MENUFUNC;
		case LEFT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx = ubCurrentSubSubMenuItemIdx;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx    = ubNextSubSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
void UI_UpdateBriLvlIcon(void)
{
	/*
	if(!tUI_PuSetting.BriLvL.ubBL_UpdateCnt)
		return;
	if(!(--tUI_PuSetting.BriLvL.ubBL_UpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
	}
	*/
}
//------------------------------------------------------------------------------
void UI_UpdateVolLvlIcon(void)
{
	if(!tUI_PuSetting.VolLvL.ubVOL_UpdateCnt)
		return;
	
	if(!(--tUI_PuSetting.VolLvL.ubVOL_UpdateCnt))
	{
		if(tUI_State == UI_DISPLAY_STATE)
		{
			OSD_IMG_INFO tOsdImgInfo;

			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 1119;
			tOsdImgInfo.uwHSize  = 672;
			tOsdImgInfo.uwVSize  = 161;
			OSD_EraserImg2(&tOsdImgInfo);
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateOsdImg4MultiView(UI_CamViewType_t tView_Type, OSD_RESULT(*pOsdImgFuncPtr)(OSD_IMG_INFO *, OSD_UPDATE_TYP), OSD_IMG_INFO *pOsdImgInfo)
{
	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	uint16_t uwOriXStart = pOsdImgInfo->uwXStart, uwOriYStart = pOsdImgInfo->uwYStart;

	pOsdImgInfo->uwYStart -= (ulLcd_VSize/2);
	if(QUAL_VIEW == tView_Type)
	{
		pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
		pOsdImgInfo->uwXStart += (ulLcd_HSize/2);
		pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
		pOsdImgInfo->uwYStart = uwOriYStart;
	}
	pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
	pOsdImgInfo->uwXStart = uwOriXStart;
	pOsdImgInfo->uwYStart = uwOriYStart;
	pOsdImgFuncPtr(pOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPUStatusIcon(void)
{
	static OSD_IMG_INFO tPuOsdImgInfo[2];
	static uint8_t ubUI_RdPuOsdImgFlag = FALSE;
	OSD_IMG_INFO tOsdImgInfo;
	uint32_t ulQualMask_YStart = uwLCD_GetLcdVoSize()/2;
	uint32_t ulBatLvL_YStart = 0x48D;
	UI_CamViewType_t tUI_CamViewType = (TRUE == tUI_PuSetting.ubDualModeEn)?DUAL_VIEW:tCamViewSel.tCamViewType;
	
	if(FALSE == ubUI_RdPuOsdImgFlag)
	{
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PU_STSICON, 1, &tPuOsdImgInfo[0]);
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BATLVL4_STSICON, 1, &tPuOsdImgInfo[1]);
		
		ubUI_RdPuOsdImgFlag = TRUE;
	}
	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
			tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
			if(TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
			{
				//tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
				break;
			}
			//tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_HDSTATUSMASK, 1, &tOsdImgInfo);
			//tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			//tOSD_Img2(&tPuOsdImgInfo[0], OSD_QUEUE);
			//tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
			//OSD_Weight(OSD_WEIGHT_8DIV8);
			
			//tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
			
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		case DUAL_VIEW:
		case QUAL_VIEW:
			if(TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
			{
				tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
				UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_QUALSTATUSMASK, 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart = ulQualMask_YStart;
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img1, &tOsdImgInfo);
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[0]);
			tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateBuStatusOsdImg(OSD_IMG_INFO *pOsdImgInfo, OSD_UPDATE_TYP tUpdateMode, UI_OsdImgFnType_t tOsdImgFnType, UI_DisplayLocation_t tDispLoc)
{
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
//	uint16_t uwXOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
//	                         [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
//							 [DISP_LEFT] 	   = 0,				  [DISP_RIGHT] 		 = 0};
//	uint16_t uwYOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
//	                         [DISP_LOWER_LEFT] = 0, 			  [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
//							 [DISP_LEFT] 	   = 0, 		      [DISP_RIGHT] 		 = (ulLcd_VSize/2)};
	uint16_t uwXStart = 0, uwYStart = 0;

	uwXStart = pOsdImgInfo->uwXStart;
	uwYStart = pOsdImgInfo->uwYStart;
	//pOsdImgInfo->uwXStart += uwXOffset[tDispLoc];
	//pOsdImgInfo->uwYStart -= uwYOffset[tDispLoc];
	if(UI_OsdUpdate == tOsdImgFnType)
		tOSD_Img2(pOsdImgInfo, tUpdateMode);
	//else if(UI_OsdErase == tOsdImgFnType)
		OSD_EraserImg2(pOsdImgInfo);
	
	//pOsdImgInfo->uwXStart = uwXStart;
	//pOsdImgInfo->uwYStart = uwYStart;
}
//------------------------------------------------------------------------------
void UI_ClearBuConnectStatusFlag(void)
{
	UI_CamNum_t tCamNum;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(pUI_BuConnectFlag[0])
			*(pUI_BuConnectFlag[0]+tCamNum) = FALSE;
		if(pUI_BuConnectFlag[1])
			*(pUI_BuConnectFlag[1]+tCamNum) = FALSE;
	}
}
void UI_UpdateBarIcon_Part1(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwAntLvLIdx = ANT_NOSIGNAL, uwBatLvLIdx = BAT_LVL0;

	UI_CamNum_t tSelCamNum = tCamViewSel.tCamViewPool[0]; 
			
	uwAntLvLIdx = 6 - tUI_CamStatus[tSelCamNum].tCamAntLvl;
	uwBatLvLIdx = 4;// BAT_LVL4;	

	if(ubEnterTimeMenuFlag == 0)		
		UI_TimeShowSystemTime(1);
			
	//! Camera Number
	//UI_UpdateBuStatusOsdImg(&tCamNumOsdImgInfo[tCamNum], OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);	 	

	/*
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_2, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 920;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_5, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 904;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	*/

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_ANT0+uwAntLvLIdx, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 1190;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_BAT0+uwBatLvLIdx, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 10;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
}

void UI_UpdateBarIcon_Part2(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	UI_CamNum_t tSelCamNum; 

	tSelCamNum = tCamViewSel.tCamViewPool[0] ;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_CAM1+tSelCamNum, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 1070;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_TEMPC+(!tUI_PuSetting.ubTempunitFlag), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 874;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

	if((tUI_PuSetting.NightmodeFlag >> tSelCamNum) & (0x01) == 1)	
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NIGHT, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 1130;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 1130;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	if((tUI_PuSetting.ubScanModeEn == TRUE) && (tUI_PuSetting.ubScanTime > 0))
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_SCAN, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 1010;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 1010;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

	if(ubAlarmIconFlag == 1)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_ALARM, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 950;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_BLANK1 , 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 950;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

}
//------------------------------------------------------------------------------
void UI_RedrawBuConnectStatusIcon(UI_CamNum_t tCamNum)
{
	static OSD_IMG_INFO tCamNumOsdImgInfo[CAM_4T];
	//static OSD_IMG_INFO tAdoSrcOsdImgInfo, tMarkAdoOsdImgInfo;
	static OSD_IMG_INFO tCamAntLvlOsdImgInfo[7], tCamBatLvlOsdImgInfo[6];
	static uint8_t ubUI_RdBuStsOsdImgFlag 	    = FALSE;
	static uint8_t ubUI_BuOnlineFlag[CAM_4T]    = {FALSE, FALSE, FALSE, FALSE};
	static uint8_t ubUI_BuOfflineFlag[CAM_4T]   = {FALSE, FALSE, FALSE, FALSE};
	static uint8_t ubUI_NoSignalOsdFlag[CAM_4T] = {FALSE, FALSE, FALSE, FALSE};
	uint16_t uwCamNumOsdIdx[CAM_4T] = {OSD2IMG_BAR_CAM1, OSD2IMG_BAR_CAM2,
									   OSD2IMG_BAR_CAM3, OSD2IMG_BAR_CAM4};
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;
	uint16_t uwAntLvLIdx = ANT_NOSIGNAL, uwBatLvLIdx = BAT_LVL0;



	if(FALSE == ubUI_RdBuStsOsdImgFlag)
	{
		UI_CamNum_t tSelCamNum;

		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADOSRC_STSICON, 1, &tAdoSrcOsdImgInfo);
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STSMASK_STSICON, 1, &tMarkAdoOsdImgInfo);

		/*
		for(tSelCamNum = CAM1; tSelCamNum < tUI_PuSetting.ubTotalBuNum; tSelCamNum++)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCamNumOsdIdx[tSelCamNum], 1, &tCamNumOsdImgInfo[tSelCamNum]);
			tCamNumOsdImgInfo[tSelCamNum].uwXStart = 0;
			tCamNumOsdImgInfo[tSelCamNum].uwYStart = 1070;	
		}

		//printf("osd 1\n");
	
		*/
		
		pUI_BuConnectFlag[0] = &ubUI_BuOnlineFlag[0];
		pUI_BuConnectFlag[1] = &ubUI_BuOfflineFlag[0];
		ubUI_RdBuStsOsdImgFlag = TRUE;
	}
	if(TRUE == tUI_PuSetting.ubDualModeEn)
		tUI_DispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
	else
		tUI_DispLoc = (tCamViewSel.tCamViewType == SINGLE_VIEW)?DISP_UPPER_RIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;
	if((TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) ||
       (UI_MAINMENU_STATE == tUI_State))
	{
		ubUI_BuOnlineFlag[tCamNum]  = FALSE;
		ubUI_BuOfflineFlag[tCamNum] = FALSE;
		if(UI_MAINMENU_STATE == tUI_State)
			return;
		return;
	}
	switch(tUI_CamStatus[tCamNum].tCamConnSts)
	{
		case CAM_ONLINE:
			//printf("dis osd 2\n");

			if(TRUE == ubUI_BuOnlineFlag[tCamNum])
				break;

		
			//! Audio Source
			//if(tUI_PuSetting.tAdoSrcCamNum == tCamNum)
			//	UI_UpdateBuStatusOsdImg(&tAdoSrcOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			
			ubUI_NoSignalOsdFlag[tCamNum] 	= FALSE;
			ubUI_BuOnlineFlag[tCamNum]  	= TRUE;
			ubUI_BuOfflineFlag[tCamNum] 	= FALSE;
			break;
		case CAM_OFFLINE:
			if(TRUE == ubUI_BuOfflineFlag[tCamNum])
				return;
			
			ubUI_NoSignalOsdFlag[tCamNum] 	= TRUE;
			ubUI_BuOfflineFlag[tCamNum] 	= TRUE;
			ubUI_BuOnlineFlag[tCamNum]  	= FALSE;
			break;
		default:
			break;
	}

/*
	tCamAntLvlOsdImgInfo[uwAntLvLIdx].uwXStart = 0;
	tCamAntLvlOsdImgInfo[uwAntLvLIdx].uwYStart = 1190;	

	tCamBatLvlOsdImgInfo[uwBatLvLIdx].uwXStart = 0;
	tCamBatLvlOsdImgInfo[uwBatLvLIdx].uwYStart = 10;	
	
	//! Antenna Level
	UI_UpdateBuStatusOsdImg(&tCamAntLvlOsdImgInfo[uwAntLvLIdx], OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
	//! Battery Level
	UI_UpdateBuStatusOsdImg(&tCamBatLvlOsdImgInfo[uwBatLvLIdx], OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
*/
}
//------------------------------------------------------------------------------
void UI_DrawBUStatusIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tCamNum, tDrawCamNum;
	UI_CamViewType_t tUI_CamViewType = (TRUE == tUI_PuSetting.ubDualModeEn)?DUAL_VIEW:tCamViewSel.tCamViewType;
	uint8_t ubUI_TotalBuNum = (TRUE == tUI_PuSetting.ubDualModeEn)?CAM_2T:tUI_PuSetting.ubTotalBuNum;

	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
		case SCAN_VIEW:
			if(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID)
				UI_RedrawBuConnectStatusIcon(tCamViewSel.tCamViewPool[0]);
			break;
		case DUAL_VIEW:
		case QUAL_VIEW:
			for(tCamNum = CAM1; tCamNum < ubUI_TotalBuNum; tCamNum++)
			{
				tDrawCamNum = tCamNum;
				if(tCamViewSel.tCamViewType == DUAL_VIEW) {
					tDrawCamNum = (tCamNum == CAM1)?tCamViewSel.tCamViewPool[0]:tCamViewSel.tCamViewPool[1];
				}
				if(tUI_CamStatus[tDrawCamNum].ulCAM_ID != INVALID_ID) {
					UI_RedrawBuConnectStatusIcon(tDrawCamNum);
				}
			}
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RemoveLostLinkLogo(void)
{
	tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_ShowLostLinkLogo(uint16_t *pThreadCnt)
{
	UI_CamNum_t tCamNum;
	uint16_t uwUI_LostPeriod = UI_SHOWLOSTLOGO_PERIOD * 3;
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	if(FALSE == ubUI_ResetPeriodFlag)	
		uwUI_LostPeriod = UI_SHOWLOSTLOGO_PERIOD * 3; //UI_SHOWLOSTLOGO_PERIOD * 5

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
		{
			UI_DrawPUStatusIcon();
			UI_DrawBUStatusIcon();
			return;
		}
	}

	if((FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) && (*pThreadCnt == uwUI_LostPeriod))
	{
		uint32_t ulLostLogAddr = *((uint32_t *)(pbPROF_GetParam(SYSPARAM) + LOSTLOGO_ADDR));
		//UI_CamNum_t tCamNum;

		tUI_PuSetting.IconSts.ubShowLostLogoFlag = TRUE;
		//for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
		//	UI_RedrawBuConnectStatusIcon(tCamNum);

		if(ubFactoryModeFLag == 0)
		{
			if(tUI_PuSetting.ubDefualtFlag	== FALSE)
			{
				tUI_State = UI_DISPLAY_STATE;
			
				tOsdImgInfo.uwHSize  = 1280;
				tOsdImgInfo.uwVSize  = 720;
				tOsdImgInfo.uwXStart = 48;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			
				UI_ClearStatusBarOsdIcon();
				tLCD_JpegDecodeDisable();
				OSD_LogoJpeg(OSDLOGO_LOSTLINK);

				if(ubNoAddCamFlag == 1)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (23*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart = 170;
					tOsdImgInfo.uwYStart = 432;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (23*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart = 502;
					tOsdImgInfo.uwYStart = 360;	
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					printf("UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL1+ (23*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 160;
					tOsdImgInfo.uwYStart =328;	
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL2+ (23*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart= 480;
					tOsdImgInfo.uwYStart =304-104;	
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					printf("UI_ShowLostLinkLogo OSD2IMG_MENU_NOSIGNAL1.\n");
				}
			}
			else
			{	
				tOsdImgInfo.uwHSize  = 1280;
				tOsdImgInfo.uwVSize  = 720;
				tOsdImgInfo.uwXStart = 48;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);		
				
				OSD_LogoJpeg(OSDLOGO_BOOT);
						
				UI_FS_LangageMenuDisplay(0);

				ubFS_MenuItem = 0;
			}
		}
		else
		{
			tUI_State = UI_DISPLAY_STATE;
		
			tOsdImgInfo.uwHSize  = 1280;
			tOsdImgInfo.uwVSize  = 720;
			tOsdImgInfo.uwXStart = 48;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

			UI_ClearStatusBarOsdIcon();
			tLCD_JpegDecodeDisable();
			OSD_LogoJpeg(OSDLOGO_LOSTLINK);

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TITLE, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 60;
			tOsdImgInfo.uwYStart =450;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		}
		ubClearOsdFlag =0;
		ubUI_ResetPeriodFlag = TRUE;
	}

	if(((*pThreadCnt%3) == 0)&&(tUI_PuSetting.ubDefualtFlag == FALSE)&&(TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag))
	{
		UI_UpdateBarIcon_Part1();
	}
}
//------------------------------------------------------------------------------
void UI_RedrawNoSignalOsdIcon(UI_CamNum_t tCamNum, UI_OsdImgFnType_t tOsdImgFnType)
{
	tCamNum =tCamNum;
	tOsdImgFnType = tOsdImgFnType;
	
}
//------------------------------------------------------------------------------
void UI_ClearStatusBarOsdIcon(void)
{
	uint32_t ulLcd_HSize = (uwLCD_GetLcdHoSize() / 2);
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	OSD_IMG_INFO tOsdImgInfo = {0};

	if(FALSE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
		return;

	//tOsdImgInfo.uwHSize = 100;
	//tOsdImgInfo.uwVSize = ulLcd_VSize;
	//OSD_EraserImg1(&tOsdImgInfo);
	//if(DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
	//{
		//tOsdImgInfo.uwXStart += ulLcd_HSize;
		//OSD_EraserImg1(&tOsdImgInfo);
	//}
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_RedrawStatusBar(uint16_t *pThreadCnt)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	if(TRUE == ubUI_StopUpdateStsBarFlag)
		return;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
	{
		*pThreadCnt = (UI_UPDATESTS_PERIOD - 1);
		return;		
	}
	
	if((*pThreadCnt % UI_UPDATESTS_PERIOD) == 0)
	{	
		if(ubFactoryModeFLag == 0)
		{	
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{
				if(ubClearOsdFlag == 0)
				{	
					if(tUI_State == UI_DISPLAY_STATE)
					{
						tOsdImgInfo.uwHSize  = 1280;
						tOsdImgInfo.uwVSize  = 720;
						tOsdImgInfo.uwXStart = 48;
						tOsdImgInfo.uwYStart = 0;
						OSD_EraserImg1(&tOsdImgInfo);
					}
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart = 0;
					tOsdImgInfo.uwYStart = 0;
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

					ubClearOsdFlag =1;
				}
			   	UI_UpdateBarIcon_Part1();
				UI_UpdateBarIcon_Part2();	
				UI_VolBarDisplay(ubGetVoiceTemp);
				UI_TempBarDisplay(ubRealTemp);
				if(ubPairOK_SwitchCam == 1)
				{
					UI_DisableScanMode();

					printf("UI_RedrawStatusBar tCamViewNum: %d, tPairSelCam: %d.\n", tCamViewSel.tCamViewPool[0], tPairInfo.tPairSelCam);
					if(tCamViewSel.tCamViewPool[0] != tPairInfo.tPairSelCam)
					{
						tCamViewSel.tCamViewType	= SINGLE_VIEW;
						tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam;
						tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
						UI_SwitchCameraSource();
						//ubPairOK_SwitchCam = 0;
					}
					ubPairOK_SwitchCam = 0;
					
					if((TRUE == tUI_PuSetting.ubScanModeEn) && (FALSE == ubUI_ScanStartFlag))
						UI_EnableScanMode();
				}
				
			}
			else
			{
				VDO_Stop();
				tOsdImgInfo.uwHSize  = 1280;
				tOsdImgInfo.uwVSize  = 720;
				tOsdImgInfo.uwXStart = 48;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				
				OSD_LogoJpeg(OSDLOGO_BOOT);
				//tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
				//tOsdImgInfo.uwXStart= 0;
				//tOsdImgInfo.uwYStart =0;	
				//tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);	
					
				UI_FS_LangageMenuDisplay(0);

				ubFS_MenuItem = 0;
			}
		}
		else
		{
			if(ubClearOsdFlag == 0)
			{	
				if(tUI_State == UI_DISPLAY_STATE)
				{
					tOsdImgInfo.uwHSize  = 1280;
					tOsdImgInfo.uwVSize  = 720;
					tOsdImgInfo.uwXStart = 48;
					tOsdImgInfo.uwYStart = 0;
					OSD_EraserImg1(&tOsdImgInfo);
				}
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				
				ubClearOsdFlag =1;	
			}
		   	UI_UpdateBarIcon_Part1();
			UI_UpdateBarIcon_Part2();	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TITLE, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 60;
			tOsdImgInfo.uwYStart =450;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

			UI_FactoryStatusDisplay();

			UI_VolBarDisplay(OSD_QUEUE);				
		}

		
	}
	//if(TRUE == ubUI_ShowTimeFlag)
	//	UI_ShowSysTime();

	//UI_TimeShowSystemTime(0);
	
}
void UI_FactoryStatusDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t  ubper_temp, ubir_temp1,ubir_temp2;
	uint16_t  ubir_temp;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_PER, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 200;
	tOsdImgInfo.uwYStart = 900;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TEMP, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 380;
	tOsdImgInfo.uwYStart = 900;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_IRAD, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 560;
	tOsdImgInfo.uwYStart = 900;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

//-----------------------------------------------------------------
	ubper_temp = 100 -KNL_GetPerValue(0);

	UI_TempBarDisplay(ubRealTemp);
	
	if( ubper_temp < 100)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubper_temp/10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 220;
		tOsdImgInfo.uwYStart = 750;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubper_temp%10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 220;
		tOsdImgInfo.uwYStart = 718;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 220;
		tOsdImgInfo.uwYStart = 750;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}

//------------------------------------------------------------------
	tUI_PuSetting.ubTempunitFlag = 1;

	if( ubper_temp < 100)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubRealTemp/10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 400;
		tOsdImgInfo.uwYStart = 750;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 400;
		tOsdImgInfo.uwYStart = 718;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

//-----------------------------
	ubir_temp  = ubGetIR1Temp*256+ubGetIR2Temp;

	ubir_temp1 = ubir_temp/100;
	ubir_temp2 = ubir_temp%100;


		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp1/10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 580;
		tOsdImgInfo.uwYStart = 750;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp1%10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 580;
		tOsdImgInfo.uwYStart = 718;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp2/10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 580;
		tOsdImgInfo.uwYStart = 686;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp2%10)*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 580;
		tOsdImgInfo.uwYStart = 654;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
		
}

void UI_FactorymodeKeyDisplay(uint8_t Value)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(Value)
	{
		case AKEY_MENU:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_MENUKEY ,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;

		case AKEY_ENTER:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_ENTERKEY,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	

		case AKEY_UP:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_UPKEY ,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;

		case AKEY_DOWN:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_DOWNKEY ,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;
		
		case AKEY_RIGHT:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_LEFTKEY ,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;

		case AKEY_LEFT:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_RIGHTKEY ,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	

		case AKEY_PTT:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_PTTKEY,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	

		case PKEY_ID0:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_PKEY ,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	

		case GKEY_ID0:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_VOLUKEY,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;	
		
		case GKEY_ID1:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_FACTORY_VOLDKEY,1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 60;
			tOsdImgInfo.uwYStart = 50;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		break;					
	}	
}

//------------------------------------------------------------------------------
void UI_ChangeBuPsModeToNormalMode(UI_CamNum_t tPS_CamNum)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;

	tUI_DispLoc = (tCamViewSel.tCamViewType == SINGLE_VIEW)?DISP_UPPER_RIGHT:tUI_CamStatus[tPS_CamNum].tCamDispLocation;	
	if(FALSE == tUI_PuSetting.ubScanModeEn)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
		UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
		if(tCamViewSel.tCamViewType == SINGLE_VIEW)
		{
			tOsdImgInfo.uwXStart += 180;
			tOsdImgInfo.uwYStart -= 321;
			OSD_EraserImg2(&tOsdImgInfo);
		}
		else
		{
			tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAL_VIEW)?30:0;
			UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdErase, tUI_DispLoc);
		}
	}
	if(pUI_BuConnectFlag[0])
		*(pUI_BuConnectFlag[0]+tPS_CamNum) = FALSE;
	if(pUI_BuConnectFlag[1])
		*(pUI_BuConnectFlag[1]+tPS_CamNum) = FALSE;
	tUI_CamStatus[tPS_CamNum].tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_ReportBuConnectionStatus(void *pvConnectionSts)
{
	uint8_t *pCamConnSts = (uint8_t *)pvConnectionSts;
	UI_CamNum_t tCamNum;
	/*
	UI_RssiMap2AntLvl_t tAntMap[] =
	{
		{ANT_NOSIGNAL, 		 10},
		{ANT_SIGNALLVL1, 	 90},
		{ANT_SIGNALLVL2, 	110},
		{ANT_SIGNALLVL3, 	130},
		{ANT_SIGNALLVL4, 	160},
		{ANT_SIGNALLVL5, 	255},
	};
	*/
	UI_PerMap2AntLvl_t tAntMap[] =
	{
		{ANT_NOSIGNAL, 		 10},
		{ANT_SIGNALLVL1, 	 20},
		{ANT_SIGNALLVL2, 	 30},
		{ANT_SIGNALLVL3, 	 40},
		{ANT_SIGNALLVL4, 	 60},
		{ANT_SIGNALLVL5, 	 80},
		{ANT_SIGNALLVL6, 	100},
	};
	
//	uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_RssiMap2AntLvl_t), ubIdx;
	uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_PerMap2AntLvl_t), ubIdx;
	static uint8_t ubUI_PsStsFlag = FALSE;
	static uint32_t ulUI_EcoStsCnt[CAM_4T] = {0, 0, 0, 0};

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		tUI_CamStatus[tCamNum].tCamConnSts 	= (pCamConnSts[APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum] == rLINK)?CAM_ONLINE:CAM_OFFLINE;
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
		{
			switch(tUI_CamStatus[tCamNum].tCamConnSts)
			{
				case CAM_ONLINE:
					if(TRUE == ulUI_MonitorPsFlag[tCamNum])
					{
						APP_EventMsg_t tUI_PsMessage = {0};

						tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
						tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
						tUI_PsMessage.ubAPP_Message[1] = PS_ECO_MODE;
						tUI_PsMessage.ubAPP_Message[2] = FALSE;
						tUI_PsMessage.ubAPP_Message[3] = tCamNum;
						tUI_PsMessage.ubAPP_Message[4] = TRUE;
						UI_SendMessageToAPP(&tUI_PsMessage);
						UI_ChangeBuPsModeToNormalMode(tCamNum);
						ulUI_MonitorPsFlag[tCamNum] = FALSE;
					}
					else
					{
					#define	UI_CHKBUECOSTS_PERIOD (2000 / UI_TASK_PERIOD)
						if(++ulUI_EcoStsCnt[tCamNum] > UI_CHKBUECOSTS_PERIOD)
						{
							ulUI_EcoStsCnt[tCamNum] = 0;
							UI_ChangeBuPsModeToNormalMode(tCamNum);
						}
					}
					break;
				case CAM_OFFLINE:
					if(FALSE == ulUI_MonitorPsFlag[tCamNum])
					{
						ulUI_EcoStsCnt[tCamNum] = 0;
						ulUI_MonitorPsFlag[tCamNum] = TRUE;
					}
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
					if(FALSE == ubUI_PsStsFlag)
					{
						tLCD_JpegDecodeDisable();
						UI_DrawPUStatusIcon();
						UI_DrawBUStatusIcon();
						ubUI_PsStsFlag = TRUE;
					}
					continue;
				default:
					break;
			}
		}
		else
			ulUI_EcoStsCnt[tCamNum] = 0;
		tUI_CamStatus[tCamNum].tCamAntLvl = ANT_NOSIGNAL;
//		printd(DBG_Debug3Lvl, "RSSI : %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 8]);
		printd(DBG_Debug3Lvl, "PER: %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]);
		for(ubIdx = 0; ubIdx < ubUI_AntLvlCnt; ubIdx++)
		{
//			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 8]) < tAntMap[ubIdx].ubRssiValue)
			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]) <= tAntMap[ubIdx].ubPerValue)
			{
				tUI_CamStatus[tCamNum].tCamAntLvl = tAntMap[ubIdx].tAntLvl;
				break;
			}
		}
	}
}
//------------------------------------------------------------------------------
void  UI_UpdateBuStatus(UI_CamNum_t tCamNum, void *pvStatus)
{
//	uint8_t *pUI_BuSts = (uint8_t *)pvStatus;

//	if(tCamNum > CAM4)
//		return;

//	tUI_CamStatus[tCamNum].tCamBatLvl = pUI_BuSts[0];
}
//------------------------------------------------------------------------------
void UI_RightArrowLongKey(void)
{
	if(TRUE == ubUI_ShowTimeFlag)
	{
		OSD_IMG_INFO tOsdImgInfo;

		ubUI_ShowTimeFlag = FALSE;
		tOsdImgInfo.uwXStart = 670;
		tOsdImgInfo.uwYStart = 980;
		tOsdImgInfo.uwHSize  = 50;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		return;
	}
	ubUI_ShowTimeFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ShowSysTime(void)
{
	static OSD_IMG_INFO tOsdImgInfo[11];
	static uint8_t ubUI_SysTimeFlag = FALSE;
	uint16_t uwYOffset = uwLCD_GetLcdVoSize();
	uint8_t *pDT_Num[3] = {(uint8_t *)(&tUI_PuSetting.tSysCalendar.ubHour), (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubMin),
	                       (uint8_t *)(&tUI_PuSetting.tSysCalendar.ubSec)};
	uint8_t ubRtcSec = 0;
	uint8_t ubTen = 0, ubUnit = 0, ubSysTimeIdx = 0;

	if(FALSE == ubUI_SysTimeFlag)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 11, &tOsdImgInfo[0]);
		ubUI_SysTimeFlag = TRUE;
	}
	ubRtcSec = tUI_PuSetting.tSysCalendar.ubSec;
	RTC_GetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
	printd(DBG_Debug2Lvl, "Time: %02d:%02d:%02d\n", tUI_PuSetting.tSysCalendar.ubHour, tUI_PuSetting.tSysCalendar.ubMin, tUI_PuSetting.tSysCalendar.ubSec);
	if(ubRtcSec == tUI_PuSetting.tSysCalendar.ubSec)
		return;
	for(ubSysTimeIdx = 0; ubSysTimeIdx < 3; ubSysTimeIdx++)
	{
		ubTen  = *pDT_Num[ubSysTimeIdx] / 10;
		ubUnit = *pDT_Num[ubSysTimeIdx] - (ubTen * 10);
		tOsdImgInfo[ubTen].uwXStart  = 680;
		tOsdImgInfo[ubTen].uwYStart  = uwYOffset - tOsdImgInfo[ubTen].uwVSize;
		tOSD_Img2(&tOsdImgInfo[ubTen], OSD_QUEUE);
		tOsdImgInfo[ubUnit].uwXStart = 680;
		tOsdImgInfo[ubUnit].uwYStart = tOsdImgInfo[ubTen].uwYStart - tOsdImgInfo[ubUnit].uwVSize;
		tOSD_Img2(&tOsdImgInfo[ubUnit], (ubSysTimeIdx == 2)?OSD_UPDATE:OSD_QUEUE);
		if(ubSysTimeIdx != 2)
		{
			tOsdImgInfo[10].uwXStart = 680;
			tOsdImgInfo[10].uwYStart = tOsdImgInfo[ubUnit].uwYStart - tOsdImgInfo[10].uwVSize;
			tOSD_Img2(&tOsdImgInfo[10], OSD_QUEUE);
			uwYOffset = tOsdImgInfo[10].uwYStart;
		}
		tOsdImgInfo[ubTen].uwXStart  = 0;
		tOsdImgInfo[ubTen].uwYStart  = 0;
		tOsdImgInfo[ubUnit].uwXStart = 0;
		tOsdImgInfo[ubUnit].uwYStart = 0;
		tOsdImgInfo[10].uwXStart	 = 0;
		tOsdImgInfo[10].uwYStart	 = 0;
	}
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	if(tCamNum > CAM4)
		return;

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_EnableVox(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};	

	if(DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
		return;

	LCDBL_ENABLE(UI_DISABLE);

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode = PS_VOX_MODE;
	UI_UpdateDevStatusInfo();
	LCD_UnInit();  
	LCD->LCD_MODE = LCD_GPIO;
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};
	UI_PUReqCmd_t tPsCmd;

	if(DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
		return;

	if(CAM_ONLINE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts)
	{
		tPsCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = POWER_NORMAL_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		if(UI_SendRequestToBU(osThreadGetId(), &tPsCmd) != rUI_SUCCESS)
		{
			printd(DBG_ErrorLvl, "Disable VOX Fail !\n");
			return;
		}
	}

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                         [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							 [DISP_LEFT] 	   = 0,				  [DISP_RIGHT] 		 = 0};
	uint16_t uwYOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
	                         [DISP_LOWER_LEFT] = 0, 			  [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LEFT] 	   = 0, 		      [DISP_RIGHT] 		 = (ulLcd_VSize/2)};

	if(tCamNum > CAM4)
		return;

	tUI_DispLoc = (tCamViewSel.tCamViewType == SINGLE_VIEW)?DISP_UPPER_LEFT:tUI_CamStatus[tCamNum].tCamDispLocation;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDTRIG_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset[tUI_DispLoc];
	tOsdImgInfo.uwYStart -= uwYOffset[tUI_DispLoc];
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_PuSetting.IconSts.ubDrawMdTrigFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(DISPLAY_4T1R != tUI_PuSetting.ubTotalBuNum)
		return;
	
	UI_DisableScanMode();
	tUI_PuSetting.ubDualModeEn  = FALSE;
	tCamViewSel.tCamViewType	= SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] 	= tCamNum;
	tUI_PuSetting.tAdoSrcCamNum = tCamNum;
	UI_SwitchCameraSource();
	tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	UI_ClearBuConnectStatusFlag();
	tUI_CamNumSel = tCamViewSel.tCamViewPool[0];
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToBU(osThreadId thread_id, UI_PUReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	APP_StaNumMap_t *pUI_CamNumMap = APP_GetSTANumMappingTable(ptReqCmd->tDS_CamNum);
	uint8_t ubUI_TwcRetry = 5;

	while(--ubUI_TwcRetry)
	{
		if(tTWC_Send(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
			break;
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		return rUI_FAIL;
	}
	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		tReq_Result = (tReq_Event.status == osEventSignal)?(tReq_Event.value.signals == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	return tReq_Result;
}
//------------------------------------------------------------------------------
void UI_RecvBUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus)
{
	if(NULL == tosUI_Notify.thread_id)
		return;
	tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
	if(osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
		printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
//------------------------------------------------------------------------------
void UI_RecvBURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
	switch(pTwc_Data[UI_TWC_TYPE])
	{
		case UI_REPORT:
		{
			UI_CamNum_t tCamNum = NO_CAM;

			APP_TwcTagMap2CamNum(tRecv_StaNum, tCamNum);
			if(tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction)
				tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction(tCamNum, (uint8_t *)(&pTwc_Data[UI_REPORT_DATA]));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(UI_CamNum_t tCamNum)
{
	ulUI_MonitorPsFlag[tCamNum] = FALSE;
	tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
	tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamWdrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamDisMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamFlicker, 		CAMFLICKER_60HZ);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamCbrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamCondenseMode,  CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL, 		 	64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,    64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation, 	64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue, 		64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tREC_Mode, 		REC_MANUAL);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tREC_Resolution, 	RECRES_HD);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tPHOTO_Func, 		PHOTOFUNC_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tPHOTO_Resolution, PHOTORES_3M);
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};
	UI_CamNum_t tCamNum;

	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
		memset(&tUI_CamStatus[tCamNum], 0, sizeof(UI_BUStatus_t));
	memset(&tUI_PuSetting, 0, sizeof(UI_PUSetting_t));
	osMutexWait(APP_UpdateMutex, osWaitForever);
	SF_Read(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	osMutexRelease(APP_UpdateMutex);
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_DevStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_DevStsInfo.cbUI_FwVersion);
	//if(0x93701 == tUI_DevStsInfo.ulUI_DevStsTag)
	if ((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1) == 0)
	&& (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1) == 0)) {
		memcpy(tUI_CamStatus, tUI_DevStsInfo.tBU_StatusInfo, (CAM_4T * sizeof(UI_BUStatus_t)));
		memcpy(&tUI_PuSetting, &tUI_DevStsInfo.tPU_SettingInfo, sizeof(UI_PUSetting_t));
	} else {
		printd(DBG_ErrorLvl, "TAG no match, Reset UI\n");
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.uwYear, 2018);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubMonth, 1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubDate, 1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubHour, 0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubMin, 0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubSec, 0);
	}

	tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
	
	if((tUI_PuSetting.ubFeatCode0 == 0x23)&&(tUI_PuSetting.ubFeatCode1== 0x45)&&(tUI_PuSetting.ubFeatCode2 == 0x67))
	{
		printf("tUI_PuSetting.ubDefualtFlag  %d \n",tUI_PuSetting.ubDefualtFlag);
	}
	else
	{
		printf("ubDefulat OK~~~\n");
			tUI_PuSetting.ubTotalBuNum 				 = DISPLAY_MODE;
			tUI_PuSetting.ubCamViewNum	  =    		CAM1;
			tUI_PuSetting.tAdoSrcCamNum				 = (tUI_PuSetting.tAdoSrcCamNum > CAM4)?CAM1:tUI_PuSetting.tAdoSrcCamNum;
			tUI_PuSetting.BriLvL.tBL_UpdateLvL		 = BL_LVL8; //BL_LVL5
			tUI_PuSetting.VolLvL.tVOL_UpdateLvL		 = VOL_LVL6; //VOL_LVL4
			ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
			
			tUI_PuSetting.IconSts.ubDrawStsIconFlag  = FALSE;
			tUI_PuSetting.IconSts.ubRdPairIconFlag   = FALSE;
			tUI_PuSetting.IconSts.ubClearThdCntFlag	 = FALSE;
			tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
			tUI_PuSetting.IconSts.ubDrawMdTrigFlag 	 = FALSE;
			tUI_PuSetting.ubPairedBuNum				 = 0;
			tUI_PuSetting.ubScanModeEn 				 = TRUE;
			tUI_PuSetting.ubDualModeEn				 = FALSE;
			tUI_PuSetting.ubDefualtFlag				= TRUE;

			tUI_PuSetting.ubFeatCode0				= 0x23;
			tUI_PuSetting.ubFeatCode1				= 0x45;
			tUI_PuSetting.ubFeatCode2				= 0x67;	

			tUI_PuSetting.ubScanTime 				= 1;
			tUI_PuSetting.ubHighTempSetting 		= 0;
			tUI_PuSetting.ubLowTempSetting 			= 0;
			tUI_PuSetting.ubTempAlertSetting 		= 1;
			tUI_PuSetting.ubSoundLevelSetting 		= 0;
			tUI_PuSetting.ubSoundAlertSetting 		= 1;	
			tUI_PuSetting.ubTempunitFlag 			= 0;
			tUI_PuSetting.ubSleepMode				= 1;
			tUI_PuSetting.ubZoomScale				= 0;
			tUI_PuSetting.ubFlickerFlag 			= 1;
			tUI_PuSetting.ubLangageFlag 			= 0;
			tUI_PuSetting.NightmodeFlag				= 0x00;
	}

			for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
			{
				//tCamViewSel.tCamViewPool[tCamNum]      = NO_CAM;
				//if(0x93701 == tUI_DevStsInfo.ulUI_DevStsTag)
				if ((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1) == 0)
				&& (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1) == 0)) {
					ulUI_MonitorPsFlag[tCamNum]   = FALSE;
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
					if(tCamNum >= tUI_PuSetting.ubTotalBuNum)
						tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
					if(INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID)
						tUI_PuSetting.ubPairedBuNum += 1;
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_ON);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_ON);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_ON);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamWdrMode,  		CAMSET_OFF);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamDisMode,  		CAMSET_OFF);
					UI_CHK_CAMFLICKER(tUI_CamStatus[tCamNum].tCamFlicker);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamCbrMode,  		CAMSET_ON);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamCondenseMode,   CAMSET_OFF);
					UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL, 		 64);
					UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,   64);
					UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation, 64);
					UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue, 		 64);
					UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tCamPsMode, POWER_NORMAL_MODE, POWER_NORMAL_MODE);
					UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tREC_Mode, REC_RECMODE_MAX, REC_MANUAL);
					UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tREC_Resolution, RECRES_MAX, RECRES_HD);
					UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tPHOTO_Func, PHOTOFUNC_MAX, PHOTOFUNC_OFF);
					UI_CHK_BUSYS(tUI_CamStatus[tCamNum].tPHOTO_Resolution, PHOTORES_MAX, PHOTORES_3M);
					//tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
				} else {
					//if(tCamNum >= tUI_PuSetting.ubTotalBuNum) {
						tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
					//}
					UI_ResetDevSetting(tCamNum);
				}
			}


	if((tUI_PuSetting.ubHighTempSetting == 0)&&(tUI_PuSetting.ubLowTempSetting == 0)
		&&(tUI_PuSetting.ubSoundLevelSetting == 0))
		ubAlarmIconFlag = 0;
	else
		ubAlarmIconFlag = 1;
	
	ubRealTemp = tUI_PuSetting.ubTempunitFlag?ubRealTemp:UI_TempCToF(ubRealTemp);
	printf("UI_LoadDevStatusInfo ubRealTemp: %d, ubTempunitFlag: %d.\n", ubRealTemp, tUI_PuSetting.ubTempunitFlag);
	UI_GetPairCamInfo();

	tCamViewSel.tCamViewPool[0] = tUI_PuSetting.ubCamViewNum;
	printf("tCamViewSel.tCamViewNum  %d \n",tCamViewSel.tCamViewPool[0]);

	ubFactorySettingFLag = tUI_PuSetting.ubDefualtFlag;

	//tUI_PuSetting.ubLangageFlag 			= 2;
	//printf("tUI_PuSetting.ubScanModeEn   %d \n",tUI_PuSetting.ubScanModeEn );
	//ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
	
	UI_CamvLDCModeCmd(CAMSET_ON); //20180517
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};

	tUI_PuSetting.ubCamViewNum = tCamViewSel.tCamViewPool[0];

	osMutexWait(APP_UpdateMutex, osWaitForever);
	//tUI_DevStsInfo.ulUI_DevStsTag = 0x93701;
	memcpy(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1);
	memcpy(tUI_DevStsInfo.tBU_StatusInfo, tUI_CamStatus, (CAM_4T * sizeof(UI_BUStatus_t)));
	memcpy(&tUI_DevStsInfo.tPU_SettingInfo, &tUI_PuSetting, sizeof(UI_PUSetting_t));
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize);
	SF_Write(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	SF_EnableWrProtect();
	osMutexRelease(APP_UpdateMutex);
}
//------------------------------------------------------------------------------
void UI_SwitchCameraScan(void)
{
	int i, j;
	uint8_t ubCamOnlineNum = 0;
	UI_CamNum_t tCamSwtichNum = 0xFF;
	UI_CamNum_t tSearchCam = tCamViewSel.tCamViewPool[0];

	for(i = 0; i < 4; i++)
		printf("### tUI_CamStatus[%d].ulCAM_ID: 0x%x, tUI_CamStatus[%d].tCamConnSts: %d.\n", i, tUI_CamStatus[i].ulCAM_ID, i, tUI_CamStatus[i].tCamConnSts);

	if(tUI_PuSetting.ubScanTime <= 0)
		return;

	for(i = 0; i < 4; i++)
	{
		if((tUI_CamStatus[i].ulCAM_ID != INVALID_ID) && (tUI_CamStatus[i].tCamConnSts == CAM_ONLINE))
		{
			ubCamOnlineNum++;
		}
	}

	printf("UI_SwitchCameraScan ubCamOnlineNum: %d.\n", ubCamOnlineNum);
	if(ubCamOnlineNum < 2)
		return;

	i = tSearchCam+1;
	for(j = 0; j < 4; j++)
	{
		if((tUI_CamStatus[i].ulCAM_ID != INVALID_ID) && (tUI_CamStatus[i].tCamConnSts == CAM_ONLINE))
		{
			tCamSwtichNum = i;
			break;
		}
		i++;
		if(i > 3) i = 0;
	}

	printf("UI_SwitchCameraScan tCamSwtichNum: 0x%x.\n", tCamSwtichNum);
	if(0xFF == tCamSwtichNum)
		return;
	
	tCamViewSel.tCamViewType	= SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = tCamSwtichNum;
	tUI_PuSetting.tAdoSrcCamNum = tCamSwtichNum;
	ubSetViewCam = tCamViewSel.tCamViewPool[0];
	UI_SwitchCameraSource();
	UI_ClearBuConnectStatusFlag();
}
//------------------------------------------------------------------------------
void UI_TimerDeviceEventStart(TIMER_DEVICE_t tDevice, uint32_t ulTime_ms, void *pvRegCb)
{
	TIMER_SETUP_t tUI_TimerParam;

	tUI_TimerParam.tCLK 		= TIMER_CLK_EXTCLK;	
	tUI_TimerParam.ulTmLoad 	= 10000 * ulTime_ms;
	tUI_TimerParam.ulTmCounter 	= tUI_TimerParam.ulTmLoad;
	tUI_TimerParam.ulTmMatch1 	= tUI_TimerParam.ulTmLoad + 1;
	tUI_TimerParam.tOF 			= TIMER_OF_ENABLE;
	tUI_TimerParam.tDIR 		= TIMER_DOWN_CNT;
	tUI_TimerParam.tEM 			= TIMER_CB;
	tUI_TimerParam.pvEvent 		= pvRegCb;

	printf("UI_TimerDeviceEventStart tDevice: %d, ulTime_ms: %d.\n", tDevice, ulTime_ms);
	TIMER_Start(tDevice, tUI_TimerParam);
}
//------------------------------------------------------------------------------
void UI_TimerDeviceEventStop(TIMER_DEVICE_t tDevice)
{
	printf("UI_TimerDeviceEventStop tDevice: %d.\n", tDevice);
	TIMER_Stop(tDevice);
}
//------------------------------------------------------------------------------
void UI_TimerEventStart(uint32_t ulTime_ms, void *pvRegCb)
{
	TIMER_SETUP_t tUI_TimerParam;

	printf("UI_TimerEventStart TIMER2_1.\n");
	tUI_TimerParam.tCLK 		= TIMER_CLK_EXTCLK;	
	tUI_TimerParam.ulTmLoad 	= 10000 * ulTime_ms;
	tUI_TimerParam.ulTmCounter 	= tUI_TimerParam.ulTmLoad;
	tUI_TimerParam.ulTmMatch1 	= tUI_TimerParam.ulTmLoad + 1;
	tUI_TimerParam.tOF 			= TIMER_OF_ENABLE;
	tUI_TimerParam.tDIR 		= TIMER_DOWN_CNT;
	tUI_TimerParam.tEM 			= TIMER_CB;
	tUI_TimerParam.pvEvent 		= pvRegCb;
	TIMER_Start(TIMER2_1, tUI_TimerParam);
}
//------------------------------------------------------------------------------
void UI_TimerEventStop(void)
{
	printf("UI_TimerEventStop TIMER2_1.\n");
	TIMER_Stop(TIMER2_1);
}
//------------------------------------------------------------------------------
void UI_ScanModeTimerEvent(void)
{
	UI_Event_t tScanEvent;
	osMessageQId *pUI_ScanEventQH = NULL;

	printf("UI_ScanModeTimerEvent###\n");
	UI_TimerEventStop();
	tScanEvent.tEventType = SCANMODE_EVENT;
	tScanEvent.pvEvent 	  = NULL;
	pUI_ScanEventQH 	  = pUI_GetEventQueueHandle();
    osMessagePut(*pUI_ScanEventQH, &tScanEvent, osWaitForever);
}
//------------------------------------------------------------------------------
void UI_SetupScanModeTimer(uint8_t ubTimerEn)
{
	printf("UI_SetupScanModeTimer ubTimerEn: %d, Time: %d.\n", ubTimerEn, ubCameraScanTime[tUI_PuSetting.ubScanTime]*1000);
	ubUI_ScanStartFlag = ubTimerEn;
	if(TRUE == ubTimerEn)
	{
		if((tUI_PuSetting.ubScanTime > 0) && (tUI_PuSetting.ubScanTime < 5))
			UI_TimerEventStart(ubCameraScanTime[tUI_PuSetting.ubScanTime]*1000, UI_ScanModeTimerEvent);
		else
			UI_TimerEventStop();
	}
	else
	{
		UI_TimerEventStop();
	}
}
//------------------------------------------------------------------------------
void UI_EnableScanMode(void)
{
	printf("UI_EnableScanMode ubScanTime: %d.\n", tUI_PuSetting.ubScanTime);
	if(tUI_PuSetting.ubScanTime == 0)
	{
		UI_DisableScanMode();
		return;
	}
	
	UI_CheckCameraSource4SV();
	UI_SetupScanModeTimer(TRUE);
}
//------------------------------------------------------------------------------
void UI_DisableScanMode(void)
{
	//printf("UI_DisableScanMode ubUI_ScanStartFlag: %d #\n", ubUI_ScanStartFlag);
	if(FALSE == ubUI_ScanStartFlag)
		return;
	UI_SetupScanModeTimer(FALSE);
}
//------------------------------------------------------------------------------
void UI_ScanModeExec(void)
{
	printf("UI_ScanModeExec###\n");
	#if 0
	UI_CamNum_t tSearchCam = tCamViewSel.tCamViewPool[0];
	uint8_t ubSearchCnt;

	printf("UI_ScanModeExec tSearchCam: %d.\n", tSearchCam);
	for(ubSearchCnt = 0; ubSearchCnt < tUI_PuSetting.ubTotalBuNum; ubSearchCnt++)
	{
		tSearchCam = ((tSearchCam + 1) >= CAM_4T)?CAM1:((UI_CamNum_t)(tSearchCam + 1));
		if((tUI_CamStatus[tSearchCam].ulCAM_ID != INVALID_ID) &&
	       //(tUI_CamStatus[tSearchCam].tCamConnSts == CAM_ONLINE) &&
		   (tSearchCam != tCamViewSel.tCamViewPool[0]))
		{
			tCamViewSel.tCamViewType	= SINGLE_VIEW;
			tCamViewSel.tCamViewPool[0] 	= tSearchCam;
			tUI_PuSetting.tAdoSrcCamNum = tSearchCam;
			UI_SwitchCameraSource();
			UI_ClearBuConnectStatusFlag();

			ubSetViewCam = tCamViewSel.tCamViewPool[0];
			
			break;
		}
	}
	UI_SetupScanModeTimer(TRUE);
	#else
	UI_SwitchCameraScan();
	UI_SetupScanModeTimer(TRUE);
	#endif
}
//------------------------------------------------------------------------------
UI_Result_t UI_CheckCameraSource4SV(void)
{
	UI_CamNum_t tCamViewNum;

	if((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
	   (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
		return rUI_SUCCESS;

	for(tCamViewNum = CAM1; tCamViewNum < tUI_PuSetting.ubTotalBuNum; tCamViewNum++)
	{
		if((tUI_CamStatus[tCamViewNum].ulCAM_ID != INVALID_ID) &&
	       (tUI_CamStatus[tCamViewNum].tCamConnSts == CAM_ONLINE))
		{
			tCamViewSel.tCamViewType	= SINGLE_VIEW;
			tCamViewSel.tCamViewPool[0] 	= tCamViewNum;
			tUI_PuSetting.tAdoSrcCamNum = tCamViewNum;
			return rUI_SUCCESS;
		}
	}
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_SwitchCameraSource(void)
{
	APP_EventMsg_t tUI_SwitchBuMsg = {0};

	printf("UI_SwitchCameraSource###\n");
	tUI_SwitchBuMsg.ubAPP_Event 	 = APP_VIEWTYPECHG_EVENT;
	tUI_SwitchBuMsg.ubAPP_Message[0] = 3;		//! Message Length	
	tUI_SwitchBuMsg.ubAPP_Message[1] = tCamViewSel.tCamViewType;
	tUI_SwitchBuMsg.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
	tUI_SwitchBuMsg.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
	UI_SendMessageToAPP(&tUI_SwitchBuMsg);
	
	if ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW)) {
		tUI_PuSetting.tAdoSrcCamNum = tCamViewSel.tCamViewPool[0];
		UI_UpdateDevStatusInfo();
	}
	
	RTC_WriteUserRam(RTC_RECORD_VIEW_MODE_ADDR, tCamViewSel.tCamViewType);	
	RTC_WriteUserRam(RTC_RECORD_VIEW_CAM_ADDR, (tCamViewSel.tCamViewPool[0] << 4) | tCamViewSel.tCamViewPool[1]);
}
//------------------------------------------------------------------------------
void UI_EngModeKey(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	OSD_IMGIDXARRARY_t tUI_EngOsdImg;
	uint16_t uwUI_NumArray[10];
	uint16_t uwUI_UpperLetterArray[26];
	uint16_t uwUI_LowerLetterArray[26];
	uint16_t uwUI_SymbolArray[4];
	uint8_t i;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	tOsdImgInfo.uwXStart = 670;
	tOsdImgInfo.uwYStart = 980;
	tOsdImgInfo.uwHSize  = 50;
	tOsdImgInfo.uwVSize  = 300;
	OSD_EraserImg2(&tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOsdImgInfo.uwHSize  = 40;
	tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
	OSD_EraserImg1(&tOsdImgInfo);
	for(i = 0; i < 10; i++)
		uwUI_NumArray[i] = OSD2IMG_ENG_NUM0 + i;
	tUI_EngOsdImg.pNumImgIdxArray = uwUI_NumArray;
	for(i = 0; i < 26; i++)
	{
		uwUI_UpperLetterArray[i] = OSD2IMG_ENG_UPA + i;
		uwUI_LowerLetterArray[i] = OSD2IMG_ENG_LWA + i;
	}
	tUI_EngOsdImg.pUpperLetterImgIdxArray = uwUI_UpperLetterArray;
	tUI_EngOsdImg.pLowerLetterImgIdxArray = uwUI_LowerLetterArray;
	for(i = 0; i < 4; i++)
		uwUI_SymbolArray[i] = OSD2IMG_ENG_COLONSYM + i;
	tUI_EngOsdImg.pSymbolImgIdxArray = uwUI_SymbolArray;
	EN_SetupOsdImgInfo(&tUI_EngOsdImg);
	EN_OpenEnMode(TRUE);
	tUI_State = UI_ENGMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey)
{
	pvUiFuncPtr UI_EngFuncPrt[] = {[UP_ARROW] 	 = EN_UpKey,
								   [DOWN_ARROW]	 = EN_DownKey,
								   NULL, NULL,
								   [ENTER_ARROW] = EN_EnterKey,
								   NULL};
	switch(tArrowKey)
	{
		case EXIT_ARROW:
		{
			EN_OpenEnMode(FALSE);
			UI_ClearBuConnectStatusFlag();
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		default:
			if(UI_EngFuncPrt[tArrowKey])
				UI_EngFuncPrt[tArrowKey]();
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingControl(UI_ArrowKey_t tArrowKey)
{
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
}

UI_CamNum_t UI_GetPairSelCam(void)
{
	return tPairInfo.tPairSelCam;
}

UI_CamNum_t UI_GetCamViewPoolID(void)
{
	return tCamViewSel.tCamViewPool[0];
}

//------------------------------------------------------------------------------
