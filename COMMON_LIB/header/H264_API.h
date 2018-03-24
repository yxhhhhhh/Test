/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMG.c
	\brief		Image Control
	\author		Bruce Hsu	
	\version	0.8
	\date		2017/12/19
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __H264_API__
#define __H264_API__

#include "_510PF.h"
//#include "IMG_API.h"

/*!	
\file H264_API.h
*/

typedef enum {
	_800x480,
	_1280x720,
	_1920x1088
}Resolution;

typedef enum {
	FPS15,
	FPS30
}FPS;

typedef struct{
	Resolution RES;
	FPS FPS;
	uint8_t H264Clock;
	uint8_t IMGClock;
}ClockRate;

typedef enum 
{
	H264_ENCODE = 0,		
	H264_DECODE
}H264_TYPE;

typedef enum
{
	H264_SUCESS,
	H264_FAIL
}H264_BOOL_RESULT;

typedef enum
{
	H264_DISABLE,
	H264_ENABLE
	
}H264_ENABLE_STATUS;


typedef enum
{
	CQP,						//!< Constant QP
	CBR,					//!< Constant Bitrate
	LBR						//!< Lower Bitrate
	
}H264_RATE_CONTROL_MODE;
/*!
\brief H264 Encode stream selection 
*/
typedef enum
{
	ENCODE_0,			//!< Encoder Index 0
	ENCODE_1,			//!< Encoder Index 1
	ENCODE_2,			//!< Encoder Index 2
	ENCODE_3			//!< Encoder Index 3
	
}H264_ENCODE_INDEX;
/*!
\brief H264 Decode stream selection 
*/
typedef enum
{
	DECODE_0,			//!< Decoder Index 0	
	DECODE_1,			//!< Decoder Index 1
	DECODE_2,			//!< Decoder Index 2
	DECODE_3,			//!< Decoder Index 3
	DECODE_4,			//!< Decoder Index 4
	DECODE_5			//!< Decoder Index 5
}H264_DECODE_INDEX;
/*!
\brief MROI index selection 
*/
typedef enum
{
	MROI_0,				//!< MROI Index 0	
	MROI_1,				//!< MROI Index 1
	MROI_2,				//!< MROI Index 2
	MROI_3,				//!< MROI Index 3
	MROI_4,				//!< MROI Index 4
	MROI_5,				//!< MROI Index 5
	MROI_6,				//!< MROI Index 6
	MROI_7				//!< MROI Index 7
	
}H264_MROI_INDEX;

/*!
\brief MROI extension size 
*/
typedef enum
{
	SIZE_0,				//!<Extension Size =0
	SIZE_3,				//!<Extension Size =3
	SIZE_7				//!<Extension Size =7	
	
}H264_MROI_EXT_SIZE;
/*!
\brief AROI method 
*/
typedef enum
{
	MV_OR_SKIN,		//!<MV||Skin
	MV,						//!<Only MV	
	SKIN,					//!<Only Skin
	MV_AND_SKIN		//!<MV&&Skin
}AROI_METHOD;

/*!
\brief ROI report method 
*/
typedef enum
{
	MROI,         //!< MROI first              	                                                                                                                                                                                                                                              
	AROI,					//!< AROI first
	QP_LOWER			//!< QP Lower first
	
}ROI_PRIORITY;

/*!
\brief H264 result structure 
*/
typedef struct 
{
	H264_TYPE Type;									//!< H264 working type (ENCODE/DECODE) 
	H264_ENCODE_INDEX EncodeStream;	//!< Index of encode stream (0~3)
	H264_DECODE_INDEX DecodeStream;	//!< Index of decode stream (0~3)
	H264_BOOL_RESULT Result;					//!< IMG_SUCESS/IMG_FAIL
	uint32_t Size;									//!< Size of encoded/decoded result data
	uint32_t YuvAddr;								//!< Yuv data address
	uint32_t BSAddr;								//!< Bitstream data address
	uint32_t ulFrmIdx;							//!< Frame index
	uint32_t ulGop;									//!< GOP
}H264_RESULT;
/*!
\brief H264 and JPEG and Scalling down result structure 
*/
struct IMG_RESULT
{
	H264_RESULT* H264Result;
	uint32_t YuvAddr;
	uint32_t JPEGDesAddr;
	uint32_t ScalingDesAddr;
	uint32_t JPEGSize;
	uint32_t ScalingSize;
};
/*!
\brief H264 task structure 
*/
struct H264_TASK
{
	H264_TYPE Type;									//!< H264 working type (ENCODE/DECODE) 
	H264_ENCODE_INDEX EncodeStream;	//!< Index of encode stream (0~3)
	H264_DECODE_INDEX DecodeStream;	//!< Index of decode stream (0~3)
	uint32_t DesAddr;								//!< Destination address
};

