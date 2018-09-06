/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RC.c
	\brief		Rate Control function
	\author		Justin Chen
	\version	0.6
	\date		2018/07/26
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#include "APP_CFG.h"
#include "BB_API.h"
#include "KNL.h"
#include "SEN.h"
#include "RC.h"
#include "CLI.h"
#include "H264_API.h"
#include "IQ_PARSER_API.h"
#include "USBD_API.h"
#include "APP_HS.h"

uint8_t ubRC_Qp[4][10] = 
{
	42,42,42,42,42,42,42,42,42,42,
	42,42,42,42,42,42,42,42,42,42,
	42,42,42,42,42,42,42,42,42,42,
	42,42,42,42,42,42,42,42,42,42
};

uint8_t ubRC_CurFps = 15;
uint8_t ubRC_IncFlg = 1;

RC_INFO tRC_Info[4];

uint8_t ubRC_FinalMaxQp[4];
uint8_t ubRC_FinalMinQp[4];
uint32_t ulRC_EngModeBitRate[4];
uint8_t ubRC_EngModeFps[4];

void RC_EngModeSet(uint8_t ubCodecIdx,uint32_t ulTarBitRate,uint8_t ubFps)
{
	ulRC_EngModeBitRate[ubCodecIdx] = ulTarBitRate;
	ubRC_EngModeFps[ubCodecIdx]		= ubFps;
}

uint32_t ulRC_GetEngModeBitRate(uint8_t ubCodecIdx)
{
	return ulRC_EngModeBitRate[ubCodecIdx];
}

uint8_t ubRC_GetEngModeFps(uint8_t ubCodecIdx)
{
	return ubRC_EngModeFps[ubCodecIdx];
}

uint32_t ulRC_MaxQP;
uint32_t ulRC_MinQP;

uint8_t ubRC_RefreshRate;
uint8_t ubRC_UpdateFlg[4] = {0,0,0,0};

uint32_t ulRC_FinalBitRate[4];
osThreadId  RC_ThreadId[4] = {NULL,NULL,NULL,NULL};
osThreadId  RC_SysThreadId;

uint8_t ubRC_EnableFlg = 0;

#define RC_MAJORVER    0        //!< Major version = 0
#define RC_MINORVER    6        //!< Minor version = 6
uint16_t uwRC_GetVersion (void)
{
    return ((RC_MAJORVER << 8) + RC_MINORVER);
}

uint32_t ulRC_GetInitBitRate(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ulInitBitRate;
}

uint8_t ubRC_GetTargetQp(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubTargetQp;
}

uint8_t ubRC_GetMinQp(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubMinQp;
}

uint8_t ubRC_GetMaxQp(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubMaxQp;
}

#if OP_STA
uint8_t ubRC_GetAvgQp(uint8_t ubCodecIdx)
{
	uint8_t i;
	uint16_t uwTemp;
	
	uwTemp = 0;
	for(i=0;i<10;i++)
	{
		uwTemp += ubRC_Qp[ubCodecIdx][i];
	}
	
	return (uwTemp/10);
}
#endif

uint8_t ubRC_GetFps(void)
{
	return ubRC_CurFps;
}

