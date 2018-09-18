/*!
    The information contained herein is the exclusive property of SONiX and
    shall not be distributed, or disclosed in whole or in part without prior
    permission of SONiX.
    SONiX reserves the right to make changes without further notice to the
    product to improve reliability, function or design. SONiX does not assume
    any liability arising out of the application or use of any product or
    circuits described herein. All application information is advisor and does
    not from part of the specification.

    \file       UI_VBMPU.h
    \brief      User Interface Header file (for High Speed Mode)
    \author     Hanyi Chiu
    \version    1.10
    \date       2018/08/02
    \copyright  Copyright (C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_VBMPU_H_
#define _UI_VBMPU_H_

#include "UI.h"
#include "APP_HS.h"
#include "OSD.h"
#include "RTC_API.h"
#include "FS_API.h"
#include "TIMER.h"

#define INVALID_ID                  0xFFFFFFFF
#define INVAILD_ACT                 0xFFFFFFFE
#define UI_UPDATESTS_PERIOD         (1000 / UI_TASK_PERIOD)
#define UI_SHOWLOSTLOGO_PERIOD      (1000 / UI_TASK_PERIOD)
#define UI_UPDATEBRILVL_PERIOD      (1000 / UI_TASK_PERIOD)
#define UI_UPDATEVOLLVL_PERIOD      (1000 / UI_TASK_PERIOD)
#define UI_PHOTOGRAPHSTS_PERIOD     (1000 / UI_TASK_PERIOD)

#define QUAL_TYPE_ITEM          6
#define SCAN_TYPE_ITEM          5
#define DUAL_TYPE_ITEM          4

#define RECORD_PWRSTS_ADDR      0
#define PWRSTS_KEEP             3

#define TEMP_ALARM_INTERVAL     270
#define PICk_ALARM_INTERVAL     270

typedef void (*pvUiFuncPtr)(void);

typedef enum
{
    UI_ICON_NORMAL = 0,
    UI_ICON_HIGHLIGHT,
    UI_ICON_READY
} UI_IconType_t;

typedef enum
{
    UP_ARROW,
    DOWN_ARROW,
    LEFT_ARROW,
    RIGHT_ARROW,
    ENTER_ARROW,
    EXIT_ARROW,
} UI_ArrowKey_t;

//typedef enum
//{
//  KEY_NOT_ACTION,
//  KEY_ACTION,
//} UI_ArrowKeyAct_t;

typedef enum
{
    UI_DISPLAY_STATE = 0x20,            //! Display Image
    UI_MAINMENU_STATE,                  //! Menu
    UI_SUBMENU_STATE,                   //! Sub menu
    UI_SUBSUBMENU_STATE,                //! Sub sub menu
    UI_SUBSUBSUBMENU_STATE,             //! Sub sub sub menu
    UI_CAM_SEL_STATE,                   //! Camera Selection
    UI_SET_VDOMODE_STATE,               //! Setting Video mode
    UI_SET_ADOSRC_STATE,                //! Selection Audio source
    UI_SET_BUECOMODE_STATE,             //! Setting ECO mode of BU
    UI_SET_PUPSMODE_STATE,              //! Setting VOX/WOR mode of PU
    UI_CAMSETTINGMENU_STATE,
    UI_SET_CAMCOLOR_STATE,
    UI_DPTZ_CONTROL_STATE,
    UI_MD_WINDOW_STATE,
    UI_PAIRING_STATE,
    UI_DUALVIEW_CAMSEL_STATE,
    UI_SDFWUPG_STATE,
	UI_VOXPS_STATE,
	UI_ADOONLYPS_STATE,
    UI_RECFOLDER_SEL_STATE,
    UI_RECFILES_SEL_STATE,
    UI_RECPLAYLIST_STATE,
    UI_PHOTOPLAYLIST_STATE,
    UI_ENGMODE_STATE,
    UI_STATE_MAX,
}UI_State_t;

typedef enum
{
    BRIGHT_ITEM,
    AUTOLCD_ITEM,
    ALARM_ITEM,
    TIME_ITEM,
    ZOOM_ITEM,
    CAMERAS_ITEM,
    SETTING_ITEM,
    MENUITEM_MAX,
}UI_MenuItemList_t;

typedef enum
{
    NOT_ACTION,
    DRAW_MENUPAGE,
    DRAW_HIGHLIGHT_MENUICON,
    EXECUTE_MENUFUNC,
    EXIT_MENUFUNC,
}UI_MenuAct_t;


typedef enum
{
    ADJUST_BR_ITEM,
    AUTO_BR_ITEM,
    BRIGHTNESS_MAX
}UI_BrightnessSubMenuItemList_t;


typedef enum
{
//  CAMVIEW_ITEM,
//  PTZ_ITEM,
//  CAMBL_ITEM,
    CAMSSELCAM_ITEM,
    CAMSANR_ITEM,
    CAMS3DNR_ITEM,
    CAMSvLDS_ITEM,
    CAMSAEC_ITEM,
    CAMSDIS_ITEM,
    CAMSCBR_ITEM,
    CAMSCONDENSE_ITEM,
    CAMSFLICKER_ITEM,
    CAMSITEM_MAX
}UI_CamsSubMenuItemList_t;

typedef enum
{
    CAMSET_OFF,
    CAMSET_ON,
}UI_CamsSetMode_t;

typedef enum
{
    CAMFLICKER_50HZ,
    CAMFLICKER_60HZ,
}UI_CamFlicker_t;

typedef enum
{
    CAM1VIEW_ITEM,
    CAM2VIEW_ITEM,
    CAM3VIEW_ITEM,
    CAM4VIEW_ITEM,
    QUALVIEW_ITEM,
    CAMVIEWITEM_MAX,
    SINGLEVIEW_ITEM = 1,
    DUALVIEW_ITEM = 3,
}UI_CamsSubSubMenuItemList_t;

typedef enum
{
    PAIRCAM_ITEM,
    DELCAM_ITEM,
    PAIRITEM_MAX
}UI_PairSubMenuItemList_t;

typedef enum
{
//  RECSELCAM_ITEM,
//  RECMODE_ITEM,
//  RECRES_ITEM,
//  SDCARD_ITEM,
    RECMODE_ITEM,
    RECTIME_ITEM,
    RECITEM_MAX
}UI_RecordSubMenuItemList_t;

typedef enum
{
    REC_LOOPING,
    REC_MANUAL,
    REC_TRIGGER,
    REC_OFF,
    REC_RECMODE_MAX,
}UI_RecordMode_t;

typedef enum
{
    RECRES_FHD,
    RECRES_HD,
    RECRES_WVGA,
    RECRES_MAX,
}UI_RecordResolution_t;

typedef enum
{
    RECTIME_1MIN,
    RECTIME_2MIN,
    RECTIME_5MIN,
    RECTIME_10MIN,
    RECTIME_MAX,
}UI_RecordTime_t;

typedef enum
{
    PHOTOSELCAM_ITEM,
    PHOTOFUNC_ITEM,
    PHOTORES_ITEM,
    PHOTOITEM_MAX
}UI_PhotoSubMenuItemList_t;

typedef enum
{
    PHOTOFUNC_OFF,
    PHOTOFUNC_ON,
    PHOTOFUNC_MAX,
}UI_PhotoFunction_t;

typedef enum
{
    PHOTORES_3M,
    PHOTORES_5M,
    PHOTORES_12M,
    PHOTORES_MAX,
}UI_PhotoResolution_t;

#if UI_NOTIMEMENU
typedef enum
{
    NIGHTMODE_ITEM,
    FLICKER_ITEM,
    LANGUAGESET_ITEM,
    DEFAULT_ITEM,
    PRODUCT_INFO_ITEM,
    CONTACT_ITEM,
    SETTINGITEM_MAX,
   TEMPUNIT_ITEM,
}UI_SettingSubMenuItemList_t;
#else
typedef enum
{
    NIGHTMODE_ITEM,
    FLICKER_ITEM,
    LANGUAGESET_ITEM,
    DEFAULT_ITEM,
     TEMPUNIT_ITEM,
    PRODUCT_INFO_ITEM,
    CONTACT_ITEM,
    SETTINGITEM_MAX,
}UI_SettingSubMenuItemList_t;
#endif

typedef enum
{
    AECFUNC_OFF,
    AECFUNC_ON,
    AECFUNC_MAX,
} UI_AecFunction_t;

typedef enum
{
    CCAFUNC_OFF,
    CCAFUNC_ON,
    CCAMODE_MAX,
} UI_CCAMode_t;

typedef enum
{
    YEAR_ITEM,
    MONTH_ITEM,
    DATE_ITEM,
    HOUR_ITEM,
    MIN_ITEM,
    SEC_ITEM,
    ALLCALE_ITEM,
} UI_CalendarItem_t;

typedef enum
{
    TEMP_ALARM_IDLE = 0,
    HIGH_TEMP_ALARM_ON,
    HIGH_TEMP_ALARM_ING,
    LOW_TEMP_ALARM_ON,
    LOW_TEMP_ALARM_ING,
    TEMP_ALARM_OFF,
} TEMP_ALARM_STATE;

typedef enum
{
    PICKUP_ALARM_IDLE = 0,
    PICKUP_ALARM_ON,
    PICKUP_ALARM_ING,
    PICKUP_ALARM_OFF,
} PICKUP_ALARM_STATE;

typedef enum
{
    MC0_OFF = 0,
    MC0_ON,
    MC0_TOP,
} MCO_STATE;

typedef enum
{
    MC1_OFF = 0,
    MC1_ON,
    MC1_TOP,
} MC1_STATE;

typedef enum
{
    MC_UP_DOWN_OFF      = 0x00,
    MC_UP_ON            = 0x11,
    MC_UP_TOP           = 0x12,
    MC_DOWN_ON          = 0x21,
    MC_DOWN_TOP         = 0x22,
    MC_LEFT_RIGHT_OFF   = 0x01,
    MC_LEFT_ON          = 0x31,
    MC_LEFT_TOP         = 0x32,
    MC_RIGHT_ON         = 0x41,
    MC_RIGHT_TOP        = 0x42,
} MC_STATE;


typedef enum
{
    PWR_ON = 0,
    PWR_Prep_Sleep,
    PWR_Start_Sleep,
    PWR_Sleep_Complete,
    PWR_Prep_Wakeup,
    PWR_Start_Wakeup,
} PWR_STATE;

typedef struct
{
    void (*pvFuncPtr)(UI_ArrowKey_t);
} UI_MenuFuncPtr_t;

typedef struct
{
    void (*pvFuncPtr)(void);
} UI_DrawSubMenuFuncPtr_t;

typedef struct
{
    uint8_t      ubItemIdx;
    uint8_t      ubItemPreIdx;
} UI_MenuItem_t;

typedef struct
{
    uint8_t       ubFirstItem;
    uint8_t       ubItemCount;
    UI_MenuItem_t tSubMenuInfo;
} UI_SubMenuItem_t;

typedef struct
{
    UI_CamNum_t tCamNum4CamSetSub;
    UI_CamNum_t tCamNum4CamView;
    UI_CamNum_t tCamNum4RecSub;
    UI_CamNum_t tCamNum4PhotoSub;
} UI_SubMenuCamNum_t;

typedef struct
{
    UI_SubMenuItem_t tCameraViewPage;
} UI_CamsSubSubMenuItem_t;

typedef struct
{
    UI_SubMenuItem_t tPairS[PAIRITEM_MAX];
} UI_PairSubSubMenuItem_t;

typedef struct
{
    UI_SubMenuItem_t tRecordS[RECITEM_MAX];
}UI_RecSubSubMenuItem_t;

typedef struct
{
    UI_SubMenuItem_t tPhotoS[CAM_4T][PHOTOITEM_MAX];
} UI_PhotoSubSubMenuItem_t;

typedef struct
{
    UI_SubMenuItem_t tSettingS[SETTINGITEM_MAX];
} UI_SettingSubSubMenuItem_t;

typedef struct
{
    uint8_t      ubItemIdx;
    uint8_t      ubItemPreIdx;
} UI_SettingSubSubSubItem_t;

typedef enum
{
    UI_OsdUpdate,
    UI_OsdErase,
} UI_OsdImgFnType_t;

typedef struct
{
    UI_CamViewType_t tCamViewType;
    UI_CamNum_t      tCamViewPool[CAM_4T];
} UI_CamViewSelect_t;

typedef struct
{
    uint32_t ulCardSize;
    uint32_t ulRemainSize;
} UI_SdInfo_t;

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
    CAM_OFFLINE,
    CAM_ONLINE,
    CAM_STSMAX,
} UI_CamConnectStatus_t;

typedef enum
{
    ANT_SIGNALLVL6,
    ANT_SIGNALLVL5,
    ANT_SIGNALLVL4,
    ANT_SIGNALLVL3,
    ANT_SIGNALLVL2,
    ANT_SIGNALLVL1,
    ANT_NOSIGNAL,
} UI_AntLvl_t;

typedef enum
{
    BAT_LVL0,
    BAT_LVL1,
    BAT_LVL2,
    BAT_LVL3,
    BAT_LVL4,
    BAT_CHARGE,
} UI_BatLvl_t;

typedef enum
{
    BL_LVL0,
    BL_LVL1,
    BL_LVL2,
    BL_LVL3,
    BL_LVL4,
    BL_LVL5,
    BL_LVL6,
    BL_LVL7,
    BL_LVL8,
} UI_BrightnessLvl_t;

typedef enum
{
    VOL_LVL0,
    VOL_LVL1,
    VOL_LVL2,
    VOL_LVL3,
    VOL_LVL4,
    VOL_LVL5,
    VOL_LVL6,
    VOL_LVL7,
    VOL_LVL8,
} UI_VolumeLvl_t;

typedef struct
{
    UI_CamNum_t             tPairSelCam;
    UI_DisplayLocation_t    tDispLocation;
    uint8_t                 ubDrawFlag;
} UI_PairingInfo_t;

typedef struct
{
    UI_AntLvl_t tAntLvl;
    uint8_t     ubRssiValue;
} UI_RssiMap2AntLvl_t;

typedef struct
{
    UI_AntLvl_t tAntLvl;
    uint8_t     ubPerValue;
} UI_PerMap2AntLvl_t;

typedef enum
{
    UI_SCALEUP_2X = 2,
    UI_SCALEUP_3X,
    UI_SCALEUP_4X,
} UI_ScaleUp_t;

typedef enum
{
    DUAL_CAM1_CAM2,
    DUAL_CAM1_CAM3,
    DUAL_CAM1_CAM4,
    DUAL_CAM2_CAM1,
    DUAL_CAM2_CAM3,
    DUAL_CAM2_CAM4,
    DUAL_CAM3_CAM1,
    DUAL_CAM3_CAM2,
    DUAL_CAM3_CAM4,
    DUAL_CAM4_CAM1,
    DUAL_CAM4_CAM2,
    DUAL_CAM4_CAM3,
    DUAL_VIEWCAMSEL_MAX,
} UI_DualViewCamSel_t;

typedef enum
{
    UI_CAMISP_SETUP,
    UI_CAMFUNC_SETUP,
} UI_CameraSettingMenu_t;

typedef enum
{
    UI_COLOR_ITEM,
    UI_DPTZ_ITEM,
    UI_MD_ITEM,
} UI_CameraSettingItem_t;

typedef struct
{
    LCD_DYN_INFOR_TYP tUI_LcdCropParam;
    UI_ScaleUp_t      tScaleParam;
} UI_DPTZParam_t;

typedef struct
{
    uint8_t ubColorBL;
    uint8_t ubColorContrast;
    uint8_t ubColorSaturation;
    uint8_t ubColorHue;
} UI_ColorParam_t;

typedef struct
{
    uint16_t      ubMin;
    uint16_t      ubMax;
    uint8_t       ubValue;
} LightSenseAdjust_t;

typedef struct
{
    uint16_t ubMinBat;
    uint16_t ubMaxBat;
    uint8_t  ubBatLev;
} BatteryMap_t;


typedef enum
{
    UI_OSDLDDISP_OFF,
    UI_OSDLDDISP_ON,
}UI_OsdLdDispSts_t;

typedef enum
{
    UI_SEARCH_DCIMFOLDER,
    UI_SEARCH_RECFILES,
}UI_LoadingState_t;

typedef enum
{
    UI_VDORECLOOP_MODE,
    UI_VDORECMANU_MODE,
    UI_VDORECTRIG_MODE,
    UI_VDOPHOTO_MODE,
    UI_VDOMODE_MAX
}UI_VdoRecMode_t;

typedef enum
{
    UI_RECIMG_COLOR1,
    UI_RECIMG_COLOR2,
    UI_RECIMG_COLOR3,
    UI_RECIMG_COLOR4,
    UI_RECIMG_COLOR5,
    UI_RECIMG_COLOR6,
    UI_RECIMG_DEFU = 0xFF,
}UI_RecImgColor_t;

typedef enum
{
    UI_RECFILE_PAUSE,
    UI_RECFILE_PLAY,
}UI_RecPlayStatus_t;

typedef enum
{
    UI_RECSKIPBKFWD_ITEM,
    UI_RECPLAYPAUSE_ITEM,
    UI_RECSKIPFRFWD_ITEM,
    UI_RECPLAYLISTITEM_MAX
}UI_RecPlayListItem_t;

#define REC_FOLDER_LIST_MAXNUM  10
#define REC_FILE_LIST_MAXNUM    10
#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */

