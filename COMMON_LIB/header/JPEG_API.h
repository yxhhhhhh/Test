/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		JPEG_API.h
	\brief		JPEG Codec API Header file
	\author		Hanyi Chiu
	\version	0.5
	\date		2017/11/30
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _JPEG_API_H_
#define _JPEG_API_H_

#include "_510PF.h"

#define JPEG_DECODE_ORIGINAL			0
#define JPEG_DECODE_FAIL 				0
#define JPEG_DECODE_OK 					1
#define JPEG_MAX_BS_SZ					0x800000L

#define JPEG_FuncEnable					(GLB->JPG_FUNC_DIS = 0)
#define JPEG_FuncDisable				(GLB->JPG_FUNC_DIS = 1)

typedef enum
{
	JPEG_PASS,
	JPEG_FAIL
}JPEG_RESULT;

typedef enum
{
	JPEG_SEMAPHORE,						//!< Event triggered by RTOS semaphore
	JPEG_QUEUE,							//!< Event triggered by RTOS Queue
	JPEG_EM_NULL
}JPEG_EVENT_METHOD_t;

typedef enum
{
	JPEG_FN_USE_POLLING,				//!< Notify mode use CPU polling
	JPEG_FN_USE_ISR						//!< Nofity mode use interrupt
}JPEG_CODEC_FN_MODE_t;

typedef enum
{
	JPEG_DECODE = 0,
	JPEG_ENCODE
}JPEG_CODEC_MODE_t;

typedef enum
{
	JPEG_CODEC_DISABLE = 0,
	JPEG_CODEC_ENABLE
}JPEG_CODEC_STATUS_t;

typedef enum
{
	JPEG_YUV420,						//!< YUV420 format
	JPEG_YUV422							//!< YUV422 format
}JPEG_CODEC_FMT_t;

typedef enum
{
	JPEG_CODEC_ERR,
	JPEG_CODEC_AHB_ERR,
	JPEG_CODEC_OK,
}JPEG_CODEC_ERR_FLAG_t;

typedef enum
{
	JPEG_SCALE_DOWN_HALF = 1,			//!< Scaling down 1/2
	JPEG_SCALE_DOWN_QUARTER				//!< Scaling down 1/4
}JPEG_DEC_SCALER_MODE_t;

typedef enum
{
	JPEG_MIRROR_DISABLE = 0,
	JPEG_H_MIRROR = 1,					//!< Horizontal mirror
	JPEG_V_MIRROR						//!< Vertical mirror
}JPEG_DEC_MIRROR_MODE_t;

typedef enum
{
	JPEG_ROT_DISABLE = 0,
	JPEG_ROT_90Deg = 1,					//!< 90 degree
}JPEG_DEC_ROT_MODET_t;

typedef struct
{
	uint32_t ulJpeg_Buf_Start;			//!< Start address
	uint32_t ulJpeg_Buf_End;			//!< End address
}JPEG_FIFO_Addr_t;

typedef struct
{
	uint16_t uwH_ORI_SIZE;				//!< Horizontal size of the original image
	uint16_t uwH_SIZE;					//!< Horizontal size
	uint16_t uwV_SIZE;					//!< Vertical size
	uint16_t uwH_START;					//!< Horizontal start position of the original image
	uint16_t uwV_START;					//!< Vertical start position of the original image
	uint8_t ubJPG_Fmt;					//!< JPEG bitstream format
}JPEG_ENC_INFO_t;

typedef struct
{
	uint16_t uwH_ORI_SIZE;				//!< Horizontal size of the original image
	uint16_t uwH_SIZE;					//!< Horizontal size
	uint16_t uwV_SIZE;					//!< Vertical size
	uint8_t ubJPG_Fmt;					//!< JPEG bitstream format
	uint16_t uwQP;
	uint8_t ubJPG_ScaleMode;			//!< JPEG scaling mode of the decoded image
	uint8_t ubJPG_Mirror;				//!< JPEG mirror mode of the decoded image
	uint8_t ubJPG_Rotate;				//!< JPEG rotate mode of the decoded image
}JPEG_DEC_INFO_t;

