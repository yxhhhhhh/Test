/*!
    The information contained herein is the exclusive property of SONiX and
    shall not be distributed, or disclosed in whole or in part without prior
    permission of SONiX.
    SONiX reserves the right to make changes without further notice to the
    product to improve reliability, function or design. SONiX does not assume
    any liability arising out of the application or use of any product or
    circuits described herein. All application information is advisor and does
    not from part of the specification.

    \file       UI_VBMBU.h
    \brief      User Interface Header file (for High Speed Mode)
    \author     Hanyi Chiu
    \version    1.1
    \date       2017/11/30
    \copyright  Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_VBMBU_H_
#define _UI_VBMBU_H_

#include "UI.h"
#include "APP_HS.h"
#include "MD_API.h"
#include "RTC_API.h"

#define UI_UPDATESTS_PERIOD     (1000 / UI_TASK_PERIOD)
#define UI_PAIRINGLED_PERIOD    (500 / UI_TASK_PERIOD)

typedef enum
{
    CAMFLICKER_50HZ,
    CAMFLICKER_60HZ,
} UI_CamFlicker_t;

typedef enum
{
    CAMSET_OFF,
    CAMSET_ON,
} UI_CamsSetMode_t;

typedef enum
{
    RECLOOP_MODE,
    RECMANU_MODE,
    PHOTO_MODE,
    RECPHOTO_MODE,
    VDOMODE_MAX
} UI_CamVdoMode_t;

typedef enum
{
    REC_LOOPING,
    REC_MANUAL,
    REC_RECMODE_MAX,
} UI_RecordMode_t;

typedef enum
{
    RECRES_FHD,
    RECRES_HD,
    RECRES_WVGA,
    RECRES_MAX,
} UI_RecordResolution_t;

typedef enum
{
    PHOTOFUNC_OFF,
    PHOTOFUNC_ON,
    PHOTOFUNC_MAX,
} UI_PhotoFunction_t;

typedef enum
{
    PHOTORES_3M,
    PHOTORES_5M,
    PHOTORES_12M,
    PHOTORES_MAX,
} UI_PhotoResolution_t;

typedef enum
{
    BAT_LVL0,
    BAT_LVL1,
    BAT_LVL2,
    BAT_LVL3,
    BAT_LVL4,
    BAT_CHARGE,
} UI_BatLvl_t;

typedef struct
{
    uint32_t ulCardSize;
    uint32_t ulRemainSize;
} UI_SdInfo_t;

typedef struct
{
    uint8_t ubColorBL;
    uint8_t ubColorContrast;
    uint8_t ubColorSaturation;
    uint8_t ubColorHue;
} UI_ColorParam_t;

typedef struct
{
    char                  cbUI_DevStsTag[11];
    char                  cbUI_FwVersion[11];
    uint32_t              ulCAM_ID;
    UI_DisplayLocation_t  tCamDispLocation;
    UI_CamVdoMode_t       tCamVdoMode;
    UI_BatLvl_t           tCamBatLvl;
    UI_SdInfo_t           tSdInfo;
    UI_RecordMode_t       tREC_Mode;
    UI_RecordResolution_t tREC_Resolution;
    UI_PhotoFunction_t    tPHOTO_Func;
    UI_PhotoResolution_t  tPHOTO_Resolution;
    UI_CamsSetMode_t      tCamAnrMode;
    UI_CamsSetMode_t      tCam3DNRMode;
    UI_CamsSetMode_t      tCamvLDCMode;
    UI_CamsSetMode_t      tCamWdrMode;
    UI_CamsSetMode_t      tCamDisMode;
    UI_CamFlicker_t       tCamFlicker;
    UI_CamsSetMode_t      tCamCbrMode;
    UI_CamsSetMode_t      tCamCondenseMode;
    UI_ColorParam_t       tCamColorParam;
    struct
    {
        uint8_t           ubMD_Mode;
        uint8_t           ubMD_Param[4];
    }MdParam;
    UI_PowerSaveMode_t    tCamPsMode;
    UI_CamsSetMode_t      tCamScanMode;
    uint8_t               tCamUVCMode;
    uint8_t               tNightModeFlag;
    uint8_t               ubReserved[210];
} UI_BUStatus_t;

typedef enum
{
    UI_BU_CMD_VERSION = 0x20,
    UI_BU_CMD_PS_MODE,
    UI_BU_CMD_ALARM_TYPE,
    UI_BU_CMD_PICKUP_VOLUME,
    UI_BU_CMD_SN_VALUE,
} UI_BUTOPUCmdID_t;

typedef enum
{
    UI_UPDATE_BUSTS = 0x20,
    UI_VOX_TRIG,
    UI_MD_TRIG,
    UI_VOICE_TRIG,
    UI_VOICE_CHECK,
    UI_TEMP_CHECK,
    UI_BU_TO_PU_CMD,
} UI_BUReqCmdID_t;

typedef enum
{
    UI_GET_BU_VERSION_CMD = 0x20,
    UI_SET_BU_ADO_TEST_CMD,
    UI_SET_TALK_ON_CMD,
    UI_SET_TALK_OFF_CMD,
    UI_GET_BU_PS_MODE_CMD,
    UI_SET_BU_ALARM_VALUE_CMD,
} UI_PUTOBUCmdID_t;

typedef enum
{
    UI_PTZ_SETTING = 1,
    UI_RECMODE_SETTING,
    UI_RECRES_SETTING,
    UI_SDCARD_SETTING,
    UI_PHOTOMODE_SETTING,
    UI_PHOTORES_SETTING,
    UI_SYSINFO_SETTING,
    UI_VOXMODE_SETTING,
    UI_ECOMODE_SETTING,
    UI_WORMODE_SETTING,
    UI_ADOANR_SETTING,
    UI_IMGPROC_SETTING,
    UI_MD_SETTING,
    UI_VOICETRIG_SETTING,
    UI_MOTOR_SETTING,
    UI_NIGHTMODE_SETTING,
    UI_PU_TO_BU_CMD_SETTING,
    UI_TEST_SETTING,
} UI_PUReqCmdID_t;

typedef enum
{
    UI_IMG3DNR_SETTING,
    UI_IMGvLDC_SETTING,
    UI_IMGWDR_SETTING,
    UI_IMGDIS_SETTING,
    UI_IMGCBR_SETTING,
    UI_IMGCONDENSE_SETTING,
    UI_FLICKER_SETTING,
    UI_IMGBL_SETTING,
    UI_IMGCONTRAST_SETTING,
    UI_IMGSATURATION_SETTING,
    UI_IMGHUE_SETTING,
    UI_IMGSETTING_MAX,
} UI_ImgProcSettingItem_t;

#define UI_CLEAR_THREADCNT(Flag, Count)         do { if(Flag == TRUE) { (Count) = 0; Flag = FALSE; } } while(0)
#define UI_CLEAR_CAMSETTINGTODEFU(xFUNC, mDEFU) do { xFUNC = mDEFU; } while(0)
#define UI_CHK_CAMSFUNCTS(Mode, Status)         do { if(Mode > CAMSET_ON) { Mode = Status; } } while(0)
#define UI_CHK_CAMFLICER(HZ)                    do { if(HZ > CAMFLICKER_60HZ) { HZ = CAMFLICKER_60HZ; } } while(0)
#define UI_CHK_CAMPARAM(Param, Value)           do { if(Param >= 128) { Param = Value; } } while(0)
#define UI_CHK_MDMODE(Mode, Status)             do { if(Mode > MD_ON) { Mode = Status; } } while(0)
#define UI_CHK_PSMODE(Mode, State)              do { if(Mode > State) { Mode = State; } } while(0)

//! Two way command timeout
#define UI_TWC_TIMEOUT      (3 * 1000)              //! Unit: ms
//! Two way command format of UI
#define UI_TWC_TYPE         0
#define UI_REPORT_ITEM      1
#define UI_REPORT_DATA      2
#define UI_SETTING_ITEM     1
#define UI_SETTING_DATA     2

typedef enum
{
    UI_REPORT,
    UI_SETTING,
} UI_TwcDataType_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{
    uint8_t         ubCmd_Len;
    uint8_t         ubCmd[8];
} UI_BUReqCmd_t;

typedef struct
{
    osThreadId  thread_id;
    int32_t     iSignals;
    UI_Result_t tReportSts;
} UI_ThreadNotify_t;
#pragma pack(pop)

typedef struct
{
    void (*pvAction)(void *);
} UI_ReportFuncPtr_t;

typedef struct
{
    void (*pvAction)(void *);
} UI_SettingFuncPtr_t;

typedef struct
{
    void (*pvImgFunc)(uint8_t);
    uint8_t *pImgParam;
} UI_IspSettingFuncPtr_t;

typedef enum
{
    UI_SYSVOICELVL_CHK  = 0x01,
    UI_SYSTEMPDATA_CHK  = 0x02,
    UI_SYSIRLEDDATA_CHK = 0x04,
    UI_SYSCHKSTS_MAX = 3,
} UI_SysChkType_t;

void UI_PowerKey(void);
void UI_PairingKey(void);
void UI_UpdateBUStatusToPU(void);
UI_Result_t UI_SendRequestToPU(osThreadId thread_id, UI_BUReqCmd_t *ptReqCmd);
void UI_RecvPUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus);
void UI_RecvPURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data);
void UI_SystemSetup(void);
void UI_PowerSaveSetting(void *pvPS_Mode);
void UI_ChangePsModeToWorMode(void);
void UI_ChangePsModeToNormalMode(void);
void UI_DisableVox(void);
void UI_VoxTrigger(void);
void UI_VoiceTrigSetting(void *pvTrigMode);
void UI_VoiceTrigger(void);
void UI_ANRSetting(void *pvSysInfo);
void UI_PtzControlSetting(void *pvMCParam);
void UI_IspSetup(void);
void UI_ImageProcSetting(void *pvImgProc);
void UI_MDTrigger(void);
void UI_MDSetting(void *pvMdParam);
void UI_ResetDevSetting(void);
void UI_LoadDevStatusInfo(void);
void UI_UpdateDevStatusInfo(void);

void UI_VoiceCheck(void);
void UI_TempCheck(void);

void UI_MotoControlInit(void);
void UI_MCStateCheck(void);
void UI_BrightnessCheck(void);

void UI_BuInit(void);
void UI_TestCheck(void);
void UI_TestSetting(void *pvTSParam);
void UI_SetCamUVCMode(uint8_t Value);
uint8_t UI_GetCamUVCMode(void);
void UI_PairingLongKey(void);
void UI_NightModeSetting(void *pvNMParam);
uint8_t UI_SendVersionToPu(void);
uint8_t UI_SendPsModeToPu(void);
uint8_t UI_SendAlarmTypeToPu(uint8_t AlarmType);
uint8_t UI_SendPickupVolumeToPu(uint32_t ulUI_AdcRpt);
void UI_RecvPUCmdSetting(void *pvRecvPuParam);
uint8_t UI_CheckAlarmWakeUp(void);
void UI_AlarmTrigger(void);
uint8_t UI_readSN(void);
uint8_t UI_SendSnValueToPu(uint8_t n);

#endif