typedef struct
{
    uint16_t uwUI_RecNumArray[10];
    uint16_t uwUI_RecUpperLetterArray[26];
    uint16_t uwUI_RecLowerLetterArray[26];
    uint16_t uwUI_RecSymbolArray[4];
}UI_RecOsdImgDb_t;

typedef struct
{
    uint8_t ubTotalRecFolderNum;
    uint8_t ubRecFolderSelIdx;
    FS_FoldersInfo_t tRecFolderInfo[50];
}UI_RecFoldersInfo_t;

typedef struct
{
    uint16_t uwTotalRecFileNum;
    uint16_t uwRecFileSelIdx;
    FS_FilesInfo_t tRecFilesInfo[1000];
}UI_RecFilesInfo_t;

typedef struct
{
    uint32_t              ulCAM_ID;
    UI_DisplayLocation_t  tCamDispLocation;
    UI_CamConnectStatus_t tCamConnSts;
    UI_CamVdoMode_t       tCamVdoMode;
    UI_AntLvl_t           tCamAntLvl;
    UI_BatLvl_t           tCamBatLvl;
    UI_SdInfo_t           tSdInfo;
    UI_RecordMode_t       tREC_Mode;
    UI_RecordResolution_t tREC_Resolution;
    UI_PhotoFunction_t    tPHOTO_Func;
    UI_PhotoResolution_t  tPHOTO_Resolution;
    UI_CamsSetMode_t      tCamAnrMode;
    UI_CamsSetMode_t      tCam3DNRMode;
    UI_CamsSetMode_t      tCamvLDCMode;
    UI_CamsSetMode_t      tCamAecMode;
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
    uint8_t               ubReserved[216];
}UI_BUStatus_t;

