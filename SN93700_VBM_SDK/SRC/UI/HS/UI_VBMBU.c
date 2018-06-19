/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_VBMBU.c
	\brief		User Interface of VBM Baby Unit (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.3
	\date		2018/05/21
	\copyright	Copyright (C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include "UI_VBMBU.h"
#include "SEN.h"
#include "RTC_API.h"
#include "ISP_API.h"
#include "MD_API.h"
#include "SF_API.h"
#include "Buzzer.h"
#include "FWU_API.h"
#include "MC_API.h"
#include "i2c.h"
#include "IQ_API.h"
#include "WDT.h"

#define osUI_SIGNALS	0x6A

#define MC_ENABLE  1

/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
UI_KeyEventMap_t UiKeyEventMap[] =
{
	{NULL,				0,					NULL},
	{GKEY_ID0, 			0,					UI_PairingKey},
	{GKEY_ID0, 			50,					UI_PairingLongKey},	
	{PKEY_ID0, 			0,					UI_PairingKey},
	{PKEY_ID0, 			20,					UI_PowerKey},
};
UI_SettingFuncPtr_t tUiSettingMap2Func[] =
{
	[UI_PTZ_SETTING] 			= NULL,	
	[UI_RECMODE_SETTING] 		= NULL,
	[UI_RECRES_SETTING] 		= NULL,
	[UI_SDCARD_SETTING] 		= NULL,
	[UI_PHOTOMODE_SETTING] 		= NULL,
	[UI_PHOTORES_SETTING] 		= NULL,
	[UI_SYSINFO_SETTING] 		= NULL,
	[UI_VOXMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_ECOMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_WORMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_ADOANR_SETTING]			= UI_ANRSetting,
	[UI_IMGPROC_SETTING]		= UI_ImageProcSetting,
	[UI_MD_SETTING]				= UI_MDSetting,
	[UI_VOICETRIG_SETTING]		= UI_VoiceTrigSetting,
	[UI_MOTOR_SETTING]		    = UI_PtzControlSetting,
	[UI_TEST_SETTING]		    = UI_TestSetting,
};

//ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n36DB, R2R_VOL_n23p5DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};
//ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};
ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n39p1DB, R2R_VOL_n36DB, R2R_VOL_n29p8DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n11p9DB, R2R_VOL_n5p6DB, R2R_VOL_n0DB};

static UI_BUStatus_t tUI_BuStsInfo;
static APP_State_t tUI_SyncAppState;
static UI_ThreadNotify_t tosUI_Notify;
static uint8_t ubUI_ClearThdCntFlag;
static uint8_t ubUI_WorModeEnFlag;
static uint8_t ubUI_WorWakeUpCnt;
static uint8_t ubUI_SyncDisVoxFlag;
osMutexId UI_BUMutex;

static uint8_t ubUI_Mc1RunFlag;
static uint8_t ubUI_Mc2RunFlag;
static uint8_t ubUI_Mc3RunFlag;
static uint8_t ubUI_Mc4RunFlag;
static uint8_t ubUI_Mc1RunCnt;
static uint8_t ubUI_Mc2RunCnt;
static uint16_t ubUI_McHandshake;
static uint16_t ubUI_McPreHandshake;
static uint8_t ubMcHandshakeLost = 0;


uint8_t ubVoicetemp_bak = 0xff;
uint8_t ubTemp_bak = 25;

I2C1_Type *pTempI2C;

uint8_t ubTestMode = 1;

//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;

	KEY_Event_t *ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt))
		{
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
		}
		ubUI_KeyEventIdx = 0;
		return;
	}
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{
			ubUI_KeyEventIdx = uwIdx;
			if((ptKeyEvent->uwKeyCnt != 0) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr))
			{
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				ubUI_KeyEventIdx = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osMutexDef(UI_BUMutex);
	UI_BUMutex = osMutexCreate(osMutex(UI_BUMutex));
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvPUResponse, UI_RecvPURequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI setting 2way command fail !\n");
	ubUI_ClearThdCntFlag = FALSE;
	ubUI_SyncDisVoxFlag  = FALSE;
	UI_LoadDevStatusInfo();
	ubUI_WorModeEnFlag	 = (PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)?TRUE:FALSE;
	ubUI_WorWakeUpCnt 	 = 0;
	tUI_BuStsInfo.tCamScanMode = CAMSET_OFF;
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	static uint8_t ubUI_BuSysSetFlag = FALSE;

	osMutexWait(UI_BUMutex, osWaitForever);
	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];

			if(rUI_SUCCESS == tPair_Result)
				UI_ResetDevSetting();
			//PAIRING_LED_IO = 0;
			break;
		}
		case APP_LINKSTS_RPT:
			break;
		default:
			break;
	}
	ubUI_ClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	if((FALSE == ubUI_BuSysSetFlag) && (APP_IDLE_STATE == tUI_SyncAppState))
	{
		UI_SystemSetup();
		ubUI_BuSysSetFlag = TRUE;
	}
	osMutexRelease(UI_BUMutex);
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};

	osMutexWait(UI_BUMutex, osWaitForever);
	UI_CLEAR_THREADCNT(ubUI_ClearThdCntFlag, *pThreadCnt);
	UI_MCStateCheck(); //20180529
	switch(tUI_SyncAppState)
	{
		case APP_LINK_STATE:
			if(TRUE == ubUI_WorModeEnFlag)
				UI_ChangePsModeToNormalMode();
			if(PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode)
				UI_VoxTrigger();
			if(CAMSET_ON == tUI_BuStsInfo.tCamScanMode)
				UI_VoiceTrigger();
			//if(MD_ON == tUI_BuStsInfo.MdParam.ubMD_Mode)
				//UI_MDTrigger();

			if(((*pThreadCnt)%5) == 0)
			{
				//UI_TestCheck(); //20180517
			}
			
			if(((*pThreadCnt)%10) == 0)
			{
				UI_VoiceCheck();
				//UI_BrightnessCheck();
			}
			
			if(((*pThreadCnt)%10) == 0)
			{
				UI_TempCheck();
			}
			
			if((*pThreadCnt % UI_UPDATESTS_PERIOD) != 0)
				UI_UpdateBUStatusToPU();
			(*pThreadCnt)++;
			break;
		case APP_LOSTLINK_STATE:
			if(PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)
			{
				if(FALSE == ubUI_WorModeEnFlag)
					UI_ChangePsModeToWorMode();
				if(!ubUI_WorWakeUpCnt)
					UI_VoiceTrigger();
				else if(++ubUI_WorWakeUpCnt > (6000 / UI_TASK_PERIOD))
					UI_PowerSaveSetting(&tUI_BuStsInfo.tCamPsMode);
			}
			break;
		case APP_PAIRING_STATE:
			//if((*pThreadCnt % UI_PAIRINGLED_PERIOD) == 0)
				//PAIRING_LED_IO = ~PAIRING_LED_IO;
			(*pThreadCnt)++;
			osMutexRelease(UI_BUMutex);
			return;
		default:
			break;
	}
	//PAIRING_LED_IO = 0;
	tUI_GetLinkStsMsg.ubAPP_Event = APP_LINKSTATUS_REPORT_EVENT;
	UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	osMutexRelease(UI_BUMutex);
}
//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
		case GKEY_EVENT:			
			UI_KeyEventExec(ptEventPtr->pvEvent);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	//POWER_LED_IO   = 0;
	//PAIRING_LED_IO = 0;
	//SIGNAL_LED_IO  = 0;
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	printd(DBG_Debug1Lvl, "Power OFF !\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------
void UI_PairingKey(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};
	printf("UI_PairingKey###\n");
	tUI_PairMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_PAIRING_START_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
	BUZ_PlaySingleSound();
}

