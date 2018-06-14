/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_HS.h
	\brief		Application header file (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.3
	\date		2017/11/27
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_HS_H_
#define _APP_HS_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
#include "UI.h"
#include "KNL.h"
#include "PAIR.h"
#include "BSP.h"
#include "USBD_API.h"
//------------------------------------------------------------------------------
#define APP_EVENTQUEUE_SZ			30
#define APP_PAIRING_TIMEOUT			60	//!< Unit: seconds
#define osHeapSize                  ((size_t) (768*1024))

typedef enum
{
	APP_STATE_NULL = 0,       		  	//!< System reserved state. Don't change this line
	APP_POWER_OFF_STATE,
	APP_IDLE_STATE,
	APP_LINK_STATE,
	APP_LOSTLINK_STATE,
	APP_PAIRING_STATE,
}APP_State_t;

typedef enum
{
	rFAIL = 0,
	rSUCCESS,
	rLOSTLINK = 0,
	rLINK,
}APP_Result_t;

typedef enum
{
	APP_LINKSTS_RPT = 1,
	APP_PAIRSTS_RPT,
	APP_PAIRUDBU_PRT,
	APP_VWMODESTS_RPT,
	APP_VOXMODESTS_RPT,
	APP_RPT_NONE = 0xFF,
}APP_ReportType_t;

typedef struct
{
	APP_State_t  	 tAPP_State;
	APP_ReportType_t tAPP_ReportType;
	uint8_t 	 	 ubAPP_Report[14];
}APP_StatusReport_t;

typedef enum
{
	APP_TUNINGMODE_OFF,
	APP_TUNINGMODE_ON,
}APP_TuningMode_t;

typedef struct
{
	void(*pvAPP_TuningFunc)(void);
}APP_TuningFuncPtr_t;

typedef void(*pvAPP_StateCtrl)(APP_EventMsg_t *);

typedef struct
{
	void (*pvFuncPtr)(APP_EventMsg_t *ptAPP_Message);
}APP_StateFunc_t;

typedef struct
{
	KNL_ROLE	tKNL_StaNum;
	PAIR_TAG	tPAIR_StaNum;
	TWC_TAG		tTWC_StaNum;
}APP_StaNumMap_t;

typedef struct
{
	void (*APP_tActFunPtr)(void);
}APP_ActFuncPtr_t;

#ifdef VBM_PU
typedef struct
{
	KNL_DISP_LOCATION tKNL_DispLocation;
}APP_DispLocMap_t;

typedef struct
{
	KNL_DISP_LOCATION tKNL_DispLoc;
}APP_KNLRoleInfo_t;

typedef struct
{
	KNL_ROLE 			tPairBURole;
	KNL_DISP_LOCATION 	tPairBUDispLoc;
}APP_PairRoleInfo_t;
#endif

typedef struct
{
//	uint32_t 		  ulKNLInfo_Tag;
	char 		   	  cbKNL_InfoTag[12];
	char 		      cbKNL_FwVersion[11];
	KNL_ROLE		  tKNL_Role;
	KNL_OPMODE		  tKNL_OpMode;
#ifdef VBM_PU
	APP_KNLRoleInfo_t tBURoleInfo[4];		//! [0] = KNL_STA1, [1] = KNL_STA2, [2] = KNL_STA3, [3] = KNL_STA4
#endif
	KNL_ROLE		  tAdoSrcRole;
	USBD_ClassMode_t  tUsbdClassMode;
	APP_TuningMode_t  tTuningMode;
}APP_KNLInfo_t;

#define APP_KNLRoleMap2CamNum(RoleNum, CamNum)							\
{																		\
	for(CamNum = CAM1; CamNum <= CAM4; CamNum++)						\
	{																	\
		if(APP_GetSTANumMappingTable(CamNum)->tKNL_StaNum == RoleNum)	\
			break;														\
	}																	\
}

#define APP_PairTagMap2CamNum(StaTag, CamNum)							\
{																		\
	for(CamNum = CAM1; CamNum <= CAM4; CamNum++)						\
	{																	\
		if(APP_GetSTANumMappingTable(CamNum)->tPAIR_StaNum == StaTag)	\
			break;														\
	}																	\
}

#define APP_TwcTagMap2CamNum(StaTag, CamNum)							\
{																		\
	for(CamNum = CAM1; CamNum <= CAM4; CamNum++)						\
	{																	\
		if(APP_GetSTANumMappingTable(CamNum)->tTWC_StaNum == StaTag)	\
			break;														\
	}																	\
}
//------------------------------------------------------------------------------
void APP_StateFlowCtrl(APP_EventMsg_t *ptEventMsg);
void APP_PowerCtrlFunc(APP_EventMsg_t *ptEventMsg);
void APP_IdleStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_LinkStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_LostLinkStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_PairingStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_doPairingStart(void *pvPairInfo);
#ifdef VBM_PU
void APP_doUnbindBU(APP_EventMsg_t *ptEventMsg);
#endif
uint8_t APP_UpdateLinkStatus(void);
APP_StaNumMap_t *APP_GetSTANumMappingTable(UI_CamNum_t tCamNum);
void APP_LoadKNLSetupInfo(void);
void APP_UpdateKNLSetupInfo(void);
void APP_KNLParamSetup(void);
void APP_FWUgradeSetup(void);
#ifdef VBM_PU
void APP_SwitchViewTypeExec(APP_EventMsg_t *ptEventMsg);
#endif
void APP_PowerSaveExec(APP_EventMsg_t *ptEventMsg);
void APP_SetTuningToolMode(APP_TuningMode_t tTuningMode);
APP_TuningMode_t APP_GetTuningToolMode(void);
void APP_Start(void);

//! Extern
extern osMutexId APP_UpdateMutex;
#endif																			//!< End of _APP_HS_H_ definition