typedef struct
{
    uint8_t             ubTotalBuNum;
    uint8_t             ubPairedBuNum;
    RTC_Calendar_t      tSysCalendar;
    uint8_t             ubAEC_Mode;
    uint8_t             ubCCA_Mode;
    uint8_t             ubSTORAGE_Mode;
    struct
    {
        uint8_t             ubDrawStsIconFlag;
        uint8_t             ubRdPairIconFlag;
        uint8_t             ubClearThdCntFlag;
        uint8_t             ubShowLostLogoFlag;
        uint8_t             ubDrawNoCardIconFlag;
        uint8_t             ubDrawMdTrigFlag;
    }IconSts;
    struct
    {
        UI_BrightnessLvl_t  tBL_UpdateLvL;
        uint8_t             ubBL_UpdateCnt;
    }BriLvL;
    struct
    {
        UI_VolumeLvl_t      tVOL_UpdateLvL;
        uint8_t             ubVOL_UpdateCnt;
    }VolLvL;
    UI_CamNum_t         tAdoSrcCamNum;
    UI_PowerSaveMode_t  tPsMode;
    struct
    {
        UI_RecordMode_t     tREC_Mode;
        UI_RecordTime_t     tREC_Time;
    }RecInfo;
    UI_VdoRecMode_t     tVdoRecMode;
    uint8_t             ubVdoRecStsCnt;
    uint8_t             ubScanModeEn;
    uint8_t             ubDualModeEn;
    uint8_t             ubDefualtFlag;
    uint8_t             ubFeatCode0;
    uint8_t             ubFeatCode1;
    uint8_t             ubFeatCode2;
    uint16_t            ubScanTime;
    uint8_t             ubHighTempSetting;
    uint8_t             ubLowTempSetting;
    uint8_t             ubTempAlertSetting;
    uint8_t             ubSoundLevelSetting;
    uint8_t             ubSoundAlertSetting;
    uint8_t             ubAutoBrightness;
    uint8_t             ubSleepMode;
    uint8_t             ubZoomScale;
    uint8_t             ubFlickerFlag;
    uint8_t             ubLangageFlag;
    uint8_t             ubTempunitFlag;
    uint8_t             NightmodeFlag;
    uint8_t             ubCamViewNum;
	uint8_t				ubSquealWarnCnt;	
	uint8_t				ubReserved[206];	
}UI_PUSetting_t;

