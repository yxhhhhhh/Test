/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VDO.h
	\brief		Video Process Header file for VBM
	\author		Hanyi Chiu
	\version	0.6
	\date		2017/11/27
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _VDO_H_
#define _VDO_H_

#include "APP_CFG.h"
#include "KNL.h"
#include "Buf.h"
#include "SEN.h"

// Default video mode setting
#define VDO_DISP_TYPE					((DISPLAY_MODE == DISPLAY_4T1R)?KNL_DISP_QUAD:(DISPLAY_MODE == DISPLAY_2T1R)?KNL_DISP_DUAL_C:KNL_DISP_SINGLE)
//#define VDO_DISP_TYPE					((DISPLAY_MODE == DISPLAY_4T1R)?KNL_DISP_SINGLE:(DISPLAY_MODE == DISPLAY_2T1R)?KNL_DISP_DUAL_C:KNL_DISP_SINGLE)
#define VDO_DISP_SCAN					((VDO_DISP_TYPE == KNL_DISP_SINGLE)?FALSE:FALSE)

#ifdef VBM_PU
#define VDO_MAIN_H_SIZE					((VDO_DISP_TYPE == KNL_DISP_QUAD)?VGA_WIDTH:HD_WIDTH)
#define VDO_MAIN_V_SIZE					((VDO_DISP_TYPE == KNL_DISP_QUAD)?VGA_HEIGHT:HD_HEIGHT)
#endif
#ifdef VBM_BU
#define VDO_MAIN_H_SIZE					((VDO_DISP_TYPE == KNL_DISP_QUAD)?VGA_WIDTH:HD_WIDTH)
#define VDO_MAIN_V_SIZE					((VDO_DISP_TYPE == KNL_DISP_QUAD)?VGA_HEIGHT:HD_HEIGHT)
#endif

#define VDO_SUB_H_SIZE					VGA_WIDTH
#define VDO_SUB_V_SIZE					VGA_HEIGHT

#ifdef VBM_BU
#define KNL_SenorSetup(KNL_MainSrcNum, KNL_SubSrcNum)																\
										{																			\
											SEN_SetPathSrc(KNL_MainSrcNum, KNL_SRC_NONE, KNL_SubSrcNum);			\
											SEN_SetOutResolution(SENSOR_PATH1, VDO_MAIN_H_SIZE, VDO_MAIN_V_SIZE);	\
											SEN_SetOutResolution(SENSOR_PATH3, VDO_SUB_H_SIZE,  VDO_SUB_V_SIZE);	\
										}
#define KNL_SensorStartProcess()		SEN_InitProcess();
#endif

#ifdef VBM_PU
#define LCD_H_SIZE						HD_WIDTH
#define LCD_V_SIZE						HD_HEIGHT
#define KNL_VdoDisplaySetting()																				\
										{																	\
											KNL_SetDispType(VDO_DISP_TYPE);									\
											KNL_SetDispHV(LCD_H_SIZE, LCD_V_SIZE);							\
											KNL_SetDispRotate(KNL_DISP_ROTATE_90);							\
										}
#define KNL_VdoDisplayParamUpdate()		ubKNL_SetDispCropScaleParam();
#endif

#define KNL_SetVdoResolution(KNL_SrcNum, H_SIZE, V_SIZE)													\
										{																	\
											KNL_SetVdoH(KNL_SrcNum, H_SIZE);								\
											KNL_SetVdoV(KNL_SrcNum, V_SIZE);								\
										}
#define KNL_BufSetup()																						\
										{																	\
											BUF_ResetFreeAddr();											\
											KNL_BufInit();													\
										}

typedef enum
{
	VDO_STOP,
	VDO_START,
}VDO_PlayState_t;

typedef enum
{
	VDO_MAIN_SRC,
	VDO_SUB_SRC,
	VDO_SRC_MAX,
}VDO_SrcType_t;

typedef struct
{
	KNL_SRC		tKNL_SrcNum;
	uint8_t		ubVDO_CodecIdx;
}VDO_KNLRoleParam_t;

typedef struct
{
	VDO_KNLRoleParam_t tVDO_KNLParam[VDO_SRC_MAX];
}VDO_KNLRoleInfo_t;

typedef struct
{
	VDO_PlayState_t   tVdoPlaySte[6];
	KNL_DISP_TYPE	  tVdoDispType;
}VDO_Status_t;

typedef struct
{
	void (*VDO_tPsFunPtr)(void);
}VDO_PsFuncPtr_t;

void VDO_Init(void);
void VDO_Setup(void);
void VDO_Start(void);
void VDO_Stop(void);
#ifdef VBM_BU
void VDO_KNLSysInfoSetup(KNL_ROLE tVDO_KNLRole);
#endif
#ifdef VBM_PU
void VDO_SetPlayRole(KNL_ROLE tKnlRole);
void VDO_UpdateDisplayParameter(void);
void VDO_DisplayLocationSetup(KNL_ROLE tVDO_BURole, KNL_DISP_LOCATION tVDO_DispLocation);
void VDO_SwitchDisplayType(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_BURole);
void VDO_RemoveDataPath(KNL_ROLE tVDO_BURole);
#endif
void VDO_DataPathSetup(KNL_ROLE tVDO_KNLRole, VDO_SrcType_t tVDO_SrcType);
void VDO_ChangePlayState(KNL_ROLE tVDO_KNLRole, VDO_PlayState_t tVdoPlySte);
KNL_SRC VDO_GetSourceNumber(KNL_ROLE tVDO_KNLRole);

#endif
