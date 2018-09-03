/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       PLY_API.h
	\brief		Play header file
	\author		Wales
	\version    0.2
	\date		2017/03/21
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _PLY_API_H_
#define _PLY_API_H_

#include "FS_API.h"

//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define PLY_FRMTYPE_NULL        (0)             //!< Bitstream type in avi file is undefined
#define PLY_FRMTYPE_VDO         (0x63643030)    //!< Bitstream type in avi file is video
#define PLY_FRMTYPE_ADO         (0x62773130)    //!< Bitstream type in avi file is audio
//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------
typedef enum _PlyExtFuncErrCode
{
	PLY_ADO_START,		//!< Bit 0, 
	PLY_ADO_STOP,		//!< Bit 1, 
	PLY_ADO_CHPLYSET,	//!< Bit 2, 
	PLY_ADO_CHSET,		//!< Bit 3, 
	PLY_ADO_CHGET,		//!< Bit 4, 
	PLY_ADO_BUFWR,		//!< Bit 5, 
	PLY_ADO_BUFCHK,		//!< Bit 6, 
	PLY_ADO_BUFRST,		//!< Bit 7, 
	PLY_ADO_MUTE,		//!< Bit 8, 
	PLY_TIMER_1MS,		//!< Bit 9, 
	PLY_RTC_1S,			//!< Bit 10, 
	PLY_H264_ENTRY,		//!< Bit 11, 
	PLY_H264_CHGET,		//!< Bit 12,
	PLY_EVENT_FILE,		//!< Bit 13, 
	PLY_EVENT_TIMEBAR,	//!< Bit 14, 
	PLY_EVENT_PLAYOVER,	//!< Bit 15, 
	PLY_FUNC_MAX,
}PLY_EXTFUNC_ERR_CODE;

typedef struct _PlyAudioExtFuncTab
{
	void (*AdoStart)(void);
	void (*AdoStop)(void);
	void (*AdoSetPlayCh)(void);
	void	(*AdoCurChSet)(uint8_t ubCh);
	uint8_t (*ubAdoCurChGet)(void);
	void (*AdoBufWrite)(uint32_t ulSrcAdr, uint32_t ulSize, uint8_t ubCopy);
	uint8_t (*ubAdoBuffChk)(uint32_t ulSize);
	void (*AdoResetBuf)(void);
	void	(*AdoMute)(uint8_t ubEn);
} PLY_AUDIO_EXTFUNC_TAB;

typedef struct _PlyTimeExtFuncTab
{
	uint32_t (*Time1msCntGet)(void);
	void (*Time1sRTCGet)(uint32_t *ulVal);
}PLY_TIME_EXTFUNC_TAB;

typedef struct _PlyH264ExtFuncTab
{
	int32_t (*H264DecEntry)(uint8_t ubCh, uint8_t ubDecSkipFrameFg);
	uint8_t (*H264DeceTargetChGet)(void);
}PLY_H264_EXTFUNC_TAB;

typedef struct _PlyEventExtFuncTab
{
	void (*EventFileOpened)( uint8_t ubDispMode);
	void (*EventTimeBarfresh)(void);
	void (*EventPlayOver)(void);
} PLY_EVENT_EXTFUNC_TAB;

typedef struct _PlyExtFuncCtrl
{
	PLY_AUDIO_EXTFUNC_TAB sAdoFunc;
	PLY_TIME_EXTFUNC_TAB sTimeFunc;
	PLY_H264_EXTFUNC_TAB sH264Func;
	PLY_EVENT_EXTFUNC_TAB sEventFunc;
}PLY_EXTFUNC_CTRL;

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Get Play Version
\return	(no)
*/
uint16_t uwPLY_GetVersion(void);

uint8_t ubPLY_AudioExtFuncInit(PLY_AUDIO_EXTFUNC_TAB *sAdoFuncTab);
uint8_t ubPLY_TimeExtFuncInit(PLY_TIME_EXTFUNC_TAB *pTimeFuncTab);
uint8_t ubPLY_H264ExtFuncInit(PLY_H264_EXTFUNC_TAB *pH264FuncTab);
uint8_t ubPLY_EventExtFuncInit(PLY_EVENT_EXTFUNC_TAB *pEventFuncTab);

