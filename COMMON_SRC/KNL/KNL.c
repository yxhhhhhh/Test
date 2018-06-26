/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		KNL.c
	\brief		Kernel Control function
	\author		Justin Chen
	\version	1.7
	\date		2018/05/18
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include "KNL.h"
#include "BUF.h"
#include "APP_CFG.h"
#include "BB_API.h"
#include "EN_API.h"
#include "TWC_API.h"
#include "H264_API.h"
#include "IMG_API.h"
#include "ISP_API.h"
#include "JPEG_API.h"
#include "CRC_API.h"
#include "DMAC_API.h"
#include "USBD_API.h"
#include "USBH_API.h"
#include "RTC_API.h"
#include "LCD.h"
#include "SEN.h"
#include "TIMER.h"
#include "RC.h"
#include "BSP.h"
#include "VDO.h"

#include "SD_API.h"
#include "FS_API.h"
#include "REC_API.h"
#include "PLY_API.h"
#include "MEDIA.h"

//------------------------------------------------------------------------------
/*!	\file KNL.c	
KNL Initial Flow Chart:	
	\dot
		digraph G {
	node [shape=record,fontsize=10];
	"Start" -> "Set System Information";	
	node [shape=record,fontsize=10];
	"Set System Information" -> "Set Data Path";
	node [shape=record,fontsize=10];
	"Set Data Path" -> "KNL_Init";
	node [shape=record,fontsize=10];
	"KNL_Init" -> "KNL_BufInit";
	node [shape=record,fontsize=10];
	"KNL_BufInit" -> "KNL_BlockInit";	
	}
	\enddot
*/
//------------------------------------------------------------------------------
osMutexId tKNL_LinkSem;
uint8_t ubKNL_BbRtyFlg = 0;
uint8_t ubKNL_ResetIFlg = 0;
uint8_t ubKNL_SenStartFlg = 0;
uint8_t ubKNL_ImgStabFlg = 0;
uint8_t ubKNL_ChgResFlg = 0;
KNL_SrcLocateMap_t KNL_SwDispInfo;

//For Process
KNL_INFO tKNL_Info;
osMessageQId KNL_VdoInProcQueue;
osMessageQId KNL_VdoCodecProcQueue;
osMessageQId KNL_AdoCdoecProcQueue;
osMessageQId KNL_CommTxProcQueue;

//For AVG
osMessageQId KNL_TwcMonitQueue;
osMessageQId KNL_AvgPlyQueue[4];
osThreadId KNL_AvgThreadId[4];
uint8_t ubKNL_AvgPlyStartFlg[4] = {0,0,0,0};
uint8_t ubKNL_AvgPlyCnt[4] = {0,0,0,0};
uint8_t ubKNL_AvgPlyStartNum = 4;

//For Block Init
uint8_t ubKNL_InitBBFlg = 0;
uint8_t ubKNL_InitImgFlg = 0;
uint8_t ubKNL_InitAdoFlg = 0;
uint8_t ubKNL_InitJpegFlg = 0;

//For Jpeg
uint8_t ubKNL_JpegPreNode;	//For BUC JPEG IP
uint8_t ubKNL_JpegSrc;			//For BUC JPEG IP
osMessageQId KNL_QueueJpegMonit;
osSemaphoreId JPEG_CodecSem;

//For Img/H264
uint8_t ubKNL_ImgSrc;			//For IMG/H264 IP
uint8_t ubKNL_ImgTrigSrc;		//For IMG Merge
uint8_t ubKNL_ImgTrigType;		//For IMG Merge
osSemaphoreId tKNL_ImgSem;
osMutexId tKNL_BsBufMutex;
uint32_t ulKNL_CurFrmIdx[4];
uint8_t ubKNL_Qp[4] = {42,42,42,42};
uint8_t ubKNL_VdoChkSrcNumFlg[4] = {0,0,0,0};
uint8_t ubKNL_ImgBusyFlg;
pvRoleSrcMap ptKNL_VdoRoleSrcMapT;

//For Node
KNL_NODE_STATE tKNL_NodeState[KNL_SRC_NUM][256];
KNL_NODE_INFO tKNL_VdoNodeInfo[KNL_SRC_NUM][KNL_MAX_NODE_NUM];		//For Video
KNL_NODE_INFO tKNL_AdoNodeInfo[KNL_SRC_NUM][KNL_MAX_NODE_NUM];		//For Audio

//For Communication
osSemaphoreId tKNL_TwcSem;
osMessageQId KNL_LinkQ;
osMessageQId KNL_QueBbFrmOk;
osMessageQId KNL_QueTxVdo;
osMessageQId KNL_QueTxAdo;
osMessageQId KNL_QueRxVdo;
osMessageQId KNL_QueRxAdo;
uint8_t ubKNL_LinkStatus[6] = {BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK};
uint8_t ubKNL_RtLinkStatus[6] = {BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK,BB_LOST_LINK};
KNL_COMM_STATE tKNL_CommState = KNL_COMM_STATE_STOP;
static uint8_t ubKNL_WakeUpFlag[KNL_MAX_ROLE] = {0,0,0,0,0,0,0};

//For Audio
ADO_KNL_PARA_t tKNL_AdoInfo;
uint32_t ulKNL_DacStartToPlayCnt = 0;
uint8_t ubKNL_DacStartToPlayFlg = 0;
uint8_t ubKNL_AdcFlowActiveFlg = 0;
uint8_t ubKNL_DacFlowActiveFlg[KNL_SRC_NUM];
pvRoleSrcMap ptKNL_AdoRoleSrcMapT;
osMessageQId tKNL_EncEventQue;		
osMessageQId tKNL_DecEventQue;

//For Video
uint8_t ubKNL_ImgRdy = 1;
uint8_t ubKNL_VdoFlowActiveFlg[KNL_SRC_NUM];
uint8_t ubKNL_RcvFirstIFrame[KNL_SRC_NUM];
uint8_t ubKNL_VdoGroupIdx[4] = {0,0,0,0};
uint8_t ubKNL_VdoBsBusyCnt[4] = {0,0,0,0};
uint8_t ubKNL_VdoResendITwcFlg[4] = {FALSE, FALSE, FALSE, FALSE};
uint8_t ubKNL_VdoResChgTwcFlg[4] = {FALSE, FALSE, FALSE, FALSE};

//For Display
LCD_BUF_TYP *pLcdCh0Buf,*pLcdCh1Buf,*pLcdCh2Buf,*pLcdCh3Buf;
uint8_t ubKNL_DispCh0ActiveFlg=1,ubKNL_DispCh1ActiveFlg=1,ubKNL_DispCh2ActiveFlg=1,ubKNL_DispCh3ActiveFlg=1;
LCD_INFOR_TYP *pLcdCropScaleParam;
uint8_t ubKNL_LcdDispParamActiveFlg=1;

//For TWC
uint8_t ubKNL_TwcEndFlg = 1;
uint8_t ubKNL_TwcResult = 0;

//For Status Report
uint32_t ulKNL_VdoOutAccCnt[4] = {0,0,0,0};		//Bit-Rate
uint32_t ulKNL_VdoOutAccCntTemp[4] = {0,0,0,0};	//Bit-Rate
uint32_t ulKNL_AdoOutAccCnt = 0;				//Bit-Rate
uint32_t ulKNL_AdoOutAccCntTemp = 0;			//Bit-Rate

uint32_t ulKNL_OutVdoFpsCnt[KNL_SRC_NUM];		//For Output
uint32_t ulKNL_OutVdoFpsCntTemp[KNL_SRC_NUM];	//For Output

uint32_t ulKNL_InVdoFpsCnt[KNL_SRC_NUM];		//For Input
uint32_t ulKNL_InVdoFpsCntTemp[KNL_SRC_NUM];	//For Input

uint32_t ulKNL_FrmTRxNum[KNL_MAX_ROLE];			//For Output
uint32_t ulKNL_FrmTRxNumTemp[KNL_MAX_ROLE];		//For Output

//WOR
uint8_t ubKNL_WorSts;

//For FS
FS_KNL_PARA_t tAPP_Fs;

//For Record
uint8_t ubKNL_StartRecFlag = 0;

//For USB Tuning tool
KNL_TuningMode_t tKNL_TuningMode;

static void KNL_VdoInProcThread(void const *argument);
static void KNL_VdoCodecProcThread(void const *argument);
static void KNL_AdoCodecProcThread(void const *argument);
static void KNL_AdoEncMonitThread(void const *argument);
static void KNL_AdoDecMonitThread(void const *argument);
static void KNL_CommLinkMonitThread(void const *argument);
static void KNL_CommLinkUpdateThread(void const *argument);
static void KNL_CommAdoRxMonitThread(void const *argument);
static void KNL_TwcMonitThread(void const *argument);
#if OP_AP
static void KNL_CommVdoRxMonitThread(void const *argument);
#endif
static void KNL_CommTxProcThread(void const *argument);
static void KNL_BbFrmMonitThread(void const *argument);
//static void KNL_ImgMonitThread(void const *argument);
static void KNL_JpegMonitThread(void const *argument);

static void KNL_AvgPly0Thread(void const *argument);
static void KNL_AvgPly1Thread(void const *argument);
static void KNL_AvgPly2Thread(void const *argument);
static void KNL_AvgPly3Thread(void const *argument);

static void KNL_SysMonitThread(void const *argument);
static void KNL_SecMonitThread(void const *argument);
pvKNL_BbFrmOkCbFunc ptKNL_BbFrmMonitCbFunc;
//------------------------------------------------------------------------------

#define KNL_MAJORVER    1        //!< Major version = 1
#define KNL_MINORVER    8        //!< Minor version = 8
uint16_t uwKNL_GetVersion (void)
{
    return ((KNL_MAJORVER << 8) + KNL_MINORVER);
}

uint8_t ubKNL_GetQp(uint8_t ubCodecIdx)
{
	return ubKNL_Qp[ubCodecIdx];
}

uint8_t ubKNL_SrcNumMap(uint8_t ubSrcNum)
{
	if((ubSrcNum == KNL_SRC_1_MAIN)||(ubSrcNum == KNL_SRC_1_SUB)||(ubSrcNum == KNL_SRC_1_AUX)||(ubSrcNum == KNL_SRC_1_OTHER_A)||(ubSrcNum == KNL_SRC_1_OTHER_B))
	{
		return 0;
	}
	else if((ubSrcNum == KNL_SRC_2_MAIN)||(ubSrcNum == KNL_SRC_2_SUB)||(ubSrcNum == KNL_SRC_2_AUX)||(ubSrcNum == KNL_SRC_2_OTHER_A)||(ubSrcNum == KNL_SRC_2_OTHER_B))
	{
		return 1;
	}
	else if((ubSrcNum == KNL_SRC_3_MAIN)||(ubSrcNum == KNL_SRC_3_SUB)||(ubSrcNum == KNL_SRC_3_AUX)||(ubSrcNum == KNL_SRC_3_OTHER_A)||(ubSrcNum == KNL_SRC_3_OTHER_B))
	{
		return 2;
	}
	else if((ubSrcNum == KNL_SRC_4_MAIN)||(ubSrcNum == KNL_SRC_4_SUB)||(ubSrcNum == KNL_SRC_4_AUX)||(ubSrcNum == KNL_SRC_4_OTHER_A)||(ubSrcNum == KNL_SRC_4_OTHER_B))
	{
		return 3;
	}
	else if(ubSrcNum == KNL_SRC_MASTER_AP)
	{
		return KNL_MASTER_AP;
	}
	printd(DBG_ErrorLvl, "Err @ubKNL_SrcNumMap\r\n");
	return 0xFF;
}

void KNL_SetPlyMode(KNL_PLY_MODE tPlyMode)
{
	if(NULL == KNL_AvgPlyQueue[0])
	{
		osMessageQDef(KNL_AVG_PLY_PROCESS0, KNL_AVG_PLY_QUEUE_NUM, KNL_AVG_PLY_PROCESS);
		KNL_AvgPlyQueue[0] = osMessageCreate(osMessageQ(KNL_AVG_PLY_PROCESS0), NULL);
	}
	if(NULL == KNL_AvgPlyQueue[1])
	{
		osMessageQDef(KNL_AVG_PLY_PROCESS1, KNL_AVG_PLY_QUEUE_NUM, KNL_AVG_PLY_PROCESS);
		KNL_AvgPlyQueue[1] = osMessageCreate(osMessageQ(KNL_AVG_PLY_PROCESS1), NULL);
	}
	if(NULL == KNL_AvgPlyQueue[2])
	{
		osMessageQDef(KNL_AVG_PLY_PROCESS2, KNL_AVG_PLY_QUEUE_NUM, KNL_AVG_PLY_PROCESS);
		KNL_AvgPlyQueue[2] = osMessageCreate(osMessageQ(KNL_AVG_PLY_PROCESS2), NULL);
	}
	if(NULL == KNL_AvgPlyQueue[3])
	{
		osMessageQDef(KNL_AVG_PLY_PROCESS3, KNL_AVG_PLY_QUEUE_NUM, KNL_AVG_PLY_PROCESS);
		KNL_AvgPlyQueue[3] = osMessageCreate(osMessageQ(KNL_AVG_PLY_PROCESS3), NULL);
	}
	if(tPlyMode == KNL_AVG_PLY)
	{
		ubKNL_AvgPlyStartFlg[0] = 0;
		ubKNL_AvgPlyStartFlg[1] = 0;
		ubKNL_AvgPlyStartFlg[2] = 0;
		ubKNL_AvgPlyStartFlg[3] = 0;
		if(NULL == KNL_AvgThreadId[0])
		{
			osThreadDef(KNLAvgPlyThread0, KNL_AvgPly0Thread, THREAD_PRIO_KNL_AVG_PLY, 1, THREAD_STACK_KNL_AVG_PLY);
			KNL_AvgThreadId[0] = osThreadCreate(osThread(KNLAvgPlyThread0), NULL);
		}
		if(NULL == KNL_AvgThreadId[1])
		{
			osThreadDef(KNLAvgPlyThread1, KNL_AvgPly1Thread, THREAD_PRIO_KNL_AVG_PLY, 1, THREAD_STACK_KNL_AVG_PLY);
			KNL_AvgThreadId[1] = osThreadCreate(osThread(KNLAvgPlyThread1), NULL);
		}
		if(NULL == KNL_AvgThreadId[2])
		{
			osThreadDef(KNLAvgPlyThread2, KNL_AvgPly2Thread, THREAD_PRIO_KNL_AVG_PLY, 1, THREAD_STACK_KNL_AVG_PLY);
			KNL_AvgThreadId[2] = osThreadCreate(osThread(KNLAvgPlyThread2), NULL);
		}
		if(NULL == KNL_AvgThreadId[3])
		{
			osThreadDef(KNLAvgPlyThread3, KNL_AvgPly3Thread, THREAD_PRIO_KNL_AVG_PLY, 1, THREAD_STACK_KNL_AVG_PLY);
			KNL_AvgThreadId[3] = osThreadCreate(osThread(KNLAvgPlyThread3), NULL);
		}
	}
	tKNL_Info.tPlyMode = tPlyMode;
}

KNL_PLY_MODE tKNL_GetPlyMode(void)
{
	return tKNL_Info.tPlyMode;
}

void KNL_SetStartPlyNum(uint8_t ubFrameNum)
{
	ubKNL_AvgPlyStartNum = ubFrameNum;
}

uint8_t ubKNL_GetStartPlyNum(void)
{
	return ubKNL_AvgPlyStartNum;
}

void KNL_SetVdoFps(uint8_t ubFps)
{
	tKNL_Info.ubVdoFps = ubFps;
}

uint8_t ubKNL_GetVdoFps(void)
{
	return tKNL_Info.ubVdoFps;
}

uint32_t ulKNL_GetBbIpBufSz(void)
{
	uint32_t ulBufSz = 0;
	uint32_t ulBufStartAddr,ulBufEndAddr;
	
	//BB_VariableInit(0);
	BB_VariableInit(ulBUF_GetFreeAddr());	
	
	ulBufStartAddr = ulBB_GetBasebandUseAddr(BB_USE_START);
	ulBufEndAddr = ulBB_GetBasebandUseAddr(BB_USE_END);
	
	ulBufSz = ulBufEndAddr-ulBufStartAddr+1;
	return ulBufSz;	
}
//------------------------------------------------------------------------------
void KNL_SetAdoInfo(ADO_KNL_PARA_t tAdoInfo)
{
	tKNL_AdoInfo.ulADO_BufStartAddr         = tAdoInfo.ulADO_BufStartAddr;
	tKNL_AdoInfo.Sys_speed					= tAdoInfo.Sys_speed;	
	tKNL_AdoInfo.Rec_device 				= tAdoInfo.Rec_device;
	tKNL_AdoInfo.Ply_device 				= tAdoInfo.Ply_device;
    tKNL_AdoInfo.ADO_SigDelAdcMode  		= tAdoInfo.ADO_SigDelAdcMode;
	tKNL_AdoInfo.Rec_fmt.sign_flag   		= tAdoInfo.Rec_fmt.sign_flag;
	tKNL_AdoInfo.Rec_fmt.channel     		= tAdoInfo.Rec_fmt.channel;
	tKNL_AdoInfo.Rec_fmt.sample_size 		= tAdoInfo.Rec_fmt.sample_size;
	tKNL_AdoInfo.Rec_fmt.sample_rate		= tAdoInfo.Rec_fmt.sample_rate;	
	tKNL_AdoInfo.Ply_fmt.sign_flag   		= tAdoInfo.Ply_fmt.sign_flag;
	tKNL_AdoInfo.Ply_fmt.channel     		= tAdoInfo.Ply_fmt.channel;
	tKNL_AdoInfo.Ply_fmt.sample_size		= tAdoInfo.Ply_fmt.sample_size;
	tKNL_AdoInfo.Ply_fmt.sample_rate 		= tAdoInfo.Ply_fmt.sample_rate;	
	tKNL_AdoInfo.Compress_method			= tAdoInfo.Compress_method;	
	tKNL_AdoInfo.Adpcm_func.extra_header	= tAdoInfo.Adpcm_func.extra_header;
	tKNL_AdoInfo.Adpcm_func.step_size		= tAdoInfo.Adpcm_func.step_size;
	tKNL_AdoInfo.Adpcm_func.enc_smpl_num	= tAdoInfo.Adpcm_func.enc_smpl_num;
	tKNL_AdoInfo.Adpcm_func.dec_byte_num	= tAdoInfo.Adpcm_func.dec_byte_num;	
	tKNL_AdoInfo.Rec_buf_size        		= tAdoInfo.Rec_buf_size;
	tKNL_AdoInfo.Ply_buf_size        		= tAdoInfo.Ply_buf_size;
	tKNL_AdoInfo.Audio32_En_buf_size 		= tAdoInfo.Audio32_En_buf_size;
	tKNL_AdoInfo.Audio32_De_buf_size 		= tAdoInfo.Audio32_De_buf_size;
	tKNL_AdoInfo.AAC_En_buf_size     		= tAdoInfo.AAC_En_buf_size;
	tKNL_AdoInfo.AAC_De_buf_size     		= tAdoInfo.AAC_De_buf_size;	
	tKNL_AdoInfo.Rec_buf_th					= tAdoInfo.Rec_buf_th;
	tKNL_AdoInfo.Ply_buf_th        			= tAdoInfo.Ply_buf_th;
	tKNL_AdoInfo.Audio32_En_buf_th 			= tAdoInfo.Audio32_En_buf_th;
	tKNL_AdoInfo.Audio32_De_buf_th 			= tAdoInfo.Audio32_De_buf_th;
	tKNL_AdoInfo.AAC_En_buf_th     			= tAdoInfo.AAC_En_buf_th;
	tKNL_AdoInfo.AAC_De_buf_th     			= tAdoInfo.AAC_De_buf_th;
}
//------------------------------------------------------------------------

void KNL_SetVdoGop(uint32_t ulGop)
{
	tKNL_Info.ulGop = ulGop;
}

uint32_t ulKNL_GetVdoGop(void)
{
	return tKNL_Info.ulGop;
}

uint32_t ulKNL_GetVdoFrmIdx(uint8_t ubCh)
{
	return ulKNL_CurFrmIdx[ubCh];
}

//------------------------------------------------------------------------------
void KNL_Init(void)
{
	KNL_ROLE tKNLRole;

	pLcdCropScaleParam 		= NULL;
	ptKNL_VdoRoleSrcMapT    = NULL;
	ptKNL_AdoRoleSrcMapT    = NULL;
	tKNL_Info.ubDisp1SrcNum = KNL_SRC_NONE;
	tKNL_Info.ubDisp2SrcNum = KNL_SRC_NONE;
	tKNL_Info.ubDisp3SrcNum = KNL_SRC_NONE;
	tKNL_Info.ubDisp4SrcNum = KNL_SRC_NONE;
	tKNL_TuningMode 		= KNL_TUNINGMODE_OFF;
	ubKNL_ChgResFlg			= FALSE;
	ubKNL_ImgBusyFlg		= FALSE;
	ubKNL_ImgStabFlg		= FALSE;
	ubKNL_WorSts			= 1;
	KNL_SwDispInfo.ubSetupFlag = FALSE;
	for(tKNLRole = KNL_STA1; tKNLRole <= KNL_STA4; tKNLRole++)
	{
		KNL_AvgThreadId[tKNLRole]			= NULL;
		KNL_AvgPlyQueue[tKNLRole]			= NULL;
		ubKNL_VdoBsBusyCnt[tKNLRole]		= 0;
		ubKNL_VdoResendITwcFlg[tKNLRole]	= FALSE;
		ubKNL_VdoChkSrcNumFlg[tKNLRole]	 	= FALSE;
		ubKNL_VdoResChgTwcFlg[tKNLRole]     = FALSE;		
		KNL_SwDispInfo.tSrcNum[tKNLRole] 	= KNL_SRC_NONE;
		KNL_SwDispInfo.tSrcLocate[tKNLRole] = KNL_DISP_LOCATION_ERR;
	}
	for(tKNLRole = KNL_STA1; tKNLRole <= KNL_MAX_ROLE; tKNLRole++)
		ubKNL_WakeUpFlag[tKNLRole] = FALSE;

	KNL_NodeStateReset();
	KNL_VdoReset();
	KNL_SetAuxInfoFunc(1);		//Add Aux-Information for packet transmission
	KNL_SetStartPlyNum(5);		//Start play frame number for AVG-PLY

	KNL_SetVdoGop(900);

	osMutexDef(tKNL_LinkSem);
	tKNL_LinkSem 	= osMutexCreate(osMutex(tKNL_LinkSem));

	osSemaphoreDef(tKNL_ImgSem);
	tKNL_ImgSem	= osSemaphoreCreate(osSemaphore(tKNL_ImgSem), 1);

	osSemaphoreDef(tKNL_TwcSem);
	tKNL_TwcSem	= osSemaphoreCreate(osSemaphore(tKNL_TwcSem), 1);

	osMutexDef(tKNL_BsBufMutex);
	tKNL_BsBufMutex = osMutexCreate(osMutex(tKNL_BsBufMutex));

	osMessageQDef(KNL_VDOPROCESS, KNL_PROC_QUEUE_NUM, KNL_PROCESS);
	KNL_VdoCodecProcQueue = osMessageCreate(osMessageQ(KNL_VDOPROCESS), NULL);

	osMessageQDef(KNL_ADOPROCESS, KNL_PROC_QUEUE_NUM, KNL_PROCESS);
	KNL_AdoCdoecProcQueue = osMessageCreate(osMessageQ(KNL_ADOPROCESS), NULL);

	osMessageQDef(KNL_VDOINPROCESS, BUF_NUM_SEN_YUV, KNL_PROCESS);
	KNL_VdoInProcQueue = osMessageCreate(osMessageQ(KNL_VDOINPROCESS), NULL);

	osMessageQDef(KNL_COMMTXPROCESS, KNL_PROC_QUEUE_NUM, KNL_PROCESS);
	KNL_CommTxProcQueue = osMessageCreate(osMessageQ(KNL_COMMTXPROCESS), NULL);

	osMessageQDef(KNL_AVG_TWC_MONIT, 32, KNL_PROCESS);
	KNL_TwcMonitQueue = osMessageCreate(osMessageQ(KNL_AVG_TWC_MONIT), NULL);

	osThreadDef(KNLVdoInProcThread, KNL_VdoInProcThread, THREAD_PRIO_KNL_VDOIN_PROC, 1, THREAD_STACK_KNL_VDOIN_PROC);
	osThreadCreate(osThread(KNLVdoInProcThread), NULL);

	osThreadDef(KNLCommTxProcThread, KNL_CommTxProcThread, THREAD_PRIO_COMM_TX_VDO, 1, THREAD_STACK_COMM_TX_VDO);
	osThreadCreate(osThread(KNLCommTxProcThread), NULL);

	osThreadDef(KNLVdoCodecProcThread, KNL_VdoCodecProcThread, THREAD_PRIO_KNL_PROC, 1, THREAD_STACK_KNL_PROC);
	osThreadCreate(osThread(KNLVdoCodecProcThread), NULL);

	osThreadDef(KNLAdoCodecProcThread, KNL_AdoCodecProcThread, THREAD_PRIO_KNL_PROC, 1, THREAD_STACK_KNL_PROC);
	osThreadCreate(osThread(KNLAdoCodecProcThread), NULL);

	osThreadDef(KNLTwcMonitThread, KNL_TwcMonitThread, THREAD_PRIO_KNL_TWC_MONIT, 1, THREAD_STACK_KNL_TWC_MONIT);
	osThreadCreate(osThread(KNLTwcMonitThread), NULL);

	osThreadDef(SysMonitThread, KNL_SysMonitThread, THREAD_PRIO_SYS_MONIT, 1, THREAD_STACK_SYS_MONIT);
    osThreadCreate(osThread(SysMonitThread), NULL);

	osThreadDef(KNLSecMonitThread, KNL_SecMonitThread, THREAD_PRIO_SEC_MONIT, 1, THREAD_STACK_SEC_MONIT);
	osThreadCreate(osThread(KNLSecMonitThread), NULL);
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetDataBitRate(uint8_t ubDataType,uint8_t ubCodecIdx)
{
	if(ubDataType == 0)	//Video
	{
		return ulKNL_VdoOutAccCnt[ubCodecIdx];
	}
	else				//Audio
	{
		return ulKNL_AdoOutAccCnt;
	}
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetFps(KNL_FPS_TYPE tFpsType,uint8_t ubSrcNum)
{
	if(tFpsType == KNL_FPS_OUT)
	{
		return ulKNL_OutVdoFpsCnt[ubSrcNum];
	}
	else if (tFpsType == KNL_FPS_IN)
	{
		return ulKNL_InVdoFpsCnt[ubSrcNum];
	}
	else
	{
		uint8_t ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);

		return ulKNL_FrmTRxNum[ubSrcNumMap];
	}
}
//------------------------------------------------------------------------------
static void KNL_SecMonitThread(void const *argument)
{
	uint8_t i;

	while(1)
	{
		//Output Bit-Rate (Video Data)
		ulKNL_VdoOutAccCnt[0] = ulKNL_VdoOutAccCntTemp[0];
		ulKNL_VdoOutAccCntTemp[0] = 0;	

		ulKNL_VdoOutAccCnt[1] = ulKNL_VdoOutAccCntTemp[1];
		ulKNL_VdoOutAccCntTemp[1] = 0;

		ulKNL_VdoOutAccCnt[2] = ulKNL_VdoOutAccCntTemp[2];
		ulKNL_VdoOutAccCntTemp[2] = 0;

		ulKNL_VdoOutAccCnt[3] = ulKNL_VdoOutAccCntTemp[3];
		ulKNL_VdoOutAccCntTemp[3] = 0;		

		//Output/Input Frame-Rate (Video Data)
		for(i=0;i<KNL_SRC_NUM;i++)
		{
			ulKNL_OutVdoFpsCnt[i] = ulKNL_OutVdoFpsCntTemp[i];
			ulKNL_OutVdoFpsCntTemp[i] = 0;
//			ulKNL_InVdoFpsCnt[i] = ulKNL_InVdoFpsCntTemp[i];
//			ulKNL_InVdoFpsCntTemp[i] = 0;
		}
		for(i=0;i<KNL_MAX_ROLE;i++)
		{
			ulKNL_FrmTRxNum[i] = ulKNL_FrmTRxNumTemp[i];
			ulKNL_FrmTRxNumTemp[i] = 0;
		}
//		printd(DBG_InfoLvl, "VdoTp[0]:%d KB\r\n",(ulKNL_VdoOutAccCnt[0])/8/1024);
//		printd(DBG_InfoLvl, "VdoTp[1]:%d KB\r\n",(ulKNL_VdoOutAccCnt[1])/8/1024);
//		printd(DBG_InfoLvl, "VdoTp[2]:%d KB\r\n",(ulKNL_VdoOutAccCnt[2])/8/1024);
//		printd(DBG_InfoLvl, "VdoTp[3]:%d KB\r\n",(ulKNL_VdoOutAccCnt[3])/8/1024);
#if OP_AP
		printd(DBG_CriticalLvl, "FPS:%d,%d,%d,%d\r\n",
			ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_1_MAIN),
			ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_2_MAIN),
			ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_3_MAIN),
			ulKNL_GetFps(KNL_BB_FRM_OK, KNL_SRC_4_MAIN));		
#endif
		osDelay(1000);
	}
}
//------------------------------------------------------------------------------
static void KNL_AdoEncMonitThread(void const *argument)
{
	ADO_Queue_INFO tAdoInfo;
	KNL_PROCESS tProcess;
	KNL_SRC tKNL_AdoSrcNum;
	uint32_t ulAddr,ulSize;
	uint8_t ubSrcNum;

	while(1)
	{
		//Get Queue from Audio Interface
		osMessageGet(tKNL_EncEventQue, &tAdoInfo, osWaitForever);
		if(tAdoInfo.EncType == NONE)
		{
			if(tKNL_AdoInfo.Compress_method == COMPRESS_MSADPCM)
			{
				printd(DBG_Debug3Lvl, "AdoEncQ(ADPCM) : Addr:0x%x,SZ:0x%x\r\n",tAdoInfo.PcmAddr,tAdoInfo.PcmSize);
			}
			else if(tKNL_AdoInfo.Compress_method == COMPRESS_NONE)
			{
				printd(DBG_Debug3Lvl, "AdoEncQ(PCM) : Addr:0x%x,SZ:0x%x\r\n",tAdoInfo.PcmAddr,tAdoInfo.PcmSize);
			}
			ulAddr = tAdoInfo.PcmAddr;
			ulSize = tAdoInfo.PcmSize;
		}
		else if(tAdoInfo.EncType == AUDIO32)
		{
			printd(DBG_Debug3Lvl, "AdoEncQ(Audio32) : Addr:0x%x,SZ:0x%x\r\n",tAdoInfo.Audio32Addr,tAdoInfo.Audio32Size);
			ulAddr = tAdoInfo.Audio32Addr;
			ulSize = tAdoInfo.Audio32Size;
		}
		else if(tAdoInfo.EncType == AAC)
		{	
			printd(DBG_Debug3Lvl, "AdoEncQ(AAC) : Addr:0x%x,SZ:0x%x\r\n",tAdoInfo.AACAddr,tAdoInfo.AACSize);
			ulAddr = tAdoInfo.AACAddr;
			ulSize = tAdoInfo.AACSize;
		}

		tKNL_AdoSrcNum = KNL_SRC_NONE;
		//Send Queue to processthread
		for(ubSrcNum = KNL_SRC_1_OTHER_A; ubSrcNum <= KNL_SRC_4_OTHER_B; ubSrcNum++)
		{
			if(ubKNL_ExistNode(ubSrcNum, KNL_NODE_ADC))
			{
				#ifdef OP_STA
				if((ubSrcNum % 4) == ubKNL_GetRole())
				{
					tKNL_AdoSrcNum = (KNL_SRC)ubSrcNum;
					break;
				}
				#endif
				#ifdef OP_AP
				tKNL_AdoSrcNum = (KNL_SRC)ubSrcNum;
				break;
				#endif
			}
		}
		if((KNL_SRC_NONE != tKNL_AdoSrcNum) && (ubKNL_ChkAdoFlowAct(tKNL_AdoSrcNum)))
		{
			tProcess.ubSrcNum	 = tKNL_AdoSrcNum;
			tProcess.ubCurNode 	 = KNL_NODE_ADC;
			tProcess.ubNextNode  = ubKNL_GetNextNode(tKNL_AdoSrcNum,KNL_NODE_ADC);
			tProcess.ulDramAddr1 = 0;
			tProcess.ulDramAddr2 = ulAddr;
			tProcess.ulSize		 = ulSize;
			KNL_AdcBufProcess(tProcess);
//			if(osMessagePut(KNL_AdoCdoecProcQueue, &tProcess, 0) == osErrorResource)
//			{
//				printd(DBG_ErrorLvl, "KNL_ADO Q->Full !!!!\r\n");
//			}
		}
	}
}
//------------------------------------------------------------------------------
static void KNL_AdoDecMonitThread(void const *argument)
{	
	ADO_DEC_EVENT tKNL_DecEvent;

	while(1)
	{
		//Get Queue from Audio Interface
		osMessageGet(tKNL_DecEventQue, &tKNL_DecEvent, osWaitForever);
		switch(tKNL_DecEvent)
		{
			case DAC_RDY:
#if defined(VBM_PU) || defined(VBM_BU)
				ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
#endif
#if defined(BPC_RX) || defined(BPC_CAM)
				//ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, OFF);
#endif
				#ifdef VBM_PU
				SPEAKER_EN(FALSE);
				TIMER_Delay_us(1000);
				SPEAKER_EN(TRUE);
				TIMER_Delay_us(2);
				SPEAKER_EN(FALSE);
				TIMER_Delay_us(2);
				#endif
				SPEAKER_EN(TRUE);
				break;
			case PLAY_BUF_EMP:
				printf("-Dac play empty-\n");
				break;
		}
	}
}
//------------------------------------------------------------------------------
void KNL_BufInit(void)
{
	KNL_NODE_INFO tNodeInfo;
	float fBufSize;	
	uint8_t ubNodeExist;
	uint8_t ubSrc;
	uint32_t i;	
	uint32_t ulAddr;
#if KNL_REC_FUNC_ENABLE
    uint32_t ulNeedMemSize;
    uint32_t ulTotalMemSize;
    uint32_t ulResiMemSize;
#endif
#if KNL_USBH_FUNC_ENABLE
	uint8_t uvc_stream_num = 0;
    USBH_UVC_FRAME_INFO uvc_frame_info;
#endif

	//COMM Node
	//================================================================
	ubNodeExist = ubKNL_ChkExistNode(KNL_NODE_COMM_TX_VDO) | ubKNL_ChkExistNode(KNL_NODE_COMM_RX_VDO)| ubKNL_ChkExistNode(KNL_NODE_COMM_TX_ADO)| ubKNL_ChkExistNode(KNL_NODE_COMM_RX_ADO);
	if(ubNodeExist)
	{
		fBufSize = ulKNL_GetBbIpBufSz();
		BUF_BufInit(BUF_BB_IP,1,fBufSize,0);	//IP Buffer
	}

#if USBD_ENABLE
	fBufSize = USBD_BufSetup(ulBUF_GetFreeAddr());
	BUF_BufInit(BUF_USBD_IP,1,fBufSize,0);	    //USBD IP Buffer
#endif

	//Audio
	//================================================================
	//IP
	if(ubKNL_ChkExistNode(KNL_NODE_ADC) || ubKNL_ChkExistNode(KNL_NODE_DAC))
	{
		BUF_BufInit(BUF_ADO_IP,1,ulADO_GetTotalBuffSize(&tKNL_AdoInfo),0);
	}	
	//ADC
	if(ubKNL_ChkExistNode(KNL_NODE_ADC_BUF))
	{		
		BUF_BufInit(BUF_ADO_ADC,BUF_NUM_ADC,ulKNL_AlignAdoPktSz(ulADO_BufTh[tKNL_AdoInfo.Rec_buf_th]),0);
		BUF_Reset(BUF_ADO_ADC);
	}
	//DAC
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_OTHER_A+i,KNL_NODE_DAC_BUF) ||ubKNL_ExistNode(KNL_SRC_1_OTHER_B+i,KNL_NODE_DAC_BUF))
		{
			BUF_BufInit(BUF_ADO_DAC0+i,BUF_NUM_DAC,ulKNL_AlignAdoPktSz(ulADO_BufTh[tKNL_AdoInfo.Ply_buf_th]),0);
			BUF_Reset(BUF_ADO_DAC0+i);
		}
	}

#if OP_STA
	//SEN_YUV Node
	//================================================================	
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
	if(ubSrc != KNL_SRC_NONE)
	{
		if (KNL_GetTuningToolMode() == KNL_TUNINGMODE_ON) {
            fBufSize = ((float)uwKNL_GetVdoH(ubSrc))*((float)uwKNL_GetVdoV(ubSrc))*2;
		} else {
            fBufSize = ISP_WIDTH * ISP_HEIGHT * 1.5;	//! ((float)uwKNL_GetVdoH(ubSrc))*((float)uwKNL_GetVdoV(ubSrc))*1.5;
		}
		BUF_BufInit(BUF_SEN_1_YUV,BUF_NUM_SEN_YUV,fBufSize,0);
		BUF_Reset(BUF_SEN_1_YUV);
	}
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH2);
	if(ubSrc != KNL_SRC_NONE)
	{
		fBufSize = ((float)uwKNL_GetVdoH(ubSrc))*((float)uwKNL_GetVdoV(ubSrc))*1.5;
		BUF_BufInit(BUF_SEN_2_YUV,BUF_NUM_SEN_YUV,fBufSize,0);
		BUF_Reset(BUF_SEN_2_YUV);
	}
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH3);
	if(ubSrc != KNL_SRC_NONE)
	{
		fBufSize = ((float)uwKNL_GetVdoH(ubSrc))*((float)uwKNL_GetVdoV(ubSrc))*1.5;	
		BUF_BufInit(BUF_SEN_3_YUV,BUF_NUM_SEN_YUV,fBufSize,0);
		BUF_Reset(BUF_SEN_3_YUV);
	}
    // ISP pipe
	//================================================================
    fBufSize = (float)ISP_WIDTH*(float)ISP_HEIGHT*10/8;
    BUF_BufInit(BUF_ISP_3DNR_IP, 1, fBufSize, 0);	    //ISP(3DNR) IP Buffer

    fBufSize = 0x58000;
    BUF_BufInit(BUF_ISP_MD_W0_IP, 1, fBufSize, 0);	    //ISP(MD_W0) IP Buffer
    fBufSize = 0x800;
    BUF_BufInit(BUF_ISP_MD_W1_IP, 1, fBufSize, 0);	    //ISP(MD_W1) IP Buffer
    fBufSize = 0x800;
    BUF_BufInit(BUF_ISP_MD_W2_IP, 1, fBufSize, 0);	    //ISP(MD_W2) IP Buffer
    // IQ bin file (128k)
	//================================================================
    fBufSize = 0x20000;
    BUF_BufInit(BUF_IQ_BIN_FILE, 1, fBufSize, 0);	    //IQ bin file Buffer

    SEN_SetPathAddr(ISP_3DNR, ulBUF_GetBlkBufAddr(0, BUF_ISP_3DNR_IP));
    SEN_SetPathAddr(ISP_MD_W0, ulBUF_GetBlkBufAddr(0, BUF_ISP_MD_W0_IP));
    SEN_SetPathAddr(ISP_MD_W1, ulBUF_GetBlkBufAddr(0, BUF_ISP_MD_W1_IP));
    SEN_SetPathAddr(ISP_MD_W2, ulBUF_GetBlkBufAddr(0, BUF_ISP_MD_W2_IP));      
    SEN_SetPathAddr(IQ_BIN_FILE, ulBUF_GetBlkBufAddr(0, BUF_IQ_BIN_FILE)); 
#endif

	//VDO_BS_BUF Node
	//================================================================
	//(For BS_BUF1)
	//Main
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_MAIN+i,KNL_NODE_VDO_BS_BUF1))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_MAIN+i,KNL_NODE_VDO_BS_BUF1);
			tNodeInfo.uwVdoH = ISP_WIDTH;
			tNodeInfo.uwVdoV = ISP_HEIGHT;
			fBufSize = ( ((uint32_t)tNodeInfo.uwVdoH) * ((uint32_t)tNodeInfo.uwVdoV) * 1.5 )/KNL_MIN_COMPRESS_RATIO;
			BUF_BufInit(BUF_VDO_MAIN_BS0+i,BUF_NUM_VDO_BS,fBufSize,0);
			BUF_Reset(BUF_VDO_MAIN_BS0+i);
		}
	}
	//Sub
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_SUB+i,KNL_NODE_VDO_BS_BUF1))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_SUB+i,KNL_NODE_VDO_BS_BUF1);
			fBufSize = ( ((uint32_t)tNodeInfo.uwVdoH) * ((uint32_t)tNodeInfo.uwVdoV) * 1.5 )/KNL_MIN_COMPRESS_RATIO;
			BUF_BufInit(BUF_VDO_SUB_BS00+i,BUF_NUM_VDO_BS,fBufSize,0);
			BUF_Reset(BUF_VDO_SUB_BS00+i);
		}
	}
	//Aux
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_AUX+i,KNL_NODE_VDO_BS_BUF1))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_AUX+i,KNL_NODE_VDO_BS_BUF1);
			fBufSize = ( ((uint32_t)tNodeInfo.uwVdoH) * ((uint32_t)tNodeInfo.uwVdoV) * 1.5 )/KNL_MIN_COMPRESS_RATIO;
			BUF_Reset(BUF_VDO_AUX_BS0+i);
			BUF_BufInit(BUF_VDO_AUX_BS0+i,BUF_NUM_VDO_BS,fBufSize,0);
		}
	}
	//(For BS_BUF2)	
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_SUB+i,KNL_NODE_VDO_BS_BUF2))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_SUB+i,KNL_NODE_VDO_BS_BUF2);
			fBufSize = ( ((uint32_t)tNodeInfo.uwVdoH) * ((uint32_t)tNodeInfo.uwVdoV) * 1.5 )/KNL_MIN_COMPRESS_RATIO;
			BUF_BufInit(BUF_VDO_SUB_BS01+i,BUF_NUM_VDO_BS,fBufSize,0);
			BUF_Reset(BUF_VDO_SUB_BS01+i);
		}
	}

	//IMG_MERGE Node
	//================================================================
	ubNodeExist = ubKNL_ChkExistNode(KNL_NODE_IMG_MERGE_H)|ubKNL_ChkExistNode(KNL_NODE_IMG_MERGE_BUF);
	if(ubNodeExist)
	{
		fBufSize  = ulKNL_GetImgMergeBufSz();
		BUF_BufInit(BUF_IMG_MERGE,1,fBufSize,0);

		//Clear Merge Buffer @Dummy Buffer
		if((tKNL_GetDispType() == KNL_DISP_H) && (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0))
		{
			ulAddr = ulBUF_GetBlkBufAddr(0,BUF_IMG_MERGE);
			ubSrc = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
			tDMAC_MemSet (0x00,ulAddr,((float)uwKNL_GetLcdDmyImgH())*((float)uwKNL_GetVdoV(ubSrc))*1.5,NULL);
		}
	}

	//LCD Node
	//================================================================
	ubNodeExist = ubKNL_ChkExistNode(KNL_NODE_LCD);
	if(ubNodeExist)
	{
		fBufSize = ulKNL_CalLcdBufSz();
		BUF_BufInit(BUF_LCD_IP,1,fBufSize,0);
	}

	//H264_ENC Node	
	//================================================================
	//For Main Source
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_MAIN+i,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_MAIN+i,KNL_NODE_H264_ENC);
			tNodeInfo.uwVdoH = ISP_WIDTH;
			tNodeInfo.uwVdoV = ISP_HEIGHT;
			fBufSize = ulH264_GetENCBufferSize(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			if(tNodeInfo.ubCodecIdx == ENCODE_0)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,0);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_1)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,1);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_2)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,2);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_3)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,3);
			}
		}
	}	
	//For Aux Source
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_AUX+i,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_AUX+i,KNL_NODE_H264_ENC);		
			fBufSize = ulH264_GetENCBufferSize(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			if(tNodeInfo.ubCodecIdx == ENCODE_0)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,0);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_1)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,1);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_2)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,2);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_3)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,3);
			}
		}
	}	
	//For Sub Source
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_SUB+i,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_SUB+i,KNL_NODE_H264_ENC);		
			fBufSize = ulH264_GetENCBufferSize(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			if(tNodeInfo.ubCodecIdx == ENCODE_0)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,0);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_1)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,1);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_2)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,2);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_3)
			{
				BUF_BufInit(BUF_IMG_ENC,1,fBufSize,3);
			}
		}
	}
	
	//H264_DEC Node	
	//================================================================
	//For Main Source
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_MAIN+i,KNL_NODE_H264_DEC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_MAIN+i,KNL_NODE_H264_DEC);
			fBufSize = ulH264_GetDECBufferSize(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			if(tNodeInfo.ubCodecIdx == DECODE_0)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,0);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_1)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,1);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_2)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,2);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_3)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,3);
			}
		}
	}	
	//For Aux Source
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_AUX+i,KNL_NODE_H264_DEC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_AUX+i,KNL_NODE_H264_DEC);
			fBufSize = ulH264_GetDECBufferSize(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			if(tNodeInfo.ubCodecIdx == DECODE_0)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,0);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_1)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,1);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_2)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,2);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_3)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,3);
			}
		}
	}
	//For Sub Source
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_SUB+i,KNL_NODE_H264_DEC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_SUB+i,KNL_NODE_H264_DEC);
			fBufSize = ulH264_GetDECBufferSize(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			if(tNodeInfo.ubCodecIdx == DECODE_0)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,0);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_1)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,1);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_2)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,2);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_3)
			{
				BUF_BufInit(BUF_IMG_DEC,1,fBufSize,3);
			}
		}
	}	
	
#if KNL_REC_FUNC_ENABLE
    //FS
	if(1)
	{
        ulNeedMemSize = ulFS_GetTotalBufSize(FS_RES_MODE_HDx1);
		printd(DBG_Debug3Lvl, "FS Buf:0x%X\r\n",ulNeedMemSize);
		BUF_BufInit(BUF_FS,1,ulNeedMemSize,0);
	}
    
    //REC
    if(1)
    {
        ulTotalMemSize = 0x2000000;
        ulResiMemSize = ulTotalMemSize-(ulBUF_GetBlkBufAddr(0,BUF_FS)+ulBUF_AlignAddrTo1K(ulFS_GetTotalBufSize(FS_RES_MODE_HDx1)));
        BUF_BufInit(BUF_REC,1,ulResiMemSize,0);
        
        vREC_FileFormatSet(REC_FILE_AVI);
        Media_FileFormatConfigInit();
        printd(DBG_Debug3Lvl, "MaxRECTime %d minutes\r\n",ulREC_ModeSet(ulResiMemSize));
    }
#endif
    
#if KNL_USBH_FUNC_ENABLE	
	memset(&uvc_frame_info, 0, sizeof(USBH_UVC_FRAME_INFO));    
    
#if defined (CONFIG_DUAL_HOST)
	uvc_stream_num = 4;
	for(i = 0; i < uvc_stream_num; i++) {
    uvc_frame_info.fmt[i]      = USBH_UVC_MJPEG;
    uvc_frame_info.ulWidth[i]  = 1280;
    uvc_frame_info.ulHeight[i] = 720;
  }
	fBufSize = ulUSBH_GetBufferSize(0, 1, uvc_stream_num, uvc_frame_info);  // example
#else
	uvc_stream_num = 2;
	for(i = 0; i < uvc_stream_num; i++) {
    uvc_frame_info.fmt[i]      = USBH_UVC_MJPEG;
    uvc_frame_info.ulWidth[i]  = 1280;
    uvc_frame_info.ulHeight[i] = 720;
  }
	fBufSize = ulUSBH_GetBufferSize(0, 0, uvc_stream_num, uvc_frame_info);  // example
#endif
    
    BUF_BufInit(BUF_USBH_IP,1,fcBufSize,0);
#endif
}

//------------------------------------------------------------------------------
void KNL_ResendIAction(TWC_TAG GetSta,uint8_t *pData)
{
	GetSta = GetSta;
	pData  = pData;
	ubKNL_ResetIFlg = 1;
	printd(DBG_CriticalLvl, "Resend I\r\n");	
}

//------------------------------------------------------------------------------
void KNL_VdoResSetting(TWC_TAG GetSta, uint8_t *pData)
{
#if OP_STA
	uint16_t uwKNL_NHSize = 0, uwKNL_NVSize = 0;
	uint16_t uwKNL_DynHSize = 0, uwKNL_DynVSize = 0;
	uint8_t ubSrcNum = 0;

	uwKNL_DynHSize = (pData[0] << 8) + pData[1];
	uwKNL_DynVSize = (pData[2] << 8) + pData[3];
	ubSrcNum 	   = pData[4];
	uwKNL_NHSize   = uwKNL_GetVdoH(ubSrcNum);
	uwKNL_NVSize   = uwKNL_GetVdoV(ubSrcNum);
	if((uwKNL_NHSize != uwKNL_DynHSize) || (uwKNL_NVSize != uwKNL_DynVSize))
	{
		osMessageReset(KNL_VdoCodecProcQueue);
		BB_ClearTxBuf(BB_TX_MASTER, BB_DATA_VIDEO);
		KNL_SetVdoH(ubSrcNum, uwKNL_DynHSize);
		KNL_SetVdoV(ubSrcNum, uwKNL_DynVSize);
		ubKNL_ChgResFlg = TRUE;
		if((ubSrcNum == KNL_SRC_1_MAIN)||(ubSrcNum == KNL_SRC_2_MAIN)||(ubSrcNum == KNL_SRC_3_MAIN)||(ubSrcNum == KNL_SRC_4_MAIN))
		{
			SEN_SetResChgState(SENSOR_PATH1, uwKNL_DynHSize, uwKNL_DynVSize);
		}
		else if((ubSrcNum == KNL_SRC_1_SUB)||(ubSrcNum == KNL_SRC_2_SUB)||(ubSrcNum == KNL_SRC_3_SUB)||(ubSrcNum == KNL_SRC_4_SUB))
		{
			SEN_SetResChgState(SENSOR_PATH2, uwKNL_DynHSize, uwKNL_DynVSize);
		}
		else if((ubSrcNum == KNL_SRC_1_AUX)||(ubSrcNum == KNL_SRC_2_AUX)||(ubSrcNum == KNL_SRC_3_AUX)||(ubSrcNum == KNL_SRC_4_AUX))
		{
			SEN_SetResChgState(SENSOR_PATH3, uwKNL_DynHSize, uwKNL_DynVSize);
		}
	}
#endif
}

//------------------------------------------------------------------------------
void KNL_BlockInit(void)
{
	uint8_t ubNodeExist;
	KNL_NODE_INFO tNodeInfo;
	uint8_t i;
#if KNL_REC_FUNC_ENABLE
    REC_EXTFUNC_CTRL sRecExtFunc;
#endif

	//For Sensor
	//=======================================================================
	if(ubKNL_ChkExistNode(KNL_NODE_SEN))
	{
#ifdef OP_STA
		uint8_t ubSrcNum = KNL_SRC_NONE;
		SEN_RegisterEventQueue(KNL_VdoInProcQueue);
		SEN_RegisterEventNode(KNL_NODE_SEN_YUV_BUF);
		SEN_SetIspFinishCbFunc(KNL_ImgStabNotifyFunc);
        SEN_LoadIQData();
        //osDelay(1);
		SEN_InitProcess();
		ubSrcNum = ubSEN_GetPathSrc(SENSOR_PATH1);
		if((ubKNL_GetRole() <= KNL_STA4) && (KNL_SRC_NONE != ubSrcNum))
			KNL_SenStart(ubSrcNum);
#endif
	}
	else
		ISP_FuncDisable;

	ubNodeExist = ubKNL_ChkExistNode(KNL_NODE_H264_ENC)|ubKNL_ChkExistNode(KNL_NODE_H264_DEC)|ubKNL_ChkExistNode(KNL_NODE_IMG_MERGE_BUF)|ubKNL_ChkExistNode(KNL_NODE_IMG_MERGE_H);
	if(ubNodeExist && (ubKNL_InitImgFlg==0))
	{
		#ifdef OP_STA
//		SetH264Rate(800,480,15);
		SetH264Rate(_1280x720,FPS30);
		#endif

		#ifdef OP_AP
		GLB->H264_RATE = 1;
		GLB->IMG_RATE  = 3;
		#endif

		IMG_Init();
		H264_Reset();

		IMG_SetCodecFinishCbFunc(KNL_ImgMonitorFunc);
//		osThreadDef(ImgMonitThread, KNL_ImgMonitThread, THREAD_PRIO_IMG_MONIT, 1, THREAD_STACK_IMG_MONIT);
//		osThreadCreate(osThread(ImgMonitThread), NULL);

		ubKNL_InitImgFlg = 1;
	}

	//For H264 Encode
	//=======================================================================
	//Main
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_MAIN+i,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_MAIN+i,KNL_NODE_H264_ENC);
			if(tNodeInfo.ubCodecIdx == ENCODE_0)
			{
				KNL_ImgEncInit(ENCODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_1)
			{
				KNL_ImgEncInit(ENCODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_2)
			{
				KNL_ImgEncInit(ENCODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_3)
			{
				KNL_ImgEncInit(ENCODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
		}
	}
	//Aux
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_AUX+i,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_AUX+i,KNL_NODE_H264_ENC);
			if(tNodeInfo.ubCodecIdx == ENCODE_0)
			{
				KNL_ImgEncInit(ENCODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_1)
			{
				KNL_ImgEncInit(ENCODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_2)
			{
				KNL_ImgEncInit(ENCODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_3)
			{
				KNL_ImgEncInit(ENCODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
		}
	}	
	//Sub
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_SUB+i,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_SUB+i,KNL_NODE_H264_ENC);
			if(tNodeInfo.ubCodecIdx == ENCODE_0)
			{
				KNL_ImgEncInit(ENCODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_1)
			{
				KNL_ImgEncInit(ENCODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_2)
			{
				KNL_ImgEncInit(ENCODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == ENCODE_3)
			{
				KNL_ImgEncInit(ENCODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
		}
	}
	
	//For H264 Decode
	//=======================================================================
	//Main
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_MAIN+i,KNL_NODE_H264_DEC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_MAIN+i,KNL_NODE_H264_DEC);
			if(tNodeInfo.ubCodecIdx == DECODE_0)
			{
				KNL_ImgDecInit(DECODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_1)
			{
				KNL_ImgDecInit(DECODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_2)
			{
				KNL_ImgDecInit(DECODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_3)
			{
				KNL_ImgDecInit(DECODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
		}
	}
	//Aux
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_AUX+i,KNL_NODE_H264_DEC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_AUX+i,KNL_NODE_H264_DEC);
			if(tNodeInfo.ubCodecIdx == DECODE_0)
			{
				KNL_ImgDecInit(DECODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_1)
			{
				KNL_ImgDecInit(DECODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_2)
			{
				KNL_ImgDecInit(DECODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_3)
			{
				KNL_ImgDecInit(DECODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
		}
	}
	//Sub
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_SUB+i,KNL_NODE_H264_DEC))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_SUB+i,KNL_NODE_H264_DEC);
			if(tNodeInfo.ubCodecIdx == DECODE_0)
			{
				KNL_ImgDecInit(DECODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_1)
			{
				KNL_ImgDecInit(DECODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_2)
			{
				KNL_ImgDecInit(DECODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
			else if(tNodeInfo.ubCodecIdx == DECODE_3)
			{
				KNL_ImgDecInit(DECODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
			}
		}
	}

	//For Rate-Control
	for(i=0;i<4;i++)
	{
		if(ubRC_GetFlg(i))
		{
			RC_Init(i);
		}
	}	
	
	//For BB
	ubNodeExist = ubKNL_ChkExistNode(KNL_NODE_COMM_TX_VDO)|ubKNL_ChkExistNode(KNL_NODE_COMM_RX_VDO)|ubKNL_ChkExistNode(KNL_NODE_COMM_TX_ADO)|ubKNL_ChkExistNode(KNL_NODE_COMM_RX_ADO);		
	if(ubNodeExist && (ubKNL_InitBBFlg == 0))
	{
		SET_SLOT_MODE tKNL_BbSlotMode;
		uint8_t ubKNL_PsValue = 0;

		BB_VariableInit(ulBUF_GetBlkBufAddr(0,BUF_BB_IP));
		
		for(i=0;i<6;i++)
		{
			ubKNL_LinkStatus[i] = BB_LOST_LINK;
			ubKNL_RtLinkStatus[i] = BB_LOST_LINK;
		}

		tKNL_BbSlotMode = (tKNL_Info.ubOpMode == KNL_OPMODE_VBM_4T)?BB_SLOT_4:
						  (tKNL_Info.ubOpMode == KNL_OPMODE_VBM_2T)?BB_SLOT_2:BB_SLOT_1;
		ubKNL_PsValue  = wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR);
		ubKNL_PsValue &= 0xF0;
		KNL_DisableWORFunc();
		if((!ubRTC_GetKey()) && (ubKNL_PsValue == RTC_PS_WOR_TAG))
		{			
			BB_Init(tKNL_BbSlotMode, NULL, 0,0x2B,0x2B,0,165);	//Step(n)
			ubKNL_WorSts = BB_ConfirmWakeUpInf();
		}
		else
			BB_Init(tKNL_BbSlotMode, NULL, 1,0x2B,0x2B,0,165);	//Step(n)

		osMessageQDef(KNL_QueBbFrmOk, 150, uint8_t);
		KNL_QueBbFrmOk = osMessageCreate(osMessageQ(KNL_QueBbFrmOk), NULL);
		BB_FrameOkReqisteredQueue(&KNL_QueBbFrmOk);
		
		osThreadDef(KNL_BbFrmMonitThread, KNL_BbFrmMonitThread, osPriorityAboveNormal, 1, 512);
		osThreadCreate(osThread(KNL_BbFrmMonitThread), NULL);

		//Step(n+1)
		//=============================================================
		osMessageQDef(KNL_QUETXVDO, 30, uint8_t);
		KNL_QueTxVdo = osMessageCreate(osMessageQ(KNL_QUETXVDO), NULL);
		osMessageQDef(KNL_QUETXADO, 30, uint8_t);
		KNL_QueTxAdo = osMessageCreate(osMessageQ(KNL_QUETXADO), NULL);
		osMessageQDef(KNL_QUERXVDO, 30, RX_DON);
		KNL_QueRxVdo = osMessageCreate(osMessageQ(KNL_QUERXVDO), NULL);
		osMessageQDef(KNL_QUERXADO, 30, RX_DON);
		KNL_QueRxAdo = osMessageCreate(osMessageQ(KNL_QUERXADO), NULL);
		osMessageQDef(KNL_LINKRPT, 30, LINK_REPORT);
		KNL_LinkQ = osMessageCreate(osMessageQ(KNL_LINKRPT), NULL);	

		BB_LinkStatusReqisteredQueue(&KNL_LinkQ);		
		BB_RxDataReqisteredQueue(&KNL_QueRxVdo,BB_DATA_VIDEO);
		BB_RxDataReqisteredQueue(&KNL_QueRxAdo,BB_DATA_AUDIO);

		osThreadDef(CommLinkMonitThread, KNL_CommLinkMonitThread, THREAD_PRIO_LINK_MONIT, 1, THREAD_STACK_LINK_MONIT);
		osThreadCreate(osThread(CommLinkMonitThread), NULL);

		osThreadDef(CommLinkUpdateThread, KNL_CommLinkUpdateThread, THREAD_PRIO_LINK_UPDATE, 1, THREAD_STACK_LINK_UPDATE);
		osThreadCreate(osThread(CommLinkUpdateThread), NULL);
		//=============================================================

	#if OP_STA
		BB_SetPacketRetryTime(BB_ADO,BB_TIME_ALWAYS);
		BB_SetPacketRetryTime(BB_VDO,BB_TIME_ALWAYS);
		BB_SetDataPath(BB_TX_ADO_MASTER_AP,BB_RX_ADO_MASTER_AP,BB_PAYLOAD_NONE);
	#endif
	#if OP_AP
		BB_SetPacketRetryTime(BB_ADO,BB_TIME_ALWAYS);
		BB_SetDataPath(BB_TX_ADO_STA1,BB_RX_ADO_ALL_STA,BB_PAYLOAD_NONE);
	#endif

		AFH_Start(AFH_ADA_CH,0,0);
		BB_Start(THREAD_STACK_BB_HANDLER, THREAD_PRIO_BB_HANDLER);
		if((!ubRTC_GetKey()) && (ubKNL_PsValue == RTC_PS_WOR_TAG))
		{
			if(!ubKNL_WorSts)
			{
				printd(DBG_CriticalLvl, "Not to me\n");
				KNL_EnableWORFunc();
				return;
			}
		}
		EN_Start(THREAD_STACK_EN_HANDLER, THREAD_PRIO_EN_HANDLER);

	#if OP_AP	
		osThreadDef(CommVdoRxMonitThread, KNL_CommVdoRxMonitThread, THREAD_PRIO_COMM_RX_VDO, 1, THREAD_STACK_COMM_RX_VDO);
		osThreadCreate(osThread(CommVdoRxMonitThread), NULL);
	#endif

		osThreadDef(CommAdoRxMonitThread, KNL_CommAdoRxMonitThread, THREAD_PRIO_COMM_RX_ADO, 1, THREAD_STACK_COMM_RX_ADO);
		osThreadCreate(osThread(CommAdoRxMonitThread), NULL);
		
		if(tTWC_RegTransCbFunc(TWC_RESEND_I,KNL_TwcResult,KNL_ResendIAction) == TWC_FAIL)
		{
			printd(DBG_ErrorLvl, "Register RESEND_I TWC Fail !!!\r\n");
		}

		if(tTWC_RegTransCbFunc(TWC_VDORES_SETTING,KNL_TwcResult,KNL_VdoResSetting) == TWC_FAIL)
		{
			printd(DBG_ErrorLvl, "Register Video Resolution TWC Fail !!!\r\n");
		}

		ubKNL_InitBBFlg = 1;
	}

	//For LCD
	//=======================================================================
	if(ubKNL_ChkExistNode(KNL_NODE_LCD))
	{
	#if KNL_LCD_FUNC_ENABLE
		if(ubKNL_SetDispCropScaleParam() == 0)
		{
			printd(DBG_ErrorLvl, "Set Crop & Scale Parameter Fail !!!\r\n");
		}
		LCD_Start();
	#endif
	}
	else
		LCD_FuncDisable;

	//For Audio
	//=======================================================================
	if((ubKNL_ChkExistNode(KNL_NODE_ADC) || ubKNL_ChkExistNode(KNL_NODE_DAC)) && (ubKNL_InitAdoFlg == 0))
	{
		tKNL_AdoInfo.ubQ_InitFlg = 1;
		tKNL_AdoInfo.ulADO_BufStartAddr = ulBUF_GetBlkBufAddr(0,BUF_ADO_IP);

		ADO_Setup(&tKNL_AdoInfo,&tKNL_EncEventQue,&tKNL_DecEventQue);

		//---------------------------------
		// Sigma-delta ADC gain
		//---------------------------------
       	//ADO_SetSigmaDeltaAdcGain(ADO_SIG_BOOST_37DB, ADO_SIG_PGA_33DB);
       	#ifdef VBM_PU
		ADO_SetSigmaDeltaAdcGain(ADO_SIG_BOOST_0DB, ADO_SIG_PGA_3DB); //20180524
		#endif

		#ifdef VBM_BU
		ADO_SetSigmaDeltaAdcGain(ADO_SIG_BOOST_0DB, ADO_SIG_PGA_16p5DB); //20180524
		#endif

		//SDADC->AGC_OFF = 1;
		//ADO_SetAdcMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
		
		//---------------------------------
		// ADC djust functions
		//---------------------------------
		ADO_SetAdcDcComp(0xA, 0x14, 0x25, ADO_OFF);	// DC compensation

		//---------------------------------
		// DAC djust functions
		//---------------------------------
		//ADO_SetDacR2RVol(R2R_VOL_n0DB);	// DAC R2R volume
		ADO_SetDacM2so(ADO_OFF);		// M2SO
		//====================================	

		osThreadDef(ADOEncProcessThread, KNL_AdoEncMonitThread, THREAD_PRIO_ADO_ENC_PROC, 1, THREAD_STACK_ADO_PROC);
		osThreadCreate(osThread(ADOEncProcessThread), NULL);

		osThreadDef(ADODecProcessThread, KNL_AdoDecMonitThread, THREAD_PRIO_ADO_DEC_PROC, 1, THREAD_STACK_ADO_PROC);
		osThreadCreate(osThread(ADODecProcessThread), NULL);

		ubKNL_InitAdoFlg = 1;
	}

	//For JPEG Codec
	//=======================================================================
	if((ubKNL_ChkExistNode(KNL_NODE_JPG_ENC)||ubKNL_ChkExistNode(KNL_NODE_JPG_DEC1)||ubKNL_ChkExistNode(KNL_NODE_JPG_DEC2))&&(ubKNL_InitJpegFlg==0))
	{
		JPEG_Init();
		KNL_SetJpegQp(32);

		osSemaphoreDef(JPEG_CodecSem);
		JPEG_CodecSem = osSemaphoreCreate(osSemaphore(JPEG_CodecSem), 1);

        osMessageQDef(KNL_JPGMONITOR, KNL_JPEG_QUEUE_NUM, JPEG_CODEC_INFO_t);
        KNL_QueueJpegMonit = osMessageCreate(osMessageQ(KNL_JPGMONITOR), NULL);

        osThreadDef(JpegMonitThread, KNL_JpegMonitThread, THREAD_PRIO_JPEG_MONIT, 1, THREAD_STACK_JPEG_MONIT);
        osThreadCreate(osThread(JpegMonitThread), NULL);

		ubKNL_InitJpegFlg = 1;
	}
	else
		JPEG_FuncDisable;

#if USBD_ENABLE
	//For USB Device
	USBD_Start();
#endif

	SD_FuncDisable;
#if KNL_REC_FUNC_ENABLE
    //For FS
    if(1)
    {
        tAPP_Fs.ulFS_BufStartAddr = ulBUF_GetBlkBufAddr(0,BUF_FS);
        tAPP_Fs.Mode = FS_RES_MODE_HDx1;
        FS_Init(&tAPP_Fs);
        printd(DBG_Debug3Lvl, "FS buf %d bytes\r\n",ulFS_GetTotalBufSize(FS_RES_MODE_HDx1));
    }
    
    //For REC
    if(1)
    {
        vREC_AdoEnableSet(1);

        memset(&sRecExtFunc,0x0,sizeof(REC_EXTFUNC_CTRL));
        sRecExtFunc.ulAdoSkipFrameAdrGet = ulAPP_ADO_GetMemInitValueAdr;
        sRecExtFunc.ulAdoSkipFrameSizeGet = ulAPP_ADO_GetWifiPacketSize;
        sRecExtFunc.Time1msCntGet = APP_TIMER_Get1ms;
        sRecExtFunc.EventRecOnceEnd = APP_RecordOnceEnd_SDK;
        ubREC_ExternFuncInit(&sRecExtFunc);

        REC_Init();

        //----------------------------------------
        // pre-record setting -> no use
        printd(DBG_Debug3Lvl, "PrecT=%d \r\n",ubREC_PrecordTime(30,15,0x5000,(0x30000*4)));
        REC_PreMemInit(0);	
        printd(DBG_Debug3Lvl, "ulREC_PreGetBufSz:0x%x\r\n", ulREC_PreGetBufSz());
        ubREC_PreStart();
        //----------------------------------------

        Media_SemaphoreCreate();
        Media_Init(ulBUF_GetBlkBufAddr(0,BUF_REC));
        printd(DBG_Debug3Lvl, "ulMedia_GetBufSz:0x%x\r\n", ulMedia_GetBufSz());

        // Test Send Video and Audio Frame
        APP_1MSCounter();
    }
#endif

	USBH_FuncDisable;
#if KNL_USBH_FUNC_ENABLE
	USBH_Init(ulBUF_GetBlkBufAddr(0,BUF_USBH_IP));
#endif
}
//------------------------------------------------------------------------------
void KNL_SetRole(uint8_t ubRole)
{
	tKNL_Info.ubRole = ubRole;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetRole(void)
{
	return tKNL_Info.ubRole;
}
//------------------------------------------------------------------------------
void KNL_SetOpMode(uint8_t ubOpMode)
{
	tKNL_Info.ubOpMode = ubOpMode;
}

uint8_t ubKNL_GetOpMode(void)
{
	return tKNL_Info.ubOpMode;
}

void KNL_SetAuxInfoFunc(uint8_t ubEnable)
{
	tKNL_Info.ubAuxInfoFlg = ubEnable;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetAuxInfoFunc(void)
{	
	return tKNL_Info.ubAuxInfoFlg;
}
//------------------------------------------------------------------------------
uint32_t ulKNL_AddAuxInfo(KNL_PACKET_TYPE tPktType,uint8_t ubSrcNum,uint32_t ulAddr,uint32_t ulSize,uint32_t ulFrmIdx,uint32_t ulGop,uint8_t ubVdoGroupIdx)
{
//	uint8_t ubCrc8 = 0;
//	CRC_t tCRC2_Setup;
	uint32_t ulTemp;
	uint32_t ulRtnSz;
	uint32_t i;
	uint32_t ulDmy0Time;
	uint32_t ulCrcCalSz;

	//Case (%16) = 0
	//-------------------------------------------
	//|							 				|
	//| Original Size			 				|
	//|							 				|
	//-------------------------------------------
	//|CrcCalSz(4)|(12)			 				|
	//-------------------------------------------
	//|Crc(4)|  (12)   			 				|
	//--------------------------------------------------
	//|SrcNum(1)|OpMode(1)|FrmIdx(4)|GOP(4)|VGOP(1)|(5)|
	//--------------------------------------------------

	if(!ubKNL_GetAuxInfoFunc())
	{
		return ulSize;
	}

	if(tPktType == KNL_VDO_PKT)
	{
		ulTemp = ulSize;
		if((ulTemp % 16) == 0)
		{
			ulRtnSz    = ulTemp + 16 + 16 + 16;
			ulCrcCalSz = ulTemp;
		}
		else
		{
			ulRtnSz    = ((ulTemp/16)*16) + (16*3) + 16;
			ulCrcCalSz = ((ulTemp/16)*16) + 16;
			//Padd 0 value
			ulDmy0Time = 16 - (ulSize%16);
			for(i=0;i<ulDmy0Time;i++)
			{
				*((uint8_t *)(ulAddr+ulSize+i)) = 0;
			}
		}

		//CRC-Information
		if(KNL_AUX_CRC_FUNC)
		{
			ulCrcCalSz = ulCrcCalSz;
//			tCRC2_Setup.CRC_INIT_VALUE = INIT_ALL_ZERO;
//			tCRC2_Setup.CRC_FINAL_XOR_VALUE = XOR_ALL_ZERO;
//			tCRC2_Setup.CRC_ORDER = 7;
//			ubCrc8 = (uint8_t)ulCRC2_Calc(tCRC2_Setup, CRC_P_8, ulAddr, ulCrcCalSz);
//			//printd(DBG_Debug3Lvl, "crc2:0x%x\r\n",ubCrc8);
//			//printd(DBG_Debug3Lvl, "ulCrcCalSz:0x%x\r\n",ulCrcCalSz);
//			
//			*((uint32_t *)(ulAddr+ulRtnSz-16-16-16)) = ulCrcCalSz;
//			*((uint32_t *)(ulAddr+ulRtnSz-16-16)) = (uint32_t)ubCrc8;
		}

		//Aux-Information
		*((uint8_t *)(ulAddr+ulRtnSz-16)) = ubSrcNum;									//SrcNum Information
		*((uint8_t *)(ulAddr+ulRtnSz-15)) = ubKNL_GetOpMode();				//OpMode Information

		*((uint8_t *)(ulAddr+ulRtnSz-14)) = (uint8_t)(((ulFrmIdx&0x000000FF)>>0));	//Frame Index Information
		*((uint8_t *)(ulAddr+ulRtnSz-13)) = (uint8_t)(((ulFrmIdx&0x0000FF00)>>8));
		*((uint8_t *)(ulAddr+ulRtnSz-12)) = (uint8_t)(((ulFrmIdx&0x00FF0000)>>16));
		*((uint8_t *)(ulAddr+ulRtnSz-11)) = (uint8_t)(((ulFrmIdx&0xFF000000)>>24));	

		*((uint8_t *)(ulAddr+ulRtnSz-10)) = (uint8_t)(((ulGop&0x000000FF)>>0));	//Codec GOP Information
		*((uint8_t *)(ulAddr+ulRtnSz-9))  = (uint8_t)(((ulGop&0x0000FF00)>>8));
		*((uint8_t *)(ulAddr+ulRtnSz-8))  = (uint8_t)(((ulGop&0x00FF0000)>>16));
		*((uint8_t *)(ulAddr+ulRtnSz-7))  = (uint8_t)(((ulGop&0xFF000000)>>24));

		*((uint8_t *)(ulAddr+ulRtnSz-6))  = ubVdoGroupIdx;	//Video Group Information
		*((uint8_t *)(ulAddr+ulRtnSz-5))  = uwKNL_GetVdoH(ubSrcNum) >> 8;
		*((uint8_t *)(ulAddr+ulRtnSz-4))  = uwKNL_GetVdoH(ubSrcNum) & 0xFF;
		*((uint8_t *)(ulAddr+ulRtnSz-3))  = uwKNL_GetVdoV(ubSrcNum) >> 8;
		*((uint8_t *)(ulAddr+ulRtnSz-2))  = uwKNL_GetVdoV(ubSrcNum) & 0xFF;
		*((uint8_t *)(ulAddr+ulRtnSz-1))  = (uint8_t)(ulRtnSz - ulTemp);
	}
	else if(tPktType == KNL_ADO_PKT)
	{
		ulTemp = ulSize;

		if((ulTemp % KNL_ADO_SUB_PKT_LEN) == 0)
		{
			ulRtnSz = ulTemp+KNL_ADO_SUB_PKT_LEN;
		}
		else
		{
			if((ulTemp+16)  <= (((ulTemp/KNL_ADO_SUB_PKT_LEN)*KNL_ADO_SUB_PKT_LEN)+KNL_ADO_SUB_PKT_LEN))
			{
				ulRtnSz = ((ulTemp/KNL_ADO_SUB_PKT_LEN)*KNL_ADO_SUB_PKT_LEN)+KNL_ADO_SUB_PKT_LEN;
			}
			else
			{
				ulRtnSz = ((ulTemp/KNL_ADO_SUB_PKT_LEN)*KNL_ADO_SUB_PKT_LEN)+(KNL_ADO_SUB_PKT_LEN*2);
			}
		}
		//Aux-Information
		*((uint8_t *)(ulAddr+ulRtnSz-16)) = ubSrcNum;								//SrcNum Information
		
		*((uint8_t *)(ulAddr+ulRtnSz-6))  = (uint8_t)(((ulSize&0x000000FF)>>0));	//Real-Size Information
		*((uint8_t *)(ulAddr+ulRtnSz-5))  = (uint8_t)(((ulSize&0x0000FF00)>>8));
		*((uint8_t *)(ulAddr+ulRtnSz-4))  = (uint8_t)(((ulSize&0x00FF0000)>>16));
		*((uint8_t *)(ulAddr+ulRtnSz-3))  = (uint8_t)(((ulSize&0xFF000000)>>24));
	}

	return ulRtnSz;
}
//------------------------------------------------------------------------------
void KNL_SetJpegQp(uint8_t ubQp)
{
	tKNL_Info.ubJpegCodecQp = ubQp;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetJpegQp(void)
{
	return tKNL_Info.ubJpegCodecQp;
}

//------------------------------------------------------------------------------
void KNL_SetNodeState(uint8_t ubSrcNum,uint8_t ubNode,uint8_t ubState)
{
	tKNL_NodeState[ubSrcNum][ubNode] = (KNL_NODE_STATE)ubState;
}
//------------------------------------------------------------------------------
void KNL_TwcResult(TWC_TAG GetSta,TWC_STATUS ubStatus)
{
	if(ubStatus == TWC_SUCCESS)
	{
		ubKNL_TwcResult = TWC_SUCCESS;
	}
	else if(ubStatus == TWC_FAIL)
	{
		ubKNL_TwcResult = TWC_FAIL;
	}
	ubKNL_TwcEndFlg = 1;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_TwcSend(uint8_t ubRole,TWC_OPC Opc,uint8_t *Data,uint8_t ubLen,uint8_t ubRetry)
{
#define KNL_TWC_RETRY_MAX	50
	uint8_t ubStatus = TWC_FAIL, ubRetryCnt = 0;
	TWC_TAG Tag;

	osSemaphoreWait(tKNL_TwcSem, osWaitForever);

	if(ubRole == KNL_STA1)
	{
		Tag = TWC_STA1; 
	}
	else if(ubRole == KNL_STA2)
	{
		Tag = TWC_STA2;
	}
	else if(ubRole == KNL_STA3)
	{
		Tag = TWC_STA3;
	}
	else if(ubRole == KNL_STA4)
	{
		Tag = TWC_STA4;
	}
	else if(ubRole == KNL_SLAVE_AP)
	{
		Tag = TWC_AP_SLAVE;
	}
	else if(ubRole == KNL_MASTER_AP)
	{
		Tag = TWC_AP_MASTER;
	}

	ubKNL_TwcEndFlg = 0;
	if(tTWC_Send(Tag,Opc,Data,ubLen,ubRetry) != TWC_SUCCESS)
	{
		ubKNL_TwcEndFlg = 1;
		ubStatus = TWC_FAIL;
	}
	else
	{
		while(ubKNL_TwcEndFlg == 0)
		{
			osDelay(10);
			if(++ubRetryCnt >= KNL_TWC_RETRY_MAX)
			{
				ubKNL_TwcResult = TWC_FAIL;
				break;
			}
		}
		ubStatus = ubKNL_TwcResult;
		tTWC_StopTwcSend(Tag, Opc);
	}
	if(TWC_FAIL == ubStatus)
	{
		switch(Opc)
		{
			case TWC_RESEND_I:
				ubKNL_VdoResendITwcFlg[ubRole] = FALSE;
				break;
			case TWC_VDORES_SETTING:
				ubKNL_VdoResChgTwcFlg[ubRole]  = FALSE;
				break;
			default:
				break;
		}
	}

	osSemaphoreRelease(tKNL_TwcSem);

	return ubStatus;
}

//------------------------------------------------------------------------------
void KNL_NodeStateReset(void)
{
	uint16_t i,j;
	
	for(j=0;j<KNL_SRC_NUM;j++)
	{
		for(i=0;i<256;i++)
		{
			tKNL_NodeState[j][i] = KNL_NODE_STOP;	
		}
	}
}
//------------------------------------------------------------------------------
void KNL_VdoReset(void)
{
	uint8_t i;

	for(i=0;i<KNL_SRC_NUM;i++)
	{
		ubKNL_VdoFlowActiveFlg[i] = 0;
		ubKNL_RcvFirstIFrame[i] = 0;

		ulKNL_OutVdoFpsCnt[i] = 0;
		ulKNL_OutVdoFpsCntTemp[i] = 0;

		ulKNL_InVdoFpsCnt[i] = 0;
		ulKNL_InVdoFpsCntTemp[i] = 0;
	}
	for(i=0;i<KNL_MAX_ROLE;i++)
	{
		ulKNL_FrmTRxNum[i] = 0;
		ulKNL_FrmTRxNumTemp[i] = 0;
	}

	//Reset Status Report
	ulKNL_VdoOutAccCnt[0] = 0;		//Bit-Rate
	ulKNL_VdoOutAccCnt[1] = 0;		//Bit-Rate
	ulKNL_VdoOutAccCnt[2] = 0;		//Bit-Rate
	ulKNL_VdoOutAccCnt[3] = 0;		//Bit-Rate
	
	ulKNL_VdoOutAccCntTemp[0] = 0;	//Bit-Rate
	ulKNL_VdoOutAccCntTemp[1] = 0;	//Bit-Rate
	ulKNL_VdoOutAccCntTemp[2] = 0;	//Bit-Rate
	ulKNL_VdoOutAccCntTemp[3] = 0;	//Bit-Rate
	
	ulKNL_AdoOutAccCnt = 0;		//Bit-Rate
	ulKNL_AdoOutAccCntTemp = 0;	//Bit-Rate
}
//------------------------------------------------------------------------------
void KNL_SetMultiOutNode(uint8_t ubNode,uint8_t ubEnable,uint8_t ubInSrc,uint8_t ubOutSrc1,uint8_t ubOutSrc2)
{
	tKNL_Info.ubMultiOutFlg[ubNode]		= ubEnable;
	tKNL_Info.ubMultiInSrc[ubNode] 		= ubInSrc;
	tKNL_Info.ubMultiOutSrc1[ubNode] 	= ubOutSrc1;
	tKNL_Info.ubMultiOutSrc2[ubNode] 	= ubOutSrc2;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkMultiOutNode(uint8_t ubNode)
{
	return tKNL_Info.ubMultiOutFlg[ubNode];
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetMultiInSrc(uint8_t ubNode)
{
	return tKNL_Info.ubMultiInSrc[ubNode];
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetMultiOutSrc(uint8_t ubNode,uint8_t ubOutSrc)
{
	if(ubOutSrc == 0)
	{
		return tKNL_Info.ubMultiOutSrc1[ubNode];
	}
	else if(ubOutSrc == 1)
	{
		return tKNL_Info.ubMultiOutSrc2[ubNode];
	}
	else
	{
		printd(DBG_ErrorLvl, "Err @ubKNL_GetMultiOutSrc\r\n");
		return 0;
	}
}
//------------------------------------------------------------------------------
void KNL_SetLcdDmyImgH(uint16_t uwH)
{
	tKNL_Info.uwLcdDmyImgH = uwH;
}
//------------------------------------------------------------------------------
uint16_t uwKNL_GetLcdDmyImgH(void)
{
	return tKNL_Info.uwLcdDmyImgH;
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetImgMergeBufSz(void)
{	
	uint32_t ulBufSz1,ulBufSz2,ulBufSz3;
	uint32_t ulBufSz;		
	uint8_t ubDisp1Src,ubDisp2Src,ubDisp3Src,ubDisp4Src;

	ulBufSz1 	= ulBufSz1;
	ulBufSz2 	= ulBufSz2;
	ulBufSz3 	= ulBufSz3;
	ulBufSz		= ulBufSz;
	ubDisp1Src	= ubDisp1Src;
	ubDisp2Src	= ubDisp2Src;
	ubDisp3Src	= ubDisp3Src;
	ubDisp4Src	= ubDisp4Src;
	
#if BUC_CU
	ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
	ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
	ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
	ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);
	
	//H-View
	if((tKNL_GetDispType() == KNL_DISP_H )&&(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0) )
	{			
		ulBufSz1 = (((uint32_t)uwKNL_GetVdoH(ubDisp1Src))*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5);	//L Image
		ulBufSz2 = (((uint32_t)uwKNL_GetLcdDmyImgH())*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5);			//Dummy Image
		ulBufSz3 = (((uint32_t)uwKNL_GetVdoH(ubDisp4Src))*((uint32_t)uwKNL_GetVdoV(ubDisp4Src))*1.5);	//R Image
		
		ulBufSz = ulBufSz1+ulBufSz2+ulBufSz3;
		return ulBufSz;					
	}
	//Drive-View
	else if((tKNL_GetDispType() == KNL_DISP_3T_1T2B)&&(tKNL_GetDispRotate()== KNL_DISP_ROTATE_0))
	{		
		ulBufSz2 = (((uint32_t)uwKNL_GetVdoH(ubDisp1Src))*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5);	//L Image
		ulBufSz3 = (((uint32_t)uwKNL_GetVdoH(ubDisp3Src))*((uint32_t)uwKNL_GetVdoV(ubDisp3Src))*1.5);	//R Image
		
		ulBufSz = ulBufSz2+ulBufSz3;
		return ulBufSz;
	}
	//Reverse-View
	else if((tKNL_GetDispType() == KNL_DISP_3T_2T1B)&&(tKNL_GetDispRotate()== KNL_DISP_ROTATE_0))
	{	
		ulBufSz1 = (((uint32_t)uwKNL_GetVdoH(ubDisp1Src))*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5);	//L Image		
		ulBufSz2 = (((uint32_t)uwKNL_GetVdoH(ubDisp2Src))*((uint32_t)uwKNL_GetVdoV(ubDisp2Src))*1.5);	//R Image
		
		ulBufSz = ulBufSz1+ulBufSz2;
		return ulBufSz;
	}
	
#endif
	
	printd(DBG_ErrorLvl, "Err @ulKNL_GetImgMergeBufSz\r\n");
	return 0;
}

//------------------------------------------------------------------------------
void KNL_SetDispRotate(KNL_DISP_ROTATE tRotateType)
{
	tKNL_Info.tDispRotate = tRotateType;
}
//------------------------------------------------------------------------------
KNL_DISP_ROTATE tKNL_GetDispRotate(void)
{
	return tKNL_Info.tDispRotate;
}
//------------------------------------------------------------------------------
void KNL_SetDispHV(uint16_t uwDispH,uint16_t uwDispV)
{
	tKNL_Info.uwDispH = uwDispH;
	tKNL_Info.uwDispV = uwDispV;
}
//------------------------------------------------------------------------------
void KNL_SetDispType(KNL_DISP_TYPE tDispType)
{
	tKNL_Info.tDispType	= tDispType;
}
//------------------------------------------------------------------------------
KNL_DISP_TYPE tKNL_GetDispType(void)
{
	return tKNL_Info.tDispType;
}
//------------------------------------------------------------------------------
KNL_DISP_LOCATION tKNL_SearchSrcLocation(uint8_t ubSrcNum)
{
	uint8_t i;

	for(i = 0; i < 4; i++)
	{
		if(KNL_SwDispInfo.tSrcNum[i] == ubSrcNum)
			return KNL_SwDispInfo.tSrcLocate[i];
	}
	return KNL_DISP_LOCATION_ERR;
}
//------------------------------------------------------------------------------
void KNL_ModifyDispType(KNL_DISP_TYPE tDispType, KNL_SrcLocateMap_t tSrcLocate)
{
#if KNL_LCD_FUNC_ENABLE
	LCD_INFOR_TYP sLcdInfor;
	uint8_t ubDisp1Src;
	uint8_t ubDisp2Src;
	uint8_t ubDisp3Src;
	uint8_t ubDisp4Src;
	static KNL_DISP_TYPE tKNL_DispType;

	//Step1
	ubKNL_DispCh0ActiveFlg = 0;
	ubKNL_DispCh1ActiveFlg = 0;
	ubKNL_DispCh2ActiveFlg = 0;
	ubKNL_DispCh3ActiveFlg = 0;

	//Step2
	tKNL_DispType = tKNL_GetDispType();
	if((tKNL_DispType != tDispType) &&
	   (LCD_JPEG_DISABLE == tLCD_GetJpegDecoderStatus()))
	{
		LCD_ChDisable(LCD_CH0);
		LCD_ChDisable(LCD_CH1);
		LCD_ChDisable(LCD_CH2);
		LCD_ChDisable(LCD_CH3);
	}

	//Get Correspond Source
	ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
	ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
	ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
	ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);

	KNL_SetDispType(tDispType);

	memcpy(&KNL_SwDispInfo, &tSrcLocate, sizeof(KNL_SrcLocateMap_t));

	if(tDispType == KNL_DISP_SINGLE)
	{
		sLcdInfor.tDispType = LCD_DISP_1T;
		sLcdInfor.ubChNum = 1;

		KNL_SwDispInfo.ubSetupFlag = FALSE;
		ubDisp1Src = KNL_SwDispInfo.tSrcNum[0];
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);
		}
		else
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoH(ubDisp1Src);
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_DUAL_U)
	{
		sLcdInfor.tDispType = LCD_DISP_2T_H;
		sLcdInfor.ubChNum = 2;

		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			ubDisp1Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);

			ubDisp2Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);	
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);	
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);
		}
		else
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			ubDisp1Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoH(ubDisp1Src);

			ubDisp2Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoH(ubDisp2Src);
		}
		if(TRUE == KNL_SwDispInfo.ubSetupFlag)
		{
			uint8_t ubRole[2];
			uint8_t i;

			for(i = 0; i < 2; i++)
				ubRole[i] = ubKNL_SrcNumMap(KNL_SwDispInfo.tSrcNum[i]);
			if(ubKNL_GetCommLinkStatus(ubRole[0]) == BB_LOST_LINK)
				LCD_ChDisable(LCD_CH1);
			if(ubKNL_GetCommLinkStatus(ubRole[1]) == BB_LOST_LINK)
				LCD_ChDisable(LCD_CH0);
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_QUAD)
	{
		sLcdInfor.tDispType = LCD_DISP_4T;
		sLcdInfor.ubChNum = 4;
		
		KNL_SwDispInfo.ubSetupFlag = FALSE;
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);

			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);

			ubDisp3Src = (KNL_SRC_NONE == ubDisp3Src)?KNL_SRC_3_MAIN:ubDisp3Src;
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoV(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoV(ubDisp3Src);

			ubDisp4Src = (KNL_SRC_NONE == ubDisp4Src)?KNL_SRC_4_MAIN:ubDisp4Src;
			sLcdInfor.tChRes[3].uwChInputHsize = uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[3].uwChInputVsize = uwKNL_GetVdoV(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropHstart = 0;
			sLcdInfor.tChRes[3].uwCropVstart = 0;
			sLcdInfor.tChRes[3].uwCropHsize = uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropVsize = uwKNL_GetVdoV(ubDisp4Src);
		}
		else
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoH(ubDisp1Src);

			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoH(ubDisp2Src);

			ubDisp3Src = (KNL_SRC_NONE == ubDisp3Src)?KNL_SRC_3_MAIN:ubDisp3Src;
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoV(ubDisp3Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoV(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoH(ubDisp3Src);

			ubDisp4Src = (KNL_SRC_NONE == ubDisp4Src)?KNL_SRC_4_MAIN:ubDisp4Src;
			sLcdInfor.tChRes[3].uwChInputHsize = uwKNL_GetVdoV(ubDisp4Src);
			sLcdInfor.tChRes[3].uwChInputVsize = uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropHstart = 0;
			sLcdInfor.tChRes[3].uwCropVstart = 0;
			sLcdInfor.tChRes[3].uwCropHsize = uwKNL_GetVdoV(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropVsize = uwKNL_GetVdoH(ubDisp4Src);
		}		
	}
	else if(tKNL_GetDispType() == KNL_DISP_DUAL_C)
	{
		sLcdInfor.tDispType = LCD_DISP_2T_V;
		sLcdInfor.ubChNum = 2;

		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			//Source[0]
			ubDisp1Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart   = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?210:0;
			sLcdInfor.tChRes[0].uwCropVstart   = 0;
			sLcdInfor.tChRes[0].uwCropHsize    = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?860:uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize    = uwKNL_GetVdoV(ubDisp1Src);

			//Source[1]
			ubDisp2Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart   = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?210:0;
			sLcdInfor.tChRes[1].uwCropVstart   = 0;
			sLcdInfor.tChRes[1].uwCropHsize    = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?860:uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize    = uwKNL_GetVdoV(ubDisp2Src);
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			//Source[0]
			ubDisp1Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?210:0;
			sLcdInfor.tChRes[0].uwCropHsize  = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize  = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?860:uwKNL_GetVdoH(ubDisp1Src);

			//Source[1]
			ubDisp2Src = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?210:0;
			sLcdInfor.tChRes[1].uwCropHsize  = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize  = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?860:uwKNL_GetVdoH(ubDisp2Src);
		}
		if(TRUE == KNL_SwDispInfo.ubSetupFlag)
		{
			uint8_t ubRole[2];
			uint8_t i;

			for(i = 0; i < 2; i++)
				ubRole[i] = ubKNL_SrcNumMap(KNL_SwDispInfo.tSrcNum[i]);
			if(ubKNL_GetCommLinkStatus(ubRole[0]) == BB_LOST_LINK)
				LCD_ChDisable(LCD_CH1);
			if(ubKNL_GetCommLinkStatus(ubRole[1]) == BB_LOST_LINK)
				LCD_ChDisable(LCD_CH0);
		}
	}
	if(NULL == pLcdCropScaleParam)
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	else
		memcpy(pLcdCropScaleParam, &sLcdInfor, sizeof(LCD_INFOR_TYP));
	ubKNL_LcdDispParamActiveFlg = 1;
	//Step3
	ubKNL_DispCh0ActiveFlg = 1;
	ubKNL_DispCh1ActiveFlg = 1;
	ubKNL_DispCh2ActiveFlg = 1;
	ubKNL_DispCh3ActiveFlg = 1;
#endif
}
//------------------------------------------------------------------------------
uint32_t ulKNL_CalLcdBufSz(void)
{
#if KNL_LCD_FUNC_ENABLE
	LCD_CALBUF_TYP	sInfor;
	KNL_DISP_TYPE tDispType;
	uint8_t ubDisp1Src;
	uint8_t ubDisp2Src;
	uint8_t ubDisp3Src;
	uint8_t ubDisp4Src;		

	sInfor.uwLcdHSize = tKNL_Info.uwDispH;	//Target H of LCD
	sInfor.uwLcdVSize = tKNL_Info.uwDispV;	//Target V of LCD

	//Get Correspond Source
	ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
	ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
	ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
	ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);

	tDispType = tKNL_GetDispType();
	if(tDispType == KNL_DISP_SINGLE)
	{
		sInfor.ubChMax = 1;

		//For 0
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[0].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[0].bJpegEn = TRUE;
		}
		ubDisp1Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[0])?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE != ubDisp1Src)?ubDisp1Src:KNL_SRC_1_MAIN;
		sInfor.tInput[0].uwHSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp1Src);
		sInfor.tInput[0].uwVSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp1Src);
	}
	else if(tDispType == KNL_DISP_DUAL_U)
	{
		sInfor.ubChMax = 2;
		
		//For 0
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[0].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[0].bJpegEn = TRUE;
		}
		ubDisp1Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[0])?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE != ubDisp1Src)?ubDisp1Src:KNL_SRC_1_MAIN;
		sInfor.tInput[0].uwHSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp1Src);
		sInfor.tInput[0].uwVSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp1Src);
		
		//For 1
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[1].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[1].bJpegEn = TRUE;
		}
		ubDisp2Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[1])?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE != ubDisp2Src)?ubDisp2Src:KNL_SRC_2_MAIN;
		sInfor.tInput[1].uwHSize = (KNL_SRC_NONE == ubDisp2Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp2Src);
		sInfor.tInput[1].uwVSize = (KNL_SRC_NONE == ubDisp2Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp2Src);
	}
	else if(tDispType == KNL_DISP_QUAD)
	{
		sInfor.ubChMax = 4;
		
		//For 0
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[0].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[0].bJpegEn = TRUE;
		}		
		sInfor.tInput[0].uwHSize = (KNL_SRC_NONE == ubDisp1Src)?uwKNL_GetVdoH(KNL_SRC_1_MAIN):uwKNL_GetVdoH(ubDisp1Src);
		sInfor.tInput[0].uwVSize = (KNL_SRC_NONE == ubDisp1Src)?uwKNL_GetVdoV(KNL_SRC_1_MAIN):uwKNL_GetVdoV(ubDisp1Src);		
		
		//For 1
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[1].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[1].bJpegEn = TRUE;
		}	
		sInfor.tInput[1].uwHSize = (KNL_SRC_NONE == ubDisp2Src)?uwKNL_GetVdoH(KNL_SRC_2_MAIN):uwKNL_GetVdoH(ubDisp2Src);
		sInfor.tInput[1].uwVSize = (KNL_SRC_NONE == ubDisp2Src)?uwKNL_GetVdoV(KNL_SRC_2_MAIN):uwKNL_GetVdoV(ubDisp2Src);
		
		//For 2
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[2].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[2].bJpegEn = TRUE;
		}
		sInfor.tInput[2].uwHSize = (KNL_SRC_NONE == ubDisp3Src)?uwKNL_GetVdoH(KNL_SRC_3_MAIN):uwKNL_GetVdoH(ubDisp3Src);
		sInfor.tInput[2].uwVSize = (KNL_SRC_NONE == ubDisp3Src)?uwKNL_GetVdoV(KNL_SRC_3_MAIN):uwKNL_GetVdoV(ubDisp3Src);
		
		//For 3
		if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
		{
			sInfor.tInput[3].bJpegEn = FALSE;
		}
		else
		{
			sInfor.tInput[3].bJpegEn = TRUE;
		}	
		sInfor.tInput[3].uwHSize = (KNL_SRC_NONE == ubDisp4Src)?uwKNL_GetVdoH(KNL_SRC_4_MAIN):uwKNL_GetVdoH(ubDisp4Src);
		sInfor.tInput[3].uwVSize = (KNL_SRC_NONE == ubDisp4Src)?uwKNL_GetVdoV(KNL_SRC_4_MAIN):uwKNL_GetVdoV(ubDisp4Src);
	}
	else if(tDispType == KNL_DISP_H)
	{
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sInfor.ubChMax = 3;
			
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}		
			sInfor.tInput[0].uwHSize = uwKNL_GetVdoH(ubDisp2Src);
			sInfor.tInput[0].uwVSize = uwKNL_GetVdoV(ubDisp2Src);	
			
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}				
			sInfor.tInput[1].uwHSize = uwKNL_GetVdoH(ubDisp1Src)+uwKNL_GetLcdDmyImgH()+uwKNL_GetVdoH(ubDisp4Src);
			sInfor.tInput[1].uwVSize = uwKNL_GetVdoV(ubDisp2Src);
			
			//For 2
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[2].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[2].bJpegEn = TRUE;
			}	
			sInfor.tInput[2].uwHSize = uwKNL_GetVdoH(ubDisp3Src);
			sInfor.tInput[2].uwVSize = uwKNL_GetVdoV(ubDisp3Src);		
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{
			sInfor.ubChMax = 3;
		}
	}	
	
	else if(tDispType == KNL_DISP_3T_1T2B)
	{
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{			
			sInfor.ubChMax = 2;
			
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}		
			sInfor.tInput[0].uwHSize = uwKNL_GetVdoH(ubDisp1Src);
			sInfor.tInput[0].uwVSize = uwKNL_GetVdoV(ubDisp1Src);	
			
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}	
			sInfor.tInput[1].uwHSize = uwKNL_GetVdoH(ubDisp2Src)+uwKNL_GetVdoH(ubDisp3Src);
			sInfor.tInput[1].uwVSize = uwKNL_GetVdoV(ubDisp2Src)+0;	
			
		}
	}	
	else if(tDispType == KNL_DISP_3T_2T1B)
	{
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{			
			sInfor.ubChMax = 2;
			
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}		
			sInfor.tInput[0].uwHSize = uwKNL_GetVdoH(ubDisp1Src)+uwKNL_GetVdoH(ubDisp2Src);
			sInfor.tInput[0].uwVSize = uwKNL_GetVdoV(ubDisp1Src);	
			
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}
			sInfor.tInput[1].uwHSize = uwKNL_GetVdoH(ubDisp3Src)/2;
			sInfor.tInput[1].uwVSize = uwKNL_GetVdoV(ubDisp3Src)/2;		
		}
	}
		
	else if(tDispType == KNL_DISP_3T_2L1R)
	{
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{			
			// For Kernel
			// -----------------
			// | Disp1 |	   |
			// | F(2)  |       |
			// |-------| Disp3 |
			// | Disp2 |  R(4) |
			// |  B(3) |	   |
			// -----------------	
			
			// For LCD IP
			// -----------------
			// |  CH0  |	   |
			// | 	   |       |
			// |-------|  CH1  |
			// |       |       |
			// |  CH2  |	   |
			// -----------------
			
			sInfor.ubChMax = 3;
			
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}		
			sInfor.tInput[0].uwHSize = uwKNL_GetVdoH(ubDisp1Src);
			sInfor.tInput[0].uwVSize = uwKNL_GetVdoV(ubDisp1Src);			
	
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}		
			sInfor.tInput[1].uwHSize = uwKNL_GetVdoH(ubDisp3Src);
			sInfor.tInput[1].uwVSize = uwKNL_GetVdoV(ubDisp3Src);	
			
			//For 2
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[2].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[2].bJpegEn = TRUE;
			}		
			sInfor.tInput[2].uwHSize = uwKNL_GetVdoH(ubDisp2Src);
			sInfor.tInput[2].uwVSize = uwKNL_GetVdoV(ubDisp2Src);				
		}
	}	
	
	else if(tDispType == KNL_DISP_3T_1L2R)
	{
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{			
			// For Kernel	
			// -----------------
			// |   	   | Disp2 |
			// | 	   | F(2)  |
			// |  L(1) |-------|
			// | Disp1 |  B(3) |
			// |   	   | Disp3 |
			// -----------------
			
			// For LCD IP	
			// -----------------
			// |       |  CH0  |
			// | 	   |       |
			// |  CH1  |-------|
			// |       |       |
			// |       |  CH2  |
			// -----------------
			
			sInfor.ubChMax = 3;
			
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}		
			sInfor.tInput[0].uwHSize = uwKNL_GetVdoH(ubDisp2Src);
			sInfor.tInput[0].uwVSize = uwKNL_GetVdoV(ubDisp2Src);			
	
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}		
			sInfor.tInput[1].uwHSize = uwKNL_GetVdoH(ubDisp1Src);
			sInfor.tInput[1].uwVSize = uwKNL_GetVdoV(ubDisp1Src);	
			
			//For 2
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[2].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[2].bJpegEn = TRUE;
			}		
			sInfor.tInput[2].uwHSize = uwKNL_GetVdoH(ubDisp3Src);
			sInfor.tInput[2].uwVSize = uwKNL_GetVdoV(ubDisp3Src);				
		}
	}	
	
	else if(tDispType == KNL_DISP_DUAL_C)
	{
		// For Kernel	
		// -----------------
		// |   	 Disp1     |
		// | 	  F(2)     |
		// |---------------|
		// |      B(3)     |
		// |   	 Disp2     |
		// -----------------
		
		// For LCD IP	
		// -----------------
		// |	  CH0      |
		// | 	           |
		// |---------------|
		// |               |
		// |	  CH1      |
		// -----------------
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{			
			sInfor.ubChMax = 2;
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}
			ubDisp1Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[0])?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE != ubDisp1Src)?ubDisp1Src:KNL_SRC_1_MAIN;
			sInfor.tInput[0].uwHSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp1Src);
			sInfor.tInput[0].uwVSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp1Src);		
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}	
			ubDisp2Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[1])?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE != ubDisp2Src)?ubDisp2Src:KNL_SRC_2_MAIN;
			sInfor.tInput[1].uwHSize = (KNL_SRC_NONE == ubDisp2Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp2Src);
			sInfor.tInput[1].uwVSize = (KNL_SRC_NONE == ubDisp2Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp2Src);	
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{
			sInfor.ubChMax = 2;
			//For 0
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[0].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[0].bJpegEn = TRUE;
			}
			ubDisp1Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[0])?KNL_SwDispInfo.tSrcNum[0]:(KNL_SRC_NONE != ubDisp1Src)?ubDisp1Src:KNL_SRC_1_MAIN;
			sInfor.tInput[0].uwHSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp1Src);
			sInfor.tInput[0].uwVSize = (KNL_SRC_NONE == ubDisp1Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp1Src);
			//For 1
			if(ubKNL_GetVdoCodec() == KNL_VDO_CODEC_H264)
			{
				sInfor.tInput[1].bJpegEn = FALSE;
			}
			else
			{
				sInfor.tInput[1].bJpegEn = TRUE;
			}
			ubDisp2Src = (KNL_SRC_NONE != KNL_SwDispInfo.tSrcNum[1])?KNL_SwDispInfo.tSrcNum[1]:(KNL_SRC_NONE != ubDisp2Src)?ubDisp2Src:KNL_SRC_2_MAIN;
			sInfor.tInput[1].uwHSize = (KNL_SRC_NONE == ubDisp2Src)?sInfor.uwLcdVSize:uwKNL_GetVdoV(ubDisp2Src);
			sInfor.tInput[1].uwVSize = (KNL_SRC_NONE == ubDisp2Src)?sInfor.uwLcdHSize:uwKNL_GetVdoH(ubDisp2Src);
		}
	}

	sInfor.tAlign = LCD_BUF_1024BYTES_ALIG;
	return ulLCD_CalLcdBufSize(&sInfor);
#else
	return 0;
#endif	
}
//------------------------------------------------------------------------------
uint8_t ubKNL_SetDispCropScaleParam(void)
{
#if KNL_LCD_FUNC_ENABLE
	static LCD_INFOR_TYP sLcdInfor;
	uint8_t ubDisp1Src;
	uint8_t ubDisp2Src;
	uint8_t ubDisp3Src;
	uint8_t ubDisp4Src;
	
	if(tKNL_GetDispType() == KNL_DISP_SINGLE)
	{
		sLcdInfor.tDispType = LCD_DISP_1T;
		sLcdInfor.ubChNum = 1;
//		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
//		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();		

		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);		
	
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);
		}
		else
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoH(ubDisp1Src);
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}
	
	if(tKNL_GetDispType() == KNL_DISP_DUAL_U)
	{
		sLcdInfor.tDispType = LCD_DISP_2T_H;
		sLcdInfor.ubChNum = 2;
//		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
//		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();		

		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);		
		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			//Source[0]
			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);			
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);		
		
			//Source[1]
			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);		
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;		
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);			
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{			
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			//Source[0]
			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;				
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoH(ubDisp1Src);			
			
			//Source[1]
			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoH(ubDisp2Src);			
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;			
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoH(ubDisp2Src);				
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}
	
	if(tKNL_GetDispType() == KNL_DISP_QUAD)
	{
		sLcdInfor.tDispType = LCD_DISP_4T;
		sLcdInfor.ubChNum = 4;
//		sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
//		sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);		
		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			//Source[0]
			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);

			//Source[1]
			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);

			//Source[2]
			ubDisp3Src = (KNL_SRC_NONE == ubDisp3Src)?KNL_SRC_3_MAIN:ubDisp3Src;
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoV(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoV(ubDisp3Src);

			//Source[3]
			ubDisp4Src = (KNL_SRC_NONE == ubDisp4Src)?KNL_SRC_4_MAIN:ubDisp4Src;
			sLcdInfor.tChRes[3].uwChInputHsize = uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[3].uwChInputVsize = uwKNL_GetVdoV(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropHstart = 0;
			sLcdInfor.tChRes[3].uwCropVstart = 0;
			sLcdInfor.tChRes[3].uwCropHsize = uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropVsize = uwKNL_GetVdoV(ubDisp4Src);
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			//Source[0]
			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoH(ubDisp1Src);

			//Source[1]
			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoH(ubDisp2Src);

			//Source[2]
			ubDisp3Src = (KNL_SRC_NONE == ubDisp3Src)?KNL_SRC_3_MAIN:ubDisp3Src;
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoV(ubDisp3Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoV(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoH(ubDisp3Src);

			//Source[3]
			ubDisp4Src = (KNL_SRC_NONE == ubDisp4Src)?KNL_SRC_4_MAIN:ubDisp4Src;
			sLcdInfor.tChRes[3].uwChInputHsize = uwKNL_GetVdoV(ubDisp4Src);
			sLcdInfor.tChRes[3].uwChInputVsize = uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropHstart = 0;
			sLcdInfor.tChRes[3].uwCropVstart = 0;
			sLcdInfor.tChRes[3].uwCropHsize = uwKNL_GetVdoV(ubDisp4Src);
			sLcdInfor.tChRes[3].uwCropVsize = uwKNL_GetVdoH(ubDisp4Src);
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}
	if(tKNL_GetDispType() == KNL_DISP_H)
	{		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{				
			sLcdInfor.tDispType = LCD_DISP_3T_2PIP;
			sLcdInfor.ubChNum = 3;
			sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
			sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();			
			
			//Get Correspond Source
			ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
			ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
			ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
			ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);
			
			//Source[0]			
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);			
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);		
		
			//Source[1]						
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src)+uwKNL_GetLcdDmyImgH()+uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);		
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;					
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src)+uwKNL_GetLcdDmyImgH()+uwKNL_GetVdoH(ubDisp4Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);		
			
			//Source[2]
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoV(ubDisp3Src);			
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;		
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoV(ubDisp3Src);
		}	
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{
			printd(DBG_Debug3Lvl, "Need to Add !!!!\r\n");
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);		
	}
	
	if(tKNL_GetDispType() == KNL_DISP_3T_1T2B)
	{		
		sLcdInfor.tDispType = LCD_DISP_2T_V;
		sLcdInfor.ubChNum = 2;
		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();		
		
		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);
			
		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{	
			//Source[0]
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);			
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);		
		
			//Source[1]			
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src)+uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src)+0;		
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;		
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src)+uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src)+0;		
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{	
			if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
			{
				printd(DBG_Debug3Lvl, "Need to Add !!!!\r\n");
			}			
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}
	
	if(tKNL_GetDispType() == KNL_DISP_3T_2T1B)
	{		
		sLcdInfor.tDispType = LCD_DISP_2T_V;
		sLcdInfor.ubChNum = 2;
		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();		
		
		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);			
		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{	
			//Source[0]
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src)+uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);			
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src)+uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);		
		
			//Source[1]
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp3Src);		
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;		
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp3Src);		
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{	
			if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
			{
				printd(DBG_Debug3Lvl, "Need to Add !!!!\r\n");
			}			
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}
	
	if(tKNL_GetDispType() == KNL_DISP_3T_2L1R)
	{		
		sLcdInfor.tDispType = LCD_DISP_3T_2L1R;
		sLcdInfor.ubChNum = 3;
		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();		
		
		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);			
		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{	
			// For Kernel
			// -----------------
			// | Disp1 |       |
			// |  F(2) |       |
			// |-------| Disp3 |
			// | Disp2 |  R(4) |
			// |  B(3) |	   |
			// -----------------	
			
			// For LCD IP
			// -----------------
			// |  CH0  |       |
			// | 	   |       |
			// |-------|  CH1  |
			// |       |       |
			// |  CH2  |	   |
			// -----------------
			
			//Source[0]
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);			
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);			
	
			//Source[1]		
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp3Src);			
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;		
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp3Src);		

			//Source[2]		
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);			
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;		
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);
			
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{	
			if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
			{
				printd(DBG_Debug3Lvl, "Need to Add !!!!\r\n");
			}			
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}
	
	if(tKNL_GetDispType() == KNL_DISP_3T_1L2R)
	{		
		sLcdInfor.tDispType = LCD_DISP_3T_1L2R;
		sLcdInfor.ubChNum = 3;
		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();	
		
		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);			
		
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{	
			// For Kernel	
			// -----------------
			// |   	   | Disp2 |
			// | 	   |  F(2) |
			// |  L(1) |-------|
			// | Disp1 |  B(3) |
			// |   	   | Disp3 |
			// -----------------
			
			// For LCD IP	
			// -----------------
			// |   	   |  CH0  |
			// | 	   |       |
			// |  CH1  |-------|
			// |       |       |
			// |       |  CH2  |
			// -----------------			
			
			//Source[0]
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);			
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);			
	
			//Source[1]		
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);			
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;		
			sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);		

			//Source[2]		
			sLcdInfor.tChRes[2].uwChInputHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwChInputVsize = uwKNL_GetVdoV(ubDisp3Src);			
			sLcdInfor.tChRes[2].uwCropHstart = 0;
			sLcdInfor.tChRes[2].uwCropVstart = 0;		
			sLcdInfor.tChRes[2].uwCropHsize = uwKNL_GetVdoH(ubDisp3Src);
			sLcdInfor.tChRes[2].uwCropVsize = uwKNL_GetVdoV(ubDisp3Src);
			
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{	
			if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
			{
				printd(DBG_Debug3Lvl, "Need to Add !!!!\r\n");
			}			
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}	
	
	if(tKNL_GetDispType() == KNL_DISP_DUAL_C)
	{		
		sLcdInfor.tDispType = LCD_DISP_2T_V;
		sLcdInfor.ubChNum = 2;
//		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
//		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();		
		
		//Get Correspond Source
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)
		{	
			// For Kernel	
			// -----------------
			// |   	 Disp1     |
			// | 	  F(2)     |
			// |---------------|
			// |      B(3)     |
			// |   	 Disp2     |
			// -----------------
			
			// For LCD IP	
			// -----------------
			// |	   CH0     |
			// | 	           |
			// |---------------|
			// |               |
			// |	   CH1     |
			// -----------------			
			
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispH;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispV;

			//Source[0]
			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoV(ubDisp1Src);			
			//sLcdInfor.tChRes[0].uwCropHstart = 0;
			sLcdInfor.tChRes[0].uwCropHstart = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?210:0;
			sLcdInfor.tChRes[0].uwCropVstart = 0;		
			//sLcdInfor.tChRes[0].uwCropHsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHsize = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?860:uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize = uwKNL_GetVdoV(ubDisp1Src);			

			//Source[1]
			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoV(ubDisp2Src);			
			//sLcdInfor.tChRes[1].uwCropHstart = 0;
			sLcdInfor.tChRes[1].uwCropHstart = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?210:0;
			sLcdInfor.tChRes[1].uwCropVstart = 0;		
			//sLcdInfor.tChRes[1].uwCropHsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHsize = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?860:uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize = uwKNL_GetVdoV(ubDisp2Src);			
		}
		else if(tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)
		{
			sLcdInfor.uwLcdOutputHsize = tKNL_Info.uwDispV;
			sLcdInfor.uwLcdOutputVsize = tKNL_Info.uwDispH;

			//Source[0]
			ubDisp1Src = (KNL_SRC_NONE == ubDisp1Src)?KNL_SRC_1_MAIN:ubDisp1Src;
			sLcdInfor.tChRes[0].uwChInputHsize = uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwChInputVsize = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropHstart = 0;
			//sLcdInfor.tChRes[0].uwCropVstart = 0;
			sLcdInfor.tChRes[0].uwCropVstart = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?210:0;
			sLcdInfor.tChRes[0].uwCropHsize  = uwKNL_GetVdoV(ubDisp1Src);
			//sLcdInfor.tChRes[0].uwCropVsize  = uwKNL_GetVdoH(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize  = (uwKNL_GetVdoH(ubDisp1Src) == 1280)?860:uwKNL_GetVdoH(ubDisp1Src);

			//Source[1]
			ubDisp2Src = (KNL_SRC_NONE == ubDisp2Src)?KNL_SRC_2_MAIN:ubDisp2Src;
			sLcdInfor.tChRes[1].uwChInputHsize = uwKNL_GetVdoV(ubDisp2Src);
			sLcdInfor.tChRes[1].uwChInputVsize = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropHstart = 0;
			//sLcdInfor.tChRes[1].uwCropVstart = 0;
			sLcdInfor.tChRes[1].uwCropVstart = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?210:0;
			sLcdInfor.tChRes[1].uwCropHsize  = uwKNL_GetVdoV(ubDisp2Src);
			//sLcdInfor.tChRes[1].uwCropVsize  = uwKNL_GetVdoH(ubDisp2Src);
			sLcdInfor.tChRes[1].uwCropVsize  = (uwKNL_GetVdoH(ubDisp2Src) == 1280)?860:uwKNL_GetVdoH(ubDisp2Src);
		}
		pLcdCropScaleParam = (LCD_INFOR_TYP *)(&sLcdInfor);
	}	

	if(NULL == pLcdCropScaleParam)
		return 0;

	ubKNL_LcdDispParamActiveFlg = 1;
	return 1;
//	if (LCD_OK != tLCD_CropScale(&sLcdInfor))
//	{		
//		return 0;
//	}
//	else
//	{
//		return 1;
//	}
#else
	return 0;
#endif
}
//------------------------------------------------------------------------------
uint8_t KNL_ChkLcdDispLocation(uint8_t ubSrcNum)
{
#if KNL_LCD_FUNC_ENABLE
	KNL_DISP_LOCATION tDispLocate;

	tDispLocate = tKNL_GetDispLocation(ubSrcNum);
	return (tDispLocate > KNL_DISP_LOCATION4)?0:1;
#else
	return 0;
#endif
}
//------------------------------------------------------------------------------
void KNL_LcdDisplaySetting(void)
{
#if KNL_LCD_FUNC_ENABLE
	LCD_RESULT tKNL_LcdJpegDisRet;

	tKNL_LcdJpegDisRet = tLCD_JpegDecodeDisable();
	if((NULL == pLcdCropScaleParam) || ((!ubKNL_LcdDispParamActiveFlg) && (LCD_JPEG_DECDIS_FAIL == tKNL_LcdJpegDisRet)))
		return;

	LCD_SetLcdBufAddr(ulBUF_GetBlkBufAddr(0, BUF_LCD_IP));
	if (LCD_OK != tLCD_CropScale(pLcdCropScaleParam))
	{		
		printd(DBG_ErrorLvl, "Set LCD Crop & Scale Fail !!!\r\n");
		return;
	}
	ubKNL_LcdDispParamActiveFlg = 0;
#endif
}
//------------------------------------------------------------------------------
void KNL_ResetLcdChannel(void)
{
#if KNL_LCD_FUNC_ENABLE
	LCD_ChDisable(LCD_CH0);
	LCD_ChDisable(LCD_CH1);
	LCD_ChDisable(LCD_CH2);
	LCD_ChDisable(LCD_CH3);
#endif
}
//------------------------------------------------------------------------------
void KNL_ResetDispSrc(uint8_t ubSrcNum)
{
	if(tKNL_Info.ubDisp1SrcNum == ubSrcNum)
	{
		tKNL_Info.ubDisp1SrcNum = KNL_SRC_NONE;
	}
	else if(tKNL_Info.ubDisp2SrcNum == ubSrcNum)
	{
		tKNL_Info.ubDisp2SrcNum = KNL_SRC_NONE;
	}
	else if(tKNL_Info.ubDisp3SrcNum == ubSrcNum)
	{
		tKNL_Info.ubDisp3SrcNum = KNL_SRC_NONE;
	}
	else if(tKNL_Info.ubDisp4SrcNum == ubSrcNum)
	{
		tKNL_Info.ubDisp4SrcNum = KNL_SRC_NONE;
	}
}
//------------------------------------------------------------------------------
void KNL_SetDispSrc(KNL_DISP_LOCATION tDispLocation,uint8_t ubDispSrcNum)
{
	KNL_ResetDispSrc(ubDispSrcNum);
	if(tDispLocation == KNL_DISP_LOCATION1)
	{
		tKNL_Info.ubDisp1SrcNum = ubDispSrcNum;
	}
	if(tDispLocation == KNL_DISP_LOCATION2)
	{
		tKNL_Info.ubDisp2SrcNum = ubDispSrcNum;
	}
	if(tDispLocation == KNL_DISP_LOCATION3)
	{
		tKNL_Info.ubDisp3SrcNum = ubDispSrcNum;
	}
	if(tDispLocation == KNL_DISP_LOCATION4)
	{
		tKNL_Info.ubDisp4SrcNum = ubDispSrcNum;
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetDispSrc(KNL_DISP_LOCATION tDispLocation)
{
	if(tDispLocation == KNL_DISP_LOCATION1)
	{
		return tKNL_Info.ubDisp1SrcNum;
	}
	else if(tDispLocation == KNL_DISP_LOCATION2)
	{
		return tKNL_Info.ubDisp2SrcNum;
	}
	else if(tDispLocation == KNL_DISP_LOCATION3)
	{
		return tKNL_Info.ubDisp3SrcNum;
	}
	else if(tDispLocation == KNL_DISP_LOCATION4)
	{
		return tKNL_Info.ubDisp4SrcNum;
	}
	else
	{
		return KNL_SRC_NONE;
	}
}
//------------------------------------------------------------------------------
KNL_DISP_LOCATION tKNL_GetDispLocation(uint8_t ubSrcNum)
{
	if(tKNL_Info.ubDisp1SrcNum == ubSrcNum)
	{
		return KNL_DISP_LOCATION1;
	}
	else if(tKNL_Info.ubDisp2SrcNum == ubSrcNum)
	{
		return KNL_DISP_LOCATION2;
	}
	else if(tKNL_Info.ubDisp3SrcNum == ubSrcNum)
	{
		return KNL_DISP_LOCATION3;
	}
	else if(tKNL_Info.ubDisp4SrcNum == ubSrcNum)
	{
		return KNL_DISP_LOCATION4;
	}
	else
	{
		printd(DBG_ErrorLvl, "Err DispLocation !!!!\r\n");
		return KNL_DISP_LOCATION_ERR;
	}
}
//------------------------------------------------------------------------------
KNL_FRAME_TYPE tKNL_GetFrameType(uint32_t ulAddr)
{
	if((*((uint8_t *)(ulAddr+4))) == 0x67)
	{
		return KNL_I_FRAME;
	}
	else
	{
		return KNL_P_FRAME;
	}	
}
//------------------------------------------------------------------------------
void KNL_DacStopCase(void)
{
	ulKNL_DacStartToPlayCnt = 0;
	ubKNL_DacStartToPlayFlg = 0;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkImgRdy(void)
{
	return ubKNL_ImgRdy;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkVdoFlowAct(uint8_t ubSrcNum)
{	
	return ubKNL_VdoFlowActiveFlg[ubSrcNum];	
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkAdoFlowAct(uint8_t ubSrcNum)
{
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_ADC))
	{
		return ubKNL_AdcFlowActiveFlg;
	}
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_DAC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_DAC_BUF)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
	{
		return ubKNL_DacFlowActiveFlg[ubSrcNum];
	}
	printd(DBG_ErrorLvl, "Err[%d] @ubKNL_ChkAdoFlowAct\r\n", ubSrcNum);
	return 0;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkDacFlowAct(uint8_t ubSrcNum)
{	
	return ubKNL_DacFlowActiveFlg[ubSrcNum];	
}

H264_ENCODE_INDEX tKNL_GetEncIdx(void)
{
	return tKNL_Info.tEncIdx;
}
//------------------------------------------------------------------------------
void KNL_ImgEncInit(H264_ENCODE_INDEX CodecIdx,uint16_t uwVdoH,uint16_t uwVdoV)
{
	H264_MROI_SETUP MROISetup;	
	
	if(CodecIdx == ENCODE_0)
	{
		ulKNL_CurFrmIdx[0] = 0;
	}
	else if(CodecIdx == ENCODE_1)
	{
		ulKNL_CurFrmIdx[1] = 0;
	}
	else if(CodecIdx == ENCODE_2)
	{
		ulKNL_CurFrmIdx[2] = 0;
	}
	else if(CodecIdx == ENCODE_3)
	{
		ulKNL_CurFrmIdx[3] = 0;
	}
	else
	{
		printd(DBG_ErrorLvl, "Encode Index Err !\n");
		return;
	}
	
	if(CodecIdx == ENCODE_0)
	{
		H264_EncodeInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(0,BUF_IMG_ENC),ubKNL_GetVdoFps(),ulKNL_GetVdoGop());//DECODE[0]
		tKNL_Info.tEncIdx = ENCODE_0;
	}
	if(CodecIdx == ENCODE_1)
	{
		H264_EncodeInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(1,BUF_IMG_ENC),ubKNL_GetVdoFps(),ulKNL_GetVdoGop());//DECODE[1]
		tKNL_Info.tEncIdx = ENCODE_1;
	}
	if(CodecIdx == ENCODE_2)
	{
		H264_EncodeInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(2,BUF_IMG_ENC),ubKNL_GetVdoFps(),ulKNL_GetVdoGop());//DECODE[2]
		tKNL_Info.tEncIdx = ENCODE_2;
	}
	if(CodecIdx == ENCODE_3)
	{
		H264_EncodeInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(3,BUF_IMG_ENC),ubKNL_GetVdoFps(),ulKNL_GetVdoGop());//DECODE[3]
		tKNL_Info.tEncIdx = ENCODE_3;
	}	
	
	H264_SetGOP(CodecIdx,ulKNL_GetVdoGop());
	
	H264_SetAROIEN(CodecIdx,H264_ENABLE,MROI,MV_OR_SKIN);	
	
	H264_SetCondensedMode(CodecIdx,H264_ENABLE,7);
	
	MROISetup.Num = MROI_0;	
	MROISetup.ENABLE = H264_DISABLE;	
	MROISetup.ubWeight =1;
	MROISetup.uwQP_Value =-1;
	MROISetup.ulPosX = 3;
	MROISetup.ulWidth = 3;
	MROISetup.ulPosY = 1;
	MROISetup.ulHeight = 4;
	MROISetup.ExtSize =SIZE_0;	
	H264_SetMROI(CodecIdx,MROISetup);
	
	MROISetup.Num = MROI_1;	
	MROISetup.ENABLE = H264_DISABLE;	
	MROISetup.ubWeight =7;
	MROISetup.uwQP_Value =-2;
	MROISetup.ulPosX = 1;
	MROISetup.ulWidth = 4;
	MROISetup.ulPosY = 3;
	MROISetup.ulHeight = 3;
	MROISetup.ExtSize =SIZE_0;	
	H264_SetMROI(CodecIdx,MROISetup);

	MROISetup.Num = MROI_2;
	MROISetup.ENABLE = H264_DISABLE;	
	MROISetup.ubWeight =5;
	MROISetup.uwQP_Value =9;
	MROISetup.ulPosX = 2;
	MROISetup.ulWidth = 3;
	MROISetup.ulPosY = 4;
	MROISetup.ulHeight = 2;
	MROISetup.ExtSize =SIZE_0;	
	H264_SetMROI(CodecIdx,MROISetup);

	MROISetup.Num = MROI_7;	
	MROISetup.ENABLE = H264_DISABLE;	
	MROISetup.ubWeight =1;
	MROISetup.uwQP_Value =6;
	MROISetup.ulPosX = 2;
	MROISetup.ulWidth = 3;
	MROISetup.ulPosY = 1;
	MROISetup.ulHeight = 1;
	MROISetup.ExtSize =SIZE_0;	
	H264_SetMROI(CodecIdx,MROISetup);
		
	H264_SetMROIModeEnable(CodecIdx,H264_DISABLE);	
	H264_SetSkinMode(CodecIdx,H264_DISABLE);		

	if (KNL_GetTuningToolMode() == KNL_TUNINGMODE_ON) { 
	    H264_SetQp(CodecIdx,ubRC_GetTargetQp(CodecIdx),ubRC_GetTargetQp(CodecIdx));
	    H264_RcSetEN(CodecIdx,H264_ENABLE,CBR,0x244000L);      		        
	} else {
		if(ubRC_GetFlg(CodecIdx))
		{		
			H264_SetMaxQP(CodecIdx,ubRC_GetMaxQp(CodecIdx));
			H264_SetMinQP(CodecIdx,ubRC_GetMinQp(CodecIdx));				
			H264_RcSetEN(CodecIdx,H264_ENABLE,CBR,ulRC_GetInitBitRate(CodecIdx));		
		}
		else
		{
			H264_SetQp(CodecIdx,ubRC_GetTargetQp(CodecIdx),ubRC_GetTargetQp(CodecIdx));
			H264_RcSetEN(CodecIdx,H264_DISABLE,CBR,0x0A0000L);
		}
	}
}
//------------------------------------------------------------------------------
void KNL_ImgDecInit(H264_DECODE_INDEX CodecIdx,uint16_t uwVdoH,uint16_t uwVdoV)
{
	if(CodecIdx == DECODE_0)
	{
		H264_DecoderInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(0,BUF_IMG_DEC));	
	}
	if(CodecIdx == DECODE_1)
	{
		H264_DecoderInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(1,BUF_IMG_DEC));	
	}
	if(CodecIdx == DECODE_2)
	{
		H264_DecoderInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(2,BUF_IMG_DEC));	
	}
	if(CodecIdx == DECODE_3)
	{
		H264_DecoderInit(CodecIdx,uwVdoH,uwVdoV,ulBUF_GetBlkBufAddr(3,BUF_IMG_DEC));	
	}
	if(CodecIdx > DECODE_3)
	{
		printd(DBG_ErrorLvl, "Decode Index Err\n");
	}
}
//------------------------------------------------------------------------------
void KNL_ImageDecodeSetup(uint8_t ubSrcNum)
{
	KNL_NODE_INFO tNodeInfo;

	H264_Reset();
	tNodeInfo = tKNL_GetNodeInfo(ubSrcNum, KNL_NODE_H264_DEC);
	KNL_ImgDecInit((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx, uwKNL_GetVdoH(ubSrcNum), uwKNL_GetVdoV(ubSrcNum));
	ubKNL_RcvFirstIFrame[ubSrcNum] = 0;
}
//------------------------------------------------------------------------------
void KNL_ImageEncodeSetup(uint8_t ubSrcNum)
{
	KNL_NODE_INFO tNodeInfo;

	if(ubKNL_ExistNode(ubSrcNum, KNL_NODE_H264_ENC))
	{
		tNodeInfo = tKNL_GetNodeInfo(ubSrcNum, KNL_NODE_H264_ENC);
		KNL_ImgEncInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx, tNodeInfo.uwVdoH, tNodeInfo.uwVdoV);
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ImgEnc(H264_ENCODE_INDEX CodecIdx,uint32_t ulYuvAddr,uint32_t ulBsAddr)
{
	IMG_IMAGE_TASK ImageTask;
	struct H264_TASK H264Task;

	//Pre-Process
	//ubKNL_ImgRdy = 0;
	osSemaphoreWait(tKNL_ImgSem, osWaitForever);

	//printd(DBG_Debug1Lvl, "H->ET_Y:0x%x_B:0x%x\r\n",ulYuvAddr,ulBsAddr);
	//printd(DBG_Debug1Lvl, "H->ET\n");	

	H264Task.DesAddr 		= ulBsAddr;
	H264Task.EncodeStream 	= CodecIdx;
	H264Task.Type			= H264_ENCODE;	
	ImageTask.InputSrcAddr 	= ulYuvAddr;
	ImageTask.H264_Task 	= &H264Task;
	ImageTask.JPEGEnable	= IMG_DISABLE;
	ImageTask.ScalingEnable = IMG_DISABLE;	
	
	IMG_StartUp(ImageTask);	
	
	return 1;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ImgDec(H264_DECODE_INDEX CodecIdx,uint32_t ulYuvAddr,uint32_t ulBsAddr)
{	
	IMG_IMAGE_TASK ImageTask;	
	struct H264_TASK H264Task;
	IMG_MERGE_SETUP MergeSetup;	
	IMG_BOOL_RESULT RESULT;

	//Pre-Process
	//ubKNL_ImgRdy = 0;
	osSemaphoreWait(tKNL_ImgSem, osWaitForever);
	//printd(DBG_Debug3Lvl, "H->DT\r\n");

	H264Task.Type 			= H264_DECODE;
	H264Task.DecodeStream 	= CodecIdx;	
	H264Task.DesAddr 		= ulYuvAddr;
	ImageTask.InputSrcAddr 	= ulBsAddr;
	ImageTask.H264_Task 	= &H264Task;
	ImageTask.JPEGEnable	= IMG_DISABLE;
	ImageTask.ScalingEnable = IMG_ENABLE;

	MergeSetup.STATUS = IMG_DISABLE;	
	RESULT = IMG_MergeSetup(&MergeSetup);
	if(RESULT == IMG_FAIL)
	{
		printd(DBG_ErrorLvl, "Merge Setup Fail2 \n");
		return 0;
	}

	IMG_StartUp(ImageTask);

	return 1;
}
//------------------------------------------------------------------------------
void KNL_VdoPathReset(void)
{
	uint8_t i,j;

	for(j=0;j<KNL_SRC_NUM;j++)
	{
		for(i=0;i<KNL_MAX_NODE_NUM;i++)
		{				
			tKNL_VdoNodeInfo[j][i].ubPreNode  = KNL_NODE_NONE;
			tKNL_VdoNodeInfo[j][i].ubCurNode  = KNL_NODE_NONE;
			tKNL_VdoNodeInfo[j][i].ubNextNode = KNL_NODE_NONE;
		}
	}
}
//------------------------------------------------------------------------------
void KNL_AdoPathReset(void)
{
	uint8_t i;
	uint8_t j;
	
	for(j=0;j<KNL_SRC_NUM;j++)
	{
		for(i=0;i<KNL_MAX_NODE_NUM;i++)
		{				
			tKNL_AdoNodeInfo[j][i].ubPreNode 	= KNL_NODE_NONE;
			tKNL_AdoNodeInfo[j][i].ubCurNode 	= KNL_NODE_NONE;
			tKNL_AdoNodeInfo[j][i].ubNextNode	= KNL_NODE_NONE;
		}
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_SetVdoPathNode(uint8_t ubSrcNum,uint8_t ubNodeIdx,KNL_NODE_INFO tNodeInfo)
{	
	if((ubNodeIdx > KNL_MAX_NODE_NUM)||(ubSrcNum > KNL_SRC_NUM))
	{
		return 0;
	}
	
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubPreNode 	= tNodeInfo.ubPreNode;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubCurNode		= tNodeInfo.ubCurNode;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubNextNode	= tNodeInfo.ubNextNode;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].uwVdoH		= tNodeInfo.uwVdoH;	
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].uwVdoV		= tNodeInfo.uwVdoV;	
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubHMirror		= tNodeInfo.ubHMirror;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubVMirror		= tNodeInfo.ubVMirror;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubRotate		= tNodeInfo.ubRotate;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubHScale		= tNodeInfo.ubHScale;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubVScale		= tNodeInfo.ubVScale;	
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubMergeSrc1 	= tNodeInfo.ubMergeSrc1;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubMergeSrc2 	= tNodeInfo.ubMergeSrc2;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubMergeDest 	= tNodeInfo.ubMergeDest;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].uwMergeH		= tNodeInfo.uwMergeH;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].uwMergeV		= tNodeInfo.uwMergeV;
	tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubCodecIdx  	= tNodeInfo.ubCodecIdx;	
	
	return 1;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_SetAdoPathNode(uint8_t ubSrcNum,uint8_t ubNodeIdx,KNL_NODE_INFO tNodeInfo)
{	
	if(ubNodeIdx > KNL_MAX_NODE_NUM)
	{
		return 0;
	}
	
	tKNL_AdoNodeInfo[ubSrcNum][ubNodeIdx].ubPreNode 	= tNodeInfo.ubPreNode;
	tKNL_AdoNodeInfo[ubSrcNum][ubNodeIdx].ubCurNode		= tNodeInfo.ubCurNode;
	tKNL_AdoNodeInfo[ubSrcNum][ubNodeIdx].ubNextNode	= tNodeInfo.ubNextNode;	
	
	return 1;
}
//------------------------------------------------------------------------------
void KNL_VdoPathNodeReset(uint8_t ubSrcNum)
{
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_LCD))
	{
		KNL_ResetLcdChannel();
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetNextNode(uint8_t ubSrcNum,uint8_t ubNode)
{	
	uint8_t ubNodeIdx;	
	uint8_t ubIsAdoPath;	//0->Video Path, 1->Audio Path
	
	ubIsAdoPath = 0;	
	if((ubNode==KNL_NODE_ADC)||(ubNode==KNL_NODE_DAC)||(ubNode==KNL_NODE_ADC_BUF)||(ubNode==KNL_NODE_DAC_BUF)||(ubNode==KNL_NODE_COMM_TX_ADO)||(ubNode==KNL_NODE_COMM_RX_ADO))
	{
		ubIsAdoPath = 1;
	}	
	ubNodeIdx = ubKNL_GetNodeIdx(ubSrcNum,ubNode);		
	
	if(ubIsAdoPath == 0)
	{
		return tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubNextNode;
	}	
	else	
	{
		return tKNL_AdoNodeInfo[ubSrcNum][ubNodeIdx].ubNextNode;
	}		
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetPreNode(uint8_t ubSrcNum,uint8_t ubNode)
{
	uint8_t ubNodeIdx;	
	uint8_t ubIsAdoPath;	//0->Video Path, 1->Audio Path
	
	ubIsAdoPath = 0;	
	if((ubNode==KNL_NODE_ADC)||(ubNode==KNL_NODE_DAC)||(ubNode==KNL_NODE_ADC_BUF)||(ubNode==KNL_NODE_DAC_BUF)||(ubNode==KNL_NODE_COMM_TX_ADO)||(ubNode==KNL_NODE_COMM_RX_ADO))
	{
		ubIsAdoPath = 1;
	}		
	
	ubNodeIdx = ubKNL_GetNodeIdx(ubSrcNum,ubNode);	
	if(ubIsAdoPath == 0)
	{
		return tKNL_VdoNodeInfo[ubSrcNum][ubNodeIdx].ubPreNode;	
	}
	else
	{
		return tKNL_AdoNodeInfo[ubSrcNum][ubNodeIdx].ubPreNode;	
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetNodeIdx(uint8_t ubSrcNum,uint8_t ubNode)
{
	uint8_t i;
	uint8_t ubIsAdoPath;	//0->Video Path, 1->Audio Path
	
	ubIsAdoPath = 0;	
	if((ubNode==KNL_NODE_ADC)||(ubNode==KNL_NODE_DAC)||(ubNode==KNL_NODE_ADC_BUF)||(ubNode==KNL_NODE_DAC_BUF)||(ubNode==KNL_NODE_COMM_TX_ADO)||(ubNode==KNL_NODE_COMM_RX_ADO))
	{
		ubIsAdoPath = 1;
	}
	
	for(i=0;i<KNL_MAX_NODE_NUM;i++)
	{
		if(ubIsAdoPath == 0)
		{
			if(tKNL_VdoNodeInfo[ubSrcNum][i].ubCurNode == ubNode)
			{
				return i;
			}
		}
		if(ubIsAdoPath == 1)
		{
			if(tKNL_AdoNodeInfo[ubSrcNum][i].ubCurNode == ubNode)
			{
				return i;
			}
		}
	}
	return 0xFF;
}
//------------------------------------------------------------------------------
KNL_NODE_INFO tKNL_GetNodeInfo(uint8_t ubSrcNum,uint8_t ubNode)
{
	uint8_t i;
	KNL_NODE_INFO tNodeInfo;
	
	//Video node Only
	tNodeInfo.ubPreNode		= KNL_NODE_NONE;
	tNodeInfo.ubCurNode 	= KNL_NODE_NONE;
	tNodeInfo.ubNextNode	= KNL_NODE_NONE;
	tNodeInfo.uwVdoH		= 0;
	tNodeInfo.uwVdoV		= 0;
	tNodeInfo.ubHMirror		= 0;
	tNodeInfo.ubVMirror		= 0;
	tNodeInfo.ubRotate		= 0;
	tNodeInfo.ubHScale		= KNL_SCALE_X1;
	tNodeInfo.ubVScale		= KNL_SCALE_X1;	
	
	for(i=0;i<KNL_MAX_NODE_NUM;i++)
	{
		if(tKNL_VdoNodeInfo[ubSrcNum][i].ubCurNode == ubNode)
		{
			return tKNL_VdoNodeInfo[ubSrcNum][i];
		}
	}
	printd(DBG_ErrorLvl, "Err @tKNL_GetNodeInfo\r\n");
	return tNodeInfo;
}
//------------------------------------------------------------------------------
void KNL_ShowVdoPathNode(uint8_t ubSrcNum)
{
	uint8_t i;
	uint8_t ubCurNode;	
	
	printd(DBG_Debug3Lvl, "**********************************************************************\r\n");
	printd(DBG_Debug3Lvl, "****************     Video Path Information(%d)     ******************\r\n",ubSrcNum);
	printd(DBG_Debug3Lvl, "**********************************************************************\r\n");	
	
	for(i=0;i<KNL_MAX_NODE_NUM;i++)
	{		
		ubCurNode = tKNL_VdoNodeInfo[ubSrcNum][i].ubCurNode;	
		
		//Current
		if(ubCurNode == KNL_NODE_SEN)
		{
			printd(DBG_Debug3Lvl, "{SEN}->");
		}
		else if(ubCurNode == KNL_NODE_SEN_YUV_BUF)
		{
			printd(DBG_Debug3Lvl, "{SEN_YUV_BUF}->");
		}
		else if(ubCurNode == KNL_NODE_H264_ENC)
		{
			printd(DBG_Debug3Lvl, "{H264_ENC}->");
		}
		else if(ubCurNode == KNL_NODE_VDO_BS_BUF1)
		{
			printd(DBG_Debug3Lvl, "{VDO_BS_BUF1}->");
		}
		else if(ubCurNode == KNL_NODE_VDO_BS_BUF2)
		{
			printd(DBG_Debug3Lvl, "{VDO_BS_BUF2}->");
		}		
		else if(ubCurNode == KNL_NODE_COMM_TX_VDO)
		{
			printd(DBG_Debug3Lvl, "{COMM_TX_VDO}->");
		}
		else if(ubCurNode == KNL_NODE_COMM_RX_VDO)
		{
			printd(DBG_Debug3Lvl, "{COMM_RX_VDO}->");
		}
		else if(ubCurNode == KNL_NODE_H264_DEC)
		{
			printd(DBG_Debug3Lvl, "{H264_DEC}->");
		}
		else if(ubCurNode == KNL_NODE_IMG_MERGE_BUF)
		{
			printd(DBG_Debug3Lvl, "{IMG_MERGE_BUF}->");
		}
		else if(ubCurNode == KNL_NODE_IMG_MERGE_H)
		{
			printd(DBG_Debug3Lvl, "{IMG_MERGE_H}->");
		}		
		else if(ubCurNode == KNL_NODE_LCD)
		{
			printd(DBG_Debug3Lvl, "{LCD}->");
		}
		else if(ubCurNode == KNL_NODE_VDO_REC)
		{
			printd(DBG_Debug3Lvl, "{VDO_REC}->");
		}
		else if(ubCurNode == KNL_NODE_JPG_ENC)
		{
			printd(DBG_Debug3Lvl, "{JPG_ENC}->");
		}				
		else if(ubCurNode == KNL_NODE_JPG_DEC1)
		{
			printd(DBG_Debug3Lvl, "{JPG_DEC1}->");
		}
		else if(ubCurNode == KNL_NODE_JPG_DEC2)
		{
			printd(DBG_Debug3Lvl, "{JPG_DEC2}->");
		}
		else if(ubCurNode == KNL_NODE_UVC_MAIN)
		{
			printd(DBG_Debug3Lvl, "{UVC_MAIN}->");
		}
		else if(ubCurNode == KNL_NODE_UVC_SUB)
		{
			printd(DBG_Debug3Lvl, "{UVC_SUB}->");
		}
		else if(ubCurNode == KNL_NODE_END)
		{
			printd(DBG_Debug3Lvl, "{END}\r\n");
			printd(DBG_Debug3Lvl, "\r\n");
		}		
	}	
}
//------------------------------------------------------------------------------
void KNL_ShowAdoPathNode(uint8_t ubSrcNum)
{
	uint8_t i;	
	uint8_t ubCurNode;	
	
	printd(DBG_Debug3Lvl, "r\n");
	printd(DBG_Debug3Lvl, "****     Audio Path Information(%d)    ******\r\n",ubSrcNum);
	printd(DBG_Debug3Lvl, "******************************************\r\n");	
	
	for(i=0;i<KNL_MAX_NODE_NUM;i++)
	{	
		ubCurNode = tKNL_AdoNodeInfo[ubSrcNum][i].ubCurNode;			
		
		//Current
		if(ubCurNode == KNL_NODE_ADC)
		{
			printd(DBG_Debug3Lvl, "{ADC}->");
		}		
		else if(ubCurNode == KNL_NODE_ADC_BUF)
		{
			printd(DBG_Debug3Lvl, "{ADC_BUF}->");
		}
		else if(ubCurNode == KNL_NODE_DAC_BUF)
		{
			printd(DBG_Debug3Lvl, "{DAC_BUF}->");
		}
		else if(ubCurNode == KNL_NODE_DAC)
		{
			printd(DBG_Debug3Lvl, "{DAC}->");
		}
		else if(ubCurNode == KNL_NODE_COMM_TX_ADO)
		{
			printd(DBG_Debug3Lvl, "{COMM_TX_ADO}->");
		}
		else if(ubCurNode == KNL_NODE_COMM_RX_ADO)
		{
			printd(DBG_Debug3Lvl, "{COMM_RX_ADO}->");			
		}
		else if(ubCurNode == KNL_NODE_END)
		{
			printd(DBG_Debug3Lvl, "{END}\r\n");
			printd(DBG_Debug3Lvl, "\r\n");
		}		
	}	
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ExistNode(uint8_t ubSrcNum,uint8_t ubNode)
{
	uint8_t i;
	
	//For Video Node
	for(i=0;i<KNL_MAX_NODE_NUM;i++)
	{
		if(tKNL_VdoNodeInfo[ubSrcNum][i].ubCurNode == ubNode)
		{
			return 1;
		}
	}	
	
	//For Audio Node
	for(i=0;i<KNL_MAX_NODE_NUM;i++)
	{
		if(tKNL_AdoNodeInfo[ubSrcNum][i].ubCurNode == ubNode)
		{
			return 1;
		}
	}
	
	return 0;	
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkExistNode(uint8_t ubNode)
{
	uint8_t i,j;
	
	//For Video Node
	for(j=0;j<KNL_SRC_NUM;j++)
	{
		for(i=0;i<KNL_MAX_NODE_NUM;i++)
		{
			if(tKNL_VdoNodeInfo[j][i].ubCurNode == ubNode)
			{
				return 1;
			}
		}
	}	
	
	//For Audio Node
	for(j=0;j<KNL_SRC_NUM;j++)
	{
		for(i=0;i<KNL_MAX_NODE_NUM;i++)
		{
			if(tKNL_AdoNodeInfo[j][i].ubCurNode == ubNode)
			{
				return 1;
			}
		}
	}
	
	return 0;	
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetLcdDispAddr(uint8_t ubSrcNum)
{
	uint32_t ulAddr = 0;
#if KNL_LCD_FUNC_ENABLE
	KNL_DISP_LOCATION tDispLocate;

	if((tKNL_GetDispType() == KNL_DISP_H) && (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0))
	{
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION2)
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			pLcdCh2Buf = pLCD_GetLcdChBufInfor(LCD_CH2);
			ulAddr = pLcdCh2Buf->ulBufAddr;	
		}
		else if((tDispLocate == KNL_DISP_LOCATION1)||(tDispLocate == KNL_DISP_LOCATION4))
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_SINGLE)
	{
		pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
		ulAddr = pLcdCh0Buf->ulBufAddr;
	}
	else if((tKNL_GetDispType() == KNL_DISP_DUAL_U)||(tKNL_GetDispType() == KNL_DISP_QUAD))
	{
		if(TRUE == KNL_SwDispInfo.ubSetupFlag)
			tDispLocate = tKNL_SearchSrcLocation(ubSrcNum);
		else
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			pLcdCh2Buf = pLCD_GetLcdChBufInfor(LCD_CH2);
			ulAddr = pLcdCh2Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION4)
		{
			pLcdCh3Buf = pLCD_GetLcdChBufInfor(LCD_CH3);
			ulAddr = pLcdCh3Buf->ulBufAddr;			
		}		
	}
	else if(tKNL_GetDispType() == KNL_DISP_3T_1T2B)
	{
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}
		else if((tDispLocate == KNL_DISP_LOCATION2)||(tDispLocate == KNL_DISP_LOCATION3))
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}
		else
		{
			printd(DBG_ErrorLvl, "Err @ulKNL_GetLcdDispAddr\r\n");
		}
	}	
	else if(tKNL_GetDispType() == KNL_DISP_3T_2T1B)
	{
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if((tDispLocate == KNL_DISP_LOCATION1) || (tDispLocate == KNL_DISP_LOCATION2))
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}			
		else
		{
			printd(DBG_ErrorLvl, "Err @ulKNL_GetLcdDispAddr\r\n");
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_3T_2L1R)
	{	
		// For Kernel
		// -----------------
		// | Disp1 |       |
		// |  F(2) |       |
		// |-------| Disp3 |
		// | Disp2 |  R(4) |
		// |  B(3) |	   |
		// -----------------	
		
		// For LCD IP
		// -----------------
		// |  CH0  |       |
		// | 	   |       |
		// |-------|  CH1  |
		// |       |       |
		// |  CH2  |	   |
		// -----------------
	
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			pLcdCh2Buf = pLCD_GetLcdChBufInfor(LCD_CH2);
			ulAddr = pLcdCh2Buf->ulBufAddr;			
		}		
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}				
		else
		{
			printd(DBG_ErrorLvl, "Err @ulKNL_GetLcdDispAddr\r\n");
		}	
	}	
	else if(tKNL_GetDispType() == KNL_DISP_3T_1L2R)
	{	
		// For Kernel	
		// -----------------
		// |   	   | Disp2 |
		// | 	   |  F(2) |
		// |  L(1) |-------|
		// | Disp1 |  B(3) |
		// |   	   | Disp3 |
		// -----------------
		
		// For LCD IP	
		// -----------------
		// |   	   |  CH0  |
		// | 	   |       |
		// |  CH1  |-------|
		// |       |       |
		// |       |  CH2  |
		// -----------------
	
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}		
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			pLcdCh2Buf = pLCD_GetLcdChBufInfor(LCD_CH2);
			ulAddr = pLcdCh2Buf->ulBufAddr;			
		}				
		else
		{
			printd(DBG_ErrorLvl, "Err @ulKNL_GetLcdDispAddr\r\n");
		}	
	}	
	else if(tKNL_GetDispType() == KNL_DISP_DUAL_C)
	{	
		// For Kernel	
		// -----------------
		// |   	 Disp1     |
		// | 	  F(2)     |
		// |---------------|
		// |      B(3)     |
		// |   	 Disp2     |
		// -----------------
		
		// For LCD IP	
		// -----------------
		// |	  CH0      |
		// | 	           |
		// |---------------|
		// |               |
		// |	  CH1      |
		// -----------------
		if(TRUE == KNL_SwDispInfo.ubSetupFlag)
			tDispLocate = tKNL_SearchSrcLocation(ubSrcNum);
		else
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
			ulAddr = pLcdCh0Buf->ulBufAddr;			
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			pLcdCh1Buf = pLCD_GetLcdChBufInfor(LCD_CH1);
			ulAddr = pLcdCh1Buf->ulBufAddr;			
		}					
		else
		{
			printd(DBG_ErrorLvl, "Err @ulKNL_GetLcdDispAddr\r\n");
		}	
	}
#endif
	return ulAddr;
}
//------------------------------------------------------------------------------
void KNL_ActiveLcdDispBuf(uint8_t ubSrcNum)
{
#if KNL_LCD_FUNC_ENABLE
	KNL_DISP_LOCATION tDispLocate;

	if((tKNL_GetDispType() == KNL_DISP_H) && (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0))
	{
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		
		if(tDispLocate == KNL_DISP_LOCATION2)
		{			
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			if(ubKNL_DispCh2ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh2Buf);
				LCD_ChEnable(LCD_CH2);
			}
		}
		else if((tDispLocate == KNL_DISP_LOCATION1)||(tDispLocate == KNL_DISP_LOCATION4))
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_SINGLE)
	{
		if(ubKNL_DispCh0ActiveFlg)
		{
			LCD_SetChBufReady(pLcdCh0Buf);
			LCD_ChEnable(LCD_CH0);
		}
	}
	else if((tKNL_GetDispType() == KNL_DISP_DUAL_U)||(tKNL_GetDispType() == KNL_DISP_QUAD))
	{
		if(TRUE == KNL_SwDispInfo.ubSetupFlag)
			tDispLocate = tKNL_SearchSrcLocation(ubSrcNum);
		else
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{			
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			if(ubKNL_DispCh2ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh2Buf);
				LCD_ChEnable(LCD_CH2);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION4)
		{
			if(ubKNL_DispCh3ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh3Buf);
				LCD_ChEnable(LCD_CH3);
			}
		}		
	}
	else if(tKNL_GetDispType() == KNL_DISP_3T_1T2B)
	{
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}
		else if((tDispLocate == KNL_DISP_LOCATION2)||(tDispLocate == KNL_DISP_LOCATION3))
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_ActiveLcdDispBuf\r\n");
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_3T_2T1B)
	{
		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if((tDispLocate == KNL_DISP_LOCATION1)||(tDispLocate == KNL_DISP_LOCATION2))
		{
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}			
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_ActiveLcdDispBuf\r\n");
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_3T_2L1R)
	{	
		// For Kernel
		// -----------------
		// | Disp1 |       |
		// |  F(2) |       |
		// |-------| Disp3 |
		// | Disp2 |  R(4) |
		// |  B(3) |	   |
		// -----------------	
		
		// For LCD IP
		// -----------------
		// |  CH0  |       |
		// | 	   |       |
		// |-------|  CH1  |
		// |       |       |
		// |  CH2  |	   |
		// -----------------

		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			if(ubKNL_DispCh2ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh2Buf);
				LCD_ChEnable(LCD_CH2);
			}
		}		
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}		
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_ActiveLcdDispBuf\r\n");
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_3T_1L2R)
	{	
		// For Kernel	
		// -----------------
		// |   	   | Disp2 |
		// | 	   |  F(2) |
		// |  L(1) |-------|
		// | Disp1 |  B(3) |
		// |   	   | Disp3 |
		// -----------------
		
		// For LCD IP	
		// -----------------
		// |   	   |  CH0  |
		// | 	   |       |
		// |  CH1  |-------|
		// |       |       |
		// |       |  CH2  |
		// -----------------

		tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}		
		else if(tDispLocate == KNL_DISP_LOCATION3)
		{
			if(ubKNL_DispCh2ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh2Buf);
				LCD_ChEnable(LCD_CH2);
			}
		}		
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_ActiveLcdDispBuf\r\n");
		}
	}
	else if(tKNL_GetDispType() == KNL_DISP_DUAL_C)
	{	
		// For Kernel	
		// -----------------
		// |   	 Disp1     |
		// | 	  F(2)     |
		// |---------------|
		// |      B(3)     |
		// |   	 Disp2     |
		// -----------------
		
		// For LCD IP	
		// -----------------
		// |	  CH0      |
		// | 	           |
		// |---------------|
		// |               |
		// |	  CH1	   |
		// -----------------

		if(TRUE == KNL_SwDispInfo.ubSetupFlag)
			tDispLocate = tKNL_SearchSrcLocation(ubSrcNum);
		else
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);
		if(tDispLocate == KNL_DISP_LOCATION1)
		{
			if(ubKNL_DispCh0ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh0Buf);
				LCD_ChEnable(LCD_CH0);
			}
		}
		else if(tDispLocate == KNL_DISP_LOCATION2)
		{
			if(ubKNL_DispCh1ActiveFlg)
			{
				LCD_SetChBufReady(pLcdCh1Buf);
				LCD_ChEnable(LCD_CH1);
			}
		}
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_ActiveLcdDispBuf\r\n");
		}
	}
#endif
}
//------------------------------------------------------------------------------
void KNL_SenYuvBufProcess(KNL_PROCESS tProc)
{
	KNL_PROCESS tKNLInfo;	
	uint32_t ulAddr;
	uint8_t ubNextNode;		
	uint32_t ulCnt;

	ulKNL_OutVdoFpsCntTemp[tProc.ubSrcNum]++;
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_SEN_YUV_BUF);

		if(ubNextNode == KNL_NODE_LCD)
		{
			DMAC_RESULT tDmaResult = DMAC_OK;
			//Get Disp Address
			ulAddr = ulKNL_GetLcdDispAddr(tProc.ubSrcNum);

			//Copy Data to Lcd Buffer
			ulCnt = ((uint32_t)((float)uwKNL_GetVdoH(tProc.ubSrcNum)*(float)uwKNL_GetVdoV(tProc.ubSrcNum)*1.5));
			tDmaResult = tDMAC_MemCopy(tProc.ulDramAddr1,ulAddr,ulCnt,NULL);

			//Release Buffer
			ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);

			//Active Lcd Buffer
			if(DMAC_OK == tDmaResult)
				KNL_ActiveLcdDispBuf(tProc.ubSrcNum);
		}
		else if((ubNextNode == KNL_NODE_NONE) || (ubNextNode == KNL_NODE_END))
		{
			//Release Buffer
			ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
			printd(DBG_ErrorLvl, "Err @KNL_SenYuvBufProcess\r\n");
		}
		else
		{
			//Send Q to Next Node
			tKNLInfo.ubSrcNum	 = tProc.ubSrcNum;
			tKNLInfo.ubCurNode 	 = KNL_NODE_SEN_YUV_BUF;
			tKNLInfo.ubNextNode	 = ubKNL_GetNextNode(tProc.ubSrcNum, KNL_NODE_SEN_YUV_BUF);
			tKNLInfo.ulSize		 = tProc.ulSize;
			tKNLInfo.ulDramAddr1 = tProc.ulDramAddr1;
			tKNLInfo.ulDramAddr2 = tProc.ulDramAddr2;
            if(osMessagePut(KNL_VdoCodecProcQueue, &tKNLInfo, 0) != osOK)
			{
				ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
				printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
			}
		}
	}
	else
	{
		ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
	}
}

//------------------------------------------------------------------------------
void KNL_H264CodecProcess(KNL_PROCESS tProc)
{
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		uint8_t ubCodecRetryNum = 0, ubCodecRetryCnt = (KNL_NODE_H264_DEC == tProc.ubNextNode)?15:30;

		for(;;)
		{
			if(ubKNL_ChkImgRdy())
			{
				if(TRUE == ubKNL_ImgBusyFlg)
					ubKNL_ImgBusyFlg = FALSE;
				break;
			}
			if(ubCodecRetryNum++ >= ubCodecRetryCnt)
			{
				printd(DBG_ErrorLvl, (KNL_NODE_H264_DEC == tProc.ubNextNode)?"H->D NRDY\r\n":"H->E NRDY\r\n");
				if(KNL_NODE_H264_DEC == tProc.ubNextNode)
					ubKNL_ImgBusyFlg = TRUE;
				goto RELEASE_CODEC_BUF;
			}
			osDelay(10);
		}
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_H264_ENC:
				KNL_H264EncProcess(tProc);
				break;
			case KNL_NODE_H264_DEC:
				KNL_H264DecProcess(tProc);
				break;
			default:
				break;
		}
	}
	else
	{
RELEASE_CODEC_BUF:
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_H264_ENC:
				ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
				break;
			case KNL_NODE_H264_DEC:
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC, tProc.ubSrcNum, tProc.ulDramAddr2);
				break;
			default:
				break;
		}
	}
}

//------------------------------------------------------------------------------
void KNL_H264EncProcess(KNL_PROCESS tProc)
{	
	uint8_t ubNextNode;
	uint32_t ulTemp = BUF_FAIL;
	KNL_NODE_INFO tNodeInfo;

	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		if(TRUE == ubKNL_ChgResFlg)
		{
			static uint8_t ubKNL_H264ResetFlg = FALSE;
			uint32_t ulResSize = uwKNL_GetVdoH(tProc.ubSrcNum) * uwKNL_GetVdoV(tProc.ubSrcNum) * 3 / 2;

			if(FALSE == ubKNL_H264ResetFlg)
			{
				uint8_t ubNodeIdx;
				uint8_t ubBufIdx[4] = {[ENCODE_0] = 0, [ENCODE_1] = 1, [ENCODE_2] = 2, [ENCODE_3] = 3};

				H264_Reset();
				tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_H264_ENC);
				tNodeInfo.uwVdoH = uwKNL_GetVdoH(tProc.ubSrcNum);
				tNodeInfo.uwVdoV = uwKNL_GetVdoV(tProc.ubSrcNum);
				ubNodeIdx = ubKNL_GetNodeIdx(tProc.ubSrcNum, KNL_NODE_H264_ENC);
				ubKNL_SetVdoPathNode(tProc.ubSrcNum, ubNodeIdx, tNodeInfo);
				H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(ubBufIdx[(H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx],BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());
				H264_SetMaxQP((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ubRC_GetMaxQp(tNodeInfo.ubCodecIdx));
				H264_SetMinQP((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ubRC_GetMinQp(tNodeInfo.ubCodecIdx));
				H264_RcSetEN((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE,CBR,ulRC_GetInitBitRate(tNodeInfo.ubCodecIdx));
				H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);
				ubKNL_H264ResetFlg = TRUE;
				ulResSize = 0;
			}
			if(ulResSize != tProc.ulSize)
			{
				ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
				return;
			}
			ubKNL_ChgResFlg 	= FALSE;
			ubKNL_H264ResetFlg  = FALSE;
		}

		//! (1)Get BS Buffer for Encode
		ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_H264_ENC);
		if(ubNextNode == KNL_NODE_VDO_BS_BUF1)
		{
			ulTemp = ulKNL_GetBsBufAddr(tProc.ubSrcNum);
			if(ulTemp == BUF_FAIL)
			{
				printd(DBG_ErrorLvl, "BUF_VDO_BS Err !!!\r\n");
			}
		}
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_H264EncProcess\r\n");
		}
		if(ulTemp == BUF_FAIL)
		{
			ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
			return;
		}

		//! (2)Get the Resource First
		ubKNL_ImgRdy = 0;
		KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_H264_ENC,KNL_NODE_START);

		//! (3)Video Encode
		tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_H264_ENC);
		ubKNL_ImgSrc = tProc.ubSrcNum;	//For ImgMonitTask
		ubKNL_ImgEnc((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tProc.ulDramAddr1,ulTemp);
	}
	else
	{
		ubBUF_ReleaseSenYuvBuf(tProc.ulDramAddr1);
	}
}

//------------------------------------------------------------------------------
void KNL_H264DecProcess(KNL_PROCESS tProc)
{
	KNL_NODE_INFO tNodeInfo;
	IMG_SCALLING_DOWN_SETUP	tScalingInfo;
	uint32_t ulAddr;
	uint8_t ubNextNode;

	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_H264_DEC);
		if(ubNextNode == KNL_NODE_LCD)
		{
			if(!KNL_ChkLcdDispLocation(tProc.ubSrcNum))
			{
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
				return;
			}
			KNL_LcdDisplaySetting();
			ulAddr = ulKNL_GetLcdDispAddr(tProc.ubSrcNum);
			if(ulAddr == 0)
			{
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
				return;
			}

			//Lock the Resource First
			ubKNL_ImgRdy = 0;
			KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_H264_DEC,KNL_NODE_START);

			tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_H264_DEC);
			//Rotate
			if(tNodeInfo.ubRotate == 1)
			{
				H264_SetRotationEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE);
			}
			else
			{
				H264_SetRotationEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,H264_DISABLE);
			}

			//Scaling
			if((tNodeInfo.ubHScale == KNL_SCALE_X0P5) && (tNodeInfo.ubVScale == KNL_SCALE_X0P5))
			{
				tScalingInfo.ulDesAddr	= ulAddr;
				tScalingInfo.ulHeight	= tNodeInfo.uwVdoV;
				tScalingInfo.ulWidth	= tNodeInfo.uwVdoH;
				tScalingInfo.RATIO		= SCALING_DOWN_2;	//X0.5
				IMG_ScalingDownSetup(&tScalingInfo);
			}
			else
			{
				tScalingInfo.ulDesAddr	= ulAddr;
				tScalingInfo.ulHeight	= tNodeInfo.uwVdoV;
				tScalingInfo.ulWidth	= tNodeInfo.uwVdoH;
				tScalingInfo.RATIO		= SCALING_DOWN_1;	//X1.0
				IMG_ScalingDownSetup(&tScalingInfo);
			}

			//H-Mirror(Mirror)
			if(tNodeInfo.ubHMirror)
			{
				H264_SetMirrorEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE);
			}
			else
			{
				H264_SetMirrorEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,H264_DISABLE);
			}

			//V-Mirror(Flip)
			if(tNodeInfo.ubVMirror)
			{
				H264_SetFlipEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE);
			}
			else
			{
				H264_SetFlipEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,H264_DISABLE);
			}

			ubKNL_ImgSrc = tProc.ubSrcNum;	//For ImgMonitTask
			if(!ubKNL_ImgDec((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx,ulAddr,tProc.ulDramAddr2))
			{
				ubKNL_ImgRdy = 1;
				KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_H264_DEC,KNL_NODE_STOP);
			}
		}
		else
		{
			ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
			printd(DBG_ErrorLvl, "Err @KNL_H264DecProcess\r\n");
		}
	}
	else
	{
		ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
	}
}

//------------------------------------------------------------------------------
//Img Merge Buffer
uint32_t ulKNL_GetImgMergeBufAddr(uint8_t ubSrcNum)
{
	uint32_t ulAddr;
	uint8_t ubDisp1Src;
	uint8_t ubDisp2Src;
	uint8_t ubDisp3Src;
	uint8_t ubDisp4Src;	
	
	ubDisp1Src = ubDisp1Src;
	ubDisp2Src = ubDisp2Src;
	ubDisp3Src = ubDisp3Src;
	ubDisp4Src = ubDisp4Src;
	
	if((tKNL_GetDispType() == KNL_DISP_H )&&(tKNL_GetDispRotate()==KNL_DISP_ROTATE_0))
	{			
		//BUF(DISP_4)->BUF(DISP_1)
		if(KNL_DISP_LOCATION1 == tKNL_GetDispLocation(ubSrcNum))
		{
			ulAddr = ulBUF_GetBlkBufAddr(0,BUF_IMG_MERGE);
			ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
			ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
			ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
			ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);			
			
			ulAddr = ulAddr + (((uint32_t)uwKNL_GetVdoH(ubDisp1Src))*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5)+(((uint32_t)uwKNL_GetLcdDmyImgH())*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5);
		}				
		else if(KNL_DISP_LOCATION4 == tKNL_GetDispLocation(ubSrcNum))
		{
			ulAddr = ulBUF_GetBlkBufAddr(0,BUF_IMG_MERGE);			
		}					
	}
	else if((tKNL_GetDispType() == KNL_DISP_3T_1T2B)&&(tKNL_GetDispRotate()== KNL_DISP_ROTATE_0))
	{
		//BUF(DISP_2)->BUF(DISP_3)
		ulAddr = ulBUF_GetBlkBufAddr(0,BUF_IMG_MERGE);
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);
		
		if(KNL_DISP_LOCATION2 == tKNL_GetDispLocation(ubSrcNum))
		{
			ulAddr = ulAddr+0;
		}
		else if(KNL_DISP_LOCATION3 == tKNL_GetDispLocation(ubSrcNum))
		{
			ulAddr = ulAddr+(((uint32_t)uwKNL_GetVdoH(ubDisp2Src))*((uint32_t)uwKNL_GetVdoV(ubDisp2Src))*1.5);
		}
	}
	else if((tKNL_GetDispType() == KNL_DISP_3T_2T1B)&&(tKNL_GetDispRotate()== KNL_DISP_ROTATE_0))
	{
		//BUF(DISP_1)->BUF(DISP_2)
		ulAddr = ulBUF_GetBlkBufAddr(0,BUF_IMG_MERGE);
		ubDisp1Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
		ubDisp2Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
		ubDisp3Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
		ubDisp4Src = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);
		
		if(KNL_DISP_LOCATION1 == tKNL_GetDispLocation(ubSrcNum))
		{
			ulAddr = ulAddr+0;
		}
		else if(KNL_DISP_LOCATION2 == tKNL_GetDispLocation(ubSrcNum))
		{
			ulAddr = ulAddr+(((uint32_t)uwKNL_GetVdoH(ubDisp1Src))*((uint32_t)uwKNL_GetVdoV(ubDisp1Src))*1.5);
		}
	}
	
	return ulAddr;
}
//------------------------------------------------------------------------------
void KNL_JpegDec1Process(KNL_PROCESS tProc)
{
	uint8_t ubNextNode;
	uint32_t ulYuvAddr;
	KNL_NODE_INFO tNodeInfo;
	uint16_t uwVdoH;
	uint16_t uwVdoV;
	KNL_PROCESS tKNLInfo;	
	
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		if(osSemaphoreWait(JPEG_CodecSem, 0) == osOK)
		{
			//Lock Resource First
			KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_JPG_DEC1,KNL_NODE_START);
			
			ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_JPG_DEC1);
			
			if(ubNextNode == KNL_NODE_IMG_MERGE_BUF)
			{				
				ulYuvAddr = ulKNL_GetImgMergeBufAddr(tProc.ubSrcNum);
				tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_JPG_DEC1);
				ubKNL_JpegPreNode = KNL_NODE_JPG_DEC1;
				ubKNL_JpegSrc 	  = tProc.ubSrcNum;
				ubKNL_JPEGDecode(&tNodeInfo, tNodeInfo.uwVdoH, tNodeInfo.uwVdoV, ulYuvAddr, tProc.ulDramAddr2);
			}
			else if(ubNextNode == KNL_NODE_LCD)
			{
				ulYuvAddr = ulKNL_GetLcdDispAddr(tProc.ubSrcNum);
				tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_JPG_DEC1);

				uwVdoH = uwKNL_GetVdoH(tProc.ubSrcNum);
				uwVdoV = uwKNL_GetVdoV(tProc.ubSrcNum);

				ubKNL_JpegPreNode = KNL_NODE_JPG_DEC1;
				ubKNL_JpegSrc = tProc.ubSrcNum;
				ubKNL_JPEGDecode(&tNodeInfo, uwVdoH, uwVdoV, ulYuvAddr, tProc.ulDramAddr2);
			}
			else
			{			
				printd(DBG_ErrorLvl, "Err @KNL_JpegDec1Process\r\n");
			}
		}
		else
		{
			printd(DBG_Debug3Lvl, "J->DQ1\r\n");
		#if FPGA
			osDelay(50);
		#endif
		#if ASIC
			osDelay(10);
		#endif
			tKNLInfo.ubSrcNum			= tProc.ubSrcNum;
			tKNLInfo.ubCurNode 			= ubKNL_GetPreNode(tProc.ubSrcNum,KNL_NODE_JPG_DEC1);			
			tKNLInfo.ubNextNode			= KNL_NODE_JPG_DEC1;	
			tKNLInfo.ulDramAddr1		= tProc.ulDramAddr1;
			tKNLInfo.ulDramAddr2		= tProc.ulDramAddr2;
            if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
			{		
				printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
			}		
		}
	}	
}
//------------------------------------------------------------------------------
void KNL_JpegDec2Process(KNL_PROCESS tProc)
{
	uint8_t ubNextNode;
	uint32_t ulOutAddr;
	KNL_NODE_INFO tNodeInfo;	
	KNL_PROCESS tKNLInfo;	

	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{		
		if(osSemaphoreWait(JPEG_CodecSem, 0) == osOK)
		{
			//Lock Resource First
			KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_JPG_DEC2,KNL_NODE_START);

			ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_JPG_DEC2);
			
			if(ubNextNode == KNL_NODE_LCD)
			{			
				if((tKNL_GetDispType() == KNL_DISP_H) && (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0))
				{
					ulOutAddr = ulKNL_GetLcdDispAddr(tProc.ubSrcNum);	
					tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum, KNL_NODE_JPG_DEC2);
					ubKNL_JpegPreNode = KNL_NODE_JPG_DEC2;
					ubKNL_JpegSrc = tProc.ubSrcNum;
					ubKNL_JPEGDecode(&tNodeInfo, tNodeInfo.uwVdoH, tNodeInfo.uwVdoV, ulOutAddr, tProc.ulDramAddr2);
				}
			}
		}
		else
		{
			printd(DBG_Debug3Lvl, "J->DQ2\r\n");
		#if FPGA
			osDelay(50);
		#endif
		#if ASIC
			osDelay(30);
		#endif
			tKNLInfo.ubSrcNum			= tProc.ubSrcNum;
			tKNLInfo.ubCurNode 			= ubKNL_GetPreNode(tProc.ubSrcNum,KNL_NODE_JPG_DEC2);
			tKNLInfo.ubNextNode			= KNL_NODE_JPG_DEC2;	
			tKNLInfo.ulDramAddr1		= tProc.ulDramAddr1;
			tKNLInfo.ulDramAddr2		= tProc.ulDramAddr2;
            if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
			{		
				printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
			}		
		}
	}
}
//------------------------------------------------------------------------------
void KNL_JpegEncProcess(KNL_PROCESS tProc)
{
	uint8_t ubNextNode;	
	KNL_NODE_INFO tNodeInfo;
	uint32_t ulMergeImgBsAddr;
	uint32_t ulMergeImgYuvAddr;
	KNL_PROCESS tKNLInfo;	

	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{		
		if(osSemaphoreWait(JPEG_CodecSem, 0) == osOK)
		{
			//Lock Resource First
			KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_JPG_ENC,KNL_NODE_START);
			
			ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_JPG_ENC);
			tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_JPG_ENC);
			
			if(ubNextNode == KNL_NODE_VDO_BS_BUF2)
			{
				ulMergeImgYuvAddr = ulBUF_GetBlkBufAddr(0,BUF_IMG_MERGE);
				
				if(tProc.ubSrcNum == KNL_SRC_1_SUB)
				{
					ulMergeImgBsAddr	= ulBUF_GetVdoSubBs01FreeBuf();
				}
				else if(tProc.ubSrcNum == KNL_SRC_4_SUB)
				{
					ulMergeImgBsAddr	= ulBUF_GetVdoSubBs31FreeBuf();
				}
				ubKNL_JpegPreNode 	= KNL_NODE_JPG_ENC;
				ubKNL_JpegSrc 		= tProc.ubSrcNum;
				ubKNL_JPEGEncode(tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulMergeImgYuvAddr,ulMergeImgBsAddr);
			}
			else
			{
				printd(DBG_ErrorLvl, "Err @KNL_JpegEnc1Process\r\n");
			}
		}
		else
		{
			printd(DBG_Debug3Lvl, "J->EQ1\r\n");
		#if FPGA
			osDelay(50);
		#endif
		#if ASIC
			osDelay(30);
		#endif
			tKNLInfo.ubSrcNum			= tProc.ubSrcNum;
			tKNLInfo.ubCurNode 			= ubKNL_GetPreNode(tProc.ubSrcNum,KNL_NODE_JPG_ENC);			
			tKNLInfo.ubNextNode			= KNL_NODE_JPG_ENC;	
			tKNLInfo.ulDramAddr1		= tProc.ulDramAddr1;
			tKNLInfo.ulDramAddr2		= tProc.ulDramAddr2;
            if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
			{		
				printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
			}
		}
	}	
}

//------------------------------------------------------------------------------
void KNL_ImgMergeBufProcess(KNL_PROCESS tProc)
{	
	KNL_PROCESS tMergeProc;
	
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		tMergeProc.ulDramAddr1	= tProc.ulDramAddr1;
		tMergeProc.ulDramAddr2	= tProc.ulDramAddr2;	
		tMergeProc.ubSrcNum		= tProc.ubSrcNum;			
		tMergeProc.ubCurNode	= KNL_NODE_IMG_MERGE_BUF;									
		tMergeProc.ubNextNode 	= ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_IMG_MERGE_BUF);					
        if(osMessagePut(KNL_VdoCodecProcQueue, &tMergeProc, 0) == osErrorResource)
		{		
			printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
		}
	}
}
//------------------------------------------------------------------------------
void KNL_ImgMergeHProcess(KNL_PROCESS tProc)
{	
	KNL_PROCESS tMergeProc;	
	uint32_t ulAddr1,ulAddr2,ulOutAddr;	
	KNL_NODE_INFO tNodeInfo;	
	IMG_BOOL_RESULT RESULT;	 	
	IMG_IMAGE_TASK ImageTask;		
	IMG_MERGE_SETUP MergeSetup;
	IMG_SCALLING_DOWN_SETUP sScalingSetup;	
	
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{	
		if(ubKNL_ChkImgRdy())
		{
			//Lock the Resource First
			ubKNL_ImgRdy = 0;
			KNL_SetNodeState(tProc.ubSrcNum,KNL_NODE_IMG_MERGE_H,KNL_NODE_START);
			
			tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_IMG_MERGE_H);			
			
			ulAddr1 = ulKNL_GetImgMergeBufAddr(tNodeInfo.ubMergeSrc1);	//IMG Src1		
			ulAddr2 = ulKNL_GetImgMergeBufAddr(tNodeInfo.ubMergeSrc2);	//IMG Src2		
			if(tNodeInfo.ubMergeDest == IMG_MERGE_LCD)									//IMG Dest			
			{				
				ulOutAddr = ulKNL_GetLcdDispAddr(tProc.ubSrcNum);			
			}	
			else
			{
				printd(DBG_ErrorLvl, "Err @KNL_ImgMergeH1Process@\r\n");
			}
			
			//HW Img Merge
			osSemaphoreWait(tKNL_ImgSem, osWaitForever);
			
			sScalingSetup.ulWidth	= tNodeInfo.uwMergeH;
			sScalingSetup.ulHeight	= tNodeInfo.uwMergeV;
			
			sScalingSetup.RATIO = SCALING_DOWN_1;
			sScalingSetup.ulDesAddr = ulOutAddr;
			RESULT = IMG_ScalingDownSetup(&sScalingSetup);	//Merge Output		
			if(RESULT == IMG_FAIL)
			{
				printd(DBG_ErrorLvl, "Merge Setup Fail1 \n");
			}
			
			MergeSetup.STATUS =IMG_ENABLE;
			MergeSetup.LOCATION = ISP_IMAGE_IN_1_LOCATION;
			MergeSetup.TYPE = COMBINE_2_IMAGE;	
			MergeSetup.IMG_1_Addr = ulAddr2;		//Source2 for Merge		
			RESULT = IMG_MergeSetup(&MergeSetup);
			if(RESULT == IMG_FAIL)
			{
				printd(DBG_ErrorLvl, "Merge Setup Fail2 \n");
			}
			
			ImageTask.InputSrcAddr	= ulAddr1;	//Source1 for Merge		
			ImageTask.H264_Task = NULL;
			ImageTask.JPEGEnable = IMG_DISABLE;	
			ImageTask.ScalingEnable = IMG_ENABLE;
			
			ubKNL_ImgTrigSrc	= tProc.ubSrcNum;
			ubKNL_ImgTrigType	= KNL_IMG_MERGE_H;
			IMG_StartUp(ImageTask);
		}
		else
		{
			printd(DBG_Debug3Lvl, "IMH1-Q\r\n");
			tMergeProc.ulDramAddr1	= tProc.ulDramAddr1;
			tMergeProc.ulDramAddr2	= tProc.ulDramAddr2;	
			tMergeProc.ubSrcNum		= tProc.ubSrcNum;			
			tMergeProc.ubCurNode	= ubKNL_GetPreNode(tProc.ubSrcNum,KNL_NODE_IMG_MERGE_H);							
			tMergeProc.ubNextNode 	= KNL_NODE_IMG_MERGE_H;
            if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tMergeProc, 0) == osErrorResource)
			{		
				printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
			}
		}
	}
}
//------------------------------------------------------------------------------
void KNL_VdoBsBuf1Process(KNL_PROCESS tProc)
{
	KNL_PROCESS tKNLInfo;
	KNL_NODE_INFO tNodeInfo;
#if KNL_REC_FUNC_ENABLE
    REC_INFO RecInfo;
#endif

	if (KNL_GetTuningToolMode() == KNL_TUNINGMODE_ON)
	{
        if(tProc.ubSrcNum == KNL_SRC_1_MAIN)
        {
            if(USB_UVC_VS_FORMAT_FRAME_BASED == UVC_GetVdoFormat() && (UVC_GetVdoWidth() == USB_UVC_HD_WIDTH || UVC_GetVdoWidth() == USB_UVC_FHD_WIDTH))
            {
				uvc_update_image((uint32_t *)tProc.ulDramAddr2, (uint32_t)tProc.ulSize);
            }
			else if(USB_UVC_VS_FORMAT_UNCOMPRESSED == UVC_GetVdoFormat())
			{
                uvc_update_image((uint32_t *)tProc.ulDramAddr1, (uint32_t)(YUY2_WIDTH*YUY2_HEIGHT*2));
            }
        }
		else if(tProc.ubSrcNum == KNL_SRC_1_AUX)
		{
            if(USB_UVC_VS_FORMAT_FRAME_BASED == UVC_GetVdoFormat() && UVC_GetVdoWidth() == USB_UVC_VGA_WIDTH)
                uvc_update_image((uint32_t *)tProc.ulDramAddr2, (uint32_t)tProc.ulSize);
        }
	}

	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		osMessageQId osMsgQId;

		tKNLInfo.ubSrcNum		= tProc.ubSrcNum;
		tKNLInfo.ubCurNode 		= KNL_NODE_VDO_BS_BUF1;
		tKNLInfo.ubNextNode		= ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_VDO_BS_BUF1);
		tKNLInfo.ulDramAddr1	= tProc.ulDramAddr1;
		tKNLInfo.ulDramAddr2	= tProc.ulDramAddr2;
		tKNLInfo.ulSize			= tProc.ulSize;
		tKNLInfo.ulIdx			= tProc.ulIdx;
		tKNLInfo.ulGop			= tProc.ulGop;
		tKNLInfo.ubCodecIdx		= tProc.ubCodecIdx;
		tKNLInfo.ubVdoGop		= tProc.ubVdoGop;
		osMsgQId				= (KNL_NODE_COMM_TX_VDO == tKNLInfo.ubNextNode)?KNL_CommTxProcQueue:KNL_VdoCodecProcQueue;
		if(osMessagePut(osMsgQId, &tKNLInfo, 0) != osOK)
		{
			tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum, KNL_NODE_VDO_BS_BUF1);
			if((tNodeInfo.ubPreNode == KNL_NODE_COMM_RX_VDO) && (tNodeInfo.ubNextNode == KNL_NODE_H264_DEC))
			{
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
			}
			else if(tNodeInfo.ubPreNode == KNL_NODE_H264_ENC)
			{
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC,tProc.ubSrcNum,tProc.ulDramAddr2);
			}
			printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
		}

		if(ubKNL_StartRecFlag == 1)
		{
		#if KNL_REC_FUNC_ENABLE
			RecInfo.ubCh = 0;
			RecInfo.ubRestartFg = 0;
			if(tKNL_GetFrameType(tProc.ulDramAddr2) == KNL_I_FRAME)
			{
				//printd(DBG_Debug3Lvl, "KNL_I_FRAME\r\n");
				RecInfo.ubPictureType = REC_I_VFRM;
			}
			else
			{
				//printd(DBG_Debug3Lvl, "KNL_P_FRAME\r\n");
				RecInfo.ubPictureType = REC_P_VFRM;
			}
			RecInfo.ulDramAddr = (uint32_t)tProc.ulDramAddr2;
			RecInfo.ulSize = tProc.ulSize;
			RecInfo.ulTimeStamp = APP_TIMER_Get1ms();
			RecInfo.ubCmd = REC_CMD_VIDEXT;
			if(!ubREC_SendVDOQueue(&RecInfo))
				printd(DBG_ErrorLvl, "SendVDOQueue Fail: %s: %d\n\r", __FILE__, __LINE__);
		#endif
		}
	}
	else
	{
		tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_VDO_BS_BUF1);
		if((tNodeInfo.ubPreNode == KNL_NODE_COMM_RX_VDO) && (tNodeInfo.ubNextNode == KNL_NODE_H264_DEC))
		{
			ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
		}
		else if(tNodeInfo.ubPreNode == KNL_NODE_H264_ENC)
		{
			ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC,tProc.ubSrcNum,tProc.ulDramAddr2);
		}
	}
}
//------------------------------------------------------------------------------
void KNL_VdoBsBuf2Process(KNL_PROCESS tProc)
{
	KNL_PROCESS tKNLInfo;
	
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
		osMessageQId osMsgQId;

		tKNLInfo.ubSrcNum		= tProc.ubSrcNum;
		tKNLInfo.ubCurNode 		= KNL_NODE_VDO_BS_BUF2;
		tKNLInfo.ubNextNode		= ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_VDO_BS_BUF2);
		tKNLInfo.ulDramAddr1	= tProc.ulDramAddr1;
		tKNLInfo.ulDramAddr2	= tProc.ulDramAddr2;
		tKNLInfo.ulSize			= tProc.ulSize;
		tKNLInfo.ulIdx			= tProc.ulIdx;
		tKNLInfo.ulGop			= tProc.ulGop;
		tKNLInfo.ubCodecIdx		= tProc.ubCodecIdx;
		tKNLInfo.ubVdoGop		= tProc.ubVdoGop;
		osMsgQId				= (KNL_NODE_COMM_TX_VDO == tKNLInfo.ubNextNode)?KNL_CommTxProcQueue:KNL_VdoCodecProcQueue;
        if(osMessagePut(osMsgQId, &tKNLInfo, 0) == osErrorResource)
		{
			printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
		}
	}
}

//------------------------------------------------------------------------------
void KNL_BbTxVdoProcess(KNL_PROCESS tProc)
{
	uint8_t ubTemp;
	uint32_t ulExtSz;
	KNL_NODE_INFO tNodeInfo;
	struct H264_ENCODE *codec;
	uint8_t ubTotalBufNum;
	uint8_t ubUsedBufNum;
	uint8_t ubBbStartFlg = 0;
	uint8_t ubBbLinkFlg	 = 0;
	uint8_t ubBbBufFlg	 = 0;

	tNodeInfo = tNodeInfo;
	codec = codec;
	if(ubKNL_ChkVdoFlowAct(tProc.ubSrcNum))
	{
RTYSEND_VDO:
		//Check 1
		if(ubBB_GetStartFlg() == BB_TASK_START)
		{
			ubBbStartFlg = 1;
		}
		else
		{
			ubBbStartFlg = 0;
		}

		//Check 2
		if(ubKNL_GetCommLinkStatus(KNL_MASTER_AP) == BB_LINK)
		{
			ubBbLinkFlg	= 1;
		}
		else
		{
			ubBbLinkFlg	= 0;
		}

		//Check 3
		ubTotalBufNum = ubBB_GetTxTotalBufNum(BB_DATA_VIDEO,BB_TX_MASTER);
		ubUsedBufNum  = ubBB_GetTxUsedBufNum(BB_DATA_VIDEO,BB_TX_MASTER);
		if((ubTotalBufNum - ubUsedBufNum) >= 1)
		{
			ubBbBufFlg = 1;
		}
		else
		{
			ubBbBufFlg = 0;
		}

		printd(DBG_Debug2Lvl, "BB[%d]: %d_%d_%d(%d)\r\n", tProc.ubSrcNum, ubBbStartFlg, ubBbLinkFlg, ubBbBufFlg, ubKNL_BbRtyFlg);
		//Case 1
		if(ubBbStartFlg && ubBbLinkFlg && ubBbBufFlg)
		{
			ubKNL_BbRtyFlg = 0;
			ulExtSz = ulKNL_AddAuxInfo(KNL_VDO_PKT,tProc.ubSrcNum,tProc.ulDramAddr2,tProc.ulSize,tProc.ulIdx,tProc.ulGop,tProc.ubVdoGop);
			if(tBB_SendData(NULL,BB_DATA_VIDEO,(uint8_t *)tProc.ulDramAddr2,ulExtSz,BB_TX_MASTER) != BB_SET_BUF_SUCCESS)
			{
				printd(DBG_ErrorLvl, "BB Busy !\r\n");
			}
			//printd(DBG_Debug3Lvl, "V:0x%x->%d_BW %d KB\r\n",tProc.ulSize,ubUsedBufNum,ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW)/1024);
			//Relase BS Buffer
			ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_TX_VDO,tProc.ubSrcNum,tProc.ulDramAddr2);
			if(ubTemp == BUF_OK)
			{
				printd(DBG_Debug3Lvl, "Release BUF_VDO_BS Ok\r\n");
			}
			else
			{
				printd(DBG_ErrorLvl, "Release BUF_VDO_BS Fail\r\n");
			}
		}
		//Case 2
		else if(ubBbStartFlg && (!ubBbLinkFlg))
		{
			ubKNL_BbRtyFlg = 0;
			//printd(DBG_Debug3Lvl, "BU(V)\r\n");

			BB_ClearTxBuf(BB_TX_MASTER,BB_DATA_VIDEO);
			//Relase BS Buffer
			ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_TX_VDO,tProc.ubSrcNum,tProc.ulDramAddr2);
			if(ubTemp == BUF_OK)
			{
				printd(DBG_Debug3Lvl, "Release BUF_VDO_BS Ok\r\n");
			}
			else
			{
				printd(DBG_ErrorLvl, "Release BUF_VDO_BS Fail\r\n");
			}			
			if(ubKNL_ExistNode(tProc.ubSrcNum,KNL_NODE_H264_ENC))
			{
				tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_H264_ENC);
				if(KNL_GetTuningToolMode() == KNL_TUNINGMODE_OFF) 
            	    H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);	//! ubKNL_ResetIFlg = 1;
			}
		}
		//Case 3
		else if(ubBbStartFlg && ubBbLinkFlg && (!ubBbBufFlg) && (!ubKNL_BbRtyFlg))
		{
			ubKNL_BbRtyFlg = 1;
			printd(DBG_Debug3Lvl, "V->RTY:0x%x\r\n",tProc.ulSize);
			osDelay(30);
			goto RTYSEND_VDO;
		}
		else
		{
			ubKNL_BbRtyFlg = 0;
			printd(DBG_Debug3Lvl, "V->RTY:0x%x->Clr\r\n",tProc.ulSize);
			BB_ClearTxBuf(BB_TX_MASTER,BB_DATA_VIDEO);
			//Relase BS Buffer
			ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_TX_VDO,tProc.ubSrcNum,tProc.ulDramAddr2);
			if(ubTemp == BUF_OK)
			{
				printd(DBG_Debug3Lvl, "Release BUF_VDO_BS Ok\r\n");
			}
			else
			{
				printd(DBG_ErrorLvl, "Release BUF_VDO_BS Fail\r\n");
			}
			if(ubKNL_ExistNode(tProc.ubSrcNum,KNL_NODE_H264_ENC))
			{
				tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_H264_ENC);
				if(KNL_GetTuningToolMode() == KNL_TUNINGMODE_OFF)
				H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);	//! ubKNL_ResetIFlg = 1;
			}
		}
	}
	else
	{
		BB_ClearTxBuf(BB_TX_MASTER,BB_DATA_VIDEO);
		//Relase BS Buffer	
		ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_TX_VDO,tProc.ubSrcNum,tProc.ulDramAddr2);
		if(ubTemp == BUF_OK)
		{
			printd(DBG_Debug3Lvl, "Release BUF_VDO_BS Ok\r\n");
		}
		else
		{
			printd(DBG_ErrorLvl, "Release BUF_VDO_BS Fail\r\n");
		}
		if(ubKNL_ExistNode(tProc.ubSrcNum,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(tProc.ubSrcNum,KNL_NODE_H264_ENC);
			if(KNL_GetTuningToolMode() == KNL_TUNINGMODE_OFF)
				H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);	//! ubKNL_ResetIFlg = 1;
		}
	}
}

//------------------------------------------------------------------------------
uint32_t ulKNL_GetPktSZ(uint32_t ulAddr,uint32_t ulSize)
{
	uint32_t ulRtnValue;	
	
	ulRtnValue = 0;
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-6))))<<0);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-5))))<<8);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-4))))<<16);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-3))))<<24);
	
	return ulRtnValue;
}		

//------------------------------------------------------------------------------
void KNL_AdcBufProcess(KNL_PROCESS tProc)
{
	uint32_t ulTempAddr;
	uint8_t ubTemp;
	uint8_t ubNextNode;	
	uint8_t ubLinkRole;
	uint32_t ulExtSz;
	uint8_t ubAdoRtyCnt = 0;
	uint8_t ubTotalBufNum;
	uint8_t ubUsedBufNum;	
	uint8_t ubBufOkFlg = 0;

RTYSEND_ADO:
	if(tBB_GetTxAdoPath() == BB_TX_ADO_STA1)
	{
		ubLinkRole = KNL_STA1;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_STA2)
	{
		ubLinkRole = KNL_STA2;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_STA3)
	{
		ubLinkRole = KNL_STA3;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_STA4)
	{
		ubLinkRole = KNL_STA4;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_SLAVE_AP)
	{
		ubLinkRole = KNL_SLAVE_AP;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_MASTER_AP)
	{
		ubLinkRole = KNL_MASTER_AP;
	}

	ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,KNL_NODE_ADC_BUF);
	if(ubNextNode == KNL_NODE_COMM_TX_ADO)
	{
		if(ubKNL_GetCommLinkStatus(ubLinkRole) == BB_LOST_LINK)		
			return;

		ubBufOkFlg = 0;
		ubTotalBufNum = ubBB_GetTxTotalBufNum(BB_DATA_AUDIO,BB_TX_MASTER);
		ubUsedBufNum  = ubBB_GetTxUsedBufNum(BB_DATA_AUDIO,BB_TX_MASTER);
		if((ubTotalBufNum-ubUsedBufNum) >= 1)
			ubBufOkFlg = 1;
		if((ubBB_GetStartFlg() == BB_TASK_START) && (ubBufOkFlg))
		{
			SET_BUF tKNL_BbStatus;

			if(!ubAdoRtyCnt)
			{
				DMAC_RESULT tDmaResult = DMAC_OK;

				ulTempAddr = ulBUF_GetAdcFreeBuf();
				if(BUF_FAIL == ulTempAddr)
					return;
				tDmaResult = tDMAC_MemCopy(tProc.ulDramAddr2,ulTempAddr,tProc.ulSize,NULL);
				if(DMAC_OK != tDmaResult)
				{
					ubBUF_ReleaseAdcBuf(ulTempAddr);
					printd(DBG_ErrorLvl, "DMA NRDY @%s !\n", __func__);
					return;
				}
				ulExtSz = ulKNL_AddAuxInfo(KNL_ADO_PKT,tProc.ubSrcNum,ulTempAddr,tProc.ulSize,0,0,0);
			}
			tKNL_BbStatus = tBB_SendData(NULL,BB_DATA_AUDIO,(uint8_t *)ulTempAddr,ulExtSz,(SET_TX_PATH)NULL);
			switch(tKNL_BbStatus)
			{
				case BB_SET_BUF_SUCCESS:
					printd(DBG_Debug3Lvl, "A:0x%x\r\n",ulExtSz);
					break;
				case BB_SET_BUF_BUSY:
					printd(DBG_Debug3Lvl, "A->RTY:0x%x\r\n",ulExtSz);
					if(++ubAdoRtyCnt < 25)
					{
						osDelay(20);
						goto RTYSEND_ADO;
					}
					break;
				default:
					break;
			}
			ubTemp = ubBUF_ReleaseAdcBuf(ulTempAddr);
			if(ubTemp == BUF_OK)
			{
				printd(DBG_Debug3Lvl, "Release BUF_ADC Ok\r\n");
			}
			else
			{
				printd(DBG_ErrorLvl, "Release BUF_ADC Fail\r\n");
			}
		}
		else
		{
			BB_ClearTxBuf(BB_TX_MASTER, BB_DATA_AUDIO);
		}
	}
}
//------------------------------------------------------------------------------
void KNL_RetryAdcBufProcess(KNL_PROCESS tProc)
{ 
	KNL_PROCESS tKNLInfo;
	uint8_t ubTemp;
	LINK_ROLE tLinkRole;

	if(tBB_GetTxAdoPath() == BB_TX_ADO_STA1)
	{
		tLinkRole = BB_STA1;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_STA2)
	{
		tLinkRole = BB_STA2;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_STA3)
	{
		tLinkRole = BB_STA3;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_STA4)
	{
		tLinkRole = BB_STA4;
	}	
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_MASTER_AP)
	{
		tLinkRole = BB_MASTER_AP;
	}
	else if(tBB_GetTxAdoPath() == BB_TX_ADO_SLAVE_AP)
	{
		tLinkRole = BB_SLAVE_AP;
	}	

	if((ubBB_GetStartFlg() == BB_TASK_START) && (ubKNL_GetCommLinkStatus(tLinkRole) == BB_LINK))
	{
		SET_BUF tKNL_BbStatus;

		tKNL_BbStatus = tBB_SendData(NULL,BB_DATA_AUDIO,(uint8_t *)tProc.ulDramAddr2,tProc.ulSize,(SET_TX_PATH)NULL);
		if(BB_SET_BUF_SUCCESS == tKNL_BbStatus)	
		{
			printd(DBG_Debug3Lvl, "RA:0x%x\r\n",tProc.ulSize);
			//Relase BS Buffer
			//-------------------------------------------------------------
			ubTemp = ubBUF_ReleaseAdcBuf(tProc.ulDramAddr2);
			if(ubTemp == BUF_OK)
			{
				printd(DBG_Debug3Lvl, "Release BUF_ADO Ok\r\n");
			}
			else
			{
				printd(DBG_ErrorLvl, "Release BUF_ADC Fail\r\n");
			}
		}
		else if(BB_SET_BUF_BUSY == tKNL_BbStatus)	
		{
			printd(DBG_Debug3Lvl, "RAQ:0x%x\r\n",tProc.ulSize);
			osDelay(30);
			//Retry Method
			tKNLInfo.ubSrcNum    = tProc.ubSrcNum;
			tKNLInfo.ubCurNode   = KNL_NODE_RETRY_ADC_BUF;
			tKNLInfo.ubNextNode  = KNL_NODE_RETRY_ADC_BUF;
			tKNLInfo.ulDramAddr1 = tProc.ulDramAddr1;
			tKNLInfo.ulDramAddr2 = tProc.ulDramAddr2;
			tKNLInfo.ulSize      = tProc.ulSize;
			if(osMessagePutToFront(KNL_AdoCdoecProcQueue, &tKNLInfo, 0) == osErrorResource)
			{
				ubBUF_ReleaseAdcBuf(tKNLInfo.ulDramAddr2);
				printd(DBG_ErrorLvl, "KNL_ADO Q->Full !!!!\r\n");
			}
		}
	}
	else
	{
		printd(DBG_Debug3Lvl, "BU(A)\r\n");
		//Relase BS Buffer
		ubTemp = ubBUF_ReleaseAdcBuf(tKNLInfo.ulDramAddr2);
		if(ubTemp == BUF_OK)
		{
			printd(DBG_Debug3Lvl, "Release BUF_ADC Ok\r\n");
		}
		else
		{
			printd(DBG_ErrorLvl, "Release BUF_ADC Fail\r\n");
		}
	}
}
//------------------------------------------------------------------------------
void KNL_DacBufProcess(KNL_PROCESS tProc)
{
	uint8_t ubTemp = BUF_FAIL, ubSrcNumMap;
	uint8_t ubNextNode;
//	uint32_t ulDwCnt;
	static ADO_Queue_INFO EN_INFO;

	ubNextNode = ubKNL_GetNextNode(tProc.ubSrcNum,tProc.ubNextNode);
	if(ubKNL_ChkAdoFlowAct(tProc.ubSrcNum))
	{
		if(ubNextNode == KNL_NODE_DAC)
		{
			if((tADO_GetAdo32Enable() == ADO_ON) && (tKNL_AdoInfo.Compress_method == COMPRESS_NONE))
			{
				EN_INFO.EncType = AUDIO32;			
				EN_INFO.Audio32Addr = tProc.ulDramAddr2;
				EN_INFO.Audio32Size	= tProc.ulSize;
			}
			else if((tADO_GetAACEnable() == ADO_ON) && (tKNL_AdoInfo.Compress_method == COMPRESS_NONE))
			{
				EN_INFO.EncType = AAC;			
				EN_INFO.AACAddr = tProc.ulDramAddr2;
				EN_INFO.AACSize	= tProc.ulSize;
			}
			else if((tADO_GetAdo32Enable() == ADO_OFF) && (tADO_GetAACEnable() == ADO_OFF) && (tKNL_AdoInfo.Compress_method == COMPRESS_MSADPCM))
			{
				//MS-ADPCM
				EN_INFO.EncType = NONE;			
				EN_INFO.PcmAddr = tProc.ulDramAddr2;
				EN_INFO.PcmSize	= tProc.ulSize;
			}
			else if((tADO_GetAdo32Enable() == ADO_OFF) && (tADO_GetAACEnable() == ADO_OFF) && (tKNL_AdoInfo.Compress_method == COMPRESS_NONE))
			{
				//PCM
				EN_INFO.EncType = NONE;			
				EN_INFO.PcmAddr = tProc.ulDramAddr2;
				EN_INFO.PcmSize	= tProc.ulSize;
			}
			EN_INFO.ubSrcNum = ubKNL_SrcNumMap(tProc.ubSrcNum);
//			printf("----->Src:%d\n",EN_INFO.ubSrcNum);
			ADO_DecodeBufferWrtIn(&GlobalAudioMeta,&EN_INFO);
		}	
		else
		{
			printd(DBG_ErrorLvl, "Err @KNL_DacBufProcess\r\n");
		}
	}
	//Release Buffer
	ubSrcNumMap = ubKNL_SrcNumMap(tProc.ubSrcNum);
	if(ubSrcNumMap == 0)
	{
		ubTemp = ubBUF_ReleaseDac0Buf(tProc.ulDramAddr2);
	}
	else if(ubSrcNumMap == 1)
	{
		ubTemp = ubBUF_ReleaseDac1Buf(tProc.ulDramAddr2);
	}
	else if(ubSrcNumMap == 2)
	{
		ubTemp = ubBUF_ReleaseDac2Buf(tProc.ulDramAddr2);
	}
	else if(ubSrcNumMap == 3)
	{
		ubTemp = ubBUF_ReleaseDac3Buf(tProc.ulDramAddr2);
	}
	if(ubTemp == BUF_FAIL)
	{
		printd(DBG_ErrorLvl, "Release Dac Buffer Err !!!!\r\n");
	}
}
//------------------------------------------------------------------------------
uint32_t ulKNL_AlignAdoPktSz(uint32_t ulInputSz)
{
//	if((ulInputSz%BB_STA_ADO_SUB_PKT_LEN) == 0)
//	{
//		return ulInputSz;
//	}
//	else
//	{
//		return ((ulInputSz/BB_STA_ADO_SUB_PKT_LEN)*BB_STA_ADO_SUB_PKT_LEN)+BB_STA_ADO_SUB_PKT_LEN;
//	}
	
	//if((ulInputSz%BB_STA1_ADO_SUB_PKT_LEN) == 0)
	if((ulInputSz%KNL_ADO_SUB_PKT_LEN) == 0)
	{
		return ulInputSz;
	}
	else
	{
		//return ((ulInputSz/BB_STA1_ADO_SUB_PKT_LEN)*BB_STA1_ADO_SUB_PKT_LEN)+BB_STA1_ADO_SUB_PKT_LEN;
		return ((ulInputSz/KNL_ADO_SUB_PKT_LEN)*KNL_ADO_SUB_PKT_LEN)+KNL_ADO_SUB_PKT_LEN;
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetPktSrcNum(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-16));
}

uint8_t ubKNL_GetPktOpMode(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-15));
}

uint32_t ulKNL_GetPktFrmIdx(uint32_t ulAddr,uint32_t ulSize)
{
	uint32_t ulRtnValue;	
	
	ulRtnValue = 0;
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-14))))<<0);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-13))))<<8);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-12))))<<16);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-11))))<<24);
	
	return ulRtnValue;
}

uint32_t ulKNL_GetPktGop(uint32_t ulAddr,uint32_t ulSize)
{
	uint32_t ulRtnValue;	
	
	ulRtnValue = 0;
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-10))))<<0);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-9))))<<8);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-8))))<<16);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-7))))<<24);
	
	return ulRtnValue;
}

uint8_t ubKNL_GetPktVdoGop(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-6));
}

uint16_t ubKNL_GetPktHRes(uint32_t ulAddr,uint32_t ulSize)
{
	return (*((uint8_t *)(ulAddr+ulSize-5)) << 8) + (*((uint8_t *)(ulAddr+ulSize-4)));
}

uint16_t ubKNL_GetPktVRes(uint32_t ulAddr,uint32_t ulSize)
{
	return (*((uint8_t *)(ulAddr+ulSize-3)) << 8) + (*((uint8_t *)(ulAddr+ulSize-2)));
}

uint16_t ubKNL_GetPaddLen(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-1));
}

uint8_t ubKNL_ChkDebugPkt(uint32_t ulAddr,uint32_t ulSize)
{
//	uint8_t hw_crc_8 = 0;	
//	CRC2_t CRC2_Setup;	
//	uint32_t ulCrcCalSz;
//	uint8_t  ubCorrectCrc;	
	
	return 1;
	
//	if(!ubKNL_GetVdoChkAct())
//	{
//		return 1;
//	}

//	ulCrcCalSz = *((uint32_t *)(ulAddr+ulSize-32));
//	
//	CRC2_Setup.CRC_INIT_VALUE = INIT_ALL_ZERO;
//	CRC2_Setup.CRC_FINAL_XOR_VALUE = XOR_ALL_ZERO;
//	CRC2_Setup.CRC_ORDER = 7;	
//	hw_crc_8 = (uint8_t)CRC2_Calc(CRC2_Setup, P_8, ulAddr, ulAddr, ulCrcCalSz);		
//	
//	ubCorrectCrc = *((uint8_t *)(ulAddr+ulSize-16));
//	if(hw_crc_8 != ubCorrectCrc)
//	{	
//		return 0;
//	}	
//	return 1;		
}

static void KNL_TwcMonitThread(void const *argument)
{	
	KNL_PROCESS tProc;
	uint8_t ubData[8] = {0}, ubLen = 1;

	while(1)
	{
		osMessageGet(KNL_TwcMonitQueue, &tProc, osWaitForever);
		ubLen = 1;
		switch((TWC_OPC)tProc.ubTwcCmd)
		{
			case TWC_VDORES_SETTING:
				memset(&ubData, 0, 8);
				ubData[0] = uwKNL_GetVdoH(tProc.ubSrcNum) >> 8;
				ubData[1] = uwKNL_GetVdoH(tProc.ubSrcNum) & 0xFF;
				ubData[2] = uwKNL_GetVdoV(tProc.ubSrcNum) >> 8;
				ubData[3] = uwKNL_GetVdoV(tProc.ubSrcNum) & 0xFF;
				ubData[4] = tProc.ubSrcNum;
				ubLen 	  = 5;
				ubKNL_VdoResChgTwcFlg[tProc.ubTargetRole]  = TRUE;
				break;
			case TWC_RESEND_I:
				ubKNL_VdoResendITwcFlg[tProc.ubTargetRole] = TRUE;
				break;
			default:
				continue;
		}
		if(ubKNL_TwcSend(tProc.ubTargetRole,(TWC_OPC)tProc.ubTwcCmd,ubData,ubLen,8) == TWC_SUCCESS)
		{
			printd(DBG_CriticalLvl, "[%d]:%s->OK\r\n", tProc.ubTargetRole, ((TWC_OPC)tProc.ubTwcCmd == TWC_RESEND_I)?"Resend I":"VideoRes");
		}
		else
		{
			printd(DBG_CriticalLvl, "[%d]:%s->Fail\r\n",tProc.ubTargetRole, ((TWC_OPC)tProc.ubTwcCmd == TWC_RESEND_I)?"Resend I":"VideoRes");
		}
	}
}

#define osTick_RATE		100
static void KNL_AvgPly0Thread(void const *argument)
{	
	KNL_PROCESS tProc;	
	uint32_t ulDlyMs;	
	uint32_t ulTimeBase = 1000/osTick_RATE;	
	uint32_t ulMinusMs = 0;
	uint32_t ulQNum;
	
	while(1)
	{
		if(!ubKNL_AvgPlyStartFlg[0])
		{
			ulMinusMs = 0;
			ulDlyMs = 1000/ubKNL_GetVdoFps();
			ulDlyMs = (ulDlyMs/ulTimeBase)*ulTimeBase;			
			osDelay(20);			
		}
		else
		{			
			ulQNum = osMessages(KNL_AvgPlyQueue[0]);
			printd(DBG_Debug3Lvl, "AQ[0]:%d\r\n",ulQNum);			
			
			if(ulQNum < (ubKNL_GetStartPlyNum()-1))
			{				
				osDelay(ulDlyMs/2);				
				ulMinusMs = 0;
			}
			else if(ulQNum == (ubKNL_GetStartPlyNum()-1))
			{
				ulMinusMs = 0;
			}
			else
			{
				ulMinusMs = ulDlyMs/2;
			}
			
			if(osMessages(KNL_AvgPlyQueue[0]))
			{
				osMessageGet(KNL_AvgPlyQueue[0], &tProc, osWaitForever);
				printd(DBG_Debug3Lvl, "AvgPlyQue->%d\r\n",tProc.ulIdx);			
				
				if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tProc, 0) == osErrorResource)
				{		
					printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
				}			
				
				osDelay(ulDlyMs-ulMinusMs);				
			}
		}
	}
}

static void KNL_AvgPly1Thread(void const *argument)
{	
	KNL_PROCESS tProc;	
	uint32_t ulDlyMs;	
	uint32_t ulTimeBase = 1000/osTick_RATE;	
	uint32_t ulMinusMs = 0;
	uint32_t ulQNum;
	
	while(1)
	{        
		if(!ubKNL_AvgPlyStartFlg[1])
		{
			ulMinusMs = 0;
			ulDlyMs = 1000/ubKNL_GetVdoFps();
			ulDlyMs = (ulDlyMs/ulTimeBase)*ulTimeBase;			
			osDelay(20);			
		}
		else
		{			
			ulQNum = osMessages(KNL_AvgPlyQueue[1]);
			//printd(DBG_Debug3Lvl, "AQ[1]:%d\r\n",ulQNum);			
			
			if(ulQNum < (ubKNL_GetStartPlyNum()-1))
			{				
				osDelay(ulDlyMs/2);				
				ulMinusMs = 0;
			}
			else if(ulQNum == (ubKNL_GetStartPlyNum()-1))
			{
				ulMinusMs = 0;
			}
			else
			{
				ulMinusMs = ulDlyMs/2;
			}
			
			if(osMessages(KNL_AvgPlyQueue[1]))
			{
				osMessageGet(KNL_AvgPlyQueue[1], &tProc, osWaitForever);
				printd(DBG_Debug3Lvl, "AvgPlyQue->%d\r\n",tProc.ulIdx);			
				
				if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tProc, 0) == osErrorResource)
				{		
					printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
				}			
				
				osDelay(ulDlyMs-ulMinusMs);				
			}
		}
	}
}

static void KNL_AvgPly2Thread(void const *argument)
{
	KNL_PROCESS tProc;	
	uint32_t ulDlyMs;	
	uint32_t ulTimeBase = 1000/osTick_RATE;	
	uint32_t ulMinusMs = 0;
	uint32_t ulQNum;
	
	while(1)
	{        
		if(!ubKNL_AvgPlyStartFlg[2])
		{
			ulMinusMs = 0;
			ulDlyMs = 1000/ubKNL_GetVdoFps();
			ulDlyMs = (ulDlyMs/ulTimeBase)*ulTimeBase;			
			osDelay(20);			
		}
		else
		{			
			ulQNum = osMessages(KNL_AvgPlyQueue[2]);
			printd(DBG_Debug3Lvl, "AQ[2]:%d\r\n",ulQNum);			
			
			if(ulQNum < (ubKNL_GetStartPlyNum()-1))
			{				
				osDelay(ulDlyMs/2);				
				ulMinusMs = 0;
			}
			else if(ulQNum == (ubKNL_GetStartPlyNum()-1))
			{
				ulMinusMs = 0;
			}
			else
			{
				ulMinusMs = ulDlyMs/2;
			}
			
			if(osMessages(KNL_AvgPlyQueue[2]))
			{
				osMessageGet(KNL_AvgPlyQueue[2], &tProc, osWaitForever);
				printd(DBG_Debug3Lvl, "AvgPlyQue->%d\r\n",tProc.ulIdx);			
				
				if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tProc, 0) == osErrorResource)
				{		
					printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
				}			
				
				osDelay(ulDlyMs-ulMinusMs);				
			}
		}
	}
}

static void KNL_AvgPly3Thread(void const *argument)
{
	KNL_PROCESS tProc;	
	uint32_t ulDlyMs;	
	uint32_t ulTimeBase = 1000/osTick_RATE;	
	uint32_t ulMinusMs = 0;
	uint32_t ulQNum;
	
	while(1)
	{        
		if(!ubKNL_AvgPlyStartFlg[3])
		{
			ulMinusMs = 0;
			ulDlyMs = 1000/ubKNL_GetVdoFps();
			ulDlyMs = (ulDlyMs/ulTimeBase)*ulTimeBase;			
			osDelay(20);			
		}
		else
		{			
			ulQNum = osMessages(KNL_AvgPlyQueue[3]);
			printd(DBG_Debug3Lvl, "AQ[3]:%d\r\n",ulQNum);			
			
			if(ulQNum < (ubKNL_GetStartPlyNum()-1))
			{				
				osDelay(ulDlyMs/2);				
				ulMinusMs = 0;
			}
			else if(ulQNum == (ubKNL_GetStartPlyNum()-1))
			{
				ulMinusMs = 0;
			}
			else
			{
				ulMinusMs = ulDlyMs/2;
			}
			
			if(osMessages(KNL_AvgPlyQueue[3]))
			{
				osMessageGet(KNL_AvgPlyQueue[3], &tProc, osWaitForever);
				printd(DBG_Debug3Lvl, "AvgPlyQue->%d\r\n",tProc.ulIdx);			
				
				if(osMessagePutToFront(KNL_VdoCodecProcQueue, &tProc, 0) == osErrorResource)
				{		
					printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
				}			
				
				osDelay(ulDlyMs-ulMinusMs);				
			}
		}
	}
}

//------------------------------------------------------------------------------
static void KNL_VdoInProcThread(void const *argument)
{
	KNL_PROCESS tProc;

	while(1)
	{
		osMessageGet(KNL_VdoInProcQueue, &tProc, osWaitForever);
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_SEN_YUV_BUF:
				KNL_SenYuvBufProcess(tProc);
				break;
			default:
			#if OP_STA
				printd(DBG_ErrorLvl, "[%d]Node Err !\n", tProc.ubNextNode);
				SEN_SetFirstOutFlg(1);
				if(ubSEN_GetActiveFlg(SENSOR_PATH1))
				{
					BUF_Reset(BUF_SEN_1_YUV);
				}
				if(ubSEN_GetActiveFlg(SENSOR_PATH2))
				{
					BUF_Reset(BUF_SEN_2_YUV);
				}
				if(ubSEN_GetActiveFlg(SENSOR_PATH3))
				{
					BUF_Reset(BUF_SEN_3_YUV);
				}
				osMessageReset(KNL_VdoInProcQueue);
				SEN_SetFirstOutFlg(0);
			#endif
				break;
		}
	}
}

//------------------------------------------------------------------------------
static void KNL_VdoCodecProcThread(void const *argument)
{
	KNL_PROCESS tProc;

	while(1)
	{
		osMessageGet(KNL_VdoCodecProcQueue, &tProc, osWaitForever);
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_H264_ENC:
			case KNL_NODE_H264_DEC:
				KNL_H264CodecProcess(tProc);
				break;

			case KNL_NODE_JPG_DEC1:
				KNL_JpegDec1Process(tProc);
				break;

			case KNL_NODE_JPG_DEC2:
				KNL_JpegDec2Process(tProc);
				break;

			case KNL_NODE_JPG_ENC:
				KNL_JpegEncProcess(tProc);
				break;

			case KNL_NODE_IMG_MERGE_BUF:
				KNL_ImgMergeBufProcess(tProc);
				break;

			case KNL_NODE_VDO_BS_BUF1:
				KNL_VdoBsBuf1Process(tProc);
				break;

			case KNL_NODE_VDO_BS_BUF2:
				KNL_VdoBsBuf2Process(tProc);
				break;

			case KNL_NODE_IMG_MERGE_H:
				KNL_ImgMergeHProcess(tProc);
				break;

			case KNL_NODE_END:
				ubKNL_ReleaseBsBufAddr(KNL_NODE_END,tProc.ubSrcNum,tProc.ulDramAddr2);
				break;
		}
	}
}

//------------------------------------------------------------------------------
static void KNL_AdoCodecProcThread(void const *argument)
{
	KNL_PROCESS tProc;

	while(1)
	{
        osMessageGet(KNL_AdoCdoecProcQueue, &tProc, osWaitForever);
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_ADC_BUF:
				KNL_AdcBufProcess(tProc);
				break;

			case KNL_NODE_RETRY_ADC_BUF:
				KNL_RetryAdcBufProcess(tProc);
				break;

			case KNL_NODE_DAC_BUF:
				KNL_DacBufProcess(tProc);
				break;

			default:
				break;
		}
	}
}

//------------------------------------------------------------------------------
static void KNL_CommTxProcThread(void const *argument)
{
	KNL_PROCESS tProc;

	while(1)
	{
		osMessageGet(KNL_CommTxProcQueue, &tProc, osWaitForever);
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_COMM_TX_VDO:
				KNL_BbTxVdoProcess(tProc);
				break;
			default:
				break;
		}
	}
}

//------------------------------------------------------------------------------
uint8_t ubKNL_ReleaseBsBufAddr(uint8_t ubCurNode,uint8_t ubSrcNum,uint32_t ulBufAddr)
{
	KNL_NODE_INFO tNodeInfo;
	uint8_t ubTemp;

	osMutexWait(tKNL_BsBufMutex, osWaitForever);
	tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,ubCurNode);
	if(tNodeInfo.ubPreNode == KNL_NODE_VDO_BS_BUF1)
	{
		if(ubSrcNum == KNL_SRC_1_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs0Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs1Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs2Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs3Buf(ulBufAddr);
		}
		
		else if(ubSrcNum == KNL_SRC_1_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs0Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs1Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs2Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs3Buf(ulBufAddr);
		}
		
		else if(ubSrcNum == KNL_SRC_1_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs00Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs10Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs20Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs30Buf(ulBufAddr);
		}	
	}
	else if(tNodeInfo.ubNextNode == KNL_NODE_VDO_BS_BUF1)
	{
		if(ubSrcNum == KNL_SRC_1_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs0Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs1Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs2Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_MAIN)
		{
			ubTemp = ubBUF_ReleaseVdoMainBs3Buf(ulBufAddr);
		}
		
		else if(ubSrcNum == KNL_SRC_1_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs0Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs1Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs2Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_AUX)
		{
			ubTemp = ubBUF_ReleaseVdoAuxBs3Buf(ulBufAddr);
		}
		
		else if(ubSrcNum == KNL_SRC_1_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs00Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs10Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs20Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs30Buf(ulBufAddr);
		}	
	}
	else if(tNodeInfo.ubPreNode == KNL_NODE_VDO_BS_BUF2)
	{
		if(ubSrcNum == KNL_SRC_1_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs01Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_2_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs11Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_3_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs21Buf(ulBufAddr);
		}
		else if(ubSrcNum == KNL_SRC_4_SUB)
		{
			ubTemp = ubBUF_ReleaseVdoSubBs31Buf(ulBufAddr);
		}	
	}
	osMutexRelease(tKNL_BsBufMutex);
	return ubTemp;
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetBsBufAddr(uint8_t ubSrcNum)
{
	uint32_t ulTemp = BUF_FAIL;
	
	osMutexWait(tKNL_BsBufMutex, osWaitForever);
	if(ubSrcNum == KNL_SRC_1_MAIN)
	{
		ulTemp = ulBUF_GetVdoMainBs0FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err1 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_2_MAIN)
	{
		ulTemp = ulBUF_GetVdoMainBs1FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err2 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_3_MAIN)
	{
		ulTemp = ulBUF_GetVdoMainBs2FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err3 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_4_MAIN)
	{
		ulTemp = ulBUF_GetVdoMainBs3FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err4 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	
	else if(ubSrcNum == KNL_SRC_1_SUB)
	{
		ulTemp = ulBUF_GetVdoSubBs00FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err5 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_2_SUB)
	{
		ulTemp = ulBUF_GetVdoSubBs10FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err6 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_3_SUB)
	{
		ulTemp = ulBUF_GetVdoSubBs20FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err7 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_4_SUB)
	{
		ulTemp = ulBUF_GetVdoSubBs30FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err8 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_1_AUX)
	{
		ulTemp = ulBUF_GetVdoAuxBs0FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err1 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_2_AUX)
	{
		ulTemp = ulBUF_GetVdoAuxBs1FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//"Err2 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_3_AUX)
	{
		ulTemp = ulBUF_GetVdoAuxBs2FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err3 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	else if(ubSrcNum == KNL_SRC_4_AUX)
	{
		ulTemp = ulBUF_GetVdoAuxBs3FreeBuf();
		if(ulTemp == BUF_FAIL)
		{
			//printd(DBG_ErrorLvl, "Err4 @ulKNL_GetBsBufAddr\r\n");
		}
	}
	osMutexRelease(tKNL_BsBufMutex);	
	
	return ulTemp;
}

uint8_t ubKNL_GetRtCommLinkStatus(uint8_t ubRole)
{
	if(ubRole == KNL_STA1)
	{
		return ubKNL_RtLinkStatus[0];		
	}
	else if(ubRole == KNL_STA2)
	{
		return ubKNL_RtLinkStatus[1];		
	}
	else if(ubRole == KNL_STA3)
	{
		return ubKNL_RtLinkStatus[2];	
	}
	else if(ubRole == KNL_STA4)
	{
		return ubKNL_RtLinkStatus[3];	
	}
	else if(ubRole == KNL_SLAVE_AP)
	{
		return ubKNL_RtLinkStatus[4];	
	}
	else if(ubRole == KNL_MASTER_AP)
	{
		return ubKNL_RtLinkStatus[5];	
	}

	printd(DBG_ErrorLvl, "Err @ubKNL_GetRtCommLinkStatus\r\n");
	return 0;	
}

//------------------------------------------------------------------------------
uint8_t ubKNL_GetCommLinkStatus(uint8_t ubRole)
{
	uint8_t ubTemp;	

	osMutexWait(tKNL_LinkSem, osWaitForever);
	if(ubRole == KNL_STA1)
	{		
		ubTemp = ubKNL_LinkStatus[0];
	}
	else if(ubRole == KNL_STA2)
	{		
		ubTemp = ubKNL_LinkStatus[1];
	}
	else if(ubRole == KNL_STA3)
	{		
		ubTemp = ubKNL_LinkStatus[2];
	}
	else if(ubRole == KNL_STA4)
	{		
		ubTemp = ubKNL_LinkStatus[3];
	}
	else if(ubRole == KNL_SLAVE_AP)
	{		
		ubTemp = ubKNL_LinkStatus[4];
	}
	else if(ubRole == KNL_MASTER_AP)
	{		
		ubTemp = ubKNL_LinkStatus[5];
	}
	else
	{
		printd(DBG_ErrorLvl, "Err @ubKNL_GetCommLinkStatus\r\n");		
		ubTemp = 0;
	}	
	osMutexRelease(tKNL_LinkSem);
	
	return ubTemp;
}
//------------------------------------------------------------------------------
static void KNL_CommLinkMonitThread(void const *argument)
{
	LINK_REPORT tLinkRpt;
	uint8_t ubIdx;

	while(1)
	{
		//(1)Wait Event
        osMessageGet(KNL_LinkQ, &tLinkRpt, osWaitForever);
		if(tLinkRpt.tStatus == BB_LOST_LINK)
		{
			printd(DBG_CriticalLvl, "(RT)Lost[%d]\r\n",tLinkRpt.tRole);
			if(tLinkRpt.tRole == BB_STA1)
			{
				ubIdx = 0;
			}
			else if(tLinkRpt.tRole == BB_STA2)
			{
				ubIdx = 1;
			}
			else if(tLinkRpt.tRole == BB_STA3)
			{
				ubIdx = 2;
			}
			else if(tLinkRpt.tRole == BB_STA4)
			{
				ubIdx = 3;
			}
			else if(tLinkRpt.tRole == BB_SLAVE_AP)
			{
				ubIdx = 4;
			}
			else if(tLinkRpt.tRole == BB_MASTER_AP)
			{
				ubIdx = 5;
			}	
					
			ubKNL_RtLinkStatus[ubIdx] = BB_LOST_LINK;			
		}
		else if(tLinkRpt.tStatus == BB_LINK)
		{
			printd(DBG_CriticalLvl, "(RT)Link[%d]\r\n",tLinkRpt.tRole);
			
			if(tLinkRpt.tRole == BB_STA1)
			{
				ubIdx = 0;
			}
			else if(tLinkRpt.tRole == BB_STA2)
			{
				ubIdx = 1;
			}
			else if(tLinkRpt.tRole == BB_STA3)
			{
				ubIdx = 2;
			}
			else if(tLinkRpt.tRole == BB_STA4)
			{
				ubIdx = 3;
			}
			else if(tLinkRpt.tRole == BB_SLAVE_AP)
			{
				ubIdx = 4;
			}
			else if(tLinkRpt.tRole == BB_MASTER_AP)
			{
				ubIdx = 5;
			}			
			ubKNL_RtLinkStatus[ubIdx] = BB_LINK;
		}
	}
}
//------------------------------------------------------------------------------
static void KNL_CommLinkUpdateThread(void const *argument)
{
	KNL_PROCESS tProc;
	KNL_SRC tAdoSrcNum = KNL_SRC_NONE;
	uint32_t i,j,k;
	uint8_t ubLinkTemp[6][10];
	uint8_t ubSampleNum 	= 10;
	uint8_t ubLinkLoopCycle = 1;//10;
	uint8_t ubLinkTh    	= 1;//4;
	uint8_t ubLinkCnt   	= 0;
	uint16_t uwLinkPeriod	= 50;
	uint8_t ubSrcNumMap1,ubSrcNumMap2,ubSrcNumMap3;
	uint8_t ubKNL_SetLhFlag = FALSE;

	tProc = tProc;
	tAdoSrcNum = tAdoSrcNum;
	ubSrcNumMap1 = ubSrcNumMap1;
	ubSrcNumMap2 = ubSrcNumMap2;
	ubSrcNumMap3 = ubSrcNumMap3;
	ubKNL_SetLhFlag	= ubKNL_SetLhFlag;
	for(i=0;i<6;i++)
	{
		for(j=0;j<ubSampleNum;j++)
		{
			ubLinkTemp[i][j] = BB_LOST_LINK;
		}
	}
	i = 0;
	//===========================
	while(1)
	{
		ubLinkTemp[0][i%ubSampleNum] = ubKNL_RtLinkStatus[0];
		ubLinkTemp[1][i%ubSampleNum] = ubKNL_RtLinkStatus[1];
		ubLinkTemp[2][i%ubSampleNum] = ubKNL_RtLinkStatus[2];
		ubLinkTemp[3][i%ubSampleNum] = ubKNL_RtLinkStatus[3];
		ubLinkTemp[4][i%ubSampleNum] = ubKNL_RtLinkStatus[4];
		ubLinkTemp[5][i%ubSampleNum] = ubKNL_RtLinkStatus[5];
		if((++i%ubLinkLoopCycle) == 0)
		{
			for(k=0;k<6;k++)
			{
				ubLinkCnt = 0;
				for(j=0;j<ubSampleNum;j++)
				{
					if(ubLinkTemp[k][j])
					{
						ubLinkCnt++;
					}
					if(ubLinkCnt >= ubLinkTh)
					{
						break;
					}
				}

				if(ubLinkCnt >= ubLinkTh)
				{
					if(ubKNL_LinkStatus[k] == BB_LOST_LINK)
					{
						printd(DBG_CriticalLvl, "(STB)Link[%d]\r\n",k);
						
						if(TRUE == ubKNL_WakeUpFlag[k])
							KNL_WakeupDevice((KNL_ROLE)k, FALSE);

					#ifdef VBM_BU
						if(k == KNL_MASTER_AP)
						{
							if(ubRC_GetFlg(0))
							{
								printd(DBG_CriticalLvl, "Rst RC[0]\r\n");
								H264_RcSetEN((H264_ENCODE_INDEX)0,H264_ENABLE,CBR,ulRC_GetFinalBitRate(0));
							}
							if(ubRC_GetFlg(1))
							{
								printd(DBG_CriticalLvl, "Rst RC[1]\r\n");
								H264_RcSetEN((H264_ENCODE_INDEX)1,H264_ENABLE,CBR,ulRC_GetFinalBitRate(1));
							}
							if(ubRC_GetFlg(2))
							{
								printd(DBG_CriticalLvl, "Rst RC[2]\r\n");
								H264_RcSetEN((H264_ENCODE_INDEX)2,H264_ENABLE,CBR,ulRC_GetFinalBitRate(2));
							}
							if(ubRC_GetFlg(3))
							{
								printd(DBG_CriticalLvl, "Rst RC[3]\r\n");
								H264_RcSetEN((H264_ENCODE_INDEX)3,H264_ENABLE,CBR,ulRC_GetFinalBitRate(3));
							}
							if(ubRC_GetFlg(0)||ubRC_GetFlg(1)||ubRC_GetFlg(2)||ubRC_GetFlg(3))
							{
								if((ubKNL_GetVdoFps() <= 5) && (H264_GetCurrentQP() > 3))
								{
									H264_ResetRateControl(H264_GetCurrentQP() - 3);
								}
								else
								{
									H264_ResetRateControl(H264_GetCurrentQP());	
								}
							}
						}
//						if(ptKNL_AdoRoleSrcMapT)
//						{
//							tAdoSrcNum = ptKNL_AdoRoleSrcMapT((KNL_ROLE)ubKNL_GetRole());
//							KNL_AdoResume(tAdoSrcNum);
//						}
					#endif
					}
					if(FALSE == ubKNL_SetLhFlag)
					{
						ubLinkTh		= 4;
						ubLinkLoopCycle	= 10;
						uwLinkPeriod    = 100;
						ubKNL_SetLhFlag = TRUE;
					}
					#ifdef OP_STA
					if((ubKNL_SenStartFlg) && (ubSEN_GetFirstOutFlg()))
					{
						if(TRUE == ubKNL_ImgStabFlg)
						{
							SEN_SetFirstOutFlg(0);
						}
						else
						{
							ubLinkTh		= 1;
							ubLinkLoopCycle	= 1;
							uwLinkPeriod    = 50;
							ubKNL_SetLhFlag = FALSE;
						}
					}
					#endif
					ubKNL_LinkStatus[k] = BB_LINK;
				}
				else
				{
					if(ubKNL_LinkStatus[k] == BB_LINK)
					{
					#ifdef VBM_PU
						if(tKNL_GetPlyMode() == KNL_AVG_PLY)
						{
							if(k == 0)
							{
								ubSrcNumMap1 = KNL_SRC_1_MAIN;
								ubSrcNumMap2 = KNL_SRC_1_SUB;
								ubSrcNumMap3 = KNL_SRC_1_AUX;
							}
							else if(k == 1)
							{
								ubSrcNumMap1 = KNL_SRC_2_MAIN;
								ubSrcNumMap2 = KNL_SRC_2_SUB;
								ubSrcNumMap3 = KNL_SRC_2_AUX;
							}
							else if(k == 2)
							{
								ubSrcNumMap1 = KNL_SRC_3_MAIN;
								ubSrcNumMap2 = KNL_SRC_3_SUB;
								ubSrcNumMap3 = KNL_SRC_3_AUX;
							}
							else if(k == 3)
							{
								ubSrcNumMap1 = KNL_SRC_4_MAIN;
								ubSrcNumMap2 = KNL_SRC_4_SUB;
								ubSrcNumMap3 = KNL_SRC_4_AUX;
							}
							if((ubKNL_ChkVdoFlowAct(ubSrcNumMap1)||ubKNL_ChkVdoFlowAct(ubSrcNumMap2)||ubKNL_ChkVdoFlowAct(ubSrcNumMap3)))
							{
								printd(DBG_Debug3Lvl, "Rst AvgPly[%d]\r\n",k);
								ubKNL_AvgPlyStartFlg[k] = 0;
								while(1)
								{
									if(osMessages(KNL_AvgPlyQueue[k]))
									{
										osMessageGet(KNL_AvgPlyQueue[k], &tProc, 0);
										printd(DBG_Debug3Lvl, "SrcNum:%d _ BS_Adr:0x%x\r\n",tProc.ubSrcNum, tProc.ulDramAddr2);
										ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,tProc.ubSrcNum,tProc.ulDramAddr2);
									}
									else
									{
										printd(DBG_ErrorLvl, "Without AvgQ\r\n");
										break;
									}
								}
								osMessageReset(KNL_AvgPlyQueue[k]);
							}
						}
					#endif
						if(TRUE == ubKNL_SetLhFlag)
						{
							for(j=0;j<ubSampleNum;j++)
								ubLinkTemp[k][j] = BB_LOST_LINK;
							ubLinkTh		= 1;
							ubLinkLoopCycle	= 2;
							uwLinkPeriod    = 50;
							ubKNL_SetLhFlag = FALSE;
						}
						printd(DBG_CriticalLvl, "(STB)Lost[%d]\r\n",k);
					}
					ubKNL_LinkStatus[k] = BB_LOST_LINK;
				}
			}
			i = 0;
		}
		osDelay(uwLinkPeriod);
	}
}
//------------------------------------------------------------------------------
#if OP_AP
static void KNL_CommVdoRxMonitThread(void const *argument)
{
	KNL_PROCESS tKNLInfo;
	RX_DON Don;
	uint32_t ulTemp;
	uint8_t ubSrcNum;
	uint8_t ubSrcNumMap;
	uint8_t ubOpMode;
	uint8_t ubVdoGop;
	uint32_t ulGop;
	uint32_t ulCurFrmIdx[4];
	uint32_t ulPreFrmIdx[4];
	uint8_t ubPreVdoGop[4];

	uint8_t ubChkSeqFlg[4] = {0,0,0,0};
	uint8_t ubChkSeqOk[4] = {1,1,1,1};

	uint32_t ulChkSz;
	uint8_t ubOutSrc1;
	uint8_t ubOutSrc2;
	uint32_t ulOutAddr1;
	uint32_t ulOutAddr2;
	uint8_t ubMultiOutFlg = 0;
	KNL_NODE_INFO tNodeInfo;

	uint8_t ubCnt[4] = {0,0,0,0};
	uint32_t ulSum[4] = {0,0,0,0};

	ubOutSrc1 = ubOutSrc1;		//Avoid Warning
	ulOutAddr1 = ulOutAddr1;	//Avoid Warning
	ulGop = ulGop;

	ulCurFrmIdx[0] = 0;
	ulCurFrmIdx[1] = 0;
	ulCurFrmIdx[2] = 0;
	ulCurFrmIdx[3] = 0;

	ulPreFrmIdx[0] = 0;
	ulPreFrmIdx[1] = 0;
	ulPreFrmIdx[2] = 0;
	ulPreFrmIdx[3] = 0;

	while(1)
	{
		//(1)Wait Event
        osMessageGet(KNL_QueRxVdo, &Don, osWaitForever);
		if(/*Don.ubGetCrc != ubBB_GetCrcReport(Don.ulAddr,Don.ulCrcLen)*/0)
		{
			ubSrcNum 	= ubKNL_GetPktSrcNum(Don.ulAddr,Don.ulSize);
			ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
			if(ubSrcNumMap <= KNL_STA4)
			{
				ubKNL_RcvFirstIFrame[ubSrcNum] = 0;
				ubChkSeqFlg[ubSrcNumMap] = 0;
				tKNLInfo.ubSrcNum     = ubSrcNum;
				tKNLInfo.ubTargetRole = Don.tSTA;
				tKNLInfo.ubTwcCmd	  = TWC_RESEND_I;
				if(FALSE == ubKNL_VdoResendITwcFlg[tKNLInfo.ubTargetRole])
				{
					if(osMessagePut(KNL_TwcMonitQueue, &tKNLInfo, 0) == osErrorResource)
					{
						printd(DBG_ErrorLvl, "KNL_TwcMonitQ->Full !!!!\r\n");
					}
				}
			}
			else
			{
				if(FALSE == ubKNL_VdoChkSrcNumFlg[Don.tSTA])
					ubKNL_VdoChkSrcNumFlg[Don.tSTA] = TRUE;
			}
			printd(DBG_ErrorLvl, "(Case2)Frm Seq->Fail[%d][%d_%d]\r\n", ubSrcNum, Don.tSTA, Don.Type);
			BB_RxBufRelease(Don.Type,Don.tSTA);
		}
		else
		{
			KNL_FRAME_TYPE tKNL_RecvFrmType;
			uint16_t uwKNL_PktHSize = 0, uwKNL_PktVSize = 0;
			//uint8_t ubKNL_PaddLen = 0;

			//Reset the Status
			ubMultiOutFlg = 0;
			ulOutAddr1 = BUF_FAIL;
			ulOutAddr2 = BUF_FAIL;

			ubSrcNum = ubKNL_GetPktSrcNum(Don.ulAddr,Don.ulSize);
			ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
			ubOpMode = ubKNL_GetPktOpMode(Don.ulAddr,Don.ulSize);
			ulCurFrmIdx[ubSrcNumMap] = ulKNL_GetPktFrmIdx(Don.ulAddr,Don.ulSize);
			ulGop	 = ulKNL_GetPktGop(Don.ulAddr,Don.ulSize);
			ubVdoGop = ubKNL_GetPktVdoGop(Don.ulAddr,Don.ulSize);

			if((ubOpMode != ubKNL_GetOpMode() || (ubSrcNumMap > KNL_STA4)))
				goto RX_VDORES_ERR;

			//ubKNL_PaddLen  = ubKNL_GetPaddLen(Don.ulAddr,Don.ulSize);
			uwKNL_PktHSize = ubKNL_GetPktHRes(Don.ulAddr,Don.ulSize);
			uwKNL_PktVSize = ubKNL_GetPktVRes(Don.ulAddr,Don.ulSize);
			if((uwKNL_PktHSize != uwKNL_GetVdoH(ubSrcNum)) || (uwKNL_PktVSize != uwKNL_GetVdoV(ubSrcNum)))
			{
				ubKNL_RcvFirstIFrame[ubSrcNum] = 0;
				ubChkSeqFlg[ubSrcNumMap] = 0;
				tKNLInfo.ubSrcNum     = ubSrcNum;
				tKNLInfo.ubTargetRole = Don.tSTA;
				tKNLInfo.ubTwcCmd	  = TWC_VDORES_SETTING;
				if((ubSrcNumMap <= KNL_STA4) && (FALSE == ubKNL_VdoResChgTwcFlg[ubSrcNumMap]))
				{
					if(osMessagePut(KNL_TwcMonitQueue, &tKNLInfo, 0) == osErrorResource)
					{
						printd(DBG_ErrorLvl, "KNL_TwcMonitQ->Full !!!!\r\n");
					}
					printd(DBG_CriticalLvl, "Res Chg Case[%d]\r\n",ubSrcNum);
				}
				goto RX_VDORES_ERR;
			}

			ubKNL_VdoResChgTwcFlg[ubSrcNumMap] = FALSE;
			tKNL_RecvFrmType = tKNL_GetFrameType(Don.ulAddr);
			//ulKNL_InVdoFpsCntTemp[ubSrcNum]++;
			//printd(DBG_Debug2Lvl, "V[%d]:0x%x_M:%d_I:%d_G:%d\r\n",ubSrcNum,Don.ulSize,ubOpMode,ulCurFrmIdx[ubSrcNum],ulGop);
			printd(DBG_Debug2Lvl, "V[%d]:0x%x_M:%d_I:%d_G:%d_VG:%d\r\n",ubSrcNum,Don.ulSize,ubOpMode,ulCurFrmIdx[ubSrcNum],ulGop,ubVdoGop);

			//Seq Check
			//-------------------------------------------------------------------
			if(ubChkSeqFlg[ubSrcNumMap])
			{
				if((ulCurFrmIdx[ubSrcNumMap] != 0) && (ulCurFrmIdx[ubSrcNumMap] != (ulPreFrmIdx[ubSrcNumMap]+1)) && (ubVdoGop == ubPreVdoGop[ubSrcNumMap]))
				{
					ubChkSeqOk[ubSrcNumMap] = 0;
				}
				else if((ulCurFrmIdx[ubSrcNumMap] != 0) && (ulCurFrmIdx[ubSrcNumMap] == (ulPreFrmIdx[ubSrcNumMap]+1)) && (ubVdoGop != ubPreVdoGop[ubSrcNumMap]))
				{
					ubChkSeqOk[ubSrcNumMap] = 2;
				}
				else if((ulCurFrmIdx[ubSrcNum] == 0) && (tKNL_RecvFrmType == KNL_I_FRAME) && (ubKNL_RcvFirstIFrame[ubSrcNum]))
				{
					ubKNL_RcvFirstIFrame[ubSrcNum] = 0;
				}
				else
				{
					ubChkSeqOk[ubSrcNumMap] = 1;
				}
			}
			ulPreFrmIdx[ubSrcNumMap] = ulCurFrmIdx[ubSrcNumMap];

			if(ubKNL_ChkVdoFlowAct(ubSrcNum) && (ubKNL_RcvFirstIFrame[ubSrcNum] == 0))
			{
				if(tKNL_RecvFrmType == KNL_I_FRAME)
				{
					ubKNL_VdoResendITwcFlg[ubSrcNumMap] = FALSE;
					ubKNL_RcvFirstIFrame[ubSrcNum] = 1;
					printd(DBG_CriticalLvl, "Find I[%d]\r\n",ubSrcNum);
					if(TRUE == ubKNL_WakeUpFlag[ubSrcNumMap])
						KNL_WakeupDevice((KNL_ROLE)ubSrcNumMap, FALSE);

					ubChkSeqFlg[ubSrcNumMap] = 1;
					ubChkSeqOk[ubSrcNumMap]  = 1;
					ubPreVdoGop[ubSrcNumMap] = ubVdoGop;
					ulCurFrmIdx[ubSrcNumMap] = 0;
					ulPreFrmIdx[ubSrcNumMap] = 0;

					ubCnt[ubSrcNumMap] = 0;
					ulSum[ubSrcNumMap] = 0;

					ubKNL_AvgPlyStartFlg[ubSrcNumMap] = 0;
					ubKNL_AvgPlyCnt[ubSrcNumMap] = 0;

					ulKNL_InVdoFpsCnt[ubSrcNum] = ulKNL_InVdoFpsCntTemp[ubSrcNum];
					ulKNL_InVdoFpsCntTemp[ubSrcNum] = 0;
				}
			}
			else if(ubKNL_ChkVdoFlowAct(ubSrcNum) && ubKNL_RcvFirstIFrame[ubSrcNum] && (ubChkSeqOk[ubSrcNumMap] == 0))
			{
				printd(DBG_ErrorLvl, "(Case1)Frm Seq->Fail[%d]\r\n",ubSrcNum);
				if(ubSrcNumMap <= KNL_STA4)
				{
					ubKNL_RcvFirstIFrame[ubSrcNum] = 0;
					ubChkSeqFlg[ubSrcNumMap] = 0;
					tKNLInfo.ubSrcNum     = ubSrcNum;
					tKNLInfo.ubTargetRole = Don.tSTA;
					tKNLInfo.ubTwcCmd	  = TWC_RESEND_I;
					if(FALSE == ubKNL_VdoResendITwcFlg[tKNLInfo.ubTargetRole])
					{
						if(osMessagePut(KNL_TwcMonitQueue, &tKNLInfo, 0) == osErrorResource)
						{
							printd(DBG_ErrorLvl, "KNL_TwcMonitQ->Full !!!!\r\n");
						}
					}
				}
				goto RX_VDORES_ERR;
			}
			else if(ubKNL_ChkVdoFlowAct(ubSrcNum) && ubKNL_RcvFirstIFrame[ubSrcNum] && (ubChkSeqOk[ubSrcNumMap] == 2))
			{
				ubPreVdoGop[ubSrcNumMap] = ubVdoGop;
			}

			if(ubKNL_ChkVdoFlowAct(ubSrcNum) && ubKNL_RcvFirstIFrame[ubSrcNum] && ubChkSeqOk[ubSrcNumMap])
			{
				if(!ubKNL_AvgPlyStartFlg[ubSrcNumMap])
				{
					ubKNL_AvgPlyCnt[ubSrcNumMap]++;
				}

				ulKNL_InVdoFpsCntTemp[ubSrcNum]++;
				ulSum[ubSrcNumMap] = ulSum[ubSrcNumMap] + Don.ulSize;
				ubCnt[ubSrcNumMap]++;

				if(ubCnt[ubSrcNumMap] == ubKNL_GetVdoFps())
				{
					printd(DBG_CriticalLvl, "TB[%d]:%d\r\n",ubSrcNum,ulSum[ubSrcNumMap]/1024);
					ulKNL_InVdoFpsCnt[ubSrcNum] = ulKNL_InVdoFpsCntTemp[ubSrcNum];
					ulKNL_InVdoFpsCntTemp[ubSrcNum] = 0;
					ubCnt[ubSrcNumMap] = 0;
					ulSum[ubSrcNumMap] = 0;
				}

				//(2)Request Buffer to Temp
				ulTemp = ulKNL_GetBsBufAddr(ubSrcNum);
				if(ulTemp != BUF_FAIL)
				{
					DMAC_RESULT tDmaResult = DMAC_OK;

					if(Don.tSTA <= KNL_STA4)
						ubKNL_VdoBsBusyCnt[Don.tSTA] = 0;
					//(3)Check MultiOut Node
					if(ubKNL_ChkMultiOutNode(KNL_NODE_COMM_RX_VDO))
					{
						if(ubKNL_GetMultiInSrc(KNL_NODE_COMM_RX_VDO) == ubSrcNum)
						{
							ubMultiOutFlg = 1;
							ubOutSrc1 = ubKNL_GetMultiOutSrc(KNL_NODE_COMM_RX_VDO,0);
							ubOutSrc2 = ubKNL_GetMultiOutSrc(KNL_NODE_COMM_RX_VDO,1);
							if(ubOutSrc2 == KNL_SRC_1_AUX)
							{
								ulOutAddr2 = ulBUF_GetVdoAuxBs0FreeBuf();
								if(ulOutAddr2 == BUF_FAIL)
								{
									printd(DBG_ErrorLvl, "Err9 @%s\n", __func__);
								}
							}
							if(ubOutSrc2 == KNL_SRC_2_AUX)
							{
								ulOutAddr2 = ulBUF_GetVdoAuxBs1FreeBuf();
								if(ulOutAddr2 == BUF_FAIL)
								{
									printd(DBG_ErrorLvl, "Err10 @%s\n", __func__);
								}
							}
							if(ubOutSrc2 == KNL_SRC_3_AUX)
							{
								ulOutAddr2 = ulBUF_GetVdoAuxBs2FreeBuf();
								if(ulOutAddr2 == BUF_FAIL)
								{
									printd(DBG_ErrorLvl, "Err11 @%s\n", __func__);
								}
							}
							if(ubOutSrc2 == KNL_SRC_4_AUX)
							{
								ulOutAddr2 = ulBUF_GetVdoAuxBs3FreeBuf();
								if(ulOutAddr2 == BUF_FAIL)
								{
									printd(DBG_ErrorLvl, "Err12 @%s\n", __func__);
								}
							}
						}
					}

					//(4)Copy BB's Buffer to Kernel
					//Copy Data to Src1
					tDmaResult = tDMAC_MemCopy(Don.ulAddr, ulTemp, Don.ulSize, NULL);
					if(DMAC_OK == tDmaResult)
					{
						ulChkSz = Don.ulSize;
						//(5)Get Source Side Information
						tKNLInfo.ubSrcNum	= ubSrcNum;

						//(6)Release BB Buffer
						BB_RxBufRelease(Don.Type,Don.tSTA);	

						if(ubKNL_ChkDebugPkt(ulTemp,ulChkSz) == 1)
						{
							//MultiOut Process
							if(ubMultiOutFlg)
							{
								//Copy Data to Src2
								tDMAC_MemCopy(ulTemp,ulOutAddr2,ulChkSz,NULL);
							}

							//printd(DBG_Debug3Lvl, "CRC->P\r\n");

							//(7)Send Queue to Next Node
							if(ubMultiOutFlg == 0)	//Single Out Case
							{
								if(tKNL_GetPlyMode() == KNL_NORMAL_PLY)
								{
									tKNLInfo.ubSrcNum       = ubSrcNum;
									tKNLInfo.ulDramAddr2    = ulTemp;
									tKNLInfo.ubCurNode      = KNL_NODE_COMM_RX_VDO;
									tKNLInfo.ubNextNode     = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_COMM_RX_VDO);
									if(osMessagePut(KNL_VdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
									{
										ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, tKNLInfo.ulDramAddr2);
										printd(DBG_ErrorLvl, "KNL_Q->Full !!!\r\n");
									}
								}
								else if(tKNL_GetPlyMode() == KNL_AVG_PLY)
								{
									if((ubKNL_AvgPlyCnt[ubSrcNumMap] >= ubKNL_GetStartPlyNum()) && (!ubKNL_AvgPlyStartFlg[ubSrcNumMap]))
									{
										ubKNL_AvgPlyStartFlg[ubSrcNumMap] = 1;
									}
									tKNLInfo.ubSrcNum       = ubSrcNumMap;
									tKNLInfo.ulDramAddr2    = ulTemp;
									tKNLInfo.ubCurNode      = KNL_NODE_COMM_RX_VDO;
									tKNLInfo.ubNextNode     = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_COMM_RX_VDO);
									if(osMessagePut(KNL_AvgPlyQueue[ubSrcNumMap], &tKNLInfo, 0) == osErrorResource)
									{
										ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, tKNLInfo.ulDramAddr2);
										printd(DBG_ErrorLvl, "KNL_Q->Full !!!\r\n");
									}
								}
							}
							else if(ubMultiOutFlg)	//Multiple Out Case
							{
								tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,KNL_NODE_COMM_RX_VDO);
								tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,tNodeInfo.ubNextNode);
								if((tNodeInfo.ubNextNode == KNL_NODE_NONE)||(tNodeInfo.ubNextNode == KNL_NODE_END))
								{
									ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, ulTemp);
								}
								else
								{
									//Src1
									tKNLInfo.ubSrcNum       = ubSrcNum;
									tKNLInfo.ulDramAddr2    = ulTemp;
									tKNLInfo.ubCurNode      = KNL_NODE_COMM_RX_VDO;
									tKNLInfo.ubNextNode     = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_COMM_RX_VDO);
									if(osMessagePut(KNL_VdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
									{
										ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, tKNLInfo.ulDramAddr2);
										printd(DBG_ErrorLvl, "KNL_Q->Full !!!\r\n");
									}
								}

								//Src2
								tKNLInfo.ubSrcNum       = ubOutSrc2;
								tKNLInfo.ulDramAddr2    = ulOutAddr2;
								tKNLInfo.ubCurNode      = KNL_NODE_COMM_RX_VDO;
								tKNLInfo.ubNextNode     = ubKNL_GetNextNode(ubOutSrc2,KNL_NODE_COMM_RX_VDO);
								if(osMessagePut(KNL_VdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
								{
									ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, tKNLInfo.ulDramAddr2);
									printd(DBG_ErrorLvl, "KNL_Q->Full !!!\r\n");
								}
							}
						}
						else
						{
							ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, ulTemp);
							printd(DBG_ErrorLvl, "Err13 @%s\n", __func__);
							//while(1);
						}
					}
					else
					{
						ubKNL_ReleaseBsBufAddr(KNL_NODE_COMM_RX_VDO, ubSrcNum, ulTemp);
						BB_RxBufRelease(Don.Type,Don.tSTA);
						printd(DBG_ErrorLvl, "DMA NRDY @%s !\n", __func__);
					}
				}
				else
				{
					printd(DBG_ErrorLvl, "Busy[%d]:%d\r\n", Don.tSTA, ubSrcNum);
					//(6)Release BB Buffer
					BB_RxBufRelease(Don.Type, Don.tSTA);
					if(Don.tSTA <= KNL_STA4)
						ubKNL_VdoBsBusyCnt[Don.tSTA]++;
				}
			}
			else
			{
				if((ubKNL_ChkVdoFlowAct(ubSrcNum)) && (ubSrcNumMap <= KNL_STA4))
				{
					ubKNL_RcvFirstIFrame[ubSrcNum] 	= 0;
					ubChkSeqFlg[ubSrcNumMap] 		= 0;
					tKNLInfo.ubSrcNum     = ubSrcNum;
					tKNLInfo.ubTargetRole = Don.tSTA;
					tKNLInfo.ubTwcCmd	  = TWC_RESEND_I;
					if(FALSE == ubKNL_VdoResendITwcFlg[tKNLInfo.ubTargetRole])
					{
						if(osMessagePut(KNL_TwcMonitQueue, &tKNLInfo, 0) == osErrorResource)
						{
							printd(DBG_ErrorLvl, "KNL_TwcMonitQ->Full !!!!\r\n");
						}
					}
				}
RX_VDORES_ERR:
				printd(DBG_Debug2Lvl, "R[%d]: %d_%d_%d\r\n", ubSrcNum, ubKNL_ChkVdoFlowAct(ubSrcNum), ubOpMode, ubKNL_GetOpMode());
				BB_RxBufRelease(Don.Type,Don.tSTA);
			}
		}
	}
}
#endif
//------------------------------------------------------------------------------
static void KNL_CommAdoRxMonitThread(void const *argument)
{		
	KNL_PROCESS tKNLInfo;
	RX_DON Don;
	uint32_t ulTemp;		
	uint8_t ubIdx;	
	uint32_t ulRealSz;

	// ADO
	//=================================
	//| STA1       -> KNL_SRC_1_MAIN  |
	//| STA2       -> KNL_SRC_2_MAIN  |
	//| STA3       -> KNL_SRC_3_MAIN  |
	//| STA4       -> KNL_SRC_4_MAIN  |
	//| AP(Master) -> KNL_SRC_1_AUX   |
	//| AP(Slave)  -> KNL_SRC_2_AUX   |
	//=================================	

	while(1)
	{
		//(1)Wait Event
		osMessageGet(KNL_QueRxAdo, &Don, osWaitForever);
		//printd(DBG_Debug3Lvl, "A[%d]:0x%x\r\n",Don.tSTA,Don.ulSize);

        if( ADO_GetIpReadyStatus() == ADO_IP_READY )
        {
            ubIdx = ubKNL_GetPktSrcNum(Don.ulAddr,Don.ulSize);

            if(ubKNL_ChkAdoFlowAct(ubIdx))
            {
                //(2)Request Buffer to Temp
                if(ubKNL_SrcNumMap(ubIdx) == 0)
                {
                    ulTemp = ulBUF_GetDac0FreeBuf();
                }
                else if(ubKNL_SrcNumMap(ubIdx) == 1)
                {
                    ulTemp = ulBUF_GetDac1FreeBuf();
                }
                else if(ubKNL_SrcNumMap(ubIdx) == 2)
                {
                    ulTemp = ulBUF_GetDac2FreeBuf();
                }
                else if(ubKNL_SrcNumMap(ubIdx) == 3)
                {
                    ulTemp = ulBUF_GetDac3FreeBuf();
                }
				else
				{
					ulTemp = BUF_FAIL;
				}

                if(ulTemp == BUF_FAIL)
                {
                    printd(DBG_ErrorLvl, "[%d]BUF_ADO Err !!!\r\n", ubIdx);
					BB_RxBufRelease(Don.Type,Don.tSTA);
                }
                else
                {
					DMAC_RESULT tDmaResult = DMAC_OK;
					uint8_t ubAdoSizeErrFlag = FALSE;

                    tDmaResult = tDMAC_MemCopy((uint32_t)Don.ulAddr,(uint32_t)ulTemp,Don.ulSize,NULL);
					if(DMAC_OK == tDmaResult)
					{
						//(4-1)Check Real-Size Information
						ulRealSz = ulKNL_GetPktSZ(ulTemp,Don.ulSize);
						//printd(DBG_Debug3Lvl, "Real-Sz:0x%x\r\n",ulRealSz);

						if(ulRealSz < 0x400)
						{
							//(5)Send Queue to Next Node
							tKNLInfo.ubSrcNum    = ubIdx;
							tKNLInfo.ulDramAddr2 = ulTemp;
							tKNLInfo.ulSize		 = ulRealSz;
							tKNLInfo.ubCurNode   = KNL_NODE_COMM_RX_ADO;
							tKNLInfo.ubNextNode  = ubKNL_GetNextNode(ubIdx,KNL_NODE_COMM_RX_ADO);
							KNL_DacBufProcess(tKNLInfo);
//							if(osMessagePut(KNL_AdoCdoecProcQueue, &tKNLInfo, 0) == osErrorResource)
//							{
//								ubBUF_ReleaseDac0Buf(tKNLInfo.ulDramAddr2);
//								printd(DBG_ErrorLvl, "KNL_ADO Q->Full !!!\r\n");
//							}
						}
						else
							ubAdoSizeErrFlag = TRUE;
					}

					//(4-2)Release BB Buffer
					BB_RxBufRelease(Don.Type,Don.tSTA);

					if((DMAC_OK != tDmaResult) || (TRUE == ubAdoSizeErrFlag))
					{
						if(ubKNL_SrcNumMap(ubIdx) == 0)
						{
							ubBUF_ReleaseDac0Buf(ulTemp);
						}
						else if(ubKNL_SrcNumMap(ubIdx) == 1)
						{
							ubBUF_ReleaseDac1Buf(ulTemp);
						}
						else if(ubKNL_SrcNumMap(ubIdx) == 2)
						{
							ubBUF_ReleaseDac2Buf(ulTemp);
						}
						else if(ubKNL_SrcNumMap(ubIdx) == 3)
						{
							ubBUF_ReleaseDac3Buf(ulTemp);
						}
						if(DMAC_OK != tDmaResult)
							printd(DBG_ErrorLvl, "DMA NRDY @%s\n", __func__);
						if(TRUE == ubAdoSizeErrFlag)
							printd(DBG_ErrorLvl, "ADO Size: 0x%X\n", ulRealSz);
					}
                }
            }
            else
            {
                BB_RxBufRelease(Don.Type,Don.tSTA);
            }
        }
        else
        {
            BB_RxBufRelease(Don.Type,Don.tSTA);
        }
	}
}
//------------------------------------------------------------------------------
static void KNL_JpegMonitThread(void const *argument)
{		
	KNL_PROCESS tJpegMonitProc;		
	JPEG_CODEC_INFO_t tJpegInfo;
	uint8_t ubNextNode;	
	uint8_t ubInfo_PreNode;
	uint8_t ubInfo_PreSrc;
	uint8_t ubInfo_Action;
	uint32_t ulInfo_YuvAddr;
	uint32_t ulInfo_BsAddr;
	uint32_t ulInfo_Size;	
	
	while(1)
	{
        osMessageGet(KNL_QueueJpegMonit, &tJpegInfo, osWaitForever);
		
		//(1)Update Information First
		ubInfo_PreNode 		= ubKNL_JpegPreNode;
		ubInfo_PreSrc		= ubKNL_JpegSrc;
		ubInfo_Action		= tJpegInfo.tJPEG_CodecMode;
		ulInfo_YuvAddr		= tJpegInfo.ulJPEG_YUVAddr;
		ulInfo_BsAddr		= tJpegInfo.ulJPEG_BsAddr;
		ulInfo_Size			= tJpegInfo.ulJPEG_BsSize;
		
		//(2)Release JPEG Codec
		osSemaphoreRelease(JPEG_CodecSem);

		/*
		printd(DBG_Debug3Lvl, "JPEG_Action:%d\r\n",ubInfo_Action);
		printd(DBG_Debug3Lvl, "JPEGc_YuvAddr:0x%x\r\n",ulInfo_YuvAddr);
		printd(DBG_Debug3Lvl, "JPEG_BsAddr:0x%x\r\n",ulInfo_BsAddr);
		printd(DBG_Debug3Lvl, "JPEG_Size:0x%x\r\n",ulInfo_Size);		
		*/
		if((ubInfo_Action == 1) && (ubInfo_PreNode == KNL_NODE_JPG_DEC1))
		{
			KNL_SetNodeState(ubInfo_PreSrc,KNL_NODE_JPG_DEC1,KNL_NODE_STOP);			
			printd(DBG_Debug3Lvl, "J->D1[%d]:0x%x_0x%x_0x%x\r\n",ubInfo_PreSrc,ulInfo_YuvAddr,ulInfo_BsAddr,ulInfo_Size);
		}
		else if((ubInfo_Action == 1) && (ubInfo_PreNode == KNL_NODE_JPG_DEC2))
		{
			KNL_SetNodeState(ubInfo_PreSrc,KNL_NODE_JPG_DEC2,KNL_NODE_STOP);
			printd(DBG_Debug3Lvl, "J->D2[%d]:0x%x_0x%x_0x%x\r\n",ubInfo_PreSrc,ulInfo_YuvAddr,ulInfo_BsAddr,ulInfo_Size);
		}
		else if((ubInfo_Action == 0) && (ubInfo_PreNode == KNL_NODE_JPG_ENC))
		{
			KNL_SetNodeState(ubInfo_PreSrc,KNL_NODE_JPG_ENC,KNL_NODE_STOP);
			printd(DBG_Debug3Lvl, "J->E[%d]:0x%x_0x%x_0x%x\r\n",ubInfo_PreSrc,ulInfo_YuvAddr,ulInfo_BsAddr,ulInfo_Size);
		}
		
		//(3)Release BS Buffer
		if((ubInfo_PreNode == KNL_NODE_JPG_DEC1) || (ubInfo_PreNode == KNL_NODE_JPG_DEC2))
		{			
			ubKNL_ReleaseBsBufAddr(ubInfo_PreNode,ubInfo_PreSrc,ulInfo_BsAddr);
		}
		
		//(4)To Next Node
		if(ubKNL_ChkVdoFlowAct(ubInfo_PreSrc))
		{
			ubNextNode = ubKNL_GetNextNode(ubInfo_PreSrc,ubInfo_PreNode);
			if(ubNextNode == KNL_NODE_LCD)
			{
				KNL_ActiveLcdDispBuf(ubInfo_PreSrc);
			}			
			else
			{
				//Next Node
				tJpegMonitProc.ubSrcNum     = ubInfo_PreSrc;
				tJpegMonitProc.ubCurNode    = ubInfo_PreNode;			
				tJpegMonitProc.ubNextNode   = ubKNL_GetNextNode(ubInfo_PreSrc,ubInfo_PreNode);	
				tJpegMonitProc.ulDramAddr1  = ulInfo_YuvAddr;			
				tJpegMonitProc.ulDramAddr2  = ulInfo_BsAddr;
                if(osMessagePut(KNL_VdoCodecProcQueue, &tJpegMonitProc, 0) == osErrorResource)
				{		
					printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void KNL_ResetDecoder(uint8_t ubSrcNum)
{
	KNL_NODE_INFO tNodeInfo;

	osMessageReset(KNL_VdoCodecProcQueue);
	IMG_ClearImgMsgQueue();
	if(ubKNL_ExistNode(ubSrcNum, KNL_NODE_H264_DEC))
	{
		tNodeInfo = tKNL_GetNodeInfo(ubSrcNum, KNL_NODE_H264_DEC);
		if(tNodeInfo.ubCodecIdx == DECODE_0)
		{
			KNL_ImgDecInit(DECODE_0,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
		}
		else if(tNodeInfo.ubCodecIdx == DECODE_1)
		{
			KNL_ImgDecInit(DECODE_1,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
		}
		else if(tNodeInfo.ubCodecIdx == DECODE_2)
		{
			KNL_ImgDecInit(DECODE_2,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
		}
		else if(tNodeInfo.ubCodecIdx == DECODE_3)
		{
			KNL_ImgDecInit(DECODE_3,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV);
		}
	}
}

//------------------------------------------------------------------------------
void KNL_RecoveryImgCodec(void)
{
	KNL_ROLE tKNL_Role;
	KNL_SRC tSrcNum = KNL_SRC_NONE;

	H264_Reset();
	for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
	{
		tSrcNum = (KNL_SRC)(KNL_SRC_1_MAIN + tKNL_Role);
		ubKNL_VdoFlowActiveFlg[tSrcNum] = 0;
		KNL_VdoSuspend(tSrcNum);
		BUF_Reset((BUF_VDO_MAIN_BS0 + tKNL_Role));
		KNL_ResetDecoder(tSrcNum);
	}
	osSemaphoreRelease(tKNL_ImgSem);
	ubKNL_ImgRdy = 1;
	for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
	{
		tSrcNum = (KNL_SRC)(KNL_SRC_1_MAIN + tKNL_Role);
		KNL_VdoResume(tSrcNum);
		ubKNL_VdoFlowActiveFlg[tSrcNum] = 1;
	}
}

//------------------------------------------------------------------------------
void KNL_RestartImgCodec(void)
{
	KNL_ROLE tKNL_Role;
	KNL_SRC tSrcNum = KNL_SRC_NONE;

	H264_Reset();
	for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
	{
		tSrcNum = (KNL_SRC)(KNL_SRC_1_MAIN + tKNL_Role);
		KNL_VdoStop(tSrcNum);
		ubKNL_WaitNodeFinish(tSrcNum);
//		BUF_Reset((BUF_VDO_MAIN_BS0 + tKNL_Role));
		KNL_ResetDecoder(tSrcNum);
	}
	for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
		KNL_VdoStart((KNL_SRC_1_MAIN + tKNL_Role));
}

//------------------------------------------------------------------------------
void KNL_RestartDataPath(uint8_t ubRole)
{
	KNL_SRC tSrcNum = KNL_SRC_NONE;

	if(ptKNL_VdoRoleSrcMapT)
		tSrcNum = ptKNL_VdoRoleSrcMapT((KNL_ROLE)ubRole);
	if(KNL_SRC_NONE != tSrcNum)
	{
		KNL_VdoStop(tSrcNum);
		ubKNL_WaitNodeFinish(tSrcNum);
		if(TRUE == ubKNL_WakeUpFlag[ubRole])
			KNL_WakeupDevice((KNL_ROLE)ubRole, FALSE);
		osDelay(200);
//		BUF_Reset((BUF_VDO_MAIN_BS0 + ubRole));
		KNL_VdoStart(tSrcNum);
	}
}

//------------------------------------------------------------------------------
static void KNL_SysMonitThread(void const *argument)
{
#if OP_AP
	KNL_ROLE tKNL_Role;
	uint8_t ubSysErrFlg1 = FALSE;
	uint8_t ubSysIChkCnt[4] = {0, 0, 0, 0};
	uint8_t ubSysVResChkCnt[4] = {0, 0, 0, 0};
	#if KNL_RFPWR_CTRL_ENABLE
	uint8_t ubSysLoop = 0, ubSysLoopNum = 20;
	uint8_t ubSysPowerMap[2] = {0x37, 0x2C};
	uint8_t ubSysPowerIdx = 0, ubSysPowerDectFlag = FALSE;
	#endif
#endif

	while(1)
	{
#if OP_AP
		ubSysErrFlg1 = FALSE;
		for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
		{
			if(TRUE == ubKNL_VdoChkSrcNumFlg[tKNL_Role])
				ubSysErrFlg1 = TRUE;
			if(ubKNL_VdoBsBusyCnt[tKNL_Role] >= 2)
			{
				printd(DBG_Debug1Lvl, "BS Busy[%d]", tKNL_Role);
				KNL_RestartDataPath(tKNL_Role);
				ubKNL_VdoBsBusyCnt[tKNL_Role] = 0;
			}
			if(TRUE == ubKNL_VdoResendITwcFlg[tKNL_Role])
			{
				if(++ubSysIChkCnt[tKNL_Role] >= 2)
				{
					ubKNL_VdoResendITwcFlg[tKNL_Role] = FALSE;
					ubSysIChkCnt[tKNL_Role] = 0;
				}
			}
			else
				ubSysIChkCnt[tKNL_Role] = 0;
			if(TRUE == ubKNL_VdoResChgTwcFlg[tKNL_Role])
			{
				if(++ubSysVResChkCnt[tKNL_Role] >= 2)
				{
					ubKNL_VdoResChgTwcFlg[tKNL_Role] = FALSE;
					ubSysVResChkCnt[tKNL_Role] = 0;
				}
			} 
			else
				ubSysVResChkCnt[tKNL_Role] = 0;
		}
		if((TRUE == ubKNL_ImgBusyFlg))
		{
			printd(DBG_Debug1Lvl, "Decode Busy !\n");
			KNL_RecoveryImgCodec();
			if(TRUE == ubKNL_ImgBusyFlg)
				ubKNL_ImgBusyFlg = FALSE;
		}
		if(TRUE == ubSysErrFlg1)
		{
			for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
			{
				if(TRUE == ubKNL_VdoChkSrcNumFlg[tKNL_Role])
				{
					KNL_RestartDataPath(tKNL_Role);
					ubKNL_VdoChkSrcNumFlg[tKNL_Role] = FALSE;
				}
			}
		}
		#if KNL_RFPWR_CTRL_ENABLE
		if(!(ubSysLoop % ubSysLoopNum))
		{
			KNL_ROLE tSysMaxRole;
			uint8_t ubSysPer[4] = {100, 100, 100, 100}, ubSysPowerFlag = FALSE;

			if(FALSE == ubSysPowerDectFlag)
			{
				ubSysPowerDectFlag = TRUE;
				ubSysLoopNum = 5;
			}
			tSysMaxRole = (tKNL_Info.ubOpMode == KNL_OPMODE_VBM_4T)?KNL_STA4:
					      (tKNL_Info.ubOpMode == KNL_OPMODE_VBM_2T)?KNL_STA2:KNL_STA1;
			BB_SetTxPwr(0, 0x37);
			for(tKNL_Role = KNL_STA1; tKNL_Role <= tSysMaxRole; tKNL_Role++)
			{
				ubSysPer[tKNL_Role] = KNL_GetPerValue(tKNL_Role);
				if((ubSysPer[tKNL_Role] <= 50) && (ubSysPowerIdx))
				{
					ubSysPowerFlag = TRUE;
					ubSysPowerIdx  = 0;
					BB_SetTxPwr(1, ubSysPowerMap[ubSysPowerIdx]);
					break;
				}
			}
			if((FALSE == ubSysPowerFlag) && (!ubSysPowerIdx))
			{
				ubSysPowerIdx = 1;
				BB_SetTxPwr(1, ubSysPowerMap[ubSysPowerIdx]);
			}
			printd(DBG_CriticalLvl, "PER: %d:%d:%d:%d\n", ubSysPer[KNL_STA1], ubSysPer[KNL_STA2], ubSysPer[KNL_STA3], ubSysPer[KNL_STA4]);
		}
		#endif
		osDelay(200);
#else
		osDelay(5000);
#endif
	}
}

//------------------------------------------------------------------------------
static void KNL_BbFrmMonitThread(void const *argument)
{
	uint8_t ubBbFrmStatus;
	KNL_ROLE tKNL_LinkRole[(BB_MASTER_AP+1)] = {[BB_STA1] 	   = KNL_STA1,
												[BB_STA2] 	   = KNL_STA2,
												[BB_STA3] 	   = KNL_STA3,
												[BB_STA4] 	   = KNL_STA4,
												[BB_SLAVE_AP]  = KNL_SLAVE_AP,
												[BB_MASTER_AP] = KNL_MASTER_AP};
	while(1)
	{
		osMessageGet(KNL_QueBbFrmOk, &ubBbFrmStatus, osWaitForever);
		ulKNL_FrmTRxNumTemp[tKNL_LinkRole[ubBbFrmStatus]]++;
		if(ptKNL_BbFrmMonitCbFunc)
			ptKNL_BbFrmMonitCbFunc(ubBbFrmStatus);
    }
}

//------------------------------------------------------------------------------
void KNL_SetBbFrmMonitCbFunc(pvKNL_BbFrmOkCbFunc BbFrmOkCbFunc)
{
	ptKNL_BbFrmMonitCbFunc = BbFrmOkCbFunc;
}

#if 1
//------------------------------------------------------------------------------
void KNL_ImgMonitorFunc(struct IMG_RESULT ReceiveResult)
{
	KNL_PROCESS tImgProc;
	KNL_NODE_INFO tNodeInfo;
	static uint32_t ulSubDr[30];
	static uint32_t ulRefreshCnt = 0;
	static uint32_t ulFrmIdx[4] = {0, 0, 0 , 0};
	static uint8_t ubKNL_ImgStsFlg = FALSE;
	static uint8_t ubPreFps = 0;
	uint32_t ulBsSz;
	uint32_t ulGop;
	uint32_t ulRtDr = 0;//Bit-Rate
	uint8_t ubNextNode;
	uint8_t ubPreNode;
	uint8_t ubTemp;
	uint8_t ubSrcNum;
	uint8_t ubTrigType;
	uint8_t ubIdxMap;
	uint8_t i;

	if(FALSE == ubKNL_ImgStsFlg)
	{
		ubPreFps = ubKNL_GetVdoFps();
		for(i=0;i<30;i++)
			ulSubDr[i] = 0;
		ubKNL_ImgStsFlg = TRUE;
	}
	//Encode Process
	//========================================================================
	if(ReceiveResult.H264Result->Type == H264_ENCODE)
	{
		//(1)Update Information First (SrcNum/BS Size)
		ubSrcNum = ubKNL_ImgSrc;
		KNL_SetNodeState(ubSrcNum,KNL_NODE_H264_ENC,KNL_NODE_STOP);
		ulBsSz = ReceiveResult.H264Result->Size;//ulH264_GetStreamSize();
		if(ReceiveResult.H264Result->EncodeStream == ENCODE_0)
		{
			ubIdxMap = 0;
		}
		else if(ReceiveResult.H264Result->EncodeStream == ENCODE_1)
		{
			ubIdxMap = 1;
		}
		else if(ReceiveResult.H264Result->EncodeStream == ENCODE_2)
		{
			ubIdxMap = 2;
		}
		else if(ReceiveResult.H264Result->EncodeStream == ENCODE_3)
		{
			ubIdxMap = 3;
		}
		ulFrmIdx[ubIdxMap] = ReceiveResult.H264Result->ulFrmIdx;
		if(ulFrmIdx[ubIdxMap] == 0)
		{
			ubKNL_VdoGroupIdx[ubIdxMap]++;
		}
		ulGop = ReceiveResult.H264Result->ulGop;
		ubKNL_Qp[ubIdxMap] = H264_GetCurrentQP();

		ulKNL_CurFrmIdx[ubIdxMap] = ulFrmIdx[ubIdxMap];
		if((ulFrmIdx[ubIdxMap]%ubKNL_GetVdoFps()) == (ubKNL_GetVdoFps()-1))
		{
			ulRefreshCnt++;
		}

		//printd(DBG_Debug3Lvl, "BS[%d]_Q:%d_S:%d\r\n",ulFrmIdx[ubIdxMap],H264_GetCurrentQP(),ulBsSz/1024);

		//For Report
		if((ulFrmIdx[ubIdxMap] % ubKNL_GetVdoFps()) == 0)
		{
			ulKNL_VdoOutAccCntTemp[ubIdxMap] = 0;
		}

		if(ReceiveResult.H264Result->EncodeStream == ENCODE_0)
		{
			ulKNL_VdoOutAccCntTemp[0] += (ulBsSz*8);
		}
		else if(ReceiveResult.H264Result->EncodeStream == ENCODE_1)
		{
			ulKNL_VdoOutAccCntTemp[1] += (ulBsSz*8);
		}
		else if(ReceiveResult.H264Result->EncodeStream == ENCODE_2)
		{
			ulKNL_VdoOutAccCntTemp[2] += (ulBsSz*8);
		}
		else if(ReceiveResult.H264Result->EncodeStream == ENCODE_3)
		{
			ulKNL_VdoOutAccCntTemp[3] += (ulBsSz*8);
		}

		ulSubDr[ulFrmIdx[ubIdxMap] % ubKNL_GetVdoFps()] = (ulBsSz*8);
		if((ulFrmIdx[ubIdxMap] % ubKNL_GetVdoFps()) == (ubKNL_GetVdoFps()-1))
		{
			ulRtDr = 0;
			for(i=0;i<ubKNL_GetVdoFps();i++)
			{
				ulRtDr = ulRtDr + ulSubDr[i];
			}
			printd(DBG_Debug2Lvl, "== RtDr:%d KB ==\r\n",ulRtDr/8192);
			ulKNL_VdoOutAccCnt[ubIdxMap] = ulKNL_VdoOutAccCntTemp[ubIdxMap];
			ulKNL_VdoOutAccCntTemp[ubIdxMap] = 0;
		}

		//(2)Release Buffer
		ubPreNode = ubKNL_GetPreNode(ubSrcNum,KNL_NODE_H264_ENC);
		if(ubPreNode == KNL_NODE_SEN_YUV_BUF)
		{
			ubTemp = ubBUF_ReleaseSenYuvBuf(ReceiveResult.H264Result->YuvAddr/*ReceiveResult.YuvAddr*/);
			if(ubTemp != BUF_OK)
			{
				printd(DBG_ErrorLvl, "Err2 @%s[0x%X]\n", __func__, ReceiveResult.H264Result->YuvAddr);
			}
		}

		//(3)Post-Process
		if(((ubKNL_ChkVdoFlowAct(ubSrcNum)) && (!ubKNL_ResetIFlg)) ||	//! if(ubKNL_ChkVdoFlowAct(ubSrcNum))
			(KNL_GetTuningToolMode() == KNL_TUNINGMODE_ON))
		{
			tImgProc.ulDramAddr1	= ReceiveResult.H264Result->YuvAddr;//ReceiveResult.YuvAddr;
			tImgProc.ulDramAddr2	= ReceiveResult.H264Result->BSAddr;	//BS Buffer Address
			tImgProc.ubSrcNum		= ubSrcNum;
			tImgProc.ubCurNode		= KNL_NODE_H264_ENC;
			tImgProc.ubNextNode 	= ubKNL_GetNextNode(ubSrcNum,KNL_NODE_H264_ENC);
			tImgProc.ulSize 		= ulBsSz;
			tImgProc.ubCodecIdx		= ubIdxMap;
			tImgProc.ulIdx			= ulFrmIdx[ubIdxMap];
			tImgProc.ubVdoGop		= ubKNL_VdoGroupIdx[ubIdxMap];
			tImgProc.ulGop			= ulGop;
			if(osMessagePut(KNL_VdoCodecProcQueue, &tImgProc, 0) != osOK)
			{
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
				printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
			}
		}
		else
		{
			ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
			if(ubTemp != BUF_OK)
			{
				printd(DBG_ErrorLvl, "Err3 @%s[0x%X]\n", __func__, ReceiveResult.H264Result->BSAddr);
			}
		}

		if(ubRC_GetFlg(ubIdxMap) && ((ulFrmIdx[ubIdxMap]% ubKNL_GetVdoFps()) == (ubKNL_GetVdoFps()-1)) && (ulRefreshCnt >= ubRC_GetRefreshRate(0)))
		{
			ulRefreshCnt = 0;
			//printd(DBG_Debug3Lvl, "Rst RC1\r\n");

			tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,KNL_NODE_H264_ENC);

			if(ubKNL_ResetIFlg)
			{
				ubKNL_ResetIFlg = 0;
				H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);
			}

			if(ubPreFps != ubRC_GetFps())
			{
				//printd(DBG_Debug3Lvl, "Rst RC11\r\n");
				ubPreFps = ubRC_GetFps();
				if(ReceiveResult.H264Result->EncodeStream == ENCODE_0)
				{
					H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(0,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
				}
				else if(ReceiveResult.H264Result->EncodeStream == ENCODE_1)
				{
					H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(1,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
				}
				else if(ReceiveResult.H264Result->EncodeStream == ENCODE_2)
				{
					H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(2,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
				}
				else if(ReceiveResult.H264Result->EncodeStream == ENCODE_3)
				{
					H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(3,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
				}
			}

			H264_RcSetEN((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE,CBR,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx));

			if((ubKNL_GetVdoFps() <= 5) && (H264_GetCurrentQP() > 3))
			{
				H264_ResetRateControl(H264_GetCurrentQP() - 3);
			}
			else
			{
				H264_ResetRateControl(H264_GetCurrentQP());	
			}
			H264_SetRCParameter((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx),ubKNL_GetVdoFps());
		}
		else if(ubRC_GetFlg(ubIdxMap) && (ubRC_GetUpdateFlg(ubIdxMap)))
		{
			RC_SetUpdateFlg(ubIdxMap,0);

			ulRefreshCnt = 0;
			//printd(DBG_Debug3Lvl, "Rst RC2\r\n");

			tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,KNL_NODE_H264_ENC);

			if(ubKNL_ResetIFlg)
			{
				ubKNL_ResetIFlg = 0;
				H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);
			}
			if(ubPreFps != ubRC_GetFps())
			{
				//printd(DBG_Debug3Lvl, "Rst RC22\r\n");
				ubPreFps = ubRC_GetFps();
				H264_SetFrameRate((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ubRC_GetFps());
			}
			H264_RcSetEN((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE,CBR,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx));
			if((ubKNL_GetVdoFps() <= 5) && (H264_GetCurrentQP() > 3))
			{
				H264_ResetRateControl(H264_GetCurrentQP() - 3);
			}
			else
			{
				H264_ResetRateControl(H264_GetCurrentQP());
			}
			H264_SetRCParameter((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx),ubKNL_GetVdoFps());
		}

		//(4)Release IMG/H264
		osSemaphoreRelease(tKNL_ImgSem);
		ubKNL_ImgRdy = 1;	//Codec Ready
		printd(DBG_Debug3Lvl, "H->EOK\r\n");
	}
	//Decode Process
	//========================================================================
	else if(ReceiveResult.H264Result->Type == H264_DECODE)
	{
		//(1)Update SrcNum
		ubSrcNum = ubKNL_ImgSrc;
		KNL_SetNodeState(ubSrcNum,KNL_NODE_H264_DEC,KNL_NODE_STOP);

		//(2)Release Buffer
		//--------------------------------------------------------------
		ubPreNode = ubKNL_GetPreNode(ubSrcNum,KNL_NODE_H264_DEC);
		if((ubPreNode == KNL_NODE_VDO_BS_BUF1) || (ubPreNode == KNL_NODE_VDO_BS_BUF2))
		{
			ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
			if(ubTemp != BUF_OK)
				printd(DBG_ErrorLvl, "Err4 @%s[0x%X]\n", __func__, ReceiveResult.H264Result->BSAddr);
		}

		if(H264_FAIL == ReceiveResult.H264Result->Result)
			KNL_RestartImgCodec();
		else
			printd(DBG_Debug3Lvl, "H->DOK\r\n");

		//(3)Post-Process
		ubNextNode = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_H264_DEC);
		if(ubKNL_ChkVdoFlowAct(ubSrcNum))
		{
			if(ubNextNode == KNL_NODE_LCD)
			{
				KNL_ActiveLcdDispBuf(ubSrcNum);
			}
			else
			{
				tImgProc.ulDramAddr1	= ReceiveResult.H264Result->YuvAddr;
				tImgProc.ubSrcNum		= ubSrcNum;
				tImgProc.ubCurNode		= KNL_NODE_H264_DEC;
				tImgProc.ubNextNode 	= ubNextNode;
				tImgProc.ulSize			= ((uint32_t)uwKNL_GetVdoH(ubSrcNum))*((uint32_t)uwKNL_GetVdoV(ubSrcNum))*1.5;
				if(osMessagePut(KNL_VdoCodecProcQueue, &tImgProc, 0) == osErrorResource)
				{
					ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
					printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
				}
			}
		}

		//(4)Release IMG/H264
		osSemaphoreRelease(tKNL_ImgSem);
		ubKNL_ImgRdy = 1;	//Codec Ready
	}
	//Merge Process
	//========================================================================
	else
	{
		//(0)Update Info First
		ubSrcNum = ubKNL_ImgTrigSrc;
		ubTrigType = ubKNL_ImgTrigType;

		//(1)Release IMG/H264
		osSemaphoreRelease(tKNL_ImgSem);
		ubKNL_ImgRdy = 1;	//Codec Ready
		printd(DBG_Debug1Lvl, "IMG->MOK\r\n");

		//(2)To Next Node
		if(ubTrigType == KNL_IMG_MERGE_H)
		{
			KNL_SetNodeState(ubSrcNum,KNL_NODE_IMG_MERGE_H,KNL_NODE_STOP);
			tImgProc.ubCurNode 	= KNL_NODE_IMG_MERGE_H;
			ubNextNode = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_IMG_MERGE_H);
		}
		tImgProc.ubSrcNum	= ubSrcNum;
		tImgProc.ubNextNode = ubNextNode;

		if(ubKNL_ChkVdoFlowAct(ubSrcNum))
		{
			if(ubNextNode == KNL_NODE_LCD)
			{
				KNL_ActiveLcdDispBuf(ubSrcNum);
			}
			else
			{
				osMessagePut(KNL_VdoCodecProcQueue, &tImgProc, 0);
			}
		}
	}
}

//------------------------------------------------------------------------------
#else

static void KNL_ImgMonitThread(void const *argument)
{
	struct IMG_RESULT ReceiveResult;
	KNL_PROCESS tImgProc;
	uint8_t ubNextNode;
	uint8_t ubPreNode;
	uint8_t ubTemp;	
	uint8_t ubSrcNum;	
	uint8_t ubTrigType;
	uint32_t ulBsSz;
	uint32_t ulFrmIdx[4];
	uint32_t ulGop;
//	struct H264_ENCODE *codec;
	uint32_t ulRefreshCnt = 0;
	KNL_NODE_INFO tNodeInfo;
	uint8_t ubIdxMap;

	uint8_t i;
	uint32_t ulSubDr[30];
	uint32_t ulRtDr = 0;//Bit-Rate

	uint8_t ubPreFps;

	ubPreFps = ubKNL_GetVdoFps();

	for(i=0;i<30;i++)
	{
		ulSubDr[i] = 0;
	}

	while(1)
	{
        osMessageGet(IMG_EventQueue, &ReceiveResult, osWaitForever);
        //Encode Process
        //========================================================================
        if(ReceiveResult.H264Result->Type == H264_ENCODE)
        {
            //(1)Update Information First (SrcNum/BS Size)
            ubSrcNum = ubKNL_ImgSrc;
            KNL_SetNodeState(ubSrcNum,KNL_NODE_H264_ENC,KNL_NODE_STOP);
            ulBsSz = ReceiveResult.H264Result->Size;	//ulH264_GetStreamSize();
			if(ReceiveResult.H264Result->EncodeStream == ENCODE_0)
			{
				ubIdxMap = 0;
			}
			else if(ReceiveResult.H264Result->EncodeStream == ENCODE_1)
			{
				ubIdxMap = 1;
			}
			else if(ReceiveResult.H264Result->EncodeStream == ENCODE_2)
			{
				ubIdxMap = 2;
			}
			else if(ReceiveResult.H264Result->EncodeStream == ENCODE_3)
			{
				ubIdxMap = 3;
			}
			ulFrmIdx[ubIdxMap] = ReceiveResult.H264Result->ulFrmIdx;
			if(ulFrmIdx[ubIdxMap] == 0)
			{
				ubKNL_VdoGroupIdx[ubIdxMap]++;
			}
			ulGop	= ReceiveResult.H264Result->ulGop;
			ubKNL_Qp[ubIdxMap] = H264_GetCurrentQP();

			ulKNL_CurFrmIdx[ubIdxMap] = ulFrmIdx[ubIdxMap];
			if((ulFrmIdx[ubIdxMap]%ubKNL_GetVdoFps()) == (ubKNL_GetVdoFps()-1))
			{
				ulRefreshCnt++;
			}

			//printd(DBG_Debug3Lvl, "BS[%d]_Q:%d_S:%d\r\n",ulFrmIdx[ubIdxMap],H264_GetCurrentQP(),ulBsSz/1024);

			//For Report
			if((ulFrmIdx[ubIdxMap] % ubKNL_GetVdoFps()) == 0)
			{
				ulKNL_VdoOutAccCntTemp[ubIdxMap] = 0;
			}

			if(ReceiveResult.H264Result->EncodeStream == ENCODE_0)
			{
				ulKNL_VdoOutAccCntTemp[0] += (ulBsSz*8);
			}
			else if(ReceiveResult.H264Result->EncodeStream == ENCODE_1)
			{
				ulKNL_VdoOutAccCntTemp[1] += (ulBsSz*8);
			}
			else if(ReceiveResult.H264Result->EncodeStream == ENCODE_2)
			{
				ulKNL_VdoOutAccCntTemp[2] += (ulBsSz*8);
			}
			else if(ReceiveResult.H264Result->EncodeStream == ENCODE_3)
			{
				ulKNL_VdoOutAccCntTemp[3] += (ulBsSz*8);
			}

			ulSubDr[ulFrmIdx[ubIdxMap] % ubKNL_GetVdoFps()] = (ulBsSz*8);
			if((ulFrmIdx[ubIdxMap] % ubKNL_GetVdoFps()) == (ubKNL_GetVdoFps()-1))
			{
				ulRtDr = 0;
				for(i=0;i<ubKNL_GetVdoFps();i++)
				{
					ulRtDr = ulRtDr + ulSubDr[i];
				}
				printd(DBG_Debug2Lvl, "== RtDr:%d KB ==\r\n",ulRtDr/8192);
				ulKNL_VdoOutAccCnt[ubIdxMap] = ulKNL_VdoOutAccCntTemp[ubIdxMap];
				ulKNL_VdoOutAccCntTemp[ubIdxMap] = 0;
			}

            //(2)Release Buffer
            ubPreNode = ubKNL_GetPreNode(ubSrcNum,KNL_NODE_H264_ENC);
            if(ubPreNode == KNL_NODE_SEN_YUV_BUF)
            {
				ubTemp = ubBUF_ReleaseSenYuvBuf(ReceiveResult.H264Result->YuvAddr/*ReceiveResult.YuvAddr*/);
                if(ubTemp != BUF_OK)
                {
                    printd(DBG_ErrorLvl, "Err2 @%s[0x%X]\n", __func__, ReceiveResult.H264Result->YuvAddr);
                }
            }

            //(3)Post-Process
            if(ubKNL_ChkVdoFlowAct(ubSrcNum))
            {
                tImgProc.ulDramAddr1	= ReceiveResult.H264Result->YuvAddr;//ReceiveResult.YuvAddr;
                tImgProc.ulDramAddr2	= ReceiveResult.H264Result->BSAddr;	//BS Buffer Address
                tImgProc.ubSrcNum		= ubSrcNum;			
                tImgProc.ubCurNode		= KNL_NODE_H264_ENC;						
                tImgProc.ubNextNode 	= ubKNL_GetNextNode(ubSrcNum,KNL_NODE_H264_ENC);												
                tImgProc.ulSize 		= ulBsSz;
				tImgProc.ubCodecIdx		= ubIdxMap;
				tImgProc.ulIdx			= ulFrmIdx[ubIdxMap];
				tImgProc.ubVdoGop		= ubKNL_VdoGroupIdx[ubIdxMap];
				tImgProc.ulGop			= ulGop;
                if(osMessagePut(KNL_VdoCodecProcQueue, &tImgProc, 0) != osOK)
                {
					ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
                    printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
                }
				else
					printd(DBG_Debug1Lvl, "E->OK\n");
            }
			else
			{
				ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
				if(ubTemp != BUF_OK)
                {
                    printd(DBG_ErrorLvl, "Err3 @%s[0x%X]\n", __func__, ReceiveResult.H264Result->BSAddr);
                }
			}
			
			if(ubRC_GetFlg(ubIdxMap) && ((ulFrmIdx[ubIdxMap]% ubKNL_GetVdoFps()) == (ubKNL_GetVdoFps()-1)) && (ulRefreshCnt >= ubRC_GetRefreshRate(0)))
			{
				ulRefreshCnt = 0;
				//printd(DBG_Debug3Lvl, "Rst RC1\r\n");

				tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,KNL_NODE_H264_ENC);

				if(ubKNL_ResetIFlg)
				{
					ubKNL_ResetIFlg = 0;
					H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);
				}
				
				if(ubPreFps != ubRC_GetFps())
				{					
					//printd(DBG_Debug3Lvl, "Rst RC11\r\n");
					
					ubPreFps = ubRC_GetFps();
					if(ReceiveResult.H264Result->EncodeStream == ENCODE_0)
					{				
						H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(0,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]												
					}
					else if(ReceiveResult.H264Result->EncodeStream == ENCODE_1)
					{				
						H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(1,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
					}
					else if(ReceiveResult.H264Result->EncodeStream == ENCODE_2)
					{				
						H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(2,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
					}
					else if(ReceiveResult.H264Result->EncodeStream == ENCODE_3)
					{				
						H264_EncodeInit((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,tNodeInfo.uwVdoH,tNodeInfo.uwVdoV,ulBUF_GetBlkBufAddr(3,BUF_IMG_ENC),ubRC_GetFps(),ulKNL_GetVdoGop());//DECODE[0]
					}
				}
				
				H264_RcSetEN((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE,CBR,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx));
				
				if((ubKNL_GetVdoFps() <= 5) && (H264_GetCurrentQP() > 3))
				{
					H264_ResetRateControl(H264_GetCurrentQP() - 3);				
				}
				else
				{
					H264_ResetRateControl(H264_GetCurrentQP());	
				}
				
				H264_SetRCParameter((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx),ubKNL_GetVdoFps());				
			}
			else if(ubRC_GetFlg(ubIdxMap) && (ubRC_GetUpdateFlg(ubIdxMap)))
			{
				RC_SetUpdateFlg(ubIdxMap,0);
				
				ulRefreshCnt = 0;
				//printd(DBG_Debug3Lvl, "Rst RC2\r\n");
				
				tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,KNL_NODE_H264_ENC);	
				
				if(ubKNL_ResetIFlg)
				{
					ubKNL_ResetIFlg = 0;						
					H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);
				}				
				
				if(ubPreFps != ubRC_GetFps())
				{					
					//printd(DBG_Debug3Lvl, "Rst RC22\r\n");					
					ubPreFps = ubRC_GetFps();					
					H264_SetFrameRate((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ubRC_GetFps());					
				}				
				H264_RcSetEN((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,H264_ENABLE,CBR,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx));
				if((ubKNL_GetVdoFps() <= 5) && (H264_GetCurrentQP() > 3))
				{
					H264_ResetRateControl(H264_GetCurrentQP() - 3);				
				}
				else
				{
					H264_ResetRateControl(H264_GetCurrentQP());	
				}
				H264_SetRCParameter((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx,ulRC_GetFinalBitRate(tNodeInfo.ubCodecIdx),ubKNL_GetVdoFps());				
			}
			
			//(4)Release IMG/H264
			osSemaphoreRelease(tKNL_ImgSem);
            ubKNL_ImgRdy = 1;	//Codec Ready
			printd(DBG_Debug3Lvl, "H->EOK\r\n");
        }
        //Decode Process
        //========================================================================
        else if(ReceiveResult.H264Result->Type == H264_DECODE)
        {
			//(1)Update SrcNum
            ubSrcNum = ubKNL_ImgSrc;
            KNL_SetNodeState(ubSrcNum,KNL_NODE_H264_DEC,KNL_NODE_STOP);

            //(2)Release Buffer
            //-------------------------------------------------------------
            ubPreNode = ubKNL_GetPreNode(ubSrcNum,KNL_NODE_H264_DEC);
            if((ubPreNode == KNL_NODE_VDO_BS_BUF1) || (ubPreNode == KNL_NODE_VDO_BS_BUF2))
            {
                ubTemp = ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
                if(ubTemp != BUF_OK)
                {
                    printd(DBG_ErrorLvl, "Err4 @%s[0x%X]\n", __func__, ReceiveResult.H264Result->BSAddr);
                }
            }

			if(H264_FAIL == ReceiveResult.H264Result->Result)
				KNL_RestartImgCodec();
			else
				printd(DBG_Debug3Lvl, "H->DOK\r\n");

            //(3)Post-Process
            ubNextNode = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_H264_DEC);

            if(ubKNL_ChkVdoFlowAct(ubSrcNum))
            {
                if(ubNextNode == KNL_NODE_LCD)
                {
                    KNL_ActiveLcdDispBuf(ubSrcNum);
                }
                else
                {
                    tImgProc.ulDramAddr1	= ReceiveResult.H264Result->YuvAddr;
                    tImgProc.ubSrcNum		= ubSrcNum;
                    tImgProc.ubCurNode		= KNL_NODE_H264_DEC;
                    tImgProc.ubNextNode 	= ubNextNode;
                    tImgProc.ulSize			= ((uint32_t)uwKNL_GetVdoH(ubSrcNum))*((uint32_t)uwKNL_GetVdoV(ubSrcNum))*1.5;
                    if(osMessagePut(KNL_VdoCodecProcQueuev, &tImgProc, 0) == osErrorResource)
					{
						ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,ubSrcNum,ReceiveResult.H264Result->BSAddr);
						printd(DBG_ErrorLvl, "KNL_Q->Full !!!!\r\n");
					}
                }
            }

			//(4)Release IMG/H264
			osSemaphoreRelease(tKNL_ImgSem);
			ubKNL_ImgRdy = 1;	//Codec Ready
        }
        //Merge Process
        //========================================================================
        else
        {
            //(0)Update Info First
            ubSrcNum = ubKNL_ImgTrigSrc;
            ubTrigType = ubKNL_ImgTrigType;
            
            //(1)Release IMG/H264
			osSemaphoreRelease(tKNL_ImgSem);
            ubKNL_ImgRdy = 1;	//Codec Ready
			printd(DBG_Debug1Lvl, "IMG->MOK\r\n");
                
            //(2)To Next Node
            if(ubTrigType == KNL_IMG_MERGE_H)
            {
                KNL_SetNodeState(ubSrcNum,KNL_NODE_IMG_MERGE_H,KNL_NODE_STOP);					
                tImgProc.ubCurNode 	= KNL_NODE_IMG_MERGE_H;
                ubNextNode = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_IMG_MERGE_H);
            }				
            tImgProc.ubSrcNum	= ubSrcNum;						
            tImgProc.ubNextNode = ubNextNode;				
            
            if(ubKNL_ChkVdoFlowAct(ubSrcNum))
            {
                if(ubNextNode == KNL_NODE_LCD)
                {
                    KNL_ActiveLcdDispBuf(ubSrcNum);
                }
                else
                {
                    osMessagePut(KNL_VdoCodecProcQueue, &tImgProc, 0);
                }
            }
        }
    }	
}
#endif
//------------------------------------------------------------------------------
void KNL_SetVdoCodec(uint8_t ubVdoCodec)
{
	tKNL_Info.ubVdoCodec = ubVdoCodec;
}

uint8_t ubKNL_GetVdoCodec(void)
{
	return tKNL_Info.ubVdoCodec;
}

//------------------------------------------------------------------------------
void KNL_SetVdoH(uint8_t ubSrcNum,uint16_t uwVdoH)
{
	tKNL_Info.uwVdoH[ubSrcNum] = uwVdoH;
}
//------------------------------------------------------------------------------
uint16_t uwKNL_GetVdoH(uint8_t ubSrcNum)
{
	if(ubSrcNum == KNL_SRC_NONE)
	{
		return 0;
	}
	return tKNL_Info.uwVdoH[ubSrcNum];
}
//------------------------------------------------------------------------------
void KNL_SetVdoV(uint8_t ubSrcNum,uint16_t uwVdoV)
{
	tKNL_Info.uwVdoV[ubSrcNum] = uwVdoV;
}
//------------------------------------------------------------------------------
uint16_t uwKNL_GetVdoV(uint8_t ubSrcNum)
{
	if(ubSrcNum == KNL_SRC_NONE)
	{
		return 0;
	}
	return tKNL_Info.uwVdoV[ubSrcNum];
}

//------------------------------------------------------------------------------
void KNL_AdoStart(uint8_t ubSrcNum)
{
	uint8_t ubRxVdo_Path;
	uint8_t ubTxAdo_Path;
	
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_ADC))
	{
		if((ubKNL_GetRole() == KNL_SLAVE_AP) || (ubKNL_GetRole() == KNL_MASTER_AP))
		{
			ubRxVdo_Path = ubBB_GetRxVdoDataPath();
			ubTxAdo_Path = ubSrcNum % 4;
			
			BB_SetDataPath((TXADO)ubTxAdo_Path,BB_RX_ADO_ALL_STA,(PAYLOAD_PATH)ubRxVdo_Path);
		}		
			
		KNL_AdcStart();
	}	
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_DAC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_DAC_BUF)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
	{
		KNL_DacStart(ubSrcNum);
	}	
}
//------------------------------------------------------------------------------
void KNL_AdoStop(uint8_t ubSrcNum)
{
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_ADC))
	{
		KNL_AdcStop();
	}	
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_DAC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_DAC_BUF)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
	{
		KNL_DacStop(ubSrcNum);
	}	
}
//------------------------------------------------------------------------------
void KNL_AdoResume(uint8_t ubSrcNum)
{
	if(ubKNL_ExistNode(ubSrcNum, KNL_NODE_ADC))
	{
		ubKNL_AdcFlowActiveFlg = 1;
	}
}
//------------------------------------------------------------------------------
void KNL_AdoSuspend(uint8_t ubSrcNum)
{
	if(ubKNL_ExistNode(ubSrcNum, KNL_NODE_ADC))
	{
		ubKNL_AdcFlowActiveFlg = 0;
		BUF_Reset(BUF_ADO_ADC);
	}
}
//------------------------------------------------------------------------------
void KNL_AdcStart(void)
{
	if(ubKNL_AdcFlowActiveFlg)
		return;
	ubKNL_AdcFlowActiveFlg = 1;
//	ADO_Reset();
	ADO_RecStart();	
}
//------------------------------------------------------------------------------
void KNL_AdcStop(void)
{
	//======== Pre-Process ========
	ubKNL_AdcFlowActiveFlg = 0;
	ADO_RecStop();

	//======== Transient ==========
	//Timer_Delay_ms(500);

	//======== Post-Process =======
	BUF_Reset(BUF_ADO_ADC);
}
//------------------------------------------------------------------------------
void KNL_DacStart(uint8_t ubSrcNum)
{
	ubKNL_DacFlowActiveFlg[ubSrcNum] = 1;
}
//------------------------------------------------------------------------------
void KNL_DacStop(uint8_t ubSrcNum)
{
	ubKNL_DacFlowActiveFlg[ubSrcNum] = 0;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_WaitNodeFinish(uint8_t ubSrcNum)
{
	while(1)
	{
		if(ubSrcNum == KNL_SRC_NONE)
			return 0;

		if(ubKNL_ChkNodeFinish(ubSrcNum))
		{
			printd(DBG_CriticalLvl, "Src[%d]->Done\r\n", ubSrcNum);
			break;
		}
		else
		{
			osDelay(10);
		}
	}
	return 1;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkNodeFinish(uint8_t ubSrcNum)
{
	uint8_t ubSrcNumMap;
	KNL_PROCESS tProc;
	static uint8_t ubKNL_ChkNodeCnt = 0;

	//AvgPly Related
	if(tKNL_GetPlyMode() == KNL_AVG_PLY)
	{
		ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
		while(1)
		{
			if(osMessages(KNL_AvgPlyQueue[ubSrcNumMap]))
			{
				osMessageGet(KNL_AvgPlyQueue[ubSrcNumMap], &tProc, 0);
				ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_DEC,ubSrcNumMap,tProc.ulDramAddr2);
			}
			else
			{
				ubKNL_AvgPlyCnt[ubSrcNumMap] = 0;
				ubKNL_AvgPlyStartFlg[ubSrcNumMap] = 0;
				break;
			}
		}
	}

	//IMG,H264 Related
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_ENC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_DEC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_IMG_MERGE_BUF)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_IMG_MERGE_H))
	{
		if(ubKNL_ChkImgRdy() == 0)
		{
			if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_DEC) && (ubKNL_ChkNodeCnt++ >= 30))
			{
				ubKNL_ImgBusyFlg = TRUE;
				ubKNL_ChkNodeCnt = 0;
			}
			return 0;
		}
		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_DEC))
			osDelay(50);
	}
	if(TRUE == ubKNL_ImgBusyFlg)
		ubKNL_ImgBusyFlg = FALSE;
	ubKNL_ChkNodeCnt = 0;

	//JPEG Related
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_JPG_ENC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_JPG_DEC1)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_JPG_DEC2))
	{
		if(osSemaphoreWait(JPEG_CodecSem, 0) != osOK)
		{
			osSemaphoreRelease(JPEG_CodecSem);
			return 0;
		}
	}

	//SEN Related
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_SEN))
	{
		if(ubKNL_ChkSenStateChangeDone() == 0)
		{
			return 0;
		}
	}

	ubKNL_VdoFlowActiveFlg[ubSrcNum] = 0;
	KNL_VdoSuspend(ubSrcNum);

	if(ubKNL_ExistNode(ubSrcNum, KNL_NODE_VDO_BS_BUF1))
	{
		BUF_Reset((BUF_VDO_MAIN_BS0+ubSrcNumMap));
	}

	return 1;
}
//------------------------------------------------------------------------------
void KNL_VdoStart(uint8_t ubSrcNum)
{
	KNL_NODE_INFO tNodeInfo;
	uint16_t i;
	uint8_t ubSrcNumMap;
#if OP_AP
	uint8_t ubRxVdo_OriPath,ubRxVdo_NewPath;
	uint8_t ubTxAdo_Path;
#endif

	printf("KNL_VdoStart ubSrcNum: %d ###.\n", ubSrcNum);
	ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
	if(ubSrcNumMap <= KNL_STA4)
	{
		ubKNL_VdoFlowActiveFlg[ubSrcNum] = 1;
		ubKNL_RcvFirstIFrame[ubSrcNum]	 = 0;

		for(i=0;i<256;i++)
		{
			if(ubKNL_ExistNode(ubSrcNum,i))
			{
				tKNL_NodeState[ubSrcNum][i] = KNL_NODE_START;
			}
		}

		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_ENC))
		{
			tNodeInfo = tKNL_GetNodeInfo(ubSrcNum,KNL_NODE_H264_ENC);
			H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);
		}

		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_SEN))
		{
			KNL_SenStart(ubSrcNum);
		}

#if OP_AP
		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_VDO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_VDO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_ADO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
		{
			ubRxVdo_OriPath = ubBB_GetRxVdoDataPath();
			ubTxAdo_Path	= ubBB_GetTxAdoDataPath();
			
			if(ubSrcNumMap == 0)
			{
				ubRxVdo_NewPath = ubRxVdo_OriPath | BB_OPEN_STA1_PAYLOAD;
			}
			else if(ubSrcNumMap == 1)
			{
				ubRxVdo_NewPath = ubRxVdo_OriPath | BB_OPEN_STA2_PAYLOAD;
			}
			else if(ubSrcNumMap == 2)
			{
				ubRxVdo_NewPath = ubRxVdo_OriPath | BB_OPEN_STA3_PAYLOAD;
			}
			else if(ubSrcNumMap == 3)
			{
				ubRxVdo_NewPath = ubRxVdo_OriPath | BB_OPEN_STA4_PAYLOAD;
			}
			BB_SetDataPath((TXADO)ubTxAdo_Path,BB_RX_ADO_ALL_STA,(PAYLOAD_PATH)ubRxVdo_NewPath);
		}
#endif	
	}
}
//------------------------------------------------------------------------------
void KNL_VdoStop(uint8_t ubSrcNum)
{
	uint16_t i;
	uint8_t ubSrcNumMap;
#if OP_AP
	uint8_t ubRxVdo_OriPath,ubRxVdo_NewPath;	
	uint8_t ubTxAdo_Path;
#endif

	ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
	if(ubSrcNumMap <= KNL_STA4)
	{
		ubKNL_VdoFlowActiveFlg[ubSrcNum] 	= 0;
		ubKNL_RcvFirstIFrame[ubSrcNum]	 	= 0;
		ubKNL_VdoResendITwcFlg[ubSrcNumMap] = FALSE;
		ubKNL_VdoResChgTwcFlg[ubSrcNumMap]  = FALSE;
		for(i=0;i<256;i++)
		{
			tKNL_NodeState[ubSrcNum][i] = KNL_NODE_STOP;
		}
#if OP_AP
		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_VDO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_VDO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_ADO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
		{
			KNL_SRC tAdoSrcNum;

			if(ptKNL_AdoRoleSrcMapT)
				tAdoSrcNum = ptKNL_AdoRoleSrcMapT((KNL_ROLE)ubSrcNumMap);
			if(!ubKNL_ChkDacFlowAct((uint8_t)tAdoSrcNum))
			{
				ubRxVdo_OriPath = ubBB_GetRxVdoDataPath();
				ubTxAdo_Path	= ubBB_GetTxAdoDataPath();
				if(ubSrcNumMap == 0)
				{
					ubRxVdo_NewPath = ubRxVdo_OriPath & 0xFE;
				}
				else if(ubSrcNumMap == 1)
				{
					ubRxVdo_NewPath = ubRxVdo_OriPath & 0xFD;
				}
				else if(ubSrcNumMap == 2)
				{
					ubRxVdo_NewPath = ubRxVdo_OriPath & 0xFB;
				}
				else if(ubSrcNumMap == 3)
				{
					ubRxVdo_NewPath = ubRxVdo_OriPath & 0xF7;
				}
				BB_SetDataPath((TXADO)ubTxAdo_Path,BB_RX_ADO_ALL_STA,(PAYLOAD_PATH)ubRxVdo_NewPath);
			}
		}
#endif
		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_SEN))
		{
			KNL_SenStop(ubSrcNum);
		}
	}
}
//------------------------------------------------------------------------------
void KNL_VdoSuspend(uint8_t ubSrcNum)
{
	uint16_t i;
	uint8_t ubRole;

	for(i = 0; i < 256; i++)
		tKNL_NodeState[ubSrcNum][i] = KNL_NODE_STOP;
	ubKNL_RcvFirstIFrame[ubSrcNum]   = 0;
	ulKNL_OutVdoFpsCnt[ubSrcNum]     = 0;
	ulKNL_OutVdoFpsCntTemp[ubSrcNum] = 0;
	ulKNL_InVdoFpsCnt[ubSrcNum]      = 0;
	ulKNL_InVdoFpsCntTemp[ubSrcNum]  = 0;
	ubRole = ubKNL_SrcNumMap(ubSrcNum);
	ulKNL_VdoOutAccCnt[ubRole] 		 = 0;
	ulKNL_VdoOutAccCntTemp[ubRole] 	 = 0;
	ulKNL_FrmTRxNum[ubRole]		 	 = 0;
	ulKNL_FrmTRxNumTemp[ubRole]	 	 = 0;
	ubKNL_VdoResendITwcFlg[ubRole] 	 = FALSE;
	ubKNL_VdoResChgTwcFlg[ubRole]  	 = FALSE;
}
//------------------------------------------------------------------------------
void KNL_VdoResume(uint8_t ubSrcNum)
{
	uint16_t i;

	for(i = 0; i < 256; i++)
	{
		if(ubKNL_ExistNode(ubSrcNum, i))
			tKNL_NodeState[ubSrcNum][i] = KNL_NODE_START;
	}
}
//------------------------------------------------------------------------------
void KNL_SenStart(uint8_t ubSrcNum)
{
#ifdef OP_STA
	uint32_t ulTemp;
	if(ubSEN_GetPathSrc(SENSOR_PATH1) == ubSrcNum)
	{
		if(!ubSEN_GetActiveFlg(SENSOR_PATH1))
		{
			BUF_Reset(BUF_SEN_1_YUV);
			ulTemp = ulBUF_GetSen1YuvFreeBuf();
			SEN_SetPathAddr(SENSOR_PATH1,ulTemp);
			SEN_SetActiveFlg(SENSOR_PATH1,1);
			SEN_SetPathState(SENSOR_PATH1, 1);
			SEN->VIDEO_STR_EN_1 = 1;
		}
	}
	else if(ubSEN_GetPathSrc(SENSOR_PATH2) == ubSrcNum)
	{
		if(!ubSEN_GetActiveFlg(SENSOR_PATH2))
		{
			BUF_Reset(BUF_SEN_2_YUV);
			ulTemp = ulBUF_GetSen2YuvFreeBuf();
			SEN_SetPathAddr(SENSOR_PATH2,ulTemp);
			SEN_SetActiveFlg(SENSOR_PATH2,1);
			SEN_SetPathState(SENSOR_PATH2, 1);
			SEN->VIDEO_STR_EN_2 = 1;
		}
	}
	else if(ubSEN_GetPathSrc(SENSOR_PATH3) == ubSrcNum)
	{
		if(!ubSEN_GetActiveFlg(SENSOR_PATH3))
		{
			BUF_Reset(BUF_SEN_3_YUV);
			ulTemp = ulBUF_GetSen3YuvFreeBuf();
			SEN_SetPathAddr(SENSOR_PATH3,ulTemp);
			SEN_SetActiveFlg(SENSOR_PATH3,1);
			SEN_SetPathState(SENSOR_PATH3, 1);
			SEN->VIDEO_STR_EN_3 = 1;
		}
	}

	if(!ubKNL_SenStartFlg)
	{
/*
        SEN_SetPathAddr(ISP_3DNR, ulBUF_GetBlkBufAddr(0, BUF_ISP_3DNR_IP));
        SEN_SetPathAddr(ISP_MD_W0, ulBUF_GetBlkBufAddr(0, BUF_ISP_MD_W0_IP));
        SEN_SetPathAddr(ISP_MD_W1, ulBUF_GetBlkBufAddr(0, BUF_ISP_MD_W1_IP));
        SEN_SetPathAddr(ISP_MD_W2, ulBUF_GetBlkBufAddr(0, BUF_ISP_MD_W2_IP));
*/
		SEN_UpdatePathAddr();
		SEN_SetStateChangeFlg(1);
		//SEN_SetFirstOutFlg(1);
        SEN_SetSensorFreeRun(0);
		SEN_EnableVideo();
		ubKNL_SenStartFlg = 1;
	}
#endif
}
//------------------------------------------------------------------------------
void KNL_SenStop(uint8_t ubSrcNum)
{	
	uint8_t ubOffFlg = 0;	

	ubOffFlg = ubOffFlg;
#ifdef OP_STA
	if(ubSEN_GetPathSrc(SENSOR_PATH1) == ubSrcNum)
	{
		SEN_SetActiveFlg(SENSOR_PATH1,0);
	}
	else if(ubSEN_GetPathSrc(SENSOR_PATH2) == ubSrcNum)
	{
		SEN_SetActiveFlg(SENSOR_PATH2,0);
	}
	else if(ubSEN_GetPathSrc(SENSOR_PATH3) == ubSrcNum)
	{
		SEN_SetActiveFlg(SENSOR_PATH3,0);
	}
	ubOffFlg = 1;
	if(ubSEN_GetActiveFlg(SENSOR_PATH1))
	{
		ubOffFlg = 0;
	}
	if(ubSEN_GetActiveFlg(SENSOR_PATH2))
	{
		ubOffFlg = 0;
	}
	if(ubSEN_GetActiveFlg(SENSOR_PATH3))
	{
		ubOffFlg = 0;
	}
	if(ubOffFlg)
	{
		ubKNL_SenStartFlg = 0;
	}
#endif
}
//------------------------------------------------------------------------------
void KNL_ImgStabNotifyFunc(void)
{
#ifdef OP_STA
    SEN_SetFrameRate(SENSOR_PATH1, ubKNL_GetVdoFps());
	ubKNL_ImgStabFlg = TRUE;
#endif
}
//------------------------------------------------------------------------------
uint8_t ubKNL_ChkSenStateChangeDone(void)
{
#ifdef OP_STA
	return (!ubSEN_GetStateChangeFlg());
#else
	return 1;
#endif
}
//------------------------------------------------------------------------------
uint8_t ubKNL_JPEGEncode(uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr)
{
	JPEG_FIFO_Addr_t 	tJpgFiFo;
	JPEG_ENC_INFO_t 	tJpgEncInfo;
	JPEG_CODEC_FN_ES_t	tJpgCodecFnEs;

	tJpgFiFo.ulJpeg_Buf_Start	= ulJpgAddr;
	tJpgFiFo.ulJpeg_Buf_End		= ulJpgAddr + JPEG_MAX_BS_SZ;
	JPEG_Ring_FIFO_Setup(tJpgFiFo);

	JPEG_Set_Start_Address(ulVdoAddr,ulJpgAddr);

	tJpgEncInfo.uwH_ORI_SIZE		= uwH;
	tJpgEncInfo.uwH_SIZE			= uwH;
	tJpgEncInfo.uwV_SIZE			= uwV;
	tJpgEncInfo.uwH_START			= 0;
	tJpgEncInfo.uwV_START			= 0;
	tJpgEncInfo.ubJPG_Fmt 			= JPEG_YUV420;

	JPEG_Set_QP_Value(ubKNL_GetJpegQp());

	tJpgCodecFnEs.tEM				= JPEG_QUEUE;
	tJpgCodecFnEs.pvEvent			= &KNL_QueueJpegMonit;
	tJpgCodecFnEs.tNotifyMode		= JPEG_FN_USE_ISR;
	JPEG_Encode_Setup(tJpgEncInfo, tJpgCodecFnEs);

	JPEG_Codec_Enable();

	if(tJpgCodecFnEs.tNotifyMode ==  JPEG_FN_USE_POLLING)
	{
		while(!JPEG->JPG_TX_OK_FLAG);
		JPEG->CLR_JPG_TX_OK_FLAG = 1;
		printd(DBG_Debug3Lvl, "Size{JPEG}:0x%x @ WAIT Mode\r\n", ulJPEG_Get_BS_Size());
		JPEG_Codec_Disable();
		osSemaphoreRelease(JPEG_CodecSem);
		return 1;
	}
	return 1;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_JPEGDecode(KNL_NODE_INFO *pKNL_NodeInfo, uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr)
{
	JPEG_FIFO_Addr_t 	tJpgFiFo;
	JPEG_DEC_INFO_t 	tJpgDecInfo;
	JPEG_CODEC_FN_ES_t	tJpgCodecFnEs;

	tJpgFiFo.ulJpeg_Buf_Start		= ulJpgAddr;
	tJpgFiFo.ulJpeg_Buf_End			= ulJpgAddr + JPEG_MAX_BS_SZ;
	JPEG_Ring_FIFO_Setup(tJpgFiFo);	

	JPEG_Set_Start_Address(ulVdoAddr,ulJpgAddr);

	tJpgDecInfo.uwH_ORI_SIZE		= uwH;
	tJpgDecInfo.uwH_SIZE			= uwH;
	tJpgDecInfo.uwV_SIZE			= uwV;
	tJpgDecInfo.uwQP				= ubKNL_GetJpegQp();
	tJpgDecInfo.ubJPG_Fmt 			= JPEG_YUV420;
	tJpgDecInfo.ubJPG_ScaleMode		= 0;
	tJpgDecInfo.ubJPG_Mirror		= (pKNL_NodeInfo->ubHMirror == 1)?JPEG_H_MIRROR:(pKNL_NodeInfo->ubVMirror == 1)?JPEG_V_MIRROR:JPEG_MIRROR_DISABLE;
	tJpgDecInfo.ubJPG_Rotate		= (pKNL_NodeInfo->ubRotate == 1)?JPEG_ROT_90Deg:JPEG_ROT_DISABLE;

	tJpgCodecFnEs.tEM				= JPEG_QUEUE;
	tJpgCodecFnEs.pvEvent			= &KNL_QueueJpegMonit;
	tJpgCodecFnEs.tNotifyMode       = JPEG_FN_USE_ISR;
	JPEG_Decode_Setup(tJpgDecInfo, tJpgCodecFnEs);	

	JPEG_Codec_Enable();	

	if(tJpgCodecFnEs.tNotifyMode == JPEG_FN_USE_POLLING)
	{
		while(!JPEG->JPG_TX_OK_FLAG);
		JPEG->CLR_JPG_TX_OK_FLAG = 1;
		JPEG_Codec_Disable();
		osSemaphoreRelease(JPEG_CodecSem);
		return JPEG_DECODE_OK;
	}
	return JPEG_DECODE_OK;
}
//------------------------------------------------------------------------------
void KNL_SetVdoRoleInfoCbFunc(pvRoleSrcMap VdoRoleMap_cb)
{
	ptKNL_VdoRoleSrcMapT = VdoRoleMap_cb;
}
//------------------------------------------------------------------------------
void KNL_SetAdoRoleInfoCbFunc(pvRoleSrcMap AdoRoleMap_cb)
{
	ptKNL_AdoRoleSrcMapT = AdoRoleMap_cb;
}
//------------------------------------------------------------------------------
uint8_t KNL_GetRssiValue(KNL_ROLE tKNL_Role)
{
	GET_RSSI_ROLE tBB_RssiRole[6] = {[KNL_STA1] 	 = BB_GET_STA1_RSSI,
								     [KNL_STA2] 	 = BB_GET_STA2_RSSI,
									 [KNL_STA3] 	 = BB_GET_STA3_RSSI,
								     [KNL_STA4] 	 = BB_GET_STA4_RSSI,
									 [KNL_SLAVE_AP]  = BB_GET_SLAVE_AP_RSSI,
									 [KNL_MASTER_AP] = BB_GET_MASTER_AP_RSSI};
	return ubBB_GetRssiValue(tBB_RssiRole[tKNL_Role]);
}
//------------------------------------------------------------------------------
uint8_t KNL_GetPerValue(KNL_ROLE tKNL_Role)
{
	GET_PER_ROLE tBB_PerRole[6] = 	{[KNL_STA1] 	 = BB_GET_STA1_PER,
								     [KNL_STA2] 	 = BB_GET_STA2_PER,
									 [KNL_STA3] 	 = BB_GET_STA3_PER,
								     [KNL_STA4] 	 = BB_GET_STA4_PER,
									 [KNL_SLAVE_AP]  = BB_GET_SLAVE_AP_PER,
									 [KNL_MASTER_AP] = BB_GET_MASTER_AP_PER};
	return (100 - ubBB_GetPer(BB_HEAD_PER, tBB_PerRole[tKNL_Role]));
}
//------------------------------------------------------------------------------
void KNL_EnableWORFunc(void)
{
	uint8_t ubKNL_PsValue = 0;

	if(ubKNL_WorSts)
		osDelay(50);
	ubKNL_PsValue  = wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR);
	ubKNL_PsValue &= 0xF;
	ubKNL_PsValue |= RTC_PS_WOR_TAG;
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, ubKNL_PsValue);
	RTC_SetGPO_0(0, RTC_PullDownEnable);
	ubRTC_GetGPI_0(RTC_PullDownDisable);
	RTC_WakeupByGPI0Enable();
	BB_EnableWOR(96, 53);
	RTC_PowerDisable();
}
//------------------------------------------------------------------------------
void KNL_DisableWORFunc(void)
{
	uint8_t ubKNL_PsValue = 0;

	ubKNL_PsValue  = wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR);
	ubKNL_PsValue &= 0xF;
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, ubKNL_PsValue);
	RTC_WakeupByGPI0Disable();
}
//------------------------------------------------------------------------------
uint8_t KNL_WakeupDevice(KNL_ROLE tKNL_Role, uint8_t ubMode)
{
	LINK_ROLE tBB_LinkRole[KNL_MAX_ROLE] = {[KNL_STA1] = BB_STA1,
								            [KNL_STA2] = BB_STA2,
								            [KNL_STA3] = BB_STA3,
								            [KNL_STA4] = BB_STA4,
								            [KNL_MASTER_AP] = BB_MASTER_AP};
#ifdef OP_AP
	if(tKNL_Role > KNL_STA4)
#endif
#ifdef OP_STA
	if(tKNL_Role != KNL_MASTER_AP)
#endif
		return FALSE;
	if((FALSE == ubMode) && (FALSE == ubKNL_WakeUpFlag[tKNL_Role]))
		return TRUE;
	BB_SetWakeUp(ubMode, tBB_LinkRole[tKNL_Role]);
	ubKNL_WakeUpFlag[tKNL_Role] = ubMode;
#ifdef OP_STA
	if(ubMode)
		BB_ClearTxBuf(BB_TX_MASTER,BB_DATA_VIDEO);
#endif
	printd(DBG_Debug3Lvl, "  >WakeUp %s[%X]\n", (ubMode)?"?":"", tBB_LinkRole[tKNL_Role]);
	return TRUE;
}
//------------------------------------------------------------------------------
void KNL_TurnOnTuningTool(void)
{
#if OP_STA
	KNL_SetVdoGop(15);
	SEN_SetUvcPathFlag(1);
	IQ_SetupTuningToolMode(KNL_TUNINGMODE_ON);
	tKNL_TuningMode = KNL_TUNINGMODE_ON;
#endif
}
//------------------------------------------------------------------------------
void KNL_TurnOffTuningTool(void)
{
#if OP_STA
	SEN_SetUvcPathFlag(0);
	IQ_SetupTuningToolMode(KNL_TUNINGMODE_OFF);
	tKNL_TuningMode = KNL_TUNINGMODE_OFF;
#endif
}
//------------------------------------------------------------------------------
KNL_TuningMode_t KNL_GetTuningToolMode(void)
{
#if (OP_AP || !USBD_ENABLE)
	tKNL_TuningMode = KNL_TUNINGMODE_OFF;
#endif
	return tKNL_TuningMode;
}
//------------------------------------------------------------------------------

#define AUDIO_PLAY_BUF_MALLOC           0x400	// 1KB malloc size for buf
#define	AUDIO_WIFI_SEND_BUFF_TH			0x100
uint8_t	MENINITVALUE[AUDIO_PLAY_BUF_MALLOC]	 = {
	0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes	

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes	

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//16Bytes
};

uint32_t ul1ms_cnt=0;
uint32_t APP_TIMER_Get1ms(void)
{
	return ul1ms_cnt;
}

uint32_t ulAPP_ADO_GetMemInitValueAdr(void)
{
	return (uint32_t) MENINITVALUE;
}

uint32_t ulAPP_ADO_GetWifiPacketSize(void)
{
	return AUDIO_WIFI_SEND_BUFF_TH;
}

void APP_RecordOnceEnd_SDK(void)
{
	printd(DBG_Debug3Lvl, "record once finish!!\r\n");
}

void APP_1MSTrigger(void)
{
	ul1ms_cnt++;
}

void APP_1MSCounter(void)
{
	TIMER_SETUP_t TmSetup;

	TmSetup.tCLK 		= TIMER_CLK_EXTCLK;
	TmSetup.ulTmLoad 	= 10000;
	TmSetup.ulTmCounter = TmSetup.ulTmLoad;
	TmSetup.ulTmMatch1 	= TmSetup.ulTmLoad + 1;
	TmSetup.ulTmMatch2 	= TmSetup.ulTmLoad + 1;
	TmSetup.tOF 		= TIMER_OF_ENABLE;
	TmSetup.tDIR 		= TIMER_DOWN_CNT;
	TmSetup.tEM 		= TIMER_CB;
	TmSetup.pvEvent 	= APP_1MSTrigger;
	TIMER_Start(TIMER2_1, TmSetup);
}