void RC_FuncSet(RC_INFO *pInfo)
{		
	tRC_Info[pInfo->ubCodecIdx].ubEnableFlg 		= pInfo->ubEnableFlg;	
	tRC_Info[pInfo->ubCodecIdx].ubCodecIdx			= pInfo->ubCodecIdx;	
	tRC_Info[pInfo->ubCodecIdx].ubMode 				= pInfo->ubMode;
	tRC_Info[pInfo->ubCodecIdx].ubMinQp 			= pInfo->ubMinQp;
	tRC_Info[pInfo->ubCodecIdx].ubMaxQp				= pInfo->ubMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubTargetQp			= pInfo->ubTargetQp;

	tRC_Info[pInfo->ubCodecIdx].ulStepBitRate		= pInfo->ulStepBitRate;	
	
	tRC_Info[pInfo->ubCodecIdx].ubMaxFps	 		= pInfo->ubMaxFps;
	tRC_Info[pInfo->ubCodecIdx].ubMinFps	 		= pInfo->ubMinFps;
	tRC_Info[pInfo->ubCodecIdx].ubStepFps 			= pInfo->ubStepFps;	
	
	tRC_Info[pInfo->ubCodecIdx].ubKeySecRatio 		= pInfo->ubKeySecRatio;
	tRC_Info[pInfo->ubCodecIdx].ubNonKeySecRatio 	= pInfo->ubNonKeySecRatio;
	tRC_Info[pInfo->ubCodecIdx].tAdjSequence 		= pInfo->tAdjSequence;
	tRC_Info[pInfo->ubCodecIdx].ulUpdateRatePerMs 	= pInfo->ulUpdateRatePerMs;
	tRC_Info[pInfo->ubCodecIdx].ubRefreshRate		= pInfo->ubRefreshRate;	
	
	tRC_Info[pInfo->ubCodecIdx].ubQpBuf				= pInfo->ubQpBuf;
	tRC_Info[pInfo->ubCodecIdx].ulBwBuf				= pInfo->ulBwBuf;
	tRC_Info[pInfo->ubCodecIdx].ubFpsBuf			= pInfo->ubFpsBuf;
	
	tRC_Info[pInfo->ubCodecIdx].ulHighQtyTh			= pInfo->ulHighQtyTh;
	tRC_Info[pInfo->ubCodecIdx].ulLowQtyTh			= pInfo->ulLowQtyTh;	
	tRC_Info[pInfo->ubCodecIdx].ubHighBwMaxQp		= pInfo->ubHighBwMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubHighBwMinQp		= pInfo->ubHighBwMinQp;	
	tRC_Info[pInfo->ubCodecIdx].ubMediumBwMaxQp		= pInfo->ubMediumBwMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubMediumBwMinQp		= pInfo->ubMediumBwMinQp;	
	tRC_Info[pInfo->ubCodecIdx].ubLowBwMaxQp		= pInfo->ubLowBwMaxQp;
	tRC_Info[pInfo->ubCodecIdx].ubLowBwMinQp		= pInfo->ubLowBwMinQp;
	
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[0]			= pInfo->ulBwTh[0];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[1]			= pInfo->ulBwTh[1];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[2]			= pInfo->ulBwTh[2];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[3]			= pInfo->ulBwTh[3];	
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[4]			= pInfo->ulBwTh[4];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[5]			= pInfo->ulBwTh[5];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[6]			= pInfo->ulBwTh[6];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[7]			= pInfo->ulBwTh[7];
	tRC_Info[pInfo->ubCodecIdx].ulBwTh[8]			= pInfo->ulBwTh[8];
	
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[0]		= pInfo->ubTargetFps[0];	
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[1]		= pInfo->ubTargetFps[1];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[2]		= pInfo->ubTargetFps[2];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[3]		= pInfo->ubTargetFps[3];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[4]		= pInfo->ubTargetFps[4];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[5]		= pInfo->ubTargetFps[5];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[6]		= pInfo->ubTargetFps[6];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[7]		= pInfo->ubTargetFps[7];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[8]		= pInfo->ubTargetFps[8];
	tRC_Info[pInfo->ubCodecIdx].ubTargetFps[9]		= pInfo->ubTargetFps[9];
	
	tRC_Info[pInfo->ubCodecIdx].ubInitFps			= pInfo->ubInitFps;
	tRC_Info[pInfo->ubCodecIdx].ulInitBitRate		= pInfo->ulInitBitRate;
}

void ubRC_SetFlg(uint8_t ubCodecIdx, uint8_t ubRcCtrlFlg)
{
	tRC_Info[ubCodecIdx].ubEnableFlg = ubRcCtrlFlg;
}

uint8_t ubRC_GetFlg(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubEnableFlg;
}

uint8_t ubRC_GetRefreshRate(uint8_t ubCodecIdx)
{
	return tRC_Info[ubCodecIdx].ubRefreshRate;
}

uint8_t ubRC_GetUpdateFlg(uint8_t ubCodecIdx)
{
	return ubRC_UpdateFlg[ubCodecIdx];
}	

void RC_SetUpdateFlg(uint8_t ubCodecIdx,uint8_t ubFlg)
{
	ubRC_UpdateFlg[ubCodecIdx] = ubFlg;
}	

