/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       REC_API.h
	\brief		Record header file
	\author		Wales
	\version    0.3
	\date		2017/03/21
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _REC_API_H_
#define _REC_API_H_

#include <stdint.h>

//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define REC_SRC_NUM         (0x04)  //!< Define numbers of record Source 
//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------
typedef enum _eRecFileFormat
{
	REC_FILE_AVI,   //!< AVI File format
	REC_FILE_MP4,   //!< MP4 File format
}EREC_FILE_FORMAT;

typedef enum _RecStreamType
{
	REC_STR_V1,
	REC_STR_A1,
	REC_STR_MAX,
	REC_STR_V2,	
}REC_STREAM_TYPE;

typedef enum _RecFrameType
{
	REC_P_VFRM = 0,
	REC_I_VFRM = 1,
	REC_SKIP_FRM = 2,
}REC_VDO_FRM_TYPE;

typedef enum _RecResolution
{
	REC_RES_NONE,		//!< Undefine
	REC_RES_FHD,		//!< FHD
	REC_RES_HD,		//!< HD
	REC_RES_WVGA,		//!< WVGA
	REC_RES_VGA,		//!< VGA
}REC_RESOLUTION;

typedef enum _RecSrcConfig{
	REC_SRC_UNCONFIGED, //!< File format of record source is not configuration
	REC_SRC_CONFIGED,   //!< File format of record source is configuration
}REC_SRC_CONFIG;

typedef enum _RecMode{
	REC_MODE_NORMAL,	//!< Stop Record type
	REC_MODE_PRERECORD,	//!< Record only once type
	REC_MODE_TIMELAPSE,	//!< Record continue type
}REC_MODE;

typedef enum _RecCmd{
	REC_CMD_START,      //!< start command
	REC_CMD_STOP,       //!< stop command
	REC_CMD_RESTART,    //!< re-start command
	REC_CMD_NEXT,       //!< to next file command
	REC_CMD_VIDEXT,		//!< Video stream come from external(wifi buffer) command
	REC_CMD_AUDEXT,		//!< Audio stream come from external(wifi buffer) command
	REC_CMD_VIDINT,		//!< Video stream come from internal(pre record buffer) command
	REC_CMD_AUDINT,		//!< Audio stream come from internal(pre record buffer) command
	REC_CMD_PREINIT,    //!< Pre-record initial command
	REC_CMD_PREVID,     //!< Switch video external command to pre record video command.
	REC_CMD_PREAUD,		//!< Switch audio external command to pre record audio command.
}REC_CMD;

typedef enum _RecEvent
{
	REC_EVENT_NORMAL = 0,   //!< Normal Record Event
	REC_EVENT_SCH = 1,      //!< Schdule Record Event
	REC_EVENT_MD = 2,       //!< Motion Record Event
}REC_EVENT;

typedef enum _RecState{
	REC_CREATE_NULL,    //!< Record thread is in IDEL status
	REC_CREATE_INIT,    //!< Record thread is in Initial status
	REC_CREATE_OK,      //!< Record thread create file and is in Recording status
	REC_CLOSE,          //!< Record thread close file status
}REC_STATE;

typedef enum _RecErrCode{
	REC_ERR_SRC_CONFIG,     //!< No any sourcees dont configured
	REC_ERR_FUNC_UNDEFINE,    //!< System timer doesnt install
	REC_ERR_PRE_MEMSIZE,    //!< Precord memory size is not enough
	REC_ERR_PRE_GOP,        //!< Precord GOP number is not enough
	REC_ERR_PRE_MEMINIT,    //!< Precord memory is not initial
	REC_ERR_MAX,
}REC_ERR_CODE;

typedef enum _RecExtFuncErrCode
{
	REC_ADO_ADR,		//!< Bit 0, 
	REC_ADO_SIZE,		//!< Bit 1, 
	REC_TIMER_1MS,		//!< Bit 2, 
	REC_EVENT_RECONE,	//!< Bit 3, 
	REC_FUNC_MAX,
}REC_EXTFUNC_ERR_CODE;

//------------------------------------------------------------------------------
//! brief Record Information
typedef struct _RecInfo {
	uint8_t     ubCh;           //!< Input Source
	uint8_t     ubCmd;          //!< Command
	uint8_t     ubPictureType;  //!< Frame Type
	uint8_t     ubRestartFg;    //!< System Restart Flag, it uses with "ubREC_VdoPrecReSync", let VdoAddSkipFrame() and vPrecCheckTimeResync() to achive the function of time switch(Tx time<->Rx time).
	uint32_t    ulDramAddr;     //!< Source frame  memory address
	uint32_t    ulSize;         //!< Source frame size
	uint32_t    ulTimeStamp;    //!< Frame time stamp
}REC_INFO;

//------------------------------------------------------------------------------
//! brief Record File Format
typedef struct _RecFileFormat {
	uint8_t     ubConfiged;     //!< Source file format configuration flag
	uint8_t     ubCh;           //!< Source Channel
	uint8_t     ubSourceType;   //!< Defines source type
	uint8_t     uwReserve;      //!< Reserve. Unused
	uint16_t    uwVFrmInterval; //!< Video Frame Interval
	uint16_t    uwAFrmInterval; //!< Audio Frame Interval
	uint32_t    ulHoSize1;      //!< Video1 horizontal resolution
	uint32_t    ulVoSize1;      //!< Video1 vertical resolution
	uint32_t    ulHoSize2;      //!< Video2 horizontal resolution
	uint32_t    ulVoSize2;      //!< Video2 vertical resolution
	uint32_t    ulSampleRate;   //!< Audio Sample Rate
	uint16_t    uwBlockAlign;   //!< Audio block alignment bytes.
	uint16_t    uwReserve1;     //!< Reserve
} REC_FILEFORMAT;

typedef struct _RecExtFuncCtrl
{
	uint32_t (*ulAdoSkipFrameAdrGet)(void);
	uint32_t (*ulAdoSkipFrameSizeGet)(void);
	uint32_t (*Time1msCntGet)(void);
	void (*EventRecOnceEnd)(void);
}REC_EXTFUNC_CTRL;
//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Get Record Version
\return	Firmware Verison
*/
uint16_t uwREC_GetVersion(void);

uint8_t ubREC_ExternFuncInit(REC_EXTFUNC_CTRL *pEventFuncTab);

//------------------------------------------------------------------------------
/*!
\brief
\param	FileFormat A pointer point to file format strcture
\return	0 Configure fail
		1 Configure success.
*/
uint8_t REC_FileFormatConfigure(REC_FILEFORMAT *FileFormat);

//------------------------------------------------------------------------------
/*!
\brief 	Send Video Queue Command
\param	pRecInfo A pointer point to record information strcture
\return	0 Send fail
		1 Send success
*/
uint8_t ubREC_SendVDOQueue(REC_INFO *pRecInfo);

//------------------------------------------------------------------------------
/*!
\brief 	Send Audio Queue Command
\param	pRecInfo A pointer point to record information strcture
\return	0 Send fail
		1 Send success
*/
uint8_t ubREC_SendADOQueue(REC_INFO *pRecInfo);

//------------------------------------------------------------------------------
/*!
\brief Set record file format
\param	ubFmt record file format
\return	(no)
*/
void vREC_FileFormatSet(uint8_t ubFmt);

//------------------------------------------------------------------------------
/*!
\brief Get record file format
\return	Record file format
*/
uint8_t ubREC_FileFormatGet(void);

#endif