typedef struct
{
    //uint32_t     ulUI_DevStsTag;
    char           cbUI_DevStsTag[11];
    char           cbUI_FwVersion[11];
    UI_PUSetting_t tPU_SettingInfo;
    UI_BUStatus_t  tBU_StatusInfo[CAM_4T];
} UI_DeviceStatusInfo_t;
#pragma pack(pop)

#define UI_CLEAR_THREADCNT(Flag, Count)         do { if(Flag == TRUE) { (Count) = 0; Flag = FALSE; } } while(0)
#define UI_CLEAR_CAMSETTINGTODEFU(xFUNC, mDEFU) do { xFUNC = mDEFU; } while(0)
#define UI_CLEAR_CALENDAR_TODEFU(xFUNC, mDEFU)  do { xFUNC = mDEFU; } while(0)

#define UI_CHK_CAMSFUNCS(Mode, Status)          do { if(Mode > CAMSET_ON) { Mode = Status; } } while(0)
#define UI_CHK_CAMFLICKER(HZ)                   do { if(HZ > CAMFLICKER_60HZ) { HZ = CAMFLICKER_50HZ; } } while(0)
#define UI_CHK_CAMPARAM(Param, Value)           do { if(Param >= 128) { Param = Value; } } while(0)
#define UI_CHK_BUSYS(SysParam, Limit, Target)   do { if(SysParam >= Limit) { SysParam = Target; } } while(0)
#define UI_CHK_PUSYS(SysParam, Limit, Target)   do { if(SysParam >= Limit) { SysParam = Target; } } while(0)

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