void RC_Init(uint8_t ubCodecIdx)
{	
	ubRC_UpdateFlg[ubCodecIdx] = 0;	
	
	ulRC_FinalBitRate[ubCodecIdx] = tRC_Info[ubCodecIdx].ulInitBitRate;	
	ubRC_CurFps = tRC_Info[ubCodecIdx].ubInitFps;
	
//	printf("Init->FBR[%d]:%d KB\r\n",ubCodecIdx,ulRC_FinalBitRate[ubCodecIdx]/8192);
//	printf("Init->FPS[%d]:%d \r\n",ubCodecIdx,ubRC_CurFps);
	
	if(tRC_Info[ubCodecIdx].ubEnableFlg)
	{
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);
	}	

	if(RC_SysThreadId == NULL)
	{
	#if OP_STA	
		osThreadDef(RcSysThread, RC_SysThread, THREAD_PRIO_RC_SYS_MONIT, 1, THREAD_STACK_RC_SYS_MONIT);
		RC_SysThreadId = osThreadCreate(osThread(RcSysThread), NULL);
	#endif
	}
	
	if(RC_ThreadId[ubCodecIdx] == NULL)
	{
	#if OP_STA
		if(ubCodecIdx == 0)
		{
			osThreadDef(RcMonitThread0, RC_MonitThread0, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[0] = osThreadCreate(osThread(RcMonitThread0), NULL);
		}
		if(ubCodecIdx == 1)
		{
			osThreadDef(RcMonitThread1, RC_MonitThread1, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[1] = osThreadCreate(osThread(RcMonitThread1), NULL);
		}
		if(ubCodecIdx == 2)
		{
			osThreadDef(RcMonitThread2, RC_MonitThread2, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[2] = osThreadCreate(osThread(RcMonitThread2), NULL);
		}
		if(ubCodecIdx == 3)
		{
			osThreadDef(RcMonitThread3, RC_MonitThread3, THREAD_PRIO_RC_MONIT, 1, THREAD_STACK_RC_MONIT);
			RC_ThreadId[3] = osThreadCreate(osThread(RcMonitThread3), NULL);
		}
	#endif
	}
}

#if OP_STA
void RC_QtyAndFpsProcess1(uint8_t ubCodecIdx)
{
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw = 0,ulN1Bw = 0,ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;
	uint8_t ubUpdateFlg = 0;
	uint8_t ubAdjQtyFlg = 0;
	uint8_t ubAdjFpsFlg = 0;
	uint8_t ubAdjDownFlg = 0;

	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
		#if 0
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
		{
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		}
		else
		{
			ulCurBw = 0;
		}
		#else
		ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		#endif

		ulAvgCurBw = ulCurBw;

		ulBw = (ulAvgCurBw)/8192;

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMaxQp;	
		ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMinQp;
		
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);	
		
		ulN1Bw = ulCurTargetDr/8192;
		
		//======================================================
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}		
		}
		//======================================================
		
		//Case Judgement
		//------------------------------------------------------
		if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) >= ulAvgCurBw)
		{
			//printf("(-)F\r\n");
			ubAdjQtyFlg = 0;
			ubAdjFpsFlg = 1;
			ubAdjDownFlg = 1;
		}
		else if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf >= ulAvgCurBw) && (ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) < ulAvgCurBw))
		{
			//printf("(-)Q\r\n");
			ubAdjQtyFlg = 1;
			ubAdjFpsFlg = 0;		
			ubAdjDownFlg = 1;		
		}
		else if(
		((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) < ulAvgCurBw) &&
		(ubRC_CurFps != tRC_Info[ubCodecIdx].ubMaxFps))	
		{
			//printf("(+)F\r\n");
			ubAdjQtyFlg = 0;
			ubAdjFpsFlg = 1;		
			ubAdjDownFlg = 0;
		}	
		
		//Adjust Quality
		//================================================
		if(ubAdjDownFlg && ubAdjQtyFlg)
		{
			if(ulCurTargetDr > tRC_Info[ubCodecIdx].ulStepBitRate)
			{
				ulCurTargetDr -= tRC_Info[ubCodecIdx].ulStepBitRate;
			}
			else
			{
				ulCurTargetDr = ulCurTargetDr/2;
			}
		}
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;					
		ulN2Bw = ulCurTargetDr/8192;	

		//Adjust FPS
		if(ubAdjDownFlg && ubAdjFpsFlg)
		{		
			if(ubRC_CurFps >= (tRC_Info[ubCodecIdx].ubMinFps + tRC_Info[ubCodecIdx].ubStepFps))
			{
				ubRC_CurFps = ubRC_CurFps - tRC_Info[ubCodecIdx].ubStepFps;
				ubUpdateFlg = 1;
				//printf("(-)New FPS : %d\r\n",ubRC_CurFps);
				
				SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
				if(ubRC_CurFps <= 5)
				{
					IQ_SetAEPID(1);
				}
				else
				{
					IQ_SetAEPID(0);
				}				
				KNL_SetVdoFps(ubRC_CurFps);	
			}
		}
		else if((!ubAdjDownFlg) && ubAdjFpsFlg)
		{
			if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps) <= tRC_Info[ubCodecIdx].ubMaxFps)
			{
				ubRC_CurFps = ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps;
				ubUpdateFlg = 1;
				//printf("(+)New FPS : %d\r\n",ubRC_CurFps);
				
				SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
				if(ubRC_CurFps <= 5)
				{
					IQ_SetAEPID(1);
				}
				else
				{
					IQ_SetAEPID(0);
				}				
				KNL_SetVdoFps(ubRC_CurFps);			
			}
		}

		//======================================================
		if(ubUpdateFlg)
		{
			RC_SetUpdateFlg(ubCodecIdx,1);
		}
		//H264_SetRCParameter(tRC_Info[0].ubCodecIdx,ulCurTargetDr,ubKNL_GetVdoFps());
	}
