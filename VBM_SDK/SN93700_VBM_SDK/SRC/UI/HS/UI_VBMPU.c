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
	\version	1.10
	\date		2018/08/02
	\copyright	Copyright (C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include "UI_VBMPU.h"
#include "SF_API.h"
#include "EN_API.h"
#include "FWU_API.h"
#include "TIMER.h"
#include "VDO.h"
#include "LCD.h"
#include "Buzzer.h"
#include "SADC.h"
#include "WDT.h"
#include "PAIR.h"
#include "SD_API.h"

#define osUI_SIGNALS    0x66

#define UI_TEST_MODE    0
//#define Current_Test  0
//#define Current_Mode  PS_VOX_MODE //A:PS_VOX_MODE / B: PS_WOR_MODE

#define SD_UPDATE_TEST  0

#define TIME_TEST       0
#define TEMP_TEST       0

#define ENG_MODE_TEST   0

#define PICKUP_VOLUME_TEST  0

#define UI_BATT_TEST    0

#define UI_NOTIMEMENU	0
#define UI_TEMPMENU	0
#define UI_NOTEMPMENU	1

#define Factory_x_vol   10
#define Factory_y_vol   80
/**
 * Key event mapping table
 *
 * @param ubKeyID           Key ID
 * @param ubKeyCnt          Key count   (100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr   Key event mapping to function
 */
UI_KeyEventMap_t UiKeyEventMap[] =
{
    {NULL,              0,          NULL,                       NULL},
    {AKEY_MENU,         20,         UI_MenuLongKey,             NULL},
    {AKEY_MENU,         0,          UI_MenuKey,                 NULL},
    {AKEY_UP,           0,          UI_UpArrowKey,              NULL},
    {AKEY_DOWN,         0,          UI_DownArrowKey,            NULL},
    {AKEY_DOWN,         5,         NULL,                       NULL},
    {AKEY_LEFT,         0,          UI_LeftArrowKey,            NULL},
    {AKEY_LEFT,         30,         NULL,                       NULL},
    {AKEY_RIGHT,        0,          UI_RightArrowKey,           NULL},
    {AKEY_RIGHT,        30,         NULL,                       NULL},
    {AKEY_ENTER,        0,          UI_EnterKey,                NULL},
    {AKEY_ENTER,        20,         UI_EnterLongKey,            NULL},
    {AKEY_PS,           0,          UI_BuPowerSaveKey,          NULL},
    {AKEY_PS,           20,         UI_PuPowerSaveKey,          NULL},
    {AKEY_PTT,          0,          UI_PushTalkKeyShort,        NULL},
    {AKEY_PTT,          3,          UI_PushTalkKey,             NULL},
    {PKEY_ID0,          0,          UI_PowerKeyShort,           NULL},
    {PKEY_ID0,          20,         UI_PowerKey,                NULL},
    {GKEY_ID0,          0,          UI_VolUpKey,                NULL},
    {GKEY_ID1,          0,          UI_VolDownKey,              NULL},
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
	[UI_SDFWUPG_STATE]			= UI_FwUpgExecSel,	
	[UI_RECFOLDER_SEL_STATE]	= NULL,
	[UI_RECFILES_SEL_STATE]		= NULL,
	[UI_RECPLAYLIST_STATE]      	= NULL,
	[UI_PHOTOPLAYLIST_STATE]	= NULL,
	[UI_ENGMODE_STATE]			= UI_EngModeCtrl,
};
static UI_MenuItem_t tUI_MenuItem;
static UI_SubMenuItem_t tUI_SubMenuItem[MENUITEM_MAX] =
{
	{ADJUST_BR_ITEM,	BRIGHTNESS_MAX	},
	{PAIRCAM_ITEM,		PAIRITEM_MAX	},
	{RECMODE_ITEM,	RECITEM_MAX		},
	{PHOTOSELCAM_ITEM, 	PHOTOITEM_MAX	},
	{NULL},
	{CAMSSELCAM_ITEM,	CAMSITEM_MAX	},
	{NIGHTMODE_ITEM, 	SETTINGITEM_MAX	},
};
UI_ReportFuncPtr_t tUiReportMap2Func[] =
{
    [UI_UPDATE_BUSTS]   = UI_UpdateBuStatus,
    [UI_VOX_TRIG]       = UI_VoxTrigger,
    [UI_MD_TRIG]        = UI_MDTrigger,
    [UI_VOICE_TRIG]     = UI_VoiceTrigger,
    [UI_VOICE_CHECK]    = UI_GetVolData,
    [UI_TEMP_CHECK]     = UI_GetTempData,
    [UI_BU_TO_PU_CMD]   = UI_GetBUCMDData,
};


ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n39p1DB, R2R_VOL_n29p8DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};
uint32_t ulUI_BLTable[] = {0, 3, 10, 17, 24, 31, 38, 45, 52};//ruizhiwei
//uint32_t ulUI_BLTable[] = {0, 5, 17, 26, 37, 48, 58, 67, 74};//yushun

osMutexId UI_PUMutex;
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
static uint8_t ubUI_RecSubMenuFlag;
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

//uint8_t brightness_temp =1;
uint8_t ubAutoBrightnessFlag =0;
uint8_t ubAutoBrightnessValue = 4;
uint8_t ubSleepModeFlag = 1;
uint8_t ubAutoSleepTime[4] = {0, 1, 3, 5};
uint8_t ubZoomFlag = 0;
uint8_t ubLangageFlag = 0;
uint8_t ubFlickerFlag = 0;
uint8_t ubDefualtFlag = 0;
uint8_t ubTempunitFlag = 0;
uint8_t ubNightmodeFlag[4] = {0,0,0,0};

uint8_t ubTimeHour = 12;
uint8_t ubTimeMin = 0;
uint8_t ubTimeAMPM = 1;
uint8_t ubTimeSec = 0;

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
uint8_t ubAlertTime[4] = {10,20,30,60};
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
uint8_t ubGetVoice1 = 0;
uint8_t ubGetVoice2 = 0;
uint8_t ubGetVoice3 = 0;
uint8_t ubGetVoice4 = 0;
uint32_t ubPickupVolume = 0;

uint8_t ubPairSelCamcnt = 0; 

uint8_t ubCamPairFlag[4] = {0,0,0,0};
uint8_t ubPairSelCam = 0;
uint8_t ubCamFullFlag = 0;
uint8_t ubNoAddCamFlag = 0;
//uint16_t ubScanTime = 5000;

uint8_t ubEnterTimeMenuFlag = 0;
uint8_t ubDisplaymodeFlag = 0;

uint8_t ubGetTempData = 25;
uint8_t ubRealTemp = 0xFF;
uint8_t ubRealTempCheckCnt = 0;
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
uint8_t ubAlarmPlayState = 0;

uint8_t ubDelCamitem = 0;
uint8_t ubCameraScanTime[5] = {0, 5, 10, 15, 30};
uint8_t ubShowCamPairFullSta = 0;
uint8_t ubCameraOnlineNum = 0;
uint8_t ubMenuKeyPairing = 0;

//uint8_t ubShowAlarmAgain = 0;
uint8_t ubShowAlarmType = 0;
uint8_t ubShowAlarmstate = 0;
uint16_t ubPlayAlarmCount = 0;
uint16_t ubTempAlarmCheckCount = 0;
uint8_t ubTempAlarmState = TEMP_ALARM_IDLE;
uint16_t ubTempAlarmTriggerCount = 0;
uint8_t ubTempBelowZoreSta = 0;
uint8_t ubTempInvalidSta = 1;

uint16_t ubPickupAlarmCheckCount = 0;
uint16_t ubPickupAlarmState = PICKUP_ALARM_IDLE;
uint16_t ubPickupAlarmTriggerCount = 0;

uint8_t ubDisplayPTN = 0;
uint8_t ubMotorDirection = 0;
uint8_t ubMotor0State = MC_LEFT_RIGHT_OFF;
uint8_t ubMotor1State = MC_UP_DOWN_OFF;
uint8_t ubMC0OnCount = 0;
uint8_t ubMC1OnCount = 0;
uint8_t ubPairOK_SwitchCam = 0;
uint8_t ubSetViewCam = 0;

uint8_t ubBatLowCount = 0;
int16_t ubGetBatPercent = -1;
uint16_t ubGetBatVoltage = 0;
uint16_t ubBatLvLIdx = BAT_LVL0;

uint16_t ubLinkStateCheckCount = 0;
uint8_t ubLostLinkEnterSanMode = 0;
uint8_t ubLogoInitStaus = 0;

uint8_t ubFactorySettingFLag ;
uint8_t ubClearOsdFlag_2 =0;
uint8_t ubFactoryModeFLag = 0;
uint8_t ubPUEnterAdotestFLag = 0;
uint8_t ubBUEnterAdotestFLag = 0;

uint8_t ubGetIR1Temp = 0;
uint8_t ubGetIR2Temp = 0;

uint8_t ubPuHWVersion = 1;
uint32_t ubPuSWVersion = 13;
uint32_t ubHWVersion = 13;
uint8_t ubBuHWVersion = 0;
uint32_t ubBuSWVersion = 0;

uint32_t ubPttEndCount = 0;

uint8_t ubFastShowLostLinkSta = 0;
uint8_t ubGetPsModeFlag = 0;

uint8_t ubPowerState = PWR_ON;
uint8_t ubSleepWaitCnt = 0;
uint8_t ubWakeUpWaitCnt = 0;

uint16_t ubSpeakerCount = 0;
uint8_t ubReStartWakeUpFlag;
uint8_t ubWorWakeUpFlag;


uint8_t ubTimerDevEventStopSta = 0;
uint8_t ubCamPairOkState = 0;
uint8_t ubAlarmWakeupType = 0;
uint8_t ubSwitchCamWakeupSstate = 0;

uint8_t ubNormalModeToBuRet = rUI_FAIL;
uint8_t ubVOXModeToBuRet = rUI_FAIL;

uint8_t ubAlarmOnlyAlertFlag  = 0;
uint8_t ubFactoryDeleteCam = 0;
uint8_t ubSwitchNormalFlag  = 1;
extern uint16_t ubTest_uwCropHsize;
extern uint16_t ubTest_uwCropVsize;
extern uint16_t ubTest_uwCropHstart;
extern uint16_t ubTest_uwCropVstart;
extern uint32_t ulPAIR_TimeCnt;

char RxSNdata[16] = {0};
char TxSNdata[16] = {'X','X','X','X','X','X','X','X','X','X','X','X','X','X','X','X'};

uint8_t ubComboKeyFlag  = 0;
uint8_t ubAdoOnlyFlag  = 0;
uint16_t ubAdoOnlyCnt = 0;
uint8_t ubUpdateFWFlag = 0;

uint8_t ubVolWarnCnt  = 0;
uint8_t ubVolWarnFlag  = 0;

uint8_t ubEnterFactoryDelCam = 0;
uint8_t ubFactoryDelCamFlag  = 0;

//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
    static uint8_t ubUI_KeyEventIdx = 0;
    uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;
    OSD_IMG_INFO tOsdImgInfo;

    KEY_Event_t *ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
    uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	printd(1,"UI_KeyEventExec ptKeyEvent->ubKeyID =%x ptKeyEvent->ubKeyAction =%d\n",ptKeyEvent->ubKeyID,ptKeyEvent->ubKeyAction);

    if (tUI_PuSetting.ubDefualtFlag == TRUE) //恢复出厂设置只有上下左右,Enter,PowerKey键有用
    {
         if ((ptKeyEvent->ubKeyID < AKEY_UP) || (ptKeyEvent->ubKeyID > AKEY_ENTER))
        {
           		 if (ptKeyEvent->ubKeyID != PKEY_ID0)
               		 return;
        }
/*	//语言设置界面进行唤醒，但是唤醒之后无LOGO
	   if (PWR_ON != ubPowerState)
	   {
           		 if (ptKeyEvent->ubKeyID != PKEY_ID0) //Powerkey
           	 	{
           	 		if(ubFS_MenuItem == 0)
           	 		{
           	 			if ((ptKeyEvent->ubKeyID == AKEY_UP) || (ptKeyEvent->ubKeyID == AKEY_DOWN) || (ptKeyEvent->ubKeyID > AKEY_ENTER))
					{
					           printd(1,"11111111111111111111\n");
						UI_SetSleepState(1);
						LCDBL_ENABLE(UI_ENABLE);
						return;
           	 				}
           	 		}
				if(ubFS_MenuItem == 1)
           	 		{
           	 			printd(1,"22222222222222\n");
           	 			if ((ptKeyEvent->ubKeyID >= AKEY_UP) ||(ptKeyEvent->ubKeyID <= AKEY_ENTER))
					{
           	 			printd(1,"333333333333333333333\n");
						UI_SetSleepState(1);
						LCDBL_ENABLE(UI_ENABLE);
						return;
           	 				}
           	 		}
           	 	}
	   }	
*/
    }
    else
    {
#if Current_Test
        if (PWR_ON != ubPowerState)
        {
            if (ptKeyEvent->ubKeyID != PKEY_ID0) //Powerkey
            {
                if ((ubUI_PttStartFlag == TRUE) && (ptKeyEvent->ubKeyID == AKEY_PTT))
                {
                    printd(Apk_DebugLvl, "AKEY_PTT###\n");
                }
                else
                {
                    UI_SetSleepState(0);
                    return;
                }
            }
        }
#endif
    }

    if (ubFactoryModeFLag == 1)
    {
        if ((ptKeyEvent->ubKeyID == PKEY_ID0)&&(GPIO->GPIO_I9 == 0))
        {
            if (ubPUEnterAdotestFLag == 0)
            {
                ubPUEnterAdotestFLag = 1;
                //UI_EnterLocalAdoTest_RX();
            }
        }

        if ((ptKeyEvent->ubKeyID == PKEY_ID0)&&(GPIO->GPIO_I10 == 0))
        {
		    uint8_t CmdData;
		    uint8_t sendflag = 0;
		   CmdData = UI_SET_BUMOTOR_AUTO_CMD;
		  UI_SendToBUCmd(&CmdData, 1);
		   printd(1," SEND BU CMD !\n");	             
        }

        if (ptKeyEvent->ubKeyAction == KEY_DOWN_ACT)
        {
            UI_FactorymodeKeyDisplay(ptKeyEvent->ubKeyID);
        }
        if (ptKeyEvent->ubKeyAction == KEY_UP_ACT)
        {
            /*
            tOsdImgInfo.uwHSize  = 76;
            tOsdImgInfo.uwVSize  = 300;
            tOsdImgInfo.uwXStart = 60;
            tOsdImgInfo.uwYStart = 50;
            OSD_EraserImg2(&tOsdImgInfo);
            */
            //ubPUEnterAdotestFLag = 0;
            //ubBUEnterAdotestFLag = 0;
        }
    }
    else
    {
        if((ptKeyEvent->ubKeyAction == KEY_UP_ACT)&&((ptKeyEvent->ubKeyID == GKEY_ID0)
            ||(ptKeyEvent->ubKeyID == GKEY_ID1)))
        {
            if(ubUI_PttStartFlag == TRUE)
                ubComboKeyFlag = 1;
        }
        if((ptKeyEvent->ubKeyID == PKEY_ID0)&&(ubUI_PttStartFlag == TRUE))
        {
            ubComboKeyFlag = 1;
        }
        if(ubUI_PttStartFlag == FALSE)
            ubComboKeyFlag = 0;
    }

    if (tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        if (ubUI_ResetPeriodFlag == FALSE)
            return;
    }

    if (ptKeyEvent->ubKeyID != PKEY_ID0)
    {
        UI_CameraResetCycleTime(ptKeyEvent->ubKeyAction);
    }

    if (UI_AutoLcdResetSleepTime(ptKeyEvent->ubKeyAction) == 1)
        return;

    if (ptKeyEvent->ubKeyAction == KEY_UP_ACT)
    {
        if ((UI_DISPLAY_STATE == tUI_State) &&
           ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
            (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
        {
            if (ubShowAlarmstate) // UI_GetAlarmStatus()
                return;
        	printd(1,"22222222222222\n");

//          UI_DPTZ_KeyRelease(ptKeyEvent->ubKeyID);
            UI_MotorControl((ptKeyEvent->ubKeyID > AKEY_DOWN)?1:0);
            return;
        }
        if (((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt)) ||
           (ptKeyEvent->ubKeyID == AKEY_PTT))
        {
            if (ptKeyEvent->ubKeyID == AKEY_PTT)
            {
                if((ubUI_PttStartFlag == TRUE)&&(ubComboKeyFlag == 1))
                {
                    ubUI_KeyEventIdx = 15;
                    ubComboKeyFlag = 0;
                }
                ubUI_PttStartFlag = FALSE;
            }
            if (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
            {
                UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();

                //if (UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
                    //UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
            }
        }
        if (ubUI_PttStartFlag == FALSE)
            ubUI_KeyEventIdx = 0;
        return;
    }

    for (uwIdx=1; uwIdx<uwUiKeyEvent_Cnt; uwIdx++)
    {
        if ((UI_DISPLAY_STATE == tUI_State) &&
           ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
            (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT) ||
            (ptKeyEvent->ubKeyID == GKEY_ID0)  || (ptKeyEvent->ubKeyID == GKEY_ID1)))
        {
            if (UI_DPTZ_KeyPress(ptKeyEvent->ubKeyID, uwIdx) == rUI_FAIL)
                continue;

            if (ubShowAlarmstate)
                return;
			
            if (ubUI_PttStartFlag == FALSE)
                ubUI_KeyEventIdx = 0;

            if (UiKeyEventMap[uwIdx].KeyEventFuncPtr)
            {
                if (ptKeyEvent->ubKeyAction == KEY_CNT_ACT)
                {
                	printd(Apk_DebugLvl,"443333333333333333ptKeyEvent->uwKeyCnt =%d\n",ptKeyEvent->uwKeyCnt);
                	if(((ptKeyEvent->ubKeyID == GKEY_ID0)  || (ptKeyEvent->ubKeyID == GKEY_ID1)) && (ptKeyEvent->uwKeyCnt % 3 == 0))
                	{
                		printd(Apk_DebugLvl,"111111111111111111\n");
				UiKeyEventMap[uwIdx].KeyEventFuncPtr();
                	}
                }
		  else if(ptKeyEvent->ubKeyAction == KEY_DOWN_ACT )
		 {
                		printd(Apk_DebugLvl,"44444444444444444444\n");
				UiKeyEventMap[uwIdx].KeyEventFuncPtr();	
		 }
   	       break;

            }

        }

        if ((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
           (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
        {
            ubUI_KeyEventIdx = uwIdx;
            //if(((ptKeyEvent->uwKeyCnt) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)) ||
               //(ptKeyEvent->ubKeyID == AKEY_PTT))
            if(((ptKeyEvent->uwKeyCnt) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)))
            {
                if(ptKeyEvent->ubKeyID == AKEY_PTT)
                    ubUI_PttStartFlag = TRUE;
                UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();

                //if (UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
                    //UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
                ubUI_KeyEventIdx = (ptKeyEvent->ubKeyID == AKEY_PTT)?ubUI_KeyEventIdx:0;
            }
        }
    }
}
//------------------------------------------------------------------------------
void UI_OnInitDialog(void)
{
//  OSD_IMG_INFO tOsdImgInfo;
//  uint8_t i = 0;
	if(ubWorWakeUpFlag != 1)	//20190905 yxh
	OSD_LogoJpeg(OSDLOGO_BOOT);


	if ((DISPLAY_MODE != DISPLAY_1T1R) && wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) == RTC_WATCHDOG_CHK_TAG) {
		printd(DBG_ErrorLvl, "Watch Dog Rst\n");
		tCamViewSel.tCamViewType    = (UI_CamViewType_t)wRTC_ReadUserRam(RTC_RECORD_VIEW_MODE_ADDR);
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

		tCamViewSel.tCamViewType = SINGLE_VIEW;
	}

    if (tCamViewSel.tCamViewType == SCAN_VIEW) {
        UI_EnableScanMode();
    } else {
        UI_DisableScanMode();
    }

    UI_LoadDevStatusInfo();
    ubSetViewCam = tCamViewSel.tCamViewPool[0];

    if (iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
    {
        printd(DBG_ErrorLvl, "Calendar base setting fail!\n");
        //return;
    }

	if (((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF) != RTC_PWRSTS_KEEP_TAG) &&
		((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF) != RTC_WATCHDOG_CHK_TAG))
		RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));

    RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_WATCHDOG_CHK_TAG);

	ubLogoInitStaus = 1;
	
	printd(1,"ubWorWakeUpFlag = %d\n",ubWorWakeUpFlag);
	if(ubWorWakeUpFlag != 1)  //20190905 yxh
	{
		LCDBL_ENABLE(UI_ENABLE);
	}
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
    osMutexDef(UI_PUMutex);
    UI_PUMutex                = osMutexCreate(osMutex(UI_PUMutex));
    tosUI_Notify.thread_id    = NULL;
    tosUI_Notify.iSignals     = 0;
    ubUI_ResetPeriodFlag      = FALSE;
    ubUI_FastStateFlag        = FALSE;
    ubUI_ShowTimeFlag         = FALSE;
    ubUI_PttStartFlag         = FALSE;
    ubUI_ScanStartFlag        = FALSE;
    ubUI_DualViewExFlag       = FALSE;
    ubUI_StopUpdateStsBarFlag = FALSE;
    tUI_State                 = UI_DISPLAY_STATE;
    tUI_SyncAppState          = APP_STATE_NULL;
    ulUI_LogoIndex            = OSDLOGO_BOOT;
    printd(Apk_DebugLvl, "UI_StateReset###\n");
    ubLangageFlag = tUI_PuSetting.ubLangageFlag;

    tUI_MenuItem.ubItemIdx    = BRIGHT_ITEM;
    tUI_MenuItem.ubItemPreIdx = BRIGHT_ITEM;

	UI_ResetSubMenuInfo();
	UI_ResetSubSubMenuInfo();
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvBUResponse, UI_RecvBURequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
	UI_LoadDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_UpdateFwUpgStatus(void *ptUpgStsReport)
{
	APP_StatusReport_t *pFWU_StsRpt = (APP_StatusReport_t *)ptUpgStsReport;
	OSD_IMG_INFO tOsdImgInfo;

	switch (pFWU_StsRpt->ubAPP_Report[0])
	{
		case FWU_UPG_INPROGRESS:
			ubUpdateFWFlag =1;
			
			OSD_IMG_INFO tOsdInfo;

			tOsdInfo.uwHSize  = 672;
			tOsdInfo.uwVSize  = 1280;
			tOsdInfo.uwXStart = 48;
			tOsdInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdInfo);
			
			tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearBuConnectStatusFlag();

			tLCD_JpegDecodeDisable();
			OSD_LogoJpeg(OSDLOGO_FW_INPROGRESS);
			break;
		case FWU_UPG_SUCCESS:
			uint8_t ubKNL_PsValue = 0;

			ubKNL_PsValue  = wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR);
			ubKNL_PsValue &= 0xF;
			ubKNL_PsValue |= 0x40;
			RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, ubKNL_PsValue);

			
			tLCD_JpegDecodeDisable();
			OSD_LogoJpeg(OSDLOGO_FW_OK);
			osDelay(1000);
            break;	
		
		case FWU_UPG_FAIL:
		case FWU_UPG_DEVTAG_FAIL:
		{
			tLCD_JpegDecodeDisable();
			OSD_LogoJpeg(OSDLOGO_FW_FAIL);
			osDelay(1000);
			ubClearOsdFlag = 0;
			break;
		}
		
		default:
		{
			//printf("fw fail 2~~~~~\n");
			/*	
			uint8_t ubProgScaleIdx = (pFWU_StsRpt->ubAPP_Report[0] / pFWU_StsRpt->ubAPP_Report[1]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_FWUPROG0P_ICON + ubProgScaleIdx), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(100 == pFWU_StsRpt->ubAPP_Report[0])
				osDelay(1000);
			*/	
			break;
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
    APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
    static uint8_t ubUI_PuStartUpFlag = FALSE;

    osMutexWait(UI_PUMutex, osWaitForever);
    switch (pAppStsRpt->tAPP_ReportType)
    {
    case APP_PAIRSTS_RPT:
        {
            UI_Result_t tPair_Result  = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];
            UI_CamNum_t tAppAdoSrcNum = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[1];

            UI_ReportPairingResult(tPair_Result);
            if ((rUI_SUCCESS == tPair_Result) &&
               (tUI_PuSetting.tAdoSrcCamNum != tAppAdoSrcNum))
                pAppStsRpt->ubAPP_Report[1] = tUI_PuSetting.tAdoSrcCamNum;
            break;
        }
    case APP_LINKSTS_RPT:
        UI_ReportBuConnectionStatus(pAppStsRpt->ubAPP_Report);
        break;
    case APP_VWMODESTS_RPT:
        if (pAppStsRpt->ubAPP_Report[0] != SCAN_VIEW)
            UI_DisableScanMode();
        break;
    case APP_VOXMODESTS_RPT:
        ubUI_StopUpdateStsBarFlag = pAppStsRpt->ubAPP_Report[0];
        tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
        UI_ClearStatusBarOsdIcon();
        UI_ClearBuConnectStatusFlag();
        break;
    case APP_PAIRUDBU_PRT:
        {
            UI_CamNum_t tDelCam = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[0];
            UI_UnBindBu(tDelCam);
            break;
        }
    default:
        break;
    }
    if (pAppStsRpt->tAPP_State == APP_LINK_STATE)
    {
        if (tUI_SyncAppState != pAppStsRpt->tAPP_State)
        {
            printd(Apk_DebugLvl, "APP_LINK_STATE###\n");
            UI_RemoveLostLinkLogo();
        }
        ubUI_ResetPeriodFlag = TRUE;
        if (FALSE == ubUI_PuStartUpFlag)
        {
#if Current_Test
            //if (PS_VOX_MODE == tUI_PuSetting.tPsMode)
                //UI_EnableVox();
#endif
            ubUI_PuStartUpFlag = TRUE;
        }
        /*
        if ((tCamViewSel.tCamViewType == SCAN_VIEW) && (FALSE == ubUI_ScanStartFlag))
            UI_EnableScanMode();
        */

	   if ((FALSE == ubUI_ScanStartFlag) && (tUI_PuSetting.ubScanTime > 0)&& (tUI_PuSetting.ubDefualtFlag == FALSE))
	   {
	       UI_EnableScanMode();
	   }
	   ubUI_PuStartUpFlag = TRUE;
    }
    tUI_PuSetting.IconSts.ubClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
    tUI_SyncAppState = pAppStsRpt->tAPP_State;
    osMutexRelease(UI_PUMutex);
}

void UI_PairStateCheck(uint16_t ubCheckCount)
{
    if(tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        if((APP_PAIRING_STATE == tUI_SyncAppState) || (ubShowAlarmstate > 0))
        {
            //UI_TimerDeviceEventStart(TIMER1_2, ubAutoSleepTime[tUI_PuSetting.ubSleepMode]*1000*60, UI_AutoLcdSetSleepTimerEvent);
            UI_TimerDeviceEventStop(TIMER1_2);
        }
    }
    else
    {
        if((APP_PAIRING_STATE == tUI_SyncAppState) || (ubShowAlarmstate > 0))
        {
            //UI_TimerDeviceEventStart(TIMER1_2, ubAutoSleepTime[tUI_PuSetting.ubSleepMode]*1000*60, UI_AutoLcdSetSleepTimerEvent);
            UI_TimerDeviceEventStop(TIMER1_2);
        }
    }
}
//------------------------------------------------------------------------------
void UI_LinkStatusCheck(uint16_t ubLinkCheckCount)
{
    if (ubFactoryModeFLag == 1)
    {
        static uint8_t getBuResult = 0;
        static uint8_t getBuMICResult = 0;
        if (getBuResult == rUI_FAIL)
        {
            getBuResult = UI_GetBuVersion();
        }
/*	if(ubBUEnterAdotestFLag == 1)	
	{
      		  if (getBuMICResult == rUI_FAIL)
      		  	{
			  	getBuMICResult = UI_GetBuMICTest();
				ubBUEnterAdotestFLag =0;
      		  	}
	}  
	ubBUEnterAdotestFLag = 1;

	*/
    }
    
    
}
//------------------------------------------------------------------------------
void UI_StatusCheck(uint16_t ubCheckCount)
{
    OSD_IMG_INFO tOsdInfo;
    OSD_IMG_INFO tOsdImgInfo;
    static uint8_t ubSetAlarmRet = rUI_FAIL;
    static uint8_t ubNightModeRet = rUI_FAIL;

    if (ubFactoryModeFLag == 1)
    {
        if (ubCheckCount%5 == 0)
        {
            UI_FactoryStatusDisplay();
        }

        if (ubPUEnterAdotestFLag == 1)
        {
            ubPUEnterAdotestFLag = 2;
            UI_EnterLocalAdoTest_RX();
            ubPUEnterAdotestFLag = 0;
        }
    }


    if ((ubCheckCount%10) == 0)
    {
        UI_GetBatLevel(ubCheckCount);
    }

    UI_CheckUsbCharge();

    if (tUI_SyncAppState != APP_LOSTLINK_STATE)
    {
        ubLostLinkEnterSanMode = 0;
    }

    if (tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        if (tUI_SyncAppState == APP_LINK_STATE)
        {
#if Current_Test
            /*
            if((ubGetPsModeFlag == 0) && (UI_GetBuPsMode() == rUI_SUCCESS))
                ubGetPsModeFlag = 1;
            */
#endif

            if (ubSetAlarmRet == rUI_FAIL)
                ubSetAlarmRet = UI_SendAlarmSettingToBu();

            if (ubNightModeRet == rUI_FAIL)
                ubNightModeRet = UI_SendNightModeToBu();

            if (ubPowerState == PWR_ON)
            {
                if (ubNormalModeToBuRet == rUI_FAIL)
                    ubNormalModeToBuRet = UI_SendPwrNormalModeToBu();
            }
	   if(ubPowerState == PWR_Sleep_Complete)
	   {
		if(ubVOXModeToBuRet == rUI_FAIL)
			ubVOXModeToBuRet = UI_SendPwrVoxModeToBu();
	   }
			

            ubLinkStateCheckCount = 0;
            UI_SetSpeaker(0, 1);
        }
        else
        {
            ubLinkStateCheckCount++;
            if (ubSpeakerCount == 0)
                UI_SetSpeaker(1, 0);

#if Current_Test
            ubGetPsModeFlag = 0;
            if ((ubLinkStateCheckCount % 15) && (ubPowerState == PWR_Sleep_Complete))
            {
                //ubWakeUpWaitCnt = 0;
                //ubPowerState = PWR_Prep_Wakeup;
            }
#endif
            ubSetAlarmRet = rUI_FAIL;
            ubNightModeRet = rUI_FAIL;
        }

        if (ubPowerState != PWR_ON)
            ubNormalModeToBuRet = 0;

        if (ubSpeakerCount >= 1)
            ubSpeakerCount++;

        if (ubSpeakerCount == 4)
            UI_SetSpeaker(1, TRUE);

        /*
        if((APP_PAIRING_STATE == tUI_SyncAppState) || (ubShowAlarmstate > 0))
        {
            UI_TimerDeviceEventStop(TIMER1_2);
        }
        */

#if Current_Test
        UI_CheckPowerMode();
#endif

        ubCameraOnlineNum = UI_GetCamOnLineNum(0);
        if(ubMenuKeyPairing >= 2)
        {
            ubMenuKeyPairing++;
            if(ubMenuKeyPairing >= 4)
            {
                tOsdInfo.uwHSize  = 672;
                tOsdInfo.uwVSize  = 1280;
                tOsdInfo.uwXStart = 48;
                tOsdInfo.uwYStart = 0;
                OSD_EraserImg2(&tOsdInfo);
                ubMenuKeyPairing = 0;
                tUI_State = UI_DISPLAY_STATE;
			
		  if (ubNoAddCamFlag == 1)
	    	  {
	            OSD_LogoJpeg(OSDLOGO_WHITE_BG);
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart = 175;
	            tOsdImgInfo.uwYStart = 279;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	            if (tUI_PuSetting.ubLangageFlag == 2)
	            {
	                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
	                tOsdImgInfo.uwXStart = 503;
	                tOsdImgInfo.uwYStart = 360 - 80 - 5;
	                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	            }
	            else
	            {
	                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
	                tOsdImgInfo.uwXStart = 503;
	                tOsdImgInfo.uwYStart = 360 - 80;
	                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	            }
	            printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
	        }
    
                return;
            }
        }
    }
    else
    {
        ubLinkStateCheckCount++;
        if (ubSpeakerCount == 0)
            UI_SetSpeaker(1, 0);

#if Current_Test
        ubGetPsModeFlag = 0;
        if ((ubLinkStateCheckCount % 15) && (ubPowerState == PWR_Sleep_Complete))
        {
            //ubWakeUpWaitCnt = 0;
            //ubPowerState = PWR_Prep_Wakeup;
        }
#endif

        ubSetAlarmRet = rUI_FAIL;
        ubNightModeRet = rUI_FAIL;

        if (ubPowerState != PWR_ON)
            ubNormalModeToBuRet = 0;

        if (ubSpeakerCount >= 1)
            ubSpeakerCount++;

        if (ubSpeakerCount == 4)
            UI_SetSpeaker(1, TRUE);

        /*
        if((APP_PAIRING_STATE == tUI_SyncAppState) || (ubShowAlarmstate > 0))
        {
            UI_TimerDeviceEventStop(TIMER1_2);
        }
        */

#if Current_Test
        UI_CheckPowerMode();
#endif
    }


    if (ubFactoryModeFLag== 1)
    {
        static uint8_t ubCamId = 0;
	if(1 == ubFactoryDelCamFlag)
	{
	        if (tUI_CamStatus[ubCamId].ulCAM_ID != INVALID_ID)
	        {
	            UI_CamDeleteCamera(1, ubCamId);
	        }
	        ubCamId++;
	        if (ubCamId > 4)
	        {
	            ubCamId = 0;  

		    ubFactoryDelCamFlag =0;
		    printf("factory del finish\n");
		    osDelay(200);
		    UI_LoadDevStatusInfo();		
		    tUI_PuSetting.ubDefualtFlag = 0;

		    // WDT_Disable(WDT_RST);
                       //WDT_RST_Enable(WDT_CLK_EXTCLK, 1);//reboot	
                        //while(1);
	        }
	}

	printd(Apk_DebugLvl,"tUI_PuSetting.ubDefualtFlag =%d.\n",tUI_PuSetting.ubDefualtFlag);

  }


	
    if (ubFactoryDeleteCam == 1)
    {
        static uint8_t ubCamId = 0;
        if (tUI_CamStatus[ubCamId].ulCAM_ID != INVALID_ID)
        {
            UI_CamDeleteCamera(1, ubCamId);
        }
        ubCamId++;
        if (ubCamId > 4)
        {
            ubCamId = 0;
            ubFactoryDeleteCam = 0;
            WDT_Disable(WDT_RST);
            WDT_RST_Enable(WDT_CLK_EXTCLK, 1);//reboot
            while(1);
        }
    }
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
    APP_EventMsg_t tUI_GetLinkStsMsg = {0};
    uint8_t ubUI_SendMsg2AppFlag = FALSE;
    OSD_IMG_INFO tOsdImgInfo;

    osMutexWait(UI_PUMutex, osWaitForever);
    UI_CLEAR_THREADCNT(tUI_PuSetting.IconSts.ubClearThdCntFlag, *pThreadCnt);
    //	printd(Apk_DebugLvl, "UI_UpdateStatus tUI_SyncAppState: %d.\n", tUI_SyncAppState);
	//printd(Apk_DebugLvl, "UI_UpdateStatus :tUI_PuSetting.tPsMode %d.\n", tUI_PuSetting.tPsMode);

    if (ubUpdateFWFlag == 0)
    	WDT_RST_Enable(WDT_CLK_EXTCLK, WDT_TIMEOUT_CNT);
    switch (tUI_SyncAppState)
    {
    case APP_IDLE_STATE:
        ubUI_SendMsg2AppFlag = ((tUI_State == UI_DISPLAY_STATE)||(tUI_State == UI_SUBSUBMENU_STATE))?TRUE:FALSE;
        break;
    case APP_LOSTLINK_STATE:
	printd(Apk_DebugLvl,"UI_UpdateStatus ubLostLinkEnterSanMode = %d.tUI_PuSetting.ubDefualtFlag = %d",ubLostLinkEnterSanMode,tUI_PuSetting.ubDefualtFlag);

	ubShowAlarmstate  = 0;
    if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
		ubVOXModeToBuRet = rUI_FAIL;
	
    if ((tUI_PuSetting.ubScanTime > 0) && (UI_GetCamOnLineNum(0) != 0))
        {
            if (tUI_PuSetting.ubDefualtFlag == FALSE)
            {
                ubLostLinkEnterSanMode++;
                if (ubLostLinkEnterSanMode == 15)
                {
                    UI_SwitchCameraScan(0);
                    ubLostLinkEnterSanMode = 0;
                }
            }
        }
        else
        {
            ubLostLinkEnterSanMode = 0;
            UI_DisableScanMode();
        }

/*		if ((ubTempAlarmState == HIGH_TEMP_ALARM_ING)||(ubTempAlarmState == LOW_TEMP_ALARM_ING)||(ubPickupAlarmState == PICKUP_ALARM_ING))
		{
			 //if (ubShowAlarmstate == 3)
        	//{
				//if(ubAlarmOnlyAlertFlag == 0)
               	 //UI_ShowAlarm(3);
				//ubAlarmOnlyAlertFlag = 0;
        	//}
			 ubShowAlarmAgain=1;
		}
*/
        if (tUI_PuSetting.ubDefualtFlag == FALSE)
        {
            UI_ShowLostLinkLogo(pThreadCnt);
            (*pThreadCnt)++;
            ubUI_SendMsg2AppFlag = TRUE;
            goto END_UPDATESTS;
        }
        else
        {
            if ((ubFactorySettingFlag == 0)&&tUI_State == UI_DISPLAY_STATE)
            {
                tOsdImgInfo.uwHSize  = 1280;
                tOsdImgInfo.uwVSize  = 720;
                tOsdImgInfo.uwXStart = 48;
                tOsdImgInfo.uwYStart = 0;
                OSD_EraserImg1(&tOsdImgInfo);

                OSD_LogoJpeg(OSDLOGO_LOGO_BG);
                /*
                tOSD_GetOsdImgInfor (1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 0;
                tOsdImgInfo.uwYStart =0;
                tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
                */
//              UI_FS_LangageMenuDisplay(ubLangageFlag);
                UI_FS_MenuDisplay(ubFS_MenuItem);
                ubFactorySettingFlag = 1;
            }
            ubUI_SendMsg2AppFlag = TRUE;
            goto END_UPDATESTS;
        }
        break;
    case APP_LINK_STATE:

        UI_LinkStatusCheck(*pThreadCnt);
        if (tUI_PuSetting.ubDefualtFlag == FALSE)
        {
     		#if UI_TEMPMENU
            UI_TempAlarmCheck();
		#endif
            UI_PickupAlarmCheck();
            UI_MotorStateCheck();

            UI_RedrawStatusBar(pThreadCnt);
            (*pThreadCnt)++;
            ubUI_SendMsg2AppFlag = TRUE;
            goto END_UPDATESTS;
        }
        else
        {
            if ((ubFactorySettingFlag == 0)&&tUI_State == UI_DISPLAY_STATE)
            {
                tOsdImgInfo.uwHSize  = 1280;
                tOsdImgInfo.uwVSize  = 720;
                tOsdImgInfo.uwXStart = 48;
                tOsdImgInfo.uwYStart = 0;
                OSD_EraserImg1(&tOsdImgInfo);

                OSD_LogoJpeg(OSDLOGO_LOGO_BG);
                /*
                tOSD_GetOsdImgInfor (1, OSD_IMG1, OSD1IMG_ANKER_LOGO28, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 0;
                tOsdImgInfo.uwYStart =0;
                tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
                */

                UI_FS_MenuDisplay(ubFS_MenuItem);
//              UI_FS_LangageMenuDisplay(ubLangageFlag);

//              ubFS_MenuItem = 0;
                ubFactorySettingFlag = 1;
            }
            ubUI_SendMsg2AppFlag = TRUE;
            goto END_UPDATESTS;
        }
        break;
    case APP_PAIRING_STATE:
        *pThreadCnt = 0;
        UI_PairStateCheck(*pThreadCnt);
        ubCamPairOkState = 1;
        UI_UpdateBarIcon_Part1();
        UI_DrawPairingStatusIcon();
        osMutexRelease(UI_PUMutex);
        return;
    default:
        break;
    }
    *pThreadCnt = 0;
    tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;

END_UPDATESTS:
//  UI_UpdateBriLvlIcon();
    UI_UpdateVolLvlIcon();
    UI_StatusCheck(*pThreadCnt);
    tUI_PuSetting.IconSts.ubRdPairIconFlag  = FALSE;
    if (ubUI_SendMsg2AppFlag == TRUE)
    {
        tUI_GetLinkStsMsg.ubAPP_Event       = APP_LINKSTATUS_REPORT_EVENT;
        UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
    }

    //UI_ExScan();	

    if(ubVolWarnFlag == 1)
    {
	if(++ubVolWarnCnt == 25)
	{
		UI_CleanSquealAlert();
	}
    }	
    osMutexRelease(UI_PUMutex);
}
//------------------------------------------------------------------------------
void UI_PuInit(void)
{
    if (tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        UI_AutoLcdSetSleepTime(tUI_PuSetting.ubSleepMode);
    }
    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
    {
        ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
        UI_SetSpeaker(2, 0);
    }
    else
    {
        ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
    }

    LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_PuSetting.BriLvL.tBL_UpdateLvL]);
//  LCDBL_ENABLE(UI_ENABLE);

    if ((TRUE == tUI_PuSetting.ubScanModeEn) && (tUI_PuSetting.ubScanTime > 0) && (tUI_PuSetting.ubDefualtFlag == FALSE))
    {
        UI_EnableScanMode();
    }

    //UI_CamvLDCModeCmd(CAMSET_ON);

    if (ubFactoryModeFLag == 1)
    {
        UI_FactoryStatusDisplay();
    }
    printd(Apk_DebugLvl, "UI_PuInit ok.\n");
}

void UI_PowerOnSet(void)
{
    printd(Apk_DebugLvl, "UI_PowerOnSet ubZoomScale: %d.\n", tUI_PuSetting.ubZoomScale);
    UI_Zoom_SetScaleParam(tUI_PuSetting.ubZoomScale);
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
		case FWUPG_EVENT:
			KNL_SDUpgradeFwFunc();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SwitchMode(UI_PowerSaveMode_t tUI_PsMode)
{
//  OSD_IMG_INFO tOsdImgInfo;
    UI_PUReqCmd_t tPsCmd;

    printd(Apk_DebugLvl, "UI_SwitchMode tUI_PsMode: %d.\n", tUI_PsMode);
    if (PS_VOX_MODE == tUI_PsMode)
    {
        if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
            (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
        {
            tPsCmd.tDS_CamNum               = tCamViewSel.tCamViewPool[0];
            tPsCmd.ubCmd[UI_TWC_TYPE]       = UI_SETTING;
            tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
            tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_VOX_MODE;
            tPsCmd.ubCmd_Len                = 3;
            if (UI_SendRequestToBU(osThreadGetId(), &tPsCmd) != rUI_SUCCESS)
            {
                //printd(DBG_ErrorLvl, "VOX Notify Fail !\n");
                //return;
                ubVOXModeToBuRet = rUI_FAIL;
            }
	   else
	   {
		ubVOXModeToBuRet = rUI_SUCCESS;
	   }
			
        }
        tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode = PS_VOX_MODE;
        UI_EnableVox();
    }
    else if (PS_WOR_MODE == tUI_PsMode)
    {
        UI_SetupPuWorMode();
    }
}
//------------------------------------------------------------------------------
void UI_PowerKeyDeal(void)
{
    if (LCDBL_STATE == 0)
    {
        LCDBL_ENABLE(UI_ENABLE);
    }
    else
    {
        LCDBL_ENABLE(UI_DISABLE);
    }
}

void UI_EnterSleep(void)
{
    OSD_IMG_INFO tOsdInfo;

    printd(Apk_DebugLvl, "UI_EnterSleep.\n");

    if (APP_LOSTLINK_STATE == tUI_SyncAppState)
    {
        tOsdInfo.uwHSize  = 672;
        tOsdInfo.uwVSize  = 1280;
        tOsdInfo.uwXStart = 48;
        tOsdInfo.uwYStart = 0;
        OSD_EraserImg2(&tOsdInfo);
        tUI_State = UI_DISPLAY_STATE;
    }
}

void UI_WakeUp(void)
{
    printd(Apk_DebugLvl, "UI_WakeUp.\n");
    if (APP_LOSTLINK_STATE == tUI_SyncAppState)
    {
        //tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
        //ubFastShowLostLinkSta = 1;

        ubEnterTimeMenuFlag =0;
    }

    //if (LCDBL_STATE == 0)
    //{
    //  LCDBL_ENABLE(UI_ENABLE);
    //}

    UI_TimerDeviceEventStart(TIMER1_2, ubAutoSleepTime[tUI_PuSetting.ubSleepMode]*1000*70, UI_AutoLcdSetSleepTimerEvent);

    /*
    if ((TRUE == tUI_PuSetting.ubScanModeEn) && (FALSE == ubUI_ScanStartFlag))
    {
        UI_EnableScanMode();
    }
    */

    //UI_TriggerWakeUpAlarm();
}

void UI_CheckPowerMode(void)
{
    //printd(Apk_DebugLvl, "UI_CheckPowerMode ubPowerState: %d, Sleep: %d, Wakeup: %d.\n", ubPowerState, ubSleepWaitCnt, ubWakeUpWaitCnt);
    if (ubPowerState == PWR_ON)
    {
        ubSleepWaitCnt = 0;
        ubWakeUpWaitCnt = 0;
    }
    else if (ubPowerState == PWR_Prep_Sleep)
    {
        if (LCDBL_STATE == 1)
            LCDBL_ENABLE(UI_DISABLE);
//      UI_DisableScanMode();
        UI_TimerDeviceEventStop(TIMER1_2);
        UI_SwitchMode(Current_Mode);
        ubSleepWaitCnt = 0;
        ubPowerState = PWR_Start_Sleep;
    }
    else if (ubPowerState == PWR_Start_Sleep)
    {
        UI_EnterSleep();
        ubSleepWaitCnt = 0;
        ubPowerState = PWR_Sleep_Complete;
    }
    else if (ubPowerState == PWR_Sleep_Complete)
    {
        ubSleepWaitCnt = 0;
        ubWakeUpWaitCnt = 0;
    }
    else if (ubPowerState == PWR_Prep_Wakeup)
    {
        //ubWakeUpWaitCnt = 0;
        //ubPowerState = PWR_Start_Wakeup;
        UI_WakeUp();
        ubWakeUpWaitCnt = 0;
        ubPowerState = PWR_ON;
    }
    else if (ubPowerState == PWR_Start_Wakeup)
    {
        //UI_WakeUp();
        //ubWakeUpWaitCnt = 0;
        //ubPowerState = PWR_ON;
    }
}

void UI_SetSleepState(uint8_t type)
{
    //static uint8_t ubVoxstatus = false;
    if (type == 0)
    {
        if (ubPowerState == PWR_ON)
        {
            if (ubSwitchNormalFlag == 1)
                ubPowerState = PWR_Prep_Sleep;
        }

        if (ubPowerState == PWR_Sleep_Complete)
        {
            ubPowerState = PWR_Prep_Wakeup;
            //if (ubVoxstatus == false)
            {
                UI_DisableVox();
                UI_SwitchCameraSource();
            }
            if (tUI_PuSetting.ubDefualtFlag == TRUE)
                 ubFactorySettingFlag = 0;
			
            ubSwitchCamWakeupSstate = 1;
        }

    }
    else
    {
        ubPowerState = PWR_Prep_Wakeup;
        UI_DisableVox();
        UI_SwitchCameraSource();
        ubSwitchCamWakeupSstate = 1;
    }
}


void UI_PowerKeyShort(void)
{
    if (ubFactoryModeFLag == 1)
        return;

    if(tUI_SyncAppState == APP_PAIRING_STATE)
        return;
    
 /* if (tUI_PuSetting.ubDefualtFlag == TRUE)
    {
    	tLCD_JpegDecodeDisable();
    }
*/
#if Current_Test
    UI_SetSleepState(0);
#else
    UI_PowerKeyDeal();
#endif

    printd(1, "UI_PowerKeyShort###\n");
}

void UI_PowerOff(void)
{
    printd(Apk_DebugLvl, "UI_PowerOff Power OFF!\n");

    UI_SendPwrNormalModeToBu();
    UI_UpdateDevStatusInfo();
//  BUZ_PlayPowerOffSound();
    osDelay(600);//wait buzzer play finish
    LCDBL_ENABLE(UI_DISABLE);
    RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
    RTC_SetGPO_1(0, RTC_PullDownEnable);
    RTC_PowerDisable();
    while(1);
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
    printd(Apk_DebugLvl, "UI_PowerKey###\n");

    if (ubFactoryModeFLag == 1)
    {
        if ((GPIO->GPIO_I9 == 0) || (GPIO->GPIO_I10 == 0))
            return;
    }

    if (tUI_PuSetting.ubDefualtFlag == TRUE)
    {
        tUI_PuSetting.ubDefualtFlag = 2;
    }

    UI_SendPwrNormalModeToBu();
    UI_UpdateDevStatusInfo();
//  BUZ_PlayPowerOffSound();
    osDelay(600);           //wait buzzer play finish
    LCDBL_ENABLE(UI_DISABLE);
//  POWER_LED_IO  = 0;
//  SIGNAL_LED_IO = 0;
    RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
    RTC_SetGPO_1(0, RTC_PullDownEnable);
    printd(Apk_DebugLvl, "UI_PowerKey Power OFF!\n");
    RTC_PowerDisable();
    while(1);
}
//------------------------------------------------------------------------------
void UI_MenuKey(void)
{
    OSD_IMG_INFO tOsdInfo;
    printd(Apk_DebugLvl, "UI_MenuKey tUI_State: 0x%x.  tUI_PuSetting.VolLvL.ubVOL_UpdateCnt  =%d\n", tUI_State,tUI_PuSetting.VolLvL.ubVOL_UpdateCnt);
    switch (tUI_State)
    {
    case UI_DISPLAY_STATE:
	if(ubFactoryModeFLag == 1)
            return;
	
        if (ubShowAlarmstate > 0)
        {
            return;
        }
        if (tUI_PuSetting.VolLvL.ubVOL_UpdateCnt != 0)
        {
            tOsdInfo.uwXStart = 48;
            tOsdInfo.uwYStart = 1119;
            tOsdInfo.uwHSize  = 672;
            tOsdInfo.uwVSize  = 161;
            OSD_EraserImg2(&tOsdInfo);
            tUI_PuSetting.VolLvL.ubVOL_UpdateCnt = 0;
            return;
        }

        if (ubAutoBrightnessFlag == 0)
        {
            ubSubMenuItemFlag = 0;
            ubSubMenuItemPreFlag = 1;
        }
        else
        {
            ubSubMenuItemFlag = 1;
            ubSubMenuItemPreFlag = 0;
        }
        //brightness_temp = tUI_PuSetting.BriLvL.tBL_UpdateLvL;
        tUI_State = UI_MAINMENU_STATE;
        tUI_MenuItem.ubItemPreIdx = BRIGHT_ITEM;
        tUI_MenuItem.ubItemIdx    = BRIGHT_ITEM;
        UI_DrawMenuPage();
        break;

    case UI_MAINMENU_STATE:
    case UI_SUBMENU_STATE:
    case UI_SUBSUBMENU_STATE:
    case UI_SUBSUBSUBMENU_STATE:
        {
	     if(ubFactoryModeFLag == 1)
           	 return;
	
            if (tUI_SyncAppState == APP_PAIRING_STATE)
            {
                UI_PairingControl(EXIT_ARROW);
                tUI_SyncAppState = APP_LOSTLINK_STATE;
                //tUI_State = UI_DISPLAY_STATE;
                ubMenuKeyPairing = 1;
                return;
            }
            UI_UpdateDevStatusInfo();
            OSD_IMG_INFO tOsdInfo;

            tOsdInfo.uwHSize  = 672;
            tOsdInfo.uwVSize  = 1280;
            tOsdInfo.uwXStart = 48;
            tOsdInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdInfo);

            tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
            if (APP_LOSTLINK_STATE == tUI_SyncAppState)
            {
                if (ubNoAddCamFlag == 1)
                {
                	if(tUI_State == UI_SUBSUBMENU_STATE)
                		tLCD_JpegDecodeDisable();
                    OSD_LogoJpeg(OSDLOGO_WHITE_BG);
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
                    tOsdInfo.uwXStart = 175;
                    tOsdInfo.uwYStart = 279;
                    tOSD_Img2(&tOsdInfo, OSD_QUEUE);

                    if (tUI_PuSetting.ubLangageFlag == 2)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
                        tOsdInfo.uwXStart = 503;
                        tOsdInfo.uwYStart = 360 - 80 - 5;
                        tOSD_Img2(&tOsdInfo, OSD_UPDATE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
                        tOsdInfo.uwXStart = 503;
                        tOsdInfo.uwYStart = 360 - 80;
                        tOSD_Img2(&tOsdInfo, OSD_UPDATE);
                    }
                    printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
                }
                else
                {
                    /*
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL1 + (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdInfo);
                    tOsdInfo.uwXStart= 160;
                    tOsdInfo.uwYStart =328;
                    tOSD_Img2(&tOsdInfo, OSD_QUEUE);

                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL2+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdInfo);
                    tOsdInfo.uwXStart= 480;
                    tOsdInfo.uwYStart =304-104;
                    tOSD_Img2(&tOsdInfo, OSD_UPDATE);
                    */
                      //if(tUI_State = UI_MAINMENU_STATE)
                   // 	tLCD_JpegDecodeDisable();
                    OSD_LogoJpeg(OSDLOGO_LOSTLINK+tUI_PuSetting.ubLangageFlag);
                    printd(Apk_DebugLvl, "UI_MenuKey OSD2IMG_MENU_NOSIGNAL1.\n");
                }
            }
            tUI_State = UI_DISPLAY_STATE;
            ubEnterTimeMenuFlag = 0;
            break;
        }

    /*
    case UI_SUBMENU_STATE:
        {
            tUI_State = UI_MAINMENU_STATE;
            UI_ResetSubMenuInfo();
            if (TRUE == ubUI_FastStateFlag)
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
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_PHOTOPLAYLIST_STATE:		
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(EXIT_ARROW);
			break;
		default:
			break;
	}
}

void UI_MenuLongKey(void)
{
	printd(Apk_DebugLvl,"UI_MenuLongKey\n");
	OSD_IMG_INFO tOsdImgInfo;
	if (ubFactoryModeFLag == 1)
	{
		tUI_PuSetting.ubDefualtFlag = TRUE;

		tUI_PuSetting.ubZoomScale               = 0;
		tUI_PuSetting.VolLvL.tVOL_UpdateLvL     = VOL_LVL4;
		tUI_PuSetting.NightmodeFlag             = 0x0F;
		tUI_PuSetting.ubHighTempSetting         = 0;
		tUI_PuSetting.ubLowTempSetting          = 0;
		tUI_PuSetting.ubTempAlertSetting        = 1;
		tUI_PuSetting.ubSoundLevelSetting       = 0;
		tUI_PuSetting.ubSoundAlertSetting       = 1;
		tUI_PuSetting.ubTempunitFlag            = 0;
		tUI_PuSetting.ubFlickerFlag             = 1;
		tUI_PuSetting.ubSleepMode               = 1;
		tUI_PuSetting.BriLvL.tBL_UpdateLvL      = BL_LVL8;
		tUI_PuSetting.ubScanTime                = 2;
		tUI_PuSetting.ubScanModeEn              = TRUE;

		tUI_PuSetting.ubFeatCode0           = 0x00;
		tUI_PuSetting.ubFeatCode1           = 0x00;
		tUI_PuSetting.ubFeatCode2           = 0x00;

		LCDBL_ENABLE(FALSE);
		//UI_CamDeleteCamera(1, 0);
		printd(Apk_DebugLvl, "factory defulat ~~~\n");
		UI_UpdateDevStatusInfo();
		ubFactorySettingFlag = 0;

		tOsdImgInfo.uwHSize  = 720;
		tOsdImgInfo.uwVSize  = 1280;
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		OSD_EraserImg2(&tOsdImgInfo); //复位重启以后OSD可能会有残留

		UI_PowerOff	();	
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
		case UI_SET_ADOSRC_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
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
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
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
		case UI_SDFWUPG_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
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
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(RIGHT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_EnterKey(void)
{
	if(UI_CheckStopAlarm() == 1)
	{
		UI_StopPlayAlarm();
		return;
	}
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
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_ENGMODE_STATE:
		case UI_PAIRING_STATE:		
			tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(ENTER_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void DisplayStatus(uint8_t  type)
{
    OSD_IMG_INFO tOsdImgInfo;
    if (type == 1)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_P, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_A, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600 - 22*1;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_I, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600 - 22*2;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_R, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600 - 22*3;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_I, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600 - 22*4;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_N, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600 - 22*5;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_G, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 600 - 22*6;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
}

void FactortPair(void)
{
    APP_EventMsg_t tUI_PairMessage = {0};
   // if (ubCamFullFlag == 0)
    //{
        BUZ_PlaySingleSound();
        ubPairDisplayTime = 20; //10
        tPairInfo.tPairSelCam = ubPairSelCamcnt;
        tPairInfo.tDispLocation = ubPairSelCamcnt;

        tUI_PairMessage.ubAPP_Event      = APP_PAIRING_START_EVENT;
        tUI_PairMessage.ubAPP_Message[0] = 2;       //! Message Length
        tUI_PairMessage.ubAPP_Message[1] = ubPairSelCamcnt;//CAM1;
        tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
        tUI_PairMessage.ubAPP_Message[2] = ubPairSelCamcnt;
        UI_SendMessageToAPP(&tUI_PairMessage);
        tPairInfo.ubDrawFlag             = TRUE;
        UI_DisableScanMode();
   // }
}

void UI_EnterLongKey(void)
{
    if (ubFactoryModeFLag == 1)
    {
    	ubFactoryModeFLag = 1;
	tUI_State = UI_DISPLAY_STATE;
        FactortPair();
	 printd(1,"UI_EnterLongKey!!!!!!!  tUI_State =%x,\n",tUI_State);

    }
    //if (ubShowAlarmstate > 0)
            //return;
        
	 printd(1,"UI_EnterLongKey!!!!!!!\n");
#if SD_UPDATE_TEST
    KNL_SDUpgradeFwFunc();
#endif

#if ENG_MODE_TEST
    UI_EngModeKey();
#endif
}
//------------------------------------------------------------------------------
void UI_ShowColorSettingValue(uint8_t ubValue)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint16_t ubYStart = 0;
    uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0;

    ubHunds = ubValue / 100;
    ubTens  = (ubValue - (ubHunds * 100)) / 10;
    ubUnits = ubValue - (ubHunds * 100 + ubTens * 10);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORVALUEMASK_ICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    if (ubHunds)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubTens, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 628;
        tOsdImgInfo.uwYStart = ubYStart = 530 - tOsdImgInfo.uwVSize;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubHunds, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 628;
        tOsdImgInfo.uwYStart = 530;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 628;
        tOsdImgInfo.uwYStart = ubYStart - tOsdImgInfo.uwVSize;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else if (ubTens)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubTens, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 628;
        tOsdImgInfo.uwYStart = ubYStart = 543 - tOsdImgInfo.uwVSize;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 628;
        tOsdImgInfo.uwYStart = ubYStart - tOsdImgInfo.uwVSize;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 628;
        tOsdImgInfo.uwYStart = 530 - tOsdImgInfo.uwVSize;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu1Key(void)
{
    if (tUI_State != UI_DISPLAY_STATE)
    {
        UI_MenuKey();
        return;
    }
    if (APP_LOSTLINK_STATE == tUI_SyncAppState)
        return;
    if (DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
    {
        tCamPreViewSel.tCamViewType = tCamViewSel.tCamViewType;
        UI_CameraSelectionKey();
        return;
    }
    UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu2Key(void)
{
	if((APP_LOSTLINK_STATE == tUI_SyncAppState) ||
	   (SINGLE_VIEW != tCamViewSel.tCamViewType))
		return;

    UI_DrawCameraSettingMenu(UI_CAMFUNC_SETUP);
}
//------------------------------------------------------------------------------
void UI_DrawColorSettingMenu(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSET_BG, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORRGB_ICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORBL_ITEM, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSETUPNOR_ICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSETDNNOR_ICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    UI_ShowColorSettingValue(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorBL);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORRIGHT_ICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawMDSettingScreen(void)
{
#define MD_H_WINDOWSIZE     48
#define MD_V_WINDOWSIZE     64
    OSD_IMG_INFO tOsdImgInfo[3];
    uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
    uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
    uint8_t ubMD_V_WinNum = ulLcd_VSize / MD_V_WINDOWSIZE;
    uint8_t ubMD_H_WinNum = ulLcd_HSize / MD_H_WINDOWSIZE;
    uint8_t ubMD_Idx;

    tOSD_GetOsdImgInfor (1, OSD_IMG1, OSD1IMG_MDROWLINE, 2, &tOsdImgInfo[0]);
    for (ubMD_Idx = 0; ubMD_Idx < ubMD_H_WinNum; ubMD_Idx++)
    {
        tOsdImgInfo[0].uwXStart = (ubMD_Idx * MD_H_WINDOWSIZE);
        tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
    }
    tOsdImgInfo[0].uwXStart = ulLcd_HSize - 5;
    tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
    for (ubMD_Idx = 0; ubMD_Idx < ubMD_V_WinNum; ubMD_Idx++)
    {
        tOsdImgInfo[1].uwYStart = (ubMD_Idx * MD_V_WINDOWSIZE);
        tOSD_Img1(&tOsdImgInfo[1], OSD_QUEUE);
    }
    tOsdImgInfo[1].uwYStart = ulLcd_VSize - 5;
    tOSD_Img1(&tOsdImgInfo[1], OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MDBLOCK_ICON, 1, &tOsdImgInfo[2]);
    tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
    OSD_Weight(OSD_WEIGHT_6DIV8);
}
//------------------------------------------------------------------------------
void UI_DrawCameraSettingMenu(UI_CameraSettingMenu_t tCamSetMenu)
{
    OSD_IMG_INFO tOsdImgInfo;

	switch(tCamSetMenu)
	{
		case UI_CAMISP_SETUP:
			tUI_State = UI_SUBMENU_STATE;
//			tOsdImgInfo.uwXStart = 0;
//			tOsdImgInfo.uwYStart = 0;
//			tOsdImgInfo.uwHSize  = 100;
//			tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
//			OSD_EraserImg1(&tOsdImgInfo);
			OSD_Weight(OSD_WEIGHT_7DIV8);
			tUI_MenuItem.ubItemIdx = CAMERAS_ITEM;
			UI_DrawSubMenuPage(CAMERAS_ITEM);
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

    switch (tArrowKey)
    {
    case LEFT_ARROW:
        if (UI_COLOR_ITEM == tUI_CamSetItem)
            return;
        tUI_PrevCamSetItem = tUI_CamSetItem;
        --tUI_CamSetItem;
        break;
    case RIGHT_ARROW:
        if (UI_MD_ITEM == tUI_CamSetItem)
            return;
        tUI_PrevCamSetItem = tUI_CamSetItem;
        ++tUI_CamSetItem;
        break;
    case ENTER_ARROW:
    case EXIT_ARROW:
        UI_ClearBuConnectStatusFlag();
        tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
        tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 0;
        OSD_EraserImg1(&tOsdImgInfo);
        if (ENTER_ARROW == tArrowKey)
        {
            if (UI_COLOR_ITEM == tUI_CamSetItem)
            {
                UI_DrawColorSettingMenu();
                tUI_State = UI_SET_CAMCOLOR_STATE;
                return;
            }
            if (UI_DPTZ_ITEM == tUI_CamSetItem)
            {
                uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
                uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
                uint8_t ubArrowNum;

                tUI_DptzParam.tScaleParam                            = UI_SCALEUP_2X;
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize/tUI_DptzParam.tScaleParam;
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize/tUI_DptzParam.tScaleParam;
                tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize      = ulLcd_HSize;
                tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize      = ulLcd_VSize;
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
                tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
                for (ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, (OSD2IMG_DPTZUPARROWNOR_ICON+(ubArrowNum*2)), 1, &tOsdImgInfo);
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DPTZZOOMMSG_ICON, 1, &tOsdImgInfo);
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                tUI_CamSetItem = UI_COLOR_ITEM;
                tUI_State = UI_DPTZ_CONTROL_STATE;
                return;
            }
            if (UI_MD_ITEM == tUI_CamSetItem)
            {
                UI_DrawMDSettingScreen();
                tUI_CamSetItem = UI_COLOR_ITEM;
                tUI_State = UI_MD_WINDOW_STATE;
                return;
            }
        }
        tUI_CamSetItem = UI_COLOR_ITEM;
        tUI_State = UI_DISPLAY_STATE;
        return;
    default:
        return;
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, (uwOsdImgIdx[tUI_CamSetItem]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, uwOsdImgIdx[tUI_PrevCamSetItem], 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_CameraColorSetting(UI_ArrowKey_t tArrowKey)
{
    static uint8_t ubUI_ColorSetItem = UI_IMGBL_SETTING;
    OSD_IMG_INFO tOsdImgInfo;
    UI_PUReqCmd_t tCamSetColorCmd;
    uint8_t *pUI_ColorParm[4] = {(uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorBL,
                                 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorContrast,
                                 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorSaturation,
                                 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorHue};

    tCamSetColorCmd.tDS_CamNum              = tCamViewSel.tCamViewPool[0];
    tCamSetColorCmd.ubCmd[UI_TWC_TYPE]      = UI_SETTING;
    tCamSetColorCmd.ubCmd[UI_SETTING_ITEM]  = UI_IMGPROC_SETTING;
    switch (tArrowKey)
    {
    case LEFT_ARROW:
        if (UI_IMGBL_SETTING == ubUI_ColorSetItem)
            return;
        if (UI_IMGHUE_SETTING == ubUI_ColorSetItem)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORRIGHTMASK_ICON, 1, &tOsdImgInfo);
            tOsdImgInfo.uwYStart = 1181;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORRIGHT_ICON, 1, &tOsdImgInfo);
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        --ubUI_ColorSetItem;
        break;
    case RIGHT_ARROW:
        if (UI_IMGHUE_SETTING == ubUI_ColorSetItem)
            return;
        if (++ubUI_ColorSetItem == UI_IMGHUE_SETTING)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORRIGHTMASK_ICON, 1, &tOsdImgInfo);
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORLEFT_ICON, 1, &tOsdImgInfo);
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        break;
    case DOWN_ARROW:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSETDNHL_ICON, 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        if (*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING] > 0)
        {
            tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = ubUI_ColorSetItem;
            tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = --(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
            tCamSetColorCmd.ubCmd_Len                = 4;
            if (UI_SendRequestToBU(osThreadGetId(), &tCamSetColorCmd) == rUI_SUCCESS)
            {
                UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
                UI_UpdateDevStatusInfo();
            }
            else
                ++(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
        }
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSETDNNOR_ICON, 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        return;
    case UP_ARROW:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSETUPHL_ICON, 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        if (*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING] < 127)
        {
            tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = ubUI_ColorSetItem;
            tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = ++(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
            tCamSetColorCmd.ubCmd_Len                = 4;
            if (UI_SendRequestToBU(osThreadGetId(), &tCamSetColorCmd) == rUI_SUCCESS)
            {
                UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
                UI_UpdateDevStatusInfo();
            }
            else
                --(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
        }
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_COLORSETUPNOR_ICON, 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        return;
    case EXIT_ARROW:
        UI_ClearBuConnectStatusFlag();
        tOsdImgInfo.uwXStart = 580;
        tOsdImgInfo.uwYStart = 0;
        tOsdImgInfo.uwHSize  = 140;
        tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
        OSD_EraserImg2(&tOsdImgInfo);
        ubUI_ColorSetItem = UI_IMGBL_SETTING;
        tUI_State = UI_DISPLAY_STATE;
        return;
    default:
        return;
    }
    UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, (OSD2IMG_COLORBL_ITEM + (ubUI_ColorSetItem - UI_IMGBL_SETTING)), 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx)
{
//  OSD_IMG_INFO tOsdImgInfo;
    /*
    uint16_t uwUI_ArrowOsdImgIdx[] = {
        [AKEY_UP]    = OSD2IMG_DPTZUPARROWHL_ICON,
        [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWHL_ICON,
        [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWHL_ICON,
        [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWHL_ICON
    };
    */

    if (ubKeyID == UiKeyEventMap[ubKeyMapIdx].ubKeyID)
    {
        /* modify by wjb
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        */
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

    tOSD_GetOsdImgInfor (1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey)
{
    #define PT_STEP     10
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t ubArrowNum;

    switch (tArrowKey)
    {
    case UP_ARROW:
        if (tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart == 0)
            return;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart -= PT_STEP;
        break;
    case DOWN_ARROW:
        if ((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart +
            tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize)
            return;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart += PT_STEP;
        break;
    case LEFT_ARROW:
        if ((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart +
            tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize)
            return;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart += PT_STEP;
        break;
    case RIGHT_ARROW:
        if (tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart == 0)
            return;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart -= PT_STEP;
        break;
    case ENTER_ARROW:
        if (tUI_DptzParam.tScaleParam == UI_SCALEUP_2X)
        {
            tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize /= UI_SCALEUP_2X;
            tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize /= UI_SCALEUP_2X;
            tUI_DptzParam.tScaleParam = UI_SCALEUP_4X;
        }
        else if (tUI_DptzParam.tScaleParam == UI_SCALEUP_4X)
        {
            uint16_t uwCropHsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize*UI_SCALEUP_2X;
            uint16_t uwCropVsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize*UI_SCALEUP_2X;
            uint16_t uwPrevCropHsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize;
            uint16_t uwPrevCropVsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize;
            if (tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart < ((uwCropHsize - uwPrevCropHsize)/2))
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
            if ((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + ((uwCropVsize - uwPrevCropVsize)/2)) >= uwCropVsize)
                tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = uwCropVsize;
            tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize = uwCropHsize;
            tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize = uwCropVsize;
            tUI_DptzParam.tScaleParam = UI_SCALEUP_2X;
        }
        break;
    case EXIT_ARROW:
        {
            UI_ClearStatusBarOsdIcon();
            for (ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
            {
                if (ubArrowNum < 2)
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
            tUI_DptzParam.tScaleParam                          = UI_SCALEUP_2X;
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
void UI_MD_Window(UI_ArrowKey_t tArrowKey)
{
	tArrowKey =tArrowKey;
}
//------------------------------------------------------------------------------
void UI_CameraSelectionKey(void)
{
	
}
//------------------------------------------------------------------------------
void UI_CameraSelection(UI_ArrowKey_t tArrowKey)
{

}
//------------------------------------------------------------------------------
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey)
{

}
//------------------------------------------------------------------------------
void UI_ChangeAudioSourceKey(void)
{

}
//------------------------------------------------------------------------------
void UI_ChangeAudioSource(UI_ArrowKey_t tArrowKey)
{
    static UI_CamNum_t tAdoCamNumSel;
    static uint8_t ubUI_UpdateAdoCamSelFlag = FALSE;
    UI_CamNum_t tPreAdoCamNum = tAdoCamNumSel;
    uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?150:0;
    uint16_t uwYItemOffset = 150;
    OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_UpdateAdoCamSelFlag)
	{
		tAdoCamNumSel = tUI_PuSetting.tAdoSrcCamNum;
		ubUI_UpdateAdoCamSelFlag = TRUE;
	}
	tPreAdoCamNum = tAdoCamNumSel;
	switch(tArrowKey)
	{
		case UP_ARROW:
			UI_CameraSelectionKey();
			return;
		case DOWN_ARROW:

			return;
		case LEFT_ARROW:
			if(tAdoCamNumSel == CAM1)
				return;
			--tAdoCamNumSel;
			break;
		case RIGHT_ARROW:
			if((tAdoCamNumSel + 1) >= tUI_PuSetting.ubTotalBuNum)
				return;
			if((tUI_CamStatus[tAdoCamNumSel + 1].ulCAM_ID == INVALID_ID) ||
			   (tUI_CamStatus[tAdoCamNumSel + 1].tCamConnSts == CAM_OFFLINE))
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
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?450:0;
			tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?300:255;
			tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?335:uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_UpdateAdoCamSelFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tPreAdoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tAdoCamNumSel*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define UI_MENUICON_NUM     7
#define UI_WRICON_OFFSET    2
//------------------------------------------------------------------------------
void UI_ChangeVideoMode(UI_ArrowKey_t tArrowKey)
{
	tArrowKey = tArrowKey;
}
//------------------------------------------------------------------------------
void UI_PuPowerSaveKey(void)
{
    OSD_IMG_INFO tOsdImgInfo[4];

    if ((APP_LOSTLINK_STATE == tUI_SyncAppState)
    || (tUI_State != UI_DISPLAY_STATE)
    || (DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
    || (PS_VOX_MODE == tUI_PuSetting.tPsMode))
        return;

    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_VOXOPT_ICON, 4, &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
    tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
    tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
    tUI_State = UI_SET_PUPSMODE_STATE;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuVoxMode(void)
{
	UI_PUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;
	UI_Result_t tVoxRet = rUI_FAIL, tBuNotifyRet = rUI_SUCCESS;;

	for(tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
	{
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum 				= tCamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_VOX_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		tBuNotifyRet = UI_SendRequestToBU(osThreadGetId(), &tPsCmd);
		if(rUI_SUCCESS == tBuNotifyRet)
			tVoxRet = rUI_SUCCESS;
		else
			printd(DBG_ErrorLvl, "CAM%d:VOX Notify Fail !\n", (tCamNum + 1));
		tUI_CamStatus[tCamNum].tCamPsMode = PS_VOX_MODE;
	}
	if(rUI_SUCCESS == tVoxRet)
	{
		APP_EventMsg_t tUI_PsMessage = {0};

		LCDBL_ENABLE(UI_DISABLE);
		tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
		tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
		tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
		tUI_PsMessage.ubAPP_Message[2] = TRUE;
		UI_SendMessageToAPP(&tUI_PsMessage);
		tUI_PuSetting.tPsMode = PS_VOX_MODE;
		UI_UpdateDevStatusInfo();
	}
	return tVoxRet;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuWorMode(void)
{
    UI_PUReqCmd_t tPsCmd;
    UI_CamNum_t tCamNum;
    UI_Result_t tWorRet = rUI_FAIL, tBuNotifyRet = rUI_SUCCESS;

    tPsCmd.ubCmd[UI_TWC_TYPE]       = UI_SETTING;
    tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_WORMODE_SETTING;
    tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_WOR_MODE;
    tPsCmd.ubCmd_Len                = 3;
/*    for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
    {
       //if (CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
		//continue;
        tPsCmd.tDS_CamNum = tCamNum;
        tBuNotifyRet = UI_SendRequestToBU(osThreadGetId(), &tPsCmd);
        if (rUI_SUCCESS == tBuNotifyRet)
            tWorRet = rUI_SUCCESS;
        else
            printd(DBG_ErrorLvl, "CAM%d:WOR Setting Fail !\n", (tCamNum + 1));
    }
*/
	printd(1, "tCamViewSel.tCamViewPool[0]] %d  tCamConnSts %d \n", tCamViewSel.tCamViewPool[0],tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts);

      if(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE)
      	{
	        tPsCmd.tDS_CamNum = tCamViewSel.tCamViewPool[0];
	        tBuNotifyRet = UI_SendRequestToBU(osThreadGetId(), &tPsCmd);
	        if (rUI_SUCCESS == tBuNotifyRet)
	            tWorRet = rUI_SUCCESS;
	        else
	            printd(DBG_ErrorLvl, "CAM%d:WOR Setting Fail !\n", (tCamViewSel.tCamViewPool[0] + 1));
      	}
	  else
	  {
	        APP_EventMsg_t tUI_PsMessage = {0};

	        tUI_PuSetting.tPsMode = PS_WOR_MODE;
	        UI_UpdateDevStatusInfo();
	        tUI_PsMessage.ubAPP_Event       = APP_POWERSAVE_EVENT;
	        tUI_PsMessage.ubAPP_Message[0]  = 3;        //! Message Length
	        tUI_PsMessage.ubAPP_Message[1]  = PS_WOR_MODE;
	        tUI_PsMessage.ubAPP_Message[2]  = FALSE;
	        tUI_PsMessage.ubAPP_Message[3]  = CAM1;
	        UI_SendMessageToAPP(&tUI_PsMessage);
	        printd(Apk_DebugLvl, "UI_SetupPuWorMode!\n");
	  }
	  
    if (rUI_SUCCESS == tWorRet)
    {
        APP_EventMsg_t tUI_PsMessage = {0};

        tUI_PuSetting.tPsMode = PS_WOR_MODE;
        UI_UpdateDevStatusInfo();
        tUI_PsMessage.ubAPP_Event       = APP_POWERSAVE_EVENT;
        tUI_PsMessage.ubAPP_Message[0]  = 3;        //! Message Length
        tUI_PsMessage.ubAPP_Message[1]  = PS_WOR_MODE;
        tUI_PsMessage.ubAPP_Message[2]  = FALSE;
        tUI_PsMessage.ubAPP_Message[3]  = CAM1;
        UI_SendMessageToAPP(&tUI_PsMessage);
        printd(Apk_DebugLvl, "UI_SetupPuWorMode!\n");
    }
    return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupBuEcoMode(UI_CamNum_t tECO_CamNum)
{
    APP_EventMsg_t tUI_PsMessage = {0};

    tUI_PsMessage.ubAPP_Event       = APP_POWERSAVE_EVENT;
    tUI_PsMessage.ubAPP_Message[0]  = 4;        //! Message Length
    tUI_PsMessage.ubAPP_Message[1]  = PS_ECO_MODE;
    if ((POWER_NORMAL_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode) &&
       (FALSE == ulUI_MonitorPsFlag[tECO_CamNum]))
    {
        UI_PUReqCmd_t tPsCmd;

        tPsCmd.tDS_CamNum               = tECO_CamNum;
        tPsCmd.ubCmd[UI_TWC_TYPE]       = UI_SETTING;
        tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_ECOMODE_SETTING;
        tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_ECO_MODE;
        tPsCmd.ubCmd_Len                = 3;
        if (UI_SendRequestToBU(osThreadGetId(), &tPsCmd) == rUI_SUCCESS)
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
    else if (PS_ECO_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode)
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
    OSD_IMG_INFO tOsdImgInfo;
    uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELECOCAM1ONLINE_ICON,   OSD2IMG_SELECOCAM1ONLINE_ICON,
                                  OSD2IMG_SELECOCAM2_1ONLINE_ICON, OSD2IMG_SELECOCAM2_1OFFLINE_ICON};
    uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELECOCAM1ONLINE_ICON, OSD2IMG_SELECOCAM1OFFLINE_ICON,
                                  OSD2IMG_SELECOCAM2ONLINE_ICON, OSD2IMG_SELECOCAM2OFFLINE_ICON,
                                  OSD2IMG_SELECOCAM3ONLINE_ICON, OSD2IMG_SELECOCAM3OFFLINE_ICON,
                                  OSD2IMG_SELECOCAM4ONLINE_ICON, OSD2IMG_SELECOCAM4OFFLINE_ICON};
    uint16_t uwDisplayImgIdx;
    uint16_t uwXOffset = 5, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?155:0;
    uint16_t uwYItemOffset = 150, uwYEcoOffset = 150;
    UI_CamNum_t tCamNum;

    /*
    if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
    {
        UI_DisableVox();
        return;
    }
    */

    if ((tUI_State != UI_DISPLAY_STATE) ||
       (TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag))
        return;

    if ((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
       (SINGLE_VIEW == tCamViewSel.tCamViewType))
    {
        UI_SetupBuEcoMode(tCamViewSel.tCamViewPool[0]);
        return;
    }
    tUI_BuEcoCamNum = NO_CAM;
    for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
    {
        uwYEcoOffset     = 0;
        uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];
        if ((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) ||
           ((tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE) && (PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)))
        {
            uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1OFFLINE_ICON:(uwDisplayImgIdx+1);
        }
        else
        {
            if (PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
            {
                uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1ONLINE_ICON:(uwDisplayImgIdx-30);
                uwYEcoOffset    = (((DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum) && (CAM2 == tCamNum)) ||
                                   ((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (CAM4 == tCamNum)))?0:150;
            }
            tUI_BuEcoCamNum = (NO_CAM == tUI_BuEcoCamNum)?tCamNum:tUI_BuEcoCamNum;
        }
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);
        tOsdImgInfo.uwYStart -= (uwYOffset + uwYEcoOffset);
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    if (NO_CAM != tUI_BuEcoCamNum)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart += uwXOffset;
        tOsdImgInfo.uwYStart -= ((tUI_BuEcoCamNum*111) + uwYOffset + uwYItemOffset);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
    tUI_State = UI_SET_BUECOMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_PuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	static UI_PowerSaveMode_t tUI_PsMode = PS_VOX_MODE;
	UI_PowerSaveMode_t tUI_PrePsMode = PS_WOR_MODE;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(PS_VOX_MODE == tUI_PsMode)
				return;
			tUI_PsMode    = PS_VOX_MODE;
			tUI_PrePsMode = PS_WOR_MODE;
			break;
		case RIGHT_ARROW:
			if(PS_WOR_MODE == tUI_PsMode)
				return;
			tUI_PsMode    = PS_WOR_MODE;
			tUI_PrePsMode = PS_VOX_MODE;
			break;
		case ENTER_ARROW:
			if(PS_VOX_MODE == tUI_PsMode)
			{
				UI_SetupPuVoxMode();
			}
			else if(PS_WOR_MODE == tUI_PsMode)
			{
				UI_SetupPuWorMode();
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
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PSSELECTNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwYStart -= (tUI_PrePsMode * 183);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PSSELECTHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwYStart -= (tUI_PsMode * 183);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_BuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
    OSD_IMG_INFO tOsdImgInfo;
    UI_CamNum_t tEcoPrevCamNum = tUI_BuEcoCamNum;
    uint16_t uwXOffset = 5, uwYOffset = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?155:0;
    uint16_t uwYItemOffset = 150;

    switch (tArrowKey)
    {
    case LEFT_ARROW:
    case RIGHT_ARROW:
        tUI_BuEcoCamNum = UI_ChangeSelectCamNum4UiMenu(&tEcoPrevCamNum, &tArrowKey);
        if (tUI_BuEcoCamNum >= tUI_PuSetting.ubTotalBuNum)
        {
            tUI_BuEcoCamNum = tEcoPrevCamNum;
            return;
        }
        break;
    case ENTER_ARROW:
        if (UI_SetupBuEcoMode(tUI_BuEcoCamNum) == rUI_FAIL)
            return;
    case EXIT_ARROW:
        tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
        if (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
            UI_ClearBuConnectStatusFlag();
        tOsdImgInfo.uwXStart  = 150;
        tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?450:400;
        tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?300:210;
        tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?335:500;
        OSD_EraserImg2(&tOsdImgInfo);
//      if (ENTER_ARROW == tArrowKey)
//          osDelay(800);
        tUI_State = UI_DISPLAY_STATE;
        return;
    default:
        return;
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart += uwXOffset;
    tOsdImgInfo.uwYStart -= ((tEcoPrevCamNum*111) + uwYOffset + uwYItemOffset);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart += uwXOffset;
    tOsdImgInfo.uwYStart -= ((tUI_BuEcoCamNum*111) + uwYOffset + uwYItemOffset);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_PushTalkKeyShort(void)
{
    static uint8_t UI_PushTalkFlag = TRUE;
    //if(UI_PushTalkFlag == TRUE)
    {
        printd(Apk_DebugLvl,"UI_PushTalkKeyShort\n");
        switch (tUI_State)
        {
        case UI_MAINMENU_STATE:
            {
                OSD_IMG_INFO tOsdInfo;
                tOsdInfo.uwHSize  = 672;
                tOsdInfo.uwVSize  = 1280;
                tOsdInfo.uwXStart = 48;
                tOsdInfo.uwYStart = 0;
                OSD_EraserImg2(&tOsdInfo);

                tUI_PuSetting.IconSts.ubDrawStsIconFlag = FALSE;
                if (APP_LOSTLINK_STATE == tUI_SyncAppState)
                {
                    tLCD_JpegDecodeDisable();

                    if (ubNoAddCamFlag == 1)
                    {
                        OSD_LogoJpeg(OSDLOGO_WHITE_BG);
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
                        tOsdInfo.uwXStart = 175;
                        tOsdInfo.uwYStart = 279;
                        tOSD_Img2(&tOsdInfo, OSD_QUEUE);

                        if (tUI_PuSetting.ubLangageFlag == 2)
                        {
                            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
                            tOsdInfo.uwXStart = 503;
                            tOsdInfo.uwYStart = 360 - 80 - 5;
                            tOSD_Img2(&tOsdInfo, OSD_UPDATE);
                        }
                        else
                        {
                            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdInfo);
                            tOsdInfo.uwXStart = 503;
                            tOsdInfo.uwYStart = 360 - 80;
                            tOSD_Img2(&tOsdInfo, OSD_UPDATE);
                        }
                        printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
                    }
                    else
                    {
                        OSD_LogoJpeg(OSDLOGO_LOSTLINK+tUI_PuSetting.ubLangageFlag);
                        printd(Apk_DebugLvl, "UI_MenuKey OSD2IMG_MENU_NOSIGNAL1.\n");
                    }
                }
                tUI_State = UI_DISPLAY_STATE;
                UI_PushTalkFlag = FALSE;
                break;
            }

        case UI_DISPLAY_STATE:
            if (ubFactoryModeFLag == 0)
            {
                OSD_IMG_INFO tOsdInfo;
                if (UI_CheckStopAlarm() == 1)
                {
                    UI_StopPlayAlarm();
                    return;
                }

                if(APP_LOSTLINK_STATE == tUI_SyncAppState)
                {
                    if (ubNoAddCamFlag == 1)
                        return;
                }
                tOsdInfo.uwHSize  = 672;
                tOsdInfo.uwVSize  = 1280;
                tOsdInfo.uwXStart = 48;
                tOsdInfo.uwYStart = 0;
                OSD_EraserImg2(&tOsdInfo);
                break;
            }
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
            UI_PushTalkFlag = FALSE;
            break;
        default:
            break;
        }
    }
    //else
    //{
    //  UI_PushTalkFlag = TRUE;
    //}
}



void UI_PushTalkKey(void)
{
    OSD_IMG_INFO tOsdInfo;
    uint8_t CmdData = 0;
	

    if(APP_LOSTLINK_STATE == tUI_SyncAppState)
        return;

     if (ubFactoryModeFLag== 1)
     {
     	if(ubEnterFactoryDelCam == 0)
     	{
     	    if(ubFactoryDelCamFlag == 0)	
	    	 ubFactoryDelCamFlag = 1;
	     ubEnterFactoryDelCam =1;
	}
    
        if(FALSE == ubUI_PttStartFlag)	
        	{
		ubEnterFactoryDelCam =0;
	}

	return;	
     }	

   // printd(Apk_DebugLvl, "UI_PushTalkKey ubUI_PttStartFlag: %d.\n", ubUI_PttStartFlag);
    if(TRUE == ubUI_PttStartFlag)
    {
        if(((ubTempAlarmState > TEMP_ALARM_IDLE)&&(ubTempAlarmState < TEMP_ALARM_OFF)) ||
            ((ubPickupAlarmState > PICKUP_ALARM_IDLE)&&(ubPickupAlarmState < PICKUP_ALARM_OFF)) ||
            (tUI_SyncAppState != APP_LINK_STATE) || (ubShowAlarmstate > 0))
        {
            ubDisplaymodeFlag = 0;
            return;
        }
    }

    if (ubFactoryModeFLag == 0)
    {
        if(ubShowAlarmstate == 0)
        {
            tOsdInfo.uwHSize  = 672;
            tOsdInfo.uwVSize  = 1280;
            tOsdInfo.uwXStart = 48;
            tOsdInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdInfo);
        }
    }
    ubDisplaymodeFlag = 1;
    ubMenuKeyPairing = 0;
    tUI_State = UI_DISPLAY_STATE;

    APP_EventMsg_t tUI_PttMessage = {0};

    tUI_PttMessage.ubAPP_Event      = APP_PTT_EVENT;
    tUI_PttMessage.ubAPP_Message[0] = 1;        //! Message Length
    tUI_PttMessage.ubAPP_Message[1] = ubUI_PttStartFlag;
    UI_SendMessageToAPP(&tUI_PttMessage);
//  SPEAKER_EN(((TRUE == ubUI_PttStartFlag)?UI_DISABLE:UI_ENABLE));
    if (TRUE == ubUI_PttStartFlag)
    {
        UI_TimerDeviceEventStop(TIMER1_2);
        UI_DisableScanMode();
        UI_SetSpeaker(0, 0);
        CmdData = UI_SET_TALK_ON_CMD;
        UI_SendToBUCmd(&CmdData, 1);
    }
    else
    {
        CmdData = UI_SET_TALK_OFF_CMD;
        UI_SendToBUCmd(&CmdData, 1);
        UI_SetSpeaker(0, 1);
        if ((TRUE == tUI_PuSetting.ubScanModeEn) && (FALSE == ubUI_ScanStartFlag))
        {
            UI_EnableScanMode();
        }
        if (tUI_PuSetting.ubSleepMode)
        {
            UI_TimerDeviceEventStart(TIMER1_2, ubAutoSleepTime[tUI_PuSetting.ubSleepMode]*1000*60, UI_AutoLcdSetSleepTimerEvent);
        }
    }
}
//------------------------------------------------------------------------------
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey)
{
    OSD_IMG_INFO tOsdImgInfo;
//  uint16_t uwOsdImgIdx = 0;
    uint8_t *pT_Num[3] = {(uint8_t *)(&ubTimeHour),
                        (uint8_t *)(&ubTimeMin), (uint8_t *)(&ubTimeAMPM)
                        };
    uint8_t ubT_MaxNum[3] = {12, 59, 1};
    uint8_t ubT_MinNum[3] = {0,  0,  0};
//  UI_CamNum_t tSelCam;
//  UI_PUReqCmd_t tPsCmd;
#if 1
    if (ubFactoryModeFLag == 1)
    {
    	    if (tUI_SyncAppState == APP_LINK_STATE)
    	    {
		       uint8_t CmdData;
		    	if(tArrowKey == LEFT_ARROW ||tArrowKey == RIGHT_ARROW || tArrowKey == UP_ARROW || tArrowKey == DOWN_ARROW);
			{
			   CmdData = UI_SET_BUMOTOR_SPEED_H_CMD;
			 // CmdData = UI_SET_BUMOTOR_SPEED_L_CMD;
			   UI_SendToBUCmd(&CmdData, 1);
			   printd(1," SEND BU CMD SUCCESS!\n");	 
			}	
			switch (tArrowKey)
		        {
		        case LEFT_ARROW:
		            UI_MotorControl(MC_LEFT_ON);
		            break;

		        case RIGHT_ARROW:
		            UI_MotorControl(MC_RIGHT_ON);
		            break;

		        case UP_ARROW:
		            UI_MotorControl(MC_UP_ON);
		            break;

		        case DOWN_ARROW:
		            UI_MotorControl(MC_DOWN_ON);
		            break;

		        default:
		            break;
		        }
		        return;
    	    }
    }
    else
    {
    	    if (tUI_SyncAppState == APP_LINK_STATE)
    	    {
		       uint8_t CmdData;
		    	if(tArrowKey == LEFT_ARROW ||tArrowKey == RIGHT_ARROW || tArrowKey == UP_ARROW || tArrowKey == DOWN_ARROW);
			{
			   CmdData = UI_SET_BUMOTOR_NORMAL_CMD;
			   UI_SendToBUCmd(&CmdData, 1);
			   printd(1," ALREADLY  SEND BU NORMAL CMD !\n");	 
			}
    		}
    }
	#else
    if (ubFactoryModeFLag == 1)
    {
    	printd(1,"UI_DisplayArrowKeyFunc yesyesyesyesyes\n");
        switch (tArrowKey)
        {
        case LEFT_ARROW:
            UI_MotorControl(MC_LEFT_ON);
            break;

        case RIGHT_ARROW:
            UI_MotorControl(MC_RIGHT_ON);
            break;

        case UP_ARROW:
            UI_MotorControl(MC_UP_ON);
            break;

        case DOWN_ARROW:
            UI_MotorControl(MC_DOWN_ON);
            break;

        default:
            break;
        }
        return;
    }
#endif	
    if ((tArrowKey == ENTER_ARROW)&&(tUI_PuSetting.ubDefualtFlag == FALSE))
    {
        //if (TRUE == tUI_PuSetting.ubScanModeEn)
        //  return;
        if (UI_CheckStopAlarm() == 1)
        {
            UI_StopPlayAlarm();
            return;
        }

#if 0
        tSelCam = tCamViewSel.tCamViewPool[0] ;

        if (tSelCam < (ubPairSelCam - 1))
        {
            tSelCam = tSelCam + 1;
        }
        else if (tSelCam ==(ubPairSelCam - 1))
        {
            tSelCam = 0;
        }

        if ((tUI_CamStatus[tSelCam].ulCAM_ID != INVALID_ID) &&
             //(tUI_CamStatus[tSelCam].tCamConnSts == CAM_ONLINE) &&
             (tSelCam != tCamViewSel.tCamViewPool[0]))
        {
            tCamViewSel.tCamViewType    = SINGLE_VIEW;
            tCamViewSel.tCamViewPool[0]     = tSelCam;
            tUI_PuSetting.tAdoSrcCamNum = tSelCam;
            UI_SwitchCameraSource();
            UI_ClearBuConnectStatusFlag();

            ubSetViewCam = tCamViewSel.tCamViewPool[0];

            UI_UpdateDevStatusInfo();
        }
#else
        printd(Apk_DebugLvl,"UI_ScanModeExec\n");
        UI_SwitchCameraScan(1); //20180622
#endif
    }

    if (tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
            return;
    }

    if ((tArrowKey >= UP_ARROW) && (tArrowKey <= RIGHT_ARROW) && (UI_GetAlarmStatus()))
        return;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		{
			if(tUI_PuSetting.ubDefualtFlag == FALSE)
			{				
				UI_MotorControl(MC_LEFT_ON);
				break;
			}
			else
			{
				//if (ubFactoryModeFLag == 0)
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
		}
		case RIGHT_ARROW:
		{	
			if(tUI_PuSetting.ubDefualtFlag == TRUE)
			{
				//if (ubFactoryModeFLag == 0)
				{
					if(ubFS_MenuItem == 0)
					{
						#if UI_NOTIMEMENU
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
							ubFastShowLostLinkSta = 1;
						#else
							return;
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
						#endif	
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
							return;
						}	
						UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
					}
				}
			}
			
			else
			{				
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
				//if (ubFactoryModeFLag == 0)
				{
					if(ubFS_MenuItem == 0)
					{
						if(0 == ubLangageFlag) 
							ubLangageFlag = 3;
						else  
							ubLangageFlag--;	
						
						UI_FS_LangageMenuDisplay(ubLangageFlag);
					}
					else
					{
						if(ubFS_Timeitem == 3 || ubFS_Timeitem ==4)
							return;
						if(NULL == pT_Num[ubFS_Timeitem])
							return;

			                    if (ubFS_Timeitem == 2)
			                    {
			                        if ((ubTimeHour == 12)&&(ubTimeAMPM == 1))
			                            return;
			                        if ((ubTimeHour == 0)&&(ubTimeAMPM == 0))
			                            return;
	                  			  }

			                    if (*pT_Num[ubFS_Timeitem] == ubT_MaxNum[ubFS_Timeitem])
			                        *pT_Num[ubFS_Timeitem] = ubT_MinNum[ubFS_Timeitem];
			                    else
			                        (*pT_Num[ubFS_Timeitem])++;

			                    if (ubFS_Timeitem == 0)
			                    {
			                        if ((ubTimeHour == 12)&&(ubTimeAMPM == 0))
			                        {
			                            ubTimeHour = 0;
			                        }
			                        if ((ubTimeHour == 0)&&(ubTimeAMPM == 1))
			                        {
			                            ubTimeHour = 1;
			                        }
		                    		}
	                    			UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
	                		}
	               		 break;
				}
			}
        }
    case DOWN_ARROW:
        {
            if (tUI_PuSetting.ubDefualtFlag == FALSE)
            {
#if 0
                if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL <= VOL_LVL0)
                    break;

                tUI_PuSetting.VolLvL.tVOL_UpdateLvL--;
                UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
#endif

				UI_MotorControl(MC_DOWN_ON);
				break;
		}
		else
		{
			//if (ubFactoryModeFLag == 0)
			{
				if(ubFS_MenuItem == 0)
				{
					if(++ubLangageFlag>=4)
						ubLangageFlag = 0;	
					UI_FS_LangageMenuDisplay(ubLangageFlag);
				}
				else
				{
					if(ubFS_Timeitem == 3 || ubFS_Timeitem ==4)
						return;
			
					if(NULL == pT_Num[ubFS_Timeitem])
						return;

			                if (ubFS_Timeitem == 2)
			                {
			                    if ((ubTimeHour == 12)&&(ubTimeAMPM == 1))
			                        return;
			                    if ((ubTimeHour == 0)&&(ubTimeAMPM == 0))
			                        return;
			                }

			                if (*pT_Num[ubFS_Timeitem] == ubT_MinNum[ubFS_Timeitem])
			                    *pT_Num[ubFS_Timeitem] = ubT_MaxNum[ubFS_Timeitem];
			                else
			                    (*pT_Num[ubFS_Timeitem])--;

			                if (ubFS_Timeitem == 0)
			                {
			                    if ((ubTimeHour == 12)&&(ubTimeAMPM == 0))
			                    {
			                        ubTimeHour = 11;
			                    }
			                    if ((ubTimeHour == 0)&&(ubTimeAMPM == 1))
			                    {
			                        ubTimeHour = 12;
			                    }
		                	  }

	       	        	 UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
				}
	          	     break;
			}
	       }
        }

		case ENTER_ARROW:
		{
			if(tUI_PuSetting.ubDefualtFlag == TRUE)
			{
				//if(ubFactoryModeFLag == 0)
				{
					if(ubFS_MenuItem == 0)
					{
						#if UI_NOTIMEMENU
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

							UI_SwitchCameraSource();	

							UI_LoadDevStatusInfo();
							tUI_PuSetting.ubDefualtFlag = FALSE;
							UI_TimeSetSystemTime();
							tUI_PuSetting.ubLangageFlag = ubLangageFlag;
							UI_UpdateDevStatusInfo();
							UI_PuInit();
							ubFastShowLostLinkSta = 1;
						#else
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
						#endif
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
			                    else if (ubFS_Timeitem == 4)
			                    {
			                        tOsdImgInfo.uwHSize  = 720;
			                        tOsdImgInfo.uwVSize  = 1280;
			                        tOsdImgInfo.uwXStart = 48;
			                        tOsdImgInfo.uwYStart = 0;
			                        OSD_EraserImg1(&tOsdImgInfo);

			                        VDO_Start();

			                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
			                        tOsdImgInfo.uwXStart = 0;
			                        tOsdImgInfo.uwYStart = 0;
			                        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

			                        UI_LoadDevStatusInfo();
			                        tUI_PuSetting.ubDefualtFlag = FALSE;
			                        UI_TimeSetSystemTime();
			                        tUI_PuSetting.ubLangageFlag = ubLangageFlag;
			                        UI_UpdateDevStatusInfo();
			                        UI_PuInit();

			                        UI_SwitchCameraSource();		
			                        ubFastShowLostLinkSta = 1;
			                    }
	               		 }
				}
			}
			else
         		{
	                	//if (UI_CheckStopAlarm() == 1)
	                    //return;
	          	 }
	            	break;
		}

    default:
        return;
    }
    //tOSD_GetOsdImgInfor (1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
    //tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------
void UI_VolUpKey(void)
{
        if (ubShowAlarmstate >0)
        {
            return;
        }
		
    /*
    if (tUI_State != UI_DISPLAY_STATE)
        return;

    if (tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
            return;
    }

    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL > VOL_LVL8)
        return;
    */
	printd(1,"UI_VolUpKey \n");

    if (ubUI_PttStartFlag == TRUE)
        return;

    if (tUI_SyncAppState == APP_PAIRING_STATE)
        return;

    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL8)
    {
    }
    else
    {
        tUI_PuSetting.VolLvL.tVOL_UpdateLvL++;
    }



	uint8_t CmdData;

//修改增益 
    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL > VOL_LVL4)
    {
    	     CmdData = UI_SET_BUMIC5_8_CMD;
            UI_SendToBUCmd(&CmdData, 1);
		printd(1,"121212121212send BU 12DB !\n");

    }
    else
    {
            CmdData = UI_SET_BUMIC1_4_CMD;
            UI_SendToBUCmd(&CmdData, 1);
	printd(1,"131313131313send BU 12DB !\n");

    }


    UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
    ubMenuKeyPairing = 0;
	if(ubFactoryModeFLag ==0)
	{
	    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL5)
	    {
	    	if(APP_LINK_STATE == tUI_SyncAppState)
	    	{
			if(++tUI_PuSetting.ubSquealWarnCnt <= 3 )
			{
				
				UI_UpdateDevStatusInfo();
		    		UI_DisplaySquealAlert();
			}	
	    	}
	    }	
	}
    tUI_State = UI_DISPLAY_STATE;
}

void UI_VolDownKey(void)
{

        if (ubShowAlarmstate >0)
        {
            return;
        }
		
    /*
    if (tUI_State != UI_DISPLAY_STATE)
        return;

    if (tUI_PuSetting.ubDefualtFlag == FALSE)
    {
        if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
            return;
    }
    */


    if (ubUI_PttStartFlag == TRUE)
        return;

    if (tUI_SyncAppState == APP_PAIRING_STATE)
        return;

    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL < VOL_LVL0)
        return;

    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
    {
    }
    else
    {
        tUI_PuSetting.VolLvL.tVOL_UpdateLvL--;
    }

	uint8_t CmdData;
    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL > VOL_LVL4)
    {
    	     CmdData = UI_SET_BUMIC5_8_CMD;
            UI_SendToBUCmd(&CmdData, 1);
		printd(1,"121212121212send BU 12DB !\n");
    }
    else
    {
            CmdData = UI_SET_BUMIC1_4_CMD;
            UI_SendToBUCmd(&CmdData, 1);
		printd(1,"131313131313send BU 12DB !\n");

    }

    UI_ShowSysVolume(tUI_PuSetting.VolLvL.tVOL_UpdateLvL);
    ubMenuKeyPairing = 0;
    tUI_State = UI_DISPLAY_STATE;
}

void UI_DisplaySquealAlert(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_ALARM_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 167;
    tOsdImgInfo.uwYStart =282;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_UI_VOLWARN_DIS, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 235;
    tOsdImgInfo.uwYStart =594;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_UI_VOLWARN_TITLE+(tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 384;
    tOsdImgInfo.uwYStart =306;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	

    ubVolWarnCnt  = 0;
    ubVolWarnFlag =1;
}

void UI_CleanSquealAlert(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    ubVolWarnFlag = 0; 	
    ubVolWarnCnt = 0;
	
   tOsdImgInfo.uwHSize  = 672;
   tOsdImgInfo.uwVSize  = 1111;
   tOsdImgInfo.uwXStart = 48;
   tOsdImgInfo.uwYStart = 0;
   OSD_EraserImg2(&tOsdImgInfo);

        if (ubNoAddCamFlag == 1)
        {
            OSD_LogoJpeg(OSDLOGO_WHITE_BG);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 175;
            tOsdImgInfo.uwYStart = 279;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            if (tUI_PuSetting.ubLangageFlag == 2)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 503;
                tOsdImgInfo.uwYStart = 360 - 80 - 5;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 503;
                tOsdImgInfo.uwYStart = 360 - 80;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            }
            printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
        }
    
   
}
//------------------------------------------------------------------------------
uint8_t UI_GetAlarmStatus(void)
{
    if ((ubTempAlarmState == HIGH_TEMP_ALARM_ON) || (ubTempAlarmState == HIGH_TEMP_ALARM_ING))
        return 1;

    if ((ubTempAlarmState == LOW_TEMP_ALARM_ON) || (ubTempAlarmState == LOW_TEMP_ALARM_ING))
        return 2;

    if ((ubPickupAlarmState == PICKUP_ALARM_ON) || (ubPickupAlarmState == PICKUP_ALARM_ING))
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

    if ((ubTempAlarmState == HIGH_TEMP_ALARM_ON) || ((ubTempAlarmState == HIGH_TEMP_ALARM_ING)))
    {
        OSD_EraserImg2(&tOsdImgInfo);
        ubShowAlarmstate = 0;
        ubTempAlarmState = TEMP_ALARM_OFF;
        return 1;
    }

    if ((ubTempAlarmState == LOW_TEMP_ALARM_ON) || (ubTempAlarmState == LOW_TEMP_ALARM_ING))
    {
        OSD_EraserImg2(&tOsdImgInfo);
        ubShowAlarmstate = 0;
        ubTempAlarmState = TEMP_ALARM_OFF;
        return 1;
    }

    if ((ubPickupAlarmState == PICKUP_ALARM_ON) || (ubPickupAlarmState == PICKUP_ALARM_ING))
    {
        OSD_EraserImg2(&tOsdImgInfo);
        ubShowAlarmstate = 0;
        ubPickupAlarmState = PICKUP_ALARM_OFF;
        return 1;
    }

	if(((ubTempAlarmState == TEMP_ALARM_OFF) || (ubPickupAlarmState == PICKUP_ALARM_OFF)) && (ubShowAlarmstate >= 1))
	{
		ubShowAlarmstate = 0;
        OSD_EraserImg2(&tOsdImgInfo);
        return 1;
    }

	if(ubShowAlarmstate)
	{
		OSD_EraserImg2(&tOsdImgInfo);
	    ubShowAlarmstate = 0;
	    ubTempAlarmState = TEMP_ALARM_OFF;
		ubPickupAlarmState = PICKUP_ALARM_OFF;
	    return 1;
	}

	printd(1,"ubPickupAlarmState  %dubShowAlarmstate %d \n ",ubPickupAlarmState,ubShowAlarmstate);

    return 0;
}
//------------------------------------------------------------------------------
void UI_DrawMenuPage(void)
{
    /*
    uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {
        OSD2IMG_MENU_BRIGHTNESS,
        OSD2IMG_MENU_LCD_AUTO,
        OSD2IMG_MENU_ALARM,
        OSD2IMG_MENU_TIME,
    OSD2IMG_MENU_ZOOM,
        OSD2IMG_MENU_CAMERA,
        OSD2IMG_MENU_SETTING
    };
    */
//  uint16_t uwDisplayImgIdx = 0;
//  uint8_t i;
//  OSD_IMG_INFO tOsdImgInfo[UI_MENUICON_NUM*4];
//  OSD_IMG_INFO tOsdImgInfo;

    UI_ClearBuConnectStatusFlag();

/*
    uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += UI_ICON_HIGHLIGHT;

    for (i = 0; i < UI_MENUICON_NUM; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS+(i*3), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 48+(i*96);
        tOsdImgInfo.uwYStart =1174;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS_S+(tUI_MenuItem.ubItemIdx*3), 1 , &tOsdImgInfo);
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
    switch (tArrowKey)
    {
    case UP_ARROW:
        if (tUI_MenuItem.ubItemIdx == BRIGHT_ITEM)
        {
            tUI_MenuItem.ubItemPreIdx = BRIGHT_ITEM;
            tUI_MenuItem.ubItemIdx = SETTING_ITEM;
        }
        else
        {
            tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
            tUI_MenuItem.ubItemIdx   -= AUTOLCD_ITEM;
        }
        break;
    case DOWN_ARROW:
        if (tUI_MenuItem.ubItemIdx == SETTING_ITEM)
        {
            tUI_MenuItem.ubItemPreIdx = SETTING_ITEM;
            tUI_MenuItem.ubItemIdx = BRIGHT_ITEM;
        }
        else
        {
            tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
            tUI_MenuItem.ubItemIdx   += AUTOLCD_ITEM;
        }
        break;
    case LEFT_ARROW:
        /*
        if ((tUI_MenuItem.ubItemIdx == BRIGHT_ITEM) ||
           (tUI_MenuItem.ubItemIdx == ZOOM_ITEM))
            return;
        tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
        tUI_MenuItem.ubItemIdx--;
        */
        break;
    case RIGHT_ARROW:
        /*
        if ((tUI_MenuItem.ubItemIdx == TEMP_ITEM) ||
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
    tOSD_GetOsdImgInfor (1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemPreIdx], 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(tUI_MenuItem.ubItemPreIdx*96);
    tOsdImgInfo.uwYStart =1174;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //! Draw highlight icon
    uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += 2;//UI_ICON_HIGHLIGHT;
    tOSD_GetOsdImgInfor (1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemIdx], 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(tUI_MenuItem.ubItemIdx*96);
    tOsdImgInfo.uwYStart =1174;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    UI_DrawSelectMenuPage();
}

void UI_DrawSelectMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i=0; i<UI_MENUICON_NUM; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS_NO_S+(i*3), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 48+(i*96);
        tOsdImgInfo.uwYStart =1174;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    switch (tUI_MenuItem.ubItemIdx)
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
			#if UI_NOTIMEMENU
				UI_DrawTempSubMenuPage_NoSel();
			#else
				UI_DrawTimeSubMenuPage_NoSel();
			#endif
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
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =313;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
*/
}

void UI_DrawBrightnessSubMenuPage_NoSel(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_TITLE+ (2*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 84;
    tOsdImgInfo.uwYStart = 886;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_MAX, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 137;
    tOsdImgInfo.uwYStart = 938;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    for (i = 0; i < tUI_PuSetting.BriLvL.tBL_UpdateLvL; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_GET, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 565-(48*i);
        tOsdImgInfo.uwYStart = 947;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    for (i = tUI_PuSetting.BriLvL.tBL_UpdateLvL; i < 8; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 565-(48*i);
        tOsdImgInfo.uwYStart = 947;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_MIN, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 610;
    tOsdImgInfo.uwYStart = 938;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawBrightnessSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_SEL, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 565-(48*(tUI_PuSetting.BriLvL.tBL_UpdateLvL-1));
    tOsdImgInfo.uwYStart = 947;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRIGHTNESS, 1, &tOsdImgInfo);
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

    printd(Apk_DebugLvl, "UI_ShowSysVolume tVOL_UpdateLvL: %d.\n", value);
    tUI_PuSetting.VolLvL.ubVOL_UpdateCnt = UI_UPDATEVOLLVL_PERIOD*5;

    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
    {
        ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
        UI_SetSpeaker(0, 0);
    }
    else
    {
        if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL1)
        {
            UI_SetSpeaker(0, 1);
        }
        ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
    }
    tUI_PuSetting.VolLvL.tVOL_UpdateLvL = value;

    if (ubFactoryModeFLag == 1)
        return;

    if (tUI_State != UI_DISPLAY_STATE)
    {
        tOsdImgInfo.uwHSize  = 672;
        tOsdImgInfo.uwVSize  = 1280;
        tOsdImgInfo.uwXStart = 48;
        tOsdImgInfo.uwYStart = 0;   //0;
        OSD_EraserImg2(&tOsdImgInfo);

        if (ubNoAddCamFlag == 1 && ubVolWarnFlag == 0)
        {
            OSD_LogoJpeg(OSDLOGO_WHITE_BG);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 175;
            tOsdImgInfo.uwYStart = 279;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            if (tUI_PuSetting.ubLangageFlag == 2)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 503;
                tOsdImgInfo.uwYStart = 360 - 80 - 5;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 503;
                tOsdImgInfo.uwYStart = 360 - 80;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            }
            printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
        }
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_VOL_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 48;
    tOsdImgInfo.uwYStart = 1119;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_VOLUP, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 128;
    tOsdImgInfo.uwYStart = 1166;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    for (i = 0; i < 8; i++)
    {
        if (i < (value - 1))
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_VOLMAX, 1, &tOsdImgInfo);
        else if (i == (value - 1))
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_VOL, 1, &tOsdImgInfo);
        else
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_VOLMIN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 536 - 48*i;
        tOsdImgInfo.uwYStart = 1166;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_VOLDOWN, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 602;
    tOsdImgInfo.uwYStart = 1166;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    //UI_UpdateDevStatusInfo();
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

    if (tUI_PuSetting.ubAutoBrightness)
    {
        LightSenseVal = uwSADC_GetReport(SADC_CH2);
        for (i = 0; i < 7; i++)
        {
            if ((LightSenseVal >= LightSenseArray[i].ubMin) && (LightSenseVal <= LightSenseArray[i].ubMax))
            {
                //printd(Apk_DebugLvl, "LightSenseVal: %d, value(%d, %d).\r\n", LightSenseVal, ubAutoBrightnessValue, LightSenseArray[i].ubValue);
                if (ubAutoBrightnessValue != LightSenseArray[i].ubValue)
                {
                    LCD_BACKLIGHT_CTRL(ulUI_BLTable[LightSenseArray[i].ubValue]);
                    ubAutoBrightnessValue = LightSenseArray[i].ubValue;
                    //printd(Apk_DebugLvl, "UI_AutoBrightnessAdjust: %d#\r\n",ubAutoBrightnessValue);
                }
                break;
            }
        }
    }
}

void UI_BrightnessSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//  OSD_IMG_INFO tOsdImgInfo;
//  UI_MenuAct_t tMenuAct;

    if (tUI_State == UI_MAINMENU_STATE)
    {
        //! Draw Brightness sub menu page
        //ubCurrentMenuItemIdx = 0;
        //ubNextSubMenuItemIdx = 0;

        UI_DrawSubMenuPage(BRIGHT_ITEM);
        ubUI_FastStateFlag = TRUE;
        return;
    }

    //printd(Apk_DebugLvl, "tArrowKey  %d \n",tArrowKey);

    switch (tArrowKey)
    {
    case UP_ARROW:
        //if (ubSubMenuItemFlag == 0)
        {
            if (tUI_PuSetting.BriLvL.tBL_UpdateLvL < 8)
                tUI_PuSetting.BriLvL.tBL_UpdateLvL += 1;
            else
                tUI_PuSetting.BriLvL.tBL_UpdateLvL = 8;

            UI_BrightnessDisplay(tUI_PuSetting.BriLvL.tBL_UpdateLvL);
        }
        break;

    case DOWN_ARROW:
        //if (ubSubMenuItemFlag == 0)
        {
            if (tUI_PuSetting.BriLvL.tBL_UpdateLvL >=2)
                tUI_PuSetting.BriLvL.tBL_UpdateLvL -= 1;
            else
                tUI_PuSetting.BriLvL.tBL_UpdateLvL = 1 ;

            UI_BrightnessDisplay(tUI_PuSetting.BriLvL.tBL_UpdateLvL);
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

    for (i = 0; i < value; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_GET, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 565-(48*i);
        tOsdImgInfo.uwYStart = 947;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    for (i = value; i < 8; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 565-(48*i);
        tOsdImgInfo.uwYStart = 947;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BRI_SEL, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 565-(48*(value-1));
    tOsdImgInfo.uwYStart = 947;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

    tUI_PuSetting.BriLvL.tBL_UpdateLvL = value;
    printd(Apk_DebugLvl, "UI_BrightnessDisplay tBL_UpdateLvL: %d.\n", tUI_PuSetting.BriLvL.tBL_UpdateLvL);
    LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_PuSetting.BriLvL.tBL_UpdateLvL]);
//  UI_UpdateDevStatusInfo();
}

void UI_AutoBriDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;

    if (ubSubMenuItemFlag == 1)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_AUTOBRI_NOSEL_S+(value*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =848;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_AUTOBRI_NOSEL+(value*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =848;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
}

void UI_DrawAutoLcdSubMenuPage_NoSel(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF+(i*2)+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 233+(i*76);
        tOsdImgInfo.uwYStart =729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    if (tUI_PuSetting.ubLangageFlag  != 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
        tOsdImgInfo.uwYStart = 1104;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
        tOsdImgInfo.uwYStart = 1154;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LCD_AUTO_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+96;
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawAutoLcdSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF_S + tUI_PuSetting.ubSleepMode*2 + (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 233 + (tUI_PuSetting.ubSleepMode*76);
    tOsdImgInfo.uwYStart =729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (tUI_PuSetting.ubLangageFlag  != 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
        tOsdImgInfo.uwYStart = 1104;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
        tOsdImgInfo.uwYStart = 1154;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LCD_AUTO, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+96;
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_AutoLcdSetSleepTimerEvent(void)
{
    printd(Apk_DebugLvl, "UI_AutoLcdSetSleepTimerEvent ubShowAlarmstate: %d.\n", ubShowAlarmstate);

    if (tUI_PuSetting.ubSleepMode == 0)
    {
        UI_TimerDeviceEventStop(TIMER1_2);
        return;
    }

    if (ubShowAlarmstate > 0)
        return;

    if (TRUE == ubUI_PttStartFlag)
        return;

    if (ubShowAlarmstate == 0)
    {
#if Current_Test
        if (ubPowerState == PWR_ON)
        {
            ubPowerState = PWR_Prep_Sleep;
        }
#else
        if (LCDBL_STATE == 1)
            LCDBL_ENABLE(UI_DISABLE);
#endif
    }
}

void UI_AutoLcdSetSleepTime(uint8_t SleepMode)
{
    if (ubFactoryModeFLag == 1)
        return;

    printd(Apk_DebugLvl, "UI_AutoLcdSetSleepTime SleepMode: %d.\n", SleepMode);
    if (SleepMode > 4)
        return;

    tUI_PuSetting.ubSleepMode = SleepMode;
//  UI_UpdateDevStatusInfo();

    if (SleepMode == 0)
        UI_TimerDeviceEventStop(TIMER1_2);
    else
        UI_TimerDeviceEventStart(TIMER1_2, ubAutoSleepTime[SleepMode]*1000*70, UI_AutoLcdSetSleepTimerEvent);
}

uint8_t UI_AutoLcdResetSleepTime(uint8_t KeyAction) //20180319
{
    int ret = 0;

    if (ubFactoryModeFLag == 1)
        return 0;

#if Current_Test
    if (ubPowerState == PWR_ON)
    {
        if (KeyAction == KEY_UP_ACT)
        {
            UI_AutoLcdSetSleepTime(tUI_PuSetting.ubSleepMode);
        }
        else
        {
            UI_TimerDeviceEventStop(TIMER1_2);
        }
    }
#else
    if (PWM->PWM_EN8 == 0)
        ret = 1;

    if (KeyAction == KEY_UP_ACT)
    {
        LCDBL_ENABLE(UI_ENABLE);
        UI_AutoLcdSetSleepTime(tUI_PuSetting.ubSleepMode);
    }
    else if (KeyAction == KEY_DOWN_ACT)
    {
        UI_TimerDeviceEventStop(TIMER1_2);
    }
#endif

    return ret;
}

void UI_AutoLcdSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//  OSD_IMG_INFO tOsdImgInfo;

    if (tUI_State == UI_MAINMENU_STATE)
    {
        ubSleepModeFlag = tUI_PuSetting.ubSleepMode;
        ubSubMenuItemFlag = 0;
        ubSubMenuItemPreFlag = 1;

        UI_DrawSubMenuPage(AUTOLCD_ITEM);
        ubUI_FastStateFlag = TRUE;
        return;
    }

    switch (tArrowKey)
    {
    case UP_ARROW:
        if (0 == ubSleepModeFlag) ubSleepModeFlag = 3;
        else  ubSleepModeFlag--;
        UI_AutoLcdSubMenuDisplay(ubSleepModeFlag);
        break;

    case DOWN_ARROW:
        if (++ubSleepModeFlag>=4) ubSleepModeFlag = 0;

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

    for (i = 0; i < 4; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF+(i*2)+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 233+(i*76);
        tOsdImgInfo.uwYStart =729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SLEEP_OFF_S+(ubSleepModeFlag*2)+ (8*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 233+(ubSleepModeFlag*76);
    tOsdImgInfo.uwYStart =729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (tUI_PuSetting.ubLangageFlag  != 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
        tOsdImgInfo.uwYStart = 1104;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 262+(tUI_PuSetting.ubSleepMode*76);
        tOsdImgInfo.uwYStart = 1154;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LCD_AUTO, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+96;
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawAlarmSubMenuPage_NoSel(void)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i = 0; i < 2; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 309+(i*76);
        tOsdImgInfo.uwYStart =729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALARM_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*2);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
#else
     OSD_IMG_INFO tOsdImgInfo;

     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_TITLE+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270;
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF+(tUI_PuSetting.ubSoundLevelSetting *2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(1*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(2*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALARM_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*2);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	 
#endif
}
void UI_DrawAlarmSubMenuPage(void)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 309;
    tOsdImgInfo.uwYStart = 729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALARM, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 48+(96*2);
    tOsdImgInfo.uwYStart = 1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
#else
     OSD_IMG_INFO tOsdImgInfo;
		
     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(1*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALARM, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*2);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
#endif	
}

void UI_AlarmSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//  OSD_IMG_INFO tOsdImgInfo;

    if (tUI_State == UI_MAINMENU_STATE)
    {
        ubSubMenuItemFlag = 0;
        ubSubMenuItemPreFlag = 1;

        UI_DrawSubMenuPage(ALARM_ITEM);
        ubUI_FastStateFlag = TRUE;
        return;
    }

    switch (tArrowKey)
    {
    case UP_ARROW:
        if (ubSubMenuItemFlag >=1)
        {
            ubSubMenuItemPreFlag = ubSubMenuItemFlag;
            ubSubMenuItemFlag -= 1;
        }
        else
        {
            ubSubMenuItemPreFlag = ubSubMenuItemFlag;
            ubSubMenuItemFlag += 1;
        }

        UI_AlarmmenuDisplay();
        break;

    case DOWN_ARROW:
        if (ubSubMenuItemFlag < 1)
        {
            ubSubMenuItemPreFlag = ubSubMenuItemFlag;
            ubSubMenuItemFlag += 1;
        }
        else
        {
            ubSubMenuItemPreFlag = ubSubMenuItemFlag;
            ubSubMenuItemFlag -= 1;
        }
        UI_AlarmmenuDisplay();
        break;

    case RIGHT_ARROW:
    case ENTER_ARROW:
       // ubSubSubMenuItemFlag = 0;
        ubSubSubMenuItemPreFlag = 1;
	   if(ubSubMenuItemFlag == 0)
 		 ubSubSubMenuItemFlag = tUI_PuSetting.ubSoundLevelSetting;
	   else
	   	ubSubSubMenuItemFlag = tUI_PuSetting.ubSoundAlertSetting;
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
#if UI_TEMPMENU
    uint8_t i;
    OSD_IMG_INFO tOsdImgInfo;

    for (i = 0; i < 2; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 309+(i*76);
        tOsdImgInfo.uwYStart = 729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TEMP_ALERT_S+(ubSubMenuItemFlag*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 309+(ubSubMenuItemFlag*76);
    tOsdImgInfo.uwYStart = 729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALARM, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 48+(96*2);
    tOsdImgInfo.uwYStart = 1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
#else
 OSD_IMG_INFO tOsdImgInfo;


if(ubSubMenuItemFlag == 0)
{
     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(1*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(2*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
else
{
     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF+(tUI_PuSetting.ubSoundLevelSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(1*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S_S+(tUI_PuSetting.ubSoundAlertSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
     tOsdImgInfo.uwXStart= 270+(2*76);
     tOsdImgInfo.uwYStart =729;
     tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
#endif	
}

void UI_DrawSubSubMenu_Alarm(void)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;
//  uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
//--------------------------------------------------------------------------------
    if (ubSubMenuItemFlag == 0)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 270;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 346;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


        if (tUI_PuSetting.ubLangageFlag  == 3)
        {
            if (tUI_PuSetting.ubHighTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =400 + 50;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =400 -5;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
        }
        else
        {
            if (tUI_PuSetting.ubHighTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =400 -5;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 400;
                    else
                        tOsdImgInfo.uwYStart = 400 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart = 270;
                    tOsdImgInfo.uwYStart = 400 - 28;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 400;
                    else
                        tOsdImgInfo.uwYStart = 400 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }

        if (tUI_PuSetting.ubLangageFlag  != 2)
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431 + 38;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                            tOsdImgInfo.uwYStart =431 -7 ;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 431;
                        else
                            tOsdImgInfo.uwYStart = 431 + 24;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431 - 28;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431 - 15;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 431;
                    else
                        tOsdImgInfo.uwYStart = 431 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }
        else
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346;
                tOsdImgInfo.uwYStart =386;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346;
                tOsdImgInfo.uwYStart =386;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 422;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 309;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 306+76;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
#else
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (ubSubMenuItemFlag == 0)
    {
        for (i = 0; i < 6; i++)
        {
        	  if(i == 0)
        	  {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_OFF+ (2*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 157+(i*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	  }
	  else
	  {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_LVL1+((i-1)*2), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 157+(i*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	  }			
        }

        if(ubSubSubMenuItemFlag == 0)
        {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_OFF_S+ (2*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 157+(0*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        else
        {
	        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_LVL1_S+((ubSubSubMenuItemFlag-1)*2), 1, &tOsdImgInfo);
	        tOsdImgInfo.uwXStart= 157+(ubSubSubMenuItemFlag*76);
	        tOsdImgInfo.uwYStart =284;
	        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 186 + (tUI_PuSetting.ubSoundLevelSetting*76);

        if (tUI_PuSetting.ubLangageFlag  == 3)
            tOsdImgInfo.uwYStart =604 +10;
        else
            tOsdImgInfo.uwYStart =604 ;

        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S+(i*2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 232+(i*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
        }

	tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S_S+(ubSubSubMenuItemFlag*2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 232+(ubSubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =284;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 266 + (tUI_PuSetting.ubSoundAlertSetting*76);

        if (tUI_PuSetting.ubLangageFlag  == 3)
            tOsdImgInfo.uwYStart =587 +10;
        else
            tOsdImgInfo.uwYStart =587 ;

        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
    }
#endif	
}

void UITempSubmenuDisplay(void)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;

    switch (ubSubSubMenuItemFlag)
    {
    case 0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 270;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 346;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        if (tUI_PuSetting.ubLangageFlag  == 3)
        {
            if (tUI_PuSetting.ubHighTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =400 + 50;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =400 -5;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
        }
        else
        {
            if (tUI_PuSetting.ubHighTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =400 -5;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0_S+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart =400;
                    else
                        tOsdImgInfo.uwYStart =400 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =400 - 28;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 400;
                    else
                        tOsdImgInfo.uwYStart = 400 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }

        if (tUI_PuSetting.ubLangageFlag  != 2)
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431 + 38;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart = 346;
                        tOsdImgInfo.uwYStart = 431 - 7 ;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 431;
                        else
                            tOsdImgInfo.uwYStart = 431 + 24;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431 - 15;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        tOsdImgInfo.uwYStart =431 -28;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 431;
                        else
                            tOsdImgInfo.uwYStart = 431 + 24;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
        }
        else
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346;
                tOsdImgInfo.uwYStart =386;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {

                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346;
                tOsdImgInfo.uwYStart =386;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 422;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 1:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 270;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 346;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        if (tUI_PuSetting.ubLangageFlag  == 3)
        {
            if (tUI_PuSetting.ubHighTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =427 + 48;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {

                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270 ;
                tOsdImgInfo.uwYStart =427 -10;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
        }
        else
        {
            if (tUI_PuSetting.ubHighTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =427 - 2;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 427;
                    else
                        tOsdImgInfo.uwYStart = 427 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =427 -20;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  !=0)
                        tOsdImgInfo.uwYStart = 427;
                    else
                        tOsdImgInfo.uwYStart = 427 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }

        if (tUI_PuSetting.ubLangageFlag  != 2)
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0_S+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =400 + 35;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0_S+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        tOsdImgInfo.uwYStart =400 - 5;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0_S+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 400;
                        else
                            tOsdImgInfo.uwYStart = 400 + 30;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346 ;
                    tOsdImgInfo.uwYStart =400- 15;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        tOsdImgInfo.uwYStart =400 -25;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 400;
                        else
                            tOsdImgInfo.uwYStart = 400 + 30;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
        }
        else
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0_S+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346;
                tOsdImgInfo.uwYStart =365;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart = 346;
                    tOsdImgInfo.uwYStart = 365 - 48;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =365;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 422;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

        break;

    case 2:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 270;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 346;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        if (tUI_PuSetting.ubHighTempSetting != 0)
        {
            if (tUI_PuSetting.ubLangageFlag  == 3)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =427 + 48;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =427 - 2;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_C0+((tUI_PuSetting.ubHighTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 427;
                    else
                        tOsdImgInfo.uwYStart = 427+ 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }
        else
        {
            if (tUI_PuSetting.ubLangageFlag  == 3)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 270;
                tOsdImgInfo.uwYStart =427 - 10;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 1)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    tOsdImgInfo.uwYStart =427 - 20;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 270;
                    if (tUI_PuSetting.ubLangageFlag  != 0)
                        tOsdImgInfo.uwYStart = 427;
                    else
                        tOsdImgInfo.uwYStart = 427 + 24;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
        }

        if (tUI_PuSetting.ubLangageFlag  != 2)
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431 + 38;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        tOsdImgInfo.uwYStart =431 - 7;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 431;
                        else
                            tOsdImgInfo.uwYStart = 431 + 24;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
            else
            {
                if (tUI_PuSetting.ubLangageFlag  == 3)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 346;
                    tOsdImgInfo.uwYStart =431- 18;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
                else
                {
                    if (tUI_PuSetting.ubLangageFlag  == 1)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346;
                        tOsdImgInfo.uwYStart =431- 28;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart= 346 ;
                        if (tUI_PuSetting.ubLangageFlag  != 0)
                            tOsdImgInfo.uwYStart = 431;
                        else
                            tOsdImgInfo.uwYStart = 431 + 24;
                        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
        }
        else
        {
            if (tUI_PuSetting.ubLowTempSetting != 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LOWTEMP_C0+((tUI_PuSetting.ubLowTempSetting-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346;
                tOsdImgInfo.uwYStart =386;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 346 ;
                tOsdImgInfo.uwYStart =386;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            }
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S_S+(tUI_PuSetting.ubTempAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 422;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
#else
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

        for (i = 0; i < 6; i++)
        {
        	  if(i == 0)
        	  {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_OFF+ (2*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 157+(i*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	  }
	  else
	  {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_LVL1+((i -1)*2), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 157+(i*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	  }			
        }

        if(ubSubSubMenuItemFlag == 0)
        {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_OFF_S+ (2*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 157+(0*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        else
        {
	        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_SOUND_LVL1_S+((ubSubSubMenuItemFlag - 1)*2), 1, &tOsdImgInfo);
	        tOsdImgInfo.uwXStart= 157+(ubSubSubMenuItemFlag*76);
	        tOsdImgInfo.uwYStart =284;
	        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 186 + (tUI_PuSetting.ubSoundLevelSetting*76);

        if (tUI_PuSetting.ubLangageFlag  == 3)
            tOsdImgInfo.uwYStart =604 +10;
        else
            tOsdImgInfo.uwYStart =604 ;

        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	

#endif
}
void UISoundSubmenuDisplay(void)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;

    switch (ubSubSubMenuItemFlag)
    {
    case 0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 309;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 306+76;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 1:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF+(tUI_PuSetting.ubSoundLevelSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 309;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S_S+(tUI_PuSetting.ubSoundAlertSetting*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 306+76;
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
#else
        OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

        for (i = 0; i < 4; i++)
        {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S+(i*2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart= 232+(i*76);
	            tOsdImgInfo.uwYStart =284;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
        }


	 tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S_S+(ubSubSubMenuItemFlag*2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	 tOsdImgInfo.uwXStart= 232+(ubSubSubMenuItemFlag*76);
	 tOsdImgInfo.uwYStart =284;
	 tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 266 + (tUI_PuSetting.ubSoundAlertSetting*76);

        if (tUI_PuSetting.ubLangageFlag  == 3)
            tOsdImgInfo.uwYStart =587 +10;
        else
            tOsdImgInfo.uwYStart =587 ;

        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
	 
#endif
}
void UI_AlarmSubSubMenuUpKey(uint8_t SubMenuItem)
{
#if UI_TEMPMENU
    switch (SubMenuItem)
    {
    case 0:
        if (ubSubSubMenuItemFlag >=1)
        {
            ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
            ubSubSubMenuItemFlag  -=  1;
        }
        else
        {
            ubSubSubMenuItemFlag = 2 ;
            ubSubSubMenuItemPreFlag =  0;
        }
        UITempSubmenuDisplay();
        break;

    case 1:
        if (ubSubSubMenuItemFlag >=1)
        {
            ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
            ubSubSubMenuItemFlag  -=  1;
        }
        else
        {
            ubSubSubMenuItemFlag = 1 ;
            ubSubSubMenuItemPreFlag =  0;
        }
        UISoundSubmenuDisplay();
        break;
    }
#else
    switch (SubMenuItem)
    {
	    case 0:
	        if (ubSubSubMenuItemFlag >=1)
	        {
	            ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
	            ubSubSubMenuItemFlag  -=  1;
	        }
	        else
	        {
	            ubSubSubMenuItemFlag = 5 ;
	            ubSubSubMenuItemPreFlag =  0;
	        }
			
	        UITempSubmenuDisplay();
	        break;

	    case 1:
	        if (ubSubSubMenuItemFlag >=1)
	        {
	            ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
	            ubSubSubMenuItemFlag  -=  1;
	        }
	        else
	        {
	            ubSubSubMenuItemFlag = 3 ;
	            ubSubSubMenuItemPreFlag =  0;
	        }
			
	        UISoundSubmenuDisplay();
	        break;
    }
	
#endif	
}
void UI_AlarmSubSubMenuDownKey(uint8_t SubMenuItem)
{
#if UI_TEMPMENU
    switch (SubMenuItem)
    {
    case 0:
        if (ubSubSubMenuItemFlag < 2)
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
        if (ubSubSubMenuItemFlag < 1)
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
#else
    switch (SubMenuItem)
    {
    case 0:
        if (ubSubSubMenuItemFlag < 5)
        {
            ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
            ubSubSubMenuItemFlag += 1;
        }
        else
        {
            ubSubSubMenuItemPreFlag =  5;
            ubSubSubMenuItemFlag = 0;
        }
        UITempSubmenuDisplay();
        break;

    case 1:
        if (ubSubSubMenuItemFlag < 3)
        {
            ubSubSubMenuItemPreFlag =  ubSubSubMenuItemFlag;
            ubSubSubMenuItemFlag += 1;
        }
        else
        {
            ubSubSubMenuItemPreFlag =  3;
            ubSubSubMenuItemFlag = 0;
        }
        UISoundSubmenuDisplay();
        break;
    }
#endif	
}

#if UI_NOTEMPMENU
void UI_DisplayAlarmSubmenu(void)
{
	 OSD_IMG_INFO tOsdImgInfo;

	if(ubSubMenuItemFlag == 0)
	{
	     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF_S+(tUI_PuSetting.ubSoundLevelSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	     tOsdImgInfo.uwXStart= 270+(1*76);
	     tOsdImgInfo.uwYStart =729;
	     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S+(tUI_PuSetting.ubSoundAlertSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	     tOsdImgInfo.uwXStart= 270+(2*76);
	     tOsdImgInfo.uwYStart =729;
	     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
	     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SOUND_OFF+(tUI_PuSetting.ubSoundLevelSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	     tOsdImgInfo.uwXStart= 270+(1*76);
	     tOsdImgInfo.uwYStart =729;
	     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	     tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT5S_S+(tUI_PuSetting.ubSoundAlertSetting * 2)+ (29*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	     tOsdImgInfo.uwXStart= 270+(2*76);
	     tOsdImgInfo.uwYStart =729;
	     tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
}
#endif

void UI_AlarmSubSubMenuEnterKey(uint8_t SubMenuItem)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;

    uint8_t item_temp;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =0;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    switch (SubMenuItem)
    {
    case 0:
        tUI_State = UI_SUBSUBSUBMENU_STATE;
        item_temp = (ubSubMenuItemFlag*3)+ubSubSubMenuItemFlag;
        InitAlarmSubSubSubmenu(item_temp);

        switch (ubSubSubMenuItemFlag)
        {
        case 0:
            ubSubSubSubMenuItemFlag = tUI_PuSetting.ubHighTempSetting;
            break;
        case 1:
            ubSubSubSubMenuItemFlag = tUI_PuSetting.ubLowTempSetting;
            break;
        case 2:
            ubSubSubSubMenuItemFlag = tUI_PuSetting.ubTempAlertSetting;
            break;
        default:
            break;
        }

        UITempSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
        break;

    case 1:
        tUI_State = UI_SUBSUBSUBMENU_STATE;
        InitAlarmSubSubSubmenu(item_temp);
        switch (ubSubSubMenuItemFlag)
        {
        case 0:
            ubSubSubSubMenuItemFlag = tUI_PuSetting.ubSoundLevelSetting;
            break;
        case 1:
            ubSubSubSubMenuItemFlag = tUI_PuSetting.ubSoundAlertSetting;
            break;
        default:
            break;
        }
        UISoundSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
        break;
    }
#else
        switch (ubSubMenuItemFlag)
        {
	        case 0:
	           	tUI_PuSetting.ubSoundLevelSetting = ubSubSubMenuItemFlag;
			if(tUI_PuSetting.ubSoundLevelSetting == 0)
		            ubAlarmIconFlag = 0;
		        else
		            ubAlarmIconFlag = 1;
			UI_DisplayAlarmSubmenu();
			UITempSubmenuDisplay();	

	            break;
	        case 1:
	           	tUI_PuSetting.ubSoundAlertSetting = ubSubSubMenuItemFlag;
			UI_DisplayAlarmSubmenu();				
			UISoundSubmenuDisplay();
	            break;
	        default:
	            break;
        }
#endif	
}

void InitAlarmSubSubSubmenu(uint8_t SubMenuItem)
{
    switch (SubMenuItem)
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
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    switch (SubMenuItem)
    {
    case 0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 194;
        tOsdImgInfo.uwYStart =0;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        for (i = 1; i < 5; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_C0+((i-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(i*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        if (ubSubSubSubMenuItemFlag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        else
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_C0_S+((ubSubSubSubMenuItemFlag-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 227 +(tUI_PuSetting.ubHighTempSetting*76);
        tOsdImgInfo.uwYStart =188;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 1:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 194;
        tOsdImgInfo.uwYStart =0;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        for (i = 1; i < 5; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_LOWTEMP_C0+((i-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(i*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        if (ubSubSubSubMenuItemFlag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_HIGHTEMP_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        else
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SUBMENU_LOWTEMP_C0_S+((ubSubSubSubMenuItemFlag-1)*2)+(!tUI_PuSetting.ubTempunitFlag)*8, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(ubSubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 227 +(tUI_PuSetting.ubLowTempSetting*76);
        tOsdImgInfo.uwYStart =188;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 2:
        for (i = 0; i < 4; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 237+(i*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S_S+(ubSubSubSubMenuItemFlag*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 237+(ubSubSubSubMenuItemFlag*76);
        tOsdImgInfo.uwYStart =0;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 267 + (tUI_PuSetting.ubTempAlertSetting*76);
        tOsdImgInfo.uwYStart =195 +15;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
#else

#endif
}

void UISoundSubSubSubmenuDisplay(uint8_t SubMenuItem)
{
#if UI_TEMPMENU
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    switch (SubMenuItem)
    {
    case 0:
        for (i = 0; i < 6; i++)
        {
            if (i == 0)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_OFF+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 159+(i*76);
                tOsdImgInfo.uwYStart =0;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_LVL1+((i-1)*2), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 159+(i*76);
                tOsdImgInfo.uwYStart =0;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            }
        }

        if (ubSubSubSubMenuItemFlag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_OFF_S+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 159+(ubSubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        else
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_LVL1_S+((ubSubSubSubMenuItemFlag-1)*2), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 159+(ubSubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 191+(tUI_PuSetting.ubSoundLevelSetting*76);
        tOsdImgInfo.uwYStart =233;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 1:
        for (i = 0; i < 4; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S+(i*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 237+(i*76);
            tOsdImgInfo.uwYStart =0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ALERT_5S_S+(ubSubSubSubMenuItemFlag*2)+ (42*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 237+(ubSubSubSubMenuItemFlag*76);
        tOsdImgInfo.uwYStart =0;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 267 + (tUI_PuSetting.ubSoundAlertSetting*76);
        tOsdImgInfo.uwYStart =195 +15;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
#endif
}

void UI_AlarmSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    OSD_IMG_INFO tOsdImgInfo;

    switch (tArrowKey)
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

    switch (tArrowKey)
    {
    case UP_ARROW:
        if (0 == ubSubSubSubMenuItemFlag) ubSubSubSubMenuItemFlag = ubAlarmSubSubItemNum[item_temp];
        else  ubSubSubSubMenuItemFlag--;

        if (item_temp >= 3)
            UISoundSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
        else
            UITempSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
        break;

    case DOWN_ARROW:
        if (++ubSubSubSubMenuItemFlag>=(ubAlarmSubSubItemNum[item_temp]+1)) ubSubSubSubMenuItemFlag = 0;

        if (item_temp >= 3)
            UISoundSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
        else
            UITempSubSubSubmenuDisplay(ubSubSubMenuItemFlag);
        break;

//  case RIGHT_ARROW:
    case ENTER_ARROW:
        UI_SetAlarm(item_temp);

        if ((tUI_PuSetting.ubHighTempSetting == 0)&&(tUI_PuSetting.ubLowTempSetting == 0)
            &&(tUI_PuSetting.ubSoundLevelSetting == 0))
            ubAlarmIconFlag = 0;
        else
            ubAlarmIconFlag = 1;

//      UI_UpdateDevStatusInfo();

        tUI_State = UI_SUBSUBMENU_STATE;
        tOsdImgInfo.uwHSize  = 672;
        tOsdImgInfo.uwVSize  = 284;
        tOsdImgInfo.uwXStart = 48;
        tOsdImgInfo.uwYStart = 0;
        OSD_EraserImg2(&tOsdImgInfo);

        if (item_temp >= 3)
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
    switch (SubMenuItem)
    {
    case 0:
        tUI_PuSetting.ubHighTempSetting = ubSubSubSubMenuItemFlag;
        break;

    case 1:
        tUI_PuSetting.ubLowTempSetting = ubSubSubSubMenuItemFlag;
        break;

    case 2:
        tUI_PuSetting.ubTempAlertSetting = ubSubSubSubMenuItemFlag;
        break;

    case 3:
        tUI_PuSetting.ubSoundLevelSetting = ubSubSubSubMenuItemFlag;
        break;

    case 4:
        tUI_PuSetting.ubSoundAlertSetting = ubSubSubSubMenuItemFlag;
        break;

    default:
        break;
    }
    //UI_UpdateDevStatusInfo();
    UI_SendAlarmSettingToBu();
}

void UI_AlarmTriggerDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_ALARM_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 167;
    tOsdImgInfo.uwYStart =282;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    switch (value)
    {
    case 0: //high temp
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_TITLE+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167+263;
        tOsdImgInfo.uwYStart =282+157;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167+79;
        tOsdImgInfo.uwYStart =282+301;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        break;

    case 1: // low temp
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_TITLE+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167+263;
        tOsdImgInfo.uwYStart =282+157;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167+79;
        tOsdImgInfo.uwYStart =282+301;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        break;

    case 2:   // sound
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_ALARMON_TITLE+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167+263;
        tOsdImgInfo.uwYStart =282+157;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_ALARMON, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167+78;
        tOsdImgInfo.uwYStart =282+284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        break;
    }
}

/*********************************wjb@apical********************************/
void UI_ShowAlarm(uint8_t type)
{
    OSD_IMG_INFO tOsdImgInfo;

	//if(type<3)
	ubShowAlarmType = type;
    printd(Apk_DebugLvl, "UI_ShowAlarm type: %d, ubShowAlarmstate: %d.\n", type, ubShowAlarmstate);

    if ((ubMotor0State != MC_LEFT_RIGHT_OFF) || (ubMotor1State != MC_UP_DOWN_OFF))
    {
        UI_MotorDisplay(MC_LEFT_RIGHT_OFF);
        UI_MotorDisplay(MC_UP_DOWN_OFF);
        UI_EnableMotor(0);
    }

    switch (type)
    {
	    case 0:
	        UI_AlarmTriggerDisplay(0);
	        if (ubRealTemp/100)
	        {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp/100)*2), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart = 340;
	            tOsdImgInfo.uwYStart = 600;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	        }
	        if (ubRealTemp/10%10)
	        {
	            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp/10%10)*2), 1, &tOsdImgInfo);
	            tOsdImgInfo.uwXStart = 340;
	            tOsdImgInfo.uwYStart = 600-24;
	            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	        }
	        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_0+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
	        tOsdImgInfo.uwXStart= 340;
	        tOsdImgInfo.uwYStart =600-24-24;
	        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_HIGHTEMP_F - 2*tUI_PuSetting.ubTempunitFlag, 1, &tOsdImgInfo);
	        tOsdImgInfo.uwXStart = 334;
	        tOsdImgInfo.uwYStart = 600-72-24;
	        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	        ubShowAlarmstate = 1;
	        break;

		case 1:
			UI_AlarmTriggerDisplay(1);
			if(ubTempBelowZoreSta == 1)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_MIN, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = 340;
				tOsdImgInfo.uwYStart = 600;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}
			if(ubRealTemp/10%10)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_0+((ubRealTemp/10%10)*2), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = 340;
				tOsdImgInfo.uwYStart = 600-24;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_0+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 340;
			tOsdImgInfo.uwYStart =600-24-24;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_LOWTEMP_F - 2*tUI_PuSetting.ubTempunitFlag, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 334;
			tOsdImgInfo.uwYStart =600-72-24;	
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubShowAlarmstate = 2;
			break;

    case 2:
        UI_AlarmTriggerDisplay(2);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        ubShowAlarmstate = 3;
        break;

    case 3:
        if (ubShowAlarmstate)
        {
            tOsdImgInfo.uwHSize  = 387;
            tOsdImgInfo.uwVSize  = 717;
            tOsdImgInfo.uwXStart = 167;
            tOsdImgInfo.uwYStart = 282;
            OSD_EraserImg2(&tOsdImgInfo);
            ubShowAlarmstate = 0;
        }
        return;
//      break;

    default:
        break;
    }
    tUI_State = UI_DISPLAY_STATE;

    if(ubShowAlarmstate > 0)
    {
        if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
        {
            if(PWR_Sleep_Complete == ubPowerState)
                UI_SetSleepState(1);
        }

        UI_TimerDeviceEventStop(TIMER1_2);
        UI_DisableScanMode();
        /*
        if(LCDBL_STATE == 0)
            LCDBL_ENABLE(UI_ENABLE);
        */
    }
}

void UI_PlayAlarmSound(uint8_t type)
{
    uint8_t ubAlertSetting = 0;

    if ((type == 0) || (type == 1))
    {
        ubAlertSetting = tUI_PuSetting.ubTempAlertSetting;
    }
    else if (type == 2)
    {
        ubAlertSetting = tUI_PuSetting.ubSoundAlertSetting;
    }

    printd(1, "UI_PlayAlarmSound ubAlertSetting: %d, ubPlayAlarmCount: %d.\n", ubAlertSetting, ubPlayAlarmCount);
    if (++ubPlayAlarmCount > (ubAlertTime[ubAlertSetting]*100/20))
    {
        ubPlayAlarmCount = 0; //200
        if ((type == 0) || (type == 1))
        {
            ubTempAlarmState = TEMP_ALARM_OFF;
            if (ubPickupAlarmState != PICKUP_ALARM_OFF)
            {
                ubPickupAlarmState = PICKUP_ALARM_IDLE;
            }
        }
        else if (type == 2)
        {
            ubPickupAlarmState = PICKUP_ALARM_OFF;
            if (ubTempAlarmState != TEMP_ALARM_OFF)
            {
                ubTempAlarmState = TEMP_ALARM_IDLE;
            }
        }
        UI_StopPlayAlarm();
    }
    else
    {
        ADO_Stop(); // by xiao
        ADO_SetDacMute(DAC_MR_0p5DB_8SAMPLE, ADO_OFF); // by xiao
        ADO_SetDacR2RVol(R2R_VOL_n0DB);
        if (ubAlarmPlayState == 0)
        {
            ubAlarmPlayState = 1;
            if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == 0)
            {
                UI_SetSpeaker(2, 1);
            }
        }

        if ((ubPlayAlarmCount%4) == 0)  // 3
        {
            printd(Apk_DebugLvl, "UI_PlayAlarmSound BUZ_PlaySingleSound###\n");
            BUZ_PlayAlarmSound();

/*			if(ubShowAlarmAgain)
			{	
				//osDelay(1000);
				ubShowAlarmAgain=0;
				printd(Apk_DebugLvl, "UI_PlayAlarmSound ubShowAlarmType: %d\n", ubShowAlarmType);
				UI_ShowAlarm(ubShowAlarmType);
			}
*/			
        }
    }
}

void UI_StopPlayAlarm(void)
{
//	ubShowAlarmAgain=0;

    if (ubAlarmPlayState == 1)
    {
        ubAlarmPlayState = 0;
        if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL == 0)
        {
            UI_SetSpeaker(2, 0);
        }
        ADO_Start(tCamViewSel.tCamViewPool[0]);
        ADO_SetDacR2RVol(tUI_VOLTable[tUI_PuSetting.VolLvL.tVOL_UpdateLvL]);
    }
}

#if UI_TEMPMENU
void UI_TempAlarmCheck(void)
{
    OSD_IMG_INFO tOsdInfo;
    uint8_t ubHightempMax,ubLowtempMin;
  	printd(Apk_DebugLvl, "UI_TempAlarmCheck \n");

	if(ubRealTemp == 0xFF)
      		return;
	
    if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
        return;

    if (ubFactoryModeFLag == 1)
        return;

    /*
    if(ubPowerState != PWR_ON)
    {
        ubTempAlarmState = TEMP_ALARM_IDLE;
        return;
    }
    */

    if (tUI_State != UI_DISPLAY_STATE)
    {
        //if (ubTempAlarmState == TEMP_ALARM_IDLE)
        //  return;
        //ubTempAlarmState = TEMP_ALARM_OFF;

        /*
        if(ubAlarmOnlyAlertFlag == 0)
        {
            OSD_IMG_INFO tOsdInfo;

            tOsdInfo.uwHSize  = 672;
            tOsdInfo.uwVSize  = 1280;
            tOsdInfo.uwXStart = 48;
            tOsdInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdInfo);
            tUI_State = UI_DISPLAY_STATE;
            ubAlarmOnlyAlertFlag = 1;
        }
        */
    }

    if (ubTempAlarmState == TEMP_ALARM_IDLE)
    {
        if (tUI_PuSetting.ubTempunitFlag == 1)
        {
            ubHightempMax = ubHighTempC[tUI_PuSetting.ubHighTempSetting];
            ubLowtempMin = ubLowTempC[tUI_PuSetting.ubLowTempSetting];
        }
        else
        {
            ubHightempMax = ubHighTempF[tUI_PuSetting.ubHighTempSetting];
            ubLowtempMin = ubLowTempF[tUI_PuSetting.ubLowTempSetting];
        }

        if (tUI_PuSetting.ubHighTempSetting != 0)
        {
            if (ubRealTemp >= ubHightempMax)
            {
                if ((ubPickupAlarmState == PICKUP_ALARM_IDLE) || (ubPickupAlarmState == PICKUP_ALARM_OFF))
                {

			if(TRUE == ubUI_PttStartFlag)
				return;
                    ubTempAlarmCheckCount++;
                    if (ubTempAlarmCheckCount == 5)
                    {
				if(tUI_State != UI_DISPLAY_STATE)
				{
					tUI_State = UI_DISPLAY_STATE;
					
					tOsdInfo.uwHSize  = 672;
					tOsdInfo.uwVSize  = 1280;
					tOsdInfo.uwXStart = 48;
					tOsdInfo.uwYStart = 0;
					OSD_EraserImg2(&tOsdInfo);
				}
                        ubTempAlarmState = HIGH_TEMP_ALARM_ON;
                        ubTempAlarmCheckCount = 0;
                    }
                }
            }
        }

        if (tUI_PuSetting.ubLowTempSetting != 0)
        {
            if (ubRealTemp <= ubLowtempMin)
            {
                if ((ubPickupAlarmState == PICKUP_ALARM_IDLE) || (ubPickupAlarmState == PICKUP_ALARM_OFF))
                {

			if(TRUE == ubUI_PttStartFlag)
				return;
                    ubTempAlarmCheckCount++;
                    if (ubTempAlarmCheckCount == 5)
                    {
				if(tUI_State != UI_DISPLAY_STATE)
				{
					tUI_State = UI_DISPLAY_STATE;
					tOsdInfo.uwHSize  = 672;
					tOsdInfo.uwVSize  = 1280;
					tOsdInfo.uwXStart = 48;
					tOsdInfo.uwYStart = 0;
					OSD_EraserImg2(&tOsdInfo);
				}

                        ubTempAlarmState = LOW_TEMP_ALARM_ON;
                        ubTempAlarmCheckCount = 0;
			
                    }
                }
            }
        }

        if ((ubShowAlarmstate == 1) || (ubShowAlarmstate == 2))
        {
		     //if(ubAlarmOnlyAlertFlag == 0)
		     if(ubTempAlarmState == LOW_TEMP_ALARM_ON || ubTempAlarmState == HIGH_TEMP_ALARM_ON)
                		UI_ShowAlarm(3);
			ubAlarmOnlyAlertFlag = 0;

        }
    }
    else if (ubTempAlarmState == HIGH_TEMP_ALARM_ON)
    {
        //if(ubAlarmOnlyAlertFlag == 0)
            UI_ShowAlarm(0);
        ubPlayAlarmCount = 0;
        ubTempAlarmState = HIGH_TEMP_ALARM_ING;
    }
    else if (ubTempAlarmState == HIGH_TEMP_ALARM_ING)
    {
        UI_PlayAlarmSound(0);
    }
    else if (ubTempAlarmState == LOW_TEMP_ALARM_ON)
    {
        //if(ubAlarmOnlyAlertFlag == 0)
            UI_ShowAlarm(1);
        ubPlayAlarmCount = 0;
        ubTempAlarmState = LOW_TEMP_ALARM_ING;
    }
    else if (ubTempAlarmState == LOW_TEMP_ALARM_ING)
    {
        UI_PlayAlarmSound(1);
    }
    else if (ubTempAlarmState == TEMP_ALARM_OFF)
    {
        if (ubTempAlarmTriggerCount < TEMP_ALARM_INTERVAL) //60s
        {
            ubTempAlarmTriggerCount++;
        }
        else
        {
            ubTempAlarmState = TEMP_ALARM_IDLE;
            ubTempAlarmTriggerCount = 0;
            ubTempAlarmCheckCount = 0;
        }
    }
}
#endif
void UI_PickupAlarmCheck(void)
{
    OSD_IMG_INFO tOsdInfo;
  	printd(Apk_DebugLvl, "UI_PickupAlarmCheck \n");
    if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
        return;

    /*
    if(ubPowerState != PWR_ON)
    {
        ubPickupAlarmState = PICKUP_ALARM_IDLE;
        return;
    }
    */

    if (ubFactoryModeFLag == 1)
        return;


    if (tUI_State != UI_DISPLAY_STATE)
    {
        //if(ubPickupAlarmState == PICKUP_ALARM_IDLE)
            //return;

        //ubPickupAlarmState = PICKUP_ALARM_OFF;

        /*
        if(ubAlarmOnlyAlertFlag == 0)
        {
            OSD_IMG_INFO tOsdInfo;

            tOsdInfo.uwHSize  = 672;
            tOsdInfo.uwVSize  = 1280;
            tOsdInfo.uwXStart = 48;
            tOsdInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdInfo);
            tUI_State = UI_DISPLAY_STATE;
            ubAlarmOnlyAlertFlag = 1;
        }
        */
    }

    if (ubPickupAlarmState == PICKUP_ALARM_IDLE)
    {
        if (tUI_PuSetting.ubSoundLevelSetting != 0)
        {
            if (ubGetVoiceTemp >= tUI_PuSetting.ubSoundLevelSetting)
            {
                if ((ubTempAlarmState == TEMP_ALARM_IDLE) || (ubTempAlarmState == TEMP_ALARM_OFF))
                {
			if(TRUE == ubUI_PttStartFlag)
				return;
                    ubPickupAlarmCheckCount++;
                    if (ubPickupAlarmCheckCount == 5)
                    {
				if(tUI_State != UI_DISPLAY_STATE)
				{
					tUI_State = UI_DISPLAY_STATE;
					tOsdInfo.uwHSize  = 672;
					tOsdInfo.uwVSize  = 1280;
					tOsdInfo.uwXStart = 48;
					tOsdInfo.uwYStart = 0;
					OSD_EraserImg2(&tOsdInfo);
				}

                        ubPickupAlarmState = PICKUP_ALARM_ON;
                        ubPickupAlarmCheckCount = 0;
                    }
                }
            }
        }

        if (ubShowAlarmstate == 3)
        {
			//if(ubAlarmOnlyAlertFlag == 0)
		if(ubPickupAlarmState == PICKUP_ALARM_ON)
                UI_ShowAlarm(3);
            ubAlarmOnlyAlertFlag = 0;
        }
    }
    else if (ubPickupAlarmState == PICKUP_ALARM_ON)
    {
        UI_ShowAlarm(2);
        ubPlayAlarmCount = 0;
        ubPickupAlarmState = PICKUP_ALARM_ING;
    }
    else if (ubPickupAlarmState == PICKUP_ALARM_ING)
    {
        UI_PlayAlarmSound(2);
    }
    else if (ubPickupAlarmState == PICKUP_ALARM_OFF)
    {
        printd(1, "PICKUP: (%d) ------\n", ubPickupAlarmTriggerCount);
        if (ubPickupAlarmTriggerCount < PICk_ALARM_INTERVAL) //30s
        {
            ubPickupAlarmTriggerCount++;
        }
        else
        {
            ubPickupAlarmState = PICKUP_ALARM_IDLE;
            ubPickupAlarmTriggerCount = 0;
            ubPickupAlarmCheckCount = 0;
        }
    }
}

void UI_TriggerWakeUpAlarm(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    printd(Apk_DebugLvl, "UI_TriggerWakeUpAlarm type: %d.\n", ubAlarmWakeupType);
    switch (ubAlarmWakeupType)
    {
    case 1:
        //ubTempAlarmState = HIGH_TEMP_ALARM_ON;
        break;

    case 2:
        //ubTempAlarmState = LOW_TEMP_ALARM_ON;
        break;

    case 3:
        ubPickupAlarmState = PICKUP_ALARM_ON;
        break;

    default:
        break;
    }

    if (ubAlarmWakeupType > 0)
    {
        tOsdImgInfo.uwHSize  = 672;
        tOsdImgInfo.uwVSize  = 1280;
        tOsdImgInfo.uwXStart = 48;
        tOsdImgInfo.uwYStart = 0;
        OSD_EraserImg2(&tOsdImgInfo);
        tUI_State = UI_DISPLAY_STATE;
    }

    ubAlarmWakeupType = 0;
}
/*********************************wjb@apical********************************/

void UI_DrawTimeSubMenuPage_NoSel(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    /*
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB_TIME+ (1*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 295;
    tOsdImgInfo.uwYStart =880;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    */

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 330;
    tOsdImgInfo.uwYStart =834;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 336;
    tOsdImgInfo.uwYStart =1027;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 416;
    tOsdImgInfo.uwYStart =1027;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-----------------------------------------------
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 368;
    tOsdImgInfo.uwYStart =840 - 10;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-------------------------------------------------------------------
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 368;
    tOsdImgInfo.uwYStart =917;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 368;
    tOsdImgInfo.uwYStart =943;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 368;
    tOsdImgInfo.uwYStart =971;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------------------------------
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 368;
    tOsdImgInfo.uwYStart =1010;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 368;
    tOsdImgInfo.uwYStart =1036;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*3);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawTimeSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    UI_DrawSelectMenuPage();

    /*
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB_TIME+ (1*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 295;
    tOsdImgInfo.uwYStart =880;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    */

    UI_TimeSubMenuDisplay(ubSubMenuItemFlag);

//----------------------------------------------------------------------------
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*3);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_TimeSetSystemTime(void) //add by wjb
{
    tUI_PuSetting.tSysCalendar.uwYear = 2018;
    tUI_PuSetting.tSysCalendar.ubMonth = 1;
    tUI_PuSetting.tSysCalendar.ubDate = 1;

    printd(Apk_DebugLvl, "UI_TimeSetSystemTime ubTimeHour: %d, ubTimeMin: %d, ubTimeAMPM: %d.\n", ubTimeHour, ubTimeMin, ubTimeAMPM);
    if (ubTimeAMPM)
    {
        if (ubTimeHour == 12)
            tUI_PuSetting.tSysCalendar.ubHour = ubTimeHour;
        else
            tUI_PuSetting.tSysCalendar.ubHour = ubTimeHour + 12;
    }
    else
    {
        tUI_PuSetting.tSysCalendar.ubHour = ubTimeHour;
    }

    tUI_PuSetting.tSysCalendar.ubMin = ubTimeMin;
    tUI_PuSetting.tSysCalendar.ubSec = 0;

    if (iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
        printd(DBG_ErrorLvl, "Calendar base setting fail !\n");
    else
        RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
}

void UI_TimeShowSystemTime(uint8_t type)
{
    OSD_IMG_INFO tOsdImgInfo;
    RTC_Calendar_t tUi_Calendar;

    //printf("tUI_PuSetting.tSysCalendar.ubHour 11 = %d \n",tUI_PuSetting.tSysCalendar.ubHour);
    //printf("tUI_PuSetting.tSysCalendar.ubMin  11   =%d \n",tUI_PuSetting.tSysCalendar.ubMin);

    RTC_GetCalendar((RTC_Calendar_t *)(&tUi_Calendar));

    //printf("tUi_Calendar.ubHour  22 = %d \n",tUi_Calendar.ubHour);
    //printf("tUi_Calendar.ubMin  22   =%d \n",tUi_Calendar.ubMin);

    if ((type == 1) || (ubTimeHour != (tUi_Calendar.ubHour%12)) || (ubTimeMin != tUi_Calendar.ubMin)
        || (ubTimeAMPM^(tUi_Calendar.ubHour >= 12?1:0)))
    {
        if (tUi_Calendar.ubHour >= 12)
        {
            ubTimeAMPM = 1;
            if (tUi_Calendar.ubHour == 12)
                ubTimeHour = 12;
            else
                ubTimeHour = tUi_Calendar.ubHour - 12;
        }
        else
        {
            ubTimeAMPM = 0;
            ubTimeHour = tUi_Calendar.ubHour;
        }
        ubTimeMin = tUi_Calendar.ubMin;
        ubTimeSec= tUi_Calendar.ubSec;

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeHour/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 217;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeHour%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 198;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_TIMESIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 185;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeMin/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 169;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeMin%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 150;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_TIMEAM + ubTimeAMPM, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 95;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

#if TIME_TEST
        #define time_offset 0
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeHour/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 576 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeHour%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 557 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_TIMESIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 541 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeMin/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 525 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeMin%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 506 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_TIMESIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart =490 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeSec/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 474 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 + (ubTimeSec%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 457 - time_offset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
#endif
    }
}

void UI_TimeSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//  OSD_IMG_INFO tOsdImgInfo;
    RTC_Calendar_t tUi_Calendar;
    uint8_t ubT_MaxNum[3] = {12, 59, 1};
    uint8_t ubT_MinNum[3] = {0,  0,  0};
    uint8_t *pT_Num[3] = {(uint8_t *)(&ubTimeHour), (uint8_t *)(&ubTimeMin), (uint8_t *)(&ubTimeAMPM) };

    if (tUI_State == UI_MAINMENU_STATE)
    {
        ubSubMenuItemFlag = 0;
        ubSubMenuItemPreFlag = 1;

        RTC_GetCalendar((RTC_Calendar_t *)(&tUi_Calendar));
        if (tUi_Calendar.ubHour >= 12)
        {
            ubTimeAMPM = 1;
            if (tUi_Calendar.ubHour == 12)
                ubTimeHour = 12;
            else
                ubTimeHour = tUi_Calendar.ubHour - 12;
        }
        else
        {
            ubTimeAMPM = 0;
            ubTimeHour = tUi_Calendar.ubHour;
        }
        ubTimeMin = tUi_Calendar.ubMin;

//      printd(Apk_DebugLvl, "ubTimeHour  %d\n",ubTimeHour);
        ubEnterTimeMenuFlag = 1;

        UI_DrawSubMenuPage(TIME_ITEM);
        ubUI_FastStateFlag = TRUE;
        return;
    }

    switch (tArrowKey)
    {
    case UP_ARROW:
        if ((NULL == pT_Num[ubSubMenuItemFlag]))
                return;

        if (ubSubMenuItemFlag == 2)
        {
            if ((ubTimeHour == 12)&&(ubTimeAMPM == 1))
                return;
            if ((ubTimeHour == 0)&&(ubTimeAMPM == 0))
                return;
        }

        if (*pT_Num[ubSubMenuItemFlag] == ubT_MaxNum[ubSubMenuItemFlag])
            *pT_Num[ubSubMenuItemFlag] = ubT_MinNum[ubSubMenuItemFlag];
        else
            (*pT_Num[ubSubMenuItemFlag])++;

        if (ubSubMenuItemFlag == 0)
        {
            if ((ubTimeHour == 12)&&(ubTimeAMPM == 0))
            {
                ubTimeHour = 1;
            }
            if ((ubTimeHour == 0)&&(ubTimeAMPM == 1))
            {
                ubTimeHour = 1;
            }
        }
        UI_TimeSubMenuDisplay(ubSubMenuItemFlag);
        return;

    case DOWN_ARROW:
        if ((NULL == pT_Num[ubSubMenuItemFlag]))
            return;

        if (ubSubMenuItemFlag == 2)
        {
            if ((ubTimeHour == 12)&&(ubTimeAMPM == 1))
                return;
            if ((ubTimeHour == 0)&&(ubTimeAMPM == 0))
                return;
        }

        if (*pT_Num[ubSubMenuItemFlag] == ubT_MinNum[ubSubMenuItemFlag])
            *pT_Num[ubSubMenuItemFlag] = ubT_MaxNum[ubSubMenuItemFlag];
        else
            (*pT_Num[ubSubMenuItemFlag])--;

        if (ubSubMenuItemFlag == 0)
        {
            if ((ubTimeHour == 12)&&(ubTimeAMPM == 0))
            {
                ubTimeHour = 11;
            }
            if ((ubTimeHour == 0)&&(ubTimeAMPM == 1))
            {
                ubTimeHour = 12;
            }
        }
        UI_TimeSubMenuDisplay(ubSubMenuItemFlag);
        return;

    case LEFT_ARROW:
        if (ubSubMenuItemFlag >=1)
        {
            ubSubMenuItemFlag -= 1;
        }
        else
        {
            ubSubMenuItemFlag = 0;
            ubSubMenuItemPreFlag = 1;
	     tUI_State = UI_MAINMENU_STATE;
            ubEnterTimeMenuFlag = 0;
            UI_DrawMenuPage();

	        RTC_GetCalendar((RTC_Calendar_t *)(&tUi_Calendar));
	        if (tUi_Calendar.ubHour >= 12)
	        {
	            ubTimeAMPM = 1;
	            if (tUi_Calendar.ubHour == 12)
	                ubTimeHour = 12;
	            else
	                ubTimeHour = tUi_Calendar.ubHour - 12;
	        }
	        else
	        {
	            ubTimeAMPM = 0;
	            ubTimeHour = tUi_Calendar.ubHour;
	        }
	        ubTimeMin = tUi_Calendar.ubMin;
		UI_DrawTimeSubMenuPage_NoSel();
				
	            return;
	        }
	        UI_TimeSubMenuDisplay(ubSubMenuItemFlag);
	        break;

    case RIGHT_ARROW:
        if (ubSubMenuItemFlag < 2)
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

        ubSubMenuItemFlag = 0;
        ubSubMenuItemPreFlag = 1;

        RTC_GetCalendar((RTC_Calendar_t *)(&tUi_Calendar));
        if (tUi_Calendar.ubHour >= 12)
        {
            ubTimeAMPM = 1;
            if (tUi_Calendar.ubHour == 12)
                ubTimeHour = 12;
            else
                ubTimeHour = tUi_Calendar.ubHour - 12;
        }
        else
        {
            ubTimeAMPM = 0;
            ubTimeHour = tUi_Calendar.ubHour;
        }
        ubTimeMin = tUi_Calendar.ubMin;
	UI_DrawTimeSubMenuPage_NoSel();
        ubEnterTimeMenuFlag = 0;
        UI_DrawMenuPage();
        break;
    }
}


void UI_TimeSubMenuDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;

    /*
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB_TIME+ (1*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 295;
    tOsdImgInfo.uwYStart =880;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    */

    switch (value)
    {
    case 0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 330;
        tOsdImgInfo.uwYStart =834;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 336;
        tOsdImgInfo.uwYStart =1027;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 416;
        tOsdImgInfo.uwYStart =1027;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 368;
        tOsdImgInfo.uwYStart =840 - 10;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 917;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 943;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 971;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 1010;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 1036;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 1:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 330;
        tOsdImgInfo.uwYStart = 834;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 336;
        tOsdImgInfo.uwYStart = 936;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 416;
        tOsdImgInfo.uwYStart = 936;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 840 - 10;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 917;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0_S+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 943;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 971;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 1010;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 1036;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 2:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 330;
        tOsdImgInfo.uwYStart = 834;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_UPARROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 336;
        tOsdImgInfo.uwYStart = 864;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_DOWNARROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 416;
        tOsdImgInfo.uwYStart = 864;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_AM_S+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 840 - 10;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 917;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 943;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 971;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 1010;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 368;
        tOsdImgInfo.uwYStart = 1036;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
}

void UI_Zoom_SetScaleParam(uint8_t tZoomScale)
{
    uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
    uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();

    //printd(Apk_DebugLvl, "UI_Zoom_SetScaleParam tZoomScale: %d, tUI_PuSetting.ubZoomScale: %d.\r\n", tZoomScale, tUI_PuSetting.ubZoomScale);

    if (tZoomScale == 0) //off
    {
        tUI_DptzParam.tScaleParam                            = UI_SCALEUP_2X;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize*2/tUI_DptzParam.tScaleParam;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize*2/tUI_DptzParam.tScaleParam;
        tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize      = ulLcd_HSize;
        tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize      = ulLcd_VSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
    }
    else if (tZoomScale == 1) //1.5X
    {
        tUI_DptzParam.tScaleParam                            = UI_SCALEUP_3X;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize*2/tUI_DptzParam.tScaleParam;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize*2/tUI_DptzParam.tScaleParam;
        tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize      = ulLcd_HSize;
        tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize      = ulLcd_VSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
    }
    else if (tZoomScale == 2) //2X
    {
        tUI_DptzParam.tScaleParam                            = UI_SCALEUP_4X;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize*2/tUI_DptzParam.tScaleParam;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize*2/tUI_DptzParam.tScaleParam;
        tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize      = ulLcd_HSize;
        tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize      = ulLcd_VSize;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
        tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
    }

    ubTest_uwCropHsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize;
    ubTest_uwCropVsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize;
    ubTest_uwCropHstart = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart;
    ubTest_uwCropVstart = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart;

    tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
}

void UI_DrawZoomSubMenuPage_NoSel(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i = 0; i < 3; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF+(i*2)+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 270+(i*76);
        tOsdImgInfo.uwYStart =729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    if (tUI_PuSetting.ubLangageFlag == 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
        tOsdImgInfo.uwYStart = 1066 + 30;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
        tOsdImgInfo.uwYStart = 1066;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*4);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DrawZoomSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF_S+tUI_PuSetting.ubZoomScale*2+(6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 270 + (tUI_PuSetting.ubZoomScale*76);
    tOsdImgInfo.uwYStart =729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (tUI_PuSetting.ubLangageFlag == 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
        tOsdImgInfo.uwYStart = 1066 + 30;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
        tOsdImgInfo.uwYStart = 1066;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*4);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
void UI_ZoomSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//  OSD_IMG_INFO tOsdImgInfo;

    if (tUI_State == UI_MAINMENU_STATE)
    {
        ubSubMenuItemFlag = tUI_PuSetting.ubZoomScale;
        ubSubMenuItemPreFlag = 1;

        UI_DrawSubMenuPage(ZOOM_ITEM);
        ubUI_FastStateFlag = TRUE;
        return;
    }

    if ((APP_LINK_STATE != tUI_SyncAppState) || (CAM_OFFLINE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts))
    {
        if ((tArrowKey != LEFT_ARROW) && (tArrowKey != EXIT_ARROW))
            return;
    }

    switch (tArrowKey)
    {
    case UP_ARROW:
        ubSubMenuItemPreFlag =  ubSubMenuItemFlag;

        if (0 == ubSubMenuItemFlag) ubSubMenuItemFlag = 2;
        else ubSubMenuItemFlag--;

        UI_ZoomDisplay();
        break;

    case DOWN_ARROW:
        ubSubMenuItemPreFlag =  ubSubMenuItemFlag;

        if (++ubSubMenuItemFlag>=3) ubSubMenuItemFlag = 0;

        UI_ZoomDisplay();
        break;

    case RIGHT_ARROW:
        break;

    case ENTER_ARROW:
        tUI_PuSetting.ubZoomScale = ubSubMenuItemFlag;
        UI_Zoom_SetScaleParam(tUI_PuSetting.ubZoomScale);
        UI_ZoomDisplay();
        //UI_UpdateDevStatusInfo();
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

    for (i = 0; i < 3; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF+(i*2)+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 270+(i*76);
        tOsdImgInfo.uwYStart = 729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM_OFF_S+(ubSubMenuItemFlag*2)+ (6*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 270+(ubSubMenuItemFlag*76);
    tOsdImgInfo.uwYStart = 729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (tUI_PuSetting.ubLangageFlag == 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
        tOsdImgInfo.uwYStart = 1066 + 30;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 302+(tUI_PuSetting.ubZoomScale*75);
        tOsdImgInfo.uwYStart = 1066;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_ZOOM, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 48+(96*4);
    tOsdImgInfo.uwYStart = 1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

//------------------------------------------
void UI_DrawCamsSubMenuPage_NoSel(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i = 0; i < 3; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD+(i*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 270+(i*76);
        tOsdImgInfo.uwYStart =729;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAMERA_S, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*5);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
void UI_DrawCamsSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD_S+(ubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 270+(ubSubMenuItemFlag*76);
    tOsdImgInfo.uwYStart =729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAMERA, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*5);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_CamSubSubmenuDisplay(uint8_t SubMenuItem)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i, j;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    switch (SubMenuItem)
    {
    case 1:
        for (i = 0; i < 4; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_NOPAIR+ubCamPairFlag[i]+(i*3)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 233+(i*76);
            tOsdImgInfo.uwYStart =284;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        i = ubSubSubMenuItemFlag;
        for (j = 0; j < 4; j++)
        {
            if (ubCamPairFlag[i] == 1)
            {
                ubSubSubMenuItemFlag = i;
                break;
            }
            if (i == 3)
                i = 0;
            else
                i++;
        }

        if (ubNoAddCamFlag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_PAIR_S+(ubSubSubMenuItemFlag*3)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 233+(ubSubSubMenuItemFlag*76);
            tOsdImgInfo.uwYStart =284;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 2:
        for (i = 0; i < 5; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SCAN_OFF+(i*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 194+(i*76);
            tOsdImgInfo.uwYStart =284;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SCAN_OFF_S+(ubSubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 194+(ubSubSubMenuItemFlag*76);
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);


        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 225 + (tUI_PuSetting.ubScanTime*76);

        if (tUI_PuSetting.ubLangageFlag  == 3)
            tOsdImgInfo.uwYStart =572 +10;
        else
            tOsdImgInfo.uwYStart =572 ;

        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
}

void UI_DeletemenuDisplay(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 48;
    tOsdImgInfo.uwYStart = 0;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_MENUDEL+(ubSubSubSubMenuItemPreFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 309+(ubSubSubSubMenuItemPreFlag*76);
    tOsdImgInfo.uwYStart = 0;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_MENUDEL_S+(ubSubSubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 309+(ubSubSubSubMenuItemFlag*76);
    tOsdImgInfo.uwYStart = 0;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_GetPairCamInfo(void)
{
    uint8_t i;

    for (i = 0; i < 4 ; i++)
    {
        printd(Apk_DebugLvl, "tUI_CamStatus[%d].ulCAM_ID  %lx \n",i,tUI_CamStatus[i].ulCAM_ID);
        if (tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
        {
            ubCamPairFlag[i] = 1;
        }
        else
        {
            ubCamPairFlag[i] = 0;
        }
        printd(Apk_DebugLvl, "ubCamPairFlag[%d]= %d \n",i,ubCamPairFlag[i] );
    }


    for (i = 0; i < 4 ; i++)
    {
        if (tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
        {
            ubPairSelCam = i;
            break;
        }
    }

    if ((ubCamPairFlag[0] == 1)&&(ubCamPairFlag[1] == 1)
        &&(ubCamPairFlag[2] == 1)&&(ubCamPairFlag[3] == 1))
        ubCamFullFlag = 1;
    else
        ubCamFullFlag = 0;

    if ((ubCamPairFlag[0] == 0)&&(ubCamPairFlag[1] == 0)
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

    if (tUI_State == UI_MAINMENU_STATE)
    {
        ubSubMenuItemFlag = 0;
        ubSubMenuItemPreFlag = 1;

        UI_GetPairCamInfo();

        if (tUI_PuSetting.ubTotalBuNum == 0)
            ubDelCamitem =0;
        else
            ubDelCamitem = tUI_PuSetting.ubTotalBuNum - 1;

        UI_DrawSubMenuPage(CAMERAS_ITEM);
        ubUI_FastStateFlag = TRUE;
        return;
    }

    if (tUI_SyncAppState == APP_PAIRING_STATE) //if (tPairInfo.ubDrawFlag == TRUE)
    {
        if ((tArrowKey == UP_ARROW) || (tArrowKey == DOWN_ARROW) || (tArrowKey == RIGHT_ARROW) || (tArrowKey == ENTER_ARROW))
        {
            return;
        }
        else if ((tArrowKey == EXIT_ARROW) || (tArrowKey == LEFT_ARROW))
        {
            UI_PairingControl(tArrowKey);
            tUI_SyncAppState = APP_LOSTLINK_STATE;
            return;
        }
    }

    if ((ubShowCamPairFullSta == 1) && (ubSubMenuItemFlag == 0))
    {
        if ((tArrowKey == UP_ARROW) || (tArrowKey == DOWN_ARROW) || (tArrowKey == RIGHT_ARROW) || (tArrowKey == ENTER_ARROW))
        {
            return;
        }
        else if ((tArrowKey == EXIT_ARROW) || (tArrowKey == LEFT_ARROW))
        {
            tOsdImgInfo.uwHSize  = 672;
            tOsdImgInfo.uwVSize  = 729;
            tOsdImgInfo.uwXStart = 48;
            tOsdImgInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdImgInfo);
            ubShowCamPairFullSta = 0;
            return;
        }
    }

    switch (tArrowKey)
    {
    case UP_ARROW:
        ubSubMenuItemPreFlag =  ubSubMenuItemFlag;

        if (0 == ubSubMenuItemFlag) ubSubMenuItemFlag = 2;
        else  ubSubMenuItemFlag--;

        UI_CamSubmenuDisplay();
        break;

    case DOWN_ARROW:
        ubSubMenuItemPreFlag =  ubSubMenuItemFlag;

        if (++ubSubMenuItemFlag>=3) ubSubMenuItemFlag = 0;

        UI_CamSubmenuDisplay();
        break;

    case RIGHT_ARROW:
    case ENTER_ARROW:
        if (ubSubMenuItemFlag == 0)
        {
            for (i = 0; i < 4 ; i++)
            {
                if (tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
                {
                    ubPairSelCam = i;
                    break;
                }
            }
            //printd(Apk_DebugLvl, "ubPairSelCam %d \n",i,ubPairSelCam);

            if (ubCamFullFlag == 0)
            {
                ubPairDisplayTime = 60; //10
                tPairInfo.tPairSelCam = ubPairSelCam;
                tPairInfo.tDispLocation = ubPairSelCam;

                tUI_PairMessage.ubAPP_Event      = APP_PAIRING_START_EVENT;
                tUI_PairMessage.ubAPP_Message[0] = 2;       //! Message Length
                tUI_PairMessage.ubAPP_Message[1] = ubPairSelCam;//CAM1;
                tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
                tUI_PairMessage.ubAPP_Message[2] = ubPairSelCam;
                UI_SendMessageToAPP(&tUI_PairMessage);
                tPairInfo.ubDrawFlag             = TRUE;
                UI_DisableScanMode();

                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 48;
                tOsdImgInfo.uwYStart =284;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIRING+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 186;
                tOsdImgInfo.uwYStart =284;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                if (tUI_PuSetting.ubLangageFlag == 0)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_PAIR_SEC+ (37*1 ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 145;
                    tOsdImgInfo.uwYStart =450;
                    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                }
                else
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_PAIR_SEC+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 145;
                    tOsdImgInfo.uwYStart =450;
                    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                }
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 48;
                tOsdImgInfo.uwYStart =284;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIR_FULL+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart= 200;
                tOsdImgInfo.uwYStart =284;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                ubShowCamPairFullSta = 1;
            }
        }
        else
        {
            tUI_State = UI_SUBSUBMENU_STATE;

            if (ubSubMenuItemFlag == 2)
            {
                ubSubSubMenuItemFlag = tUI_PuSetting.ubScanTime;
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
        if (tUI_SyncAppState != APP_PAIRING_STATE)
        {
            tUI_State = UI_MAINMENU_STATE;
            UI_DrawMenuPage();
        }
        ubShowCamPairFullSta = 0;
        break;
    }
}

//------------------------------------------------------------------------------
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    int i = 0, j = 0;
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t ubMenu_MaxNum[3] = {3,ubDelCamitem,4};
    static uint8_t menu_minitem,menu_maxitem;

    if (ubSubMenuItemFlag == 0)
    {
        if ((tArrowKey == UP_ARROW) || (tArrowKey == DOWN_ARROW) || (tArrowKey == RIGHT_ARROW) || (tArrowKey == ENTER_ARROW))
        {
            return;
        }
    }

    switch (tArrowKey)
    {
    case UP_ARROW:
        if (ubSubMenuItemFlag == 1)
        {
            /*
            if (ubSubSubMenuItemFlag == 0)
                return;
            */
            for (i = 0; i < 4; i++)
            {
                if (ubCamPairFlag[i] == 1)
                {
                    menu_minitem = i;
                    break;
                }
            }
            //printf("menu_minitem %d\n", menu_minitem);
            if (ubSubSubMenuItemFlag != menu_minitem)
            {
                for (i = (ubSubSubMenuItemFlag - 1); i >= 0; i--)
                {
                    if (ubCamPairFlag[i] == 1)
                    {
                        ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                        ubSubSubMenuItemFlag    = i;
                        break;
                    }
                }
            }
            else
            {
                for (i = 3; i >= 0; i--)
                {
                    if (ubCamPairFlag[i] == 1)
                    {
                        ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                        ubSubSubMenuItemFlag    = i;
                        //printf("ubSubSubMenuItemFlag %d\n", ubSubSubMenuItemFlag);
                        break;
                    }
                }
            }
        }
        else
        {
            ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;

            if (0 == ubSubSubMenuItemFlag) ubSubSubMenuItemFlag = ubMenu_MaxNum[ubSubMenuItemFlag];
            else  ubSubSubMenuItemFlag--;
        }
        UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);
        break;

    case DOWN_ARROW:
        if (ubSubMenuItemFlag == 1)
        {
            //if (ubSubSubMenuItemFlag >= 3)
            //return;
            for (i = 3; i >= 0; i--)
            {
                if (ubCamPairFlag[i] == 1)
                {
                    menu_maxitem = i;
                    break;
                }
            }
            //printf("menu_maxitem %d\n", menu_maxitem);
            if (ubSubSubMenuItemFlag != menu_maxitem)
            {
                for (i = (ubSubSubMenuItemFlag + 1); i < 4; i++)
                {
                    if (ubCamPairFlag[i] == 1)
                    {
                        ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                        ubSubSubMenuItemFlag    = i;
                        break;
                    }
                }
            }
            else
            {
                for (i = 0; i < 4; i++)
                {
                    if (ubCamPairFlag[i] == 1)
                    {
                        ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                        ubSubSubMenuItemFlag    = i;
                        //printf("ubSubSubMenuItemFlag %d\n", ubSubSubMenuItemFlag);
                        break;
                    }
                }

            }
        }
        else
        {
            ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
            if (++ubSubSubMenuItemFlag>=(ubMenu_MaxNum[ubSubMenuItemFlag]+1)) ubSubSubMenuItemFlag = 0;

        }
        UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);
        break;

    case RIGHT_ARROW:
        if (ubSubMenuItemFlag == 1)
        {
            i = ubSubSubMenuItemFlag;
            for (j = 0; j < 4; j++)
            {
                if (ubCamPairFlag[i] == 1)
                {
                    ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                    ubSubSubMenuItemFlag    = i;
                    break;
                }
                i++;
                if (i > 3) i = 0;
            }

            if ((ubCamPairFlag[0] == 0) && (ubCamPairFlag[1] == 0)&& (ubCamPairFlag[2] == 0)&& (ubCamPairFlag[3] == 0))
                break;

            ubSubSubSubMenuItemFlag = 0;
            ubSubSubSubMenuItemPreFlag = 1;

            tUI_State = UI_SUBSUBSUBMENU_STATE;
            UI_DeletemenuDisplay();
        }
        break;

    case ENTER_ARROW:
        if (ubSubMenuItemFlag == 1)
        {
            i = ubSubSubMenuItemFlag;
            for (j = 0; j < 4; j++)
            {
                if (ubCamPairFlag[i] == 1)
                {
                    ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                    ubSubSubMenuItemFlag    = i;
                    break;
                }
                i++;
                if (i > 3) i = 0;
            }

            if ((ubCamPairFlag[0] == 0) && (ubCamPairFlag[1] == 0)&& (ubCamPairFlag[2] == 0)&& (ubCamPairFlag[3] == 0))
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
        UI_GetPairCamInfo();
        ubMenuKeyPairing = 0;
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

    if (type)
    {
        if (tUI_CamStatus[CameraId].ulCAM_ID != INVALID_ID)
        {
            tUI_CamStatus[CameraId].ulCAM_ID    = INVALID_ID;
            tUI_CamStatus[CameraId].tCamConnSts = CAM_OFFLINE;
            tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
            tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == CameraId)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
            UI_ResetDevSetting(CameraId);
            UI_UpdateDevStatusInfo();
            tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
            tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
            tUI_UnindBuMsg.ubAPP_Message[1] = CameraId;
            UI_SendMessageToAPP(&tUI_UnindBuMsg);
            ubCamPairFlag[CameraId] = 0;
        }
    }
    else
    {
        tUI_CamStatus[CameraId].ulCAM_ID    = INVALID_ID;
        tUI_CamStatus[CameraId].tCamConnSts = CAM_OFFLINE;
        tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
        tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == CameraId)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
        UI_ResetDevSetting(CameraId);
        UI_UpdateDevStatusInfo();
        tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
        tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
        tUI_UnindBuMsg.ubAPP_Message[1] = CameraId;
        UI_SendMessageToAPP(&tUI_UnindBuMsg);
        ubCamPairFlag[CameraId] = 0;
    }
}

void UI_CamSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    int i = 0;
    OSD_IMG_INFO tOsdImgInfo;

    switch (tArrowKey)
    {
    case UP_ARROW:
        ubSubSubSubMenuItemPreFlag = ubSubSubSubMenuItemFlag;
        if (0 == ubSubSubSubMenuItemFlag) ubSubSubSubMenuItemFlag = 1;
        else ubSubSubSubMenuItemFlag--;
        UI_DeletemenuDisplay();
        break;

    case DOWN_ARROW:
        ubSubSubSubMenuItemPreFlag = ubSubSubSubMenuItemFlag;
        if (++ubSubSubSubMenuItemFlag>=2) ubSubSubSubMenuItemFlag = 0;
        UI_DeletemenuDisplay();
        break;

//  case RIGHT_ARROW:
    case ENTER_ARROW:
        if (ubSubSubSubMenuItemFlag == 0) // delete
        {
            APP_EventMsg_t tUI_UnindBuMsg = {0};

            tUI_CamStatus[ubSubSubMenuItemFlag].ulCAM_ID    = INVALID_ID;
            tUI_CamStatus[ubSubSubMenuItemFlag].tCamConnSts = CAM_OFFLINE;
            tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
            tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == ubSubSubMenuItemFlag)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
            UI_ResetDevSetting(ubSubSubMenuItemFlag);
            UI_UpdateDevStatusInfo();
            tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
            tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
            tUI_UnindBuMsg.ubAPP_Message[1] = ubSubSubMenuItemFlag;
            UI_SendMessageToAPP(&tUI_UnindBuMsg);
            ubCamPairFlag[ubSubSubMenuItemFlag] = 0;
            UI_GetPairCamInfo();
            if (ubNoAddCamFlag == 1)
            {
                ubFastShowLostLinkSta = 1;
            }
        }
        tUI_State = UI_SUBSUBMENU_STATE;
        tOsdImgInfo.uwHSize  = 672;
        tOsdImgInfo.uwVSize  = 284;
        tOsdImgInfo.uwXStart = 48;
        tOsdImgInfo.uwYStart = 0;
        OSD_EraserImg2(&tOsdImgInfo);
        UI_GetPairCamInfo();

        for (i = 0; i < 4 ; i++)
        {
            if (tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
            {
                tCamViewSel.tCamViewPool[0] = i;
                ubSubSubMenuItemFlag = i;
                break;
            }
        }

        UI_CamSubSubmenuDisplay(ubSubMenuItemFlag);

        if (ubNoAddCamFlag == 1)
            tCamViewSel.tCamViewPool[0] = CAM1;

        tCamViewSel.tCamViewType    = SINGLE_VIEW;
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

    switch (value)
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

//  UI_UpdateDevStatusInfo();

    printd(Apk_DebugLvl, "UI_SetScanMenu ubScanTime: %d.\n", tUI_PuSetting.ubScanTime);
    if (tUI_PuSetting.ubScanTime == 0)
    {
        UI_DisableScanMode();
    }
    else
    {
        UI_EnableScanMode();
    }
}

void UI_CameraResetCycleTime(uint8_t KeyAction)
{
    if (ubFactoryModeFLag == 1)
        return;

    if (KeyAction == KEY_UP_ACT)
    {
        UI_SetupScanModeTimer(TRUE);
    }
    //else if (KeyAction == KEY_DOWN_ACT)
    else
    {
        UI_SetupScanModeTimer(FALSE);
    }
}

//------------------------------------------------------------------------------
//#define PAIRING_ICON_NUM  2
void UI_DrawPairingStatusIcon(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    /*
    if (ubPairDisplayCnt < 3)
    {
        ubPairDisplayCnt++;
    }
    else
    {
        ubPairDisplayCnt = 0;

        if (ubPairDisplayTime == 0)
            ubPairDisplayTime = 0;
        else
            ubPairDisplayTime -= 1;
    }
    */
	ubPairDisplayTime = ulPAIR_TimeCnt /1000;
	
	if(ubFactoryModeFLag == 0)
		ubPairDisplayTime = 60 - ubPairDisplayTime;
	else
	{
		ubPairDisplayTime = 20 - ubPairDisplayTime;
		if(ubPairDisplayTime == 0)
                UI_PairingControl(EXIT_ARROW);
	}



    printd(1,"UI_DrawPairingStatusIcon ubPairDisplayTime=%d.\n",ubPairDisplayTime);
    if(ubFactoryModeFLag == 0)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIRING+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 186;
        tOsdImgInfo.uwYStart = 284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_PAIR_0+(ubPairDisplayTime%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 144;
        tOsdImgInfo.uwYStart = 520; //488
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_PAIR_0+(ubPairDisplayTime/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 144;
        tOsdImgInfo.uwYStart = 541;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_PAIR_0+(ubPairDisplayTime%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 344;
        tOsdImgInfo.uwYStart = 720; //488
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_PAIR_0+(ubPairDisplayTime/10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 344;
        tOsdImgInfo.uwYStart = 741;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
}

void UI_CamSubmenuDisplay(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD+(ubSubMenuItemPreFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 270+(ubSubMenuItemPreFlag*76);
    tOsdImgInfo.uwYStart = 729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_ADD_S+(ubSubMenuItemFlag*2)+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 270+(ubSubMenuItemFlag*76);
    tOsdImgInfo.uwYStart = 729;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAMERA, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 48+(96*5);
    tOsdImgInfo.uwYStart = 1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

//------------------------------------------------------------------------------
void UI_ReportPairingResult(UI_Result_t tResult)
{
    OSD_IMG_INFO tOsdImgInfo;
    APP_EventMsg_t tUI_UnindBuMsg = {0};
    UI_CamNum_t tCamNum;

    tPairInfo.ubDrawFlag = FALSE;

    switch (tResult)
    {
    case rUI_SUCCESS:

if (ubFactoryModeFLag == 0)
{
        tPairInfo.tPairSelCam= ubPairSelCam;
        tPairInfo.tDispLocation = ubPairSelCam;
        ubMenuKeyPairing = 0;
        ubCamPairOkState = 2;

        tUI_PuSetting.ubPairedBuNum += (tUI_PuSetting.ubPairedBuNum >= tUI_PuSetting.ubTotalBuNum)?0:1;
        tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
        tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
        for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
        {
            if (tCamNum == tPairInfo.tPairSelCam)
                continue;
            if ((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
               (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
            {
                tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
                tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
                tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
                UI_SendMessageToAPP(&tUI_UnindBuMsg);
                tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
                tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
                tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;

                UI_ResetDevSetting(tCamNum);
            }
        }
        if ((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
           (NO_CAM == tUI_PuSetting.tAdoSrcCamNum) ||
           (INVALID_ID == tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].ulCAM_ID))
            tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;

        ubPairDisplayTime = 60; //10

       // if (ubFactoryModeFLag == 1)
            //ubPairDisplayTime = 20;

        UI_ResetDevSetting(tPairInfo.tPairSelCam);
        UI_UpdateDevStatusInfo();
        UI_AutoLcdSetSleepTime(tUI_PuSetting.ubSleepMode);

        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 48;
            tOsdImgInfo.uwYStart = 284;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIR_OK+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 235;
            tOsdImgInfo.uwYStart = 284 + 31;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
}
else
{
        tPairInfo.tPairSelCam= ubPairSelCamcnt;
        tPairInfo.tDispLocation = ubPairSelCamcnt;

        tUI_PuSetting.ubPairedBuNum += (tUI_PuSetting.ubPairedBuNum >= tUI_PuSetting.ubTotalBuNum)?0:1;
        tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
        tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
        for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
        {
            if (tCamNum == tPairInfo.tPairSelCam)
                continue;
            if ((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
               (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
            {
                tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
                tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
                tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
                UI_SendMessageToAPP(&tUI_UnindBuMsg);
                tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
                tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
                tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;

                UI_ResetDevSetting(tCamNum);
            }
        }
        if ((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
           (NO_CAM == tUI_PuSetting.tAdoSrcCamNum) ||
           (INVALID_ID == tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].ulCAM_ID))
            tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;


        UI_ResetDevSetting(tPairInfo.tPairSelCam);
        UI_UpdateDevStatusInfo();

	ubPairSelCamcnt++;
	if(ubPairSelCamcnt >=3)
		ubPairSelCamcnt = 0;	

}

	
        UI_GetPairCamInfo();

#if 0
        if (tCamViewSel.tCamViewPool[0] != tPairInfo.tPairSelCam)
        {
            ubPairOK_SwitchCam = 1;
            tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam;
        }
#else
//      if (tCamViewSel.tCamViewPool[0] != tPairInfo.tPairSelCam)
        {
            tCamViewSel.tCamViewType    = SINGLE_VIEW;
            tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam;
            tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
            UI_SwitchCameraSource();
        }
#endif

        break;
    case rUI_FAIL:
        if(ubMenuKeyPairing == 1)
        {
            ubMenuKeyPairing = 2;
        }
        if(ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 48;
            tOsdImgInfo.uwYStart = 284;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_PAIR_FAIL+ (37*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 234;
            tOsdImgInfo.uwYStart = 284 +31;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }

        break;
    default:
        break;
	}
	if(ubFactoryModeFLag)
   		 tUI_State = UI_DISPLAY_STATE;
	else
   		 tUI_State = UI_SUBSUBMENU_STATE;		
	
}

void UI_DrawSettingSubMenuPage_NoSel(uint8_t type)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

	#if UI_NOTIMEMENU

		for(i = 0; i < 6; i++)
		{	
			if(i<4)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+(i*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 156+(i*76);
				tOsdImgInfo.uwYStart =729;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			else
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+((i+1)*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart= 156+(i*76);
				tOsdImgInfo.uwYStart =729;	
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
		}
		
	#else
	for(i = 0; i < 7; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+(i*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 118+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}
	#endif
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SETTING_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*6);
	tOsdImgInfo.uwYStart =1173;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE+type);	
}

void UI_DrawSettingSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;

#if UI_NOTIMEMENU
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 156+(0*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
#else
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 118+(0*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
#endif	

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SETTING, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*6);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_SettingSubmenuDisplay(void)
{
    OSD_IMG_INFO tOsdImgInfo;

	#if UI_NOTIMEMENU
		if(ubSubMenuItemFlag < 4)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+(ubSubMenuItemFlag*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 156+(ubSubMenuItemFlag*76);
			tOsdImgInfo.uwYStart =729;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
		else
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+((ubSubMenuItemFlag+1)*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 156+(ubSubMenuItemFlag*76);
			tOsdImgInfo.uwYStart =729;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}

		if(ubSubMenuItemPreFlag < 4)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+(ubSubMenuItemPreFlag*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 156+(ubSubMenuItemPreFlag*76);
			tOsdImgInfo.uwYStart =729;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
		else
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+((ubSubMenuItemPreFlag+1)*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart= 156+(ubSubMenuItemPreFlag*76);
			tOsdImgInfo.uwYStart =729;	
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
		
	#else
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT+(ubSubMenuItemPreFlag*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 118+(ubSubMenuItemPreFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CAM_NIGHT_S+(ubSubMenuItemFlag*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 118+(ubSubMenuItemFlag*76);
	tOsdImgInfo.uwYStart =729;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	#endif

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SETTING, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48+(96*6);
    tOsdImgInfo.uwYStart =1173;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//  OSD_IMG_INFO tOsdImgInfo;

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
			#if UI_NOTIMEMENU
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				if(0 == ubSubMenuItemFlag) ubSubMenuItemFlag = 5;
				else  ubSubMenuItemFlag--;	
			#else
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				if(0 == ubSubMenuItemFlag) ubSubMenuItemFlag = 6;
				else  ubSubMenuItemFlag--;		
			#endif	
			UI_SettingSubmenuDisplay();
		break;	

		case DOWN_ARROW:	
			#if UI_NOTIMEMENU
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				if(++ubSubMenuItemFlag>=6) ubSubMenuItemFlag = 0;	
			#else
				ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
				if(++ubSubMenuItemFlag>=7) ubSubMenuItemFlag = 0;	
			#endif
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

static void UI_DisplaySN(int x, int y, char *sn, int len)
{
    OSD_IMG_INFO tOsdImgInfo;
    int idx, i;

    for (i=0; i<len; i++) {
        printd(Apk_DebugLvl,"Factory mode sn[%d] = %c\n", i, sn[i]);
    }

    for (i=0; i<len && sn[i]; i++)
    {
        if (sn[i] == ' ') {
            tOsdImgInfo.uwHSize  = 46;
            tOsdImgInfo.uwVSize  = 22;
            tOsdImgInfo.uwXStart = x;
            tOsdImgInfo.uwYStart = y - (20 * i);
            OSD_EraserImg2(&tOsdImgInfo);
            continue;
        }

        idx = sn[i] <= '9' ? sn[i] - '0' : sn[i] - 'A' + 10;
        if (idx < 0 || idx > 36) idx = 'X' - 'A' + 10;
        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SN_NUM0 + idx, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = x;
        tOsdImgInfo.uwYStart = y - (20 * i);
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
}

void UI_DrawSettingSubSubMenuPage(uint8_t SubMenuItem)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    switch (SubMenuItem)
    {
    case NIGHTMODE_ITEM:
        for (i = 0; i < 4; i++)
        {
            if (ubCamPairFlag[i])
            {
                ubSubSubMenuItemFlag    = i;
                break;
            }
        }
        if ((i == 3)&&(ubCamPairFlag[3] == 0))
            break;

        UI_NightModeDisplay(ubSubSubMenuItemFlag);
        break;

    case LANGUAGESET_ITEM:
        ubSubSubMenuRealItem = 0;
        ubSubSubMenuItemFlag = tUI_PuSetting.ubLangageFlag;
        UI_LangageDisplay(ubSubSubMenuItemFlag);
        break;

    case FLICKER_ITEM:
        ubSubSubMenuItemFlag = tUI_PuSetting.ubFlickerFlag;
        UI_FlickerDisplay(ubSubSubMenuItemFlag);
        break;

    case DEFAULT_ITEM:
        UI_DefualtDisplay(ubSubSubMenuItemFlag);
        break;

    case TEMPUNIT_ITEM:
        ubSubSubMenuItemFlag = tUI_PuSetting.ubTempunitFlag;
        UI_TempUnitDisplay(ubSubSubMenuItemFlag);
        break;

    case PRODUCT_INFO_ITEM:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SN_TITLE, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 436;
        tOsdImgInfo.uwYStart = 631;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        UI_DisplaySN(436, 605, RxSNdata, sizeof(RxSNdata));
        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_INFO_INFO + (70*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 290;
        tOsdImgInfo.uwYStart = 377;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        UI_VerDisplay();
        break;

    case CONTACT_ITEM:
        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_CONTACT_INFO, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 128;
        tOsdImgInfo.uwYStart = 284;
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

    for (i = 0; i < 4; i++)
    {
        if ((tUI_PuSetting.NightmodeFlag >> i) & (0x01) == 1)
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_ON_NOPAIR+(i*3)+ubCamPairFlag[i] + (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        else
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_OFF_NOPAIR+(i*3)+ubCamPairFlag[i] + (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 233+(i*76);
        tOsdImgInfo.uwYStart = 284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    i = value;
    for (j = 0; j < 4; j++)
    {
        if (ubCamPairFlag[i] == 1)
        {
            if ((tUI_PuSetting.NightmodeFlag >> i) & (0x01) == 1)
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_ON+(i*3)+ubCamPairFlag[i]+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            else
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_CAM1_OFF+(i*3)+ubCamPairFlag[i]+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 233+(i*76);
            tOsdImgInfo.uwYStart = 284;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            break;
        }
        i++;
        if (i > 3)
            i = 0;
    }
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_LangageDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i = 0; i < 4; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LANGAGE_0+(i*2)+ (70*i), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 232+(i*76);
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_LANGAGE_0_S+(value*2)+ (70*value), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 232+(value*76);
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 260 + (tUI_PuSetting.ubLangageFlag*76);
    tOsdImgInfo.uwYStart = 565 +25;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_FlickerDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FLICKER_INFO+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 240 - 24;
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    for (i = 0; i < 2; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FLICKER_50+(i*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 392+(i*76);
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FLICKER_50_S+(value*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 392+(value*76);
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 422 +(tUI_PuSetting.ubFlickerFlag*76);
    tOsdImgInfo.uwYStart = 596;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_DefualtDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_DEFUALT_INFO+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 240 -24;
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

    for (i = 0; i < 2; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NO-(i*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 392+(i*76);
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NO_S-(value*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 392+(value*76);
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_TempUnitDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    for (i = 0; i < 2; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TEMP_F+(i*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 308+(i*76);
        tOsdImgInfo.uwYStart =284;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_TEMP_F_S+(value*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 308+(value*76);
    tOsdImgInfo.uwYStart =284;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB2_POINT, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 338 +(tUI_PuSetting.ubTempunitFlag*76);
    tOsdImgInfo.uwYStart = 658;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_NightModeSubSubSubmenuDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUBSUBSUBMENU_BG, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 48;
    tOsdImgInfo.uwYStart =0;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    for (i = 0; i < 2; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NIG_OFF+(i*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 309+(i*76);
        tOsdImgInfo.uwYStart =0;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NIG_OFF_S+(ubSubSubSubMenuItemFlag*2)+ (70*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 309+(ubSubSubSubMenuItemFlag*76);
    tOsdImgInfo.uwYStart =0;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (tUI_PuSetting.ubLangageFlag == 1)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 338+(ubSubSubSubMenuItemFlag*76);
        tOsdImgInfo.uwYStart =167+ 40;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
    else if (tUI_PuSetting.ubLangageFlag == 3)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 338+(ubSubSubSubMenuItemFlag*76);
        tOsdImgInfo.uwYStart =167+ 70;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_SUB3_POINT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 338+(ubSubSubSubMenuItemFlag*76);
        tOsdImgInfo.uwYStart =167;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    }
}

//------------------------------------------------------------------------------
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    OSD_IMG_INFO tOsdImgInfo;

    switch (ubSubMenuItemFlag)
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


    switch (tArrowKey)
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

    case RIGHT_ARROW:
        if (NIGHTMODE_ITEM == ubSubMenuItemFlag)
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
    static uint8_t subsubmenu_maxitem;

    switch (SubMenuItem)
    {
    case NIGHTMODE_ITEM:
        for (i = 3; i >= 0; i--)
        {
            if (ubCamPairFlag[i] == 1)
            {
                subsubmenu_maxitem = i;
                break;
            }
        }

        for (i = ubSubSubMenuItemFlag; i < 4; i++)
        {
            if (ubCamPairFlag[i])
            {
                ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                ubSubSubMenuItemFlag    = i;
                break;
            }
        }
        if ((i == 3)&&(ubCamPairFlag[3] == 0))
            break;

        if (ubSubSubMenuItemFlag != 0)
            ubSubSubMenuItemFlag -= 1;
        else
            ubSubSubMenuItemFlag = subsubmenu_maxitem;

        UI_NightModeDisplay(ubSubSubMenuItemFlag);
        break;

    case LANGUAGESET_ITEM:
        if (0 == ubSubSubMenuItemFlag) ubSubSubMenuItemFlag = 3;
        else  ubSubSubMenuItemFlag--;

        ubLangageFlag = ubSubSubMenuItemFlag;
        UI_LangageDisplay(ubLangageFlag);
        break;

    case FLICKER_ITEM:
        if (ubSubSubMenuItemFlag == 1)
            ubSubSubMenuItemFlag = 0;
        else
            ubSubSubMenuItemFlag =1;

        ubFlickerFlag = ubSubSubMenuItemFlag;
        UI_FlickerDisplay(ubFlickerFlag);
        break;

    case DEFAULT_ITEM:
        if (ubSubSubMenuItemFlag == 1)
            ubSubSubMenuItemFlag = 0;
        else
            ubSubSubMenuItemFlag =1;

        ubDefualtFlag = ubSubSubMenuItemFlag;
        UI_DefualtDisplay(ubDefualtFlag);
        break;

    case TEMPUNIT_ITEM:
        if (ubSubSubMenuItemFlag == 1)
            ubSubSubMenuItemFlag = 0;
        else
            ubSubSubMenuItemFlag =1;

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
    static uint8_t  subsubmenu_minitem;

    switch (SubMenuItem)
    {
    case NIGHTMODE_ITEM:
        for (i = 0; i < 4; i++)
        {
            if (ubCamPairFlag[i])
            {
                subsubmenu_minitem = i;
                break;
            }
        }


        for (i = ubSubSubMenuItemFlag; i < 4; i++)
        {
            if (ubCamPairFlag[i])
            {
                ubSubSubMenuItemPreFlag = ubSubSubMenuItemFlag;
                ubSubSubMenuItemFlag    = i;
                break;
            }
        }
        if ((i == 3)&&(ubCamPairFlag[3] == 0))
            break;

        //if (ubSubSubMenuItemFlag >= 3)
        //  break;

        if (ubSubSubMenuItemFlag < 3)
            ubSubSubMenuItemFlag += 1;
        else
            ubSubSubMenuItemFlag =subsubmenu_minitem;

        UI_NightModeDisplay(ubSubSubMenuItemFlag);
        break;

    case LANGUAGESET_ITEM:
        if (++ubSubSubMenuItemFlag>=4) ubSubSubMenuItemFlag = 0;

        ubLangageFlag = ubSubSubMenuItemFlag;
        UI_LangageDisplay(ubLangageFlag);
        break;

    case FLICKER_ITEM:
        if (ubSubSubMenuItemFlag == 1)
            ubSubSubMenuItemFlag = 0;
        else
            ubSubSubMenuItemFlag =1;

        ubFlickerFlag = ubSubSubMenuItemFlag;
        UI_FlickerDisplay(ubFlickerFlag);
        break;
    case DEFAULT_ITEM:
        if (ubSubSubMenuItemFlag == 1)
            ubSubSubMenuItemFlag = 0;
        else
            ubSubSubMenuItemFlag =1;

        ubDefualtFlag = ubSubSubMenuItemFlag;
        UI_DefualtDisplay(ubDefualtFlag);
        break;

    case TEMPUNIT_ITEM:
        if (ubSubSubMenuItemFlag == 1)
            ubSubSubMenuItemFlag = 0;
        else
            ubSubSubMenuItemFlag =1;

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

    switch (SubMenuItem)
    {
    case NIGHTMODE_ITEM:
        if (ubNoAddCamFlag == 0)
        {
            tUI_State = UI_SUBSUBSUBMENU_STATE;
            ubSubSubSubMenuItemFlag = ((tUI_PuSetting.NightmodeFlag>>ubSubSubMenuItemFlag)&0x01)?1:0;
            ubSubSubSubMenuRealItem = 0;
            UI_NightModeSubSubSubmenuDisplay(ubSubSubSubMenuRealItem);
        }
        break;

    case LANGUAGESET_ITEM:
        tUI_PuSetting.ubLangageFlag = ubLangageFlag;
        UI_LangageDisplay(ubLangageFlag);
        UI_DrawSettingSubMenuPage_NoSel(0);
        UI_SettingSubmenuDisplay();
        //UI_UpdateDevStatusInfo();
        break;

    case FLICKER_ITEM:
        if (ubFlickerFlag == 0)
        {
            tUI_CamStatus[tSelCamNum].tCamFlicker = CAMFLICKER_50HZ;
        }
        else
        {
            tUI_CamStatus[tSelCamNum].tCamFlicker = CAMFLICKER_60HZ;
        }
        tUI_PuSetting.ubFlickerFlag = ubFlickerFlag;
        UI_FlickerDisplay(ubFlickerFlag);
        //UI_UpdateDevStatusInfo();
        break;

    case DEFAULT_ITEM:
        if (ubDefualtFlag == 1)
        {
            tUI_PuSetting.ubDefualtFlag = TRUE;

            tUI_PuSetting.ubZoomScale               = 0;
            tUI_PuSetting.VolLvL.tVOL_UpdateLvL     = VOL_LVL4;
            tUI_PuSetting.NightmodeFlag             = 0x0F;
            tUI_PuSetting.ubHighTempSetting         = 0;
            tUI_PuSetting.ubLowTempSetting          = 0;
            tUI_PuSetting.ubTempAlertSetting        = 1;
            tUI_PuSetting.ubSoundLevelSetting       = 0;
            tUI_PuSetting.ubSoundAlertSetting       = 1;
            tUI_PuSetting.ubTempunitFlag            = 0;
            tUI_PuSetting.ubFlickerFlag             = 1;
            tUI_PuSetting.ubSleepMode               = 1;
            tUI_PuSetting.BriLvL.tBL_UpdateLvL      = BL_LVL8;
            tUI_PuSetting.ubScanTime                = 2;
            tUI_PuSetting.ubScanModeEn              = TRUE;

            tUI_PuSetting.ubFeatCode0           = 0x00;
            tUI_PuSetting.ubFeatCode1           = 0x00;
            tUI_PuSetting.ubFeatCode2           = 0x00;

            LCDBL_ENABLE(FALSE);
            ubFactoryDeleteCam = 1;
            //UI_CamDeleteCamera(1, 0);
            printd(Apk_DebugLvl, "defulat ~~~\n");
            UI_UpdateDevStatusInfo();
            ubFactorySettingFlag = 0;

            tOsdImgInfo.uwHSize  = 720;
            tOsdImgInfo.uwVSize  = 1280;
            tOsdImgInfo.uwXStart = 0;
            tOsdImgInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdImgInfo); //复位重启以后OSD可能会有残留

            //osDelay(100);
            //WDT_Disable(WDT_RST);
            //WDT_RST_Enable(WDT_CLK_EXTCLK, 1);//reboot
            //while(1);
        }
    else
    {
        tUI_State = UI_SUBMENU_STATE;
            tOsdImgInfo.uwHSize  = 672;
            tOsdImgInfo.uwVSize  = 729;
            tOsdImgInfo.uwXStart = 48;
            tOsdImgInfo.uwYStart = 0;
            OSD_EraserImg2(&tOsdImgInfo);
        }
        break;

    case TEMPUNIT_ITEM:
        if (tUI_PuSetting.ubTempunitFlag != ubTempunitFlag)
            ubRealTemp = ubTempunitFlag?UI_TempFToC(ubRealTemp):UI_TempCToF(ubRealTemp);
        tUI_PuSetting.ubTempunitFlag  = ubTempunitFlag;
        UI_TempBarDisplay(ubRealTemp);
        UI_TempUnitDisplay(ubTempunitFlag);
        //UI_UpdateDevStatusInfo();
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

    switch (tArrowKey)
    {
    case UP_ARROW:
        ubSubSubSubMenuItemPreFlag = ubSubSubSubMenuItemFlag;

        if (ubSubSubSubMenuItemFlag == 1)
            ubSubSubSubMenuItemFlag = 0;
        else
            ubSubSubSubMenuItemFlag =1;
        UI_NightModeSubSubSubmenuDisplay(ubSubSubSubMenuItemFlag);
        break;

    case DOWN_ARROW:
        ubSubSubSubMenuItemPreFlag = ubSubSubSubMenuItemFlag;

        if (ubSubSubSubMenuItemFlag == 1)
            ubSubSubSubMenuItemFlag = 0;
        else
            ubSubSubSubMenuItemFlag =1;
        UI_NightModeSubSubSubmenuDisplay(ubSubSubSubMenuItemFlag);
        break;

    case RIGHT_ARROW:
        break;

    case ENTER_ARROW:
        tUI_PuSetting.NightmodeFlag &=(~(1<<ubSubSubMenuItemFlag));
        tUI_PuSetting.NightmodeFlag |=(ubSubSubSubMenuItemFlag<<ubSubSubMenuItemFlag);
        printd(Apk_DebugLvl, "UI_NightModeSubSubSubMenuPage tUI_PuSetting.NightmodeFlag: 0x%x.\n", tUI_PuSetting.NightmodeFlag);
        //UI_UpdateDevStatusInfo();
        UI_SendNightModeToBu();
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
void UI_FS_MenuDisplay(uint8_t value)
{
    if (value == 0)
    {
        UI_FS_LangageMenuDisplay(ubLangageFlag);
    }
    else
    {
        UI_FS_SetTimeMenuDisplay(ubFS_Timeitem);
    }
}

void UI_FS_LangageMenuDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_LANGAGE+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 184;
    tOsdImgInfo.uwYStart = 584;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    for (i =0 ; i < 4 ; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_ENGLISH+(i*2)+ (21*i), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 184+(i*96);
        tOsdImgInfo.uwYStart = 344;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_ENGLISH_S+(value*2)+ (21*value), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 184+(value*96);
    tOsdImgInfo.uwYStart = 344;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}


void UI_FS_SetTimeMenuDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
	printd(1,"1111111111111UI_FS_SetTimeMenuDisplay  tUI_PuSetting.tPsMode %x  value =%d \n",tUI_PuSetting.tPsMode,value);
	printd(1,"ubTimeAMPM %d  ubTimeMin %d ubTimeHour %d \n",ubTimeAMPM,ubTimeMin,ubTimeHour );

//  EN_MODE_UI  tEnUI;

/*
    tEnUI.tStatus = EN_FIX_HOPPING;                               //固定频点模式

    tEnUI.tRole = SET_STA1;                                       //在STA1端生效

    tEnUI.ubData1 = 2;                                            //在高频点生效，0、1、2代表低中高频点

    tEN_Start(tEnUI.tStatus,tEnUI.tRole,&tEnUI.ubData1,NULL);     //开启工程模式
*/

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 216;
    tOsdImgInfo.uwYStart =694;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_BACK2+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 434;
    tOsdImgInfo.uwYStart =679;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_FINISH2+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart= 434;
    tOsdImgInfo.uwYStart =217;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    switch (value)
    {
    case 0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 149;
        tOsdImgInfo.uwYStart =339;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIMEUPAROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167;
        tOsdImgInfo.uwYStart =632;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIMEDOWNAROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 288;
        tOsdImgInfo.uwYStart =632;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =371;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =479;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =513;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =574;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =612;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =646;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

        break;

    case 1:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 149;
        tOsdImgInfo.uwYStart =339;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIMEUPAROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167;
        tOsdImgInfo.uwYStart =497;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIMEDOWNAROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 288;
        tOsdImgInfo.uwYStart =497;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =371;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =479;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =513;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =574;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =612;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =646;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 2:
	//printd(1,"ubTimeAMPM %d  ubTimeMin %d ubTimeHour %d \n",ubTimeAMPM,ubTimeMin,ubTimeHour );
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 149;
        tOsdImgInfo.uwYStart =339;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIMEUPAROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 167;
        tOsdImgInfo.uwYStart =394;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIMEDOWNAROW, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 288;
        tOsdImgInfo.uwYStart =394;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_AM_S+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =371;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =479;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =513;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =574;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =612;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =646;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 3:
	//printd(1,"ubTimeAMPM %d  ubTimeMin %d ubTimeHour %d \n",ubTimeAMPM,ubTimeMin,ubTimeHour );
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 149;
        tOsdImgInfo.uwYStart =339;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =371;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =479;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =513;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =574;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =612;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =646;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_BACK1+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 434;
        tOsdImgInfo.uwYStart =679;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case 4:
	//printd(1,"ubTimeAMPM %d  ubTimeMin %d ubTimeHour %d \n",ubTimeAMPM,ubTimeMin,ubTimeHour );
	 tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_FSMENU_BLANK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 149;
        tOsdImgInfo.uwYStart =339;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
    //-----------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_AM+(ubTimeAMPM*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =371;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //-------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =479;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeMin/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =513;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =574;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    //-----------------------------------------------------------------------
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =612;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0+((ubTimeHour/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart= 216;
        tOsdImgInfo.uwYStart =646;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_FINISH1+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
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

    if (ubUpdateFWFlag == 1)return;

    if ((tCamViewSel.tCamViewPool[0] == tCamNum) && (CAM_ONLINE == tUI_CamStatus[tCamNum].tCamConnSts))
    {
        ubGetVoiceTemp = pvdata[0];
    }


    //printd(Apk_DebugLvl, "ubGetVoiceTemp %d \n",ubGetVoiceTemp);
    //printd(Apk_DebugLvl, "ubGetIR1Temp %d \n",ubGetIR1Temp);
    //printd(Apk_DebugLvl, "ubGetIR2Temp %d \n",ubGetIR2Temp);

    if ((tUI_PuSetting.ubDefualtFlag == FALSE)&&(ubClearOsdFlag == 1))
        UI_VolBarDisplay(ubGetVoiceTemp);
}


uint8_t UI_TempCToF(uint8_t cTemp)
{
    int16_t ctemp = ubTempBelowZoreSta ? -cTemp : cTemp;

    int16_t fTemp = 0;

    fTemp = ctemp*18/10 + (((ctemp*18%10) >= 5)?1:0) + 32;

   	printd(1, "UI_TempCToF cTemp: %d, fTemp: %d.\r\n", cTemp, fTemp);
	ubTempBelowZoreSta = fTemp? 0 : 1;
    return (uint8_t)abs(fTemp);
}


uint8_t UI_TempFToC(uint8_t fTemp)
{

    int16_t ftemp = ubTempBelowZoreSta ? -fTemp : fTemp;
	
    int16_t cTemp = 0;

    cTemp = ((ftemp-32)*10/18) + ((((ftemp-32)*10%18) >= 5)?1:0);
    printd(1, "UI_TempCToF cTemp: %d, fTemp: %d.\r\n", cTemp, fTemp);
	ubTempBelowZoreSta = cTemp? 0 : 1;
     return (uint8_t)abs(cTemp);
}

void UI_GetTempData(UI_CamNum_t tCamNum, void *pvTrig) //20180322
{
	uint8_t *pvdata = (uint8_t *)pvTrig;


    if (ubUpdateFWFlag == 1)return;


    if ((tCamViewSel.tCamViewPool[0] == tCamNum) && (CAM_ONLINE == tUI_CamStatus[tCamNum].tCamConnSts))
    {
        ubTempBelowZoreSta = pvdata[1];
		ubTempInvalidSta = pvdata[2];

		printd(Apk_DebugLvl, "UI_GetTempData ubRealTemp: %d, ubTempBelowZoreSta %d, ubTempInvalidSta %d. \n",ubRealTemp, ubTempBelowZoreSta, ubTempInvalidSta);
		if (ubTempInvalidSta == 1)
		{
			ubRealTemp = 0xFF;
			UI_TempBarDisplay(ubRealTemp);
			return;
		}
        ubRealTemp = tUI_PuSetting.ubTempunitFlag?pvdata[0]:UI_TempCToF(pvdata[0]);

#if TEMP_TEST
   		 OSD_IMG_INFO info;
		int16_t ubTempValue = 0;
	    	char   str[6] = {0};
		int i;

		    if (ubUpdateFWFlag == 1)return;
		//ubTempValue =ubTempBelowZoreSta ? -(int16_t)(( pvdata[3] << 8 )|pvdata[4]) : (int16_t)(( pvdata[3] << 8 )| pvdata[4]);
		ubTempValue =(int16_t)(( pvdata[3] << 8 )| pvdata[4]);
		printd(1, "UI_GetTempData  ubTempValue %d.pvdata[3] %d pvdata[4] %d  \n",ubTempValue,pvdata[3],pvdata[4]);
		 sprintf(str, "%5d", ubTempValue);
		 for(i = 0; str[i]; i++)
		{
			int img;
			
			 if (str[i] >= '0' && str[i] <= '9') 
			{
		            img = OSD2IMG_BAR_NUM_0 + (str[i] - '0');
		        }
			else if (str[i] == '-') 
		        {
		            img = OSD2IMG_TEMP_BELOW;
		        } 
			 else 
		        {
		            img = OSD2IMG_TEMP_BLANK;
		        }
			 tOSD_GetOsdImgInfor(1, OSD_IMG2, img, 1, &info);
		        info.uwXStart = 0;
		        info.uwYStart = 550 - 20 * i;
		        tOSD_Img2(&info, OSD_QUEUE);

		 }
			 tOSD_Img2(&info, OSD_UPDATE);
		
		
#endif
		
    }

    if (ubRealTemp > 199)
       return;
   printd(Apk_DebugLvl, "UI_GetTempData ubRealTemp: %d, ubTempunitFlag: %d. \n",ubRealTemp, tUI_PuSetting.ubTempunitFlag);
    if ((tUI_PuSetting.ubDefualtFlag == FALSE)&&(ubClearOsdFlag == 1))
    {
        UI_TempBarDisplay(ubRealTemp);
    }
}

void UI_GetBUCMDData(UI_CamNum_t tCamNum, void *pvTrig)
{
    uint8_t *pvdata = (uint8_t *)pvTrig;
    uint8_t ubBUCmdID = pvdata[0];

    switch (ubBUCmdID)
    {
    case UI_BU_CMD_VERSION:
        ubBuHWVersion = pvdata[1];
        ubBuSWVersion = pvdata[2]*10 + pvdata[3];
        break;

    case UI_BU_CMD_PS_MODE:
#if Current_Test
        /*
        if(ubGetPsModeFlag == 1)
        {
            if(pvdata[1] == PS_VOX_MODE)
            {
                UI_DisableVox();
            }
        }
        */
#endif
        break;

    case UI_BU_CMD_ALARM_TYPE:
        ubAlarmWakeupType = pvdata[1];
        break;

    case UI_BU_CMD_PICKUP_VOLUME:
        ubGetVoice1 = pvdata[1];
        ubGetVoice2 = pvdata[2];
        ubGetVoice3 = pvdata[3];
        ubGetVoice4 = pvdata[4];
        //printd(Apk_DebugLvl, "VolumeValue: %d.\n",(ubGetVoice1<<24)+(ubGetVoice2<<16)+(ubGetVoice3<<8)+ubGetVoice4);
        break;

    case UI_BU_CMD_SN_VALUE:
        TxSNdata[pvdata[5] + 0] = pvdata[1];
        TxSNdata[pvdata[5] + 1] = pvdata[2];
        TxSNdata[pvdata[5] + 2] = pvdata[3];
        TxSNdata[pvdata[5] + 3] = pvdata[4];
        break;

    case UI_BU_CMD_IR_VALUE:
        ubGetIR1Temp = pvdata[1];
        ubGetIR2Temp = pvdata[2];
        //printd(Apk_DebugLvl, "IR Temp %d ### \n",(ubGetIR1Temp<<8)+ubGetIR2Temp);
        break;
    default:
        break;
    }
}
/*
void UI_TempBarDisplay(uint8_t value)
{
	printd(Apk_DebugLvl,"UI_TempBarDisplay!!!!!!!\n");
    OSD_IMG_INFO tOsdImgInfo;

    if (ubUpdateFWFlag == 1)return;

    if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
        return;

    #if UI_NOTIMEMENU
	for(int i = 0;i < 3; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BLANK, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 162-(19*i);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
	}
	if(value == 0xFF)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 162;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 143;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 124;  
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		return;
	}

	if(value > 199)
	{
		return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BLANK, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 162;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BLANK, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 162;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 111; //901
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	if(ubTempBelowZoreSta == 1)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 162;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		if(value/100)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(value/100), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 162;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
	}

    if (value/10%10)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 +(value/10%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 143;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 +(value%10), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 124;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_TEMPC+(!tUI_PuSetting.ubTempunitFlag), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 93;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
#else
	for(int i = 0;i < 3; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BLANK, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 162+137+24-(19*i);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
	}
	if(value == 0xFF)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 162+137+24;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);


		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 143+137+24;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 124+137+24;  
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		return;
	}

	if(value >= 199)
	{
		return;
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 111+137+24; //901
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	if(ubTempBelowZoreSta == 1)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TEMP_BELOW, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 162+137+24;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		if(value/100)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(value/100), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 162+137+24;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		}
	}

    if (value/10%10)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 +(value/10%10), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 143+137+24;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0 +(value%10), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 124+137+24;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_TEMPC+(!tUI_PuSetting.ubTempunitFlag), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 93 +137+24;
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
#endif
}
*/

void UI_TempBarDisplay(uint8_t value)
{
    OSD_IMG_INFO info;
    int16_t temp  = ubTempBelowZoreSta ? -value : value;
    char   str[5] = {0};
    int    i;

    if (ubUpdateFWFlag == 1)return;
    if (tUI_SyncAppState != APP_LINK_STATE)
		return;
    //printd(Apk_DebugLvl,"value %d\n",value);
    if (value != 0xff)
	    sprintf(str, "%4d", temp);
    else
	    strcpy(str, " ---");
	
    for (i=0; str[i]; i++) 
    {
        int img;
        if (str[i] >= '0' && str[i] <= '9') 
	{
            img = OSD2IMG_BAR_NUM_0 + (str[i] - '0');
        }
	 else if (str[i] == '-') 
        {
            img = OSD2IMG_TEMP_BELOW;
        } 
	 else 
        {
            img = OSD2IMG_TEMP_BLANK;
        }
        tOSD_GetOsdImgInfor(1, OSD_IMG2, img, 1, &info);
        info.uwXStart = 0;
        info.uwYStart = 350 - 20 * i;
        tOSD_Img2(&info, OSD_QUEUE);
    }
	
    tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_TEMPC + (!tUI_PuSetting.ubTempunitFlag), 1, &info);
    info.uwXStart = 0;
    info.uwYStart = 350 - 20 * i - 12;
    tOSD_Img2(&info, OSD_UPDATE);
}


void UI_VolBarDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t i;
    if (ubUpdateFWFlag == 1)return;
    if (tUI_SyncAppState != APP_LINK_STATE)
		return;
    if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
        return;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_VOLCHECK, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 1089;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE); 

    if (value != 0)
    {
        for (i = 1; i< 6; i++)
        {
            if (i <= value)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_VOL1, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 0;
                tOsdImgInfo.uwYStart = 1065 - 30*(i-1);
            }
            else
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_VOL0, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 0;
                tOsdImgInfo.uwYStart = 1065 - 30*(i-1);
            }
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
    }
    else
    {
        for (i = 0; i< 5; i++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_VOL0, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 0;
            tOsdImgInfo.uwYStart = 1065 - 30*i;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
    }
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

uint8_t UI_CamNightModeCmd(uint8_t CameraId, uint8_t NightMode)
{
    UI_PUReqCmd_t tUI_NightModeReqCmd;

    //printd(Apk_DebugLvl, "UI_CamNightModeCmd CameraId: %d, NightMode: %d.\n", CameraId, NightMode);
    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
        (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
    {
        tUI_NightModeReqCmd.tDS_CamNum                  = tCamViewSel.tCamViewPool[0];
        tUI_NightModeReqCmd.ubCmd[UI_TWC_TYPE]          = UI_SETTING;
        tUI_NightModeReqCmd.ubCmd[UI_SETTING_ITEM]      = UI_NIGHTMODE_SETTING;
        tUI_NightModeReqCmd.ubCmd[UI_SETTING_DATA]      = CameraId;
        tUI_NightModeReqCmd.ubCmd[UI_SETTING_DATA+1]    = NightMode;
        tUI_NightModeReqCmd.ubCmd_Len                   = 4;

        if (UI_SendRequestToBU(osThreadGetId(), &tUI_NightModeReqCmd) != rUI_SUCCESS)
        {
            printd(DBG_ErrorLvl, "UI_CamNightModeCmd Fail!\n");
            return rUI_FAIL;
        }
        return rUI_SUCCESS;
    }

    return rUI_FAIL;
}

uint8_t UI_SendNightModeToBu(void)
{
    return UI_CamNightModeCmd(tCamViewSel.tCamViewPool[0], (tUI_PuSetting.NightmodeFlag>>tCamViewSel.tCamViewPool[0])&0x01);
}

uint8_t UI_CamvLDCModeCmd(uint8_t value)
{
    UI_PUReqCmd_t tUI_CamvLDCModeCmd;

    printd(Apk_DebugLvl, "UI_CamvLDCModeCmd value: %d.\n", value);
    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
        (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
    {
        tUI_CamvLDCModeCmd.tDS_CamNum               = tCamViewSel.tCamViewPool[0];
        tUI_CamvLDCModeCmd.ubCmd[UI_TWC_TYPE]       = UI_SETTING;
        tUI_CamvLDCModeCmd.ubCmd[UI_SETTING_ITEM]   = UI_IMGPROC_SETTING;
        tUI_CamvLDCModeCmd.ubCmd[UI_SETTING_DATA]   = UI_IMGvLDC_SETTING;
        tUI_CamvLDCModeCmd.ubCmd[UI_SETTING_DATA+1] = value;
        tUI_CamvLDCModeCmd.ubCmd_Len                = 4;
        if (UI_SendRequestToBU(osThreadGetId(), &tUI_CamvLDCModeCmd) != rUI_SUCCESS)
        {
            printd(DBG_ErrorLvl, "UI_CamvLDCModeCmd Fail!\n");
            return rUI_FAIL;
        }
        return rUI_SUCCESS;
    }

    return rUI_FAIL;
}

uint8_t UI_SendToBUCmd(uint8_t *data, uint8_t data_len)
{
    int i;
    UI_PUReqCmd_t tUI_SendCmd;

    printd(1, "UI_SendToBUCmd CMD: 0x%x, tCamViewPool[0]: %d, ConnSts: %d, ID: 0x%x.\n",data[0], tCamViewSel.tCamViewPool[0], tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts, tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID);
    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
        (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
    {
        tUI_SendCmd.tDS_CamNum              = tCamViewSel.tCamViewPool[0];
        tUI_SendCmd.ubCmd[UI_TWC_TYPE]      = UI_SETTING;
        tUI_SendCmd.ubCmd[UI_SETTING_ITEM]  = UI_PU_TO_BU_CMD_SETTING;
        for (i = 0; i < data_len; i++)
        {
            tUI_SendCmd.ubCmd[UI_SETTING_DATA+i] = data[i];
        }
        tUI_SendCmd.ubCmd_Len               = 2 + data_len;
        if (UI_SendRequestToBU(osThreadGetId(), &tUI_SendCmd) != rUI_SUCCESS)
        {
            printd(DBG_ErrorLvl, "UI_SendToBUCmd Fail!\n");
            return rUI_FAIL;
        }
        return rUI_SUCCESS;
    }

    return rUI_FAIL;
}

uint8_t UI_GetBuPsMode(void)
{
    uint8_t CmdData = UI_GET_BU_PS_MODE_CMD;
    return UI_SendToBUCmd(&CmdData, 1);
}

uint8_t UI_GetBuVersion(void)
{
    uint8_t CmdData = UI_GET_BU_VERSION_CMD;
    return UI_SendToBUCmd(&CmdData, 1);
}


uint8_t UI_GetBuMICTest(void)
{
    uint8_t CmdData = UI_SET_BU_ADO_TEST_CMD;
    return UI_SendToBUCmd(&CmdData, 1);
}


uint8_t UI_SendAlarmSettingToBu(void)
{
    uint8_t CmdData[4] = {0};

    CmdData[0] = UI_SET_BU_ALARM_VALUE_CMD;

    CmdData[1] = ubHighTempC[tUI_PuSetting.ubHighTempSetting];
    CmdData[2] = ubLowTempC[tUI_PuSetting.ubLowTempSetting];
    CmdData[3] = tUI_PuSetting.ubSoundLevelSetting;

    return UI_SendToBUCmd(CmdData, 4);
}

uint8_t UI_SendPwrNormalModeToBu(void)
{
    UI_PUReqCmd_t tPwrCmd;

    if (tUI_PuSetting.tPsMode != POWER_NORMAL_MODE)
    {
        if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
            (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
        {
            tPwrCmd.tDS_CamNum              = tCamViewSel.tCamViewPool[0];
            tPwrCmd.ubCmd[UI_TWC_TYPE]      = UI_SETTING;
            tPwrCmd.ubCmd[UI_SETTING_ITEM]  = UI_VOXMODE_SETTING;
            tPwrCmd.ubCmd[UI_SETTING_DATA]  = POWER_NORMAL_MODE;
            tPwrCmd.ubCmd_Len               = 3;
            if (UI_SendRequestToBU(osThreadGetId(), &tPwrCmd) != rUI_SUCCESS)
            {
                printd(Apk_DebugLvl, "UI_SendPwrNormalModeToBu Fail!\n");
                return rUI_FAIL;
            }
        }
        tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
        printd(Apk_DebugLvl, "UI_SendPwrNormalModeToBu Cam%d ok.\n", tCamViewSel.tCamViewPool[0]);
        return rUI_SUCCESS;
    }

    return rUI_FAIL;
}

uint8_t UI_SendPwrVoxModeToBu(void)
{
    UI_PUReqCmd_t tPwrCmd;

   // if (tUI_PuSetting.tPsMode != PS_VOX_MODE)
    {
        if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
            (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
        {
            tPwrCmd.tDS_CamNum               = tCamViewSel.tCamViewPool[0];
            tPwrCmd.ubCmd[UI_TWC_TYPE]       = UI_SETTING;
            tPwrCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
            tPwrCmd.ubCmd[UI_SETTING_DATA]   = PS_VOX_MODE;
            tPwrCmd.ubCmd_Len                = 3;
            if (UI_SendRequestToBU(osThreadGetId(), &tPwrCmd) != rUI_SUCCESS)
            {
                return rUI_FAIL;
            }
        }
        return rUI_SUCCESS;
    }

    return rUI_FAIL;
}


void UI_TestCmd(uint8_t Value1, uint8_t Value2)
{
    UI_PUReqCmd_t tUI_TestCmd;

    printd(Apk_DebugLvl, "UI_TestCmd (%d, %d), tCamViewPool[0]: %d.\n", Value1, Value2, tCamViewSel.tCamViewPool[0]);
    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
        (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
    {
        tUI_TestCmd.tDS_CamNum              = tCamViewSel.tCamViewPool[0];
        tUI_TestCmd.ubCmd[UI_TWC_TYPE]      = UI_SETTING;
        tUI_TestCmd.ubCmd[UI_SETTING_ITEM]  = UI_TEST_SETTING;
        tUI_TestCmd.ubCmd[UI_SETTING_DATA]  = Value1;
        tUI_TestCmd.ubCmd[UI_SETTING_DATA+1] = Value2;
        tUI_TestCmd.ubCmd_Len               = 4;
        if (UI_SendRequestToBU(osThreadGetId(), &tUI_TestCmd) != rUI_SUCCESS)
        {
            printd(DBG_ErrorLvl, "tUI_TestCmd Fail!\n");
        }
    }
}

void UI_EnableMotor(uint8_t value)
{
    UI_PUReqCmd_t tUI_motorReqCmd;

    printd(Apk_DebugLvl, "UI_EnableMotor value: %d, tCamConnSts: %d.\n", value, tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts);
    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
        (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
    {
        tUI_motorReqCmd.tDS_CamNum              = tCamViewSel.tCamViewPool[0];
        tUI_motorReqCmd.ubCmd[UI_TWC_TYPE]      = UI_SETTING;
        tUI_motorReqCmd.ubCmd[UI_SETTING_ITEM]  = UI_MOTOR_SETTING;
        tUI_motorReqCmd.ubCmd[UI_SETTING_DATA]  = value;
        tUI_motorReqCmd.ubCmd_Len               = 3;
        if (UI_SendRequestToBU(osThreadGetId(), &tUI_motorReqCmd) != rUI_SUCCESS)
        {
            printd(DBG_ErrorLvl, "UI_EnableMotor Fail!\n");
        }

        if (value == 0)
        {
            UI_SetSpeaker(0, 1);
        }
        else
        {
            UI_SetSpeaker(0, 0);
        }
    }
}


void UI_MotorDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;

    printd(Apk_DebugLvl, "UI_MotorDisplay value: 0x%x.\n", value);
    if ((value != MC_UP_DOWN_OFF) && (value != MC_LEFT_RIGHT_OFF))
    {
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_MOVE, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 259;
            tOsdImgInfo.uwYStart = 515;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
    }

    switch (value)
    {
    case MC_UP_ON:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_UP, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 191;
            tOsdImgInfo.uwYStart = 576;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor1State = MC_UP_ON;
        break;

    case MC_UP_TOP:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_UP, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 191;
            tOsdImgInfo.uwYStart = 576;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor1State = MC_UP_TOP;
        break;

    case MC_DOWN_ON:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_DOWN, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 516;
            tOsdImgInfo.uwYStart = 576;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor1State = MC_DOWN_ON;
        break;

    case MC_DOWN_TOP:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_DOWN, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 516;
            tOsdImgInfo.uwYStart = 576;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor1State = MC_DOWN_TOP;
        break;

    case MC_UP_DOWN_OFF:
        if (ubFactoryModeFLag == 0)
        {
            if (ubMotor1State != MC_UP_DOWN_OFF)
            {
                tOsdImgInfo.uwHSize  = 386;
                tOsdImgInfo.uwVSize  = 389;
                tOsdImgInfo.uwXStart = 191;
                tOsdImgInfo.uwYStart = 447;
                OSD_EraserImg2(&tOsdImgInfo);
            }
        }
        ubMotor1State = MC_UP_DOWN_OFF;
        break;

    case MC_LEFT_ON:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_LEFT, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 322;
            tOsdImgInfo.uwYStart = 771;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor0State = MC_LEFT_ON;
        break;

    case MC_LEFT_TOP:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_LEFT, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 322;
            tOsdImgInfo.uwYStart = 771;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor0State = MC_LEFT_TOP;
        break;

    case MC_RIGHT_ON:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_RIGHT, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 322;
            tOsdImgInfo.uwYStart = 447;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor0State = MC_RIGHT_ON;
        break;

    case MC_RIGHT_TOP:
        if (ubFactoryModeFLag == 0)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_RIGHT, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 322;
            tOsdImgInfo.uwYStart = 447;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubMotor0State = MC_RIGHT_TOP;
        break;

    case MC_LEFT_RIGHT_OFF:
        if (ubFactoryModeFLag == 0)
        {
            if (ubMotor0State != MC_LEFT_RIGHT_OFF)
            {
                tOsdImgInfo.uwHSize  = 386;
                tOsdImgInfo.uwVSize  = 389;
                tOsdImgInfo.uwXStart = 191;
                tOsdImgInfo.uwYStart = 447;
                OSD_EraserImg2(&tOsdImgInfo);
            }
        }
        ubMotor0State = MC_LEFT_RIGHT_OFF;
        break;

    default:
        break;
    }
}

void UI_MotorControl(uint8_t Value)
{
//  OSD_IMG_INFO tOsdImgInfo;

    if (tUI_PuSetting.ubDefualtFlag == TRUE)
        return;

    if(ubVolWarnFlag == 1)
	UI_CleanSquealAlert();	

    if (tUI_State != UI_DISPLAY_STATE)
        return;

    if (tUI_SyncAppState != APP_LINK_STATE)
    {
        if (ubMotor0State != MC_LEFT_RIGHT_OFF)
            UI_MotorDisplay(ubMotor0State);

        if (ubMotor1State != MC_UP_DOWN_OFF)
            UI_MotorDisplay(ubMotor1State);

        printd(Apk_DebugLvl, "UI_MotorControl NOT LINK.\n");
        return;
    }

    if (((ubMotor0State>>4)&&(Value>>4)) || (Value == ubMotor0State))
        return;

    if (((ubMotor1State>>4)&&(Value>>4)) || (Value == ubMotor1State))
        return;

    printd(Apk_DebugLvl, "UI_MotorControl Value: (0x%x) #####\n", Value);
    //UI_EnableMotor((Value&0x01)?(Value>>4):0);

    if ((Value == MC_UP_DOWN_OFF) || (Value == MC_LEFT_RIGHT_OFF))
        UI_EnableMotor(0);
    UI_MotorDisplay(Value);
}

void UI_MotorStateCheck(void)
{
    #define MC0_MAX_CNT 88 //left - right /80
    #define MC1_MAX_CNT 40 //up - down

    if (LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
        return;

    if (tUI_PuSetting.ubDefualtFlag == TRUE)
        return;

    //printd(Apk_DebugLvl, "### UP_DOWN: (0x%x, %d), LEFT_RIGHT: (0x%x, %d).\n", ubMotor1State, ubMC1OnCount, ubMotor0State, ubMC0OnCount);
    if (ubMotor1State == MC_UP_ON)
    {
        if (ubMotorDirection != 1)
            ubMC1OnCount = 0;

        ubMC1OnCount++;
        ubMotorDirection = 1;
        if (ubMC1OnCount >= MC1_MAX_CNT)
        {
            UI_MotorDisplay(MC_UP_TOP);
            UI_EnableMotor(0);
        }
        else
        {
            UI_EnableMotor(1);
        }
    }
    else if (ubMotor1State == MC_UP_TOP)
    {
        //ubMC1OnCount = 0;
    }
    else if (ubMotor1State == MC_DOWN_ON)
    {
        if (ubMotorDirection != 2)
            ubMC1OnCount = 0;

        ubMC1OnCount++;
        ubMotorDirection = 2;
        if (ubMC1OnCount >= MC1_MAX_CNT)
        {
            UI_MotorDisplay(MC_DOWN_TOP);
            UI_EnableMotor(0);
        }
        else
        {
            UI_EnableMotor(2);
        }
    }
    else if (ubMotor1State == MC_DOWN_TOP)
    {
        //ubMC1OnCount = 0;
    }
    else if (ubMotor1State == MC_UP_DOWN_OFF)
    {
        //ubMC1OnCount = 0;
    }


    if (ubMotor0State == MC_LEFT_ON)
    {
        if (ubMotorDirection != 3)
            ubMC0OnCount = 0;

        ubMC0OnCount++;
        ubMotorDirection = 3;
        if (ubMC0OnCount >= MC0_MAX_CNT)
        {
            UI_MotorDisplay(MC_LEFT_TOP);
            UI_EnableMotor(0);
        }
        else
        {
            UI_EnableMotor(3);
        }
    }
    else if (ubMotor0State == MC_LEFT_TOP)
    {
        //ubMC0OnCount = 0;
    }
    else if (ubMotor0State == MC_RIGHT_ON)
    {
        if (ubMotorDirection != 4)
            ubMC0OnCount = 0;

        ubMC0OnCount++;
        ubMotorDirection = 4;
        if (ubMC0OnCount >= MC0_MAX_CNT)
        {
            UI_MotorDisplay(MC_RIGHT_TOP);
            UI_EnableMotor(0);
        }
        else
        {
            UI_EnableMotor(4);
        }
    }
    else if (ubMotor0State == MC_RIGHT_TOP)
    {
        //ubMC0OnCount = 0;
    }
    else if (ubMotor0State == MC_LEFT_RIGHT_OFF)
    {
        //ubMC0OnCount = 0;
    }
}

void UI_GetBatLevel(uint16_t checkcount)
{
    uint8_t usbdet, i;
    int16_t percent;
    static uint16_t batmap[] = 
    {
        3500,3598,3665,3788,3984,4350,4500//3500, 3637, 3699, 3802, 3985, 4253, 4500
    };
    //printd(1,"UI_GetBatLevel UI_GetUsbDet %d ubGetBatVoltage %d\n",UI_GetUsbDet(),uwSADC_GetReport(SADC_CH4) * 3030 * 2 / 1024);

    usbdet  = !UI_GetUsbDet();

	//! CHG_ON
    GPIO->GPIO_O13 = 1;
    osDelay(50);
	
    ubGetBatVoltage = uwSADC_GetReport(SADC_CH4) * 3030 * 2 / 1024 ; // new board
//  ubGetBatVoltage = uwSADC_GetReport(SADC_CH4) * 3300 * 2 / 1024 - (usbdet ? 50  : 0); // old board
    printd(1,"UI_GetBatLevel   usbdet %d  ubGetBatVoltage %d\n",usbdet,ubGetBatVoltage);

    GPIO->GPIO_O13 = 0;

    for (i=1; i<sizeof(batmap)/sizeof(batmap[0]); i++) {
        if (ubGetBatVoltage <= batmap[i]) break;
    }
    percent = (i-1) * 20 + 20 * (ubGetBatVoltage - batmap[i-1]) / (batmap[i] - batmap[i-1]);
    percent = percent < 100 ? percent : 100;
    percent = percent > 0   ? percent : 0;
    if (checkcount > 10) {
        if (usbdet) {
            if (ubGetBatPercent < percent) ubGetBatPercent++;
        } else {
            if (ubGetBatPercent > percent) ubGetBatPercent--;
        }
    } else {
        ubGetBatPercent = percent;
    }
    if (ubGetBatPercent > 75) {
        ubBatLvLIdx = BAT_LVL0 + 4;
    } else if (ubGetBatPercent > 50) {
        ubBatLvLIdx = BAT_LVL0 + 3;
    } else if (ubGetBatPercent > 25) {
        ubBatLvLIdx = BAT_LVL0 + 2;
    } else if (ubGetBatPercent > 10) {
        ubBatLvLIdx = BAT_LVL0 + 1;
    } else {
        ubBatLvLIdx = BAT_LVL0;
    }
		printd(1,"iiiiii %d  percent %d ubGetBatPercent %d ubBatLvLIdx %d\n",i,percent,ubGetBatPercent,ubBatLvLIdx);

    if (!usbdet && ubGetBatVoltage < 3500) {
        if (++ubBatLowCount == 10) {
            UI_PowerOff();
        }
    } else {
        ubBatLowCount = 0;
    }
}

uint8_t UI_GetUsbDet(void)
{
    return GPIO->GPIO_I11;
}

uint8_t UI_GetBatChgFull(void)
{
    return GPIO->GPIO_I0;
}

void UI_CheckUsbCharge(void)
{
    static uint8_t ubUsbStatus = 0;

    //printd(Apk_DebugLvl, "### USB: %d, ChgFull: %d, USB_Config: %d.\n", UI_GetUsbDet(), UI_GetBatChgFull(), tUSBD_GetConfigStatus());

#if Current_Test
#else
    if(UI_GetUsbDet() == 0) //USB_DET
    {
        if (tUSBD_GetConfigStatus() == 1) //充电器
        {
            if (GPIO->GPIO_O12 == 1)
            {
                printd(Apk_DebugLvl, "UI_CheckUsbCharge AC Charge!\n");
                GPIO->GPIO_O12 = 0; //大电流快充
            }
        }
        else
        {
            if (GPIO->GPIO_O12 == 0)
            {
                printd(Apk_DebugLvl, "UI_CheckUsbCharge USB Charge!\n");
                GPIO->GPIO_O12 = 1;
            }
        }

        if (UI_GetBatChgFull() == 0) //电池充满
        {
        }
    }
    else
    {
        if (GPIO->GPIO_O12 == 0)
            GPIO->GPIO_O12 = 1;
    }
#endif

    if((UI_GetUsbDet() == 0) && (ubUsbStatus == 1))
    {
        if (ubLogoInitStaus == 1)
        {
            if (LCDBL_STATE == 0)
            {
                //if (ubReStartWakeUpFlag != 1)
             	if(ubWorWakeUpFlag != 1)  //20180905  yxh
                {
                    //LCDBL_ENABLE(UI_ENABLE);
                    //printd(Apk_DebugLvl, "UI_CheckUsbCharge LCDBL_ENABLE TRUE!\n");
                }
            }
        }
    }

    ubUsbStatus = UI_GetUsbDet();
}

void UI_PTNDisplay(uint8_t value)
{
    OSD_IMG_INFO tOsdImgInfo;

    if (value == 0)
        ubDisplayPTN = 0;

    if (ubDisplayPTN == 1)
        return;

    if (tUI_SyncAppState != APP_LINK_STATE)
        return;

    UI_EnableMotor(value);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_CAM_MOVE, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 259;
    tOsdImgInfo.uwYStart = 515;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    switch (value)
    {
    case 0:  //clean
        tOsdImgInfo.uwHSize  = 386;
        tOsdImgInfo.uwVSize  = 389;
        tOsdImgInfo.uwXStart = 191;
        tOsdImgInfo.uwYStart = 447;
        OSD_EraserImg2(&tOsdImgInfo);
        break;

    case 1:   // up
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_UP, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 191;
        tOsdImgInfo.uwYStart = 576;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        ubDisplayPTN = 1;
        break;

    case 2:   //down
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_DOWN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 516;
        tOsdImgInfo.uwYStart = 576;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        ubDisplayPTN = 1;
        break;

    case 3:   //left
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_LEFT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 322;
        tOsdImgInfo.uwYStart = 771;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        ubDisplayPTN = 1;
        break;

    case 4:   //right
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_PTZ_RIGHT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 322;
        tOsdImgInfo.uwYStart = 447;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        ubDisplayPTN = 1;
        break;
    }
}

void UI_DrawTempSubMenuPage_NoSel(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMPUNIT1+(i*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 309+(i*76);
		tOsdImgInfo.uwYStart =729;	
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 334+(tUI_PuSetting.ubTempunitFlag*75);
	tOsdImgInfo.uwYStart =1108;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME_S, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*3);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);		
}
void UI_TempUnitSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubSubMenuItemFlag = 0; // tUI_PuSetting.ubTempunitFlag;
		ubSubMenuItemPreFlag = 1;	
		
		UI_DrawSubMenuPage(TIME_ITEM);
		ubUI_FastStateFlag = TRUE;
		return;
	}

	switch(tArrowKey)
	{
		case UP_ARROW:
			ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
			
			if(0 == ubSubMenuItemFlag) ubSubMenuItemFlag = 1;
			else  ubSubMenuItemFlag--;

			ubTempunitFlag = ubSubMenuItemFlag;	
			UI_TempUnitSubMenuDisplay();
		break;	

		case DOWN_ARROW:
			ubSubMenuItemPreFlag =  ubSubMenuItemFlag;
			
			if(++ubSubMenuItemFlag>=2) ubSubMenuItemFlag = 0;	

			ubTempunitFlag = ubSubMenuItemFlag;	
			UI_TempUnitSubMenuDisplay();
		break;	


		case RIGHT_ARROW:
	
		break;

		case ENTER_ARROW:
			if(tUI_PuSetting.ubTempunitFlag != ubTempunitFlag)
				ubRealTemp = ubTempunitFlag?UI_TempFToC(ubRealTemp):UI_TempCToF(ubRealTemp);
			tUI_PuSetting.ubTempunitFlag  = ubTempunitFlag;
			UI_TempBarDisplay(ubRealTemp);
			UI_TempUnitSubMenuDisplay();
		break;	

		case LEFT_ARROW:
		case EXIT_ARROW:
			tUI_State = UI_MAINMENU_STATE;
			UI_DrawMenuPage();			
		break;
		
	}
}
void UI_TempUnitSubMenuDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t i;

	for(i = 0; i < 2; i++)
	{	
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMPUNIT1+(i*2), 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart= 309+(i*76);
		tOsdImgInfo.uwYStart =729;		
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	
	}	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TEMPUNIT1_S+(ubTempunitFlag*2), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 309+(ubTempunitFlag*76);
	tOsdImgInfo.uwYStart =729;		
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_SUB1_POINT, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 334+(tUI_PuSetting.ubTempunitFlag*75);
	tOsdImgInfo.uwYStart =1108;	
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);	

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_TIME, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart= 48+(96*3);
	tOsdImgInfo.uwYStart =1173;	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);	
}

void UI_VerDisplay(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SN_NUM0 + (ubPuSWVersion%10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 280 ;
	tOsdImgInfo.uwYStart = 393 ;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_DECIMAL, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 280 ;
	tOsdImgInfo.uwYStart = 415;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SN_NUM0+ (ubPuSWVersion/10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 280;
	tOsdImgInfo.uwYStart = 422;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SN_NUM0 + (ubHWVersion%10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 355;
	tOsdImgInfo.uwYStart = 407;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BAR_DECIMAL, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 355;
	tOsdImgInfo.uwYStart = 426;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SN_NUM0+ (ubHWVersion/10), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 355;
	tOsdImgInfo.uwYStart = 436;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	
}

//------------------------------------------------------------------------------
#define CAMS_SET_ITEM_OFFSET	50
/*
void UI_DrawCamsSubMenuPage(void)
{
    UI_CamNum_t tCamSelNum = tCamSelect.tCamNum4CamSetSub, tCamNum;
    OSD_IMG_INFO tOsdImgInfo[26];
    uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
    uint16_t uwDisplayImgIdx[8]  = {OSD2IMG_ANRNOR_ITEM, OSD2IMG_THREEDNRNOR_ITEM, OSD2IMG_vLDSNOR_ITEM,
                                    OSD2IMG_WDRNOR_ITEM, OSD2IMG_DISNOR_ITEM, OSD2IMG_CBRNOR_ITEM,
                                    OSD2IMG_CONDENSENOR_ITEM, OSD2IMG_FLICKERNOR_ITEM};
    uint16_t uwCamModeImgIdx[7]  = {OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamAnrMode,
                                    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCam3DNRMode,
                                    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamvLDCMode,
                                    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamWdrMode,
                                    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamDisMode,
                                    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamCbrMode,
                                    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamCondenseMode};

    uint16_t uwCamImgIdx, uwModeImgIdx, uwXStart[2];
    uint8_t i;

    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (tUI_PuSetting.ubTotalBuNum*2), &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    uwCamOsdImg[tCamSelNum] += UI_ICON_HIGHLIGHT;
    for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
    {
        uwCamImgIdx = uwCamOsdImg[tCamNum] - OSD2IMG_RECCAM1NOR_ICON;
        tOSD_Img2(&tOsdImgInfo[uwCamImgIdx], OSD_QUEUE);
    }
    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_CAMSOPTMARKNOR_ICON, (OSD2IMG_FLICKERHL_ITEM-OSD2IMG_CAMSOPTMARKNOR_ICON+1), &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    uwXStart[0] = tOsdImgInfo[0].uwXStart;
    for (i = 0; i < (CAMSITEM_MAX-2); i++)
    {
        uwDisplayImgIdx[i] -= OSD2IMG_ANRNOR_ITEM;
        tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i] + 10], OSD_QUEUE);
        uwModeImgIdx = uwCamModeImgIdx[i] - OSD2IMG_CAMSOPTMARKNOR_ICON;
        uwXStart[1]  = tOsdImgInfo[uwModeImgIdx].uwXStart;
        tOsdImgInfo[uwModeImgIdx].uwXStart += (i * CAMS_SET_ITEM_OFFSET);
        tOSD_Img2(&tOsdImgInfo[uwModeImgIdx], OSD_QUEUE);
        tOsdImgInfo[uwModeImgIdx].uwXStart  = uwXStart[1];
        tOsdImgInfo[0].uwXStart += ( i * CAMS_SET_ITEM_OFFSET);
        tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
        tOsdImgInfo[0].uwXStart  = uwXStart[0];
    }
    uwDisplayImgIdx[7] -= OSD2IMG_ANRNOR_ITEM;  //! CAMSFLICKER_ITEM - 1
    tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[7] + 10], OSD_QUEUE);
    tOSD_Img2(&tOsdImgInfo[((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1) + 6], OSD_QUEUE);
    tOsdImgInfo[0].uwXStart += (CAMS_SET_ITEM_OFFSET * 7);
    tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_CAMS_SUBMENUICON, 1, &tOsdImgInfo[0]);
    tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
*/
//------------------------------------------------------------------------------
void UI_DrawPairingSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMHL_ITEM, OSD2IMG_DELCAMNOR_ITEM};
    uint8_t i;
    for (i = 0; i < PAIRITEM_MAX; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIR_SUBMENUICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawRecordSubMenuPage(void)
{
    uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
    uint8_t i;
    UI_CamNum_t tCamNum = tCamSelect.tCamNum4RecSub;
    OSD_IMG_INFO tOsdImgInfo[OSD2IMG_SDFHL_ITEM-OSD2IMG_REC_SUBMENUICON+1] = {0};
    uint16_t uwDisplayImgIdx[6]  = {0, (OSD2IMG_RECMODENOR_ITEM - OSD2IMG_REC_SUBMENUICON), ((OSD2IMG_RECLOOPWR_ICON-OSD2IMG_REC_SUBMENUICON)+tUI_CamStatus[tCamNum].tREC_Mode),
                                   (OSD2IMG_RESNOR_ITEM - OSD2IMG_REC_SUBMENUICON), ((OSD2IMG_RESFHDWR_ICON-OSD2IMG_REC_SUBMENUICON)+tUI_CamStatus[tCamNum].tREC_Resolution),
                                   (OSD2IMG_SDFNOR_ITEM - OSD2IMG_REC_SUBMENUICON)};
    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (CAM_4T*2), &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    uwCamOsdImg[tCamNum] += UI_ICON_HIGHLIGHT;
    for (i = 0; i < CAM_4T; i++)
    {
        uwDisplayImgIdx[0] = uwCamOsdImg[i] - OSD2IMG_RECCAM1NOR_ICON;
        tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[0]], OSD_QUEUE);
    }
    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_REC_SUBMENUICON, (OSD2IMG_SDFHL_ITEM-OSD2IMG_REC_SUBMENUICON+1), &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    for (i = 1; i < 6; i++)
        tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i]], OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo[1]);
    tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
    tOsdImgInfo[1].uwXStart += 0x40;
    tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
    tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPhotoSubMenuPage(void)
{
    uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
    OSD_IMG_INFO tOsdImgInfo[OSD2IMG_PRES12MHL_ICON - OSD2IMG_PHOTO_SUBMENUICON + 1] = {0};
    UI_CamNum_t tCamSelNum = tCamSelect.tCamNum4PhotoSub, tCamNum;
    uint16_t uwDisplayImgIdx[5]  = {0, (OSD2IMG_PHOTOMODENOR_ITEM-OSD2IMG_PHOTO_SUBMENUICON), ((OSD2IMG_PHOTOOFFWR_ICON-OSD2IMG_PHOTO_SUBMENUICON)+tUI_CamStatus[tCamSelNum].tPHOTO_Func),
                                   (OSD2IMG_PRESNOR_ITEM-OSD2IMG_PHOTO_SUBMENUICON), ((OSD2IMG_PRES3MWR_ICON-OSD2IMG_PHOTO_SUBMENUICON)+tUI_CamStatus[tCamSelNum].tPHOTO_Resolution)};
    uint8_t i;

    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (CAM_4T*2), &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    uwCamOsdImg[tCamSelNum] += UI_ICON_HIGHLIGHT;
    for (tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
    {
        uwDisplayImgIdx[0] = uwCamOsdImg[tCamNum] - OSD2IMG_RECCAM1NOR_ICON;
        tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[0]], OSD_QUEUE);
    }
    if (tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PHOTO_SUBMENUICON, (OSD2IMG_PRES12MHL_ICON-OSD2IMG_PHOTO_SUBMENUICON+1), &tOsdImgInfo[0]) != OSD_OK)
    {
        printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
        return;
    }
    for (i = 1; i < 6; i++)
        tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i]], OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo[1]);
    tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
    tOsdImgInfo[1].uwXStart += 0x40;
    tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
    tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}

//------------------------------------------------------------------------------
void UI_DrawPlaybackSubMenuPage(void)
{
	//ubUI_RecSubMenuFlag = TRUE;
	//UI_DrawDCIMFolderMenu();
}
//------------------------------------------------------------------------------
void UI_DrawPowerSaveSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PS_SUBMENUICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
/*
void UI_DrawSettingSubMenuPage(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {OSD2IMG_DTHL_ITEM, OSD2IMG_AECNOR_ITEM, OSD2IMG_CCANOR_ITEM,
                                                     OSD2IMG_STORAGENOR_ITEM, OSD2IMG_LANGUAGENOR_ITEM, OSD2IMG_DEFUNOR_ITEM};
    uint8_t i;
    for (i = 0; i < SETTINGITEM_MAX; i++)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubAEC_Mode, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubCCA_Mode, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart += 65;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_STORAGATBU_ICON+tUI_PuSetting.ubSTORAGE_Mode, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart -= 0xE;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOsdImgInfo.uwXStart += (0xE + 0x32);
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_SETTING_SUBMENUICON, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
*/
//------------------------------------------------------------------------------
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem)
{
	static UI_DrawSubMenuFuncPtr_t DrawSubMenuFunc[MENUITEM_MAX] = 
	{
		UI_DrawBrightnessSubMenuPage,
		UI_DrawAutoLcdSubMenuPage,
		UI_DrawAlarmSubMenuPage,
		#if UI_NOTIMEMENU
			UI_TempUnitSubMenuDisplay,
		#else		
			UI_DrawTimeSubMenuPage,
		#endif
		UI_DrawZoomSubMenuPage,
		UI_DrawCamsSubMenuPage,
		UI_DrawSettingSubMenuPage
	};
//	OSD_IMG_INFO tOsdImgInfo;

    if (NULL == DrawSubMenuFunc[MenuItem].pvFuncPtr)
        return;
    /*
    tOSD_GetOsdImgInfor (1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
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
		#if UI_NOTIMEMENU
			UI_TempUnitSubMenuPage,
		#else
			UI_TimeSubMenuPage,
		#endif	
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
    if (SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
        SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_SubSubSubMenu(UI_ArrowKey_t tArrowKey)
{
    switch (tUI_MenuItem.ubItemIdx)
    {
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
/*
void UI_CameraSettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    UI_CamsSubMenuItemList_t tSubMenuItem = (UI_CamsSubMenuItemList_t)tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;
    UI_MenuAct_t tMenuAct;
    OSD_IMG_INFO tOsdImgInfo[8];
    UI_CamNum_t tCamSelNum;
    UI_CamsSetMode_t tCamsModeSts[CAMSITEM_MAX];

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(CAMERAS_ITEM);
		return;
	}
	if(tSubMenuItem == CAMSSELCAM_ITEM)
	{
		if((tArrowKey == LEFT_ARROW) || (tArrowKey == RIGHT_ARROW))
		{
			UI_CamNum_t tPreCamNum = tCamSelect.tCamNum4CamSetSub;
			UI_CamNum_t tNexCamNum = NO_CAM;
			tNexCamNum = UI_ChangeSelectCamNum4UiMenu(&tCamSelect.tCamNum4CamSetSub, &tArrowKey);
			if(tNexCamNum < tUI_PuSetting.ubTotalBuNum)
			{
				uint16_t uwTmpXStart = 0;
				uint8_t i;
				//! Change camera number
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON+(tPreCamNum*2), 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1HL_ICON+(tNexCamNum*2), 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFWR_ICON, 2, &tOsdImgInfo[0]);
				tCamsModeSts[CAMSANR_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamAnrMode;
				tCamsModeSts[CAMS3DNR_ITEM] 	= tUI_CamStatus[tNexCamNum].tCam3DNRMode;
				tCamsModeSts[CAMSvLDS_ITEM] 	= tUI_CamStatus[tNexCamNum].tCamvLDCMode;
				tCamsModeSts[CAMSAEC_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamAecMode;
				tCamsModeSts[CAMSDIS_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamDisMode;
				tCamsModeSts[CAMSCBR_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamCbrMode;
				tCamsModeSts[CAMSCONDENSE_ITEM] = tUI_CamStatus[tNexCamNum].tCamCondenseMode;
				for(i = CAMSANR_ITEM; i <= CAMSCONDENSE_ITEM; i++)
				{
					uwTmpXStart = tOsdImgInfo[tCamsModeSts[i]].uwXStart;
					tOsdImgInfo[tCamsModeSts[i]].uwXStart += (i - CAMSANR_ITEM) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[i]], OSD_QUEUE);
					tOsdImgInfo[tCamsModeSts[i]].uwXStart = uwTmpXStart;
				}
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS50WR_ICON, 2, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[(tUI_CamStatus[tNexCamNum].tCamFlicker == CAMFLICKER_50HZ)?0:1], OSD_UPDATE);
				tCamSelect.tCamNum4CamSetSub = tNexCamNum;
			}
			else if((tArrowKey == LEFT_ARROW) && (TRUE == ubUI_FastStateFlag))
			{
				OSD_IMG_INFO tOsdInfo;

				tOsdInfo.uwHSize  = 600;
				tOsdInfo.uwVSize  = 1180;
				tOsdInfo.uwXStart = 100;
				tOsdInfo.uwYStart = 50;
				OSD_EraserImg2(&tOsdInfo);
				ubUI_FastStateFlag = FALSE;
				UI_DrawDCIMFolderMenu();
			}
			return;
		}
	}
	tCamSelNum = tCamSelect.tCamNum4CamSetSub;
	if((tUI_CamStatus[tCamSelNum].ulCAM_ID == INVALID_ID) ||
	   (tUI_CamStatus[tCamSelNum].tCamConnSts == CAM_OFFLINE))
		return;
	tCamsModeSts[CAMSANR_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamAnrMode;
	tCamsModeSts[CAMS3DNR_ITEM] 	= tUI_CamStatus[tCamSelNum].tCam3DNRMode;
	tCamsModeSts[CAMSvLDS_ITEM] 	= tUI_CamStatus[tCamSelNum].tCamvLDCMode;
	tCamsModeSts[CAMSAEC_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamAecMode;
	tCamsModeSts[CAMSDIS_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamDisMode;
	tCamsModeSts[CAMSCBR_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamCbrMode;
	tCamsModeSts[CAMSCONDENSE_ITEM] = tUI_CamStatus[tCamSelNum].tCamCondenseMode;
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[CAMERAS_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[CAMSITEM_MAX] = {OSD2IMG_ANRNOR_ITEM,  OSD2IMG_ANRNOR_ITEM,      OSD2IMG_THREEDNRNOR_ITEM,
														  OSD2IMG_vLDSNOR_ITEM, OSD2IMG_BUAECNOR_ITEM,    OSD2IMG_DISNOR_ITEM, 
														  OSD2IMG_CBRNOR_ITEM,  OSD2IMG_CONDENSENOR_ITEM, OSD2IMG_FLICKERNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx 	= tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFWR_ICON, 8, &tOsdImgInfo[0]);
			if((ubSubMenuItemIdx != CAMSANR_ITEM) || (ubSubMenuItemPreIdx == CAMS3DNR_ITEM))
			{
				tOsdImgInfo[tCamsModeSts[ubSubMenuItemPreIdx]].uwXStart += (ubSubMenuItemPreIdx - 1) * CAMS_SET_ITEM_OFFSET;
				if(ubSubMenuItemPreIdx == CAMSFLICKER_ITEM)
					tOSD_Img2(&tOsdImgInfo[(4 + ((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1))], OSD_QUEUE);
				else
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemPreIdx]], OSD_QUEUE);
			}
			if(ubSubMenuItemIdx > CAMSSELCAM_ITEM)
			{
				tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx] + 2].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
					tOSD_Img2(&tOsdImgInfo[(6 + ((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1))], OSD_QUEUE);
				else
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx] + 2], OSD_QUEUE);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			if((ubSubMenuItemIdx != CAMSANR_ITEM) || (ubSubMenuItemPreIdx == CAMS3DNR_ITEM))
			{
				tOsdImgInfo[0].uwXStart += (ubSubMenuItemPreIdx - 1) * CAMS_SET_ITEM_OFFSET;
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			}
			if(ubSubMenuItemIdx > CAMSSELCAM_ITEM)
			{
				tOsdImgInfo[1].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
				tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			}
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + ((!ubSubMenuItemIdx)?0:UI_ICON_HIGHLIGHT)));
			break;
		}
		case DRAW_MENUPAGE:
			if(tArrowKey == ENTER_ARROW)
			{
				UI_PUReqCmd_t tCamSetCmd;
				UI_CamsSetMode_t tCamsSetMode;
				UI_CamFlicker_t tCamsFlicker = tUI_CamStatus[tCamSelNum].tCamFlicker;
				uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;
				uint8_t tUI_CamsSetItem[CAMSITEM_MAX] = {UI_ADOANR_SETTING,  UI_ADOANR_SETTING, UI_IMG3DNR_SETTING,
														 UI_IMGvLDC_SETTING, UI_ADOAEC_SETTING, UI_IMGDIS_SETTING, 
														 UI_IMGCBR_SETTING, UI_IMGCONDENSE_SETTING, UI_FLICKER_SETTING};
				UI_CamsSetMode_t *pCamsModeSts[CAMSITEM_MAX] = {&tUI_CamStatus[tCamSelNum].tCamAnrMode, &tUI_CamStatus[tCamSelNum].tCamAnrMode,
																&tUI_CamStatus[tCamSelNum].tCam3DNRMode, &tUI_CamStatus[tCamSelNum].tCamvLDCMode,
																&tUI_CamStatus[tCamSelNum].tCamAecMode, &tUI_CamStatus[tCamSelNum].tCamDisMode,
															    &tUI_CamStatus[tCamSelNum].tCamCbrMode, &tUI_CamStatus[tCamSelNum].tCamCondenseMode};

				if((ubSubMenuItemIdx == CAMSSELCAM_ITEM) || (ubSubMenuItemIdx == CAMSDIS_ITEM) ||				   
				   (ubSubMenuItemIdx == CAMSCBR_ITEM) || (ubSubMenuItemIdx == CAMSCONDENSE_ITEM))
					break;
				tCamSetCmd.tDS_CamNum 				= tCamSelNum;
				tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tCamSetCmd.ubCmd[UI_SETTING_ITEM]   = (ubSubMenuItemIdx == CAMSANR_ITEM)?UI_ADOANR_SETTING:(ubSubMenuItemIdx == CAMSAEC_ITEM)?UI_ADOAEC_SETTING:UI_IMGPROC_SETTING;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tCamsFlicker  						= (CAMFLICKER_50HZ == tUI_CamStatus[tCamSelNum].tCamFlicker)?CAMFLICKER_60HZ:CAMFLICKER_50HZ;
					tCamSetCmd.ubCmd[UI_SETTING_DATA]   = UI_FLICKER_SETTING;
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tCamsFlicker;
				}
				else
				{
					tCamsSetMode  						= (CAMSET_OFF == tCamsModeSts[ubSubMenuItemIdx])?CAMSET_ON:CAMSET_OFF;
					tCamSetCmd.ubCmd[UI_SETTING_DATA]   = ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?tCamsSetMode:tUI_CamsSetItem[ubSubMenuItemIdx];
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?0:tCamsSetMode;
				}
				tCamSetCmd.ubCmd_Len  				= ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?3:4;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS50HL_ICON, 2, &tOsdImgInfo[0]);
					tOSD_Img2(&tOsdImgInfo[tCamsFlicker], OSD_UPDATE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFHL_ICON, 2, &tOsdImgInfo[0]);
					tOsdImgInfo[tCamsSetMode].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsSetMode], OSD_UPDATE);
				}
				if(UI_SendRequestToBU(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
				{
					printd(DBG_ErrorLvl, "Camera Setting fail !\n");
					if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
					{
						tOSD_Img2(&tOsdImgInfo[tCamsFlicker], OSD_UPDATE);
					}
					else
					{
						tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx]].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
						tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx]], OSD_UPDATE);
					}
					return;
				}
				*pCamsModeSts[ubSubMenuItemIdx] 		= tCamsSetMode;
				tUI_CamStatus[tCamSelNum].tCamFlicker   = tCamsFlicker;
				UI_UpdateDevStatusInfo();
			}
			break;
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
#if 0
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    static UI_CamsSubSubMenuItem_t tCamsSubSubMenuItem =
    {
        {CAM1VIEW_ITEM, CAMVIEWITEM_MAX, {QUALVIEW_ITEM, QUALVIEW_ITEM}}
    };
    static uint8_t ubUI_CamSelStsUpdateFlag = FALSE;
    OSD_IMG_INFO tOsdImgInfo;
    UI_MenuAct_t tMenuAct;

    if (FALSE == ubUI_CamSelStsUpdateFlag)
    {
        tCamsSubSubMenuItem.tCameraViewPage.ubItemCount = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?CAMVIEWITEM_MAX:
                                                          (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?DUALVIEW_ITEM:SINGLEVIEW_ITEM;
        tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemIdx  = tCamViewSel.tCamViewPool[0];
        ubUI_CamSelStsUpdateFlag = TRUE;
    }
    tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tCamsSubSubMenuItem.tCameraViewPage);
    switch (tMenuAct)
    {
    case DRAW_HIGHLIGHT_MENUICON:
        {
            uint16_t uwSubSubMenuItemOsdImg[5] = {OSD2IMG_CAM1VIEWNOR_ICON, OSD2IMG_CAM2VIEWNOR_ICON, OSD2IMG_CAM3VIEWNOR_ICON,
                                                  OSD2IMG_CAM4VIEWNOR_ICON, OSD2IMG_CAMQUALVIEWNOR_ICON};
            uint8_t ubSubSubMenuItemPreIdx  = tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemPreIdx;
            uint8_t ubSubSubMenuItemIdx     = tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemIdx;

            if (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
                uwSubSubMenuItemOsdImg[2] = OSD2IMG_CAMDUALVIEWNOR_ICON;
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_CAMVIEWSUS_ICON, 1, &tOsdImgInfo);
            OSD_EraserImg2(&tOsdImgInfo);
            UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
            break;
        }
    case EXECUTE_MENUFUNC:
        {
            UI_CamNum_t tCamNumSel = (UI_CamNum_t)tCamsSubSubMenuItem.tCameraViewPage.tSubMenuInfo.ubItemIdx;

            tCamViewSel.tCamViewPool[0]     = tCamNumSel;
            tCamViewSel.tCamViewType    = (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)?(tCamNumSel == CAM_4T)?QUAL_VIEW:SINGLE_VIEW:
                                          (DISPLAY_2T1R == tUI_PuSetting.ubTotalBuNum)?(tCamNumSel == CAM_2T)?DUAL_VIEW:SINGLE_VIEW:SINGLE_VIEW;
            tUI_PuSetting.ubScanModeEn  = FALSE;
            UI_SwitchCameraSource();
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_CAMVIEWSUS_ICON, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewPool[0]*68);
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            break;
        }
    case EXIT_MENUFUNC:
        {
            OSD_IMG_INFO tOsdImgInfo;

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_CAM1VIEWNOR_ICON, 1, &tOsdImgInfo);
            tOsdImgInfo.uwYStart -= 150;
            tOsdImgInfo.uwHSize   = 335;
            tOsdImgInfo.uwVSize   = 420;
            OSD_EraserImg2(&tOsdImgInfo);
            ubUI_CamSelStsUpdateFlag = FALSE;
            tUI_State = UI_SUBMENU_STATE;
            break;
        }
    default:
        break;
    }
}
#endif
//------------------------------------------------------------------------------
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    UI_MenuAct_t tMenuAct;

    if (tUI_State == UI_MAINMENU_STATE)
    {
        //! Draw Cameras sub menu page
        UI_DrawSubMenuPage(PAIRING_ITEM);
        return;
    }
    tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[PAIRING_ITEM]);
    switch (tMenuAct)
    {
    case DRAW_HIGHLIGHT_MENUICON:
        {
            uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMNOR_ITEM, OSD2IMG_DELCAMNOR_ITEM};
            uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx;
            uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
            UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
            break;
        }
    case DRAW_MENUPAGE:
        {
            //! Draw sub sub menu page
            OSD_IMG_INFO tOsdImgInfo, tCamRdyOsdImgInfo = {0};
            uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
            uint8_t ubBuNum = tUI_PuSetting.ubTotalBuNum, i, ubUI_FirstHL = FALSE;
            uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
            uint16_t uwIconOffset = (ubSubMenuItemIdx == DELCAM_ITEM)?80:0;
            uint16_t uwRDY_XStart = 0, uwOsdImgIdx;

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tCamRdyOsdImgInfo);
            uwRDY_XStart = tCamRdyOsdImgInfo.uwXStart;
            if (ubSubMenuItemIdx == PAIRCAM_ITEM)
            {
                uint16_t uwItemOffset = ubBuNum * 55;
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
                OSD_EraserImg2(&tOsdImgInfo);
                tOsdImgInfo.uwXStart += uwItemOffset;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                for (i = CAM1; i < ubBuNum; i++)
                {
                    if (tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
                    {
                        if (FALSE == ubUI_FirstHL)
                        {
                            uwSubMenuItemOsdImg[i] += UI_ICON_HIGHLIGHT;
                            ubUI_FirstHL = TRUE;
                        }
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
                    }
                    else
                    {
                        uwOsdImgIdx = uwSubMenuItemOsdImg[i]+UI_ICON_READY;
                        if (FALSE == ubUI_FirstHL)
                        {
                            uwOsdImgIdx  = uwSubMenuItemOsdImg[i]+UI_ICON_HIGHLIGHT;
                            ubUI_FirstHL = TRUE;
                        }
                        tCamRdyOsdImgInfo.uwXStart = uwRDY_XStart + (i*55);
                        tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
                    }
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                }
            }
            if (ubSubMenuItemIdx == DELCAM_ITEM)
            {
                for (i = CAM1; i < ubBuNum; i++)
                {
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, (uwSubMenuItemOsdImg[i]+((i == CAM1)?UI_ICON_HIGHLIGHT:(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)?UI_ICON_NORMAL:UI_ICON_READY)), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart += uwIconOffset;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                    if (tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
                    {
                        tCamRdyOsdImgInfo.uwXStart = uwIconOffset + uwRDY_XStart + (i*55);
                        tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);
                    }
                }
            }
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIROPTMASK_ICON, 1, &tOsdImgInfo);
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
    ptSubSubMenuItem->ubFirstItem            = CAM1;
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
    switch (*ptSubMenuItem)
    {
    case PAIRCAM_ITEM:
    case DELCAM_ITEM:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx]+((tUI_CamStatus[ubSubSubMenuItemPreIdx].ulCAM_ID != INVALID_ID)?UI_ICON_READY:UI_ICON_NORMAL)), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart += uwIconOffset;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT, 1, &tOsdImgInfo);
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
    UI_PairSubMenuItemList_t tSubMenuItem = (UI_PairSubMenuItemList_t)tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
    UI_MenuAct_t tMenuAct;
    OSD_IMG_INFO tOsdImgInfo;

    if ((FALSE == ubUI_PairStsUpdateFlag) && (tSubMenuItem == PAIRCAM_ITEM))
        UI_PairingUpdateSubSubMenuItemIndex(&tPairSubSubMenuItem.tPairS[tSubMenuItem], &ubUI_PairStsUpdateFlag);
    tPairSubSubMenuItem.tPairS[tSubMenuItem].ubItemCount = tUI_PuSetting.ubTotalBuNum;
    tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
    switch (tMenuAct)
    {
    case DRAW_HIGHLIGHT_MENUICON:
        UI_PairingDrawSubSbuMenuItem(&tSubMenuItem, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
        break;
    case EXECUTE_MENUFUNC:
        {
            if (tSubMenuItem == PAIRCAM_ITEM)
            {
                if (tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)
                {
                    APP_EventMsg_t tUI_PairMessage = {0};

                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRINGSINGLE_ICON, 1, &tOsdImgInfo);
                    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                    tUI_PairMessage.ubAPP_Event      = APP_PAIRING_START_EVENT;
                    tUI_PairMessage.ubAPP_Message[0] = 2;       //! Message Length
                    tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM1;
                    tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
                    UI_SendMessageToAPP(&tUI_PairMessage);
                    tPairInfo.ubDrawFlag             = TRUE;
                    UI_DisableScanMode();
                    tUI_State = UI_PAIRING_STATE;
                }
                else if (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
                {
                    tPairInfo.tDispLocation = DISP_LEFT;
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRLEFTWINDOW_ICON, 1, &tOsdImgInfo);
                    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                    tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
                    tUI_State = UI_SUBSUBSUBMENU_STATE;
                }
                else if (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
                {
                    tPairInfo.tDispLocation = DISP_UPPER_LEFT;
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);
                    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                    tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
                    tUI_State = UI_SUBSUBSUBMENU_STATE;
                }
            }
            if (tSubMenuItem == DELCAM_ITEM)
            {
                OSD_IMG_INFO tDelOsdInfo;
                uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1HL_ICON, OSD2IMG_PAIRCAM2HL_ICON, OSD2IMG_PAIRCAM3HL_ICON, OSD2IMG_PAIRCAM4HL_ICON};
                uint16_t uwIconOffset = 80;
                UI_CamNum_t tUI_DelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;

                if (INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
                    break;
                tUI_PuSetting.tAdoSrcCamNum  = (tUI_PuSetting.tAdoSrcCamNum == tUI_DelCam)?NO_CAM:tUI_PuSetting.tAdoSrcCamNum;
                tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubMenuItemOsdImg[tUI_DelCam], 1, &tOsdImgInfo);
                tDelOsdInfo.uwHSize  = 50;
                tDelOsdInfo.uwVSize  = 80;
                tDelOsdInfo.uwXStart = tOsdImgInfo.uwXStart + uwIconOffset;
                tDelOsdInfo.uwYStart = tOsdImgInfo.uwYStart - 80;
                OSD_EraserImg2(&tDelOsdInfo);
                tOsdImgInfo.uwXStart += uwIconOffset;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                UI_UnBindBu(tUI_DelCam);
            }
            break;
        }
    case EXIT_MENUFUNC:
        {
            OSD_IMG_INFO tOsdImgInfo;
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON, 1, &tOsdImgInfo);
            if (tSubMenuItem == PAIRCAM_ITEM)
            {
                tOsdImgInfo.uwHSize   = 335;
                tOsdImgInfo.uwVSize   = 450;
                tOsdImgInfo.uwYStart -= 80;
                OSD_EraserImg2(&tOsdImgInfo);
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            }
            if (tSubMenuItem == DELCAM_ITEM)
            {
                tOsdImgInfo.uwHSize   = 335;
                tOsdImgInfo.uwVSize   = 330;
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

    switch (tArrowKey)
    {
    case UP_ARROW:
        if (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
        {
            if ((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_UPPER_RIGHT))
                return;
            tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_LOWER_LEFT)?DISP_UPPER_LEFT:DISP_UPPER_RIGHT;
            break;
        }
        return;
    case DOWN_ARROW:
        if (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
        {
            if ((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
                return;
            tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_LOWER_LEFT:DISP_LOWER_RIGHT;
            break;
        }
        return;
    case LEFT_ARROW:
        if (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
        {
            if ((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_LEFT))
                return;
            tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_RIGHT)?DISP_UPPER_LEFT:DISP_LOWER_LEFT;
        }
        else if (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
        {
            if (tPairInfo.tDispLocation == DISP_LEFT)
                return;
            tPairInfo.tDispLocation = DISP_LEFT;
        }
        break;
    case RIGHT_ARROW:
        if (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)
        {
            if ((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
                return;
            tPairInfo.tDispLocation = (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_UPPER_RIGHT:DISP_LOWER_RIGHT;
        }
        else if (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)
        {
            if (tPairInfo.tDispLocation == DISP_RIGHT)
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

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRINGMULTI_ICON, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart += uwIconXoffset;
            tOsdImgInfo.uwYStart -= uwIconYoffset;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            tUI_PairMessage.ubAPP_Event      = APP_PAIRING_START_EVENT;
            tUI_PairMessage.ubAPP_Message[0] = 2;       //! Message Length
            tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam;
            tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation;
            UI_SendMessageToAPP(&tUI_PairMessage);
            tPairInfo.ubDrawFlag             = TRUE;
            UI_DisableScanMode();
            tUI_State = UI_PAIRING_STATE;
            return;
        }
    case EXIT_ARROW:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);
        OSD_EraserImg2(&tOsdImgInfo);
        tUI_State = UI_SUBSUBMENU_STATE;
        return;
    default:
        return;
    }
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON+tPairInfo.tDispLocation, 1, &tOsdImgInfo);
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define PAIRING_ICON_NUM    2
void UI_DrawPairingStatusIcon(void)
{
    static OSD_IMG_INFO tOsdImgInfo[PAIRING_ICON_NUM] = {0};
    uint16_t uwPairIconOsdImgIdx;
    uint16_t uwIconXoffset = 0;
    uint16_t uwIconYoffset = 0;
    uint16_t uwIconXStart  = 0;
    uint16_t uwIconYStart  = 0;
    static uint8_t ubIconOffset = 0;

    if (FALSE == tPairInfo.ubDrawFlag)
        return;
    if (FALSE == tUI_PuSetting.IconSts.ubRdPairIconFlag)
    {
        uwPairIconOsdImgIdx = (tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)?OSD2IMG_PAIRINGSINGLE_ICON:
                              (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?OSD2IMG_PAIRINGMULTI_ICON:
                              (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?OSD2IMG_PAIRINGMULTI_ICON:OSD2IMG_PAIRINGSINGLE_ICON;
        if (tOSD_GetOsdImgInfor (1, OSD_IMG2, uwPairIconOsdImgIdx, PAIRING_ICON_NUM, &tOsdImgInfo[0]) != OSD_OK)
        {
            printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
            return;
        }
        tUI_PuSetting.IconSts.ubRdPairIconFlag  = TRUE;
        ubIconOffset                            = 1;
    }
    uwIconXoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?88:0:
                    (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?40:0;
    uwIconYoffset = (tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?130:0:
                    (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R)?(tPairInfo.tDispLocation == DISP_RIGHT)?130:0:0;
    uwIconXStart = tOsdImgInfo[ubIconOffset].uwXStart;
    uwIconYStart = tOsdImgInfo[ubIconOffset].uwYStart;
    tOsdImgInfo[ubIconOffset].uwXStart += uwIconXoffset;
    tOsdImgInfo[ubIconOffset].uwYStart -= uwIconYoffset;
    tOSD_Img2(&tOsdImgInfo[ubIconOffset], OSD_UPDATE);
    tOsdImgInfo[ubIconOffset].uwXStart = uwIconXStart;
    tOsdImgInfo[ubIconOffset].uwYStart = uwIconYStart;
    ubIconOffset = (++ubIconOffset == PAIRING_ICON_NUM)?0:ubIconOffset;
}
//------------------------------------------------------------------------------
void UI_ReportPairingResult(UI_Result_t tResult)
{
    OSD_IMG_INFO tOsdImgInfo;
    APP_EventMsg_t tUI_UnindBuMsg = {0};
    UI_CamNum_t tCamNum;
    uint16_t uwPairIconOsdImgIdx = 0;
    uint16_t uwIconXStart  = 0;

    tPairInfo.ubDrawFlag = FALSE;
    uwPairIconOsdImgIdx = (tUI_PuSetting.ubTotalBuNum == DISPLAY_1T1R)?OSD2IMG_PAIRINGSINGLE_ICON:
                          ((tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R) || (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R))?OSD2IMG_PAIRLUWINDOW_ICON:OSD2IMG_PAIRINGSINGLE_ICON;
    uwIconXStart        = ((tUI_PuSetting.ubTotalBuNum == DISPLAY_4T1R) || (tUI_PuSetting.ubTotalBuNum == DISPLAY_2T1R))?0:(tPairInfo.tPairSelCam*55);
    tOSD_GetOsdImgInfor (1, OSD_IMG2, uwPairIconOsdImgIdx, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart += uwIconXStart;
    OSD_EraserImg2(&tOsdImgInfo);
    switch (tResult)
    {
    case rUI_SUCCESS:
        tUI_PuSetting.ubPairedBuNum += (tUI_PuSetting.ubPairedBuNum >= tUI_PuSetting.ubTotalBuNum)?0:1;
        tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
        tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
        for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
        {
            if (tCamNum == tPairInfo.tPairSelCam)
                continue;
            if ((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
               (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
            {
                tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
                tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
                tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
                UI_SendMessageToAPP(&tUI_UnindBuMsg);
                tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
                tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
                tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart += (tCamNum*55);
                OSD_EraserImg2(&tOsdImgInfo);
                tOSD_GetOsdImgInfor (1, OSD_IMG2, ((OSD2IMG_PAIRCAM1NOR_ICON+(tCamNum*3))), 1, &tOsdImgInfo);
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
                UI_ResetDevSetting(tCamNum);
            }
        }
        if ((DISPLAY_1T1R == tUI_PuSetting.ubTotalBuNum) ||
           (NO_CAM == tUI_PuSetting.tAdoSrcCamNum) ||
           (INVALID_ID == tUI_CamStatus[tUI_PuSetting.tAdoSrcCamNum].ulCAM_ID))
            tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
        tOSD_GetOsdImgInfor (1, OSD_IMG2, ((OSD2IMG_PAIRCAM1NOR_ICON+(tPairInfo.tPairSelCam*3))+UI_ICON_READY), 1, &tOsdImgInfo);
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart += (tPairInfo.tPairSelCam*55);
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        UI_PairingSubSubMenuPage(DOWN_ARROW);
        UI_ResetDevSetting(tPairInfo.tPairSelCam);
//      UI_UpdateDevStatusInfo();
        break;
    case rUI_FAIL:
        break;
    default:
        break;
    }
    tUI_State = UI_SUBSUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_SetLdDispStatus(UI_OsdLdDispSts_t tDispSts)
{
	osMutexWait(osUI_OsdLdStsUpdMutex, osWaitForever);
	tUI_OsdLdDispSts = tDispSts;
	osMutexRelease(osUI_OsdLdStsUpdMutex);
}
//------------------------------------------------------------------------------
UI_OsdLdDispSts_t UI_GetLdDispStatus(void)
{
	UI_OsdLdDispSts_t tDispSts;

	osMutexWait(osUI_OsdLdStsUpdMutex, osWaitForever);
	tDispSts = tUI_OsdLdDispSts;
	osMutexRelease(osUI_OsdLdStsUpdMutex);
	return tDispSts;
}
//------------------------------------------------------------------------------
static void UI_OsdLoadingDisplayThread(void const *argument)
{
	uint16_t uwUI_OsdLdDispSte;
	uint8_t ubUI_RdLdOsdImgFlag = FALSE, ubLdOsdIdx = 0;
	OSD_IMG_INFO tLdOsdImg[17];

	while(1)
	{
		osMessageGet(osUI_OsdLdDispQueue, &uwUI_OsdLdDispSte, osWaitForever);		
		if(FALSE == ubUI_RdLdOsdImgFlag)
		{
			if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_LOADING1_P1_ICON, 16, &tLdOsdImg) != OSD_OK)
			{
				printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
				continue;
			}
			ubUI_RdLdOsdImgFlag = TRUE;
		}
		switch(uwUI_OsdLdDispSte)
		{
			case UI_SEARCH_DCIMFOLDER:
			case UI_SEARCH_RECFILES:
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					tOSD_Img2(&tLdOsdImg[ubLdOsdIdx++], OSD_UPDATE);
					ubLdOsdIdx = (ubLdOsdIdx >= 16)?0:ubLdOsdIdx;
					osDelay(250);
				}
				tLdOsdImg[16].uwHSize  = 200;
				tLdOsdImg[16].uwVSize  = 250;
				tLdOsdImg[16].uwXStart = 275;
				tLdOsdImg[16].uwYStart = 550;
				OSD_EraserImg2(&tLdOsdImg[16]);
				break;
			default:
				break;
		}
	}
}
//------------------------------------------------------------------------------
void UI_SetRecImgColor(UI_RecImgColor_t tColorNum)
{
	static UI_RecImgColor_t tUI_RecImgColor = UI_RECIMG_DEFU;	
	uint8_t i;
	
	if(NULL == pUI_RecOsdImgDB)
	{
		pUI_RecOsdImgDB = osMalloc(sizeof(UI_RecOsdImgDb_t));
		memset(pUI_RecOsdImgDB, 0, sizeof(UI_RecOsdImgDb_t));
	}
	if(tUI_RecImgColor == tColorNum)
		return;
	for(i = 0; i < 10; i++)
		pUI_RecOsdImgDB->uwUI_RecNumArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_NUM0 + (tColorNum * 10)) + i):
																				(OSD2IMG_FILE_C6_NUM0 + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	tUI_RecOsdImgInfo.pNumImgIdxArray = pUI_RecOsdImgDB->uwUI_RecNumArray;
	for(i = 0; i < 26; i++)
	{
		pUI_RecOsdImgDB->uwUI_RecUpperLetterArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_UPA + (tColorNum * 26)) + i):
																						(OSD2IMG_FILE_C6_UPA + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
		pUI_RecOsdImgDB->uwUI_RecLowerLetterArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_LWA + (tColorNum * 26)) + i):
																						(OSD2IMG_FILE_C6_LWA + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	}
	tUI_RecOsdImgInfo.pUpperLetterImgIdxArray = pUI_RecOsdImgDB->uwUI_RecUpperLetterArray;
	tUI_RecOsdImgInfo.pLowerLetterImgIdxArray = pUI_RecOsdImgDB->uwUI_RecLowerLetterArray;
	for(i = 0; i < 4; i++)
		pUI_RecOsdImgDB->uwUI_RecSymbolArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_COLON + (tColorNum * 4)) + i):
																				   (OSD2IMG_FILE_C6_COLON + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	tUI_RecOsdImgInfo.pSymbolImgIdxArray = pUI_RecOsdImgDB->uwUI_RecSymbolArray;
	tUI_RecImgColor = tColorNum;
}	
//------------------------------------------------------------------------------
void UI_SearchDCIMFolder(void)
{
	uint16_t uwLdState = UI_SEARCH_DCIMFOLDER;

	if(NULL == pUI_RecFoldersInfo)
	{
		pUI_RecFoldersInfo = osMalloc(sizeof(UI_RecFoldersInfo_t));
		memset(pUI_RecFoldersInfo, 0, sizeof(UI_RecFoldersInfo_t));
	}
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, osWaitForever);
	osDelay(200);
	pUI_RecFoldersInfo->ubTotalRecFolderNum = ulFS_GetSortingFolders(&pUI_RecFoldersInfo->tRecFolderInfo[0]);
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(300);
}
//------------------------------------------------------------------------------
void UI_DrawDCIMFolderMenu(void)
{	
	OSD_IMG_INFO tRecBgOsdImgInfo, tRecOsdImgInfo[3];
	uint16_t uwRecXStart[2] = {0, 0}, uwRecYStart[2] = {0, 0};

	OSD_Weight(OSD_WEIGHT_8DIV8);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecBgOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 3, &tRecOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tRecBgOsdImgInfo.uwXStart = 0;
	tRecBgOsdImgInfo.uwYStart = 0;
	tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
	tOSD_Img2(&tRecOsdImgInfo[0], OSD_UPDATE);
	UI_SearchDCIMFolder();	
	uwRecXStart[0] = tRecOsdImgInfo[1].uwXStart;
	uwRecYStart[0] = tRecOsdImgInfo[1].uwYStart;
	uwRecXStart[1] = tRecOsdImgInfo[2].uwXStart;
	uwRecYStart[1] = tRecOsdImgInfo[2].uwYStart;
	if((pUI_RecFoldersInfo) && (pUI_RecFoldersInfo->ubTotalRecFolderNum))
	{
		uint8_t ubOpenOsdIdx = 0, ubIdx;

		for(ubIdx = 0; ubIdx < ((pUI_RecFoldersInfo->ubTotalRecFolderNum < REC_FOLDER_LIST_MAXNUM)?pUI_RecFoldersInfo->ubTotalRecFolderNum:REC_FOLDER_LIST_MAXNUM); ubIdx++)
		{
			UI_SetRecImgColor((pUI_RecFoldersInfo->ubRecFolderSelIdx == ubIdx)?UI_RECIMG_COLOR6:UI_RECIMG_COLOR5);
			ubOpenOsdIdx = (((pUI_RecFoldersInfo->ubRecFolderSelIdx == ubIdx)?1:0) + 1);
			tRecOsdImgInfo[ubOpenOsdIdx].uwYStart -= ((ubIdx % 5) * 185);
			tRecOsdImgInfo[ubOpenOsdIdx].uwXStart += ((ubIdx / 5) * 265);
			tOSD_Img2(&tRecOsdImgInfo[ubOpenOsdIdx], OSD_QUEUE);
			OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubIdx % 5) * 185)), (263 + ((ubIdx / 5) * 265)), tUI_RecOsdImgInfo, OSD_UPDATE, pUI_RecFoldersInfo->tRecFolderInfo[ubIdx].cFolderName);
			tRecOsdImgInfo[ubOpenOsdIdx].uwXStart = uwRecXStart[ubOpenOsdIdx - 1];
			tRecOsdImgInfo[ubOpenOsdIdx].uwYStart = uwRecYStart[ubOpenOsdIdx - 1];
		}
	}
	tUI_State = (TRUE == ubUI_RecSubMenuFlag)?UI_SUBMENU_STATE:UI_RECFOLDER_SEL_STATE;
}
//------------------------------------------------------------------------------
void UI_DCIMFolderSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFolderSelOsdImgInfo[2], tOsdImgInfo;
	uint8_t ubUI_PrevRecFolderIdx = 0;

	if((!pUI_RecFoldersInfo->ubTotalRecFolderNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tFolderSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(!(pUI_RecFoldersInfo->ubRecFolderSelIdx % 5))
				return;
			ubUI_PrevRecFolderIdx = pUI_RecFoldersInfo->ubRecFolderSelIdx--;
			break;
		case RIGHT_ARROW:
			if(((pUI_RecFoldersInfo->ubRecFolderSelIdx % 5) == 4) ||
			   ((pUI_RecFoldersInfo->ubRecFolderSelIdx + 1) >= pUI_RecFoldersInfo->ubTotalRecFolderNum))
				return;
			ubUI_PrevRecFolderIdx = pUI_RecFoldersInfo->ubRecFolderSelIdx++;
			break;
		case UP_ARROW:
			if(pUI_RecFoldersInfo->ubRecFolderSelIdx < 5)
				return;
			ubUI_PrevRecFolderIdx = pUI_RecFoldersInfo->ubRecFolderSelIdx;
			pUI_RecFoldersInfo->ubRecFolderSelIdx -= 5;
			break;
		case DOWN_ARROW:
			if((pUI_RecFoldersInfo->ubRecFolderSelIdx + 5) >= pUI_RecFoldersInfo->ubTotalRecFolderNum)
				return;
			ubUI_PrevRecFolderIdx = pUI_RecFoldersInfo->ubRecFolderSelIdx;
			pUI_RecFoldersInfo->ubRecFolderSelIdx += 5;
			break;
		case ENTER_ARROW:
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
			UI_DrawRecordFileMenu();
			return;
		case EXIT_ARROW:
			pUI_RecFoldersInfo->ubRecFolderSelIdx = 0;
			if(APP_LOSTLINK_STATE == tUI_SyncAppState)
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				UI_ClearBuConnectStatusFlag();
				tUI_State = UI_DISPLAY_STATE;
				return;
			}
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 1180;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
			UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			return;
		default:
			return;
	}
	UI_SetRecImgColor(UI_RECIMG_COLOR5);
	tFolderSelOsdImgInfo[0].uwYStart -= ((ubUI_PrevRecFolderIdx % 5) * 185);
	tFolderSelOsdImgInfo[0].uwXStart += ((ubUI_PrevRecFolderIdx / 5) * 265);
	tOSD_Img2(&tFolderSelOsdImgInfo[0], OSD_QUEUE);
	OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_PrevRecFolderIdx % 5) * 185)), (263 + ((ubUI_PrevRecFolderIdx / 5) * 265)),
	                tUI_RecOsdImgInfo, OSD_QUEUE, pUI_RecFoldersInfo->tRecFolderInfo[ubUI_PrevRecFolderIdx].cFolderName);
	UI_SetRecImgColor(UI_RECIMG_COLOR6);
	tFolderSelOsdImgInfo[1].uwYStart -= ((pUI_RecFoldersInfo->ubRecFolderSelIdx % 5) * 185);
	tFolderSelOsdImgInfo[1].uwXStart += ((pUI_RecFoldersInfo->ubRecFolderSelIdx / 5) * 265);
	tOSD_Img2(&tFolderSelOsdImgInfo[1], OSD_QUEUE);
	OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((pUI_RecFoldersInfo->ubRecFolderSelIdx % 5) * 185)), (263 + ((pUI_RecFoldersInfo->ubRecFolderSelIdx / 5) * 265)),
	                tUI_RecOsdImgInfo, OSD_UPDATE, pUI_RecFoldersInfo->tRecFolderInfo[pUI_RecFoldersInfo->ubRecFolderSelIdx].cFolderName);
}
//------------------------------------------------------------------------------
void UI_PlayRecordFile(uint16_t uwRecFileIndex)
{
	if(!memcmp(pUI_RecFilesInfo->tRecFilesInfo[uwRecFileIndex].cFileExtName, "JPG", 3))
	{
		KNL_RecordAct_t tUI_PlayAct;

		tUI_PlayAct.tRecordFunc  	= KNL_PHOTO_PLAY;
		tUI_PlayAct.pRecordStsNtyCb = UI_PhotoPlayFinish;
		memset(tUI_PlayAct.tPhotoPlayInfo.chFolderName, 0, sizeof(tUI_PlayAct.tPhotoPlayInfo.chFolderName));
		memset(tUI_PlayAct.tPhotoPlayInfo.chFileName, 0, sizeof(tUI_PlayAct.tPhotoPlayInfo.chFileName));
		tUI_PlayAct.tPhotoPlayInfo.ubFolderNameLen = pUI_RecFoldersInfo->tRecFolderInfo[pUI_RecFoldersInfo->ubRecFolderSelIdx].ubFolderNameLen;
		memcpy(tUI_PlayAct.tPhotoPlayInfo.chFolderName, pUI_RecFoldersInfo->tRecFolderInfo[pUI_RecFoldersInfo->ubRecFolderSelIdx].cFolderName, tUI_PlayAct.tPhotoPlayInfo.ubFolderNameLen);
		tUI_PlayAct.tPhotoPlayInfo.ubFileNameLen = pUI_RecFilesInfo->tRecFilesInfo[uwRecFileIndex].ubFileNameLen;
		memcpy(tUI_PlayAct.tPhotoPlayInfo.chFileName, pUI_RecFilesInfo->tRecFilesInfo[uwRecFileIndex].cFileName, tUI_PlayAct.tPhotoPlayInfo.ubFileNameLen);		
		KNL_ExecRecordFunc(tUI_PlayAct);
		tUI_State = UI_PHOTOPLAYLIST_STATE;
	}
	else if(!memcmp(pUI_RecFilesInfo->tRecFilesInfo[uwRecFileIndex].cFileExtName, "MP4", 3))
	{
		OSD_IMG_INFO tRecPlayOsdImgInfo[10];
		uint8_t ubImgIdx = 0;

		if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_RECPLAYLISTBG, 1, &tRecPlayOsdImgInfo[0]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSENOR_ICON, 8, &tRecPlayOsdImgInfo[1]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		tRecPlayOsdImgInfo[9].uwHSize  = uwOSD_GetHSize();
		tRecPlayOsdImgInfo[9].uwVSize  = uwOSD_GetVSize();
		tRecPlayOsdImgInfo[9].uwXStart = 0;
		tRecPlayOsdImgInfo[9].uwYStart = 0;
		OSD_EraserImg1(&tRecPlayOsdImgInfo[9]);
		OSD_Weight(OSD_WEIGHT_6DIV8);
		tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
		for(ubImgIdx = 3; ubImgIdx < 8; ubImgIdx+=2)
			tOSD_Img2(&tRecPlayOsdImgInfo[ubImgIdx], OSD_QUEUE);
		tOSD_Img2(&tRecPlayOsdImgInfo[2], OSD_UPDATE);
		tUI_State = UI_RECPLAYLIST_STATE;	
	}
}
//------------------------------------------------------------------------------
void UI_PhotoPlayListSelection(UI_ArrowKey_t tArrowKey)
{
	KNL_RecordAct_t tUI_PlayAct;
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];

	switch(tArrowKey)
	{
		case EXIT_ARROW:
			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
			KNL_ExecRecordFunc(tUI_PlayAct);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_QUEUE);
			UI_DrawRecordFileMenu();
			if(TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag)
				OSD_LogoJpeg(OSDLOGO_LOSTLINK);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RecordPlayListSelection(UI_ArrowKey_t tArrowKey)
{
	static UI_RecPlayListItem_t tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
	static UI_RecPlayStatus_t tUI_RecPlaySts = UI_RECFILE_PLAY;
	static uint16_t uwRecPlayListImgIdx[UI_RECPLAYLISTITEM_MAX] = {OSD2IMG_REC_SKIPBACKWARDNOR_ICON, OSD2IMG_REC_PAUSENOR_ICON,
											                       OSD2IMG_REC_SKIPFORWARDNOR_ICON};
	UI_RecPlayListItem_t tPrevRecListItem = UI_RECPLAYPAUSE_ITEM;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if((UI_RECSKIPBKFWD_ITEM == tUI_RecPlayListItem)  || (UI_RECFILE_PAUSE == tUI_RecPlaySts))
				return;
			tPrevRecListItem = tUI_RecPlayListItem;
			tUI_RecPlayListItem--;
			break;
		case RIGHT_ARROW:
			if(((tUI_RecPlayListItem + 1) > UI_RECSKIPFRFWD_ITEM) || (UI_RECFILE_PAUSE == tUI_RecPlaySts))
				return;
			tPrevRecListItem = tUI_RecPlayListItem;
			tUI_RecPlayListItem++;
			break;
		case ENTER_ARROW:
			if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem)
			{
				OSD_IMG_INFO tRecPlayOsdImgInfo;

				if(UI_RECFILE_PLAY == tUI_RecPlaySts)
				{
					//! Play pause
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo);
					tOSD_Img2(&tRecPlayOsdImgInfo, OSD_UPDATE);
					tUI_RecPlaySts = UI_RECFILE_PAUSE;
				}
				else
				{
					//! Play
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSEHL_ICON, 1, &tRecPlayOsdImgInfo);
					tOSD_Img2(&tRecPlayOsdImgInfo, OSD_UPDATE);
					tUI_RecPlaySts = UI_RECFILE_PLAY;
				}
			}
			return;
		case EXIT_ARROW:
		{
			OSD_IMG_INFO tRecPlayOsdImgInfo[2];

			tRecPlayOsdImgInfo[0].uwHSize  = 100;
			tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
			tRecPlayOsdImgInfo[0].uwXStart = 620;
			tRecPlayOsdImgInfo[0].uwYStart = 0;
			OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);
			tUI_RecPlaySts      = UI_RECFILE_PLAY;
			tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
			uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_QUEUE);
			UI_DrawRecordFileMenu();
			return;
		}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwRecPlayListImgIdx[tPrevRecListItem], (uwRecPlayListImgIdx[tUI_RecPlayListItem] + UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_SearchRecordFile(uint8_t ubRecFolderIndex)
{
	uint16_t uwLdState = UI_SEARCH_RECFILES;

	ubRecFolderIndex = ubRecFolderIndex;
	if(NULL == pUI_RecFilesInfo)
	{
		pUI_RecFilesInfo = osMalloc(sizeof(UI_RecFilesInfo_t));
		memset(pUI_RecFilesInfo, 0, sizeof(UI_RecFilesInfo_t));
		pUI_RecFilesInfo->uwRecFileSelIdx = 0;
	}
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, osWaitForever);
	osDelay(200);
	pUI_RecFilesInfo->uwTotalRecFileNum = uwFS_GetSortingFiles(ubRecFolderIndex, SORT_BY_NAME_DESCENDING, &pUI_RecFilesInfo->tRecFilesInfo[0]);
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(300);
}
//------------------------------------------------------------------------------
void UI_ListRecFileInfo(uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	UI_RecImgColor_t tRecCamColor[] = {[CAM1] = UI_RECIMG_COLOR1,
									   [CAM2] = UI_RECIMG_COLOR2,
									   [CAM3] = UI_RECIMG_COLOR3,
									   [CAM4] = UI_RECIMG_COLOR4,}, tRecColor;
	UI_CamNum_t tRecSelCamNum;
	KNL_ROLE tRecRoleNum;
	OSD_IMG_INFO tAsteriskOsdImgInfo;
	uint16_t uwIdx;
	uint8_t ubRecGrpLNum, ubRecGrpRNum, ubGrpFlag = FALSE;
	int iGrpRIdx = 1;
	char cRecFileName[32], cRecFileCreated[16];

	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
		ubRecGrpLNum = pUI_RecFilesInfo->tRecFilesInfo[uwIdx].uwGroupIndex;
		iGrpRIdx     = uwIdx + ((uwIdx)?-1:1);
		ubRecGrpRNum = pUI_RecFilesInfo->tRecFilesInfo[iGrpRIdx].uwGroupIndex;
		ubGrpFlag	 = (ubRecGrpLNum == ubRecGrpRNum)?TRUE:FALSE;
		if((uwIdx) && (FALSE == ubGrpFlag))
		{
			ubRecGrpRNum = pUI_RecFilesInfo->tRecFilesInfo[uwIdx + 1].uwGroupIndex;
			ubGrpFlag	 = (ubRecGrpLNum == ubRecGrpRNum)?TRUE:FALSE;
		}
		tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)pUI_RecFilesInfo->tRecFilesInfo[uwIdx].SrcNum);
		if(KNL_NONE == tRecRoleNum)
		{
			printd(DBG_ErrorLvl, "Get kernel role info Err!\n");
			tRecRoleNum = KNL_STA1;
		}
		APP_KNLRoleMap2CamNum(tRecRoleNum, tRecSelCamNum);
		tRecColor = (TRUE == ubGrpFlag)?UI_RECIMG_COLOR5:tRecCamColor[tRecSelCamNum];
		UI_SetRecImgColor(tRecColor);
		memset(cRecFileName, 0, sizeof(cRecFileName));
		snprintf(cRecFileName, sizeof(cRecFileName), "%s.%s", pUI_RecFilesInfo->tRecFilesInfo[uwIdx].cFileName, pUI_RecFilesInfo->tRecFilesInfo[uwIdx].cFileExtName);
		cRecFileName[(pUI_RecFilesInfo->tRecFilesInfo[uwIdx].ubFileNameLen + 4)] = 0;
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 325, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, OSD_QUEUE, cRecFileName);
		if(TRUE == ubGrpFlag)
		{
			char cListGrp[8];

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_C5_ASTERISK, 1, &tAsteriskOsdImgInfo);
			tAsteriskOsdImgInfo.uwYStart = 280;	//! uwLCD_GetLcdVoSize() - 980 - 20
			tAsteriskOsdImgInfo.uwXStart = 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5);
			tOSD_Img2(&tAsteriskOsdImgInfo, OSD_QUEUE);
			snprintf(cListGrp, sizeof(cListGrp), "Grp%d", pUI_RecFilesInfo->tRecFilesInfo[uwIdx].uwGroupIndex);
			OSD_ImagePrintf(OSD_IMG_ROTATION_90, 1000, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, OSD_QUEUE, cListGrp);
		}
		memset(cRecFileCreated, 0, sizeof(cRecFileCreated));
		snprintf(cRecFileCreated, sizeof(cRecFileCreated), "%02d-%02d-%02d %02d:%02d", pUI_RecFilesInfo->tRecFilesInfo[uwIdx].uwCreateYear, pUI_RecFilesInfo->tRecFilesInfo[uwIdx].ubCreateMonth, pUI_RecFilesInfo->tRecFilesInfo[uwIdx].ubCreateDay,
																					   pUI_RecFilesInfo->tRecFilesInfo[uwIdx].uwCreateHour, pUI_RecFilesInfo->tRecFilesInfo[uwIdx].ubCreateMin);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 643, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, ((uwIdx + 1) == uwEndFileIdx)?tUpdateMode:OSD_QUEUE, cRecFileCreated);
	}
}
//------------------------------------------------------------------------------
void UI_DrawRecordFileMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[9];
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
	uint8_t ubIdx;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_CAM1, 9, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	UI_SearchRecordFile(pUI_RecFoldersInfo->ubRecFolderSelIdx);
	for(ubIdx = 0; ubIdx < 8; ubIdx++)
		tOSD_Img2(&tOsdImgInfo[ubIdx], ((!pUI_RecFilesInfo->uwTotalRecFileNum) && (ubIdx == 7))?OSD_UPDATE:OSD_QUEUE);
	if(pUI_RecFilesInfo->uwTotalRecFileNum)
	{
		uwFileStartIdx = (pUI_RecFilesInfo->uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
		uwFileEndIdx   = pUI_RecFilesInfo->uwTotalRecFileNum - uwFileStartIdx;
		uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecFilesInfo->uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		tOsdImgInfo[8].uwXStart += ((pUI_RecFilesInfo->uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
		UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_UPDATE);
	}
	tUI_State = UI_RECFILES_SEL_STATE;
}
//------------------------------------------------------------------------------
void UI_RecordFileSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFileSelOsdImgInfo[2];
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubUI_RecFileIdx, ubUI_PrevRecFileIdx = 0;

	if((!pUI_RecFilesInfo->uwTotalRecFileNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILESELECT, 2, &tFileSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	switch(tArrowKey)
	{
		case RIGHT_ARROW:
			return;
		case UP_ARROW:
			if(!pUI_RecFilesInfo->uwRecFileSelIdx)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecFilesInfo->uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (--pUI_RecFilesInfo->uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case DOWN_ARROW:
			if((pUI_RecFilesInfo->uwRecFileSelIdx + 1) >= pUI_RecFilesInfo->uwTotalRecFileNum)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecFilesInfo->uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (++pUI_RecFilesInfo->uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case ENTER_ARROW:
			UI_PlayRecordFile(pUI_RecFilesInfo->uwRecFileSelIdx);
			return;
		case EXIT_ARROW:
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
			pUI_RecFilesInfo->uwRecFileSelIdx = 0;
			UI_DrawDCIMFolderMenu();
			return;
		default:
			return;
	}
	if((((!(pUI_RecFilesInfo->uwRecFileSelIdx % REC_FILE_LIST_MAXNUM)) && (pUI_RecFilesInfo->uwRecFileSelIdx) && (ubUI_PrevRecFileIdx == REC_FILE_LIST_MAXNUM - 1))) ||	//! DOWN_ARROW
	   ((ubUI_PrevRecFileIdx == 0) && (ubUI_RecFileIdx == REC_FILE_LIST_MAXNUM - 1)))	//! UP_ARROW
	{
		uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;

		tOsdImgInfo.uwHSize  = 405;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 195;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
		if(DOWN_ARROW == tArrowKey)
		{
			uwFileStartIdx = pUI_RecFilesInfo->uwRecFileSelIdx;
			uwFileEndIdx   = pUI_RecFilesInfo->uwTotalRecFileNum - pUI_RecFilesInfo->uwRecFileSelIdx;
			uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecFilesInfo->uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		}
		else
		{
			uwFileStartIdx = (pUI_RecFilesInfo->uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
			uwFileEndIdx   = pUI_RecFilesInfo->uwRecFileSelIdx + 1;
		}
		UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
	}
	tFileSelOsdImgInfo[0].uwXStart += (ubUI_RecFileIdx * 40);
	tFileSelOsdImgInfo[1].uwXStart += (ubUI_PrevRecFileIdx * 40);
	tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tFileSelOsdImgInfo[1], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_RecordSubMenuPage(UI_ArrowKey_t tArrowKey)
{
//	UI_RecordSubMenuItemList_t tSubMenuItem = (UI_RecordSubMenuItemList_t)tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Record sub menu page
		//! Check Select Camera number
		UI_DrawSubMenuPage(RECORD_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[RECORD_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[RECITEM_MAX] = {OSD2IMG_REC1MODENOR_ITEM, OSD2IMG_REC1TIMENOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
			OSD_IMG_INFO tOsdImgInfo[2];

			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart += (ubSubMenuItemPreIdx * 0x40);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOsdImgInfo[1].uwXStart += (ubSubMenuItemIdx * 0x40);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			break;
		}
		case DRAW_MENUPAGE:
		{
			OSD_IMG_INFO tOsdImgInfo;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
			uint16_t uwOptMarkOffset = 0;
			uint8_t i;

			//! Draw sub sub menu page
			if(ubSubMenuItemIdx == RECMODE_ITEM)
			{
				uint16_t uwSubSubMenuItemOsdImg[REC_RECMODE_MAX] = {OSD2IMG_REC1LOOPNOR_ICON, OSD2IMG_REC1MANUNOR_ICON, OSD2IMG_REC1TRIGNOR_ICON, OSD2IMG_REC1OFFNOR_ICON};
				uint8_t i;
				uwSubSubMenuItemOsdImg[tUI_PuSetting.RecInfo.tREC_Mode] += UI_ICON_HIGHLIGHT;
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 130;
				tOsdImgInfo.uwYStart = 300;
				OSD_EraserImg2(&tOsdImgInfo);
				for(i = 0; i < REC_RECMODE_MAX; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			else if(ubSubMenuItemIdx == RECTIME_ITEM)
			{
				uint16_t uwSubSubMenuItemOsdImg[RECTIME_MAX] = {OSD2IMG_REC1RT1MINNOR_ICON, OSD2IMG_REC1RT2MINNOR_ICON,
																OSD2IMG_REC1RT5MINNOR_ICON, OSD2IMG_REC1RT10MINNOR_ICON};
				uwSubSubMenuItemOsdImg[tUI_PuSetting.RecInfo.tREC_Time] += UI_ICON_HIGHLIGHT;
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 198;
				tOsdImgInfo.uwYStart = 300;
				OSD_EraserImg2(&tOsdImgInfo);
				for(i = 0; i < RECTIME_MAX; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				uwOptMarkOffset = 0x40;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwOptMarkOffset;
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
void UI_RecordUpdateSubSubMenuItemIndex(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	switch(*tSubMenuItem)
	{
		case RECMODE_ITEM:
			ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.RecInfo.tREC_Mode;
			break;
		case RECTIME_ITEM:
			ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.RecInfo.tREC_Time;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_RecordDrawSubSubMenuItem(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem)
{
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case RECMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[REC_RECMODE_MAX] = {OSD2IMG_REC1LOOPNOR_ICON, OSD2IMG_REC1MANUNOR_ICON, OSD2IMG_REC1TRIGNOR_ICON, OSD2IMG_REC1OFFNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case RECTIME_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[RECTIME_MAX] = {OSD2IMG_REC1RT1MINNOR_ICON, OSD2IMG_REC1RT2MINNOR_ICON,
															OSD2IMG_REC1RT5MINNOR_ICON, OSD2IMG_REC1RT10MINNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RecordSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_RecSubSubMenuItem_t tRecSubSubMenuItem = 
	{
		{
		   { 0, REC_RECMODE_MAX },{ 0, RECTIME_MAX },
		},
	};
	static uint8_t ubUI_RecStsUpdateFlag = FALSE;
	UI_RecordSubMenuItemList_t tSubMenuItem = (UI_RecordSubMenuItemList_t)tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;

	if(FALSE == ubUI_RecStsUpdateFlag)
		UI_RecordUpdateSubSubMenuItemIndex(&tRecSubSubMenuItem, &tSubMenuItem, &ubUI_RecStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tRecSubSubMenuItem.tRecordS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_RecordDrawSubSubMenuItem(&tRecSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
		{
			uint8_t *pUI_Mode = (tSubMenuItem == RECMODE_ITEM)?(uint8_t *)&tUI_PuSetting.RecInfo.tREC_Mode:
					            (tSubMenuItem == RECTIME_ITEM)?(uint8_t *)&tUI_PuSetting.RecInfo.tREC_Time:NULL;
			if(pUI_Mode)
			{
				*pUI_Mode = tRecSubSubMenuItem.tRecordS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
				UI_UpdateDevStatusInfo();
			}
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo[2];

			tOsdImgInfo[0].uwHSize  = 335;
			tOsdImgInfo[0].uwVSize  = 250;
			tOsdImgInfo[0].uwXStart = 110;
			tOsdImgInfo[0].uwYStart = 300;
			OSD_EraserImg2(&tOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_REC1LOOPWR_ICON+tUI_PuSetting.RecInfo.tREC_Mode), 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_REC1RT1MINWR_ICON+tUI_PuSetting.RecInfo.tREC_Time), 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart += (((tSubMenuItem == RECMODE_ITEM)?1:0) * 0x40);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOsdImgInfo[1].uwXStart += (((tSubMenuItem == RECMODE_ITEM)?0:1) * 0x40);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			ubUI_RecStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PhotoSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubUI_RecSubMenuFlag = TRUE;
		UI_DrawDCIMFolderMenu();
		return;
	}
	UI_DCIMFolderSelection(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_PhotoUpdateSubSubMenuItemIndex(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
    UI_CamNum_t tCamNum = tCamSelect.tCamNum4PhotoSub;
    switch (*tSubMenuItem)
    {
    case PHOTOFUNC_ITEM:
        ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CamStatus[tCamNum].tPHOTO_Func;
        break;
    case PHOTORES_ITEM:
        ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CamStatus[tCamNum].tPHOTO_Resolution;
        break;
    default:
        return;
    }
    *Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_PhotoDrawSubSubMenuItem(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem)
{
    UI_CamNum_t tCamNum = tCamSelect.tCamNum4PhotoSub;
    uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
    uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx;
    switch (*tSubMenuItem)
    {
    case PHOTOFUNC_ITEM:
        break;
    case PHOTORES_ITEM:
        {
            uint16_t uwSubSubMenuItemOsdImg[PHOTORES_MAX] = {OSD2IMG_PRES3MNOR_ICON, OSD2IMG_PRES5MNOR_ICON, OSD2IMG_PRES12MNOR_ICON};
            UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
            break;
        }
    default:
        break;
    }
}
//------------------------------------------------------------------------------
void UI_PhotoSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    static UI_PhotoSubSubMenuItem_t tPhotoSubSubMenuItem =
    {
        {
           {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
           {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
           {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
           {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }}
        },
    };
    static uint8_t ubUI_PhotoStsUpdateFlag = FALSE;
    UI_PhotoSubMenuItemList_t tSubMenuItem = (UI_PhotoSubMenuItemList_t)tUI_SubMenuItem[PHOTO_ITEM].tSubMenuInfo.ubItemIdx;
    UI_CamNum_t tCamNum                    = tCamSelect.tCamNum4PhotoSub;
    UI_MenuAct_t tMenuAct;

    if (FALSE == ubUI_PhotoStsUpdateFlag)
        UI_PhotoUpdateSubSubMenuItemIndex(&tPhotoSubSubMenuItem, &tSubMenuItem, &ubUI_PhotoStsUpdateFlag);
    tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem]);
    switch (tMenuAct)
    {
    case DRAW_HIGHLIGHT_MENUICON:
        UI_PhotoDrawSubSubMenuItem(&tPhotoSubSubMenuItem, &tSubMenuItem);
        break;
    case EXECUTE_MENUFUNC:
        {
            UI_PUReqCmd_t   tPhotoCmd;
            UI_PhotoFunction_t tPHOTO_Func = (tUI_CamStatus[tCamNum].tPHOTO_Func == PHOTOFUNC_OFF)?PHOTOFUNC_ON:PHOTOFUNC_OFF;

			if((tSubMenuItem != PHOTOFUNC_ITEM) && (tSubMenuItem != PHOTORES_ITEM))
				break;
			tPhotoCmd.tDS_CamNum = tCamNum;
			tPhotoCmd.ubCmd[UI_TWC_TYPE]	 = UI_SETTING;
			tPhotoCmd.ubCmd[UI_SETTING_ITEM] = (tSubMenuItem == PHOTOFUNC_ITEM)?PHOTOFUNC_ITEM:PHOTORES_ITEM;
			tPhotoCmd.ubCmd[UI_SETTING_DATA] = (tSubMenuItem == PHOTOFUNC_ITEM)?tPHOTO_Func:
												tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem].tSubMenuInfo.ubItemIdx;
			tPhotoCmd.ubCmd_Len  = 3;
			if(UI_SendRequestToBU(osThreadGetId(), &tPhotoCmd) == rUI_SUCCESS)
			{
				if(tSubMenuItem == PHOTOFUNC_ITEM)
					tUI_CamStatus[tCamNum].tPHOTO_Func = tPHOTO_Func;
				if(tSubMenuItem == PHOTORES_ITEM)
					tUI_CamStatus[tCamNum].tPHOTO_Resolution = (UI_PhotoResolution_t)tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem].tSubMenuInfo.ubItemIdx;
			}
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;

			if(tSubMenuItem == PHOTOFUNC_ITEM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PHOTOOFFWR_ICON+tUI_CamStatus[tCamNum].tPHOTO_Func), 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			if(tSubMenuItem == PHOTORES_ITEM)
			{
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 285;
				tOsdImgInfo.uwYStart = 312;
				OSD_EraserImg2(&tOsdImgInfo);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PRES3MWR_ICON+tUI_CamStatus[tCamNum].tPHOTO_Resolution), 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 0x40;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
			}
			ubUI_PhotoStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(UI_MAINMENU_STATE == tUI_State)
	{
		UI_DrawSubMenuPage(PLAYBACK_ITEM);
		return;
	}
	UI_DCIMFolderSelection(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_PowerSaveSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(UI_MAINMENU_STATE == tUI_State)
	{
		UI_DrawSubMenuPage(PS_ITEM);
		return;
	}
}
//------------------------------------------------------------------------------
void UI_DrawSysDateTime(UI_CalendarItem_t tShowItem, UI_IconType_t tIconType, RTC_Calendar_t *ptSysCalendar)
{
    static OSD_IMG_INFO tNumOsdImgArrayInfo[22] = {0};
    static uint8_t ubUI_RdNumOsdImgFlag = FALSE;
    uint8_t ubDateOffset[2] = {0}, i;

    if (FALSE == ubUI_RdNumOsdImgFlag)
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
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubSubMenuItemOsdImg[21], 1, &tNumOsdImgArrayInfo[21]);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubSubMenuItemOsdImg[0], 21, &tNumOsdImgArrayInfo[0]);
        ubUI_RdNumOsdImgFlag = TRUE;
    }
    switch (tShowItem)
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
        for (i = 0; i < 2; i++)
        {
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 330 - (i * 30);
            tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
        }
        if (tShowItem != ALLCALE_ITEM)
            break;
    case MONTH_ITEM:
        ubDateOffset[0] = ptSysCalendar->ubMonth / 10;
        ubDateOffset[1] = ptSysCalendar->ubMonth - (ubDateOffset[0] * 10);
        for (i = 0; i < 2; i++)
        {
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 248 - (i * 30);
            tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
        }
        if (tShowItem != ALLCALE_ITEM)
            break;
    case DATE_ITEM:
        ubDateOffset[0] = ptSysCalendar->ubDate / 10;
        ubDateOffset[1] = ptSysCalendar->ubDate - (ubDateOffset[0] * 10);
        for (i = 0; i < 2; i++)
        {
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 166 - (i * 30);
            tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
        }
        if (tShowItem != ALLCALE_ITEM)
            break;
    case HOUR_ITEM:
        ubDateOffset[0] = ptSysCalendar->ubHour / 10;
        ubDateOffset[1] = ptSysCalendar->ubHour - (ubDateOffset[0] * 10);
        for (i = 0; i < 2; i++)
        {
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 390 - (i * 30);
            tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
        }
        if (tShowItem != ALLCALE_ITEM)
            break;
    case MIN_ITEM:
        ubDateOffset[0] = ptSysCalendar->ubMin / 10;
        ubDateOffset[1] = ptSysCalendar->ubMin - (ubDateOffset[0] * 10);
        for (i = 0; i < 2; i++)
        {
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 306 - (i * 30);
            tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
        }
        if (tShowItem != ALLCALE_ITEM)
            break;
    case SEC_ITEM:
        ubDateOffset[0] = ptSysCalendar->ubSec / 10;
        ubDateOffset[1] = ptSysCalendar->ubSec - (ubDateOffset[0] * 10);
        for (i = 0; i < 2; i++)
        {
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
            tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 222 - (i * 30);
            tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
        }
        break;
    default:
        return;
    }
    if (tShowItem == ALLCALE_ITEM)
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
int UI_DrawSettingSubSubMenuPage(UI_SettingSubMenuItemList_t *tSubMenuItem)
{
    OSD_IMG_INFO tOsdImgInfo;
    int iRet = -1;
    uint8_t i;

	tOsdImgInfo.uwHSize  = 335;
	tOsdImgInfo.uwVSize  = 250;
	tOsdImgInfo.uwYStart = 312;
	switch(*tSubMenuItem)
	{
		case DATETIME_ITEM:
		{
			tOsdImgInfo.uwXStart = 150;
			OSD_EraserImg2(&tOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DT_SUBMENUICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			UI_DrawSysDateTime(ALLCALE_ITEM, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
			break;
		}
		case AECSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			uwSubSubMenuItemOsdImg[tUI_PuSetting.ubAEC_Mode] += UI_ICON_HIGHLIGHT;
			tOsdImgInfo.uwXStart = 215;
			OSD_EraserImg2(&tOsdImgInfo);
			for(i = 0; i < AECFUNC_MAX; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0;
			break;
			}
		case CCASET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[CCAMODE_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			uwSubSubMenuItemOsdImg[tUI_PuSetting.ubCCA_Mode] += UI_ICON_HIGHLIGHT;
			tOsdImgInfo.uwXStart = 275;
			OSD_EraserImg2(&tOsdImgInfo);
			for(i = 0; i < CCAMODE_MAX; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 65;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x40;
			break;
		}
		case DEFUSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_DEFUNOTHL_ICON, OSD2IMG_DEFUEXECNOR_ICON};
			for(i = 0; i < 2; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x100;
			break;
		}
		default:
			break;
	}
	return iRet;
}
//------------------------------------------------------------------------------
void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
    UI_MenuAct_t tMenuAct;

    if (tUI_State == UI_MAINMENU_STATE)
    {
        //! Draw Cameras sub menu page
        UI_DrawSubMenuPage(SETTING_ITEM);
        return;
    }
    tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[SETTING_ITEM]);
    switch (tMenuAct)
    {
    case DRAW_HIGHLIGHT_MENUICON:
        {
            uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {OSD2IMG_DTNOR_ITEM, OSD2IMG_AECNOR_ITEM, OSD2IMG_CCANOR_ITEM,
                                                             OSD2IMG_STORAGENOR_ITEM, OSD2IMG_LANGUAGENOR_ITEM, OSD2IMG_DEFUNOR_ITEM};
            uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
            uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
            if (ubSubMenuItemIdx == STORAGESTS_ITEM)
            {
                ubSubMenuItemIdx = (ubSubMenuItemPreIdx > ubSubMenuItemIdx)?--tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx:
                                                                            ++tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
                tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx = (ubSubMenuItemPreIdx > ubSubMenuItemIdx)?--tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx:
                                                                                                                   ++tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
            }
            UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
            break;
        }
    case DRAW_MENUPAGE:
        {
            //! Draw sub sub menu page
            UI_SettingSubMenuItemList_t tSubMenuItemIdx = (UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
            OSD_IMG_INFO tOsdImgInfo;
            int iOptMarkOffset = UI_DrawSettingSubSubMenuPage(&tSubMenuItemIdx);

            if (iOptMarkOffset != -1)
            {
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_OPTMARKHL_ICON, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = (0xDA + iOptMarkOffset);
                tOsdImgInfo.uwYStart = 0x205;
                tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            }
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
void UI_SettingUpdateSubSubMenuItemIndex(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
    switch (*tSubMenuItem)
    {
    case AECSET_ITEM:
        ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.ubAEC_Mode;
        break;
    case CCASET_ITEM:
        ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_PuSetting.ubCCA_Mode;
        break;
    default:
        return;
    }
    *Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_SettingDrawSubSubMenuItem(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem)
{
    uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
    uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
    switch (*tSubMenuItem)
    {
    case AECSET_ITEM:
        {
            uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
            UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
            break;
        }
    case CCASET_ITEM:
        {
            OSD_IMG_INFO tOsdImgInfo;
            uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
            tOSD_GetOsdImgInfor (1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart += 65;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart += 65;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            break;
        }
    case DEFUSET_ITEM:
        {
            uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_DEFUNOTNOR_ICON, OSD2IMG_DEFUEXECNOR_ICON};
            UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
            break;
        }
    default:
        break;
    }
}
//------------------------------------------------------------------------------
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubMenuItem_t tSettingSubSubMenuItem = 
	{
		{
		   { 0, 0 			},
		   { 0, AECFUNC_MAX },
		   { 0, CCAMODE_MAX },
		   { 0, 0 			},
		   { 0, 0 			},
		   { 0, 2 			},
		},
	};
	static uint8_t ubUI_SettingStsUpdateFlag = FALSE;
	UI_SettingSubMenuItemList_t tSubMenuItem = (UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;

    if (FALSE == ubUI_SettingStsUpdateFlag)
        UI_SettingUpdateSubSubMenuItemIndex(&tSettingSubSubMenuItem, &tSubMenuItem, &ubUI_SettingStsUpdateFlag);
    tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tSettingSubSubMenuItem.tSettingS[tSubMenuItem]);
    switch (tMenuAct)
    {
    case DRAW_HIGHLIGHT_MENUICON:
        UI_SettingDrawSubSubMenuItem(&tSettingSubSubMenuItem, &tSubMenuItem);
        break;
    case EXECUTE_MENUFUNC:
        break;
    case EXIT_MENUFUNC:
        {
            OSD_IMG_INFO tOsdImgInfo;
            tUI_State = UI_SUBMENU_STATE;
            if (tSubMenuItem == DEFUSET_ITEM)
            {
                tOsdImgInfo.uwHSize  = 200;
                tOsdImgInfo.uwVSize  = 250;
                tOsdImgInfo.uwXStart = 450;
                tOsdImgInfo.uwYStart = 312;
                OSD_EraserImg2(&tOsdImgInfo);
                break;
            }
            tOsdImgInfo.uwHSize  = 350;
            tOsdImgInfo.uwVSize  = 480;
            tOsdImgInfo.uwXStart = 100;
            tOsdImgInfo.uwYStart = 100;
            OSD_EraserImg2(&tOsdImgInfo);
            //! Change camera setting
            tOSD_GetOsdImgInfor (1, OSD_IMG2, (OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubAEC_Mode), 1, &tOsdImgInfo);
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, (OSD2IMG_AECOFFWR_ICON+tUI_PuSetting.ubCCA_Mode), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart += 65;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_STORAGATBU_ICON+tUI_PuSetting.ubSTORAGE_Mode, 1, &tOsdImgInfo);
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart -= 0xE;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOsdImgInfo.uwXStart += (0xE + 0x32);
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
            ubUI_SettingStsUpdateFlag = FALSE;
            break;
        }
    default:
        if ((tSubMenuItem == DATETIME_ITEM) && (tArrowKey == RIGHT_ARROW))
        {
            UI_DrawSysDateTime(YEAR_ITEM, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
            tUI_State = UI_SUBSUBSUBMENU_STATE;
        }
        break;
    }
}
//------------------------------------------------------------------------------
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubSubItem_t tSysDtSubSubSubItem = {YEAR_ITEM, YEAR_ITEM};
	static RTC_Calendar_t tUI_SetSysCalendar;
	static uint8_t ubUI_SetSysDtFlag = FALSE;
	uint16_t *pDT_YearNum;
	uint8_t *pDT_Num[ALLCALE_ITEM];
	uint16_t uwDT_MaxYearNum = 2098, uwDT_MinYearNum = 2015;
	uint8_t ubDT_MaxNum[ALLCALE_ITEM] = {0, 12, 31, 23, 59, 59};
	uint8_t ubDT_MinNum[ALLCALE_ITEM] = {0,  1,  1,  0,  0,  0};
	uint8_t ubDT_UpdateItem = tSysDtSubSubSubItem.ubItemIdx;

	if(FALSE == ubUI_SetSysDtFlag)
	{
		memcpy((RTC_Calendar_t *)&tUI_SetSysCalendar, (RTC_Calendar_t *)&tUI_PuSetting.tSysCalendar, sizeof(RTC_Calendar_t));
		ubUI_SetSysDtFlag	= TRUE;
	}
	pDT_YearNum 		= (uint16_t *)&tUI_SetSysCalendar.uwYear;
	pDT_Num[MONTH_ITEM] = (uint8_t *)&tUI_SetSysCalendar.ubMonth;
	pDT_Num[DATE_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubDate;
	pDT_Num[HOUR_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubHour;
	pDT_Num[MIN_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubMin;
	pDT_Num[SEC_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubSec;
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
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
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
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
			return;
		case ENTER_ARROW:
		case LEFT_ARROW:
			if((tSysDtSubSubSubItem.ubItemIdx == YEAR_ITEM) || (ENTER_ARROW == tArrowKey))
			{
				if(memcmp((RTC_Calendar_t *)&tUI_SetSysCalendar, (RTC_Calendar_t *)&tUI_PuSetting.tSysCalendar, sizeof(RTC_Calendar_t)))
				{
					memcpy((RTC_Calendar_t *)&tUI_PuSetting.tSysCalendar, (RTC_Calendar_t *)&tUI_SetSysCalendar, sizeof(RTC_Calendar_t));
					if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar)) != RTC_OK)
						printd(DBG_ErrorLvl, "Calendar base setting fail !\n");
					else
						RTC_SetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
					UI_UpdateDevStatusInfo();
				}
				UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
				tSysDtSubSubSubItem.ubItemPreIdx = YEAR_ITEM;
				tSysDtSubSubSubItem.ubItemIdx    = YEAR_ITEM;
				ubUI_SetSysDtFlag = FALSE;
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
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx,    UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemPreIdx, UI_ICON_NORMAL,    (RTC_Calendar_t *)(&tUI_SetSysCalendar));
}
*/
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
				else if((tUI_PuSetting.ubPairedBuNum == 1) && (tCamNum > DUAL_TYPE_ITEM))
					return (UI_CamNum_t)(tCamNum-2);
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
			for(;tCamNum < (tMaxCamNum - 1); tCamNum++)
			{
				if((tUI_CamStatus[tCamNum+1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[tCamNum+1].tCamConnSts == CAM_ONLINE))
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum+1);
					break;
				}
			}
			if((DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum) && (tChangeCamNum == tMaxCamNum))
				tChangeCamNum = (tUI_PuSetting.ubPairedBuNum >= 1)?(UI_CamNum_t)DUAL_TYPE_ITEM:(UI_CamNum_t)QUAL_TYPE_ITEM;
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
	
	if((!(--tUI_PuSetting.VolLvL.ubVOL_UpdateCnt))||(ubMotor0State != MC_LEFT_RIGHT_OFF)||(ubMotor1State != MC_UP_DOWN_OFF))
	{
		if(tUI_State == UI_DISPLAY_STATE)
		{
			OSD_IMG_INFO tOsdImgInfo;

            tOsdImgInfo.uwXStart = 48;
            tOsdImgInfo.uwYStart = 1119;
            tOsdImgInfo.uwHSize  = 672;
            tOsdImgInfo.uwVSize  = 161;
            OSD_EraserImg2(&tOsdImgInfo);
            tUI_PuSetting.VolLvL.ubVOL_UpdateCnt = 0;
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
    if (QUAL_VIEW == tView_Type)
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
    UI_CamViewType_t tUI_CamViewType = tCamViewSel.tCamViewType;

    if (FALSE == ubUI_RdPuOsdImgFlag)
    {
        //tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_PU_STSICON, 1, &tPuOsdImgInfo[0]);
        //tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BATLVL4_STSICON, 1, &tPuOsdImgInfo[1]);

        ubUI_RdPuOsdImgFlag = TRUE;
    }
    switch (tUI_CamViewType)
    {
    case SINGLE_VIEW:
    case SCAN_VIEW:
        tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
        if (TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
        {
            //tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
            break;
        }
        //tOSD_GetOsdImgInfor (1, OSD_IMG1, OSD1IMG_HDSTATUSMASK, 1, &tOsdImgInfo);
        //tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
        //tOSD_Img2(&tPuOsdImgInfo[0], OSD_QUEUE);
        //tOSD_Img2(&tPuOsdImgInfo[1], OSD_UPDATE);
        //OSD_Weight(OSD_WEIGHT_8DIV8);

        //tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);

        tUI_PuSetting.IconSts.ubDrawStsIconFlag = TRUE;
        break;
    case DUAL_VIEW:
    case QUAL_VIEW:
        if (TRUE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
        {
            tPuOsdImgInfo[1].uwYStart = ulBatLvL_YStart;
            UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img2, &tPuOsdImgInfo[1]);
            break;
        }
        tOSD_GetOsdImgInfor (1, OSD_IMG1, OSD1IMG_QUALSTATUSMASK, 1, &tOsdImgInfo);
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
    uint16_t uwXStart = 0, uwYStart = 0;

    uwXStart = pOsdImgInfo->uwXStart;
    uwYStart = pOsdImgInfo->uwYStart;

    if (UI_OsdUpdate == tOsdImgFnType)
        tOSD_Img2(pOsdImgInfo, tUpdateMode);
//  else if (UI_OsdErase == tOsdImgFnType)
        OSD_EraserImg2(pOsdImgInfo);
}
//------------------------------------------------------------------------------
void UI_ClearBuConnectStatusFlag(void)
{
    UI_CamNum_t tCamNum;

    for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
    {
        if (pUI_BuConnectFlag[0])
            *(pUI_BuConnectFlag[0]+tCamNum) = FALSE;
        if (pUI_BuConnectFlag[1])
            *(pUI_BuConnectFlag[1]+tCamNum) = FALSE;
    }
}
void UI_UpdateBarIcon_Part1(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint16_t uwAntLvLIdx = ANT_NOSIGNAL;
    static uint8_t ubChargShowCnt = 0;

    UI_CamNum_t tSelCamNum = tCamViewSel.tCamViewPool[0];

    uwAntLvLIdx = 6 - tUI_CamStatus[tSelCamNum].tCamAntLvl;
    //ubBatLvLIdx = 4;// BAT_LVL4;

	#if UI_NOTIMEMENU

	#else
		if(ubEnterTimeMenuFlag == 0)		
			UI_TimeShowSystemTime(1);
	#endif		
	//! Camera Number
	//UI_UpdateBuStatusOsdImg(&tCamNumOsdImgInfo[tCamNum], OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);	 	

    /*
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_2, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 920;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_5, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 904;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    */

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_ANT0+uwAntLvLIdx, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 1190;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if((UI_GetUsbDet() == 0) && (UI_GetBatChgFull() == 0)) //battery charge full
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_CHG4, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 10;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        ubChargShowCnt = 0;
    }
    else if((UI_GetUsbDet() == 0) && (UI_GetBatChgFull() != 0))
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_CHG1+ubChargShowCnt, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 10;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        ubChargShowCnt++;
        if (ubChargShowCnt == 4)
            ubChargShowCnt = 0;
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_BAT0+ubBatLvLIdx, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 10;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        ubChargShowCnt = 0;
    }

    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}

void UI_UpdateBarIcon_Part2(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    UI_CamNum_t tSelCamNum;

    tSelCamNum = tCamViewSel.tCamViewPool[0];

    //printd(Apk_DebugLvl, "UI_UpdateBarIcon_Part2 tSelCamNum: %d.\n", tSelCamNum);
	printd(1,"tUI_PuSetting.ubScanModeEn =%d. tUI_PuSetting.ubScanTime = %d ubCameraOnlineNum =%d\n",tUI_PuSetting.ubScanModeEn,tUI_PuSetting.ubScanTime,ubCameraOnlineNum);
    //if ((tUI_PuSetting.ubScanModeEn == FALSE) && (tUI_PuSetting.ubScanTime == 0) && (ubCameraOnlineNum < 2))
    if(ubCameraOnlineNum < 2)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 798;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_CAM1+tSelCamNum, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 798;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
/*
     #if UI_NOTIMEMENU
	    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_TEMPC+(!tUI_PuSetting.ubTempunitFlag), 1, &tOsdImgInfo);
	    tOsdImgInfo.uwXStart = 0;
	    tOsdImgInfo.uwYStart = 93;
	    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

     #else 	 
	    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_TEMPC+(!tUI_PuSetting.ubTempunitFlag), 1, &tOsdImgInfo);
	    tOsdImgInfo.uwXStart = 0;
	    tOsdImgInfo.uwYStart = 93+137+24;
	    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
     #endif
*/
    if ((tUI_PuSetting.NightmodeFlag >> tSelCamNum) & (0x01) == 1)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NIGHT, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 1127;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 1127;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    if ((tUI_PuSetting.ubScanModeEn == TRUE) && (tUI_PuSetting.ubScanTime > 0) && (ubCameraOnlineNum >= 2))
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_SCAN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 727;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_BLANK1, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 727;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    if (ubAlarmIconFlag == 1)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_ALARM, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 870;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }
    else
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_BLANK1 , 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 0;
        tOsdImgInfo.uwYStart = 870;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

#if PICKUP_VOLUME_TEST
    ubPickupVolume = (ubGetVoice1<<24)+(ubGetVoice2<<16)+(ubGetVoice3<<8)+ubGetVoice4;
    //printd(Apk_DebugLvl, "UI_UpdateBarIcon_Part2 ubPickupVolume: %d.\n", ubPickupVolume );
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(ubPickupVolume/10000), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 685 - 19;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(ubPickupVolume/1000)%10, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart =  685 - 19*2;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(ubPickupVolume/100)%10, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 685 - 19*3;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(ubPickupVolume/10)%10, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 685 - 19*4;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_BAR_NUM_0+(ubPickupVolume%10), 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 685 - 19*5;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
#endif

    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_RedrawBuConnectStatusIcon(UI_CamNum_t tCamNum)
{
    static OSD_IMG_INFO tCamNumOsdImgInfo[CAM_4T];
    static OSD_IMG_INFO tCamAntLvlOsdImgInfo[7], tCamBatLvlOsdImgInfo[6];
    static uint8_t ubUI_RdBuStsOsdImgFlag       = FALSE;
    static uint8_t ubUI_BuOnlineFlag[CAM_4T]    = {FALSE, FALSE, FALSE, FALSE};
    static uint8_t ubUI_BuOfflineFlag[CAM_4T]   = {FALSE, FALSE, FALSE, FALSE};
    static uint8_t ubUI_NoSignalOsdFlag[CAM_4T] = {FALSE, FALSE, FALSE, FALSE};
    uint16_t uwCamNumOsdIdx[CAM_4T] = {OSD2IMG_BAR_CAM1, OSD2IMG_BAR_CAM2,
                                       OSD2IMG_BAR_CAM3, OSD2IMG_BAR_CAM4};
//  OSD_IMG_INFO tOsdImgInfo;
    UI_DisplayLocation_t tUI_DispLoc;
//  uint16_t uwAntLvLIdx = ANT_NOSIGNAL, uwBatLvLIdx = BAT_LVL0;

    if (FALSE == ubUI_RdBuStsOsdImgFlag)
    {
        UI_CamNum_t tSelCamNum;

        //tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_ADOSRC_STSICON, 1, &tAdoSrcOsdImgInfo);
        //tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_STSMASK_STSICON, 1, &tMarkAdoOsdImgInfo);

        /*
        for (tSelCamNum = CAM1; tSelCamNum < tUI_PuSetting.ubTotalBuNum; tSelCamNum++)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, uwCamNumOsdIdx[tSelCamNum], 1, &tCamNumOsdImgInfo[tSelCamNum]);
            tCamNumOsdImgInfo[tSelCamNum].uwXStart = 0;
            tCamNumOsdImgInfo[tSelCamNum].uwYStart = 1070;
        }

        //printd(Apk_DebugLvl, "osd 1\n");
        */

        pUI_BuConnectFlag[0] = &ubUI_BuOnlineFlag[0];
        pUI_BuConnectFlag[1] = &ubUI_BuOfflineFlag[0];
        ubUI_RdBuStsOsdImgFlag = TRUE;
    }
    if (TRUE == tUI_PuSetting.ubDualModeEn)
        tUI_DispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
    else
        tUI_DispLoc = (tCamViewSel.tCamViewType == SINGLE_VIEW)?DISP_UPPER_RIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;
    if ((TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) ||
       (UI_MAINMENU_STATE == tUI_State))
    {
        ubUI_BuOnlineFlag[tCamNum]  = FALSE;
        ubUI_BuOfflineFlag[tCamNum] = FALSE;
        if (UI_MAINMENU_STATE == tUI_State)
            return;
        return;
    }
    switch (tUI_CamStatus[tCamNum].tCamConnSts)
    {
    case CAM_ONLINE:
        //printd(Apk_DebugLvl, "dis osd 2\n");

        if (TRUE == ubUI_BuOnlineFlag[tCamNum])
            break;

        //! Audio Source
        //if (tUI_PuSetting.tAdoSrcCamNum == tCamNum)
        //  UI_UpdateBuStatusOsdImg(&tAdoSrcOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);

        ubUI_NoSignalOsdFlag[tCamNum]   = FALSE;
        ubUI_BuOnlineFlag[tCamNum]      = TRUE;
        ubUI_BuOfflineFlag[tCamNum]     = FALSE;
        break;
    case CAM_OFFLINE:
        if (TRUE == ubUI_BuOfflineFlag[tCamNum])
            return;

        ubUI_NoSignalOsdFlag[tCamNum]   = TRUE;
        ubUI_BuOfflineFlag[tCamNum]     = TRUE;
        ubUI_BuOnlineFlag[tCamNum]      = FALSE;
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
//  OSD_IMG_INFO tOsdImgInfo;
    UI_CamNum_t tCamNum, tDrawCamNum;
    UI_CamViewType_t tUI_CamViewType = (TRUE == tUI_PuSetting.ubDualModeEn)?DUAL_VIEW:tCamViewSel.tCamViewType;
    uint8_t ubUI_TotalBuNum = (TRUE == tUI_PuSetting.ubDualModeEn)?CAM_2T:tUI_PuSetting.ubTotalBuNum;

    switch (tUI_CamViewType)
    {
    case SINGLE_VIEW:
    case SCAN_VIEW:
        if (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID)
            UI_RedrawBuConnectStatusIcon(tCamViewSel.tCamViewPool[0]);
        break;
    case DUAL_VIEW:
    case QUAL_VIEW:
        for (tCamNum = CAM1; tCamNum < ubUI_TotalBuNum; tCamNum++)
        {
            tDrawCamNum = tCamNum;
            if (tCamViewSel.tCamViewType == DUAL_VIEW) {
                tDrawCamNum = (tCamNum == CAM1)?tCamViewSel.tCamViewPool[0]:tCamViewSel.tCamViewPool[1];
            }
            if (tUI_CamStatus[tDrawCamNum].ulCAM_ID != INVALID_ID) {
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

    if(PS_VOX_MODE == tUI_PuSetting.tPsMode)
        return;
    if (FALSE == ubUI_ResetPeriodFlag)
    {
            	//printd(1,"UI_ShowLostLinkLogo   FALSE == ubUI_ResetPeriodFlag\n");
        uwUI_LostPeriod = UI_SHOWLOSTLOGO_PERIOD * 2; //UI_SHOWLOSTLOGO_PERIOD * 5
    }
    else if (ubCamPairOkState >= 1)
    {
        	//printd(1,"UI_ShowLostLinkLogo ubCamPairOkState >= 1\n");
        ubFastShowLostLinkSta = 0;
        uwUI_LostPeriod = UI_SHOWLOSTLOGO_PERIOD * 2;
    }
    else
    {
    	//printd(1,"UI_ShowLostLinkLogo tPairInfo.ubDrawFlag %d\n",tPairInfo.ubDrawFlag);
        if (tPairInfo.ubDrawFlag == FALSE)
            ubFastShowLostLinkSta = 1;
    }
	
    	//printd(1,"UI_ShowLostLinkLogo uwUI_LostPeriod %d\n",uwUI_LostPeriod);
    for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
    {
        if (PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
        {
            UI_DrawPUStatusIcon();
            UI_DrawBUStatusIcon();
            return;
        }
    }

    if (((FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) && (*pThreadCnt == uwUI_LostPeriod))
        || ((FALSE == tUI_PuSetting.IconSts.ubShowLostLogoFlag) && (ubFastShowLostLinkSta == 1)))
    {
        uint32_t ulLostLogAddr = *((uint32_t *)(pbPROF_GetParam(SYSPARAM) + LOSTLOGO_ADDR));
        //UI_CamNum_t tCamNum;

        tUI_PuSetting.IconSts.ubShowLostLogoFlag = TRUE;
        //for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
        //  UI_RedrawBuConnectStatusIcon(tCamNum);

        if (ubFactoryModeFLag == 0)
        {
            if (tUI_PuSetting.ubDefualtFlag  == FALSE)
            {
                tUI_State = UI_DISPLAY_STATE;

                tOsdImgInfo.uwHSize  = 672;
                tOsdImgInfo.uwVSize  = 1280;
                tOsdImgInfo.uwXStart = 48;
                tOsdImgInfo.uwYStart = 0;
                OSD_EraserImg2(&tOsdImgInfo);

                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 0;
                tOsdImgInfo.uwYStart = 0;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                UI_ClearStatusBarOsdIcon();
                //OSD_LogoJpeg(OSDLOGO_LOSTLINK);

                ubFastShowLostLinkSta = 0;
                ubCamPairOkState = 0;
                if (ubNoAddCamFlag == 1)
                {
                    OSD_LogoJpeg(OSDLOGO_WHITE_BG);
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM1+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart = 175;
                    tOsdImgInfo.uwYStart = 279;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                    if (tUI_PuSetting.ubLangageFlag == 2)
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart = 503;
                        tOsdImgInfo.uwYStart = 360 - 80 - 5;
                        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                    }
                    else
                    {
                        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOCAM2+ (21*tUI_PuSetting.ubLangageFlag), 1, &tOsdImgInfo);
                        tOsdImgInfo.uwXStart = 503;
                        tOsdImgInfo.uwYStart = 360 - 80;
                        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                    }
			if(ubWorWakeUpFlag == 1)
				LCDBL_ENABLE(UI_ENABLE);
                    //printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOCAM1.\n");
                }
                else
                {
                    /*
                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL1+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 160;
                    tOsdImgInfo.uwYStart =328;
                    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_NOSIGNAL2+ (21*tUI_PuSetting.ubLangageFlag ), 1, &tOsdImgInfo);
                    tOsdImgInfo.uwXStart= 480;
                    tOsdImgInfo.uwYStart =304-104;
                    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
                    */
                    tLCD_JpegDecodeDisable();
                    OSD_LogoJpeg(OSDLOGO_LOSTLINK+tUI_PuSetting.ubLangageFlag);
                    //printd(Apk_DebugLvl, "UI_ShowLostLinkLogo OSD2IMG_MENU_NOSIGNAL1.\n");
 
 		       //ubTempAlarmState = TEMP_ALARM_IDLE;
                    //ubTempAlarmCheckCount = 0;
                    //ubTempAlarmTriggerCount = 0;
                    ubPickupAlarmState = PICKUP_ALARM_IDLE;
                    ubPickupAlarmCheckCount = 0;
                    ubPickupAlarmTriggerCount = 0;
			if(ubWorWakeUpFlag == 1)
				LCDBL_ENABLE(UI_ENABLE);	
 
                }
            }
            else
            {
                tOsdImgInfo.uwHSize  = 1280;
                tOsdImgInfo.uwVSize  = 720;
                tOsdImgInfo.uwXStart = 48;
                tOsdImgInfo.uwYStart = 0;
                OSD_EraserImg1(&tOsdImgInfo);

                OSD_LogoJpeg(OSDLOGO_LOGO_BG);

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

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 0;
            tOsdImgInfo.uwYStart = 0;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            UI_ClearStatusBarOsdIcon();
            tLCD_JpegDecodeDisable();
            OSD_LogoJpeg(OSDLOGO_LOGO_BG);

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TITLE, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 51;
            tOsdImgInfo.uwYStart =462 - 150;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        }
        ubClearOsdFlag =0;
        ubUI_ResetPeriodFlag = TRUE;
    }

    if (((*pThreadCnt%3) == 0)&&(tUI_PuSetting.ubDefualtFlag == FALSE)&&(TRUE == tUI_PuSetting.IconSts.ubShowLostLogoFlag))
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

    if (FALSE == tUI_PuSetting.IconSts.ubDrawStsIconFlag)
        return;

    //tOsdImgInfo.uwHSize = 100;
    //tOsdImgInfo.uwVSize = ulLcd_VSize;
    //OSD_EraserImg1(&tOsdImgInfo);
    //if (DISPLAY_4T1R == tUI_PuSetting.ubTotalBuNum)
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

    if (TRUE == ubUI_StopUpdateStsBarFlag)
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
						tOsdImgInfo.uwHSize  = 672;
						tOsdImgInfo.uwVSize  = 1280;
						tOsdImgInfo.uwXStart = 48;
						tOsdImgInfo.uwYStart = 0;
						OSD_EraserImg2(&tOsdImgInfo);
					}
					
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart = 0;
					tOsdImgInfo.uwYStart = 0;
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					
					//ubClearOsdFlag =1;
				}
			   	UI_UpdateBarIcon_Part1();
				UI_UpdateBarIcon_Part2();	
				UI_VolBarDisplay(ubGetVoiceTemp);
				UI_TempBarDisplay(ubRealTemp);
				if(ubPairOK_SwitchCam == 1)
				{
					UI_DisableScanMode();

                    printd(Apk_DebugLvl, "UI_RedrawStatusBar tCamViewNum: %d, tPairSelCam: %d.\n", tCamViewSel.tCamViewPool[0], tPairInfo.tPairSelCam);
                    if (tCamViewSel.tCamViewPool[0] != tPairInfo.tPairSelCam)
                    {
                        tCamViewSel.tCamViewType    = SINGLE_VIEW;
                        tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam;
                        tUI_PuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
                        UI_SwitchCameraSource();
                        //ubPairOK_SwitchCam = 0;
                    }
                    ubPairOK_SwitchCam = 0;

                    if ((TRUE == tUI_PuSetting.ubScanModeEn) && (FALSE == ubUI_ScanStartFlag))
                    {
                        UI_EnableScanMode();
                    }
                }

				if(ubClearOsdFlag == 0)
				{
					//if(ubReStartWakeUpFlag == 1)
					//	LCDBL_ENABLE(UI_ENABLE);	
					if(ubWorWakeUpFlag == 1)
						LCDBL_ENABLE(UI_ENABLE);	

					ubClearOsdFlag =1;
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
				
				OSD_LogoJpeg(OSDLOGO_LOGO_BG);
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
            if (ubClearOsdFlag == 0)
            {
                if (tUI_State == UI_DISPLAY_STATE)
                {
                    tOsdImgInfo.uwHSize  = 1280;
                    tOsdImgInfo.uwVSize  = 720;
                    tOsdImgInfo.uwXStart = 48;
                    tOsdImgInfo.uwYStart = 0;
                    OSD_EraserImg1(&tOsdImgInfo);
                }
                tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
                tOsdImgInfo.uwXStart = 0;
                tOsdImgInfo.uwYStart = 0;
                tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

                ubClearOsdFlag =1;
            }
            UI_UpdateBarIcon_Part1();
            UI_UpdateBarIcon_Part2();

            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TITLE, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart= 51;
            tOsdImgInfo.uwYStart =462 - 150;
            tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);

            //UI_FactoryStatusDisplay();

            UI_VolBarDisplay(OSD_QUEUE);
        }
    }
    //if (TRUE == ubUI_ShowTimeFlag)
    //  UI_ShowSysTime();

    //UI_TimeShowSystemTime(0);
}

void UI_FactoryStatusDisplay(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    uint8_t ubper_temp, ubir_temp1,ubir_temp2;
    uint16_t ubir_temp;

    //printd(Apk_DebugLvl, "UI_FactoryStatusDisplay.\n");
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_PER, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TEMP, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 115 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_IRAD, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 180 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TXSW, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_RXSW, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 310 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_SDCARD, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 375 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TX_SN, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 440 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_RX_SN, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 505 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 1055;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    if (ubPUEnterAdotestFLag == 1)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_RXMICSPK, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 570 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 863;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    if (tUI_SyncAppState == APP_LINK_STATE)
    {
        if (ubBUEnterAdotestFLag == 1)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TXMICSPK, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 635 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 863;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	     ubBUEnterAdotestFLag = 0;
        }
    }
    //-----------------------------------------------------------------
    if (tUI_SyncAppState == APP_LINK_STATE)
    {
        KNL_ROLE tKNL_Role = (KNL_ROLE)(UI_GetCamViewPoolID());
        ubper_temp = 100 - KNL_GetPerValue(tKNL_Role);
        uint8_t ubRssiVal = KNL_GetRssiValue(tKNL_Role);

        if ( ubper_temp < 100)
        {
            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 900+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubper_temp/10)*2), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 868+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubper_temp%10)*2), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 836+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }
        else
        {
            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_1_S, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 900+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 868+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

            tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S, 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 836+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

        #if 1
        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_SIGN, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 804+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubRssiVal/100)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 772+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S + (((ubRssiVal/10)%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 740+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S + ((ubRssiVal%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 50 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 708+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        #endif
        //------------------------------------------------------------------
        tUI_PuSetting.ubTempunitFlag = 1;
        UI_TempBarDisplay(ubRealTemp);
        //ubRealTemp = tUI_PuSetting.ubTempunitFlag?ubRealTemp:UI_TempCToF(ubRealTemp);
        if ( ubRealTemp < 100)
        {
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubRealTemp/10)*2), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 115 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 900+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
            tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubRealTemp%10)*2), 1, &tOsdImgInfo);
            tOsdImgInfo.uwXStart = 115 + Factory_x_vol;
            tOsdImgInfo.uwYStart = 868+ Factory_y_vol;
            tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        }

		//---------------------------------------------
		ubir_temp  = (ubGetIR1Temp<<8)+ubGetIR2Temp;

        ubir_temp1 = ubir_temp/100;
        ubir_temp2 = ubir_temp%100;

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp1/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 180 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 900+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp1%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 180 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 868+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp2/10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 180 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 836+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S+((ubir_temp2%10)*2), 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 180 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 804+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        //---------------------------------------------
        //BU Version
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_V, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 900+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S + (ubBuSWVersion/10)*2, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 900 - 32+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_POINT , 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 900 - 32 - 24+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S + (ubBuSWVersion%10)*2, 1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 245 + Factory_x_vol;
        tOsdImgInfo.uwYStart = 900 - 32 - 24 - 32+ Factory_y_vol;
        tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
    }

    //---------------------------------------------
    //PU Version
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_V, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 310 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 900+ Factory_y_vol;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S + (ubPuSWVersion/10)*2, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 310 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 900 - 32+ Factory_y_vol;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_POINT , 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 310 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 900 - 32 - 24+ Factory_y_vol;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_DIS_TIME_0_S + (ubPuSWVersion%10)*2, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 310 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 900 - 32 - 24 - 32+ Factory_y_vol;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    UI_DisplaySN(380 + Factory_x_vol, 1000, ubSD_ChkCardIn(SD_1) ? "ON " : "OFF", 3);
    UI_DisplaySN(445 + Factory_x_vol, 1000, TxSNdata, sizeof(TxSNdata));
    UI_DisplaySN(510 + Factory_x_vol, 1000, RxSNdata, sizeof(RxSNdata));

    if (1) {
        char str[10];
        sprintf(str, "%4d", ubGetBatVoltage);
        UI_DisplaySN(0, 700, str, sizeof(str));
        //sprintf(str, "%4d %3d", ubGetBatVoltage, ubGetBatPercent);
        //UI_DisplaySN(575 + Factory_x_vol, 1000, str, sizeof(str));
    }
}

void UI_FactorymodeKeyDisplay(uint8_t Value)
{
    OSD_IMG_INFO tOsdImgInfo;

    switch (Value)
    {
    case AKEY_MENU:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_MENUKEY ,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 238;
        tOsdImgInfo.uwYStart = 254;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case AKEY_ENTER:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_ENTERKEY,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 431;
        tOsdImgInfo.uwYStart = 254;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case AKEY_UP:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_UPKEY ,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 336;
        tOsdImgInfo.uwYStart = 254;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case AKEY_DOWN:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_DOWNKEY ,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 527;
        tOsdImgInfo.uwYStart = 254;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case AKEY_RIGHT:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_LEFTKEY ,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 431;
        tOsdImgInfo.uwYStart = 30;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case AKEY_LEFT:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_RIGHTKEY ,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 431;
        tOsdImgInfo.uwYStart = 477;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case AKEY_PTT:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_PTTKEY,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 623;
        tOsdImgInfo.uwYStart = 254;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case PKEY_ID0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_PKEY ,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 143;
        tOsdImgInfo.uwYStart = 30;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case GKEY_ID0:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_VOLUKEY,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 143;
        tOsdImgInfo.uwYStart = 477;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;

    case GKEY_ID1:
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_VOLDKEY,1, &tOsdImgInfo);
        tOsdImgInfo.uwXStart = 143;
        tOsdImgInfo.uwYStart = 254;
        tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
        break;
    }
}

void UI_EnterLocalAdoTest_RX(void)
{
    OSD_IMG_INFO tOsdImgInfo;
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_RXMICSPK, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 570 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 863;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    printd(Apk_DebugLvl,"UI_EnterLocalAdoTest_RX\n");
    ADO_SelfTest_Init(); //ADO_SelfTest_Init(5);
    ADO_SelfTest_Record();
    ADO_SelfTest_Play();
    ADO_SelfTest_Close();
	
   if (ubBUEnterAdotestFLag == 0)
   {  	
      //SET BU MIC&SPK TEST
   	while(!UI_GetBuMICTest());
	printd(1,"send MIC&SPK TEST success!\n");
	UI_SendCMDAdoTest_TX();
	 ubBUEnterAdotestFLag = 1;
    }

	
}

void UI_SendCMDAdoTest_TX(void)
{
    OSD_IMG_INFO tOsdImgInfo;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_FACTORY_TXMICSPK, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 635 + Factory_x_vol;
    tOsdImgInfo.uwYStart = 863;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    //UI_TestCmd(1,1);
}

//------------------------------------------------------------------------------
void UI_ChangeBuPsModeToNormalMode(UI_CamNum_t tPS_CamNum)
{
    OSD_IMG_INFO tOsdImgInfo;
    UI_DisplayLocation_t tUI_DispLoc;

    tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_RIGHT:tUI_CamStatus[tPS_CamNum].tCamDispLocation;
    if (tCamViewSel.tCamViewType != SCAN_VIEW)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
        UI_UpdateBuStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
        if (tCamViewSel.tCamViewType == SINGLE_VIEW)
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
    if (pUI_BuConnectFlag[0])
        *(pUI_BuConnectFlag[0]+tPS_CamNum) = FALSE;
    if (pUI_BuConnectFlag[1])
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
        {ANT_NOSIGNAL,       10},
        {ANT_SIGNALLVL1,     90},
        {ANT_SIGNALLVL2,    110},
        {ANT_SIGNALLVL3,    130},
        {ANT_SIGNALLVL4,    160},
        {ANT_SIGNALLVL5,    255},
    };
    */
    UI_PerMap2AntLvl_t tAntMap[] =
    {
        {ANT_NOSIGNAL,       10},
        {ANT_SIGNALLVL1,     20},
        {ANT_SIGNALLVL2,     30},
        {ANT_SIGNALLVL3,     40},
        {ANT_SIGNALLVL4,     60},
        {ANT_SIGNALLVL5,     80},
        {ANT_SIGNALLVL6,    100},
    };

//  uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_RssiMap2AntLvl_t), ubIdx;
    uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_PerMap2AntLvl_t), ubIdx;
    static uint8_t ubUI_PsStsFlag = FALSE;
    static uint32_t ulUI_EcoStsCnt[CAM_4T] = {0, 0, 0, 0};

    for (tCamNum = CAM1; tCamNum < tUI_PuSetting.ubTotalBuNum; tCamNum++)
    {
        tUI_CamStatus[tCamNum].tCamConnSts  = (pCamConnSts[APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum] == rLINK)?CAM_ONLINE:CAM_OFFLINE;
        if (PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
        {
            switch (tUI_CamStatus[tCamNum].tCamConnSts)
            {
            case CAM_ONLINE:
                if (TRUE == ulUI_MonitorPsFlag[tCamNum])
                {
                    APP_EventMsg_t tUI_PsMessage = {0};

                    tUI_PsMessage.ubAPP_Event      = APP_POWERSAVE_EVENT;
                    tUI_PsMessage.ubAPP_Message[0] = 4;     //! Message Length
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
                    #define UI_CHKBUECOSTS_PERIOD (2000 / UI_TASK_PERIOD)
                    if (++ulUI_EcoStsCnt[tCamNum] > UI_CHKBUECOSTS_PERIOD)
                    {
                        ulUI_EcoStsCnt[tCamNum] = 0;
                        UI_ChangeBuPsModeToNormalMode(tCamNum);
                    }
                }
                break;
            case CAM_OFFLINE:
                if (FALSE == ulUI_MonitorPsFlag[tCamNum])
                {
                    ulUI_EcoStsCnt[tCamNum] = 0;
                    ulUI_MonitorPsFlag[tCamNum] = TRUE;
                }
                tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
                if (FALSE == ubUI_PsStsFlag)
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
//      printd(DBG_Debug3Lvl, "RSSI : %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 8]);
        printd(DBG_Debug3Lvl, "PER: %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]);
        for (ubIdx = 0; ubIdx < ubUI_AntLvlCnt; ubIdx++)
        {
//          if ((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 8]) < tAntMap[ubIdx].ubRssiValue)
            if ((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]) <= tAntMap[ubIdx].ubPerValue)
            {
                tUI_CamStatus[tCamNum].tCamAntLvl = tAntMap[ubIdx].tAntLvl;
                break;
            }
        }
    }
}
//------------------------------------------------------------------------------
void UI_UpdateBuStatus(UI_CamNum_t tCamNum, void *pvStatus)
{
//  uint8_t *pUI_BuSts = (uint8_t *)pvStatus;

//  if (tCamNum > CAM4)
//      return;

//	tUI_CamStatus[tCamNum].tCamBatLvl = pUI_BuSts[0];
}
//------------------------------------------------------------------------------
void UI_LeftArrowLongKey(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubVdoRecMode;
	for(ubVdoRecMode = UI_VDORECLOOP_MODE; ubVdoRecMode < UI_VDOMODE_MAX; ubVdoRecMode++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_RECLOOPMODENOR_ICON+(ubVdoRecMode*2)+((tUI_PuSetting.tVdoRecMode == ubVdoRecMode)?UI_ICON_HIGHLIGHT:0)), 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, ((ubVdoRecMode+1) == UI_VDOMODE_MAX)?OSD_UPDATE:OSD_QUEUE);
	}
	tUI_State = UI_SET_VDOMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_RightArrowLongKey(void)
{
    if (TRUE == ubUI_ShowTimeFlag)
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

    if (FALSE == ubUI_SysTimeFlag)
    {
        tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_CK_NUM0, 11, &tOsdImgInfo[0]);
        ubUI_SysTimeFlag = TRUE;
    }
    ubRtcSec = tUI_PuSetting.tSysCalendar.ubSec;
    RTC_GetCalendar((RTC_Calendar_t *)(&tUI_PuSetting.tSysCalendar));
    printd(DBG_Debug2Lvl, "Time: %02d:%02d:%02d\n", tUI_PuSetting.tSysCalendar.ubHour, tUI_PuSetting.tSysCalendar.ubMin, tUI_PuSetting.tSysCalendar.ubSec);
    if (ubRtcSec == tUI_PuSetting.tSysCalendar.ubSec)
        return;
    for (ubSysTimeIdx = 0; ubSysTimeIdx < 3; ubSysTimeIdx++)
    {
        ubTen  = *pDT_Num[ubSysTimeIdx] / 10;
        ubUnit = *pDT_Num[ubSysTimeIdx] - (ubTen * 10);
        tOsdImgInfo[ubTen].uwXStart  = 680;
        tOsdImgInfo[ubTen].uwYStart  = uwYOffset - tOsdImgInfo[ubTen].uwVSize;
        tOSD_Img2(&tOsdImgInfo[ubTen], OSD_QUEUE);
        tOsdImgInfo[ubUnit].uwXStart = 680;
        tOsdImgInfo[ubUnit].uwYStart = tOsdImgInfo[ubTen].uwYStart - tOsdImgInfo[ubUnit].uwVSize;
        tOSD_Img2(&tOsdImgInfo[ubUnit], (ubSysTimeIdx == 2)?OSD_UPDATE:OSD_QUEUE);
        if (ubSysTimeIdx != 2)
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
        tOsdImgInfo[10].uwXStart     = 0;
        tOsdImgInfo[10].uwYStart     = 0;
    }
}
//------------------------------------------------------------------------------
void UI_UnBindBu(UI_CamNum_t tUI_DelCam)
{
    APP_EventMsg_t tUI_UnindBuMsg = {0};

    if (INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
        return;
    tUI_CamStatus[tUI_DelCam].ulCAM_ID    = INVALID_ID;
    tUI_CamStatus[tUI_DelCam].tCamConnSts = CAM_OFFLINE;
    tUI_PuSetting.ubPairedBuNum -= (tUI_PuSetting.ubPairedBuNum == 0)?0:1;
    UI_ResetDevSetting(tUI_DelCam);
    UI_UpdateDevStatusInfo();
    tUI_UnindBuMsg.ubAPP_Event      = APP_UNBIND_BU_EVENT;
    tUI_UnindBuMsg.ubAPP_Message[0] = 1;        //! Message Length
    tUI_UnindBuMsg.ubAPP_Message[1] = tUI_DelCam;
    UI_SendMessageToAPP(&tUI_UnindBuMsg);
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
    if (tCamNum > CAM4)
        return;

#if 0
    if (PS_VOX_MODE == tUI_PuSetting.tPsMode)
    {
        tUI_PsMessage.ubAPP_Event      = APP_POWERSAVE_EVENT;
        tUI_PsMessage.ubAPP_Message[0] = 2;     //! Message Length
        tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
        tUI_PsMessage.ubAPP_Message[2] = FALSE;
        UI_SendMessageToAPP(&tUI_PsMessage);
        tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
        tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
        //UI_UpdateDevStatusInfo();
    }
#else
    printd(Apk_DebugLvl, "UI_VoxTrigger tCamNum: %d, tUI_PuSetting.tPsMode: %d $$$\n", tCamNum, tUI_PuSetting.tPsMode);
    if (PS_VOX_MODE == tUI_PuSetting.tPsMode)
    {
        //UI_SetSleepState(1);
    }
#endif
}
//------------------------------------------------------------------------------
void UI_EnableVox(void)
{
    APP_EventMsg_t tUI_PsMessage = {0};

    //if (DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
        //return;
    printd(Apk_DebugLvl, "UI_EnableVox.\n");
    LCDBL_ENABLE(UI_DISABLE);

    tUI_PsMessage.ubAPP_Event      = APP_POWERSAVE_EVENT;
    tUI_PsMessage.ubAPP_Message[0] = 2;     //! Message Length
    tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
    tUI_PsMessage.ubAPP_Message[2] = TRUE;
    UI_SendMessageToAPP(&tUI_PsMessage);
    tUI_PuSetting.tPsMode = PS_VOX_MODE;
    UI_UpdateDevStatusInfo();
    LCD_UnInit();
    LCD->LCD_MODE = LCD_GPIO;
    printd(Apk_DebugLvl, "UI_EnableVox ok.\n");
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
    APP_EventMsg_t tUI_PsMessage = {0};

    //if (DISPLAY_1T1R != tUI_PuSetting.ubTotalBuNum)
        //return;

    printd(Apk_DebugLvl, "UI_DisableVox###\n");

    if (ubSwitchNormalFlag == 0)
        return;

    ubSwitchNormalFlag =0;

#if 0
    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
        (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
    {
        tPsCmd.tDS_CamNum               = tCamViewSel.tCamViewPool[0];
        tPsCmd.ubCmd[UI_TWC_TYPE]       = UI_SETTING;
        tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
        tPsCmd.ubCmd[UI_SETTING_DATA]   = POWER_NORMAL_MODE;
        tPsCmd.ubCmd_Len                = 3;
        if (UI_SendRequestToBU(osThreadGetId(), &tPsCmd) != rUI_SUCCESS)
        {
            printd(DBG_ErrorLvl, "Disable VOX Fail !\n");
            if (CAM_ONLINE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts)
                return;
        }
    }
#else
    ubNormalModeToBuRet = UI_SendPwrNormalModeToBu();
#endif

    tUI_PsMessage.ubAPP_Event      = APP_POWERSAVE_EVENT;
    tUI_PsMessage.ubAPP_Message[0] = 2;     //! Message Length
    tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
    tUI_PsMessage.ubAPP_Message[2] = FALSE;
    UI_SendMessageToAPP(&tUI_PsMessage);
    tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;
    tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode = POWER_NORMAL_MODE;

    if (APP_LINK_EVENT == APP_UpdateLinkStatus())
    {
        SSP->SSP_GPIO_MODE = 0; //0:Normal SSP Mode
        osDelay(50);            //???
        LCD_PWR_ENABLE;
        osDelay(400);
    }

    UI_UpdateDevStatusInfo();
    printd(Apk_DebugLvl, "UI_DisableVox ok###\n");
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
                             [DISP_LEFT]       = 0,               [DISP_RIGHT]       = 0};
    uint16_t uwYOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
                             [DISP_LOWER_LEFT] = 0,               [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
                             [DISP_LEFT]       = 0,               [DISP_RIGHT]       = (ulLcd_VSize/2)};

    if (tCamNum > CAM4)
        return;

    tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tUI_CamStatus[tCamNum].tCamDispLocation;
    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MDTRIG_ICON, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart += uwXOffset[tUI_DispLoc];
    tOsdImgInfo.uwYStart -= uwYOffset[tUI_DispLoc];
    tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
    tUI_PuSetting.IconSts.ubDrawMdTrigFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
    if (DISPLAY_4T1R != tUI_PuSetting.ubTotalBuNum)
        return;

    UI_DisableScanMode();
    tUI_PuSetting.ubDualModeEn  = FALSE;
    tCamViewSel.tCamViewType    = SINGLE_VIEW;
    tCamViewSel.tCamViewPool[0]     = tCamNum;
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
        if (tTWC_Send(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
            break;
        osDelay(10);
    }
    if (!ubUI_TwcRetry)
    {
        tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
        return rUI_FAIL;
    }
    tosUI_Notify.thread_id = thread_id;
    tosUI_Notify.iSignals  = osUI_SIGNALS;
    if (tosUI_Notify.thread_id != NULL)
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
    if (NULL == tosUI_Notify.thread_id)
        return;
    tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
    if (osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
        printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
//------------------------------------------------------------------------------
void UI_RecvBURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
    switch (pTwc_Data[UI_TWC_TYPE])
    {
    case UI_REPORT:
        {
            UI_CamNum_t tCamNum = NO_CAM;

            APP_TwcTagMap2CamNum(tRecv_StaNum, tCamNum);
            if (tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction)
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
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAecMode,  		CAMSET_OFF);
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
    // Read SN
    uint32_t ubUI_SFAddr = pSF_Info->ulSize - (1 * pSF_Info->ulSecSize);
    SF_Read(ubUI_SFAddr, sizeof(RxSNdata), (uint8_t*)RxSNdata);

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
	if ((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1) == 0)
	&& (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1) == 0)) {
		memcpy(tUI_CamStatus, tUI_DevStsInfo.tBU_StatusInfo, (CAM_4T * sizeof(UI_BUStatus_t)));
		memcpy(&tUI_PuSetting, &tUI_DevStsInfo.tPU_SettingInfo, sizeof(UI_PUSetting_t));
	} else {
		printd(DBG_ErrorLvl, "TAG no match, Reset UI\n");
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.uwYear, 2018);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubMonth,   1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubDate,    1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubHour,    0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubMin,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.tSysCalendar.ubSec,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.RecInfo.tREC_Mode, REC_OFF);
		UI_CLEAR_CALENDAR_TODEFU(tUI_PuSetting.RecInfo.tREC_Time, RECTIME_10MIN);
		tUI_PuSetting.tVdoRecMode = UI_VDOPHOTO_MODE;
	}
	tUI_PuSetting.IconSts.ubShowLostLogoFlag = FALSE;
	
	if((tUI_PuSetting.ubFeatCode0 == 0x23)&&(tUI_PuSetting.ubFeatCode1== 0x45)&&(tUI_PuSetting.ubFeatCode2 == 0x67))
	{
		printd(Apk_DebugLvl, "tUI_PuSetting.ubDefualtFlag  %d \n",tUI_PuSetting.ubDefualtFlag);
	}
	else
	{
		printd(Apk_DebugLvl, "ubDefulat OK~~~\n");
			tUI_PuSetting.ubTotalBuNum 				 = DISPLAY_MODE;
			tUI_PuSetting.ubCamViewNum	  =    		CAM1;
			tUI_PuSetting.tAdoSrcCamNum				 = (tUI_PuSetting.tAdoSrcCamNum > CAM4)?CAM1:tUI_PuSetting.tAdoSrcCamNum;
			tUI_PuSetting.BriLvL.tBL_UpdateLvL		 = BL_LVL6; //BL_LVL5
			tUI_PuSetting.VolLvL.tVOL_UpdateLvL		 = VOL_LVL4; //VOL_LVL4
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

		        tUI_PuSetting.ubFeatCode0                = 0x23;
		        tUI_PuSetting.ubFeatCode1                = 0x45;
		        tUI_PuSetting.ubFeatCode2                = 0x67;

			tUI_PuSetting.ubScanTime 			= 2;
			tUI_PuSetting.ubHighTempSetting 		= 0;
			tUI_PuSetting.ubLowTempSetting 		= 0;
			tUI_PuSetting.ubTempAlertSetting 		= 1;
			tUI_PuSetting.ubSoundLevelSetting 		= 0;
			tUI_PuSetting.ubSoundAlertSetting 		= 1;	
			tUI_PuSetting.ubTempunitFlag 			= 0;
			tUI_PuSetting.ubSleepMode			= 1;
			tUI_PuSetting.ubZoomScale			= 0;
			tUI_PuSetting.ubFlickerFlag 			= 1;
			tUI_PuSetting.ubLangageFlag 			= 0;
			tUI_PuSetting.NightmodeFlag				= 0x0F;
			tUI_PuSetting.ubSquealWarnCnt			=0;
	}
		tUI_PuSetting.tPsMode = POWER_NORMAL_MODE;

			tUI_PuSetting.ubPairedBuNum = 0;
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
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_OFF);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_ON);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_ON);
					UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAecMode,  		CAMSET_OFF);
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

    if ((tUI_PuSetting.ubHighTempSetting == 0)&&(tUI_PuSetting.ubLowTempSetting == 0)
        &&(tUI_PuSetting.ubSoundLevelSetting == 0))
        ubAlarmIconFlag = 0;
    else
        ubAlarmIconFlag = 1;

    ubRealTemp = tUI_PuSetting.ubTempunitFlag?ubRealTemp:UI_TempCToF(ubRealTemp);
    printd(Apk_DebugLvl, "UI_LoadDevStatusInfo ubRealTemp: %d, ubTempunitFlag: %d.\n", ubRealTemp, tUI_PuSetting.ubTempunitFlag);
    UI_GetPairCamInfo();

    tPairInfo.ubDrawFlag = FALSE;

    tCamViewSel.tCamViewPool[0] = tUI_PuSetting.ubCamViewNum;
    printd(Apk_DebugLvl, "tCamViewSel.tCamViewNum  %d \n",tCamViewSel.tCamViewPool[0]);

    ubFactorySettingFLag = tUI_PuSetting.ubDefualtFlag;
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
    uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
    UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};

    tUI_PuSetting.ubCamViewNum = tCamViewSel.tCamViewPool[0];

    osMutexWait(APP_UpdateMutex, osWaitForever);
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
uint8_t UI_GetCamOnLineNum(uint8_t type)
{
    int i = 0;
    uint8_t ubCamOnlineNum = 0;

    for (i=0; i<4; i++)
    {
        if ((tUI_CamStatus[i].ulCAM_ID != INVALID_ID) && (tUI_CamStatus[i].tCamConnSts == CAM_ONLINE))
        {
            ubCamOnlineNum++;
        }
    }

    return ubCamOnlineNum;
}

void UI_SwitchCameraScan(uint8_t type)
{
    int i, j;
    UI_CamNum_t tCamSwtichNum = (UI_CamNum_t)0xFF;
    UI_CamNum_t tSearchCam = tCamViewSel.tCamViewPool[0];

    printd(Apk_DebugLvl, "UI_SwitchCameraScan type: %d.\n", type);
    for (i = 0; i < 4; i++)
        printd(Apk_DebugLvl, "### tUI_CamStatus[%d].ulCAM_ID: 0x%x, tUI_CamStatus[%d].tCamConnSts: %d.\n", i, tUI_CamStatus[i].ulCAM_ID, i, tUI_CamStatus[i].tCamConnSts);

    if (type == 0)
    {
        if (tUI_PuSetting.ubScanTime <= 0)
            return;
    }

    //if (ubPowerState != PWR_ON)   //20180803
    //  return;

    printd(Apk_DebugLvl, "UI_SwitchCameraScan ubCameraOnlineNum: %d, tCamViewPool[0]: %d.\n", ubCameraOnlineNum, tCamViewSel.tCamViewPool[0]);
#if 0
    if (ubCamOnlineNum < 2)
        return;
#else
    if (ubCameraOnlineNum == 0)
        return;

    if (ubCameraOnlineNum == 1)
    {
        for (i = 0; i < 4; i++)
        {
           if ((tUI_CamStatus[i].ulCAM_ID != INVALID_ID) && (tUI_CamStatus[i].tCamConnSts == CAM_ONLINE))
             {
                if (tCamViewSel.tCamViewPool[0] == i)
			{
                    		return;
                	}
                else
                {
                    tCamSwtichNum = i;
                    goto SWITCH_CAMERA;
                }
            }
        }
        return;
    }
#endif

    if (tSearchCam == 3)
        i = 0;
    else
        i = tSearchCam+1;

    for (j = 0; j < 4; j++)
    {
        if ((tUI_CamStatus[i].ulCAM_ID != INVALID_ID) && (tUI_CamStatus[i].tCamConnSts == CAM_ONLINE))
        //if ((tUI_CamStatus[i].ulCAM_ID != INVALID_ID))
      {
            tCamSwtichNum = i;
            break;
        }
        i++;
        if (i > 3) i = 0;
    }

SWITCH_CAMERA:	
	printd(Apk_DebugLvl, "UI_SwitchCameraScan tCamSwtichNum: 0x%x.\n", tCamSwtichNum);
	if(0xFF == tCamSwtichNum)
		return;

      //ubPickupAlarmState = PICKUP_ALARM_IDLE;
     //ubPickupAlarmCheckCount = 0;
      //ubPickupAlarmTriggerCount = 0;
     //ubShowAlarmstate  = 0;
	
	tCamViewSel.tCamViewType	= SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = tCamSwtichNum;
	tUI_PuSetting.tAdoSrcCamNum = tCamSwtichNum;
	ubSetViewCam = tCamViewSel.tCamViewPool[0];
	UI_SwitchCameraSource();
	UI_ClearBuConnectStatusFlag();
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_TimerDeviceEventStart(TIMER_DEVICE_t tDevice, uint32_t ulTime_ms, void *pvRegCb)
{
    TIMER_SETUP_t tUI_TimerParam;

    tUI_TimerParam.tCLK         = TIMER_CLK_EXTCLK;
    tUI_TimerParam.ulTmLoad     = 10000 * ulTime_ms;
    tUI_TimerParam.ulTmCounter  = tUI_TimerParam.ulTmLoad;
    tUI_TimerParam.ulTmMatch1   = tUI_TimerParam.ulTmLoad + 1;
    tUI_TimerParam.tOF          = TIMER_OF_ENABLE;
    tUI_TimerParam.tDIR         = TIMER_DOWN_CNT;
    tUI_TimerParam.tEM          = TIMER_CB;
    tUI_TimerParam.pvEvent      = pvRegCb;

    printd(1, "UI_TimerDeviceEventStart tDevice: %d, ulTime_ms: %d.\n", tDevice, ulTime_ms);
    TIMER_Start(tDevice, tUI_TimerParam);
    ubTimerDevEventStopSta = 1;
}
//------------------------------------------------------------------------------
void UI_TimerDeviceEventStop(TIMER_DEVICE_t tDevice)
{
   // printd(1, "UI_TimerDeviceEventStop tDevice: %d.\n", tDevice);
    if (ubTimerDevEventStopSta == 1)
    {
        TIMER_Stop(tDevice);
        ubTimerDevEventStopSta = 0;
    }
}
//------------------------------------------------------------------------------
void UI_TimerEventStart(uint32_t ulTime_ms, void *pvRegCb)
{
    TIMER_SETUP_t tUI_TimerParam;

    //printd(Apk_DebugLvl, "UI_TimerEventStart TIMER2_1.\n");
    tUI_TimerParam.tCLK         = TIMER_CLK_EXTCLK;
    tUI_TimerParam.ulTmLoad     = 10000 * ulTime_ms;
    tUI_TimerParam.ulTmCounter  = tUI_TimerParam.ulTmLoad;
    tUI_TimerParam.ulTmMatch1   = tUI_TimerParam.ulTmLoad + 1;
    tUI_TimerParam.tOF          = TIMER_OF_ENABLE;
    tUI_TimerParam.tDIR         = TIMER_DOWN_CNT;
    tUI_TimerParam.tEM          = TIMER_CB;
    tUI_TimerParam.pvEvent      = pvRegCb;
    TIMER_Start(TIMER2_1, tUI_TimerParam);
}
//------------------------------------------------------------------------------
void UI_TimerEventStop(void)
{
   // printd(1, "UI_TimerEventStop TIMER2_1.\n");
    TIMER_Stop(TIMER2_1);
}
//------------------------------------------------------------------------------
void UI_ScanModeTimerEvent(void)
{
    UI_Event_t tScanEvent;
    osMessageQId *pUI_ScanEventQH = NULL;

	UI_TimerEventStop();
	tScanEvent.tEventType = SCANMODE_EVENT;
	tScanEvent.pvEvent 	  = NULL;
	pUI_ScanEventQH 	  = pUI_GetEventQueueHandle();
   // osMessagePut(*pUI_ScanEventQH, &tScanEvent, osWaitForever);
	osMessagePut(*pUI_ScanEventQH, &tScanEvent, 0);
}
//------------------------------------------------------------------------------
void UI_SetupScanModeTimer(uint8_t ubTimerEn)
{
    //printd(1, "UI_SetupScanModeTimer ubTimerEn: %d, Time: %d.\n", ubTimerEn, ubCameraScanTime[tUI_PuSetting.ubScanTime]*1000);
	//osDelay(1);
    ubUI_ScanStartFlag = ubTimerEn;
    if (TRUE == ubTimerEn)
    {
        if ((tUI_PuSetting.ubScanTime > 0) && (tUI_PuSetting.ubScanTime < 5))
        {
            UI_TimerEventStart(ubCameraScanTime[tUI_PuSetting.ubScanTime]*1000, UI_ScanModeTimerEvent);
        }
        else
        {
            UI_TimerEventStop();
        }
    }
    else
    {
        UI_TimerEventStop();
    }
}
//------------------------------------------------------------------------------
void UI_EnableScanMode(void)
{
    printd(Apk_DebugLvl, "UI_EnableScanMode ubScanTime: %d.\n", tUI_PuSetting.ubScanTime);

    if (tUI_PuSetting.ubScanTime == 0)
    {
        UI_DisableScanMode();
        return;
    }

    //if (ubPowerState != PWR_ON)   //20180803
    //  return;

    if (TRUE == ubUI_PttStartFlag)
        return;

    if (ubShowAlarmstate > 0)
        return;

    UI_CheckCameraSource4SV();
    UI_SetupScanModeTimer(TRUE);
}
//------------------------------------------------------------------------------
void UI_DisableScanMode(void)
{
    //printd(Apk_DebugLvl, "UI_DisableScanMode ubUI_ScanStartFlag: %d #\n", ubUI_ScanStartFlag);
    if (FALSE == ubUI_ScanStartFlag)
        return;
    UI_SetupScanModeTimer(FALSE);
}
//------------------------------------------------------------------------------
void UI_ScanModeExec(void)
{
    printd(Apk_DebugLvl, "UI_ScanModeExec###\n");
#if 0
    UI_CamNum_t tSearchCam = tCamViewSel.tCamViewPool[0];
    uint8_t ubSearchCnt;

    printd(Apk_DebugLvl, "UI_ScanModeExec tSearchCam: %d.\n", tSearchCam);
    for (ubSearchCnt = 0; ubSearchCnt < tUI_PuSetting.ubTotalBuNum; ubSearchCnt++)
    {
        tSearchCam = ((tSearchCam + 1) >= CAM_4T)?CAM1:((UI_CamNum_t)(tSearchCam + 1));
        if ((tUI_CamStatus[tSearchCam].ulCAM_ID != INVALID_ID) &&
           (tUI_CamStatus[tSearchCam].tCamConnSts == CAM_ONLINE) &&
           (tSearchCam != tCamViewSel.tCamViewPool[0]))
        {
            tCamViewSel.tCamViewType    = SINGLE_VIEW;
            tCamViewSel.tCamViewPool[0] = tSearchCam;
            tUI_PuSetting.tAdoSrcCamNum = tSearchCam;
            UI_SwitchCameraSource();
            UI_ClearBuConnectStatusFlag();

            ubSetViewCam = tCamViewSel.tCamViewPool[0];

            break;
        }
    }
    UI_SetupScanModeTimer(TRUE);
#else
    printd(Apk_DebugLvl,"UI_ScanModeExec\n");
     UI_SetupScanModeTimer(FALSE);	
     UI_SwitchCameraScan(0);
     UI_SetupScanModeTimer(TRUE);
#endif
}

//------------------------------------------------------------------------------
UI_Result_t UI_CheckCameraSource4SV(void)
{
    UI_CamNum_t tCamViewNum;

    if ((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
       (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
        return rUI_SUCCESS;

    for (tCamViewNum = CAM1; tCamViewNum < tUI_PuSetting.ubTotalBuNum; tCamViewNum++)
    {
        if ((tUI_CamStatus[tCamViewNum].ulCAM_ID != INVALID_ID) &&
           (tUI_CamStatus[tCamViewNum].tCamConnSts == CAM_ONLINE))
        {
            tCamViewSel.tCamViewType    = SINGLE_VIEW;
            tCamViewSel.tCamViewPool[0]     = tCamViewNum;
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

    printd(Apk_DebugLvl, "UI_SwitchCameraSource tCamViewPool[0]: %d ###\n", tCamViewSel.tCamViewPool[0]);
    tUI_SwitchBuMsg.ubAPP_Event      = APP_VIEWTYPECHG_EVENT;
    tUI_SwitchBuMsg.ubAPP_Message[0] = 3;       //! Message Length
    tUI_SwitchBuMsg.ubAPP_Message[1] = tCamViewSel.tCamViewType;
    tUI_SwitchBuMsg.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
    tUI_SwitchBuMsg.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
    UI_SendMessageToAPP(&tUI_SwitchBuMsg);

    if ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW)) {
        //if (tUI_PuSetting.tAdoSrcCamNum != tCamViewSel.tCamViewPool[0])
            //UI_UpdateDevStatusInfo();
        tUI_PuSetting.tAdoSrcCamNum = tCamViewSel.tCamViewPool[0];
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

    if (UI_DISPLAY_STATE != tUI_State)
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
    for (i = 0; i < 10; i++)
        uwUI_NumArray[i] = OSD2IMG_ENG_NUM0 + i;
    tUI_EngOsdImg.pNumImgIdxArray = uwUI_NumArray;
    for (i = 0; i < 26; i++)
    {
        uwUI_UpperLetterArray[i] = OSD2IMG_ENG_UPA + i;
        uwUI_LowerLetterArray[i] = OSD2IMG_ENG_LWA + i;
    }
    tUI_EngOsdImg.pUpperLetterImgIdxArray = uwUI_UpperLetterArray;
    tUI_EngOsdImg.pLowerLetterImgIdxArray = uwUI_LowerLetterArray;
    for (i = 0; i < 4; i++)
        uwUI_SymbolArray[i] = OSD2IMG_ENG_COLONSYM + i;
    tUI_EngOsdImg.pSymbolImgIdxArray = uwUI_SymbolArray;

    tOSD_GetOsdImgInfor (1, OSD_IMG2, OSD2IMG_MENU_BLANKBAR, 1, &tOsdImgInfo);
    tOsdImgInfo.uwXStart = 0;
    tOsdImgInfo.uwYStart = 0;
    tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

    EN_SetupOsdImgInfo(&tUI_EngOsdImg);
    EN_OpenEnMode(TRUE);
    tUI_State = UI_ENGMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey)
{
    pvUiFuncPtr UI_EngFuncPrt[] = {[UP_ARROW]    = EN_UpKey,
                                   [DOWN_ARROW]  = EN_DownKey,
                                   NULL, NULL,
                                   [ENTER_ARROW] = EN_EnterKey,
                                   NULL};
    switch (tArrowKey)
    {
    case EXIT_ARROW:
        EN_OpenEnMode(FALSE);
        UI_ClearBuConnectStatusFlag();
        tUI_State = UI_DISPLAY_STATE;
        break;
    default:
        if (UI_EngFuncPrt[tArrowKey])
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

void UI_SetSpeaker(uint8_t type, uint8_t State)
{
    if (type == 0)
    {
        if (State == 0)
        {
            if (SPEAKER_STATE == TRUE)
                SPEAKER_EN(FALSE);
            ubSpeakerCount = 0;
        }
        else
        {
            if (FALSE == ubUI_PttStartFlag)
            {
                if (ubSpeakerCount == 0)
                {
                    if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL > VOL_LVL0)
                    {
                        if (APP_LINK_STATE == tUI_SyncAppState)
                            ubSpeakerCount = 1;
                    }
                }
            }
        }
    }
    else if (type == 1)
    {
        if (State == 1)
        {
            if (tUI_PuSetting.VolLvL.tVOL_UpdateLvL > VOL_LVL0)
            {
                if (ubSpeakerCount > 0)
                {
                    if (FALSE == ubUI_PttStartFlag)
                    {
                        if (SPEAKER_STATE == FALSE)
                        {
                            SPEAKER_EN(TRUE);
                            TIMER_Delay_us(2);
                            SPEAKER_EN(FALSE);
                            TIMER_Delay_us(2);
                            SPEAKER_EN(TRUE);
                        }
                    }
                    ubSpeakerCount = 0;
                }
            }
        }
        else
        {
            if (SPEAKER_STATE == TRUE)
                SPEAKER_EN(FALSE);
            ubSpeakerCount = 0;
        }
    }
    else if (type == 2)
    {
        if (State == 0)
        {
            if (SPEAKER_STATE == TRUE)
                SPEAKER_EN(FALSE);
            ubSpeakerCount = 0;
        }
        else
        {
            if (FALSE == ubUI_PttStartFlag)
            {
                if (FALSE == ubUI_PttStartFlag)
                {
                    if (SPEAKER_STATE == FALSE)
                    {
                        SPEAKER_EN(TRUE);
                        TIMER_Delay_us(2);
                        SPEAKER_EN(FALSE);
                        TIMER_Delay_us(2);
                        SPEAKER_EN(TRUE);
                    }
                }
                ubSpeakerCount = 0;
            }
        }
    }
}

void UI_SetFactoryFlag(uint8_t Value)
{
    tUI_PuSetting.ubDefualtFlag = Value;
    UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_FwUpgViaSdCard(void)
{

}
//------------------------------------------------------------------------------
void UI_FwUpgExecSel(UI_ArrowKey_t tArrowKey)
{
	tArrowKey = tArrowKey;
}
//------------------------------------------------------------------------------
void UI_UpdateRecStsIcon(void)
{
	if(!tUI_PuSetting.ubVdoRecStsCnt)
		return;
	if(!(--tUI_PuSetting.ubVdoRecStsCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearBuConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
	}
}
//------------------------------------------------------------------------------
void UI_PhotoCaptureFinish(uint8_t ubPhotoCapRet)
{
	ubPhotoCapRet = ubPhotoCapRet;
}
//------------------------------------------------------------------------------
void UI_PhotoPlayFinish(uint8_t ubPhtoCapRet)
{
	ubPhtoCapRet = ubPhtoCapRet;
}
