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
	\version	1.18
	\date		2018/09/05
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
#include "FWU_API.h"
#include "LCD.h"
#include "SEN.h"
#include "TIMER.h"
#include "WDT.h"
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
uint8_t ubKNL_ChgResFlg = 0;
KNL_SrcLocateMap_t KNL_SwDispInfo;
uint8_t ubKNL_SysStopFlag;
uint8_t ubKNL_BbPathAct;
KNL_SrcLocateMap_t KNL_SwDispInfo;

//For Process
KNL_INFO tKNL_Info;
osMessageQId KNL_VdoInProcQueue;
osMessageQId KNL_VdoCodecProcQueue;
osMessageQId KNL_AdoCodecProcQueue;
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

//For Capture
#define osKNL_CapFinSignal			0x33
#define osKNL_PhotoPlyFinSignal		0x35
osThreadId osKNL_RecordThreadId;
osMessageQId osKNL_RecordMsgQue;
osMutexId osKNL_RecordFuncMutex;
KNL_RecordAct_t tKNL_RecordAct;
LCD_BUF_TYP *pKNL_LcdPlayBuf;
char cKNL_LatestFileName[FS_FILE_NAME_MAX_LENGTH];
#if (KNL_PHOTOGRAPH_FUNC_ENABLE || KNL_REC_FUNC_ENABLE)
static void KNL_RecordThread(void const *argument);
#endif

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
pvRoleMap2Src ptKNL_VdoRoleMap2SrcNum;

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
uint8_t ubKNL_UsbdAdoEncAct    = 0;
uint8_t ubKNL_UsbdAdoEncStFlag = TRUE;
pvRoleMap2Src ptKNL_AdoRoleMap2SrcNum;
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
uint8_t ubKNL_AppResendIFrmFlg[4] = {FALSE, FALSE, FALSE, FALSE};
uint8_t ubKNL_UsbdVdoViewType;

//For Display
LCD_BUF_TYP *pLcdCh0Buf,*pLcdCh1Buf,*pLcdCh2Buf,*pLcdCh3Buf;
uint8_t ubKNL_DispCh0ActiveFlg=1,ubKNL_DispCh1ActiveFlg=1,ubKNL_DispCh2ActiveFlg=1,ubKNL_DispCh3ActiveFlg=1;
LCD_INFOR_TYP *pLcdCropScaleParam;
uint8_t ubKNL_LcdDispParamActiveFlg=1;

//For JPEG LCD Display
KNL_JpgLcdChCtrl_t tKNL_JpgLcdChCtrl;
osMutexId osKNL_JpgLcdChMutex;

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

pvKNL_BbFrmOkCbFunc ptKNL_BbFrmMonitCbFunc;

//WOR
uint8_t ubKNL_WorSts;

//For FS
FS_KNL_PARA_t tKNL_FsParam;

//For Record
uint8_t ubKNL_StartRecFlag = 0;

//For USB Tuning tool
KNL_TuningMode_t tKNL_TuningMode;

#ifdef VBM_PU
uint16_t ubTest_uwCropHsize= 720;
uint16_t ubTest_uwCropVsize= 1280;
uint16_t ubTest_uwCropHstart= 0;
uint16_t ubTest_uwCropVstart= 0;
#endif

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
//------------------------------------------------------------------------------

#define KNL_MAJORVER    1        //!< Major version = 1
#define KNL_MINORVER    18       //!< Minor version = 18
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
	tKNL_AdoInfo.WavPlay_buf_size 		    = tAdoInfo.WavPlay_buf_size;
	tKNL_AdoInfo.AAC_En_buf_size     		= tAdoInfo.AAC_En_buf_size;
	tKNL_AdoInfo.AAC_De_buf_size     		= tAdoInfo.AAC_De_buf_size;
	tKNL_AdoInfo.Alarm_buf_size             = tAdoInfo.Alarm_buf_size;
	tKNL_AdoInfo.Alaw_Dec_buf_size          = tAdoInfo.Alaw_Dec_buf_size;
	tKNL_AdoInfo.Recording_buf_size         = tAdoInfo.Recording_buf_size;
	tKNL_AdoInfo.Rec_buf_th					= tAdoInfo.Rec_buf_th;
	tKNL_AdoInfo.Ply_buf_th        			= tAdoInfo.Ply_buf_th;
}
//------------------------------------------------------------------------

void KNL_SetVdoGop(uint32_t ulGop)
{
	tKNL_Info.ulGop = ulGop;
}

//------------------------------------------------------------------------------
uint32_t ulKNL_GetVdoGop(void)
{
	return tKNL_Info.ulGop;
}

//------------------------------------------------------------------------------
uint32_t ulKNL_GetVdoFrmIdx(uint8_t ubCh)
{
	return ulKNL_CurFrmIdx[ubCh];
}