//	printf("B[%d]:%d_%d_%d_Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ulBw,ulN1Bw,ulN2Bw,ubRC_GetAvgQp(ubCodecIdx),ubRC_CurFps);		
	printd(DBG_CriticalLvl, "B[%d]:%d_%d_%d_Q->%d,F->%d,V->%d,f->%d\r\n",
		tRC_Info[0].ubCodecIdx,
		ulBw,
		ulN1Bw,
		ulN2Bw,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_CurFps,
		ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192,
		ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP));
}

void RC_QtyAndFpsProcess2(uint8_t ubCodecIdx)
{
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw,ulN1Bw = 0, ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;
	uint8_t ubUpdateFlg = 0;
	uint8_t ubTargetFps = 0;
	
	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
		#if 0
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
		{
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		}
		else
		{
			ulCurBw = 0;
		}
		#else
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		#endif
		
		ulAvgCurBw = ulCurBw;
		
		//======================================================
		if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[8])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[9];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[7])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[8];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[6])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[7];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[5])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[6];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[4])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[5];
		}
		
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[3])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[4];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[2])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[3];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[1])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[2];
		}
		else if(ulAvgCurBw >= tRC_Info[ubCodecIdx].ulBwTh[0])
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[1];
		}
		else
		{
			ubTargetFps = tRC_Info[ubCodecIdx].ubTargetFps[0];
		}
		//======================================================
					
		ulBw = (ulAvgCurBw)/8192;	

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMaxQp;	
		ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMinQp;
		
		H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMaxQp);
		H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[ubCodecIdx].ubCodecIdx,tRC_Info[ubCodecIdx].ubMinQp);	
		
		ulN1Bw = ulCurTargetDr/8192;
		
		//======================================================
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}			
		}
		//======================================================
		
		//Adjust Quality
		//------------------------------------------------------
		if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf >= ulAvgCurBw)
		{
			//printf("(-)QTY\r\n");
			if(ulCurTargetDr > tRC_Info[ubCodecIdx].ulStepBitRate)
			{
				ulCurTargetDr -= tRC_Info[ubCodecIdx].ulStepBitRate;
			}		
		}
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;					
		ulN2Bw = ulCurTargetDr/8192;	

		//Adjust FPS
		//------------------------------------------------------	
		if((ubRC_GetAvgQp(ubCodecIdx) <= (tRC_Info[ubCodecIdx].ubTargetQp + tRC_Info[ubCodecIdx].ubQpBuf)) &&
		  ((ubRC_GetAvgQp(ubCodecIdx)+tRC_Info[ubCodecIdx].ubQpBuf) >= tRC_Info[ubCodecIdx].ubTargetQp ))
		{
			if(ubRC_CurFps != ubTargetFps)
			{
				if((ubRC_CurFps > ubTargetFps) && (ubRC_CurFps > tRC_Info[ubCodecIdx].ubStepFps))
				{
					ubRC_CurFps = ubRC_CurFps - tRC_Info[ubCodecIdx].ubStepFps;
					ubUpdateFlg = 1;
					//printf("(1-)New FPS : %d\r\n",ubRC_CurFps);
					
					SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
					if(ubRC_CurFps <= 5)
					{
						IQ_SetAEPID(1);
					}
					else
					{
						IQ_SetAEPID(0);
					}				
					KNL_SetVdoFps(ubRC_CurFps);			
				}
				else if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps) <= (ubTargetFps+tRC_Info[ubCodecIdx].ubFpsBuf))
				{
					ubRC_CurFps = ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps;
					ubUpdateFlg = 1;
					//printf("(1+)New FPS : %d\r\n",ubRC_CurFps);
					
					SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
					if(ubRC_CurFps <= 5)
					{
						IQ_SetAEPID(1);
					}
					else
					{
						IQ_SetAEPID(0);
					}
					KNL_SetVdoFps(ubRC_CurFps);			
				}
			}
		}	
		else if(ubRC_GetAvgQp(ubCodecIdx) > (tRC_Info[ubCodecIdx].ubTargetQp + tRC_Info[ubCodecIdx].ubQpBuf))
		{			
			if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubFpsBuf) > ubTargetFps)
			{
				if((ubRC_CurFps > tRC_Info[ubCodecIdx].ubStepFps) && ((ubRC_CurFps + tRC_Info[ubCodecIdx].ubFpsBuf ) >= (ubTargetFps + tRC_Info[ubCodecIdx].ubStepFps ) ))	
				{
					ubRC_CurFps = ubRC_CurFps - tRC_Info[ubCodecIdx].ubStepFps;
					
					if(ubRC_CurFps <= tRC_Info[ubCodecIdx].ubMinFps)
					{
						ubRC_CurFps = tRC_Info[ubCodecIdx].ubMinFps;
					}
				
					ubUpdateFlg = 1;
					//printf("(2-)New FPS : %d\r\n",ubRC_CurFps);
					
					SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);
					if(ubRC_CurFps <= 5)
					{
						IQ_SetAEPID(1);
					}
					else
					{
						IQ_SetAEPID(0);
					}
					KNL_SetVdoFps(ubRC_CurFps);	
				}
			}
		}
		else
		{		
			if((ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps) <= (ubTargetFps+tRC_Info[ubCodecIdx].ubFpsBuf))
			{
				ubRC_CurFps = ubRC_CurFps + tRC_Info[ubCodecIdx].ubStepFps;
				
				if(ubRC_CurFps >= tRC_Info[ubCodecIdx].ubMaxFps)
				{
					ubRC_CurFps = tRC_Info[ubCodecIdx].ubMaxFps;
				}
				ubUpdateFlg = 1;
				//printf("(2+)New FPS : %d\r\n",ubRC_CurFps);
				
				SEN_SetFrameRate(SENSOR_PATH1, ubRC_CurFps);			
				if(ubRC_CurFps <= 5)
				{
					IQ_SetAEPID(1);
				}
				else
				{
					IQ_SetAEPID(0);
				}
				KNL_SetVdoFps(ubRC_CurFps);	
			}
		}

		//======================================================
		if(ubUpdateFlg)
		{
			RC_SetUpdateFlg(ubCodecIdx,1);
		}
		//H264_SetRCParameter(tRC_Info[0].ubCodecIdx,ulCurTargetDr,ubKNL_GetVdoFps());
	}
