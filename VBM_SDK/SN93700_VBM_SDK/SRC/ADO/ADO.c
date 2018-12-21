/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ADO.c
	\brief		Audio Process for VBM
	\author		Hanyi Chiu
	\version	0.4
	\date		2017/12/25
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#if defined(VBM_PU) || defined(VBM_BU)

#include "ADO.h"

static ADO_KNLRoleInfo_t tADO_KNLRoleInfo[] = 
{
	//! OTHER_A source : Record, OTHER_B source : Push talk
	[KNL_STA1] = {KNL_SRC_1_OTHER_A, KNL_SRC_1_OTHER_B},
	[KNL_STA2] = {KNL_SRC_2_OTHER_A, KNL_SRC_2_OTHER_B},
	[KNL_STA3] = {KNL_SRC_3_OTHER_A, KNL_SRC_3_OTHER_B},
	[KNL_STA4] = {KNL_SRC_4_OTHER_A, KNL_SRC_4_OTHER_B},
	[KNL_NONE] = {KNL_SRC_NONE, 	 KNL_SRC_NONE},
};
static KNL_ROLE	tADO_TargetRole;
//------------------------------------------------------------------------------
void ADO_Init(void)
{
	KNL_ROLE tADO_KNLRole = KNL_STA1;

	//! Audio Parameter Setting
	tADO_TargetRole = KNL_NONE;
	ADO_KNLParamSetup();

	//! Data Path Setting
	KNL_AdoPathReset();
	KNL_SetAdoRoleInfoCbFunc(ADO_GetSourceNumber);
#ifdef VBM_BU
	tADO_KNLRole = (KNL_ROLE)ubKNL_GetRole();
	tADO_KNLRole = (KNL_NONE != tADO_KNLRole)?tADO_KNLRole:KNL_STA1;
	ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(tADO_KNLRole, ADO_PTT_SRC);
#endif
#ifdef VBM_PU
	for(tADO_KNLRole = KNL_STA1; tADO_KNLRole < DISPLAY_MODE; tADO_KNLRole++)
		ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(KNL_STA1, ADO_PTT_SRC);
#endif
}
//------------------------------------------------------------------------------
void ADO_DataPathSetup(KNL_ROLE tADO_KNLRole, ADO_SrcType_t tADO_SrcType)
{
#ifdef VBM_PU
	KNL_NODE tADO_KNLSrcMainPath[] = {KNL_NODE_COMM_RX_ADO, KNL_NODE_DAC_BUF, KNL_NODE_DAC, KNL_NODE_END};
	KNL_NODE tADO_KNLSrcPttPath[]  = {KNL_NODE_ADC, KNL_NODE_ADC_BUF, KNL_NODE_COMM_TX_ADO, KNL_NODE_END};
#endif
#ifdef VBM_BU
	KNL_NODE tADO_KNLSrcMainPath[] = {KNL_NODE_ADC, KNL_NODE_ADC_BUF, KNL_NODE_COMM_TX_ADO, KNL_NODE_END};
	KNL_NODE tADO_KNLSrcPttPath[]  = {KNL_NODE_COMM_RX_ADO, KNL_NODE_DAC_BUF, KNL_NODE_DAC, KNL_NODE_END};
#endif
	KNL_NODE *pADO_KNLPath[ADO_SRC_MAX] = {[ADO_MAIN_SRC] = tADO_KNLSrcMainPath,
	                                       [ADO_PTT_SRC]  = tADO_KNLSrcPttPath};
	KNL_NODE_INFO tADO_KNLNodeInfo = {0};
	KNL_SRC tADO_KNLSrcNum;
	uint16_t uwADO_NodeNum, i;

	tADO_KNLSrcNum = (tADO_SrcType == ADO_MAIN_SRC)?tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SrcNum:tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SubSrcNum;
	uwADO_NodeNum  = ((tADO_SrcType == ADO_MAIN_SRC)?(sizeof tADO_KNLSrcMainPath):(sizeof tADO_KNLSrcPttPath)) / sizeof(KNL_NODE);
	for(i = 0; i < uwADO_NodeNum; i++)
	{
		tADO_KNLNodeInfo.ubPreNode	= (i == 0)?KNL_NODE_NONE:pADO_KNLPath[tADO_SrcType][i-1];
		tADO_KNLNodeInfo.ubCurNode 	= pADO_KNLPath[tADO_SrcType][i];
		tADO_KNLNodeInfo.ubNextNode = pADO_KNLPath[tADO_SrcType][i+1];
		ubKNL_SetAdoPathNode(tADO_KNLSrcNum, i, tADO_KNLNodeInfo);
	}
}
//------------------------------------------------------------------------------
#ifdef VBM_BU
void ADO_KNLSysInfoSetup(KNL_ROLE tADO_KNLRole)
{
	ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(tADO_KNLRole, ADO_PTT_SRC);
}
#endif
//------------------------------------------------------------------------------
#ifdef VBM_PU
void ADO_RemoveDataPath(KNL_ROLE tADO_Role)
{
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_Role].tKNL_SrcNum);
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_Role].tKNL_SubSrcNum);
	if(tADO_TargetRole == tADO_Role)
		tADO_TargetRole = KNL_NONE;
}
//------------------------------------------------------------------------------
void ADO_PTTStart(void)
{
	uint32_t i;
	
	if(tADO_TargetRole > KNL_STA4)
		return;
	
	for(i=0; i<ADO_AUDIO32_MAX_NUM; i++)
	{
		ADO_Audio32_Encoder_Init(i,ADO_GetAudio32EncFormat(i));
	}
#if APP_ADOENC_TYPE == ALAW_ENC
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);	
#else
	ADO_Noise_Process_Type(NOISE_NR, AEC_NR_16kHZ);