//------------------------------------------------------------------------------
void KNL_Init(void)
{
	KNL_ROLE tKNLRole;

	ubKNL_SysStopFlag		= FALSE;
	ubKNL_BbPathAct			= FALSE;
	pLcdCropScaleParam 		= NULL;
	ptKNL_VdoRoleMap2SrcNum = NULL;
	ptKNL_AdoRoleMap2SrcNum = NULL;
	tKNL_Info.ubDisp1SrcNum = KNL_SRC_NONE;
	tKNL_Info.ubDisp2SrcNum = KNL_SRC_NONE;
	tKNL_Info.ubDisp3SrcNum = KNL_SRC_NONE;
	tKNL_Info.ubDisp4SrcNum = KNL_SRC_NONE;
	tKNL_TuningMode 		= KNL_TUNINGMODE_OFF;
	ubKNL_ChgResFlg			= FALSE;
	ubKNL_ImgBusyFlg		= FALSE;
	ubKNL_WorSts			= 1;
	ubKNL_UsbdAdoEncAct		= 0;
	ubKNL_UsbdAdoEncStFlag	= TRUE;
	ubKNL_UsbdVdoViewType	= 1;
	KNL_SwDispInfo.ubSetupFlag = FALSE;
	for(tKNLRole = KNL_STA1; tKNLRole <= KNL_STA4; tKNLRole++)
	{
		KNL_AvgThreadId[tKNLRole]			= NULL;
		KNL_AvgPlyQueue[tKNLRole]			= NULL;
		ubKNL_VdoBsBusyCnt[tKNLRole]		= 0;
		ubKNL_VdoResendITwcFlg[tKNLRole]	= FALSE;
		ubKNL_VdoChkSrcNumFlg[tKNLRole]	 	= FALSE;
		ubKNL_VdoResChgTwcFlg[tKNLRole]     = FALSE;
		ubKNL_AppResendIFrmFlg[tKNLRole]	= FALSE;
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
	tKNL_LinkSem = osMutexCreate(osMutex(tKNL_LinkSem));

	osSemaphoreDef(tKNL_ImgSem);
	tKNL_ImgSem	= osSemaphoreCreate(osSemaphore(tKNL_ImgSem), 1);

	osSemaphoreDef(tKNL_TwcSem);
	tKNL_TwcSem	= osSemaphoreCreate(osSemaphore(tKNL_TwcSem), 1);

	osMutexDef(tKNL_BsBufMutex);
	tKNL_BsBufMutex = osMutexCreate(osMutex(tKNL_BsBufMutex));

	osMessageQDef(KNL_VDOPROCESS, KNL_PROC_QUEUE_NUM, KNL_PROCESS);
	KNL_VdoCodecProcQueue = osMessageCreate(osMessageQ(KNL_VDOPROCESS), NULL);

	osMessageQDef(KNL_ADOPROCESS, KNL_PROC_QUEUE_NUM, KNL_PROCESS);
	KNL_AdoCodecProcQueue = osMessageCreate(osMessageQ(KNL_ADOPROCESS), NULL);

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

	tKNL_JpgLcdChCtrl = JPG_LCDCH_DISABLE;
	osMutexDef(KNL_JpgLcdCtrlMutex);
	osKNL_JpgLcdChMutex = osMutexCreate(osMutex(KNL_JpgLcdCtrlMutex));

	tKNL_RecordAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
	for(tKNLRole = KNL_STA1; tKNLRole <= KNL_STA4; tKNLRole++)
		tKNL_RecordAct.ubPhotoCapSrc[tKNLRole] = KNL_SRC_NONE;
	osKNL_RecordMsgQue = NULL;
#if (KNL_PHOTOGRAPH_FUNC_ENABLE || KNL_REC_FUNC_ENABLE)
	osMutexDef(KNL_RecordFuncMutex);
	osKNL_RecordFuncMutex = osMutexCreate(osMutex(KNL_RecordFuncMutex));
	osMessageQDef(KNL_RecordMsgQue, 1, KNL_RecordAct_t);
    osKNL_RecordMsgQue = osMessageCreate(osMessageQ(KNL_RecordMsgQue), NULL);
	osThreadDef(KNL_RecordThd, KNL_RecordThread, THREAD_PRIO_KNLRECORD_HANDLER, 1, THREAD_STACK_KNLRECORD_HANDLER);
	osKNL_RecordThreadId = osThreadCreate(osThread(KNL_RecordThd), NULL);
#endif
}
//------------------------------------------------------------------------------
void KNL_Stop(void)
{
	if(TRUE == ubKNL_SysStopFlag)
		return;
	BB_Stop();
	ubKNL_SysStopFlag = TRUE;
}
//------------------------------------------------------------------------------
void KNL_ReStart(void)
{
	if(FALSE == ubKNL_SysStopFlag)
		return;
	BB_Start(THREAD_STACK_BB_HANDLER, THREAD_PRIO_BB_HANDLER);
	ubKNL_SysStopFlag = FALSE;
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
		for(i = 0; i < KNL_SRC_NUM; i++)
		{
			ulKNL_OutVdoFpsCnt[i] = ulKNL_OutVdoFpsCntTemp[i];
			ulKNL_OutVdoFpsCntTemp[i] = 0;
//			ulKNL_InVdoFpsCnt[i] = ulKNL_InVdoFpsCntTemp[i];
//			ulKNL_InVdoFpsCntTemp[i] = 0;
		}
		for(i = 0; i < KNL_MAX_ROLE; i++)
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
			else if(tKNL_AdoInfo.Compress_method == COMPRESS_ALAW)
			{
				printd(DBG_Debug3Lvl, "AdoEncQ(ALAW) : Addr:0x%x,SZ:0x%x\r\n",tAdoInfo.PcmAddr,tAdoInfo.PcmSize);
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
//			KNL_AdcBufProcess(tProcess);
			if(osMessagePut(KNL_AdoCodecProcQueue, &tProcess, 0) == osErrorResource)
			{
				printd(DBG_ErrorLvl, "KNL_ADO Q->Full !!!!\r\n");
			}
		}
	}
}
//------------------------------------------------------------------------------
static void KNL_AdoDecMonitThread(void const *argument)
{	
	ADO_DAC_EVENT tKNL_DacEvent;

	while(1)
	{
		//Get Queue from Audio Interface
		osMessageGet(tKNL_DecEventQue, &tKNL_DacEvent, osWaitForever);
		switch(tKNL_DacEvent)
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

				SPEAKER_EN(TRUE);
				#endif

				#if VBM_BU
				SPEAKER_EN(TRUE);
				//SPEAKER_EN(FALSE);
				#endif
				break;
			case PLAY_BUF_EMP:
				printd(DBG_CriticalLvl, "-Dac play empty-\n");
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
	#ifdef OP_AP
	if(tUSBD_GetClassMode() == USBD_COMPOSITE_MODE)
	{
	#define KNL_USBDUVC_AUX_SIZE		(32)
	#define KNL_USBDMSC_AUX_SIZE		(16)
	#define USBDVDO_SUBFRAME_SIZE		(4 * 1024)
	#define USBDADO_SUBFRAME_SIZE		(7 * AUDIO32_ENCODE_SIZE)
	#define UVC_OFS_CH_INFO				(12)
	#define UVC_OFS_AUX_INFO			(13)
	#define UVC_OFS_TIME_STAMP			(14)
	#define UVC_OFS_NUM_SUB_FRAME		(22)
	#define UVC_OFS_SUB_FRAME_IDX		(23)
	#define UVC_OFS_SUB_FRAME_LEN		(24)
	#define UVC_OFS_TOTAL_FRAME_IDX		(26)
	#define UVC_OFS_RESERVED			(27)
	#define UVC_OFS_SUB_FRAME_DATA		KNL_USBDUVC_AUX_SIZE
		fBufSize = USBDVDO_SUBFRAME_SIZE + KNL_USBDUVC_AUX_SIZE + USBD_UVC_DUMMY_SZ;
		BUF_BufInit(BUF_USBD_VDO, BUF_NUM_USBD_VDO, fBufSize, 0);
		BUF_Reset(BUF_USBD_VDO);
		fBufSize = USBDADO_SUBFRAME_SIZE + KNL_USBDUVC_AUX_SIZE + USBD_UVC_DUMMY_SZ;
		BUF_BufInit(BUF_USBD_ADO, BUF_NUM_USBD_ADO, fBufSize, 0);
		BUF_Reset(BUF_USBD_ADO);
	}
	#endif
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
		if(ubKNL_ExistNode(KNL_SRC_1_OTHER_A+i,KNL_NODE_DAC_BUF) || ubKNL_ExistNode(KNL_SRC_1_OTHER_B+i,KNL_NODE_DAC_BUF))
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
            fBufSize = ISP_WIDTH * ISP_HEIGHT * 2;		//!< ((float)uwKNL_GetVdoH(ubSrc))*((float)uwKNL_GetVdoV(ubSrc))*2;
		} else {
            fBufSize = ISP_WIDTH * ISP_HEIGHT * 1.5;	//!< ((float)uwKNL_GetVdoH(ubSrc))*((float)uwKNL_GetVdoV(ubSrc))*1.5;
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

	//LCD Node
	//================================================================
	ubNodeExist = ubKNL_ChkExistNode(KNL_NODE_LCD);
	if(ubNodeExist)
	{
		fBufSize = ulKNL_CalLcdBufSz();
		BUF_BufInit(BUF_LCD_IP,1,fBufSize,0);
	}
	//VDO_BS_BUF Node
	//================================================================
	//(For BS_BUF1)
	//Main
	for(i=0;i<4;i++)
	{
		if(ubKNL_ExistNode(KNL_SRC_1_MAIN+i,KNL_NODE_VDO_BS_BUF1))
		{
			tNodeInfo = tKNL_GetNodeInfo(KNL_SRC_1_MAIN+i,KNL_NODE_VDO_BS_BUF1);
//			tNodeInfo.uwVdoH = ISP_WIDTH;
//			tNodeInfo.uwVdoV = ISP_HEIGHT;
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

	BUF_Reset(BUF_RESV_YUV);
#if KNL_SD_FUNC_ENABLE
    //FS
	if(ubSD_ChkIFSetup())
	{
		uint32_t ulNeedMemSize;
		FS_RESOLUTION_MODE tResMode;

		tResMode = (KNL_REC_FUNC_ENABLE)?FS_RES_MODE_HDx4:FS_RES_MODE_HDx1;
        ulNeedMemSize = ulFS_GetTotalBufSize(tResMode);
		printd(DBG_Debug3Lvl, "FS Buf:0x%X\r\n", ulNeedMemSize);
		BUF_BufInit(BUF_FS,1,ulNeedMemSize,0);
	}

	//Photo
	#if KNL_PHOTOGRAPH_FUNC_ENABLE
	if(ubSD_ChkIFSetup())
	{
		fBufSize = KNL_JPG_HEADER_SIZE + KNL_JPG_BS_SIZE;
		BUF_BufInit(BUF_JPG_BS, (1 << ubKNL_GetOpMode()), fBufSize, 0);
		if((KNL_OPMODE_VBM_1T == ubKNL_GetOpMode()) ||
		   ((KNL_OPMODE_VBM_4T == ubKNL_GetOpMode()) && ((KNL_DISP_DUAL_C != tKNL_GetDispType()) && (KNL_DISP_DUAL_U != tKNL_GetDispType()))))
		{
			fBufSize = ISP_WIDTH * ISP_HEIGHT * 1.5;
			BUF_BufInit(BUF_RESV_YUV, 1, fBufSize, 0);
		}
	}
	#endif

	//REC
	#if KNL_REC_FUNC_ENABLE
    if(ubSD_ChkIFSetup())
    {
		uint32_t ulTotalMemSize, ulResiMemSize;

        ulTotalMemSize = DDR_BSZ_MAX;
        ulResiMemSize  = ulTotalMemSize - (ulBUF_GetBlkBufAddr(0,BUF_FS)+ulBUF_AlignAddrTo1K(ulFS_GetTotalBufSize(FS_RES_MODE_HDx4))) - 1024;
        BUF_BufInit(BUF_REC,1,ulResiMemSize,0);

        vREC_FileFormatSet(REC_FILE_MP4);
        Media_FileFormatConfigInit();
        printd(DBG_Debug3Lvl, "MaxRECTime %d minutes\r\n",ulREC_ModeSet(ulResiMemSize));
    }
	#endif
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
	//Rate-Control for IQ-Tuning
	RC_EngModeSet(0,KNL_RC_BITRATE,KNL_RC_FPS);	//CodecIdx:0,Target BitRate:150KB,FPS:15
	
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
			BB_Init(tKNL_BbSlotMode, NULL, 0, 0x2B, 0x2B, 0, 165, APP_ADOENC_TYPE);	//Step(n)
			ubKNL_WorSts = BB_ConfirmWakeUpInf();
		}
		else
			BB_Init(tKNL_BbSlotMode, NULL, 1, 0x2B, 0x2B, 0, 165, APP_ADOENC_TYPE);	//Step(n)

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
//		ADO_SetSigmaDeltaAdcGain(ADO_SIG_BOOST_37DB, ADO_SIG_PGA_33DB);
#ifdef VBM_PU
		ADO_SetSigmaDeltaAdcGain(ADO_SIG_BOOST_0DB, ADO_SIG_PGA_16p5DB); // 20180524
#endif

#ifdef VBM_BU
		//ADO_SetSigmaDeltaAdcGain(ADO_SIG_BOOST_0DB, ADO_SIG_PGA_16p5DB); // 20180524
#endif
		SDADC->AGC_OFF = 1;
		
		//---------------------------------
		// ADC djust functions
		//---------------------------------
		ADO_SetAdcDcComp(0xA, 0x14, 0x25, ADO_OFF);	// DC compensation

		//---------------------------------
		// DAC djust functions
		//---------------------------------
		//ADO_SetDacR2RVol(R2R_VOL_n0DB);	// DAC R2R volume
		ADO_SetDacM2so(ADO_OFF);		// M2SO
		
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
		ADO_SetDacGain(DAC_GAIN_n14DB, ADO_ON);
#endif
		//====================================	

		osThreadDef(ADOEncProcessThread, KNL_AdoEncMonitThread, THREAD_PRIO_ADO_ENC_PROC, 1, THREAD_STACK_ADO_PROC);
		osThreadCreate(osThread(ADOEncProcessThread), NULL);

		osThreadDef(ADODecProcessThread, KNL_AdoDecMonitThread, THREAD_PRIO_ADO_DEC_PROC, 1, THREAD_STACK_ADO_PROC);
		osThreadCreate(osThread(ADODecProcessThread), NULL);

		ubKNL_InitAdoFlg = 1;
	}

	//For JPEG Codec
	//=======================================================================
	if((KNL_PHOTOGRAPH_FUNC_ENABLE || ubKNL_ChkExistNode(KNL_NODE_JPG_ENC) || ubKNL_ChkExistNode(KNL_NODE_JPG_DEC1) || ubKNL_ChkExistNode(KNL_NODE_JPG_DEC2)) && (ubKNL_InitJpegFlg == 0))
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

#if KNL_PHOTOGRAPH_FUNC_ENABLE
	KNL_UpdateJpgHeader();
#endif

#if USBD_ENABLE
	USBD_Start();
	#ifdef OP_AP
	if(tUSBD_GetClassMode() == USBD_COMPOSITE_MODE)
	{
		ADO_SetOutputCbFunc(KNL_UpdateUsbdAdoData);
		tUSBD_RegMscCbFunc(USBDM_OPC_ADO, KNL_RecvAdoDataFromApp);
		tUSBD_RegUvcReleaseBuffCbFunc(KNL_ReleaseUsbdBuf);
	}
	#endif
#endif

#if KNL_SD_FUNC_ENABLE
    //For FS
    if(ubSD_ChkIFSetup())
    {
        tKNL_FsParam.ulFS_BufStartAddr = ulBUF_GetBlkBufAddr(0,BUF_FS);
        tKNL_FsParam.Mode = FS_RES_MODE_HDx1;
        FS_Init(&tKNL_FsParam);
		FS_SetMaxRollingValOfGrpIdx(65536);
        printd(DBG_Debug3Lvl, "FS buf %d bytes\r\n",ulFS_GetTotalBufSize(FS_RES_MODE_HDx1));
    }

	#if KNL_REC_FUNC_ENABLE	
    //For REC
	if(ubSD_ChkIFSetup())
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
#else
	SD_FunctionDisable;
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
//------------------------------------------------------------------------------
uint8_t ubKNL_GetOpMode(void)
{
	return tKNL_Info.ubOpMode;
}
//------------------------------------------------------------------------------
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
#define KNL_AUX_INFO_LEN		(16 * 3)
#define KNL_INFO_FRMSEQ			(1)
#define KNL_INFO_RES			(2)
#define KNL_INFO_VGOP			(6)
#define KNL_INFO_GOP			(7)
#define KNL_INFO_FRMIDX			(11)
#define KNL_INFO_OPMODE			(15)
#define KNL_INFO_SRCNUM			(16)
#define KNL_INFO_TIMESTP_LSB	(20)
#define KNL_INFO_TIMESTP_MSB	(24)
#define KNL_INFO_PADD			(25)
#define KNL_INFO_ADOSIZE		(3)
//	uint8_t ubCrc8 = 0;
//	CRC_t tCRC2_Setup;
	uint32_t ulTemp;
	uint32_t ulRtnSz;
	uint32_t i;
	uint32_t ulDmy0Time = 0;
//	uint32_t ulCrcCalSz;
	uint8_t ubVdoIdx = 0;
	static uint8_t ubKNL_VdoFrameSeq[2] = {0, 0};
	static uint8_t ubKNL_AdoFrameSeq = 0;

	//Case (%16) = 0
	//-------------------------------------------
	//|							 				|
	//| Original Size			 				|
	//|							 				|
	//-------------------------------------------
	//|TimeStamp (24)  			 				|
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
			ulRtnSz    = ulTemp + KNL_AUX_INFO_LEN;
//			ulCrcCalSz = ulTemp;
		}
		else
		{
			ulRtnSz    = ((ulTemp / 16) * 16) + 16 + KNL_AUX_INFO_LEN;
//			ulCrcCalSz = ((ulTemp / 16) * 16) + 16;
			//Padd 0 value
			ulDmy0Time = 16 - (ulTemp % 16);
			for(i = 0; i < ulDmy0Time; i++)
			{
				*((uint8_t *)(ulAddr + ulSize + i)) = 0;
			}
		}

		//CRC-Information
//		if(KNL_AUX_CRC_FUNC)
//		{
//			ulCrcCalSz = ulCrcCalSz;
//			tCRC2_Setup.CRC_INIT_VALUE = INIT_ALL_ZERO;
//			tCRC2_Setup.CRC_FINAL_XOR_VALUE = XOR_ALL_ZERO;
//			tCRC2_Setup.CRC_ORDER = 7;
//			ubCrc8 = (uint8_t)ulCRC2_Calc(tCRC2_Setup, CRC_P_8, ulAddr, ulCrcCalSz);
//			//printd(DBG_Debug3Lvl, "crc2:0x%x\r\n",ubCrc8);
//			//printd(DBG_Debug3Lvl, "ulCrcCalSz:0x%x\r\n",ulCrcCalSz);
//			
//			*((uint32_t *)(ulAddr+ulRtnSz-16-16-16)) = ulCrcCalSz;
//			*((uint32_t *)(ulAddr+ulRtnSz-16-16)) = (uint32_t)ubCrc8;
//		}

		//Aux-Information
		
		//Padd
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_PADD))  		 = (uint8_t)ulDmy0Time;

		//Time Stamp
		*((uint32_t *)(ulAddr + ulRtnSz - KNL_INFO_TIMESTP_MSB)) = 0;
		*((uint32_t *)(ulAddr + ulRtnSz - KNL_INFO_TIMESTP_LSB)) = 0;
		
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_SRCNUM)) 		 = ubSrcNum;								//SrcNum Information
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_OPMODE)) 		 = ubKNL_GetOpMode();						//OpMode Information

		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_FRMIDX + 3))) = (uint8_t)(((ulFrmIdx&0x000000FF)>>0));	//Frame Index Information
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_FRMIDX + 2))) = (uint8_t)(((ulFrmIdx&0x0000FF00)>>8));
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_FRMIDX + 1))) = (uint8_t)(((ulFrmIdx&0x00FF0000)>>16));
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_FRMIDX)) 		 = (uint8_t)(((ulFrmIdx&0xFF000000)>>24));

		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_GOP + 3))) 	 = (uint8_t)(((ulGop&0x000000FF)>>0));		//Codec GOP Information
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_GOP + 2)))  	 = (uint8_t)(((ulGop&0x0000FF00)>>8));
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_GOP + 1)))  	 = (uint8_t)(((ulGop&0x00FF0000)>>16));
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_GOP))  		 = (uint8_t)(((ulGop&0xFF000000)>>24));

		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_VGOP))  		 = ubVdoGroupIdx;							//Video Group Information
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_RES + 3)))  	 = uwKNL_GetVdoH(ubSrcNum) >> 8;
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_RES + 2)))  	 = uwKNL_GetVdoH(ubSrcNum) & 0xFF;
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_RES + 1)))  	 = uwKNL_GetVdoV(ubSrcNum) >> 8;
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_RES))  		 = uwKNL_GetVdoV(ubSrcNum) & 0xFF;
		ubVdoIdx = (ubSrcNum <= KNL_SRC_4_MAIN)?0:1;
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_FRMSEQ))  	 = ubKNL_VdoFrameSeq[ubVdoIdx]++;		
	}
	else if(tPktType == KNL_ADO_PKT)
	{
		ulTemp = ulSize;
		if((ulTemp % KNL_ADO_SUB_PKT_LEN) == 0)
		{
			ulRtnSz = ulTemp + KNL_ADO_SUB_PKT_LEN;
		}
		else
		{
			if((ulTemp + 16) <= (((ulTemp / KNL_ADO_SUB_PKT_LEN) * KNL_ADO_SUB_PKT_LEN) + KNL_ADO_SUB_PKT_LEN))
			{
				ulRtnSz = ((ulTemp / KNL_ADO_SUB_PKT_LEN) * KNL_ADO_SUB_PKT_LEN) + KNL_ADO_SUB_PKT_LEN;
			}
			else
			{
				ulRtnSz = ((ulTemp / KNL_ADO_SUB_PKT_LEN) * KNL_ADO_SUB_PKT_LEN) + (KNL_ADO_SUB_PKT_LEN * 2);
			}
		}
		//Aux-Information
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_SRCNUM)) = ubSrcNum;					//SrcNum Information

		//Time Stamp
		*((uint32_t *)(ulAddr + ulRtnSz - KNL_INFO_TIMESTP_MSB))   = 0;
		*((uint32_t *)(ulAddr + ulRtnSz - KNL_INFO_TIMESTP_LSB))   = 0;
		

		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_ADOSIZE + 3)))  = (uint8_t)(((ulSize&0x000000FF)>>0));		//Real-Size Information
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_ADOSIZE + 2)))  = (uint8_t)(((ulSize&0x0000FF00)>>8));
		*((uint8_t *)(ulAddr + ulRtnSz - (KNL_INFO_ADOSIZE + 1)))  = (uint8_t)(((ulSize&0x00FF0000)>>16));
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_ADOSIZE))   	   = (uint8_t)(((ulSize&0xFF000000)>>24));
		*((uint8_t *)(ulAddr + ulRtnSz - KNL_INFO_FRMSEQ))  	   = ubKNL_AdoFrameSeq++;
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
				ubKNL_AppResendIFrmFlg[ubRole] = FALSE;
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
	static uint8_t ubDualDectFlag = FALSE;

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

		KNL_SwDispInfo.ubSetupFlag = TRUE;	//! FALSE;
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
			
			sLcdInfor.tChRes[0].uwCropHstart = ubTest_uwCropHstart;
			sLcdInfor.tChRes[0].uwCropVstart = ubTest_uwCropVstart;
			sLcdInfor.tChRes[0].uwCropHsize  = ubTest_uwCropHsize; // uwKNL_GetVdoV(ubDisp1Src);
			sLcdInfor.tChRes[0].uwCropVsize  = ubTest_uwCropVsize; // uwKNL_GetVdoH(ubDisp1Src);
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

			if(TRUE == ubDualDectFlag)
			{
				for(i = 0; i < 2; i++)
					ubRole[i] = ubKNL_SrcNumMap(KNL_SwDispInfo.tSrcNum[i]);
				if(ubKNL_GetCommLinkStatus(ubRole[0]) == BB_LOST_LINK)
					LCD_ChDisable(LCD_CH1);
				if(ubKNL_GetCommLinkStatus(ubRole[1]) == BB_LOST_LINK)
					LCD_ChDisable(LCD_CH0);
			}
			else
				ubDualDectFlag = FALSE;
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

			if(TRUE == ubDualDectFlag)
			{
				for(i = 0; i < 2; i++)
					ubRole[i] = ubKNL_SrcNumMap(KNL_SwDispInfo.tSrcNum[i]);
				if(ubKNL_GetCommLinkStatus(ubRole[0]) == BB_LOST_LINK)
					LCD_ChDisable(LCD_CH1);
				if(ubKNL_GetCommLinkStatus(ubRole[1]) == BB_LOST_LINK)
					LCD_ChDisable(LCD_CH0);
			}
			else
				ubDualDectFlag = TRUE;
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
/*        printd(1,"KNL_SetDispSrc\n");
        printd(1," tDispLocation %d \n ",tDispLocation);
        printd(1," ubDispSrcNum %d \n ",ubDispSrcNum);

        printd(1," tKNL_Info.ubDisp1SrcNum %d \n ",tKNL_Info.ubDisp1SrcNum);
        printd(1," tKNL_Info.ubDisp2SrcNum %d \n ",tKNL_Info.ubDisp2SrcNum);
*/        

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
/*        printd(1," tKNL_GetDispLocation \n ");
        printd(1," ubSrcNum %d \n ",ubSrcNum);
        printd(1," tKNL_Info.ubDisp1SrcNum %d \n ",tKNL_Info.ubDisp1SrcNum);
        printd(1," tKNL_Info.ubDisp2SrcNum %d \n ",tKNL_Info.ubDisp2SrcNum);
*/

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
#if OP_STA
uint8_t ubKNL_UpdateUvcImage(KNL_PROCESS tProc, KNL_NODE tCurNode)
{
	static uint8_t ubUvcChgResFlag = FALSE;
	static uint16_t uwKNL_ImgHSize = 0, uwKNL_ImgVSize = 0;
	KNL_NODE_INFO tNodeInfo;
	uint8_t ubUvcPathMode;

	tNodeInfo 	  = tKNL_GetNodeInfo(tProc.ubSrcNum, tCurNode);
	ubUvcPathMode = UVC_GetVdoFormat();
	if(USB_UVC_VS_FORMAT_UNDEFINED != ubUvcPathMode)
	{
		if(0xFF != ubKNL_SrcNumMap(tProc.ubSrcNum))
		{
			uint16_t uwVdoHSize = uwKNL_GetVdoH(tProc.ubSrcNum);
			uint16_t uwVdoVSize = uwKNL_GetVdoV(tProc.ubSrcNum);

			switch(ubUvcPathMode)
			{
				case USB_UVC_VS_FORMAT_FRAME_BASED:
					if(TRUE == ubUVC_CheckResolution(uwVdoHSize, uwVdoVSize))
					{
						SEN_SetUvcPathFlag(1);
						if(FALSE == uvc_update_image((uint32_t *)tProc.ulDramAddr2, (uint32_t)tProc.ulSize))
						{
							if(tNodeInfo.ubPreNode == KNL_NODE_H264_ENC)
							{
								ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC, tProc.ubSrcNum, tProc.ulDramAddr2);
								return 0;
							}
						}
					}
					else
					{
						uint8_t ubResData[8];

						uwKNL_ImgHSize 	= uwVdoHSize;
						uwKNL_ImgVSize 	= uwVdoVSize;
						uwVdoHSize   	= UVC_GetVdoWidth();
						uwVdoVSize   	= UVC_GetVdoHeight();
						ubResData[0] 	= (uwVdoHSize & 0xFF00) >> 8;
						ubResData[1] 	= (uwVdoHSize & 0xFF);
						ubResData[2] 	= (uwVdoVSize & 0xFF00) >> 8;
						ubResData[3] 	= (uwVdoVSize & 0xFF);
						ubResData[4] 	= tProc.ubSrcNum;
						KNL_VdoResSetting(TWC_STA1, ubResData);
						printd(DBG_InfoLvl, "	->UVC Res: %d x %d\n", uwVdoHSize, uwVdoVSize);
						ubUvcChgResFlag = TRUE;
					}
					break;
				case USB_UVC_VS_FORMAT_UNCOMPRESSED:
                    SEN_SetUvcPathFlag(1);
					uvc_update_image((uint32_t *)tProc.ulDramAddr1, (uint32_t)(YUY2_WIDTH*YUY2_HEIGHT*2));
					break;
				default:
					return 1;
			}
			if(tNodeInfo.ubPreNode == KNL_NODE_H264_ENC)
			{
				if(ubRC_GetFlg(tNodeInfo.ubCodecIdx))
					ubRC_SetFlg(tNodeInfo.ubCodecIdx, FALSE);
				if(KNL_OPMODE_VBM_1T != ubKNL_GetOpMode())
				{
					ubKNL_ReleaseBsBufAddr(KNL_NODE_H264_ENC, tProc.ubSrcNum, tProc.ulDramAddr2);
					return 0;
				}
			}
		}
	}
	else
	{
		SEN_SetUvcPathFlag(0);
		if(tNodeInfo.ubPreNode == KNL_NODE_H264_ENC)
		{
			if(TRUE == ubUvcChgResFlag)
			{
				uint8_t ubResData[8];

				ubResData[0] = (uwKNL_ImgHSize & 0xFF00) >> 8;
				ubResData[1] = (uwKNL_ImgHSize & 0xFF);
				ubResData[2] = (uwKNL_ImgVSize & 0xFF00) >> 8;
				ubResData[3] = (uwKNL_ImgVSize & 0xFF);
				ubResData[4] = tProc.ubSrcNum;
				KNL_VdoResSetting(TWC_STA1, ubResData);
				printd(DBG_InfoLvl, "	<-Img Res: %d x %d\n", uwKNL_ImgHSize, uwKNL_ImgVSize);
				ubUvcChgResFlag = FALSE;
			}
			if(!ubRC_GetFlg(tNodeInfo.ubCodecIdx))
			{
				H264_SetGOP((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx, ulKNL_GetVdoGop());
				ubRC_SetFlg(tNodeInfo.ubCodecIdx, TRUE);
			}
		}
	}
	return 1;
}
#endif
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
#if KNL_LCD_FUNC_ENABLE
	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_LCD))
	{
		KNL_DISP_LOCATION tDispLocate;

		if((tKNL_GetDispType() == KNL_DISP_H) && (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0))
		{
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);

			if(tDispLocate == KNL_DISP_LOCATION2)
			{
				LCD_ChDisable(LCD_CH0);
			}
			else if(tDispLocate == KNL_DISP_LOCATION3)
			{
				LCD_ChDisable(LCD_CH2);
			}
			else if((tDispLocate == KNL_DISP_LOCATION1)||(tDispLocate == KNL_DISP_LOCATION4))
			{
				LCD_ChDisable(LCD_CH1);
			}
		}
		else if(tKNL_GetDispType() == KNL_DISP_SINGLE)
		{
			LCD_ChDisable(LCD_CH0);
		}
		else if((tKNL_GetDispType() == KNL_DISP_DUAL_U)||(tKNL_GetDispType() == KNL_DISP_QUAD))
		{
			if(TRUE == KNL_SwDispInfo.ubSetupFlag)
				tDispLocate = tKNL_SearchSrcLocation(ubSrcNum);
			else
				tDispLocate = tKNL_GetDispLocation(ubSrcNum);
			if(tDispLocate == KNL_DISP_LOCATION1)
			{
				LCD_ChDisable(LCD_CH0);
			}
			else if(tDispLocate == KNL_DISP_LOCATION2)
			{
				LCD_ChDisable(LCD_CH1);
			}
			else if(tDispLocate == KNL_DISP_LOCATION3)
			{
				LCD_ChDisable(LCD_CH2);
			}
			else if(tDispLocate == KNL_DISP_LOCATION4)
			{
				LCD_ChDisable(LCD_CH3);
			}		
		}
		else if(tKNL_GetDispType() == KNL_DISP_3T_1T2B)
		{
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);
			if(tDispLocate == KNL_DISP_LOCATION1)
			{
				LCD_ChDisable(LCD_CH0);
			}
			else if((tDispLocate == KNL_DISP_LOCATION2)||(tDispLocate == KNL_DISP_LOCATION3))
			{
				LCD_ChDisable(LCD_CH1);
			}
		}
		else if(tKNL_GetDispType() == KNL_DISP_3T_2T1B)
		{
			tDispLocate = tKNL_GetDispLocation(ubSrcNum);
			if((tDispLocate == KNL_DISP_LOCATION1)||(tDispLocate == KNL_DISP_LOCATION2))
			{
				LCD_ChDisable(LCD_CH0);
			}
			else if(tDispLocate == KNL_DISP_LOCATION3)
			{
				LCD_ChDisable(LCD_CH1);
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
				LCD_ChDisable(LCD_CH0);
			}
			else if(tDispLocate == KNL_DISP_LOCATION2)
			{
				LCD_ChDisable(LCD_CH2);
			}		
			else if(tDispLocate == KNL_DISP_LOCATION3)
			{
				LCD_ChDisable(LCD_CH1);
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
				LCD_ChDisable(LCD_CH1);
			}
			else if(tDispLocate == KNL_DISP_LOCATION2)
			{
				LCD_ChDisable(LCD_CH0);
			}
			else if(tDispLocate == KNL_DISP_LOCATION3)
			{
				LCD_ChDisable(LCD_CH2);
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
				LCD_ChDisable(LCD_CH0);
			}
			else if(tDispLocate == KNL_DISP_LOCATION2)
			{
				LCD_ChDisable(LCD_CH1);
			}
		}
	}