//	printf("B[%d]:%d_%d_%d_Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ulBw,ulN1Bw,ulN2Bw,ubRC_GetAvgQp(ubCodecIdx),ubRC_CurFps);		
	printd(DBG_CriticalLvl, "B[%d]:%d_%d_%d_Q->%d,F->%d,V->%d,f->%d\r\n",
		tRC_Info[0].ubCodecIdx,
		ulBw,
		ulN1Bw,
		ulN2Bw,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_CurFps,
		ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192,
		ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP));
}
	
//UNIT -> "Bit" Rate
void RC_QtyAndBwProcess(uint8_t ubCodecIdx)
{
	uint32_t ulAvgCurBw = 0;
	uint32_t ulBw = 0, ulN1Bw = 0, ulN2Bw = 0;
	uint32_t ulCurBw = 0;					//Bit-Rate
	uint32_t ulCurTargetDr = 0;
	uint8_t ubIsKeySecFlg = 0;

	if((ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP)) && (ubKNL_GetCommLinkStatus(KNL_MASTER_AP)))
	{
		#if 0
		if(ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP))
		{
			ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		}
		else
		{
			ulCurBw = 0;
		}
		#else
		ulCurBw = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)*8;
		#endif

		ulAvgCurBw = ulCurBw;	

		ulBw = (ulAvgCurBw)/8192;

		if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_10)
		{
			ulCurTargetDr = ulAvgCurBw/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_20)
		{
			ulCurTargetDr = (ulAvgCurBw*2)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_30)
		{
			ulCurTargetDr = (ulAvgCurBw*3)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_40)
		{
			ulCurTargetDr = (ulAvgCurBw*4)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_50)
		{
			ulCurTargetDr = (ulAvgCurBw*5)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_60)
		{
			ulCurTargetDr = (ulAvgCurBw*6)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_70)
		{
			ulCurTargetDr = (ulAvgCurBw*7)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_80)
		{
			ulCurTargetDr = (ulAvgCurBw*8)/10;
		}
		else if(tRC_Info[ubCodecIdx].ubKeySecRatio == RC_RATIO_90)
		{
			ulCurTargetDr = (ulAvgCurBw*9)/10;
		}
		
		//===========================================================
		if(ulCurTargetDr >= tRC_Info[ubCodecIdx].ulHighQtyTh)
		{				
			if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) <=  ulAvgCurBw)
			{
				ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp;
				ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp;
				
				H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp);
			}
			else if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) >  ulAvgCurBw)	//Heavy Case
			{
				if((tRC_Info[ubCodecIdx].ubHighBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubHighBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
			else //Light Case
			{
				if((tRC_Info[ubCodecIdx].ubHighBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubHighBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubHighBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubHighBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
		}
		else if(ulCurTargetDr <= tRC_Info[ubCodecIdx].ulLowQtyTh)
		{
			if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) <=  ulAvgCurBw)
			{
				ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMaxQp;
				ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMinQp;
				
				H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMinQp);	
			}
			else if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) > ulAvgCurBw)	//Heavy Case
			{
				if((tRC_Info[ubCodecIdx].ubLowBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubLowBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
			else	//Light Case
			{
				if((tRC_Info[ubCodecIdx].ubLowBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubLowBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubLowBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubLowBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
		}
		else
		{
			if((ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx)+tRC_Info[ubCodecIdx].ulBwBuf) <=  ulAvgCurBw)
			{
				ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMaxQp;
				ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMinQp;
				
				H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMinQp);
			}
			else if(ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO,ubCodecIdx) > ulAvgCurBw) //Heavy Case
			{
				if((tRC_Info[ubCodecIdx].ubMediumBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMaxQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubMediumBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf;						
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMinQp+tRC_Info[ubCodecIdx].ubQpBuf);
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}
			else //Light Case
			{
				if((tRC_Info[ubCodecIdx].ubMediumBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMaxQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMaxQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMaxQp[ubCodecIdx] = 51;	
					H264_SetMaxQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}
				
				if((tRC_Info[ubCodecIdx].ubMediumBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2)) <= 51)
				{
					ubRC_FinalMinQp[ubCodecIdx] = tRC_Info[ubCodecIdx].ubMediumBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2);						
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,tRC_Info[ubCodecIdx].ubMediumBwMinQp+(tRC_Info[ubCodecIdx].ubQpBuf/2));
				}
				else
				{
					ubRC_FinalMinQp[ubCodecIdx] = 51;
					H264_SetMinQP((H264_ENCODE_INDEX)ubCodecIdx,51);
				}					
			}		
		}			
		//===========================================================
		
		ulN1Bw = ulCurTargetDr/8192;
		
		//======================================================
		if(ulKNL_GetVdoFrmIdx(ubCodecIdx) <= ubKNL_GetVdoFps())
		{
			ubIsKeySecFlg = 1;			
		}
		else if( (ulKNL_GetVdoGop() > (ubKNL_GetVdoFps()*2))  && (ulKNL_GetVdoFrmIdx(ubCodecIdx) > (ulKNL_GetVdoGop()-(ubKNL_GetVdoFps()*2))) )
		{
			ubIsKeySecFlg = 1;			
		}
		else
		{
			ubIsKeySecFlg = 0;			
		}	

		if(ubIsKeySecFlg)
		{
			//printf("==== KeySec ==== \r\n");
		}
		else
		{			
			if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_10)
			{
				ulCurTargetDr = ulAvgCurBw/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_20)
			{
				ulCurTargetDr = (ulAvgCurBw*2)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_30)
			{
				ulCurTargetDr = (ulAvgCurBw*3)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_40)
			{
				ulCurTargetDr = (ulAvgCurBw*4)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_50)
			{
				ulCurTargetDr = (ulAvgCurBw*5)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_60)
			{
				ulCurTargetDr = (ulAvgCurBw*6)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_70)
			{
				ulCurTargetDr = (ulAvgCurBw*7)/10;
			}
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_80)
			{
				ulCurTargetDr = (ulAvgCurBw*8)/10;
			}	
			else if(tRC_Info[ubCodecIdx].ubNonKeySecRatio == RC_RATIO_90)
			{
				ulCurTargetDr = (ulAvgCurBw*9)/10;
			}		
		}
		//======================================================
			
		
		ulRC_FinalBitRate[ubCodecIdx] = ulCurTargetDr;					
		ulN2Bw = ulCurTargetDr/8192;			
	}
	//RC_SetUpdateFlg(ubCodecIdx,1);	
	//H264_SetRCParameter(tRC_Info[0].ubCodecIdx,ulCurTargetDr,ubKNL_GetVdoFps());	