#endif 

	KNL_AdoStart(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
	printd(DBG_InfoLvl, "		=>PTT Play\n");
}
//------------------------------------------------------------------------------
void ADO_PTTStop(void)
{
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
	ubKNL_WaitNodeFinish(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
	printd(DBG_InfoLvl, "		=>PTT Stop\n");
}
#endif
//------------------------------------------------------------------------------
KNL_SRC ADO_GetSourceNumber(KNL_VA_DATAPATH tADO_Path, KNL_ROLE tADO_KNLRole)
{
	return (KNL_MAIN_PATH == tADO_Path)?tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SrcNum:(KNL_SUB_PATH == tADO_Path)?tADO_KNLRoleInfo[tADO_KNLRole].tKNL_SubSrcNum:KNL_SRC_NONE;
}
//------------------------------------------------------------------------------
void ADO_Start(KNL_ROLE tADO_Role)
{
	uint32_t i;

#ifdef VBM_PU
	KNL_ROLE tADO_KNLRole = KNL_STA1;
#endif

	if((tADO_Role > KNL_STA4) || (tADO_TargetRole == tADO_Role))
		return;

	if(KNL_NONE != tADO_TargetRole)
		ADO_Stop();
	tADO_TargetRole = tADO_Role;

	for(i=0; i<ADO_AUDIO32_MAX_NUM; i++)
	{
		ADO_Audio32_Encoder_Init(i,ADO_GetAudio32EncFormat(i));
	}

#if APP_ADOENC_TYPE == ALAW_ENC
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);	
#else
	ADO_Noise_Process_Type(NOISE_NR, AEC_NR_16kHZ);
#endif

#ifdef VBM_PU
	KNL_AdoPathReset();
	for(tADO_KNLRole = KNL_STA1; tADO_KNLRole < DISPLAY_MODE; tADO_KNLRole++)
		ADO_DataPathSetup(tADO_KNLRole, ADO_MAIN_SRC);
	ADO_DataPathSetup(tADO_TargetRole, ADO_PTT_SRC);
#endif
	KNL_AdoStart(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SrcNum);
#ifdef VBM_BU
	KNL_AdoStart(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
#endif
}
//------------------------------------------------------------------------------
void ADO_Stop(void)
{
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SrcNum);
	ubKNL_WaitNodeFinish(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SrcNum);
#ifdef VBM_BU
	KNL_AdoStop(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
	ubKNL_WaitNodeFinish(tADO_KNLRoleInfo[tADO_TargetRole].tKNL_SubSrcNum);
#endif
	tADO_TargetRole = KNL_NONE;
}
//------------------------------------------------------------------------------
void ADO_KNLParamSetup(void)
{
	ADO_KNL_PARA_t tADO_KNLParm;
	uint32_t i;

	tADO_KNLParm.Sys_speed			 = HIGH_SPEED;
	
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	tADO_KNLParm.Rec_device			 = SIG_DEL_ADC;
	tADO_KNLParm.Ply_device			 = R2R_DAC;
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	tADO_KNLParm.Rec_device			 = I2S_ADC;
	tADO_KNLParm.Ply_device			 = I2S_DAC;
#endif

    tADO_KNLParm.ADO_SigDelAdcMode   = ADO_SIG_DIFFERENTIAL;
	tADO_KNLParm.Rec_fmt.sign_flag   = SIGNED;
	tADO_KNLParm.Rec_fmt.channel     = MONO;
	tADO_KNLParm.Rec_fmt.sample_size = SAMPLESIZE_16_BIT;
	tADO_KNLParm.Rec_fmt.sample_rate = SAMPLERATE_16kHZ;
	tADO_KNLParm.Ply_fmt.sign_flag   = SIGNED;
	tADO_KNLParm.Ply_fmt.channel     = MONO;
	tADO_KNLParm.Ply_fmt.sample_size = SAMPLESIZE_16_BIT;
	tADO_KNLParm.Ply_fmt.sample_rate = SAMPLERATE_16kHZ;

#if APP_ADOENC_TYPE == AUDIO32_ENC
	tADO_KNLParm.Compress_method 	 = COMPRESS_NONE;
#endif
#if APP_ADOENC_TYPE == ALAW_ENC
	tADO_KNLParm.Compress_method 	 = COMPRESS_ALAW;
#endif

	tADO_KNLParm.Rec_buf_size        = BUF_SIZE_16KB;
	tADO_KNLParm.Ply_buf_size        = BUF_SIZE_32KB;
	tADO_KNLParm.Audio32_En_buf_size = BUF_SIZE_8KB;
	tADO_KNLParm.Audio32_De_buf_size = BUF_SIZE_8KB;
	tADO_KNLParm.WavPlay_buf_size    = BUF_SIZE_8KB;
	tADO_KNLParm.AAC_En_buf_size     = BUF_SIZE_8KB;
	tADO_KNLParm.AAC_De_buf_size     = BUF_SIZE_8KB;
	tADO_KNLParm.Alarm_buf_size 	 = BUF_SIZE_1KB;
	tADO_KNLParm.Recording_buf_size  = BUF_SIZE_64KB;
	tADO_KNLParm.Alaw_Dec_buf_size   = BUF_SIZE_16KB;

	tADO_KNLParm.Rec_buf_th			 = BUF_TH_4KB;
	tADO_KNLParm.Ply_buf_th        	 = BUF_TH_4KB;

	tADO_KNLParm.ulADO_BufStartAddr  = 0;
	KNL_SetAdoInfo(tADO_KNLParm);

	//! Latency Setting
	for(i=0; i<ADO_SRC_NUM; i++)
	{
		ADO_LatencySetting(i, 250);
	}

	//! Audio32 Setting
	for(i=0; i<ADO_AUDIO32_MAX_NUM; i++)
	{
		ADO_Audio32_Encoder_Init(i,SNX_AUD32_FMT16_16K_16KBPS);
		ADO_Audio32_Decoder_Init(i,SNX_AUD32_FMT16_16K_16KBPS);
	}
#if APP_ADOENC_TYPE == AUDIO32_ENC 
	ADO_Set_Audio32_Enable(ADO_ON);
#else
	ADO_Set_Audio32_Enable(ADO_OFF);
#endif

	//! AEC and NR Setting
#if APP_ADOENC_TYPE == ALAW_ENC
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);	
#else	
	ADO_Noise_Process_Type(NOISE_NR,AEC_NR_16kHZ);
#endif

	ADO_Set_DeHowling_Enable(ADO_OFF);

	ADO_SetDeHowlingLV(DeHowlingLV0);

	//! wav play volume compensation
	ADO_WavplayVolCompensation(1);
}
//------------------------------------------------------------------------------
#endif