#endif
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
		uint8_t ubCodecRetryNum = 0, ubCodecRetryCnt = (KNL_NODE_H264_DEC == tProc.ubNextNode)?18:30;

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
			if((KNL_PHOTO_PLAY == tKNL_GetRecordFunc()) || (JPG_LCDCH_ENABLE == tKNL_GetJpegLcdChCtrl()))
			{
				ulAddr = ulKNL_GetResvDecAddr();
				printd(DBG_InfoLvl, "		>H264 DEC: 0x%X\n", ulAddr);
			}
			else
			{
				KNL_LcdDisplaySetting();
				ulAddr = ulKNL_GetLcdDispAddr(tProc.ubSrcNum);
			}
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
				H264_SetRotationEn((H264_DECODE_INDEX)tNodeInfo.ubCodecIdx, (TRUE == ubKNL_SearchCapSrcNum(tProc.ubSrcNum))?H264_DISABLE:H264_ENABLE);
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

#if OP_STA
	if(KNL_GetTuningToolMode() == KNL_TUNINGMODE_ON)
	{
		if(!ubKNL_UpdateUvcImage(tProc, KNL_NODE_VDO_BS_BUF1))
			return;
	}
#endif

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
				if((KNL_GetTuningToolMode() == KNL_TUNINGMODE_OFF) ||
				   (USB_UVC_VS_FORMAT_UNDEFINED == UVC_GetVdoFormat()))
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
				if((KNL_GetTuningToolMode() == KNL_TUNINGMODE_OFF) ||
				   (USB_UVC_VS_FORMAT_UNDEFINED == UVC_GetVdoFormat()))
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
			if((KNL_GetTuningToolMode() == KNL_TUNINGMODE_OFF) ||
			   (USB_UVC_VS_FORMAT_UNDEFINED == UVC_GetVdoFormat()))
				H264_ResetIPCnt((H264_ENCODE_INDEX)tNodeInfo.ubCodecIdx);	//! ubKNL_ResetIFlg = 1;
		}
	}
}

//------------------------------------------------------------------------------
uint32_t ulKNL_GetAdoPktSZ(uint32_t ulAddr,uint32_t ulSize)
{
	uint32_t ulRtnValue;

	ulRtnValue = 0;
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_ADOSIZE+3)))))<<0);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_ADOSIZE+2)))))<<8);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_ADOSIZE+1)))))<<16);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-KNL_INFO_ADOSIZE))))<<24);

	return ulRtnValue;
}		