typedef enum
{
    UI_BU_CMD_VERSION = 0x20,
    UI_BU_CMD_PS_MODE,
    UI_BU_CMD_ALARM_TYPE,
    UI_BU_CMD_PICKUP_VOLUME,
    UI_BU_CMD_SN_VALUE,
    UI_BU_CMD_IR_VALUE,
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
    UI_SET_BUMIC12_CMD,
    UI_SET_BUMIC13_5_CMD,
    UI_SET_BUMOTOR_SPEED_H_CMD,
    UI_SET_BUMOTOR_NORMAL_CMD,
    UI_SET_BUMOTOR_SPEED_L_CMD,
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
    UI_ADOAEC_SETTING,
    UI_IMGPROC_SETTING,
    UI_MD_SETTING,
    UI_VOICETRIG_SETTING,
    UI_MOTOR_SETTING,
    UI_NIGHTMODE_SETTING,
    UI_PU_TO_BU_CMD_SETTING,
    UI_TEST_SETTING,
}UI_PUReqCmdID_t;

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
    UI_IMGSETTING_MAX = 20,
}UI_ImgProcSettingItem_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{
    UI_CamNum_t     tDS_CamNum;
    uint8_t         ubCmd_Len;
    uint8_t         ubCmd[8];
} UI_PUReqCmd_t;