//------------------------------------------------------------------------------
/*!
\brief 	Caculate requested memory buffer size
\param	ubSrcNum Numbers of total source channel
\return	Requested memory buffer size
*/
uint32_t ulPLY_GetBufSz(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play memory pin pon buffer initial.
\param	ubSrcNum Numbers of total source channel
\return	(no)
*/
void PLY_PinPonBuffInit(uint32_t ulStartAdr);

//------------------------------------------------------------------------------
/*!
\brief 	Set Operation Mode Play flag function
\param	ubEn Set Operation Mode Play flag
\return	(no)
*/
void PLY_SetOpModePly(uint8_t ubEn);

//------------------------------------------------------------------------------
/*!
\brief 	Set Operation Mode Record flag function
\param	ubEn Set Operation Mode Record flag
\return	(no)
*/
void PLY_SetOpModeRec(uint8_t ubEn);

//------------------------------------------------------------------------------
/*!
\brief 	Get Operation Mode function
\return	PLY_MODE_N, Kernel Operation mode is in preview mode.
		PLY_MODE_R, Kernel Operation mode is in preview with recording mode.
		PLY_MODE_PR, Kernel Operation mode is in play with recording mode.
		PLY_MODE_P, Kernel Operation mode is in play mode.
*/
uint8_t ubPLY_GetOpMode(void);

//------------------------------------------------------------------------------
/*!
\brief 	Chk RTC Time Set Ready function
\return	0, RTC Time Setting is not Ready
		1, RTC Time Setting is Ready
*/
uint8_t ubPLY_ChkTimeValid(void);

//------------------------------------------------------------------------------
/*!
\brief	Get Total Play time function
\return	Unit is second
*/
uint32_t ulPLY_GetTotalPlayTime(void);

//------------------------------------------------------------------------------
/*!
\brief 	Get Current Play time function
\return	Unit is second
*/
uint32_t ulPLY_GetSchedule(void);

//------------------------------------------------------------------------------
/*!
\brief 	Get Play timer bar level function
\return	Range = 0-20
*/
uint8_t ubPLY_GetTimeBarLvl(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play start function
\param	ubPlayMode Play mode, 1T,2T,3T,4T
\param	ubActiveFileCnt How many file will be play
\param	*pPtr A pointer point to File information structure
\return	0, Run play start function successfully
		-1, Bad Sector
		-2, HDD or SD Card not Ready
*/
uint8_t ubPLY_Start(uint8_t ubPlayMode, uint8_t ubActiveFileCnt, FS_FILE_HIDDEN_INFO_t *pPtr);

//------------------------------------------------------------------------------
/*!
\brief 	Play jump function
\param	ubType PLY_JUMP_FWD,PLY_JUMP_BWD,PLY_JUMP_KEEP
\return	(no)
*/
uint8_t ubPLY_Jump(uint8_t ubType);

//------------------------------------------------------------------------------
/*!
\brief	Play Stop function
\return	0, Run play stop success
		-1, Queue xQueue_Play full
*/
uint8_t ubPLY_Stop(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play pause function
\return	(no)
*/
uint8_t ubPLY_Pause(void);

//------------------------------------------------------------------------------
/*!
\brief 	Play video again function to restart H264 decode
\param	ubCh Source Channel
\return	0, restart H264 decode success.
		-1, restart H264 decode fail.
*/
uint8_t ubPLY_VdoAgain(uint8_t ubCh);

//------------------------------------------------------------------------------
uint8_t ubPLY_ChangeSpeed(uint8_t ubSpeed);

//------------------------------------------------------------------------------
/*!
\brief 	Set Play audio channel
\param	ubCh Source Channel
\return	(no)
*/
uint8_t ubPLY_AdoChannelSet(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get Play audio channel
\return	Nth Source channel be selected
*/
uint8_t ubPLY_AdoChannelGet(void);

//------------------------------------------------------------------------------
/*!
\brief	Get LCD display mode in play
\return	LCD display mode
*/
uint8_t ubPLY_DisplayModeGet(void);

//------------------------------------------------------------------------------
uint8_t ubPLY_GetResolution(uint8_t ubCh);

#endif