//------------------------------------------------------------------------------
void KNL_AdcBufProcess(KNL_PROCESS tProc)
{
	uint32_t ulAdcAddr = 0;
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

	ubNextNode = (tProc.ubNextNode == KNL_NODE_MSC_ADO)?KNL_NODE_MSC_ADO:ubKNL_GetNextNode(tProc.ubSrcNum, KNL_NODE_ADC_BUF);
	if((ubNextNode == KNL_NODE_COMM_TX_ADO) || (ubNextNode == KNL_NODE_MSC_ADO))
	{
		if(ubKNL_GetCommLinkStatus(ubLinkRole) == BB_LOST_LINK)		
			goto SEND_ADO_ERR;

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

				ulAdcAddr = ulBUF_GetAdcFreeBuf();
				if(BUF_FAIL == ulAdcAddr)
					goto SEND_ADO_ERR;
				switch(ubNextNode)
				{
					case KNL_NODE_COMM_TX_ADO:
						tDmaResult = tDMAC_MemCopy(tProc.ulDramAddr2, ulAdcAddr, tProc.ulSize,NULL);
						if(DMAC_OK != tDmaResult)
						{
							printd(DBG_ErrorLvl, "DMA NRDY @%s !\n", __func__);
							goto SEND_ADO_ERR;
						}
						break;
					case KNL_NODE_MSC_ADO:
					{
						ADO_AUD32_ENC_INFO tAdo32EncInfo;
						uint32_t *pEncData;

						pEncData 	= (uint32_t *)ulAdcAddr;
						pEncData[0] = ubKNL_UsbdAdoEncStFlag;
						pEncData[1] = osKernelSysTick();
						if(TRUE == ubKNL_UsbdAdoEncStFlag)
							ubKNL_UsbdAdoEncStFlag = FALSE;
						tAdo32EncInfo = ADO_Audio32_Encode(0, tProc.ulDramAddr1, (uint32_t)(&pEncData[2]), tProc.ulSize);
						if(!tAdo32EncInfo.ulOutputSize)
						{
							printd(DBG_ErrorLvl, "ADO32 ENC Fail @%s !\n", __func__);
							goto SEND_ADO_ERR;
						}
						tProc.ulSize = tAdo32EncInfo.ulOutputSize + 8;
						ubBUF_ReleaseAdoUsbdBuf(tProc.ulDramAddr1);
						break;
					}
					default:
						goto SEND_ADO_ERR;
				}
				ulExtSz = ulKNL_AddAuxInfo(KNL_ADO_PKT, tProc.ubSrcNum, ulAdcAddr, tProc.ulSize, 0, 0, 0);
			}
			tKNL_BbStatus = tBB_SendData(NULL,BB_DATA_AUDIO,(uint8_t *)ulAdcAddr,ulExtSz,(SET_TX_PATH)NULL);
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
			ubTemp = ubBUF_ReleaseAdcBuf(ulAdcAddr);
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
SEND_ADO_ERR:
	if(ubNextNode == KNL_NODE_MSC_ADO)
		ubBUF_ReleaseAdoUsbdBuf(tProc.ulDramAddr1);
	if((ulAdcAddr) && (BUF_FAIL != ulAdcAddr))
		ubBUF_ReleaseAdcBuf(ulAdcAddr);
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
			if(osMessagePutToFront(KNL_AdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
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
	uint32_t ulAdoPktSize = 0;
	ADO_Queue_INFO EN_INFO;	//!< static ADO_Queue_INFO EN_INFO;

	ulAdoPktSize = ulKNL_GetAdoPktSZ(tProc.ulDramAddr2, tProc.ulSize);
	ubNextNode   = ubKNL_GetNextNode(tProc.ubSrcNum,tProc.ubNextNode);
	if((ubKNL_ChkAdoFlowAct(tProc.ubSrcNum)) || (tUSBD_GetClassMode() == USBD_COMPOSITE_MODE))
	{
		if(ubNextNode == KNL_NODE_DAC)
		{
			if((tADO_GetAdo32Enable() == ADO_ON) && (tKNL_AdoInfo.Compress_method == COMPRESS_NONE))
			{
				EN_INFO.EncType = AUDIO32;
				EN_INFO.Audio32Addr = tProc.ulDramAddr2;
				EN_INFO.Audio32Size	= ulAdoPktSize;
				EN_INFO.PcmAddr = 0;
			}
			else if((tADO_GetAACEnable() == ADO_ON) && (tKNL_AdoInfo.Compress_method == COMPRESS_NONE))
			{
				EN_INFO.EncType = AAC;
				EN_INFO.AACAddr = tProc.ulDramAddr2;
				EN_INFO.AACSize	= ulAdoPktSize;
			}
			else if((tADO_GetAdo32Enable() == ADO_OFF) && (tADO_GetAACEnable() == ADO_OFF) && (tKNL_AdoInfo.Compress_method == COMPRESS_MSADPCM))
			{
				//MS-ADPCM
				EN_INFO.EncType = NONE;
				EN_INFO.PcmAddr = tProc.ulDramAddr2;
				EN_INFO.PcmSize	= ulAdoPktSize;
			}
			else if((tADO_GetAdo32Enable() == ADO_OFF) && (tADO_GetAACEnable() == ADO_OFF) && (tKNL_AdoInfo.Compress_method == COMPRESS_ALAW))
			{
				//ALAW
				EN_INFO.EncType = NONE;
				EN_INFO.PcmAddr = tProc.ulDramAddr2;
				EN_INFO.PcmSize	= ulAdoPktSize;
			}
			else if((tADO_GetAdo32Enable() == ADO_OFF) && (tADO_GetAACEnable() == ADO_OFF) && (tKNL_AdoInfo.Compress_method == COMPRESS_NONE))
			{
				//PCM
				EN_INFO.EncType = NONE;
				EN_INFO.PcmAddr = tProc.ulDramAddr2;
				EN_INFO.PcmSize	= ulAdoPktSize;
			}
//			printf("----->Src:%d\n",EN_INFO.ubSrcNum);
			EN_INFO.ADO_PlayType = NORMAL_PLAY;
			EN_INFO.ubSrcNum = ubKNL_SrcNumMap(tProc.ubSrcNum);
			EN_INFO.ubPlaySrcNum = EN_INFO.ubSrcNum;
			EN_INFO.ulDecAddr	 = 0;
		#ifdef OP_AP
			if(tUSBD_GetClassMode() == USBD_COMPOSITE_MODE)
			{
				EN_INFO.ubPlaySrcNum = (ubKNL_ChkAdoFlowAct(tProc.ubSrcNum))?ubKNL_SrcNumMap(tProc.ubSrcNum):0xFF;
				EN_INFO.ulDecAddr  	 = ulKNL_SetUsbdAdoPktInfo(tProc.ulDramAddr2, tProc.ulSize);
				EN_INFO.ulDecAddr 	+= ((EN_INFO.ulDecAddr)?UVC_OFS_SUB_FRAME_DATA:0);
			}
		#endif
			ADO_DecodeBufferWrtIn(&EN_INFO);
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
uint32_t ulKNL_GetTimeStamp1(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint32_t *)(ulAddr+ulSize-KNL_INFO_TIMESTP_LSB));
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetTimeStamp2(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint32_t *)(ulAddr+ulSize-KNL_INFO_TIMESTP_MSB));
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetPktSrcNum(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-KNL_INFO_SRCNUM));
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetPktOpMode(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-KNL_INFO_OPMODE));
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetPktFrmIdx(uint32_t ulAddr,uint32_t ulSize)
{
	uint32_t ulRtnValue;	
	
	ulRtnValue = 0;
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_FRMIDX+3)))))<<0);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_FRMIDX+2)))))<<8);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_FRMIDX+1)))))<<16);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-KNL_INFO_FRMIDX))))<<24);
	
	return ulRtnValue;
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetPktGop(uint32_t ulAddr,uint32_t ulSize)
{
	uint32_t ulRtnValue;	
	
	ulRtnValue = 0;
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_GOP+3)))))<<0);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_GOP+2)))))<<8);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_GOP+1)))))<<16);
	ulRtnValue += (((uint32_t)(*((uint8_t *)(ulAddr+ulSize-KNL_INFO_GOP))))<<24);
	
	return ulRtnValue;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetPktVdoGop(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-KNL_INFO_VGOP));
}
//------------------------------------------------------------------------------
uint16_t ubKNL_GetPktHRes(uint32_t ulAddr,uint32_t ulSize)
{
	return (*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_RES+3))) << 8) + (*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_RES+2))));
}
//------------------------------------------------------------------------------
uint16_t ubKNL_GetPktVRes(uint32_t ulAddr,uint32_t ulSize)
{
	return (*((uint8_t *)(ulAddr+ulSize-(KNL_INFO_RES+1))) << 8) + (*((uint8_t *)(ulAddr+ulSize-KNL_INFO_RES)));
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetFrameSeq(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-KNL_INFO_FRMSEQ));
}
//------------------------------------------------------------------------------
uint8_t ubKNL_GetPaddLen(uint32_t ulAddr,uint32_t ulSize)
{
	return *((uint8_t *)(ulAddr+ulSize-KNL_INFO_PADD));
}
//------------------------------------------------------------------------------
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