/*!
\brief MROI setup structure
*/
typedef struct 
{
	H264_ENABLE_STATUS ENABLE;			//!< ENABLE/DISABLE
	H264_MROI_INDEX Num;					//!< Index of MROI(0~6)
	uint8_t ubWeight;				//!< Priority of region,0 is first priority, 7 is last
	int16_t uwQP_Value;			//!< Delata QP Value of each region (range +9 ~ -9)
	uint32_t ulPosX;					//!< X-axis start postion of region(MB unit)
	uint32_t ulWidth;				//!< Width of region
	uint32_t ulPosY;					//!< Y-axis start postion of region(MB unit)
	uint32_t ulHeight;				//!< Height of region
	H264_MROI_EXT_SIZE ExtSize;	//!< Extension size,can be 0,3 and 7
}H264_MROI_SETUP;

typedef struct
{
	uint8_t ubParQPAdj0;
	uint8_t ubParQPAdj1;
	uint8_t ubParQPAdj2;
	uint8_t ubParQPAdj3;
	uint8_t ubParQPAdj4;
	uint8_t ubParQPAdj5;
	uint8_t ubParQPAdj6;
	uint8_t ubParQPNum0;
	uint8_t ubParQPNum1;
	uint8_t ubParQPNum2;
	uint8_t ubParQPNum3;
	uint8_t ubParQPNum4;
	uint8_t ubParQPNum5;
	uint8_t ubParQPNum6;
	uint8_t ubParQPNum7;
	
}H264_PAR_SETUP;

typedef struct
{
	
	uint8_t MB_MODE;
	uint8_t MB_THD;
	uint8_t MBQPAdj0;
	uint8_t MBQPAdj1;
	uint8_t MBQPAdj2;
	uint8_t MBQPAdj3;
	uint8_t MBQPAdj4;
	uint8_t MBQPAdj5;
	uint8_t MBQPNum0;
	uint8_t MBQPNum1;
	uint8_t MBQPNum2;
	uint8_t MBQPNum3;
	uint8_t MBQPNum4;
	uint8_t MBQPNum5;
	uint8_t MBQPNum6;
	
}H264_MB_QP_SETUP;


//------------------------------------------------------------------------
/*!
\brief Get Request Buffer Size Of Stream
\param ulWidth				Image width
\param ulHeight				Image height
\return ulBufferSize
\par [Example]
\code 
			uint32_t ulBufferSize = ulH264_Get_ENC_Buffer_Size(640,480);
\endcode
*/
uint32_t 	ulH264_GetENCBufferSize(uint32_t ulWidth,uint32_t ulHeight);
//------------------------------------------------------------------------
/*!
\brief Initial H264 Encoder Object
\param EncodeIndex			Index of encoder object
\param ulWidth					Image width
\param ulHeight					Image height
\param ulBufferStrAddr	Address of buffer of Kernel offering		
\return (no)
\par [Example]
\code 
			H264_EncodeInit(ENCODE_0,640,480,0x190000);
\endcode
*/
//void H264_EncodeInit(H264_ENCODE_INDEX EncodeIndex,uint32_t ulWidth, uint32_t ulHeight,uint32_t ulBufferStrAddr);
void H264_EncodeInit(H264_ENCODE_INDEX EncodeIndex,uint32_t ulWidth, uint32_t ulHeight,uint32_t ulBufferStrAddr,uint32_t ulFrameRate,uint32_t ulIntraPeriod);

void H264_SetFrameRate(H264_ENCODE_INDEX EncodeIndex,uint32_t ulFrameRate);
	
//------------------------------------------------------------------------
/*!
\brief Set QP of Encoder
\param EncodeIndex		Index of encoder object
\param ubIFrameQP						Value of QP(1~51)
\param ubPFrameQP						Value of QP(1~51)
\return(no)
\par [Example]
\code 
			H264_SetQP(ENCODE_0,25,25);
\endcode
*/
void H264_SetQp(H264_ENCODE_INDEX EncodeIndex,uint8_t ubIFrameQP,uint8_t ubPFrameQP);
//------------------------------------------------------------------------
/*!
\brief Set GOP of Encoder
\param EncodeIndex		Index of encoder object
\param ubGOP					Value of GOP
\return(no)
\par [Example]
\code 
			H264_SetGOP(ENCODE_0,30);
\endcode
*/
void H264_SetGOP(H264_ENCODE_INDEX EncodeIndex,uint32_t ulGOP);
//------------------------------------------------------------------------
/*!
\brief Get GOP of Encoder
\param EncodeIndex		Index of encoder object
\return GOP
\endcode
*/
uint32_t H264_GetGOP(H264_ENCODE_INDEX EncodeIndex);
//------------------------------------------------------------------------