typedef struct
{
    osThreadId  thread_id;
    int32_t     iSignals;
    UI_Result_t tReportSts;
} UI_ThreadNotify_t;
#pragma pack(pop)

typedef struct
{
	void (*pvAction)(UI_CamNum_t, void *);
}UI_ReportFuncPtr_t;

typedef struct
{
    void (*pvAction)(UI_CamNum_t, void *);
} UI_SettingFuncPtr_t;


void UI_PowerKeyShort(void); //20180319
void UI_PowerKey(void);
void UI_MenuKey(void);
void UI_MenuLongKey(void);
void UI_UpArrowKey(void);
void UI_DownArrowKey(void);
void UI_LeftArrowKey(void);
void UI_RightArrowKey(void);
void UI_EnterKey(void);
void UI_PuPowerSaveKey(void);
void UI_BuPowerSaveKey(void);
void UI_PuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey);
void UI_BuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey);
void UI_PushTalkKey(void);
void UI_PushTalkKeyShort(void);
void UI_DrawCameraSettingMenu(UI_CameraSettingMenu_t tCamSetMenu);
void UI_CameraSettingMenu1Key(void);
void UI_CameraSettingMenu2Key(void);
void UI_CameraSettingMenu(UI_ArrowKey_t tArrowKey);
void UI_CameraColorSetting(UI_ArrowKey_t tArrowKey);
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx);
void UI_DPTZ_KeyRelease(uint8_t ubKeyID);
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey);
void UI_MD_Window(UI_ArrowKey_t tArrowKey);
void UI_CameraSelectionKey(void);
void UI_CameraSelection(UI_ArrowKey_t tArrowKey);
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey);
void UI_ChangeVideoMode(UI_ArrowKey_t tArrowKey);
void UI_ChangeAudioSourceKey(void);
void UI_ChangeAudioSource(UI_ArrowKey_t tArrowKey);
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey);
void UI_DrawMenuPage(void);
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem);
void UI_Menu(UI_ArrowKey_t tArrowKey);
void UI_SubMenu(UI_ArrowKey_t tArrowKey);
void UI_SubSubMenu(UI_ArrowKey_t tArrowKey);
void UI_SubSubSubMenu(UI_ArrowKey_t tArrowKey);
void UI_CameraSettingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawPairingStatusIcon(void);
void UI_ReportPairingResult(UI_Result_t tResult);
void UI_DrawDCIMFolderMenu(void);
void UI_DrawRecordFileMenu(void);
void UI_DCIMFolderSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordFileSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordPlayListSelection(UI_ArrowKey_t tArrowKey);
void UI_PhotoPlayListSelection(UI_ArrowKey_t tArrowKey);
void UI_RecordSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_RecordSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PhotoSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PhotoSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PowerSaveSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_ResetSubMenuInfo(void);
void UI_ResetSubSubMenuInfo(void);
UI_CamNum_t UI_ChangeSelectCamNum4UiMenu(UI_CamNum_t *tCurrentCamNum, UI_ArrowKey_t *ptArrowKey);
void UI_DrawHLandNormalIcon(uint16_t uwNormalOsdImgIdx, uint16_t uwHighLigthOsdImgIdx);
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu);
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem);
void UI_UpdateBriLvlIcon(void);
void UI_UpdateVolLvlIcon(void);
void UI_DrawPUStatusIcon(void);
void UI_RemoveLostLinkLogo(void);
void UI_ShowLostLinkLogo(uint16_t *pThreadCnt);
void UI_UpdateBuStatusOsdImg(OSD_IMG_INFO *pOsdImgInfo, OSD_UPDATE_TYP tUpdateMode,
                             UI_OsdImgFnType_t tOsdImgFnType, UI_DisplayLocation_t tDispLoc);