void UI_PairingLongKey(void)
{
	/*
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_PAIRING_START_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
	*/
	printf("UI_PairingLongKey###\n");
	BUZ_PlaySingleSound();
}
//------------------------------------------------------------------------------
void UI_UpdateBUStatusToPU(void)
{
//	UI_BUReqCmd_t tUI_BuSts;

//	tUI_BuSts.ubCmd[UI_TWC_TYPE]		= UI_REPORT;
//	tUI_BuSts.ubCmd[UI_REPORT_ITEM] 	= UI_UPDATE_BUSTS;
//	tUI_BuSts.ubCmd[UI_REPORT_DATA] 	= 100;
//	tUI_BuSts.ubCmd_Len  				= 3;
//	UI_SendRequestToPU(NULL, &tUI_BuSts);
}
//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToPU(osThreadId thread_id, UI_BUReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	uint8_t ubUI_TwcRetry = 5;

	while(--ubUI_TwcRetry)
	{
		if(tTWC_Send(TWC_AP_MASTER, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
			break;
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(TWC_AP_MASTER, TWC_UI_SETTING);
		return rUI_FAIL;
	}
	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		tReq_Result = (tReq_Event.status == osEventSignal)?(tReq_Event.value.signals == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(TWC_AP_MASTER, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	return tReq_Result;
}
//------------------------------------------------------------------------------
void UI_RecvPUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus)
{
//	UI_CamNum_t tCamNum;
//	TWC_TAG tTWC_StaNum;

//	APP_KNLRoleMap2CamNum(ubKNL_GetRole(), tCamNum);
//	tTWC_StaNum = APP_GetSTANumMappingTable(tCamNum)->tTWC_StaNum;
//	if((tRecv_StaNum != tTWC_StaNum) || (NULL == tosUI_Notify.thread_id))
//		return;
	if((tRecv_StaNum != TWC_AP_MASTER) || (NULL == tosUI_Notify.thread_id))
		return;
	tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
	if(osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
		printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
//------------------------------------------------------------------------------
void UI_RecvPURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
//	UI_CamNum_t tCamNum;
//	TWC_TAG tTWC_StaNum;

//	APP_KNLRoleMap2CamNum(ubKNL_GetRole(), tCamNum);
//	tTWC_StaNum = APP_GetSTANumMappingTable(tCamNum)->tTWC_StaNum;
//	if(tRecv_StaNum != tTWC_StaNum)
//		return;
	if(tRecv_StaNum != TWC_AP_MASTER)
		return;
	switch(pTwc_Data[UI_TWC_TYPE])
	{
		case UI_SETTING:
			if(tUiSettingMap2Func[pTwc_Data[UI_SETTING_ITEM]].pvAction)
				tUiSettingMap2Func[pTwc_Data[UI_SETTING_ITEM]].pvAction((uint8_t *)(&pTwc_Data[UI_SETTING_DATA]));
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SetMotionEvent (uint8_t ubReport)
{       
	if(ubReport)
	{
		UI_BUReqCmd_t tUI_MdMsg;

		tUI_MdMsg.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
		tUI_MdMsg.ubCmd[UI_REPORT_ITEM] = UI_MD_TRIG;
		tUI_MdMsg.ubCmd[UI_REPORT_DATA] = TRUE;
		tUI_MdMsg.ubCmd_Len  			= 3;
		UI_SendRequestToPU(NULL, &tUI_MdMsg);
	}
}
//------------------------------------------------------------------------------
void UI_SystemSetup(void)
{
	UI_IspSetup();
	ADO_Noise_Process_Type((CAMSET_ON == tUI_BuStsInfo.tCamAnrMode)?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
	UI_MDSetting(&tUI_BuStsInfo.MdParam.ubMD_Param[0]);
	UI_VoiceTrigSetting(&tUI_BuStsInfo.tCamScanMode);
	if(PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode)
		ubUI_SyncDisVoxFlag = TRUE;
	else
		UI_PowerSaveSetting(&tUI_BuStsInfo.tCamPsMode);
    MD_ReportReadyCbFunc(UI_SetMotionEvent);
    MD_SetMdState(MD_UNSTABLE);
}
#define ADC_SUMRPT_VOX_THL			5000
#define ADC_SUMRPT_VOX_THH			5000
#define ADC_SUMRPT_VOICETRIG_THL	7000
#define ADC_SUMRPT_VOICETRIG_THH	7000
//------------------------------------------------------------------------------
void UI_PowerSaveSetting(void *pvPS_Mode)
{
	APP_EventMsg_t tUI_PsMessage = {0};
	UI_PowerSaveMode_t *pPS_Mode = (UI_PowerSaveMode_t *)pvPS_Mode;

	switch(pPS_Mode[0])
	{
		case PS_VOX_MODE:
			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = pPS_Mode[0];
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			if(CAMSET_ON == tUI_BuStsInfo.tCamScanMode)
			{
				ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
				tUI_BuStsInfo.tCamScanMode = CAMSET_OFF;
			}
			ADO_SetAdcRpt(ADC_SUMRPT_VOX_THL, ADC_SUMRPT_VOX_THH, ADO_ON);
			tUI_BuStsInfo.tCamPsMode = PS_VOX_MODE;
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "		=> VOX Mode Enable\n");
			break;
		case PS_ECO_MODE:
			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = pPS_Mode[0];
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			tUI_BuStsInfo.tCamPsMode = PS_ECO_MODE;
			break;
		case PS_WOR_MODE:
			ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_ON);
			if(PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)
				break;
			tUI_BuStsInfo.tCamPsMode   = PS_WOR_MODE;
			tUI_BuStsInfo.tCamScanMode = CAMSET_OFF;
			UI_UpdateDevStatusInfo();
			ubUI_WorModeEnFlag = FALSE;
			ubUI_WorWakeUpCnt  = 0;
			printd(DBG_InfoLvl, "		=> WOR Mode\n");
			break;
		case POWER_NORMAL_MODE:
			if(PS_VOX_MODE == tUI_BuStsInfo.tCamPsMode)
				UI_DisableVox();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ChangePsModeToWorMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	tUI_PsMessage.ubAPP_Message[3] = FALSE;
	tUI_PsMessage.ubAPP_Message[4] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	ubUI_WorModeEnFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ChangePsModeToNormalMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	tUI_PsMessage.ubAPP_Message[3] = FALSE;
	tUI_PsMessage.ubAPP_Message[4] = (!ubUI_WorWakeUpCnt)?TRUE:FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
	tUI_BuStsInfo.tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
	ubUI_WorModeEnFlag = FALSE;
	ubUI_WorWakeUpCnt  = 0;
	printd(DBG_InfoLvl, "		=> WOR Disable\n");
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
	APP_EventMsg_t tUI_VoxMsg = {0};

	ADO_SetAdcRpt(ADC_SUMRPT_VOX_THL, ADC_SUMRPT_VOX_THH, ADO_OFF);
	tUI_VoxMsg.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_VoxMsg.ubAPP_Message[0] = 2;		//! Message Length
	tUI_VoxMsg.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_VoxMsg.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_VoxMsg);
	tUI_BuStsInfo.tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
//	tUI_BuStsInfo.tCamScanMode = CAMSET_ON;
//	UI_VoiceTrigSetting(&tUI_BuStsInfo.tCamScanMode);
	printd(DBG_InfoLvl, "		=> VOX Mode Disable\n");
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(void)
{
	UI_BUReqCmd_t tUI_VoxReqCmd;
	uint32_t ulUI_AdcRpt = 0;

	tUI_VoxReqCmd.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
	tUI_VoxReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOX_TRIG;
	tUI_VoxReqCmd.ubCmd[UI_REPORT_DATA] = FALSE;
	tUI_VoxReqCmd.ubCmd_Len  			= 3;
	if(TRUE == ubUI_SyncDisVoxFlag)
	{
		UI_SendRequestToPU(NULL, &tUI_VoxReqCmd);
		tUI_BuStsInfo.tCamPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
		ubUI_SyncDisVoxFlag = FALSE;
		return;
	}
	ulUI_AdcRpt = ulADO_GetAdcSumHigh();
	if(ulUI_AdcRpt > ADC_SUMRPT_VOX_THH)
	{
		UI_SendRequestToPU(NULL, &tUI_VoxReqCmd);
		UI_DisableVox();
	}
}
//------------------------------------------------------------------------------
void UI_VoiceTrigSetting(void *pvTrigMode)
{
	UI_CamsSetMode_t *pVoiceTrigMode = (UI_CamsSetMode_t *)pvTrigMode;

	tUI_BuStsInfo.tCamScanMode = *pVoiceTrigMode;
	ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, (CAMSET_ON == tUI_BuStsInfo.tCamScanMode)?ADO_ON:ADO_OFF);
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(void)
{
	UI_BUReqCmd_t tUI_VoiceReqCmd;
	uint32_t ulUI_AdcRpt = 0;

	ulUI_AdcRpt = ulADO_GetAdcSumHigh();
	if(PS_WOR_MODE == tUI_BuStsInfo.tCamPsMode)
	{
		if(ulUI_AdcRpt > ADC_SUMRPT_VOICETRIG_THH)
		{
			APP_EventMsg_t tUI_PsMessage = {0};

			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			tUI_PsMessage.ubAPP_Message[3] = TRUE;
			tUI_PsMessage.ubAPP_Message[4] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
			ubUI_WorWakeUpCnt++;
			printd(DBG_InfoLvl, "		=> Voice Trigger\n");
		}
	}
	else if(CAMSET_ON == tUI_BuStsInfo.tCamScanMode)
	{
		if(ulUI_AdcRpt > ADC_SUMRPT_VOICETRIG_THH)
		{
			tUI_VoiceReqCmd.ubCmd[UI_TWC_TYPE]	  = UI_REPORT;
			tUI_VoiceReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOICE_TRIG;
			tUI_VoiceReqCmd.ubCmd[UI_REPORT_DATA] = TRUE;
			tUI_VoiceReqCmd.ubCmd_Len  			  = 3;
			UI_SendRequestToPU(NULL, &tUI_VoiceReqCmd);
			printd(DBG_InfoLvl, "		=> Voice Trigger\n");
		}
	}
}	
//------------------------------------------------------------------------------
void UI_VoiceCheck (void)
{
	UI_BUReqCmd_t tUI_VoiceReqCmd;
	uint32_t ulUI_AdcRpt = 0;
	uint8_t voice_temp,  ir_temp1,ir_temp2;

	uint16_t uwDetLvl = 0x3FF;
	uwDetLvl = uwSADC_GetReport(1);

	ADO_SetAdcRpt(128, 256, ADO_ON);
	ulUI_AdcRpt = ulADO_GetAdcSumHigh();

	printf("ulUI_AdcRpt  0x%lx , uwDetLvl %x \n",ulUI_AdcRpt,uwDetLvl);	

	if(ulUI_AdcRpt > 0x6000)
		voice_temp = 5;
	else if(ulUI_AdcRpt > 0x5500)
		voice_temp = 4;	
	else if(ulUI_AdcRpt > 0x3500)
		voice_temp = 3;	
	else if(ulUI_AdcRpt > 0x1500)
		voice_temp = 2;	
	else if(ulUI_AdcRpt > 0x0700)
		voice_temp = 1;	
	else
		voice_temp = 0;	

	ir_temp1 = uwDetLvl >>8;
	ir_temp2 = uwDetLvl & 0xff;

	//if(ubVoicetemp_bak != voice_temp)
	//{
	//	printf("voice_temp: %d \n",voice_temp);
	
		tUI_VoiceReqCmd.ubCmd[UI_TWC_TYPE]	  = UI_REPORT;
		tUI_VoiceReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOICE_CHECK;
		tUI_VoiceReqCmd.ubCmd[UI_REPORT_DATA] = voice_temp;
		tUI_VoiceReqCmd.ubCmd[UI_REPORT_DATA+1] = ir_temp1;
		tUI_VoiceReqCmd.ubCmd[UI_REPORT_DATA+2] = ir_temp2;		
		tUI_VoiceReqCmd.ubCmd_Len  			  = 5;
		UI_SendRequestToPU(NULL, &tUI_VoiceReqCmd);

	//	ubVoicetemp_bak = voice_temp;
	//}
	
}
//------------------------------------------------------------------------------
void UI_TempCheck(void) //20180322
{
	uint8_t cur_temp;
	UI_BUReqCmd_t tUI_TempReqCmd;

	//I2C1_Type *pI2C;
	uint8_t   ubData[4] = {0};
	uint8_t   ubReg = 0xE3;
	bool ret = 0;
	uint32_t tem = 0;
	 
	//pI2C = pI2C_MasterInit (I2C_1, I2C_SCL_100K);
	ret = bI2C_MasterProcess (pTempI2C,  0x40, &ubReg, 1, ubData, 2);

	tem = (17572*(ubData[0]*256+ubData[1])/65536-4685)/100;

	cur_temp = tem;
	
	//if(ubTemp_bak != cur_temp)
	{
		tUI_TempReqCmd.ubCmd[UI_TWC_TYPE]	  = UI_REPORT;
		tUI_TempReqCmd.ubCmd[UI_REPORT_ITEM] = UI_TEMP_CHECK;
		tUI_TempReqCmd.ubCmd[UI_REPORT_DATA] = cur_temp;
		tUI_TempReqCmd.ubCmd_Len  			  = 3;
		UI_SendRequestToPU(NULL, &tUI_TempReqCmd);

		ubTemp_bak = cur_temp;
	}
}
//------------------------------------------------------------------------------
void UI_ANRSetting(void *pvAnrMode)
{
	uint8_t *pUI_AnrMode = (uint8_t *)pvAnrMode;

	tUI_BuStsInfo.tCamAnrMode = (UI_CamsSetMode_t)pUI_AnrMode[0];
	ADO_Noise_Process_Type((CAMSET_ON == tUI_BuStsInfo.tCamAnrMode)?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
	UI_UpdateDevStatusInfo();
	printd(DBG_InfoLvl, "		=> ANR %s\n", (CAMSET_ON == tUI_BuStsInfo.tCamAnrMode)?"ON":"OFF");
}
//------------------------------------------------------------------------------
void UI_IspSetup(void)
{
	UI_IspSettingFuncPtr_t tUI_IspFunc[UI_IMGSETTING_MAX] = 
	{
		[UI_IMG3DNR_SETTING] 		= {ISP_NR3DSwitch, (uint8_t *)&tUI_BuStsInfo.tCam3DNRMode},
		[UI_IMGvLDC_SETTING] 		= {ISP_VLDCSwitch, (uint8_t *)&tUI_BuStsInfo.tCamvLDCMode},
		[UI_IMGWDR_SETTING] 		= {ISP_DRCSwitch,  (uint8_t *)&tUI_BuStsInfo.tCamWdrMode},
		[UI_IMGDIS_SETTING] 		= {NULL, NULL},
		[UI_IMGCBR_SETTING] 		= {NULL, NULL},
		[UI_IMGCONDENSE_SETTING] 	= {NULL, NULL},
		[UI_FLICKER_SETTING] 		= {ISP_SetAePwrFreq, 	(uint8_t *)&tUI_BuStsInfo.tCamFlicker},
		[UI_IMGBL_SETTING] 			= {ISP_SetIQBrightness, (uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorBL},
		[UI_IMGCONTRAST_SETTING] 	= {ISP_SetIQContrast, 	(uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorContrast},
		[UI_IMGSATURATION_SETTING] 	= {ISP_SetIQSaturation, (uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorSaturation},
		[UI_IMGHUE_SETTING]			= {ISP_SetIQChroma, 	(uint8_t *)&tUI_BuStsInfo.tCamColorParam.ubColorHue},
	};
	uint8_t ubUI_IspItem, ubUI_IspParam;

	for(ubUI_IspItem = UI_IMG3DNR_SETTING; ubUI_IspItem < UI_IMGSETTING_MAX; ubUI_IspItem++)
	{
		if(tUI_IspFunc[ubUI_IspItem].pvImgFunc)
		{
			ubUI_IspParam = (*tUI_IspFunc[ubUI_IspItem].pImgParam) * (((UI_IMGBL_SETTING        == ubUI_IspItem) ||
																	  (UI_IMGCONTRAST_SETTING   == ubUI_IspItem) ||
														              (UI_IMGSATURATION_SETTING == ubUI_IspItem) ||
															          (UI_IMGHUE_SETTING 		== ubUI_IspItem))?2:1);
			tUI_IspFunc[ubUI_IspItem].pvImgFunc(ubUI_IspParam);
		}
	}
}
//------------------------------------------------------------------------------
void UI_ImageProcSetting(void *pvImgProc)
{
	uint8_t *pUI_ImgProc = (uint8_t *)pvImgProc;

	switch(pUI_ImgProc[0])
	{
		case UI_IMG3DNR_SETTING:
			tUI_BuStsInfo.tCam3DNRMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_NR3DSwitch(tUI_BuStsInfo.tCam3DNRMode);
			break;
		case UI_IMGvLDC_SETTING:
			tUI_BuStsInfo.tCamvLDCMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_VLDCSwitch(tUI_BuStsInfo.tCamvLDCMode);
			break;
		case UI_IMGWDR_SETTING:
			tUI_BuStsInfo.tCamWdrMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_DRCSwitch(tUI_BuStsInfo.tCamWdrMode);
			break;
		case UI_IMGDIS_SETTING:
			tUI_BuStsInfo.tCamDisMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_IMGCBR_SETTING:
			tUI_BuStsInfo.tCamCbrMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_IMGCONDENSE_SETTING:
			tUI_BuStsInfo.tCamCondenseMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_FLICKER_SETTING:
			tUI_BuStsInfo.tCamFlicker = (UI_CamFlicker_t)pUI_ImgProc[1];
			ISP_SetAePwrFreq((CAMFLICKER_50HZ == tUI_BuStsInfo.tCamFlicker)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);
			printd(DBG_InfoLvl, "		=> Flicker: %s\n", (CAMFLICKER_50HZ == tUI_BuStsInfo.tCamFlicker)?"50Hz":"60Hz");
			break;
		case UI_IMGBL_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorBL = pUI_ImgProc[1];
			ISP_SetIQBrightness((tUI_BuStsInfo.tCamColorParam.ubColorBL*2));
			break;
		case UI_IMGCONTRAST_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorContrast = pUI_ImgProc[1];
			ISP_SetIQContrast((tUI_BuStsInfo.tCamColorParam.ubColorContrast*2));
			break;
		case UI_IMGSATURATION_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorSaturation = pUI_ImgProc[1];
			ISP_SetIQSaturation((tUI_BuStsInfo.tCamColorParam.ubColorSaturation*2));
			break;
		case UI_IMGHUE_SETTING:
			tUI_BuStsInfo.tCamColorParam.ubColorHue = pUI_ImgProc[1];
			ISP_SetIQChroma(tUI_BuStsInfo.tCamColorParam.ubColorHue*2);
			break;
		default:
			return;
	}
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
#define MD_TRIG_LVL	16
void UI_MDTrigger(void)
{
	uint32_t ulUI_MdTrig = 0;
    uint32_t ulUI_MdTrig_LV = 0;
    
	ulUI_MdTrig = uwMD_GetCnt(MD_REG1_CNT_01);    
	printd(DBG_InfoLvl, "=>MD Trig1: %d\n", ulUI_MdTrig);
   	//ulUI_MdTrig_LV = (((tUI_BuStsInfo.MdParam.ubMD_Param[2]+1) * (tUI_BuStsInfo.MdParam.ubMD_Param[3]+1)) < 30)? 
   	//     ((tUI_BuStsInfo.MdParam.ubMD_Param[2]+1) * (tUI_BuStsInfo.MdParam.ubMD_Param[3]+1) * MD_TRIG_LVL) : (30 * MD_TRIG_LVL);
    
	if(ulUI_MdTrig > ulUI_MdTrig_LV)
	{
		UI_BUReqCmd_t tUI_MdMsg;

		tUI_MdMsg.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
		tUI_MdMsg.ubCmd[UI_REPORT_ITEM] = UI_MD_TRIG;
		tUI_MdMsg.ubCmd[UI_REPORT_DATA] = TRUE;
		tUI_MdMsg.ubCmd_Len  			= 3;
		UI_SendRequestToPU(NULL, &tUI_MdMsg);
		printd(DBG_InfoLvl, "=>MD Trig2: %d\n", ulUI_MdTrig);
	}
}
//------------------------------------------------------------------------------
void UI_MDSetting(void *pvMdParam)
{
#define MD_H_WINDOWSIZE		64
#define MD_V_WINDOWSIZE		48
	uint16_t uwMD_X          = 0;
	uint16_t uwMD_Y          = 0;
	uint16_t uwMD_BlockSIdx  = 0;
	uint16_t uwMD_BlockEIdx  = 0;
	uint16_t uwMD_BlockH     = sensor_cfg.ulHSize / 30;
	uint16_t uwMD_BlockV     = sensor_cfg.ulVSize / 23;
	uint16_t uwMD_H_WinNum   = sensor_cfg.ulHSize / MD_H_WINDOWSIZE;
	uint16_t uwMD_StartIdx   = 0, i, j;
	uint16_t uwMD_TotalBlock = (30 * 23) / 2;	//1 block is 4 bits data, 1 byte is 2 block
	uint16_t uwMD_BlockNum1  = 0, uwMD_BlockNum2 = 0;
	uint8_t ubMD_BlockCnt    = 0;
	uint8_t *pMD_Param       = (uint8_t *)pvMdParam;
	uint8_t *pMD_BlockValue, *pMD_BlockGroup;
	static uint8_t ubUI_MdUpdateFlag = FALSE;

	if((pMD_Param[2] == 0) && (pMD_Param[3] == 0))
	{
		tUI_BuStsInfo.MdParam.ubMD_Mode = MD_OFF;
		if(TRUE == ubUI_MdUpdateFlag)
		{
			MD_Switch(tUI_BuStsInfo.MdParam.ubMD_Mode);
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "=>MD OFF\n");
		}
		else
			ubUI_MdUpdateFlag = TRUE;
		return;
	}
	printd(DBG_InfoLvl, "=>MD %d_%d_%d\n", ((pMD_Param[1] << 8) | pMD_Param[0]),  pMD_Param[2],  pMD_Param[3]);
	ubMD_BlockCnt  = pMD_Param[3] + 1;
	pMD_BlockValue = malloc(uwMD_TotalBlock);
	pMD_BlockGroup = malloc(uwMD_TotalBlock);
	for(i = 0; i < uwMD_TotalBlock; i++)
	{
		pMD_BlockValue[i] = MD_REG1_CNT_00;
		pMD_BlockGroup[i] = 0x88;
	}
	uwMD_StartIdx  = ((pMD_Param[1] << 8) | pMD_Param[0]);
	uwMD_X = (uwMD_StartIdx % uwMD_H_WinNum) * MD_H_WINDOWSIZE;
	uwMD_Y = (uwMD_StartIdx / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
	uwMD_BlockSIdx = ((uwMD_X / uwMD_BlockH) + ((uwMD_Y / uwMD_BlockV) * 30));
	printd(DBG_InfoLvl, "=>MD x=%d,y=%d,S=%d\n",uwMD_X,uwMD_Y,uwMD_BlockSIdx);
	if(!((uwMD_StartIdx + pMD_Param[2] + 1) % uwMD_H_WinNum))
	{
		uwMD_Y = ((uwMD_StartIdx + pMD_Param[2]) / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
		uwMD_BlockEIdx = (((uwMD_Y / uwMD_BlockV) + 1) * 30) - 1;
		printd(DBG_InfoLvl, "1=>MD y=%d,e=%d\n",uwMD_Y,uwMD_BlockEIdx);
	}
	else
	{
		uwMD_X = ((uwMD_StartIdx + pMD_Param[2]) % uwMD_H_WinNum) * MD_H_WINDOWSIZE;
		uwMD_Y = ((uwMD_StartIdx + pMD_Param[2]) / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
		uwMD_BlockEIdx = ((uwMD_X / uwMD_BlockH) + ((uwMD_Y / uwMD_BlockV) * 30));
		printd(DBG_InfoLvl, "2=>MD x=%d,y=%d,e=%d\n",uwMD_X,uwMD_Y,uwMD_BlockEIdx);
	}
	ubMD_BlockCnt  = ((((ubMD_BlockCnt * MD_V_WINDOWSIZE) / uwMD_BlockV) + 1) > 23) ? 23 : (((ubMD_BlockCnt * MD_V_WINDOWSIZE) / uwMD_BlockV) + 1);
	printd(DBG_InfoLvl, "3=>MD c=%d\n",ubMD_BlockCnt);

	MD_Init();
    MD_SetUserThreshold(MD_TRIG_LVL);
	MD_Switch(MD_OFF);
	for(i = uwMD_BlockSIdx; i <= uwMD_BlockEIdx; i++)
	{
		for(j = 0; j < ubMD_BlockCnt; j++)
		{
			uwMD_BlockNum1 = (i + (j * 30)) / 2;
			uwMD_BlockNum2 = (i + (j * 30)) % 2;
			if(!uwMD_BlockNum2)
			{
				pMD_BlockValue[uwMD_BlockNum1] = (pMD_BlockValue[uwMD_BlockNum1] & 0xF0) | MD_REG1_CNT_01;
				pMD_BlockGroup[uwMD_BlockNum1] = (pMD_BlockGroup[uwMD_BlockNum1] & 0xF0) | 2;
			}
			else
			{
				pMD_BlockValue[uwMD_BlockNum1] = (pMD_BlockValue[uwMD_BlockNum1] & 0x0F) | (MD_REG1_CNT_01 << 4);
				pMD_BlockGroup[uwMD_BlockNum1] = (pMD_BlockGroup[uwMD_BlockNum1] & 0x0F) | (2 << 4);
			}
		}
	}
	MD_SetROIindex((uint32_t *)pMD_BlockValue);
	MD_SetROIweight((uint32_t *)pMD_BlockGroup);
	MD_SetSensitivity(80);
	tUI_BuStsInfo.MdParam.ubMD_Mode = MD_ON;
	MD_Switch(tUI_BuStsInfo.MdParam.ubMD_Mode);
	free(pMD_BlockValue);
	free(pMD_BlockGroup);
	if(TRUE == ubUI_MdUpdateFlag)
	{
		for(i = 0; i < 4; i++)
			tUI_BuStsInfo.MdParam.ubMD_Param[i] = pMD_Param[i];
		UI_UpdateDevStatusInfo();
	}
	else
		ubUI_MdUpdateFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(void)
{
	uint8_t i;

	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamAnrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCam3DNRMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamvLDCMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamWdrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamDisMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamFlicker,		CAMFLICKER_60HZ);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamCbrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamCondenseMode, 	CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorBL, 		  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorContrast,	  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorSaturation, 64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamColorParam.ubColorHue, 		  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.MdParam.ubMD_Mode,	MD_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_BuStsInfo.tCamPsMode,			POWER_NORMAL_MODE);
	
	for(i = 0; i < 4; i++)
		tUI_BuStsInfo.MdParam.ubMD_Param[i] = 0;
	UI_UpdateDevStatusInfo();
	UI_SystemSetup();
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	uint8_t i;

	memset(&tUI_BuStsInfo, 0xFF, sizeof(UI_BUStatus_t));
	osMutexWait(APP_UpdateMutex, osWaitForever);
	SF_Read(ulUI_SFAddr, sizeof(UI_BUStatus_t), (uint8_t *)&tUI_BuStsInfo);
	osMutexRelease(APP_UpdateMutex);
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_BuStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_BuStsInfo.cbUI_FwVersion);
	if ((strncmp(tUI_BuStsInfo.cbUI_DevStsTag, SF_STA_UI_SECTOR_TAG, sizeof(tUI_BuStsInfo.cbUI_DevStsTag) - 1) == 0)
	&& (strncmp(tUI_BuStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_BuStsInfo.cbUI_FwVersion) - 1) == 0)) {

	} else {
		printd(DBG_ErrorLvl, "TAG no match, Reset UI\n");
	}
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamAnrMode,  		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCam3DNRMode, 		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamvLDCMode, 		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamWdrMode,  		CAMSET_OFF);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamDisMode,  		CAMSET_OFF);
	UI_CHK_CAMFLICER(tUI_BuStsInfo.tCamFlicker);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamCbrMode,  		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_BuStsInfo.tCamCondenseMode, 	CAMSET_OFF);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorBL, 		64);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorContrast, 	64);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorSaturation, 64);
	UI_CHK_CAMPARAM(tUI_BuStsInfo.tCamColorParam.ubColorHue, 		64);
	UI_CHK_MDMODE(tUI_BuStsInfo.MdParam.ubMD_Mode,		MD_OFF);
	UI_CHK_PSMODE(tUI_BuStsInfo.tCamPsMode,				POWER_NORMAL_MODE);
	for(i = 0; i < 4; i++)
	{
		if(MD_OFF == tUI_BuStsInfo.MdParam.ubMD_Mode)
			tUI_BuStsInfo.MdParam.ubMD_Param[i] = 0;
		else
			UI_CHK_CAMPARAM(tUI_BuStsInfo.MdParam.ubMD_Param[i], 0);
	}

	ADO_SetDacR2RVol(R2R_VOL_n0DB);
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);

	osMutexWait(APP_UpdateMutex, osWaitForever);
	//tUI_BuStsInfo.ulUI_DevStsTag = 0x93700;
	memcpy(tUI_BuStsInfo.cbUI_DevStsTag, SF_STA_UI_SECTOR_TAG, sizeof(tUI_BuStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_BuStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_BuStsInfo.cbUI_FwVersion) - 1);
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize);
	SF_Write(ulUI_SFAddr, sizeof(UI_BUStatus_t), (uint8_t *)&tUI_BuStsInfo);
	SF_EnableWrProtect();
	osMutexRelease(APP_UpdateMutex);
}


//===========================================
void MC0_StartHook(void)
{
	printf(">> MC0 StartHook()!!\r\n");
}

void MC0_StopHook(void)
{
	printf(">> MC0 StopHook()!!\r\n");
}

void MC0_FinishHook(void)
{
	printf(">> MC0 FinishHook()!!\r\n");
}

void MC1_StartHook(void)
{
	printf(">> MC1 StartHook()!!\r\n");
}

void MC1_StopHook(void)
{
	printf(">> MC1 StopHook()!!\r\n");
}

void MC1_FinishHook(void)
{
	printf(">> MC1 FinishHook!!\r\n");
}
//------------------------------------------------------------------------------
//NOTE: 马达控制初始化
//------------------------------------------------------------------------------
void UI_MotoControlInit(void)
{
  	#if (MC_ENABLE)
	MC_Setup_t tMC_SettingApp;
	//! MC
	GLB->PADIO0 = 1;	// MC0
	GLB->PADIO1 = 1;
	GLB->PADIO2 = 1;
	GLB->PADIO3 = 1;
	GLB->PADIO4 = 1;	// MC1
	GLB->PADIO5 = 1;
	GLB->PADIO6 = 1;
	GLB->PADIO7 = 1;

	ubUI_Mc1RunFlag = 0;
	ubUI_Mc2RunFlag = 0;
	ubUI_Mc1RunCnt = 0;
	ubUI_Mc2RunCnt = 0;
	
	tMC_SettingApp.ubMC_ClockDivider = 63;
	tMC_SettingApp.ubMC_ClockPerPeriod = 255;
	tMC_SettingApp.ubMC_HighPeriod = 24;//48/24/18
	tMC_SettingApp.ubMC_PeriodPerStep = 18;//36/18/16
	tMC_SettingApp.tMC_Inv = MC_NormalWaveForm;
	
	tMC_Setup(MC_0,&tMC_SettingApp);
	tMC_Setup(MC_1,&tMC_SettingApp);
  	#endif
}

//------------------------------------------------------------------------------
//NOTE: 收到Rx端信息
//------------------------------------------------------------------------------
void UI_StopMotor(void)
{
	if((ubUI_Mc1RunFlag == 1) || (ubUI_Mc2RunFlag == 1))
	{
		MC_Stop(MC_1);
		ubUI_Mc1RunFlag = 0;
		ubUI_Mc2RunFlag = 0;
		printf("UI_StopMotor MC_1!!!\n");
	}
	if((ubUI_Mc3RunFlag == 1) || (ubUI_Mc4RunFlag == 1))
	{
		MC_Stop(MC_0);
		ubUI_Mc3RunFlag = 0;
		ubUI_Mc4RunFlag = 0;
		printf("UI_StopMotor MC_0!!!\n");
	}

	ubUI_McHandshake = 0;
	ubUI_McPreHandshake = 0;
	ubMcHandshakeLost = 0;
}

void UI_PtzControlSetting(void *pvMCParam)
{
  	#if (MC_ENABLE)
	uint8_t *pMC_Param = (uint8_t *)pvMCParam;

	//printf("UI_PtzControlSetting pMC_Param[0]: %d.\n", pMC_Param[0]);
	switch(pMC_Param[0])
	{
		case 0:
			UI_StopMotor();
			ubUI_McHandshake = 0;
			break;
			
		case 1:	//水平,正转
			ubUI_Mc1RunCnt = 2;
			if(ubUI_Mc1RunFlag == 0)
			{
				ubUI_Mc1RunFlag = 1;
				MC_Start(MC_1, 0, MC_Clockwise, MC_WaitReady);
			}
			ubUI_McHandshake++;
			break;
		
		case 2:	//水平,反转
			ubUI_Mc1RunCnt = 2;
			if(ubUI_Mc2RunFlag == 0)
			{
				ubUI_Mc2RunFlag = 1;
				MC_Start(MC_1, 0, MC_Counterclockwise, MC_WaitReady);
			}
			ubUI_McHandshake++;
			break;
		
		case 3:	//垂直,正转
			ubUI_Mc2RunCnt = 2;
			if(ubUI_Mc3RunFlag == 0)
			{
				ubUI_Mc3RunFlag = 1;
				MC_Start(MC_0, 0, MC_Clockwise, MC_WaitReady);
			}
			ubUI_McHandshake++;
			break;
		
		case 4:	//垂直,反转
			ubUI_Mc2RunCnt = 2;
			if(ubUI_Mc4RunFlag == 0)
			{
				ubUI_Mc4RunFlag = 1;
				MC_Start(MC_0, 0, MC_Counterclockwise, MC_WaitReady);
			}
			ubUI_McHandshake++;
			break;

		default:
			break;
	}
  	#endif
}

//------------------------------------------------------------------------------
void UI_MCStateCheck(void)
{
	//printf(">> MC ubUI_McPreHandshake: %d, ubUI_McHandshake: %d, ubMcHandshakeLost: %d.\r\n", ubUI_McPreHandshake, ubUI_McHandshake, ubMcHandshakeLost);
	if(tUI_SyncAppState == APP_LINK_STATE)
	{
		if((ubUI_McPreHandshake == ubUI_McHandshake) && (ubUI_McHandshake > 0))
		{
			ubMcHandshakeLost++;
			if(ubMcHandshakeLost >= 3)
			{
				UI_StopMotor();
			}
		}
		else
		{
			ubMcHandshakeLost = 0;
			ubUI_McPreHandshake = ubUI_McHandshake;
		}
	}
	else
	{
		UI_StopMotor();
	}
}
//------------------------------------------------------------------------------
//NOTE: 
//------------------------------------------------------------------------------
void UI_UpdateMCStatus(void)
{
  	#if (MC_ENABLE)
	if((ubUI_Mc1RunFlag == 1)||(ubUI_Mc2RunFlag == 1))
	{
		ubUI_Mc1RunCnt--;
		if(ubUI_Mc1RunCnt == 0)
		{
			ubUI_Mc1RunCnt = 1;
			ubUI_Mc1RunFlag = 0;
			ubUI_Mc2RunFlag = 0;
 			MC_Stop(MC_0);
			printf(">> MC0 Stop!!\r\n");
		}
	}
	
	if((ubUI_Mc3RunFlag == 1)||(ubUI_Mc4RunFlag == 1))
	{
		ubUI_Mc2RunCnt--;
		if(ubUI_Mc2RunCnt == 0)
		{
			ubUI_Mc2RunCnt = 1;
			ubUI_Mc3RunFlag = 0;
			ubUI_Mc4RunFlag = 0;
 			MC_Stop(MC_1);
			printf(">> MC1 Stop!!\r\n");
		}
	}
  	#endif
}

void UI_BrightnessCheck(void) //20180408
{
	uint16_t uwDetLvl = 0x3FF;
	uwDetLvl = uwSADC_GetReport(1);
	//printf("uwDetLvl  0x%x \n",uwDetLvl);
	
	if(uwDetLvl < 10)
	{
		GPIO->GPIO_O4	= 0; //关
		//GPIO->GPIO_O4	= 1; //开
	}
}

void UI_BuInit(void)
{
	UI_MotoControlInit();
	pTempI2C = pI2C_MasterInit (I2C_1, I2C_SCL_100K);
	GPIO->GPIO_O4	= 1; //开
}

void UI_TestSetting(void *pvMCParam)
{
	uint8_t *pMC_Param = (uint8_t *)pvMCParam;
	uint8_t TestData0 = pMC_Param[0];
	uint8_t TestData1 = pMC_Param[1];
	
	#if 0
	MC_Stop(MC_1);
	MC_Stop(MC_0);
	ubTestMode = pMC_Param[0];
	printf("UI_TestSetting ubTestMode: %d.\n", ubTestMode);
	if(ubTestMode)
	{
		
	}
	#endif
	
	#if 1
	printf("UI_TestSetting TestData1: %d.\n", TestData0);
	APP_SetTuningToolMode(TestData0);
	WDT_Disable(WDT_RST);
	WDT_RST_Enable(WDT_CLK_EXTCLK, 1);
	while(1);
	#endif
}


void UI_TestCheck(void)
{
	#define Motor_Count		20
	#define Motor_Wait		10
	static uint8_t ubTestCount = 0;
	
	printf("UI_TestCheck ubTestCount: %d.\n", ubTestCount);
	if(ubTestCount == 0)
	{
		MC_Start(MC_1, 0, MC_Clockwise, MC_WaitReady); //水平,正转
	}
	else if(ubTestCount == Motor_Count)
	{
		MC_Stop(MC_0);
		MC_Stop(MC_1);
	}
	else if(ubTestCount == Motor_Count + Motor_Wait)
	{
		MC_Start(MC_1, 0, MC_Counterclockwise, MC_WaitReady);//水平,反转
	}
	else if(ubTestCount == Motor_Count*2 + Motor_Wait)
	{
		MC_Stop(MC_0);
		MC_Stop(MC_1);
	}
	else if(ubTestCount == Motor_Count*2 + Motor_Wait*2)
	{
		MC_Start(MC_0, 0, MC_Clockwise, MC_WaitReady);//垂直,正转
	}
	else if(ubTestCount == Motor_Count*3 + Motor_Wait*2)
	{
		MC_Stop(MC_0);
		MC_Stop(MC_1);
	}
	else if(ubTestCount == Motor_Count*3 + Motor_Wait*3)
	{
		MC_Start(MC_0, 0, MC_Counterclockwise, MC_WaitReady); //垂直,反转
	}
	else if(ubTestCount == Motor_Count*4 + Motor_Wait*3)
	{
		MC_Stop(MC_0);
		MC_Stop(MC_1);
	}
	else if(ubTestCount > Motor_Count*4 + Motor_Wait*4)
	{
		ubTestCount = 0;
		return;
	}
		
	ubTestCount++;
}

void UI_SetCamUVCMode(uint8_t Value)
{
	tUI_BuStsInfo.tCamUVCMode = Value;
}

uint8_t UI_GetCamUVCMode(void)
{
	return tUI_BuStsInfo.tCamUVCMode;
}