//	printf("B[%d]:%d_%d_%d_Q->%d,F->%d\r\n",tRC_Info[0].ubCodecIdx,ulBw,ulN1Bw,ulN2Bw,ubRC_GetAvgQp(ubCodecIdx),ubRC_CurFps);	
	printd(DBG_CriticalLvl, "B[%d]:%d_%d_%d_Q->%d,F->%d,V->%d,f->%d\r\n",
		tRC_Info[0].ubCodecIdx,
		ulBw,
		ulN1Bw,
		ulN2Bw,
		ubRC_GetAvgQp(ubCodecIdx),
		ubRC_CurFps,
		ulKNL_GetDataBitRate(KNL_DATA_TYPE_VDO, ubCodecIdx)/8192,
		ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_MASTER_AP));
}
#endif

#if OP_STA
void RC_SysThread(void const *argument)
{	
	uint8_t ubIdx = 10-1;	
	
	while(1)
	{
		ubIdx++;		
		if(ubIdx >= 10)
		{
			ubIdx = 0;
		}
		
		ubRC_Qp[0][ubIdx] = ubKNL_GetQp(0);
		ubRC_Qp[1][ubIdx] = ubKNL_GetQp(1);
		ubRC_Qp[2][ubIdx] = ubKNL_GetQp(2);
		ubRC_Qp[3][ubIdx] = ubKNL_GetQp(3);	
		
		osDelay(100);
	}	
}
#endif