void UI_ClearBuConnectStatusFlag(void);
void UI_RedrawBuConnectStatusIcon(UI_CamNum_t tCamNum);
void UI_RedrawNoSignalOsdIcon(UI_CamNum_t tCamNum, UI_OsdImgFnType_t tOsdImgFnType);
void UI_ClearStatusBarOsdIcon(void);
void UI_RedrawStatusBar(uint16_t *pThreadCnt);
void UI_ReportBuConnectionStatus(void *pvConnectionSts);
void UI_UpdateBuStatus(UI_CamNum_t tCamNum, void *pvStatus);
void UI_LeftArrowLongKey(void);
void UI_RightArrowLongKey(void);
void UI_ShowSysTime(void);
void UI_UnBindBu(UI_CamNum_t tUI_DelCam);
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_EnableVox(void);
void UI_DisableVox(void);
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig);
UI_Result_t UI_SendRequestToBU(osThreadId thread_id, UI_PUReqCmd_t *ptReqCmd);
void UI_RecvBUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus);
void UI_RecvBURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data);
void UI_ResetDevSetting(UI_CamNum_t tCamNum);
void UI_LoadDevStatusInfo(void);
void UI_UpdateDevStatusInfo(void);
void UI_SetupScanModeTimer(uint8_t ubTimerEn);
void UI_EnableScanMode(void);
void UI_DisableScanMode(void);
void UI_ScanModeExec(void);
UI_Result_t UI_CheckCameraSource4SV(void);
void UI_EngModeKey(void);
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey);
void UI_PairingControl(UI_ArrowKey_t tArrowKey);
void UI_FwUpgViaSdCard(void);
void UI_FwUpgExecSel(UI_ArrowKey_t tArrowKey);
void UI_UpdateRecStsIcon(void);
void UI_PhotoCaptureFinish(uint8_t ubPhotoCapRet);
void UI_PhotoPlayFinish(uint8_t ubPhtoCapRet);

//====================================================
void UI_DrawBrightnessSubMenuPage(void);
void UI_BrightnessSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_BrightnessDisplay(uint8_t value);
void UI_AutoBriDisplay(uint8_t value);
void UI_DrawBrightnessSubMenuPage_NoSel(void);

void UI_DrawAutoLcdSubMenuPage(void);
uint8_t UI_AutoLcdResetSleepTime(uint8_t KeyAction);
void UI_AutoLcdSetSleepTimerEvent(void);
void UI_AutoLcdSetSleepTime(uint8_t SleepMode);
void UI_AutoLcdSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_AutoLcdSubMenuDisplay(uint8_t value);
void UI_DrawAutoLcdSubMenuPage_NoSel(void);

void UI_DrawAlarmSubMenuPage(void);
void UI_AlarmSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_AlarmSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_AlarmSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawAlarmSubMenuPage_NoSel(void);
void UI_PlayAlarmSound(uint8_t type);
void UI_StopPlayAlarm(void);
void UI_AlarmmenuDisplay(void);
void UITempSubmenuDisplay(void);
void UISoundSubmenuDisplay(void);

void UI_AlarmTriggerDisplay(uint8_t value);

void UI_AlarmSubSubMenuUpKey(uint8_t SubMenuItem);
void UI_AlarmSubSubMenuDownKey(uint8_t SubMenuItem);
void UI_AlarmSubSubMenuEnterKey(uint8_t SubMenuItem);
void UITempSubSubSubmenuDisplay(uint8_t SubMenuItem);
void UISoundSubSubSubmenuDisplay(uint8_t SubMenuItem);
void InitAlarmSubSubSubmenu(uint8_t SubMenuItem);
void UI_SetAlarm(uint8_t SubMenuItem);
void UI_DrawTimeSubMenuPage(void);
void UI_TimeSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_TimeSubMenuDisplay(uint8_t value);
void UI_DrawTimeSubMenuPage_NoSel(void);

void UI_Zoom_SetScaleParam(uint8_t tZoomScale);
void UI_DrawZoomSubMenuPage(void);
void UI_ZoomSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_ZoomDisplay(void);
void UI_DrawZoomSubMenuPage_NoSel(void);

void UI_CamSubmenuDisplay(void);
void UI_CamSubSubmenuDisplay(uint8_t SubMenuItem);
void UI_CamSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawCamsSubMenuPage_NoSel(void);

void UI_DeletemenuDisplay(void);

void UI_DrawSelectMenuPage(void);

void UI_DrawSubSubMenu_Alarm(void);

void UI_SettingSubmenuDisplay(void);
void UI_DrawSettingSubSubMenuPage(uint8_t SubMenuItem);
void UI_NightModeDisplay(uint8_t value);
void UI_LangageDisplay(uint8_t value);
void UI_FlickerDisplay(uint8_t value);
void UI_DefualtDisplay(uint8_t value);
void UI_TempUnitDisplay(uint8_t value);
void UI_DrawSettingSubMenuPage_NoSel(uint8_t type);
void UI_SNDisplay(void);
void UI_VerDisplay(void);

void UI_NightModeSubSubSubmenuDisplay(uint8_t value);
void UI_NightModeSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);