typedef struct
{
	JPEG_CODEC_FN_MODE_t tNotifyMode;	//!< Event Type (CPU polling or interrupt mode)
	JPEG_EVENT_METHOD_t	 tEM;			//!< Select event method for interrupt mode
	void				 *pvEvent;		//!< Event handler
}JPEG_CODEC_FN_ES_t;

typedef struct
{
	JPEG_CODEC_MODE_t tJPEG_CodecMode;	//!< Codec Action : 0->Encode, 1->Decode
	uint32_t ulJPEG_YUVAddr;			//!< YUV Address
	uint32_t ulJPEG_BsAddr;				//!< Bit-Stream Address
	uint32_t ulJPEG_BsSize;				//!< Bit-Stream Size
}JPEG_CODEC_INFO_t;

//------------------------------------------------------------------------
/*!
\brief JPEG Codec initialize
\return(no)
*/
void JPEG_Init(void);
//------------------------------------------------------------------------
/*!
\brief Write Q Table
\param Q_Table			Q table
\return(no)
*/
void JPEG_Write_QTab(uint8_t *Q_Table);
//------------------------------------------------------------------------
/*!
\brief Read Q Table
\return Q table
*/
uint8_t *JPEG_Read_QTab(void);
//------------------------------------------------------------------------
/*!
\brief Set QP value
\param ubQP 			QP Value
\return(no)
*/
void JPEG_Set_QP_Value(uint8_t ubQP);
//------------------------------------------------------------------------
/*!
\brief Ring FIFO setup of JPEG Codec
\param jpg_addr 		Structure of JPEG FIFO address.
\return(no)
*/
void JPEG_Ring_FIFO_Setup(JPEG_FIFO_Addr_t jpg_addr);
//------------------------------------------------------------------------
/*!
\brief Setup encode parameter of JPEG Codec
\param jpg_info 		Parameter of JPEG encode.
\param EventSetup 		Setup event method for JPEC codec when encode is finish.
\par Note:
	1. tNotifyMode  : CPU polling or intterupt mode.
	2. tEM			: Semaphore or queue.
	3. pvEvent		: Semaphore or Queue handler. 
\return(no)
*/
void JPEG_Encode_Setup(JPEG_ENC_INFO_t jpg_info, JPEG_CODEC_FN_ES_t EventSetup);
//------------------------------------------------------------------------
/*!
\brief Setup decode parameter of JPEG Codec
\param jpg_info 		Parameter of JPEG decode.
\param EventSetup 		Setup event method for JPEC codec when decode is finish.
\par Note:
	1. tNotifyMode  : CPU polling or intterupt mode.
	2. tEM			: Semaphore or queue.
	3. pvEvent		: Semaphore or Queue handler. 
\return(no)
*/
void JPEG_Decode_Setup(JPEG_DEC_INFO_t jpg_info, JPEG_CODEC_FN_ES_t EventSetup);
//------------------------------------------------------------------------
/*!
\brief Setup JPEG and image start address
\param ulVDO_Start_addr 	Start address of Image.
\param ulJPG_Start_addr 	Start address of JPEG.
\return(no)
*/
void JPEG_Set_Start_Address(uint32_t ulVDO_Start_addr, uint32_t ulJPG_Start_addr);
//------------------------------------------------------------------------
/*!
\brief Enable function of JPEG Codec
\return JPEG_RESULT		Result:JPEG_PASS/JPEG_FAIL
*/
JPEG_RESULT JPEG_Codec_Enable(void);
//------------------------------------------------------------------------
/*!
\brief Disable function of JPEG Codec
\return(no)
*/
void JPEG_Codec_Disable(void);
//------------------------------------------------------------------------
/*!
\brief Obtain the bitstream size when JPEG encode is finish.
\return bs(bitstream size)
*/
uint32_t ulJPEG_Get_BS_Size(void);
//------------------------------------------------------------------------
/*!
\brief 	Get JPEG Version
\return	Version
*/
uint16_t uwJPEG_GetVersion(void);
#endif