#if OP_STA
void RC_MonitThread0(void const *argument)
{
	uint8_t ubRC_FixPreset0Flg = 0;

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(0))
		{
			if(!ubRC_FixPreset0Flg)
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));	
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset0Flg = 1;
			}
		}
		else if(tRC_Info[0].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[0].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(0);
			ubRC_FixPreset0Flg = 0;
		}
		else if((tRC_Info[0].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[0].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(0);
			ubRC_FixPreset0Flg = 0;
		}
		else if((tRC_Info[0].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[0].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(0);
			ubRC_FixPreset0Flg = 0;
		}
		osDelay(tRC_Info[0].ulUpdateRatePerMs);
	}	
}

void RC_MonitThread1(void const *argument)
{
	uint8_t ubRC_FixPreset1Flg = 0;

	osDelay(1000);
	while(1)
	{	
		if(!ubRC_GetFlg(1))
		{
			if(!ubRC_FixPreset1Flg)
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset1Flg = 1;
			}
		}
		if(tRC_Info[1].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[1].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(1);
			ubRC_FixPreset1Flg = 0;
		}
		else if((tRC_Info[1].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[1].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(1);
			ubRC_FixPreset1Flg = 0;
		}
		else if((tRC_Info[1].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[1].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(1);
			ubRC_FixPreset1Flg = 0;
		}
		osDelay(tRC_Info[1].ulUpdateRatePerMs);
	}	
}

void RC_MonitThread2(void const *argument)
{
	uint8_t ubRC_FixPreset2Flg = 0;

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(2))
		{
			if(!ubRC_FixPreset2Flg)
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset2Flg = 1;
			}
		}
		if(tRC_Info[2].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[2].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(2);
			ubRC_FixPreset2Flg = 0;
		}
		else if((tRC_Info[2].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[2].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(2);
			ubRC_FixPreset2Flg = 0;
		}
		else if((tRC_Info[2].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[2].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(2);
			ubRC_FixPreset2Flg = 0;
		}
		osDelay(tRC_Info[2].ulUpdateRatePerMs);
	}	
}

void RC_MonitThread3(void const *argument)
{
	uint8_t ubRC_FixPreset3Flg = 0;

	osDelay(1000);
	while(1)
	{
		if(!ubRC_GetFlg(3))
		{
			if(!ubRC_FixPreset3Flg)
			{
				SEN_SetFrameRate(SENSOR_PATH1, tRC_Info[0].ubInitFps);
				H264_SetGOP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx, 15);
				H264_SetMaxQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMaxQp);
				H264_SetMinQP((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,tRC_Info[0].ubMinQp);
				//IQ_SetAEPID(((tRC_Info[0].ubInitFps <= 5)?1:0));
				IQ_SetAEPID(((ubRC_GetEngModeFps(0) <= 5)?1:0));
				H264_SetRCParameter((H264_ENCODE_INDEX)tRC_Info[0].ubCodecIdx,ulRC_GetEngModeBitRate(0),ubRC_GetEngModeFps(0));
				ubRC_FixPreset3Flg = 1;
			}
		}
		if(tRC_Info[3].ubMode == RC_MODE_FIXED_DATA_RATE)
		{
		}
		else if(tRC_Info[3].ubMode == RC_MODE_DYNAMIC_QTY_AND_BW)
		{
			RC_QtyAndBwProcess(3);
			ubRC_FixPreset3Flg = 0;
		}
		else if((tRC_Info[3].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[3].tAdjSequence == RC_ADJ_QTY_THEN_FPS))
		{			
			RC_QtyAndFpsProcess1(3);
			ubRC_FixPreset3Flg = 0;
		}
		else if((tRC_Info[3].ubMode == RC_MODE_DYNAMIC_QTY_AND_FPS) && (tRC_Info[3].tAdjSequence == RC_ADJ_FPS_THEN_QTY))
		{			
			RC_QtyAndFpsProcess2(3);
			ubRC_FixPreset3Flg = 0;
		}
		osDelay(tRC_Info[3].ulUpdateRatePerMs);
	}	
}

#endif

uint32_t ulRC_GetFinalBitRate(uint8_t ubCodecIdx)
{
	return ulRC_FinalBitRate[ubCodecIdx];
}