/*!
\brief Set Max QP of Encoder
\param EncodeIndex		Index of encoder object
\param ubQP						Value of Max QP (range 1~51)
\return(no)
\par [Example]
\code 
			H264_SetMaxQP(ENCODE_0,30);
\endcode
*/
void H264_SetMaxQP(H264_ENCODE_INDEX EncodeIndex,uint8_t ubQP);
//------------------------------------------------------------------------
/*!
\brief Set Min QP of Encoder
\param EncodeIndex		Index of encoder object
\param ubQP						Value of Min QP (range 1~51)
\return(no)
\par [Example]
\code 
			H264_SetMinQP(ENCODE_0,10);
\endcode
*/
void H264_SetMinQP(H264_ENCODE_INDEX EncodeIndex,uint8_t ubQP);


//------------------------------------------------------------------------
/*!
\brief Force next frame is I frame
\param EncodeIndex	Index of encoder object
\return(no)
\par [Example]
\code 
		H264_ResetIPCnt(ENCODE_0);
\endcode
*/
void H264_ResetIPCnt(H264_ENCODE_INDEX EncodeIndex);
//------------------------------------------------------------------------

/*!
\brief H264 Reset  
\return (no)
*/
void H264_Reset(void);

//------------------------------------------------------------------------
/*!
\brief Set MROI Mode Enable 
\param EncodeIndex			Index of encoder object
\param ubStatus					Enable/Disable
\return(no)
\par [Example]
\code 
		H264_SetMROIModeEnable(ENCODE_0,ENABLE);
\endcode
*/
void H264_SetMROIModeEnable(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS ubStatus);