void KNL_ResendIframeFunc(KNL_ROLE tKNL_Role)
{
#ifdef OP_AP
	if(tKNL_Role <= KNL_STA4)
		ubKNL_AppResendIFrmFlg[tKNL_Role] = TRUE;
	printd(DBG_CriticalLvl, "	I->STA[%d]\n", tKNL_Role);
#endif
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
        osMessageGet(KNL_AdoCodecProcQueue, &tProc, osWaitForever);
		switch(tProc.ubNextNode)
		{
			case KNL_NODE_MSC_ADO:
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
					#endif
					}
					if(FALSE == ubKNL_SetLhFlag)
					{
						ubLinkTh		= 4;
						ubLinkLoopCycle	= 10;
						uwLinkPeriod    = 100;
						ubKNL_SetLhFlag = TRUE;
					}
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
void KNL_ModifyUsbdViewType(uint8_t ubViewType)
{
	ubKNL_UsbdVdoViewType = ubViewType;
}
//------------------------------------------------------------------------------
void KNL_ReleaseUsbdBuf(uint32_t ulBufAddr)
{
	if(0xFFFFFFFF == ulBufAddr)
	{
		BUF_ResetUsbdBuf();
	}
	else if(BUF_FAIL == ubBUF_ReleaseVdoUsbdBuf(ulBufAddr))
	{
		if(BUF_FAIL == ubBUF_ReleaseAdoUsbdBuf(ulBufAddr))
			printd(DBG_ErrorLvl, "Release Usbd Buf Err: 0x%X !\n", ulBufAddr);
	}
}
//------------------------------------------------------------------------------
uint8_t ubKNL_UpdateUvcImage(uint32_t ulVdoAddr, uint32_t ulVdoSize)
{
	uint32_t ulBsAddr = 0, ulTimeStp1 = 0, ulTimeStp2 = 0, ulFrmIdx = 0;
	uint32_t ulBsSize = 0, ulRemindBsSize = 0;
	uint32_t ulBsSrcAddr = 0, ulBsDestAddr = 0;
	uint32_t ulUvcAuxSize = 0, ulSubFrmSize = 0;
	uint8_t ubSrcNum, ubRoleNum, ubIdx;
	uint8_t ubFrmType, ubFrmSeq, ubNumOfSubData, ubSubIdx;
	DMAC_RESULT tDmaResult = DMAC_OK;

	ulUvcAuxSize   = KNL_USBDUVC_AUX_SIZE - UVC_OFS_CH_INFO;
	ulSubFrmSize   = USBDVDO_SUBFRAME_SIZE - KNL_USBDUVC_AUX_SIZE;
	ulBsSize 	   = ulVdoSize - KNL_AUX_INFO_LEN;
	ubNumOfSubData = ulBsSize / ulSubFrmSize;
	if(ulBsSize % ulSubFrmSize)
	{
		ulRemindBsSize = ulBsSize - (ubNumOfSubData * ulSubFrmSize);
		ubNumOfSubData++;
	}
	ubSrcNum   = ubKNL_GetPktSrcNum(ulVdoAddr, ulVdoSize);
	ubRoleNum  = ubKNL_SrcNumMap(ubSrcNum);
	ulFrmIdx   = ulKNL_GetPktFrmIdx(ulVdoAddr, ulVdoSize);
	ubFrmType  = (ulFrmIdx == 0)?KNL_VDO_I_FRAME:KNL_VDO_P_FRAME;
	ulTimeStp1 = ulKNL_GetTimeStamp1(ulVdoAddr, ulVdoSize);
	ulTimeStp2 = ulKNL_GetTimeStamp2(ulVdoAddr, ulVdoSize);
	ubFrmSeq   = ubKNL_GetFrameSeq(ulVdoAddr, ulVdoSize);
	for(ubSubIdx = 0; ubSubIdx < ubNumOfSubData; ubSubIdx++)
	{
		ulBsAddr = ulBUF_GetVdoUsbdFreeBuf();
		if(BUF_FAIL == ulBsAddr)
			return FALSE;
		memset((uint8_t *)ulBsAddr, 0, KNL_USBDUVC_AUX_SIZE);
		*((uint8_t *)(ulBsAddr + UVC_OFS_CH_INFO))	= (ubRoleNum + (ubKNL_UsbdVdoViewType * 6));
		*((uint8_t *)(ulBsAddr + UVC_OFS_AUX_INFO))	= ubFrmType;
		for(ubIdx = 0; ubIdx < 4; ubIdx++)
			*((uint8_t *)(ulBsAddr + UVC_OFS_TIME_STAMP + ubIdx)) = ((ulTimeStp1 >> (ubIdx * 8)) & 0xFF);
		for(ubIdx = 0; ubIdx < 4; ubIdx++)
			*((uint8_t *)(ulBsAddr + UVC_OFS_TIME_STAMP + ubIdx + 4)) = ((ulTimeStp2 >> (ubIdx * 8)) & 0xFF);
		*((uint8_t *)(ulBsAddr + UVC_OFS_NUM_SUB_FRAME)) 	 = ubNumOfSubData;
		*((uint8_t *)(ulBsAddr + UVC_OFS_SUB_FRAME_IDX)) 	 = ubSubIdx;
		ulBsSize = ((ubSubIdx + 1) == ubNumOfSubData)?ulRemindBsSize:ulSubFrmSize;
		*((uint8_t *)(ulBsAddr + UVC_OFS_SUB_FRAME_LEN)) 	 = ulBsSize & 0xFF;
		*((uint8_t *)(ulBsAddr + UVC_OFS_SUB_FRAME_LEN + 1)) = (ulBsSize >> 8) & 0xFF;
		*((uint8_t *)(ulBsAddr + UVC_OFS_TOTAL_FRAME_IDX))   = ubFrmSeq;
		*((uint8_t *)(ulBsAddr + UVC_OFS_RESERVED))   		 = 0;
		ulBsSrcAddr  = ulVdoAddr + (ubSubIdx * ulSubFrmSize);
		ulBsDestAddr = ulBsAddr + UVC_OFS_SUB_FRAME_DATA;
		tDmaResult   = tDMAC_MemCopy(ulBsSrcAddr, ulBsDestAddr, ulBsSize, NULL);
		if((DMAC_OK != tDmaResult) || (FALSE == uvc_update_image((uint32_t *)ulBsAddr, (ulBsSize + ulUvcAuxSize))))
		{
			KNL_ReleaseUsbdBuf(ulBsAddr);
			return FALSE;
		}
	}
	return TRUE;
}
//------------------------------------------------------------------------------
#define FRM_LOSS_ERR		1
#define FRM_CRC_ERR			2
#define FRM_TX_BUFF_FULL	3
#define FRM_SRCNUM_ERR		4
#define FRM_ERR5			5
#define FRM_ERR6			6
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

			if(TRUE == ubKNL_AppResendIFrmFlg[ubSrcNumMap])
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
				goto RX_VDORES_ERR;
			}

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
				else if((ulCurFrmIdx[ubSrcNum] == 0) && (tKNL_RecvFrmType == KNL_I_FRAME) && (ubKNL_RcvFirstIFrame[ubSrcNum]) && (ulPreFrmIdx[ubSrcNumMap] == (ulKNL_GetVdoGop() - 1)))
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
					ubKNL_AppResendIFrmFlg[ubSrcNumMap] = FALSE;
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
					printd(DBG_CriticalLvl, "TB[%d]:%d\n",ubSrcNum,ulSum[ubSrcNumMap]/1024);
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
						tKNLInfo.ubSrcNum = ubSrcNum;

						if(ubKNL_ChkDebugPkt(ulTemp,ulChkSz) == 1)
						{
							//MultiOut Process
							if(ubMultiOutFlg)
							{
								//Copy Data to Src2
								tDMAC_MemCopy(ulTemp,ulOutAddr2,ulChkSz,NULL);
							}

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
						printd(DBG_ErrorLvl, "DMA NRDY @%s !\n", __func__);
					}
				}
				else
				{
					printd(DBG_ErrorLvl, "Busy[%d]:%d\r\n", Don.tSTA, ubSrcNum);
					if(Don.tSTA <= KNL_STA4)
						ubKNL_VdoBsBusyCnt[Don.tSTA]++;
				}
				if(tUSBD_GetClassMode() == USBD_COMPOSITE_MODE)
				{
					ubKNL_UpdateUvcImage(Don.ulAddr, Don.ulSize);
				}
				//(6)Release BB Buffer
				BB_RxBufRelease(Don.Type, Don.tSTA);
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
//------------------------------------------------------------------------------
void KNL_UpdateUsbdAdoData(uint32_t ulAdoPcmAddr, uint32_t ulAdoPcmSize)
{
	uint32_t ulUsbdAdoAddr = 0;
	uint32_t ulUvcAuxSize = UVC_OFS_SUB_FRAME_DATA - UVC_OFS_CH_INFO;
	
	ulUsbdAdoAddr = ulAdoPcmAddr - UVC_OFS_SUB_FRAME_DATA;
	if(!ulAdoPcmSize)
	{
		uint8_t *pSrcNum = (uint8_t *)ulUsbdAdoAddr;

		ubBUF_ReleaseAdoUsbdBuf(ulUsbdAdoAddr);
		printd(DBG_ErrorLvl, "\n	[%d]Drop ado frame\n", pSrcNum[12]);
		return;
	}
	*((uint8_t *)(ulUsbdAdoAddr + UVC_OFS_SUB_FRAME_LEN)) 	  = ulAdoPcmSize & 0xFF;
	*((uint8_t *)(ulUsbdAdoAddr + UVC_OFS_SUB_FRAME_LEN + 1)) = (ulAdoPcmSize >> 8) & 0xFF;
	//! USBD Send
	if(FALSE == uvc_update_image((uint32_t *)ulUsbdAdoAddr, (ulAdoPcmSize + ulUvcAuxSize)))
		KNL_ReleaseUsbdBuf(ulUsbdAdoAddr);
}
//------------------------------------------------------------------------------
uint32_t ulKNL_SetUsbdAdoPktInfo(uint32_t ulAdoAddr, uint32_t ulAdoSize)
{
	uint32_t ulAdoPktAddr = 0, ulTimeStp1, ulTimeStp2;
	uint8_t ubSrcNum, ubRoleNum, ubFrmSeq, ubIdx;

	ulAdoPktAddr = ulBUF_GetAdoUsbdFreeBuf();
	if(BUF_FAIL == ulAdoPktAddr)
		return 0;

	ubSrcNum   = ubKNL_GetPktSrcNum(ulAdoAddr, ulAdoSize);
	ubRoleNum  = ubKNL_SrcNumMap(ubSrcNum);
	ulTimeStp1 = ulKNL_GetTimeStamp1(ulAdoAddr, ulAdoSize);
	ulTimeStp2 = ulKNL_GetTimeStamp2(ulAdoAddr, ulAdoSize);
	ubFrmSeq   = ubKNL_GetFrameSeq(ulAdoAddr, ulAdoSize);
	*((uint8_t *)(ulAdoPktAddr + UVC_OFS_CH_INFO))	= ubRoleNum;
	*((uint8_t *)(ulAdoPktAddr + UVC_OFS_AUX_INFO)) = KNL_ADO_FRAME;
	for(ubIdx = 0; ubIdx < 4; ubIdx++)
		*((uint8_t *)(ulAdoPktAddr + UVC_OFS_TIME_STAMP + ubIdx)) = ((ulTimeStp1 >> (ubIdx * 8)) & 0xFF);
	for(ubIdx = 0; ubIdx < 4; ubIdx++)
		*((uint8_t *)(ulAdoPktAddr + UVC_OFS_TIME_STAMP + ubIdx + 4)) = ((ulTimeStp2 >> (ubIdx * 8)) & 0xFF);
	*((uint8_t *)(ulAdoPktAddr + UVC_OFS_NUM_SUB_FRAME))   = 1;
	*((uint8_t *)(ulAdoPktAddr + UVC_OFS_SUB_FRAME_IDX))   = 0;
	*((uint8_t *)(ulAdoPktAddr + UVC_OFS_TOTAL_FRAME_IDX)) = ubFrmSeq;

	return ulAdoPktAddr;
}
//------------------------------------------------------------------------------
void KNL_RecvAdoDataFromApp(uint8_t *pAdoData)
{
	uint32_t ulPcmAddr, ulSrcAddr;
	uint16_t uwAdoSize;
	uint8_t ubAuxInfo, ubChInfo;
	DMAC_RESULT tDmaResult = DMAC_OK;
	KNL_PROCESS tProcess;

	if(!ubKNL_UsbdAdoEncAct)
		return;
	ubAuxInfo 		= *((uint8_t *)(pAdoData + 0));
	ubChInfo  		= *((uint8_t *)(pAdoData + 1));
	uwAdoSize		= *((uint8_t *)(pAdoData + 4)) + (*((uint8_t *)(pAdoData + 5)) << 8);
	if(KNL_ADO_FRAME != (ubAuxInfo & 0x07))
		return;
	ulPcmAddr = ulBUF_GetAdoUsbdFreeBuf();
	if(BUF_FAIL == ulPcmAddr)
		return;
	ulSrcAddr = (uint32_t)((pAdoData + KNL_USBDMSC_AUX_SIZE));
	tDmaResult = tDMAC_MemCopy(ulSrcAddr, ulPcmAddr, uwAdoSize, NULL);
	if(DMAC_OK != tDmaResult)
	{
		ubBUF_ReleaseAdoUsbdBuf(ulPcmAddr);
		printd(DBG_ErrorLvl, "DMA NRDY @%s !\n", __func__);
		return;
	}
	if(ptKNL_AdoRoleMap2SrcNum)
	{
		KNL_SRC tAdoSrcNum;

		tAdoSrcNum = ptKNL_AdoRoleMap2SrcNum(KNL_SUB_PATH, (KNL_ROLE)ubChInfo);
		if(KNL_SRC_NONE == tAdoSrcNum)
		{
			printd(DBG_ErrorLvl, "Err Src Num @%s !\n", __func__);
			ubBUF_ReleaseAdoUsbdBuf(ulPcmAddr);
			return;
		}
		if((ubKNL_GetRole() == KNL_SLAVE_AP) || (ubKNL_GetRole() == KNL_MASTER_AP))
		{
			uint8_t ubRxVdo_Path, ubTxAdo_Path;

			ubRxVdo_Path = ubBB_GetRxVdoDataPath();
			ubTxAdo_Path = tAdoSrcNum % 4;
			BB_SetDataPath((TXADO)ubTxAdo_Path, BB_RX_ADO_ALL_STA, (PAYLOAD_PATH)ubRxVdo_Path);
		}
		tProcess.ubSrcNum	 = tAdoSrcNum;
		tProcess.ubCurNode	 = KNL_NODE_MSC_ADO;
		tProcess.ubNextNode	 = KNL_NODE_MSC_ADO;
		tProcess.ulDramAddr1 = ulPcmAddr;
		tProcess.ulDramAddr2 = 0;
		tProcess.ulSize		 = uwAdoSize;
		if(osMessagePut(KNL_AdoCodecProcQueue, &tProcess, 0) >= osEventTimeout)
			printd(DBG_ErrorLvl, "KNL_ADO Q->Full !!!!\r\n");
	}
}
//------------------------------------------------------------------------------
void KNL_ActiveUsbdAdoEncFlag(uint8_t ubAct)
{
	ubKNL_UsbdAdoEncAct = ubAct;
	if(!ubKNL_UsbdAdoEncAct)
		ubKNL_UsbdAdoEncStFlag = TRUE;
}
#endif
//------------------------------------------------------------------------------
static void KNL_CommAdoRxMonitThread(void const *argument)
{		
	KNL_PROCESS tKNLInfo;
	RX_DON Don;
	uint32_t ulTemp;
	uint8_t ubSrcNum;
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

		if(Don.ubGetCrc != ubBB_GetCrcReport(Don.ulAddr,Don.ulCrcLen))
		{
			printd(DBG_ErrorLvl, " Ado Crc->Fail[%d][%d_%d]\r\n", ubKNL_GetPktSrcNum(Don.ulAddr,Don.ulSize), Don.tSTA, Don.Type);
			BB_RxBufRelease(Don.Type,Don.tSTA);
		}
		else if( ADO_GetIpReadyStatus() == ADO_IP_READY )
        {
            ubSrcNum = ubKNL_GetPktSrcNum(Don.ulAddr,Don.ulSize);
            if((ubKNL_ChkAdoFlowAct(ubSrcNum)) || (tUSBD_GetClassMode() == USBD_COMPOSITE_MODE))
            {
                //(2)Request Buffer to Temp
                if(ubKNL_SrcNumMap(ubSrcNum) == 0)
                {
                    ulTemp = ulBUF_GetDac0FreeBuf();
                }
                else if(ubKNL_SrcNumMap(ubSrcNum) == 1)
                {
                    ulTemp = ulBUF_GetDac1FreeBuf();
                }
                else if(ubKNL_SrcNumMap(ubSrcNum) == 2)
                {
                    ulTemp = ulBUF_GetDac2FreeBuf();
                }
                else if(ubKNL_SrcNumMap(ubSrcNum) == 3)
                {
                    ulTemp = ulBUF_GetDac3FreeBuf();
                }
				else
				{
					ulTemp = BUF_FAIL;
				}

                if(ulTemp == BUF_FAIL)
                {
                    printd(DBG_ErrorLvl, "[%d]BUF_ADO Err !!!\r\n", ubSrcNum);
					BB_RxBufRelease(Don.Type,Don.tSTA);
                }
                else
                {
					DMAC_RESULT tDmaResult = DMAC_OK;

                    tDmaResult = tDMAC_MemCopy((uint32_t)Don.ulAddr,(uint32_t)ulTemp,Don.ulSize,NULL);
					if(DMAC_OK == tDmaResult)
					{
						//(4-1)Check Real-Size Information
						ulRealSz = ulKNL_GetAdoPktSZ(ulTemp, Don.ulSize);
						printd(DBG_Debug3Lvl, "Real-Sz:0x%x\r\n", ulRealSz);

						//(5)Send Queue to Next Node
						tKNLInfo.ubSrcNum    = ubSrcNum;
						tKNLInfo.ulDramAddr2 = ulTemp;
						tKNLInfo.ulSize		 = Don.ulSize;
						tKNLInfo.ubCurNode   = KNL_NODE_COMM_RX_ADO;
						tKNLInfo.ubNextNode  = ubKNL_GetNextNode(ubSrcNum,KNL_NODE_COMM_RX_ADO);
//							KNL_DacBufProcess(tKNLInfo);
						if(osMessagePut(KNL_AdoCodecProcQueue, &tKNLInfo, 0) == osErrorResource)
						{
							ubBUF_ReleaseDac0Buf(tKNLInfo.ulDramAddr2);
							printd(DBG_ErrorLvl, "KNL_ADO Q->Full !!!\r\n");
						}
					}

					//(4-2)Release BB Buffer
					BB_RxBufRelease(Don.Type,Don.tSTA);

					if( DMAC_OK != tDmaResult )
					{
						if(ubKNL_SrcNumMap(ubSrcNum) == 0)
						{
							ubBUF_ReleaseDac0Buf(ulTemp);
						}
						else if(ubKNL_SrcNumMap(ubSrcNum) == 1)
						{
							ubBUF_ReleaseDac1Buf(ulTemp);
						}
						else if(ubKNL_SrcNumMap(ubSrcNum) == 2)
						{
							ubBUF_ReleaseDac2Buf(ulTemp);
						}
						else if(ubKNL_SrcNumMap(ubSrcNum) == 3)
						{
							ubBUF_ReleaseDac3Buf(ulTemp);
						}
						if(DMAC_OK != tDmaResult)
							printd(DBG_ErrorLvl, "DMA NRDY @%s\n", __func__);
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
void KNL_UpdateJpgHeader(void)
{
	uint8_t *pJpgHeader, i;
	uint8_t ubJpgHeader1[25] = {
		0xFF, 0xD8, 0xFF, 0xC0, 0x00, 0x11, 0x08, 0x02, 0xd0, 0x05, 0x00, 0x03, 0x01, 0x22, 0x00, 0x02, 
		0x11, 0x01, 0x03, 0x11, 0x01, 0xFF, 0xEF, 0x01, 0xB1
	};
	uint8_t ubJpgHeader2[568] = {
		0xFF, 0xDB, 0x00, 0x84, 0x00, 0x0B, 0x07, 0x07,
		0x0B, 0x07, 0x07, 0x0B, 0x0B, 0x0B, 0x0B, 0x0E, 0x0B, 0x0B, 0x0E, 0x12, 0x1D, 0x12, 0x12, 0x0E, 
		0x0E, 0x12, 0x24, 0x19, 0x19, 0x15, 0x1D, 0x2B, 0x24, 0x2B, 0x2B, 0x27, 0x24, 0x27, 0x27, 0x2F, 
		0x32, 0x40, 0x39, 0x2F, 0x32, 0x3D, 0x32, 0x27, 0x27, 0x39, 0x4F, 0x39, 0x3D, 0x44, 0x48, 0x4B, 
		0x4B, 0x4B, 0x2B, 0x36, 0x52, 0x56, 0x4F, 0x48, 0x56, 0x40, 0x48, 0x4B, 0x48, 0x01, 0x0B, 0x0E, 
		0x0E, 0x12, 0x0E, 0x12, 0x20, 0x12, 0x12, 0x20, 0x48, 0x2F, 0x27, 0x2F, 0x48, 0x48, 0x48, 0x48, 
		0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
		0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
		0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0xFF, 0xC4,
		0x01, 0xA2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x01,
		0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x00, 0x02, 0x01,
		0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D, 0x01, 0x02, 0x03,
		0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14,
		0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62,
		0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x34,
		0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54,
		0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74,
		0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93,
		0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA,
		0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8,
		0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5,
		0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0x11,
		0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
		0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
		0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0,
		0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26,
		0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
		0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
		0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
		0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3,
		0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
		0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
		0xF9, 0xFA, 0xFF, 0xDA, 0x00, 0x0C, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3F, 0x00
	};
	for(i = 0; i < (1 << ubKNL_GetOpMode()); i++)
	{
		pJpgHeader = (uint8_t *)ulBUF_GetBlkBufAddr(i, BUF_JPG_BS);
		memset(pJpgHeader, 0, 1024);
		memcpy(pJpgHeader, ubJpgHeader1, sizeof(ubJpgHeader1));
		memcpy(&pJpgHeader[456], ubJpgHeader2, sizeof(ubJpgHeader2));
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
		ubInfo_PreNode 	= ubKNL_JpegPreNode;
		ubInfo_PreSrc	= ubKNL_JpegSrc;
		ubInfo_Action	= tJpegInfo.tJPEG_CodecMode;
		ulInfo_YuvAddr	= tJpegInfo.ulJPEG_YUVAddr;
		ulInfo_BsAddr	= tJpegInfo.ulJPEG_BsAddr;
		ulInfo_Size		= tJpegInfo.ulJPEG_BsSize;

		//(2)Release JPEG Codec
		osSemaphoreRelease(JPEG_CodecSem);

		//PhotoGraph
		#if KNL_PHOTOGRAPH_FUNC_ENABLE
		switch(tKNL_GetRecordFunc())
		{
			case KNL_PHOTO_CAPTURE:
				KNL_PhotoCaptureFinFunc(ubInfo_PreNode, ubInfo_PreSrc, ulInfo_Size);
				ubInfo_PreNode = KNL_NODE_NONE;
				continue;
			case KNL_PHOTO_PLAY:
				KNL_PhotoPlayFinFunc();
				break;
			default:
				break;
		}
		#endif

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
		BUF_Reset((BUF_VDO_MAIN_BS0 + tKNL_Role));
		KNL_ResetDecoder(tSrcNum);
	}
	for(tKNL_Role = KNL_STA1; tKNL_Role <= KNL_STA4; tKNL_Role++)
		KNL_VdoStart((KNL_SRC_1_MAIN + tKNL_Role));
}

//------------------------------------------------------------------------------
void KNL_RestartDataPath(uint8_t ubRole)
{
	KNL_SRC tSrcNum = KNL_SRC_NONE;

	if(ptKNL_VdoRoleMap2SrcNum)
		tSrcNum = ptKNL_VdoRoleMap2SrcNum(KNL_MAIN_PATH, (KNL_ROLE)ubRole);
	if(KNL_SRC_NONE != tSrcNum)
	{
		KNL_VdoStop(tSrcNum);
		ubKNL_WaitNodeFinish(tSrcNum);
		if(TRUE == ubKNL_WakeUpFlag[ubRole])
			KNL_WakeupDevice((KNL_ROLE)ubRole, FALSE);
		osDelay(200);
		KNL_VdoStart(tSrcNum);
	}
}

//------------------------------------------------------------------------------
static void KNL_SysMonitThread(void const *argument)
{
#if OP_AP
	KNL_ROLE tKNL_Role;
	uint8_t ubSysErrFlg1 	   = FALSE;
	uint8_t ubSysIChkCnt[4]    = {0, 0, 0, 0};
	uint8_t ubSysVResChkCnt[4] = {0, 0, 0, 0};
	uint8_t ubSysIfChkCnt[4]   = {0, 0, 0, 0};
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
			if(TRUE == ubKNL_AppResendIFrmFlg[tKNL_Role])
			{
				if(++ubSysIfChkCnt[tKNL_Role] >= 2)
				{
					ubKNL_AppResendIFrmFlg[tKNL_Role] = FALSE;
					ubSysIfChkCnt[tKNL_Role] = 0;
				}
			}
			else
				ubSysIfChkCnt[tKNL_Role] = 0;
		}
		if((TRUE == ubKNL_ImgBusyFlg))
		{
			printd(DBG_Debug1Lvl, "Decode Busy !\n");
			KNL_RecoveryImgCodec();
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
			tSysMaxRole = 1 << ubKNL_GetOpMode();
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
		ubNextNode = ubKNL_GetNextNode(ubSrcNum, KNL_NODE_H264_DEC);
		if(ubKNL_ChkVdoFlowAct(ubSrcNum))
		{
			tNodeInfo = tKNL_GetNodeInfo(ubSrcNum, KNL_NODE_H264_DEC);
			if(KNL_PHOTO_CAPTURE == tKNL_GetRecordFunc())
			{			
				ubKNL_JpegPreNode = KNL_NODE_H264_DEC;
				ubKNL_JpegSrc 	  = ubSrcNum;
				ubKNL_JPEGEncode(tNodeInfo.uwVdoH, tNodeInfo.uwVdoV, ReceiveResult.H264Result->YuvAddr,
								 (ulBUF_GetBlkBufAddr((ubKNL_SrcNumMap(ubSrcNum)), BUF_JPG_BS) + KNL_JPG_HEADER_SIZE));
				return;
			}
			else if((KNL_PHOTO_PLAY != tKNL_GetRecordFunc()) && (JPG_LCDCH_DISABLE == tKNL_GetJpegLcdChCtrl()) &&
				    (ubNextNode == KNL_NODE_LCD))
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
	KNL_PROCESS tProc;
	static uint8_t ubKNL_ChkNodeCnt = 0;
	uint8_t ubSrcNumMap;
	uint8_t ubDecNode = 0;

	ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
	//AvgPly Related
	if(tKNL_GetPlyMode() == KNL_AVG_PLY)
	{
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
	ubDecNode = ubKNL_ExistNode(ubSrcNum, KNL_NODE_H264_DEC);
	if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_ENC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_H264_DEC)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_IMG_MERGE_BUF)||ubKNL_ExistNode(ubSrcNum,KNL_NODE_IMG_MERGE_H))
	{
		if(ubKNL_ChkImgRdy() == 0)
		{
			if((ubKNL_ChkNodeCnt++ >= 30) && (ubDecNode))
				KNL_RecoveryImgCodec();
			else
				return 0;
		}
		if(ubDecNode)
			osDelay(50);
	}
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
		BUF_Reset((BUF_VDO_MAIN_BS0 + ubSrcNumMap));
	}

	return 1;
}
//------------------------------------------------------------------------------
void KNL_SetTRXPathActivity(void)
{
	uint8_t ubTxAdo_Path;

	ubKNL_BbPathAct = TRUE;
	ubTxAdo_Path	= ubBB_GetTxAdoDataPath();
	BB_SetDataPath((TXADO)ubTxAdo_Path, BB_RX_ADO_ALL_STA, BB_OPEN_ALL_PAYLOAD);
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

	ubSrcNumMap = ubKNL_SrcNumMap(ubSrcNum);
	if(ubSrcNumMap <= KNL_STA4)
	{
		ubKNL_VdoFlowActiveFlg[ubSrcNum] = 1;
		ubKNL_RcvFirstIFrame[ubSrcNum]	 = 0;

		for(i = 0; i < 256; i++)
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
		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_VDO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_VDO) ||
		   ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_ADO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
		{
			ubRxVdo_OriPath = ubBB_GetRxVdoDataPath();
			ubTxAdo_Path	= ubBB_GetTxAdoDataPath();
			if(TRUE == ubKNL_BbPathAct)
			{
				ubRxVdo_OriPath = BB_PAYLOAD_NONE;
				ubKNL_BbPathAct = FALSE;
			}
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
		ubKNL_AppResendIFrmFlg[ubSrcNumMap]	= FALSE;
		for(i = 0; i < 256; i++)
		{
			tKNL_NodeState[ubSrcNum][i] = KNL_NODE_STOP;
		}

//		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_SEN))
//		{
//			KNL_SenStop(ubSrcNum);
//		}

#if OP_AP
		if(ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_VDO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_VDO) ||
		   ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_TX_ADO) || ubKNL_ExistNode(ubSrcNum,KNL_NODE_COMM_RX_ADO))
		{
			KNL_SRC tAdoSrcNum;

			if(TRUE == ubKNL_BbPathAct)
				return;
			if(ptKNL_AdoRoleMap2SrcNum)
				tAdoSrcNum = ptKNL_AdoRoleMap2SrcNum(KNL_MAIN_PATH, (KNL_ROLE)ubSrcNumMap);
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
	ubKNL_AppResendIFrmFlg[ubRole] 	 = FALSE;
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
	SEN_SetFirstOutFlg(0);
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
void KNL_SDUpgradeFwFunc(void)
{
	FWU_UpgResult_t tFwuUpgRet;

	KNL_Stop();
	tFwuUpgRet = FWU_SdUpgradeStart(ulBUF_GetFreeAddr());
	printd(DBG_CriticalLvl, "\r\nFW Upgrade %s\r\n", (FWU_UPG_SUCCESS == tFwuUpgRet)?"Success":"Fail !");
	if(FWU_UPG_SUCCESS != tFwuUpgRet)
	{
		KNL_ReStart();
		return;
	}
	SYS_Reboot();
}
//------------------------------------------------------------------------------
uint8_t ubKNL_JPEGEncode(uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr)
{
	JPEG_FIFO_Addr_t 	tJpgFiFo;
	JPEG_ENC_INFO_t 	tJpgEncInfo;
	JPEG_CODEC_FN_ES_t	tJpgCodecFnEs;

	tJpgFiFo.ulJpeg_Buf_Start	= ulJpgAddr;
	tJpgFiFo.ulJpeg_Buf_End		= ulJpgAddr + KNL_JPG_BS_SIZE;
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
	tJpgFiFo.ulJpeg_Buf_End			= ulJpgAddr + KNL_JPG_BS_SIZE;
	JPEG_Ring_FIFO_Setup(tJpgFiFo);	

	JPEG_Set_Start_Address(ulVdoAddr,ulJpgAddr);

	tJpgDecInfo.uwH_ORI_SIZE		= uwH;
	tJpgDecInfo.uwH_SIZE			= uwH;
	tJpgDecInfo.uwV_SIZE			= uwV;
	tJpgDecInfo.uwQP				= ubKNL_GetJpegQp();
	tJpgDecInfo.ubJPG_Fmt 			= JPEG_YUV420;
	tJpgDecInfo.ubJPG_ScaleMode		= 0;
	tJpgDecInfo.ubJPG_Mirror		= (pKNL_NodeInfo->ubHMirror)?JPEG_H_MIRROR:(pKNL_NodeInfo->ubVMirror)?JPEG_V_MIRROR:JPEG_MIRROR_DISABLE;
	tJpgDecInfo.ubJPG_Rotate		= (pKNL_NodeInfo->ubRotate)?JPEG_ROT_90Deg:JPEG_ROT_DISABLE;

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
void KNL_SetVdoRoleInfoCbFunc(pvRoleMap2Src VdoRoleMap_cb)
{
	ptKNL_VdoRoleMap2SrcNum = VdoRoleMap_cb;
}
//------------------------------------------------------------------------------
void KNL_SetAdoRoleInfoCbFunc(pvRoleMap2Src AdoRoleMap_cb)
{
	ptKNL_AdoRoleMap2SrcNum = AdoRoleMap_cb;
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
#ifdef OP_STA
	IQ_SetupTuningToolMode(KNL_TUNINGMODE_ON);
	tKNL_TuningMode = KNL_TUNINGMODE_ON;
#endif
}
//------------------------------------------------------------------------------
void KNL_TurnOffTuningTool(void)
{
#ifdef OP_STA
	SEN_SetUvcPathFlag(0);
	IQ_SetupTuningToolMode(KNL_TUNINGMODE_OFF);
	tKNL_TuningMode = KNL_TUNINGMODE_OFF;
#endif
}
//------------------------------------------------------------------------------
KNL_TuningMode_t KNL_GetTuningToolMode(void)
{
#if (defined(OP_AP) || !USBD_ENABLE)
	tKNL_TuningMode = KNL_TUNINGMODE_OFF;
#endif
	return tKNL_TuningMode;
}
//------------------------------------------------------------------------------
KNL_Status_t tKNL_EnJpegLcdCh(KNL_JpgLcdChCtrl_t tJpgLcdChCtrl)
{
	uint8_t ubCodecRetryCnt = 18;

	for(;;)
	{
		if(ubKNL_ChkImgRdy())
			break;
		if(!--ubCodecRetryCnt)
		{
			printd(DBG_ErrorLvl, "H->D NRDY@%s\r\n", __func__);
			return KNL_ERR;
		}
		osDelay(10);
	}
	osMutexWait(osKNL_JpgLcdChMutex, osWaitForever);
	tKNL_JpgLcdChCtrl = tJpgLcdChCtrl;
	if((KNL_OPMODE_VBM_1T == ubKNL_GetOpMode()) ||
	   ((KNL_OPMODE_VBM_4T == ubKNL_GetOpMode()) && ((KNL_DISP_DUAL_C != tKNL_GetDispType()) && (KNL_DISP_DUAL_U != tKNL_GetDispType()))))
	{
		if(BUF_FREE == ulBUF_GetBlkBufAddr(0, BUF_RESV_YUV))
			BUF_BufInit(BUF_RESV_YUV, 1, ((ISP_WIDTH * ISP_HEIGHT * 3) >> 1), 0);
	}
	osMutexRelease(osKNL_JpgLcdChMutex);

	return KNL_OK;
}
//------------------------------------------------------------------------------
KNL_JpgLcdChCtrl_t tKNL_GetJpegLcdChCtrl(void)
{
	KNL_JpgLcdChCtrl_t tJpgLcdChCtrl;

	osMutexWait(osKNL_JpgLcdChMutex, osWaitForever);
	tJpgLcdChCtrl = tKNL_JpgLcdChCtrl;
	osMutexRelease(osKNL_JpgLcdChMutex);

	return tJpgLcdChCtrl;
}
//------------------------------------------------------------------------------
KNL_Status_t tKNL_ChkSdCardSts(void)
{
	KNL_Status_t tSdCardSts = KNL_ErrorNoCard;
#if KNL_SD_FUNC_ENABLE
	uint8_t ubFsTimeout = 30;

	if(!ubSD_ChkCardIn(tSD_GetDevIF()))
		return KNL_ErrorNoCard;

	while(FS_SD_NOT_RDY == FS_ChkSdRdy())
	{
		osDelay(100);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "SD Card not ready !\n");
			return KNL_ErrorTimeout;
		}
	}
	tSdCardSts = KNL_OK;
#endif
	return tSdCardSts;
}
//------------------------------------------------------------------------------
uint32_t ulKNL_GetResvDecAddr(void)
{
	uint32_t ulDecAddr = 0;

#if KNL_LCD_FUNC_ENABLE
	KNL_DISP_TYPE tDispType;

	tDispType = tKNL_GetDispType();
	switch(ubKNL_GetOpMode())
	{
		case KNL_OPMODE_VBM_4T:
		{
			LCD_CH_TYP tLcdCh;

			tLcdCh    = ((KNL_DISP_DUAL_C == tKNL_GetDispType()) || (KNL_DISP_DUAL_U == tKNL_GetDispType()))?LCD_CH1:LCD_CH0;
			ulDecAddr = (LCD_CH0 == tLcdCh)?ulBUF_GetBlkBufAddr(0, BUF_RESV_YUV):pLCD_GetLcdChBufInfor(tLcdCh)->ulBufAddr;
			break;
		}
		case KNL_OPMODE_VBM_2T:
			if(KNL_DISP_SINGLE != tDispType)
			{
				ulDecAddr = pLCD_GetLcdChBufInfor(LCD_CH1)->ulBufAddr;
				break;
			}
		default:
			ulDecAddr = ulBUF_GetBlkBufAddr(0, BUF_RESV_YUV);
			break;
	}
#endif
	return ulDecAddr;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_SearchCapSrcNum(uint8_t ubSrcNum)
{
#if KNL_PHOTOGRAPH_FUNC_ENABLE
	uint8_t i;

	for(i = 0; i < (1 << ubKNL_GetOpMode()); i++)
	{
		if(tKNL_RecordAct.ubPhotoCapSrc[i] == ubSrcNum)
		{
			KNL_SetRecordFunc(KNL_PHOTO_CAPTURE);
			return TRUE;
		}
	}
#endif
	return FALSE;
}
//------------------------------------------------------------------------------
uint8_t ubKNL_UpdateCapSrcNumFin(uint8_t ubSrcNum)
{
	uint8_t ubRet = TRUE;
#if KNL_PHOTOGRAPH_FUNC_ENABLE
	uint8_t i;

	for(i = 0; i < (1 << ubKNL_GetOpMode()); i++)
	{
		if(tKNL_RecordAct.ubPhotoCapSrc[i] == ubSrcNum)
			tKNL_RecordAct.ubPhotoCapSrc[i] = KNL_SRC_NONE;
		else if(tKNL_RecordAct.ubPhotoCapSrc[i] != KNL_SRC_NONE)
			ubRet = FALSE;
	}
#endif
	return ubRet;
}
//------------------------------------------------------------------------------
#if KNL_PHOTOGRAPH_FUNC_ENABLE
void KNL_WriteCaptureFile(uint8_t ubSrcNum, uint32_t ulCapSize)
{
	FS_KNL_CRE_PROCESS_t tKNL_FsProc;
	FS_Q_CREATE_STATUS tKNL_CreateFileHd;
	FS_KNL_HIDDEN_PROCESS_t tKNL_JpgHidInfo;
	FS_CRE_TIME_INFO_t tKNL_JpgCreTimeInfo;
	uint8_t ubFsTimeout = 50;

	tKNL_FsProc.SrcNum      	  = (FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0);
	tKNL_FsProc.ubRecGroupFileNum = 1;
	tKNL_FsProc.ubFileNameLen 	  = 8;
	FS_GetLatestFileName(&cKNL_LatestFileName[0]);
	FS_FileNameHandle(&tKNL_FsProc.chFileName[0], cKNL_LatestFileName, tKNL_FsProc.ubFileNameLen);
	memcpy(cKNL_LatestFileName, &tKNL_FsProc.chFileName[0], tKNL_FsProc.ubFileNameLen);
	memcpy(&tKNL_FsProc.chFileExtName[0], "JPG", 3);
	tKNL_FsProc.uwGroupIdx = tKNL_RecordAct.uwRecordGroupIdx;
	printd(DBG_CriticalLvl, "	Photo[%d][Gp:%d]:[%s].JPG\n", ubSrcNum, tKNL_FsProc.uwGroupIdx, cKNL_LatestFileName);
	tKNL_FsProc.FileAttr = FILE_ATTR_READ_ONLY;
	tKNL_FsProc.FilePath = FILE_PATH_DEFAULT;
	tKNL_CreateFileHd = FS_CreateFile(tKNL_FsProc);
	if(tKNL_CreateFileHd != Q_CREATE_OK)
		printd(DBG_ErrorLvl, "\n\r=== ERROR_Create File:%d ERROR ===\n\r", tKNL_CreateFileHd);
	while(FS_ChkCreateStatus((FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0)) != FS_REC_CREATE_OK)
	{
		osDelay(20);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "Create File Err !!\n");
			return;
		}
	}
	FS_WriteFile((FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0), ulBUF_GetBlkBufAddr(ubKNL_SrcNumMap(ubSrcNum), BUF_JPG_BS), (ulCapSize + KNL_JPG_HEADER_SIZE));
	FS_CloseFile((FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0));
	ubFsTimeout = 50;
	while(FS_ChkCloseStatus((FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0)) != FS_REC_CLOSED_OK)
	{
		osDelay(20);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "Close File Err !!\n");
			return;
		}
	}
	tKNL_JpgHidInfo.SrcNum = (FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0);
	tKNL_JpgHidInfo.ubFileNameLen = 8;
	memcpy(&tKNL_JpgHidInfo.chFileName, &tKNL_FsProc.chFileName[0] , tKNL_JpgHidInfo.ubFileNameLen);
	memcpy(&tKNL_JpgHidInfo.chFileExtName[0], "JPG", 3);
	tKNL_JpgHidInfo.ubEvent = 0;
	tKNL_JpgHidInfo.uwGroupIdx = tKNL_RecordAct.uwRecordGroupIdx;
	tKNL_JpgCreTimeInfo = FS_GetCreateTimeInfo((FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0));
	tKNL_JpgHidInfo.uwDateInfo = tKNL_JpgCreTimeInfo.uwDateInfo;
	tKNL_JpgHidInfo.uwTimeInfo = tKNL_JpgCreTimeInfo.uwTimeInfo;
	tKNL_JpgHidInfo.ubSecondOfs = tKNL_JpgCreTimeInfo.ubSecondOfs;
	tKNL_JpgHidInfo.ullFileSize = (ulCapSize + KNL_JPG_HEADER_SIZE);
	FS_UpdateHiddenInfo(tKNL_JpgHidInfo);
	ubFsTimeout = 50;
	while(FS_ChkUpdateHiddenInfoStatus((FS_SRC_NUM)(ubSrcNum+FS_JPG_SRC_0)) != FS_UPDATE_HIDDEN_INFO_OK)
	{
		osDelay(20);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "Update File Err !!\n");
			return;
		}
	}
}
//------------------------------------------------------------------------------
KNL_Status_t tKNL_ReadCaptureFile(KNL_RecordAct_t *pPhotoInfo)
{
	uint32_t ulJpgAddr  = 0, ulJpgSize = 0;
	uint8_t ubFsTimeout = 20;

    strncpy(&pPhotoInfo->tPhotoPlayInfo.chFileExtName[0], "JPG", 3);
    pPhotoInfo->tPhotoPlayInfo.SrcNum = pPhotoInfo->tPhotoPlayInfo.SrcNum;
    pPhotoInfo->tPhotoPlayInfo.FilePath = FILE_PATH_DEFAULT;
    if(FS_OpenFile(pPhotoInfo->tPhotoPlayInfo) == Q_OPEN_OK)
    {
		while(FS_ChkOpenStatus(pPhotoInfo->tPhotoPlayInfo.SrcNum) != FS_PLY_OPEN_OK)
		{
			osDelay(100);
			if(!--ubFsTimeout)
			{
				printd(DBG_ErrorLvl, "Open JPG file Fail !!\n");
				return KNL_ErrorTimeout;
			}
		}
    }
	else
		return KNL_ErrorTimeout;
	ubFsTimeout = 30;
	ulJpgAddr = ulBUF_GetBlkBufAddr(0, BUF_JPG_BS);
	ulJpgSize = ullFS_GetOpenFileSize(pPhotoInfo->tPhotoPlayInfo.SrcNum);
    if(FS_ReadFile(ulJpgAddr, pPhotoInfo->tPhotoPlayInfo.SrcNum, 0, ulJpgSize) == Q_READ_OK)
    {
		while(FS_ChkReadStatus(pPhotoInfo->tPhotoPlayInfo.SrcNum) != FS_PLY_READ_OK)
		{
			osDelay(100);
			if(!--ubFsTimeout)
			{
				printd(DBG_ErrorLvl, "Read JPG file Fail !!\n");
				return KNL_ErrorTimeout;
			}
		}
    }
	else
		return KNL_ErrorTimeout;
	osDelay(20);
	return KNL_OK;
}
//------------------------------------------------------------------------------
void KNL_PhotoCaptureFinFunc(uint8_t ubCapNode, uint8_t ubCapSrc, uint32_t ulCapSize)
{
	KNL_NODE_INFO tNodeInfo;
	uint8_t *pJpgHeader, ubCapFlag;

	tNodeInfo 		= tKNL_GetNodeInfo(ubCapSrc, ubCapNode);
	pJpgHeader 		= (uint8_t *)ulBUF_GetBlkBufAddr(ubKNL_SrcNumMap(ubCapSrc), BUF_JPG_BS);
	pJpgHeader[7]   = (uint8_t)((tNodeInfo.uwVdoV >> 8) & 0xFF);
	pJpgHeader[8]   = (tNodeInfo.uwVdoV & 0xFF);
	pJpgHeader[9]   = (uint8_t)((tNodeInfo.uwVdoH >> 8) & 0xFF);
	pJpgHeader[10]  = (tNodeInfo.uwVdoH & 0xFF);
	KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
	ubCapFlag = ubKNL_UpdateCapSrcNumFin(ubCapSrc);
	if(KNL_NODE_H264_DEC == ubCapNode)
	{
		osSemaphoreRelease(tKNL_ImgSem);
		ubKNL_ImgRdy = 1;
	}
	KNL_WriteCaptureFile(ubCapSrc, ulCapSize);
	if(TRUE == ubCapFlag)
		osSignalSet(osKNL_RecordThreadId, osKNL_CapFinSignal);
}
//------------------------------------------------------------------------------
void KNL_PhotoCaptureFunc(void)
{
	osEvent osKNL_PhotoGraphFinSig;
	KNL_Status_t tKNL_PhotoSts;
	uint8_t ubCapSrcNum, i;

	for(i = 0; i < (1 << ubKNL_GetOpMode()); i++)
		tKNL_RecordAct.ubPhotoCapSrc[i] = KNL_SRC_NONE;
	tKNL_RecordAct.uwRecordGroupIdx = uwFS_GetLatestGroupIdx(FS_JPG_SRC_0) + 1;
	ubCapSrcNum = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[0]:ubKNL_GetDispSrc(KNL_DISP_LOCATION1);
	tKNL_RecordAct.ubPhotoCapSrc[0] = ((KNL_SRC_NONE != ubCapSrcNum) && (BB_LINK == ubKNL_GetCommLinkStatus(ubKNL_SrcNumMap(ubCapSrcNum))))?ubCapSrcNum:KNL_SRC_NONE;
	ubCapSrcNum = (TRUE == KNL_SwDispInfo.ubSetupFlag)?KNL_SwDispInfo.tSrcNum[1]:ubKNL_GetDispSrc(KNL_DISP_LOCATION2);
	tKNL_RecordAct.ubPhotoCapSrc[1] = ((KNL_SRC_NONE != ubCapSrcNum) && (BB_LINK == ubKNL_GetCommLinkStatus(ubKNL_SrcNumMap(ubCapSrcNum))))?ubCapSrcNum:KNL_SRC_NONE;
	switch(tKNL_GetDispType())
	{
		case KNL_DISP_QUAD:
			ubCapSrcNum = ubKNL_GetDispSrc(KNL_DISP_LOCATION3);
			tKNL_RecordAct.ubPhotoCapSrc[2] = ((KNL_SRC_NONE != ubCapSrcNum) && (BB_LINK == ubKNL_GetCommLinkStatus(ubKNL_SrcNumMap(ubCapSrcNum))))?ubCapSrcNum:KNL_SRC_NONE;
			ubCapSrcNum = ubKNL_GetDispSrc(KNL_DISP_LOCATION4);
			tKNL_RecordAct.ubPhotoCapSrc[3] = ((KNL_SRC_NONE != ubCapSrcNum) && (BB_LINK == ubKNL_GetCommLinkStatus(ubKNL_SrcNumMap(ubCapSrcNum))))?ubCapSrcNum:KNL_SRC_NONE;
			break;
		case KNL_DISP_SINGLE:
			tKNL_RecordAct.ubPhotoCapSrc[1] = KNL_SRC_NONE;
			break;
		default:
			break;
	}
	osKNL_PhotoGraphFinSig = osSignalWait(osKNL_CapFinSignal, 2000);
	tKNL_PhotoSts = ((osKNL_PhotoGraphFinSig.status == osEventSignal) &&
					 (osKNL_PhotoGraphFinSig.value.signals == osKNL_CapFinSignal))?KNL_OK:KNL_ErrorTimeout;
	if(KNL_OK != tKNL_PhotoSts)
	{
		if(ubKNL_ExistNode(tKNL_RecordAct.ubPhotoCapSrc[0], KNL_NODE_H264_DEC))
		{
			osSemaphoreRelease(tKNL_ImgSem);
			ubKNL_ImgRdy = 1;
		}
		JPEG_Codec_Disable();
		osSemaphoreRelease(JPEG_CodecSem);
		KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
	}
	else
		FS_ResetSortingResult();
	if(tKNL_RecordAct.pRecordStsNtyCb)
		tKNL_RecordAct.pRecordStsNtyCb(tKNL_PhotoSts);
}
//------------------------------------------------------------------------------
uint32_t ulKNL_PhotoLcdDisplaySetup(uint16_t uwPhotoHSize, uint16_t uwPhotoVSize)
{
	LCD_INFOR_TYP tLcdParam;
	LCD_CALBUF_TYP	tLcdBufCalInfo;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		tLCD_JpegDecodeDisable();
	KNL_ResetLcdChannel();
	tLcdBufCalInfo.ubChMax = 1;
	tLcdBufCalInfo.tInput[0].bJpegEn = FALSE;
	tLcdBufCalInfo.tInput[0].uwHSize = uwLCD_GetLcdHoSize();
	tLcdBufCalInfo.tInput[0].uwVSize = uwLCD_GetLcdVoSize();
	tLcdBufCalInfo.tAlign = LCD_BUF_1024BYTES_ALIG;
	ulLCD_CalLcdBufSize(&tLcdBufCalInfo);
	LCD_SetLcdBufAddr(ulBUF_GetBlkBufAddr(0, BUF_LCD_IP));
	tLcdParam.tDispType = LCD_DISP_1T;
	tLcdParam.ubChNum = 1;
	tLcdParam.tChRes[0].uwCropHstart = 0;
	tLcdParam.tChRes[0].uwCropVstart = 0;
	tLcdParam.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
	tLcdParam.uwLcdOutputVsize = uwLCD_GetLcdVoSize();
	tLcdParam.tChRes[0].uwChInputHsize = (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)?uwPhotoHSize:uwPhotoVSize;
	tLcdParam.tChRes[0].uwChInputVsize = (tKNL_GetDispRotate() == KNL_DISP_ROTATE_0)?uwPhotoVSize:uwPhotoHSize;
	tLcdParam.tChRes[0].uwCropHsize = tLcdParam.tChRes[0].uwChInputHsize;
	tLcdParam.tChRes[0].uwCropVsize = tLcdParam.tChRes[0].uwChInputVsize;
	tLCD_CropScale(&tLcdParam);
	pKNL_LcdPlayBuf = pLCD_GetLcdChBufInfor(LCD_CH0);

	return pKNL_LcdPlayBuf->ulBufAddr;
}
//------------------------------------------------------------------------------
void KNL_PhotoLcdDisplayOn(void)
{
	LCD_SetChBufReady(pKNL_LcdPlayBuf);
	LCD_ChEnable(LCD_CH0);
}
//------------------------------------------------------------------------------
void KNL_PhotoLcdDisplayOFF(void)
{
	ulKNL_CalLcdBufSz();
	ubKNL_LcdDispParamActiveFlg = 1;
	KNL_ResetLcdChannel();
}
//------------------------------------------------------------------------------
void KNL_PhotoPlayFinFunc(void)
{
	osSignalSet(osKNL_RecordThreadId, osKNL_PhotoPlyFinSignal);
}
//------------------------------------------------------------------------------
void KNL_PhotoPlayFunc(KNL_RecordAct_t *pPhotoInfo)
{
	osEvent osKNL_PhotoGraphFinSig;
	KNL_Status_t tKNL_PhotoSts;
	KNL_NODE_INFO tPhotoPlayNodeInfo;
	uint32_t ulVdoYuvAddr = 0, ulJpgBsAddr = 0;
	uint16_t uwJpgHSize = 0, uwJpgVSize = 0;
	uint8_t *pJpgAddr;

	KNL_SetRecordFunc(KNL_PHOTO_PLAY);
	while(!ubKNL_ImgRdy)
		osDelay(10);
	if(KNL_OK != tKNL_ReadCaptureFile(pPhotoInfo))
		goto PHOTOPLAY_ERR;
	osDelay(100);
	pJpgAddr   = (uint8_t *)ulBUF_GetBlkBufAddr(0, BUF_JPG_BS);
	uwJpgVSize = (pJpgAddr[7] << 8) + pJpgAddr[8];
	uwJpgHSize = (pJpgAddr[9] << 8) + pJpgAddr[10];
	memset(&tPhotoPlayNodeInfo, 0, sizeof(KNL_NODE_INFO));
	tPhotoPlayNodeInfo.ubVMirror = (tKNL_GetDispRotate() == KNL_DISP_ROTATE_90)?JPEG_V_MIRROR:JPEG_MIRROR_DISABLE;
	ulVdoYuvAddr = ulKNL_PhotoLcdDisplaySetup(uwJpgHSize, uwJpgVSize);
	ulJpgBsAddr  = ulBUF_GetBlkBufAddr(0, BUF_JPG_BS) + KNL_JPG_HEADER_SIZE;
	ubKNL_JPEGDecode(&tPhotoPlayNodeInfo, uwJpgHSize, uwJpgVSize, ulVdoYuvAddr, ulJpgBsAddr);
	osKNL_PhotoGraphFinSig = osSignalWait(osKNL_PhotoPlyFinSignal, 1000);
	tKNL_PhotoSts = ((osKNL_PhotoGraphFinSig.status == osEventSignal) &&
					 (osKNL_PhotoGraphFinSig.value.signals == osKNL_PhotoPlyFinSignal))?KNL_OK:KNL_ErrorTimeout;
	if(KNL_OK != tKNL_PhotoSts)
		goto PHOTOPLAY_ERR;
	if(KNL_DISP_ROTATE_90 == tKNL_GetDispRotate())
	{
		ubKNL_JPEGEncode(uwJpgHSize, uwJpgVSize, ulVdoYuvAddr, ulJpgBsAddr);
		osKNL_PhotoGraphFinSig = osSignalWait(osKNL_PhotoPlyFinSignal, 1000);
		tKNL_PhotoSts = ((osKNL_PhotoGraphFinSig.status == osEventSignal) &&
						 (osKNL_PhotoGraphFinSig.value.signals == osKNL_PhotoPlyFinSignal))?KNL_OK:KNL_ErrorTimeout;
		if(KNL_OK != tKNL_PhotoSts)
			goto PHOTOPLAY_ERR;
		tPhotoPlayNodeInfo.ubHMirror = JPEG_H_MIRROR;
		tPhotoPlayNodeInfo.ubRotate  = JPEG_ROT_90Deg;
		ubKNL_JPEGDecode(&tPhotoPlayNodeInfo, uwJpgHSize, uwJpgVSize, ulVdoYuvAddr, ulJpgBsAddr);
		osKNL_PhotoGraphFinSig = osSignalWait(osKNL_PhotoPlyFinSignal, 1000);
		tKNL_PhotoSts = ((osKNL_PhotoGraphFinSig.status == osEventSignal) &&
						 (osKNL_PhotoGraphFinSig.value.signals == osKNL_PhotoPlyFinSignal))?KNL_OK:KNL_ErrorTimeout;
		if(KNL_OK != tKNL_PhotoSts)
			goto PHOTOPLAY_ERR;
	}
	KNL_PhotoLcdDisplayOn();
	if(tKNL_RecordAct.pRecordStsNtyCb)
		tKNL_RecordAct.pRecordStsNtyCb(KNL_OK);
	return;
PHOTOPLAY_ERR:
	KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
	if(tKNL_RecordAct.pRecordStsNtyCb)
		tKNL_RecordAct.pRecordStsNtyCb(KNL_ErrorTimeout);
}
#endif
//------------------------------------------------------------------------------
void KNL_SetRecordFunc(KNL_RecordFunc_t tRecordFunc)
{
#if (KNL_PHOTOGRAPH_FUNC_ENABLE || KNL_REC_FUNC_ENABLE)
	osMutexWait(osKNL_RecordFuncMutex, osWaitForever);
	if(KNL_PHOTO_PLAY == tKNL_RecordAct.tRecordFunc)
		KNL_PhotoLcdDisplayOFF();
	tKNL_RecordAct.tRecordFunc = tRecordFunc;
	osMutexRelease(osKNL_RecordFuncMutex);
#endif
}
//------------------------------------------------------------------------------
KNL_RecordFunc_t tKNL_GetRecordFunc(void)
{
	KNL_RecordFunc_t tRecordFunc = KNL_RECORDFUNC_DISABLE;

#if (KNL_PHOTOGRAPH_FUNC_ENABLE || KNL_REC_FUNC_ENABLE)
	osMutexWait(osKNL_RecordFuncMutex, osWaitForever);
	tRecordFunc = tKNL_RecordAct.tRecordFunc;
	osMutexRelease(osKNL_RecordFuncMutex);
#endif
	return tRecordFunc;
}
//------------------------------------------------------------------------------
#if (KNL_PHOTOGRAPH_FUNC_ENABLE || KNL_REC_FUNC_ENABLE)
static void KNL_RecordThread(void const *argument)
{
	KNL_RecordAct_t tKNL_RecordEvt;
	KNL_Status_t tKNL_RecordSts;

	tKNL_RecordEvt.tRecordFunc = KNL_RECORDFUNC_DISABLE;
	while(1)
	{
		osMessageGet(osKNL_RecordMsgQue, &tKNL_RecordEvt, osWaitForever);
		if(KNL_RECORDFUNC_DISABLE == tKNL_RecordEvt.tRecordFunc)
		{
			KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
			continue;
		}
		tKNL_RecordAct.pRecordStsNtyCb = tKNL_RecordEvt.pRecordStsNtyCb;
		tKNL_RecordSts = tKNL_ChkSdCardSts();
		if(KNL_OK != tKNL_RecordSts)
		{
			if(tKNL_RecordAct.pRecordStsNtyCb)
				tKNL_RecordAct.pRecordStsNtyCb(tKNL_RecordSts);
			continue;
		}
	#if KNL_PHOTOGRAPH_FUNC_ENABLE
		switch(tKNL_RecordEvt.tRecordFunc)
		{
			case KNL_PHOTO_CAPTURE:
				KNL_PhotoCaptureFunc();
				break;
			case KNL_PHOTO_PLAY:
				KNL_PhotoPlayFunc(&tKNL_RecordEvt);
				break;
			default:
				break;
		}
	#endif
		tKNL_RecordEvt.tRecordFunc = KNL_RECORDFUNC_DISABLE;
	}
}
#endif
//------------------------------------------------------------------------------
void KNL_ExecRecordFunc(KNL_RecordAct_t tRecordAct)
{
	if(osKNL_RecordMsgQue)
		osMessagePut(osKNL_RecordMsgQue, &tRecordAct, 0);
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