//------------------------------------------------------------------------------
void RC_NoneModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 0;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubTargetQp			= 43;
	
	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_FixedDataRateModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_FIXED_DATA_RATE;
	tRcInfo.ulUpdateRatePerMs	= 1000;		
	tRcInfo.ubMinQp				= 28;
	tRcInfo.ubMaxQp				= 46;	
	tRcInfo.ulInitBitRate		= 0x168000L;	//180KByte
	tRcInfo.ubInitFps			= 15;	

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_QtyAndBwModePreset(void)
{
	RC_INFO tRcInfo;

	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_BW;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_50;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 49;
	tRcInfo.ubQpBuf				= 6;
	tRcInfo.ulInitBitRate		= 0x168000L;	//180KByte
	tRcInfo.ulBwBuf				= 30*1024*8L;
	tRcInfo.ubInitFps			= 15;
	tRcInfo.ulHighQtyTh			= 190*1024*8L;
	tRcInfo.ulLowQtyTh			= 100*1024*8L;	
	tRcInfo.ubHighBwMaxQp		= 43;
	tRcInfo.ubHighBwMinQp		= 29;	
	tRcInfo.ubMediumBwMaxQp		= 46;
	tRcInfo.ubMediumBwMinQp		= 32;
	tRcInfo.ubLowBwMaxQp		= 49;
	tRcInfo.ubLowBwMinQp		= 35;

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_QtyThenFpsModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_FPS;
	tRcInfo.tAdjSequence		= RC_ADJ_QTY_THEN_FPS;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_50;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 46;	
	tRcInfo.ulInitBitRate		= 0x168000L;	//180KByte
	tRcInfo.ulStepBitRate		= 50*1024*8L;
	tRcInfo.ulBwBuf				= 50*1024*8L;	
	tRcInfo.ubInitFps			= 15;
	tRcInfo.ubMaxFps			= 15;
	tRcInfo.ubMinFps			= 4;
	tRcInfo.ubStepFps			= 1;	

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_FpsThenQtyModePreset(void)
{
	RC_INFO tRcInfo;
	
	tRcInfo.ubEnableFlg 		= 1;
	tRcInfo.ubCodecIdx			= 0;
	tRcInfo.ubMode				= RC_MODE_DYNAMIC_QTY_AND_FPS;
	tRcInfo.tAdjSequence		= RC_ADJ_FPS_THEN_QTY;
	tRcInfo.ulUpdateRatePerMs	= 1000;
	tRcInfo.ubRefreshRate		= 1;
	tRcInfo.ubKeySecRatio		= RC_RATIO_70;
	tRcInfo.ubNonKeySecRatio	= RC_RATIO_50;
	tRcInfo.ubMinQp				= 29;
	tRcInfo.ubMaxQp				= 46;
	tRcInfo.ubTargetQp			= 41;
	tRcInfo.ubQpBuf				= 2;
	tRcInfo.ulInitBitRate		= 0x168000L; //180KByte
	tRcInfo.ulStepBitRate		= 50*1024*8L;
	tRcInfo.ulBwBuf				= 50*1024*8L;
	tRcInfo.ulBwTh[0]			=  30*1024*8L;
	tRcInfo.ulBwTh[1]			=  60*1024*8L;
	tRcInfo.ulBwTh[2]			=  90*1024*8L;
	tRcInfo.ulBwTh[3]			= 120*1024*8L;
	tRcInfo.ulBwTh[4]			= 150*1024*8L;
	tRcInfo.ulBwTh[5]			= 180*1024*8L;
	tRcInfo.ulBwTh[6]			= 210*1024*8L;
	tRcInfo.ulBwTh[7]			= 240*1024*8L;
	tRcInfo.ulBwTh[8]			= 270*1024*8L;
	tRcInfo.ubInitFps			= 15;
	tRcInfo.ubMaxFps			= 15;
	tRcInfo.ubMinFps			= 5;
	tRcInfo.ubStepFps			= 2;
	tRcInfo.ubFpsBuf			= 7;
	tRcInfo.ubTargetFps[0]		= 4;
	tRcInfo.ubTargetFps[1]		= 5;
	tRcInfo.ubTargetFps[2]		= 6;
	tRcInfo.ubTargetFps[3]		= 7;
	tRcInfo.ubTargetFps[4]		= 9;
	tRcInfo.ubTargetFps[5]		= 11;
	tRcInfo.ubTargetFps[6]		= 12;
	tRcInfo.ubTargetFps[7]		= 13;
	tRcInfo.ubTargetFps[8]		= 14;
	tRcInfo.ubTargetFps[9]		= 15;

	RC_FuncSet(&tRcInfo);
}
//------------------------------------------------------------------------------
void RC_PresetSetup(RC_Rreset_t tRC_Preset)
{
	RC_PresetSetup_t tRC_PresetFunc[RC_PRESET_MAX] =
	{		
		[RC_NONE]				= RC_NoneModePreset,
		[RC_FIXED_DATA_RATE] 	= RC_FixedDataRateModePreset,
		[RC_QTY_AND_BW] 		= RC_QtyAndBwModePreset,
		[RC_QTY_THEN_FPS] 		= RC_QtyThenFpsModePreset,
		[RC_FPS_THEN_QTY] 		= RC_FpsThenQtyModePreset,
	};

	if(tRC_PresetFunc[tRC_Preset].pvPresetFunc)
		tRC_PresetFunc[tRC_Preset].pvPresetFunc();
}
