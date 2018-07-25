/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VDO.c
	\brief		Video Process for VBM
	\author		Hanyi Chiu
	\version	0.7
	\date		2017/11/29
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#if defined(VBM_PU) || defined(VBM_BU)

#include "VDO.h"

static VDO_KNLRoleInfo_t tVDO_KNLRoleInfo[] = 
{
#ifdef VBM_BU
	[KNL_STA1] = { { { KNL_SRC_1_MAIN, ENCODE_0 }, { KNL_SRC_1_AUX, ENCODE_1 } } },
	[KNL_STA2] = { { { KNL_SRC_2_MAIN, ENCODE_0 }, { KNL_SRC_2_AUX, ENCODE_1 } } },
	[KNL_STA3] = { { { KNL_SRC_3_MAIN, ENCODE_0 }, { KNL_SRC_3_AUX, ENCODE_1 } } },
	[KNL_STA4] = { { { KNL_SRC_4_MAIN, ENCODE_0 }, { KNL_SRC_4_AUX, ENCODE_1 } } },
	[KNL_NONE] = { { { KNL_SRC_NONE,   ENCODE_0 }, { KNL_SRC_NONE,  ENCODE_1 } } },
#endif
#ifdef VBM_PU
	[KNL_STA1] = { { { KNL_SRC_1_MAIN, DECODE_0 }, { KNL_SRC_1_AUX, DECODE_0 } } },
	[KNL_STA2] = { { { KNL_SRC_2_MAIN, DECODE_1 }, { KNL_SRC_2_AUX, DECODE_1 } } },
	[KNL_STA3] = { { { KNL_SRC_3_MAIN, DECODE_2 }, { KNL_SRC_3_AUX, DECODE_2 } } },
	[KNL_STA4] = { { { KNL_SRC_4_MAIN, DECODE_3 }, { KNL_SRC_4_AUX, DECODE_3 } } },
	[KNL_NONE] = { { { KNL_SRC_NONE,   DECODE_0 }, { KNL_SRC_NONE,  DECODE_0 } } },
#endif
};
#ifdef VBM_PU
static VDO_Status_t tVDO_Status;
static KNL_ROLE tVDO_SvPlayRole;
static KNL_ROLE tKNL_DualBURole[2];
static uint8_t ubVDO_PathRstFlag;
extern uint8_t ubSwitchCamWakeupSstate;
#endif
#ifdef VBM_BU
static uint8_t ubVDO_SysSetupFlag;
#endif
static uint16_t uwVDO_HSIZE;
static uint16_t uwVDO_VSIZE;
//------------------------------------------------------------------------------
void VDO_Init(void)
{
	int i = 0;
	KNL_ROLE tVDO_KNLRole;
	KNL_SRC tVDO_KNLMainSrcNum;
#ifdef VBM_BU
	KNL_SRC tVDO_KNLSubSrcNum;
#endif

	KNL_SetVdoCodec(KNL_VDO_CODEC_H264);
	KNL_SetVdoFps(VDO_FRAME_RATE);
#ifdef VBM_BU
	ubVDO_SysSetupFlag = FALSE;
	tVDO_KNLRole = (KNL_ROLE)ubKNL_GetRole();
	tVDO_KNLRole = (KNL_NONE != tVDO_KNLRole)?tVDO_KNLRole:KNL_STA1;
#endif
#ifdef VBM_PU
	ubVDO_PathRstFlag  = (DISPLAY_MODE != DISPLAY_1T1R)?TRUE:FALSE;
	tVDO_SvPlayRole    = (VDO_DISP_TYPE == KNL_DISP_SINGLE)?KNL_STA1:KNL_NONE;
	tKNL_DualBURole[0] = KNL_NONE;
	tKNL_DualBURole[1] = KNL_NONE;
	//! Display Setting
	tVDO_Status.tVdoDispType = VDO_DISP_TYPE;
	KNL_VdoDisplaySetting();
	KNL_SetPlyMode(KNL_NORMAL_PLY);				//!< Normal Play
#endif
	uwVDO_HSIZE = VDO_MAIN_H_SIZE;
	uwVDO_VSIZE = VDO_MAIN_V_SIZE;
	//! Data Path Setting
	KNL_VdoPathReset();
	for(i = 0; i < 256; i++)
		KNL_SetMultiOutNode(i, 0, 0, 0, 0);		//!< Disable Multi Out Feature
	KNL_SetVdoRoleInfoCbFunc(VDO_GetSourceNumber);
#ifdef VBM_BU
	tVDO_KNLMainSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
	tVDO_KNLSubSrcNum  = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
	KNL_SetVdoResolution(tVDO_KNLMainSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
	KNL_SetVdoResolution(tVDO_KNLSubSrcNum, VDO_SUB_H_SIZE, VDO_SUB_V_SIZE);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_SUB_SRC);
	KNL_SenorSetup(tVDO_KNLMainSrcNum, tVDO_KNLSubSrcNum);
#endif
#ifdef VBM_PU
	for(tVDO_KNLRole = KNL_STA1; tVDO_KNLRole < DISPLAY_MODE; tVDO_KNLRole++)
	{
		tVDO_KNLMainSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
		KNL_SetVdoResolution(tVDO_KNLMainSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
		VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
		tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_STOP;
	}
#endif
}
//------------------------------------------------------------------------------
#ifdef VBM_BU
void VDO_KNLSysInfoSetup(KNL_ROLE tVDO_KNLRole)
{
	KNL_SRC tVDO_KNLMainSrcNum, tVDO_KNLSubSrcNum;

	KNL_SetRole(tVDO_KNLRole);
	KNL_VdoPathReset();
	tVDO_KNLMainSrcNum	= tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
	tVDO_KNLSubSrcNum 	= tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
	KNL_SetVdoResolution(tVDO_KNLMainSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
	KNL_SetVdoResolution(tVDO_KNLSubSrcNum, VDO_SUB_H_SIZE, VDO_SUB_V_SIZE);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_SUB_SRC);
	KNL_ImageEncodeSetup(tVDO_KNLMainSrcNum);
	KNL_ImageEncodeSetup(tVDO_KNLSubSrcNum);
	KNL_SenorSetup(tVDO_KNLMainSrcNum, tVDO_KNLSubSrcNum);
	if((KNL_SRC_NONE != tVDO_KNLMainSrcNum) || (KNL_SRC_NONE != tVDO_KNLSubSrcNum))
		KNL_SensorStartProcess();
	ubVDO_SysSetupFlag = TRUE;
}
#endif
//------------------------------------------------------------------------------
#ifdef VBM_PU
void VDO_SetPlayRole(KNL_ROLE tKnlRole)
{
	tVDO_SvPlayRole = tKnlRole;
}
//------------------------------------------------------------------------------
void VDO_UpdateDisplayParameter(void)
{
#if	(DISPLAY_MODE != DISPLAY_1T1R)
	KNL_ROLE tKNL_Role = tVDO_SvPlayRole;
	KNL_DISP_TYPE tVDO_DisplayType = tVDO_Status.tVdoDispType;

	tVDO_SvPlayRole = KNL_NONE;
	tVDO_Status.tVdoDispType = KNL_DISP_NONSUP;
	VDO_SwitchDisplayType(tVDO_DisplayType, &tKNL_Role);
#endif
}
//------------------------------------------------------------------------------
void VDO_DisplayLocationSetup(KNL_ROLE tVDO_BURole, KNL_DISP_LOCATION tVDO_DispLocation)
{
	KNL_DISP_LOCATION tVDO_DispLoc;

	tVDO_DispLoc = tVDO_DispLocation;
#if	(DISPLAY_MODE == DISPLAY_1T1R)
	if(tKNL_GetDispType() == KNL_DISP_SINGLE)
		tVDO_DispLoc = KNL_DISP_LOCATION1;
#endif
	KNL_SetDispSrc(tVDO_DispLoc, tVDO_KNLRoleInfo[tVDO_BURole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
}
//------------------------------------------------------------------------------
void VDO_SwitchDisplayType(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_BURole)
{
	KNL_ROLE tKNL_Role;	
	KNL_SRC tVDO_KNLSrcNum;
	KNL_SrcLocateMap_t tVDO_KNLSrcLocate;
	uint16_t uwHSize = 0, uwVSize = 0;
	uint8_t ubVDO_ResChgFlag = FALSE;
	static uint8_t ubVDO_DualPathFlag = FALSE;

	uwHSize = (KNL_DISP_QUAD == tVDO_DisplayType)?VGA_WIDTH:HD_WIDTH;
	uwVSize = (KNL_DISP_QUAD == tVDO_DisplayType)?VGA_HEIGHT:HD_HEIGHT;
	if((uwVDO_HSIZE != uwHSize) || (uwVDO_VSIZE != uwVSize))
	{
		uwVDO_HSIZE = uwHSize;
		uwVDO_VSIZE = uwVSize;
		ubVDO_ResChgFlag = TRUE;
	}
	switch(tVDO_DisplayType)
	{
		case KNL_DISP_SINGLE:
		{
			KNL_ROLE tVDO_KNLRole = KNL_NONE;

			tVDO_KNLRole = *pVDO_BURole;
			if(ubSwitchCamWakeupSstate == 1)
			{
				ubSwitchCamWakeupSstate = 0;
			}
			else
			{
				if((tVDO_SvPlayRole == tVDO_KNLRole) && (FALSE == ubVDO_DualPathFlag))
					break;
			}
			tVDO_KNLSrcLocate.ubSetupFlag = FALSE;
			tVDO_KNLSrcLocate.tSrcNum[0]  = tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			if(((TRUE == ubVDO_ResChgFlag) || (TRUE == ubVDO_PathRstFlag)) &&
			   ((tKNL_DualBURole[0] != tVDO_KNLRole) && (tKNL_DualBURole[1] != tVDO_KNLRole)))
			{
				VDO_Stop();
				KNL_VdoPathReset();
				KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
				VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
				KNL_ModifyDispType(KNL_DISP_SINGLE, tVDO_KNLSrcLocate);
				KNL_BufSetup();
				KNL_ImageDecodeSetup(tVDO_KNLSrcNum);
				ubVDO_PathRstFlag  = TRUE;
				ubVDO_DualPathFlag = FALSE;
				tKNL_DualBURole[0] = tKNL_DualBURole[1] = KNL_NONE;
			}
			else
			{
				KNL_ModifyDispType(KNL_DISP_SINGLE, tVDO_KNLSrcLocate);
				for(tKNL_Role = KNL_STA1; tKNL_Role < DISPLAY_MODE; tKNL_Role++)
				{
					if((tKNL_Role != tVDO_KNLRole) && (VDO_START == tVDO_Status.tVdoPlaySte[tKNL_Role]))
					{
						tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
						KNL_VdoStop(tVDO_KNLSrcNum);
						ubKNL_WaitNodeFinish(tVDO_KNLSrcNum);
						tVDO_Status.tVdoPlaySte[tKNL_Role] = VDO_STOP;
					}
				}
			}
			if(VDO_STOP == tVDO_Status.tVdoPlaySte[tVDO_KNLRole])
			{
				tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
				KNL_VdoStart(tVDO_KNLSrcNum);
				tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_START;
			}
			tVDO_SvPlayRole = tVDO_KNLRole;
			break;
		}
#if (DISPLAY_MODE == DISPLAY_4T1R)
		case KNL_DISP_DUAL_C:
		case KNL_DISP_DUAL_U:
		{
			uint8_t ubRoleIdx;

			tVDO_KNLSrcLocate.ubSetupFlag = TRUE;
			for(ubRoleIdx = 0; ubRoleIdx < 2; ubRoleIdx++)
			{
				tKNL_DualBURole[ubRoleIdx] = *(pVDO_BURole + ubRoleIdx);
				tVDO_KNLSrcLocate.tSrcNum[ubRoleIdx] = tVDO_KNLRoleInfo[tKNL_DualBURole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			}
			tVDO_KNLSrcLocate.tSrcLocate[0] = KNL_DISP_LOCATION2;
			tVDO_KNLSrcLocate.tSrcLocate[1] = KNL_DISP_LOCATION1;
			if((TRUE == ubVDO_ResChgFlag) || (TRUE == ubVDO_PathRstFlag))
			{
				VDO_Stop();
				KNL_VdoPathReset();
				for(ubRoleIdx = 0; ubRoleIdx < 2; ubRoleIdx++)
				{
					tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_DualBURole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
					KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
					VDO_DataPathSetup(tKNL_DualBURole[ubRoleIdx], VDO_MAIN_SRC);
				}
				KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
				KNL_BufSetup();
				for(ubRoleIdx = 0; ubRoleIdx < 2; ubRoleIdx++)
					KNL_ImageDecodeSetup(tVDO_KNLRoleInfo[tKNL_DualBURole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			}
			else
			{
				KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
				for(tKNL_Role = KNL_STA1; tKNL_Role < DISPLAY_MODE; tKNL_Role++)
				{
					if(((tKNL_Role != tKNL_DualBURole[0]) && (tKNL_Role != tKNL_DualBURole[1])) &&
					   (VDO_START == tVDO_Status.tVdoPlaySte[tKNL_Role]))
					{
						tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
						KNL_VdoStop(tVDO_KNLSrcNum);
						ubKNL_WaitNodeFinish(tVDO_KNLSrcNum);
						tVDO_Status.tVdoPlaySte[tKNL_Role] = VDO_STOP;
					}
				}
			}
			for(ubRoleIdx = 0; ubRoleIdx < 2; ubRoleIdx++)
			{
				if(VDO_STOP == tVDO_Status.tVdoPlaySte[tKNL_DualBURole[ubRoleIdx]])
				{
					tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_DualBURole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
					KNL_VdoStart(tVDO_KNLSrcNum);
					tVDO_Status.tVdoPlaySte[tKNL_DualBURole[ubRoleIdx]] = VDO_START;
				}
			}
			ubVDO_PathRstFlag  = (TRUE == ubVDO_ResChgFlag)?TRUE:ubVDO_PathRstFlag;
			ubVDO_DualPathFlag = TRUE;
			break;
		}
#endif
		default:
			if(tVDO_DisplayType == tVDO_Status.tVdoDispType)
				break;
			if ((tVDO_DisplayType == KNL_DISP_DUAL_C) || (tVDO_DisplayType == KNL_DISP_DUAL_U)) {
				tVDO_KNLSrcLocate.ubSetupFlag = TRUE;
				tVDO_KNLSrcLocate.tSrcNum[0] = KNL_SRC_1_MAIN;
				tVDO_KNLSrcLocate.tSrcNum[1] = KNL_SRC_2_MAIN;
				tVDO_KNLSrcLocate.tSrcLocate[0] = KNL_DISP_LOCATION2;
				tVDO_KNLSrcLocate.tSrcLocate[1] = KNL_DISP_LOCATION1;
			} else {
				tVDO_KNLSrcLocate.ubSetupFlag = FALSE;
			}
			if((TRUE == ubVDO_ResChgFlag) || (TRUE == ubVDO_PathRstFlag))
			{
				VDO_Stop();
				KNL_VdoPathReset();
				for(tKNL_Role = KNL_STA1; tKNL_Role < DISPLAY_MODE; tKNL_Role++)
				{
					tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
					KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
					VDO_DataPathSetup(tKNL_Role, VDO_MAIN_SRC);
				}
				KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
				KNL_BufSetup();
				for(tKNL_Role = KNL_STA1; tKNL_Role < DISPLAY_MODE; tKNL_Role++)
					KNL_ImageDecodeSetup(tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			} else {
				KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
			}
			for(tKNL_Role = KNL_STA1; tKNL_Role < DISPLAY_MODE; tKNL_Role++)
			{
				if(VDO_STOP == tVDO_Status.tVdoPlaySte[tKNL_Role])
				{
					tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
					KNL_VdoStart(tVDO_KNLSrcNum);
					tVDO_Status.tVdoPlaySte[tKNL_Role] = VDO_START;
				}
			}
			tVDO_SvPlayRole    = KNL_NONE;
			tKNL_DualBURole[0] = tKNL_DualBURole[1] = KNL_NONE;
			ubVDO_PathRstFlag  = FALSE;
			ubVDO_DualPathFlag = FALSE;
			break;
	}
	tVDO_Status.tVdoDispType = tVDO_DisplayType;
}
//------------------------------------------------------------------------------
void VDO_RemoveDataPath(KNL_ROLE tVDO_BURole)
{
	KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_BURole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_BURole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
	KNL_VdoPathNodeReset(tVDO_KNLRoleInfo[tVDO_BURole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	tVDO_Status.tVdoPlaySte[tVDO_BURole] = VDO_STOP;
}
#endif
//------------------------------------------------------------------------------
void VDO_DataPathSetup(KNL_ROLE tVDO_KNLRole, VDO_SrcType_t tVDO_SrcType)
{
#ifdef VBM_BU
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_SEN, KNL_NODE_SEN_YUV_BUF, KNL_NODE_H264_ENC, KNL_NODE_VDO_BS_BUF1, KNL_NODE_COMM_TX_VDO, KNL_NODE_END};
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_SEN, KNL_NODE_SEN_YUV_BUF, KNL_NODE_H264_ENC, KNL_NODE_VDO_BS_BUF1, KNL_NODE_END};
#endif
#ifdef VBM_PU
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_LCD, KNL_NODE_END};
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_LCD, KNL_NODE_END};
#endif
	KNL_NODE_INFO tVDO_KNLNodeInfo = {0};
	KNL_NODE *pVDO_KNLPath[VDO_SRC_MAX] = {[VDO_MAIN_SRC] = tVDO_KNLMainPath,
										   [VDO_SUB_SRC]  = tVDO_KNLSubPath};
	KNL_SRC tVDO_KNLSrcNum;
	uint16_t uwVDO_NodeNum, i;
	uint8_t ubVDO_CodecIdx;

	tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[tVDO_SrcType].tKNL_SrcNum;
	ubVDO_CodecIdx = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[tVDO_SrcType].ubVDO_CodecIdx;
	uwVDO_NodeNum  = ((tVDO_SrcType == VDO_MAIN_SRC)?(sizeof tVDO_KNLMainPath):(sizeof tVDO_KNLSubPath)) / sizeof(KNL_NODE);
	for(i = 0; i < uwVDO_NodeNum; i++)
	{
		tVDO_KNLNodeInfo.ubPreNode	= (0 == i)?KNL_NODE_NONE:pVDO_KNLPath[tVDO_SrcType][i-1];
		tVDO_KNLNodeInfo.ubCurNode 	= pVDO_KNLPath[tVDO_SrcType][i];
		tVDO_KNLNodeInfo.ubNextNode = (uwVDO_NodeNum == (i+1))?KNL_NODE_NONE:pVDO_KNLPath[tVDO_SrcType][i+1];
		tVDO_KNLNodeInfo.ubCodecIdx = ubVDO_CodecIdx;
		tVDO_KNLNodeInfo.uwVdoH		= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:uwKNL_GetVdoH(tVDO_KNLSrcNum);
		tVDO_KNLNodeInfo.uwVdoV		= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:uwKNL_GetVdoV(tVDO_KNLSrcNum);
		tVDO_KNLNodeInfo.ubHMirror	= 0;
		tVDO_KNLNodeInfo.ubVMirror	= 0;
		tVDO_KNLNodeInfo.ubRotate	= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_H264_DEC)?1:0;
		tVDO_KNLNodeInfo.ubHScale	= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:KNL_SCALE_X1;
		tVDO_KNLNodeInfo.ubVScale	= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:KNL_SCALE_X1;
		ubKNL_SetVdoPathNode(tVDO_KNLSrcNum, i, tVDO_KNLNodeInfo);
	}
}
//------------------------------------------------------------------------------
void VDO_ChangePlayState(KNL_ROLE tVDO_KNLRole, VDO_PlayState_t tVdoPlySte)
{
	switch(tVdoPlySte)
	{
		#ifdef VBM_PU
		case VDO_START:
			KNL_VdoResume(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			break;
		#endif
		case VDO_STOP:
		#ifdef VBM_BU
			KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		#endif
		#ifdef VBM_PU
			KNL_VdoSuspend(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		#endif
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
KNL_SRC VDO_GetSourceNumber(KNL_ROLE tVDO_KNLRole)
{
	return tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
}
//------------------------------------------------------------------------------
void VDO_Start(void)
{
#ifdef VBM_BU
	KNL_ROLE tVDO_KNLRole = (KNL_ROLE)ubKNL_GetRole();

	if(TRUE == ubVDO_SysSetupFlag)
	{
		KNL_BufSetup();
		ubVDO_SysSetupFlag = FALSE;
	}
	if(tVDO_KNLRole <= KNL_STA4)
	{
		KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
//		KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
	}
#endif
#ifdef VBM_PU
	KNL_ROLE tVDO_KNLRole, tVDO_KNLSRole = (KNL_DISP_SINGLE == tVDO_Status.tVdoDispType)?tVDO_SvPlayRole:KNL_STA1;
	uint8_t ubKNL_RoleNum = (KNL_DISP_SINGLE == tVDO_Status.tVdoDispType)?tVDO_SvPlayRole:DISPLAY_MODE;

	for(tVDO_KNLRole = tVDO_KNLSRole; tVDO_KNLRole <= ubKNL_RoleNum; tVDO_KNLRole++)
	{
		if(VDO_STOP == tVDO_Status.tVdoPlaySte[tVDO_KNLRole])
		{
			KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_START;
		}
	}
#endif
}
//------------------------------------------------------------------------------
void VDO_Stop(void)
{
#ifdef VBM_BU
	KNL_ROLE tVDO_KNLRole = (KNL_ROLE)ubKNL_GetRole();

	if(tVDO_KNLRole <= KNL_STA4)
	{
		KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
		ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
	}
#endif
#ifdef VBM_PU
	KNL_ROLE tVDO_KNLRole;

	for(tVDO_KNLRole = KNL_STA1; tVDO_KNLRole < DISPLAY_MODE; tVDO_KNLRole++)
	{
		if(VDO_START == tVDO_Status.tVdoPlaySte[tVDO_KNLRole])
		{
			KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_STOP;
		}
	}
#endif
}
//------------------------------------------------------------------------------
#endif //! End #if defined VBM_PU || defined VBM_BU