//------------------------------------------------------------------------
/*!
\brief Set MROI Parameter 
\param EncodeIndex			Index of encoder object
\param Setup						Structure of MROISetup
\return(no)
\par [Example]
\code 
	H264_MROI_SETUP MROISetup;
	
	MROISetup.Num = MROI_0;
	MROISetup.ENABLE = ENABLE;
	MROISetup.ubWeight =1;
	MROISetup.uwQP_Value =-1;
	MROISetup.ulPosX = 3;
	MROISetup.ulWidth = 3;
	MROISetup.ulPosY = 1;
	MROISetup.ulHeight = 4;
	MROISetup.ExtSize =SIZE_0;
	
	H264_SetMROI(ENCODE_0,MROISetup);
\endcode
*/
void H264_SetMROI(H264_ENCODE_INDEX EncodeIndex,H264_MROI_SETUP Setup);
//------------------------------------------------------------------------
/*!
\brief Set AROI Paratemer And Status
\param EncodeIndex		Index of encoder object
\param ubStatus				Enable/Disable
\param ubPriority			ROI ouput decision method(0:MROI first ,1:AROI first,2:QP Lower first)
\param ubMethod				AROI generate method (0:MV||Skin,1:MV,2:Skin,3MV&Skin)
\return(no)
\par [Example]
\code 
		H264_SetAROIEN(ENCODE_0,ENABLE,MROI,MV_AND_SKIN);
\endcode
*/
void H264_SetAROIEN(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS ubStatus,ROI_PRIORITY ubPriority,AROI_METHOD ubMethod);
//------------------------------------------------------------------------
/*!
\brief Set Skin Paratemer 
\param EncodeIndex			Index of encoder object
\param ubStatus					Enable/Disable
\return(no)
\par [Example]
\code 
		H264_SetSkinMode(ENCODE_0,ENABLE);
\endcode
*/
void H264_SetSkinMode(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS ubStatus);
//------------------------------------------------------------------------
/*!
\brief Set AROI Paratemer And Status
\param EncodeIndex		Index of encoder object
\param Status			Enable/Disable
\param ubSR				Search Range (range 0~7)
\return(no)
\par [Example]
\code 
		H264_SetCondensedMode(ENCODE_0,ENABLE,7);
\endcode
*/
void H264_SetCondensedMode(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS Status,uint8_t ubSR);
//------------------------------------------------------------------------
/*!
\brief Set Rate Control Enable
\param EncodeIndex		Index of encoder object
\param Status					Enable/Disable
\param Mode						RateControl mode	
\param ulTargetBS			Target bitRate
\return(no)
\par [Example]
\code 
		H264_RcSetEN(ENCODE_0,ENABLE,100000);
\endcode
*/
void H264_RcSetEN(H264_ENCODE_INDEX EncodeIndex,H264_ENABLE_STATUS Status ,H264_RATE_CONTROL_MODE Mode ,uint32_t ulTargetBS);
//------------------------------------------------------------------------
/*!
\brief Set Rate Control Target BitRate
\param EncodeIndex		Index of encoder object
\param ulTargetBS			Target bitRate
\return(no)
\par [Example]
\code 
		H264_RcSetTargetBS(ENCODE_0,100000);
\endcode
*/
void H264_RcSetTargetBS(H264_ENCODE_INDEX EncodeIndex,uint32_t ulTargetBS);
//------------------------------------------------------------------------
/*!
\brief Get Request Buffer Size Of Stream
\param ulWidth				Image width
\param ulHeight				Image height
\return BufferSize
\par [Example]
\code 
	uint32_t ulBufferSize = ulH264_Get_DEC_Buffer_Size(640,480);
\endcode
*/
uint32_t 	ulH264_GetDECBufferSize (uint32_t ulWidth,uint32_t ulHeight);
//------------------------------------------------------------------------
/*!
\brief Initial H264 		Decoder Object
\param DecodeIndex			Index of decoder object
\param ulWidth					Image width
\param ulHeight					Image height
\param ulBufferStrAddr	Address of buffer of Kernel offering
\return (no)
\par [Example]
\code 
	H264_DecodeInit(ENCODE_0,640,480,0x190000);
\endcode
*/
void H264_DecoderInit(H264_DECODE_INDEX DecodeIndex,uint32_t ulWidth, uint32_t ulHeight,uint32_t ulBufferAddr);
//------------------------------------------------------------------------
/*!
\brief Set H264 Decode Mirror Enable
\param DEcodeIndex		Index of decoder object
\param Status 				ENABLE\DISABLE
\return(no)
\par [Example]
\code 
		H264_SetMirrorEn(DECODE_0,ENABLE);
\endcode
*/
void H264_SetMirrorEn(H264_DECODE_INDEX DecodeIndex ,H264_ENABLE_STATUS Status);
//------------------------------------------------------------------------
/*!
\brief Set H264 Decode Flip Enable
\param DEcodeIndex		Index of decoder object
\param Status 				ENABLE\DISABLE
\return(no)
\par [Example]
\code 
		H264_SetFlipEn(DECODE_0,ENABLE);
\endcode
*/
void H264_SetFlipEn(H264_DECODE_INDEX DecodeIndex ,H264_ENABLE_STATUS Status);
//------------------------------------------------------------------------
/*!
\brief Set H264 Decode Rotation Enable
\param DEcodeIndex		Index of decoder object
\param Status 				ENABLE\DISABLE
\return(no)
\par [Example]
\code 
		H264_SetRotationEn(DECODE_0,ENABLE);
\endcode
*/
void H264_SetRotationEn(H264_DECODE_INDEX DecodeIndex ,H264_ENABLE_STATUS Status);
//------------------------------------------------------------------------
/*!
\brief Get H264 Current Frame QP
\return(no)
\code 
		H264_GetCurrentQP();
\endcode
*/
uint8_t H264_GetCurrentQP(void);
//------------------------------------------------------------------------
/*!
\brief Reset Rate Control
\return(no)
\code 
		H264_ResetRateControl();
\endcode
*/
void H264_ResetRateControl(uint8_t ubInitQp);
//------------------------------------------------------------------------
/*!
\brief Get H264 Encode Stream Size
\return H264_Encode_Size
\code 
		ulH264_GetStreamSize();
\endcode
*/
uint32_t ulH264_GetStreamSize(void);
//------------------------------------------------------------------------
/*!
\brief Set Rate Control Parameter
\return H264_Encode_Size
\code 
		ulH264_GetStreamSize();
\endcode
*/
void H264_SetRCParameter(H264_ENCODE_INDEX EncodeIndex, uint32_t Bitrate, uint32_t FrameRate);
//------------------------------------------------------------------------
/*!
\brief Get Decode Error Flag
\param DecodeIndex 				Index of decoder object
\return Error=1 
\code 
		ubH264_GetDecErrorFlg();
\endcode
*/
uint8_t ubH264_GetDecErrorFlg(H264_DECODE_INDEX DecodeIndex);
//------------------------------------------------------------------------
/*!
\brief Set Decode Error Flag
\param DecodeIndex 				Index of decoder object
\param ubFlg							If Error flag = 1 else flag = 0
\return(no)
\code 
		H264_SetErrorFlg(DECODE_0,0);
\endcode
*/
void H264_SetErrorFlg(H264_DECODE_INDEX DecodeIndex,uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief 	Get H264 Version	
\return	Version
*/
uint16_t uwH264_GetVersion(void);
//------------------------------------------------------------------------
/*!
\brief Set H264 Clock Rate
\param Resolution 				Image Resolution
\param FPS								Image Frame per second
\return(no)
\code 
		SetH264Rate(1280,720,15);
\endcode
*/
void SetH264Rate(Resolution res,FPS fps);
#endif