void UI_SettingSubSubMenuUpKey(uint8_t SubMenuItem);
void UI_SettingSubSubMenuDownKey(uint8_t SubMenuItem);
void UI_SettingSubSubMenuEnterKey(uint8_t SubMenuItem);

void UI_ShowSysVolume(uint8_t value);

void UI_FS_MenuDisplay(uint8_t value);
void UI_FS_LangageMenuDisplay(uint8_t value);
void UI_FS_SetTimeMenuDisplay(uint8_t value);

void UI_UpdateBarIcon_Part1(void);
void UI_UpdateBarIcon_Part2(void);
void UI_Update(void);
void UI_GetVolData(UI_CamNum_t tCamNum, void *pvTrig);
void UI_GetTempData(UI_CamNum_t tCamNum, void *pvTrig);
void UI_GetBUCMDData(UI_CamNum_t tCamNum, void *pvTrig);
void UI_TempBarDisplay(uint8_t value);

void UI_VolBarDisplay(uint8_t value);
void UI_GetPairCamInfo(void);
void UI_TimeShowSystemTime(uint8_t type);
void UI_TimeSetSystemTime(void);
void UI_SetScanMenu(uint8_t value);

void UI_TimerEventStop(void);
void UI_TimerEventStart(uint32_t ulTime_ms, void *pvRegCb);
void UI_AutoBrightnessAdjust(void);

uint8_t UI_TempCToF(uint8_t cTemp);
uint8_t UI_TempFToC(uint8_t fTemp);
uint8_t UI_CheckStopAlarm(void);
uint8_t UI_GetAlarmStatus(void);
#if UI_TEMPMENU
void UI_TempAlarmCheck(void);
#endif
void UI_PickupAlarmCheck(void);

void UI_PTNDisplay(uint8_t value);
void UI_EnableMotor(uint8_t value);
void UI_MotorDisplay(uint8_t value);
void UI_MotorControl(uint8_t Value);
void UI_MotorStateCheck(void);

void UI_GetBatLevel(uint16_t checkcount);
void UI_VolUpKey(void);
void UI_VolDownKey(void);

void UI_PuInit(void);

void UI_TestCmd(uint8_t Value1, uint8_t Value2);
UI_CamNum_t UI_GetPairSelCam(void);
UI_CamNum_t UI_GetCamViewPoolID(void);

void UI_FactoryStatusDisplay(void);
void UI_ClearOSDMenu(void);
void UI_FactorymodeKeyDisplay(uint8_t Value);

void UI_EnterLocalAdoTest_RX(void);
void UI_SendCMDAdoTest_TX(void);

void UI_TimerDeviceEventStart(TIMER_DEVICE_t tDevice, uint32_t ulTime_ms, void *pvRegCb);
void UI_TimerDeviceEventStop(TIMER_DEVICE_t tDevice);
uint8_t UI_GetCamOnLineNum(uint8_t type);
void UI_SwitchCameraScan(uint8_t type);
void UI_PowerOnSet(void);
void UI_CheckUsbCharge(void);
uint8_t UI_GetUsbDet(void);
uint8_t UI_GetBatChgFull(void);
uint8_t UI_CamvLDCModeCmd(uint8_t value);
uint8_t UI_CamNightModeCmd(uint8_t CameraId, uint8_t NightMode);
uint8_t UI_SendNightModeToBu(void);
uint8_t UI_GetBuVersion(void);

uint8_t UI_GetBuMICTest(void);
void UI_FactoryStatusDisplay(void);
uint8_t UI_SendToBUCmd(uint8_t *data, uint8_t data_len);
void UI_EnterLongKey(void);
UI_Result_t UI_SetupPuWorMode(void);
uint8_t UI_GetBuPsMode(void);
void UI_SetSleepState(uint8_t type);
void UI_CheckPowerMode(void);
uint8_t UI_SendAlarmSettingToBu(void);
uint8_t UI_SendPwrNormalModeToBu(void);
void UI_SetSpeaker(uint8_t type, uint8_t State);
void UI_CameraResetCycleTime(uint8_t KeyAction);
void UI_TriggerWakeUpAlarm(void);
void UI_SetFactoryFlag(uint8_t Value);
void UI_CamDeleteCamera(uint8_t type, uint8_t CameraId);
void UI_WakeUp(void);
void UI_PowerOff(void);
void UI_TempUnitSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawTempSubMenuPage_NoSel(void);
void UI_TempUnitSubMenuDisplay(void);

void UI_ShowAlarm(uint8_t type);
void UI_DisplayAlarmSubmenu(void);
void UI_DisplaySquealAlert(void);
void UI_CleanSquealAlert(void);
uint8_t UI_SendPwrVoxModeToBu(void);

void UI_DisablePuAdoOnlyMode(void);
void UI_SwitchAudioSource(UI_CamNum_t tCamNum);

void UI_ExScan(void);
UI_Result_t UI_SetupPuAdoOnlyMode(void);
#endif

