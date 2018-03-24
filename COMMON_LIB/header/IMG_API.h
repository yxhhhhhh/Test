/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMG_API.h
	\brief		Image Control header file
	\author		Bruce Hsu	
	\version	0.5
	\date		2017/12/19
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef __IMG_API__
#define __IMG_API__

#include "_510PF.h"
#include "H264_API.h"
/*!	\file IMG_API.h
	H264 Encoder /JPEG Encoder /ScalingDown FlowChart:
	\dot
	digraph IMG_Process_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		IMG_Init ->
		subgraph cluster_img{
			  label="IMG_Function_Setup(option)";
			  bgcolor="mintcream";
			  {"JPEGSetup" "ScalingDownSetup" "CroppingSetup" "MergeSetup" };
		}
		node [shape=record];
		JPEGSetup->H264_GetEncBufferSize;
		ScalingDownSetup->H264_GetEncBufferSize;
		CroppingSetup->H264_GetEncBufferSize;
		MergeSetup->H264_GetEncBufferSize;
		H264_GetEncBufferSize->
		H264_EncodeInit->
		H264_SetQP->
		H264_SetGOP->
		subgraph cluster_h264_enc{
			  label="H264_Encode_Setup(option)";
			  bgcolor="mintcream";
			  {"RateControl" "MROI" "AROI" "CondensedMode"};
		}
		RateControl->IMG_StartUp;
		MROI->IMG_StartUp;
		AROI->IMG_StartUp;
		CondensedMode->IMG_StartUp;
		IMG_StartUp->IMG_WaitResult;
	}
	\enddot
	
	H264 Decode FlowChart:
	\dot
	digraph H264_Decode_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		IMG_Init->
		subgraph cluster_dec{
				label="H264_Decode_Setup(option)";
				bgcolor="mintcream";
			  {"SetMirrorEn" "SetFlipEn" "SetRotationEn"};
		}
		SetMirrorEn->IMG_StartUp;
		SetFlipEn->IMG_StartUp;
		SetRotationEn->IMG_StartUp;
		IMG_StartUp->IMG_WaitResult;
	}
	\enddot
	
	Date Stamp FlowChart:
	\dot
	digraph DateStamp_flow{
	node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
	IMG_DSGetBufferSize->
	IMG_DSInit->IMG_DSLoadImageFromSF;
	IMG_DSInit->IMG_DSFontPrint;
	IMG_DSLoadImageFromSF->IMG_DSLoadImageFromSF[fontcolor=blue, fontsize=8, label="Queue"];
	IMG_DSFontPrint->IMG_DSFontPrint[fontcolor=blue, fontsize=8, label="Queue"];
	IMG_DSLoadImageFromSF->IMG_DSFontPrint[fontcolor=blue, fontsize=8, label="Queue"];
	IMG_DSFontPrint->IMG_DSLoadImageFromSF[fontcolor=blue, fontsize=8, label="Queue"];
	IMG_DSLoadImageFromSF->IMG_DSUpdate[fontcolor=blue, fontsize=8, label="Update"];
	IMG_DSFontPrint->IMG_DSUpdate[fontcolor=blue, fontsize=8, label="Update"];
	IMG_DSLoadImageFromSF->IMG_DSEraserBuf(AutoUpdate);
	IMG_DSFontPrint->IMG_DSEraserBuf(AutoUpdate);
	}
	\enddot
*/
//------------------------------------------------------------------------
#define IMG_DS_MENU0_ADDR		(0x1000)	//!< OSD menu SF address
#define IMG_DS_IMG10_ADDR		(0x10000)	//!< OSD image 1_0 SF address
#define IMG_DS_FONT_ADDR		(0x23230)	//!< OSD font SF address
#define IMG_DS_PAT_LEN			(0x100)		//!< OSD HW palette length
#define IMG_DS_FONT_PAT_LEN	(8)				//!< OSD font palette length
#define IMG_DS_IMG1_PAT_SFT	(1)				//!< OSD image 1 palette index shift
#define IMG_DS_IMG2_PAT_SFT	(0xA0)		//!< OSD image 2 palette index shift
#define IMG_DS_FONT_PAT_SFT	(0xF8)		//!< OSD font palette index shift
#define IMG_DS_FONT_SIZE		(16)			//!< OSD font size nxn
//------------------------------------------------------------------------
typedef enum
{
	IMG_SUCESS,
	IMG_FAIL
}IMG_BOOL_RESULT;

typedef enum
{
	IMG_DISABLE,
	IMG_ENABLE
	
}IMG_ENABLE_STATUS;
/*!
\brief Total image function structure 
*/
typedef struct 
{
	struct H264_TASK *H264_Task;	//!<  H264 task information
	uint32_t InputSrcAddr;				//!<		(must be mutiple of 8 byte)
	IMG_ENABLE_STATUS JPEGEnable;			//!<  JPEG ENABLE/DISABLE
	IMG_ENABLE_STATUS ScalingEnable;	//!<  Scalling Down ENABLE/DISABLE
}IMG_IMAGE_TASK;

/*!
\brief Scaling down ratio
*/
typedef enum
{
	SCALING_DOWN_1,			//!< 1/1 scaling down 		
	SCALING_DOWN_2,			//!< 1/2 scaling down 
	SCALING_DOWN_4			//!< 1/4 scaling down 
}IMG_SCALING_DOWN_RATIO;
/*!
\brief Mrege type
*/
typedef enum
{
	ONLY_1_IMAGE,			//!< Only 1 image 				
	COMBINE_2_IMAGE,	//!< Combine 2 image (The max size 1920/2 = 960) 	
	COMBINE_3_IMAGE		//!< Combine 2 image (The max size 1920/3 = 640)
	
}IMG_MERGE_TYPE;
/*!
\brief Merge location selection
*/
typedef enum
{
	ISP_IMAGE_IN_1_LOCATION,	//!< ISP Image is in 1 location.IMG_1 is in 2 Location. IMG_2 is in 3 Location. 
	ISP_IMAGE_IN_2_LOCATION,	//!< ISP Image is in 2 location.IMG_1 is in 1 Location. IMG_2 is in 3 Location. 
	ISP_IMAGE_IN_3_LOCATION		//!< ISP Image is in 3 location.IMG_1 is in 1 Location. IMG_2 is in 3 Location. 
	
}IMG_MERGE_LOCATION;

/*!
\brief JPEG ring buffer setup
*/
typedef struct
{
	uint32_t ulJpeg_Buf_Start;			//!< JPEG ring buffer Start address(must be mutiple of 8 byte)
	uint32_t ulJpeg_Buf_End;				//!< JPEG ring buffer End address(must be mutiple of 8 byte)
}IMG_JPEG_FIFO_Addr_t;

/*!
\brief Date stamp weighting value 
*/
typedef enum
{
	DS_WT_0DIV8,												//!< Date stamp weighting = 0/8
	DS_WT_1DIV8,												//!< Date stamp weighting = 1/8
	DS_WT_2DIV8,												//!< Date stamp weighting = 2/8
	DS_WT_3DIV8,												//!< Date stamp weighting = 3/8
	DS_WT_4DIV8,												//!< Date stamp weighting = 4/8
	DS_WT_5DIV8,												//!< Date stamp weighting = 5/8
	DS_WT_6DIV8,												//!< Date stamp weighting = 6/8
	DS_WT_7DIV8,												//!< Date stamp weighting = 7/8
	DS_WT_8DIV8													//!< Date stamp weighting = 8/8
}IMG_DS_WEIGHT;								//!< OUT = (DS_WEIGHT*TEXT + (8-DS_WEIGHT)*IMG)/8

/*!
\brief Date stamp scaling up value 
*/
typedef enum
{
	SINGLE,											//!< Do not scaling up
	DOUBLE,											//!< Scaling up double
	QUADRUPLE										//!< Scaling up quadruple
	
}IMG_DS_SCALE_UP_SIZE;

/*!
\brief Select date stamp index 
*/
typedef enum
{
	DS_1,												//!< Index of date stamp 1
	DS_2												//!< Index of date stamp 1
}IMG_DS_NUM;
/*!
\brief Merge function setup 
*/
typedef struct 
{
	IMG_ENABLE_STATUS STATUS;			//!< ENABLE/DISABLE
	IMG_MERGE_TYPE TYPE;							//!< Merge type
	IMG_MERGE_LOCATION LOCATION;			//!< Merge ISP image location
	uint32_t Width;
	uint32_t Height;
	uint32_t IMG_1_Addr;					//!< Second image star address  (if MERGE_TYPE == ONLY_1_IMAGE) can be NULL (must be mutiple of 8 byte)
	uint32_t IMG_2_Addr;					//!< Third image star address  	(if MERGE_TYPE == ONLY_1_IMAGE || COMBINE_2_IMAGE) can be NULL (must be mutiple of 8 byte)
}IMG_MERGE_SETUP;

typedef struct
{
	
	IMG_ENABLE_STATUS STATUS;			//!< ENABLE/DISABLE
	uint32_t Width;
	uint32_t Height;
	uint32_t StartX;
	uint32_t StartY;
	
}IMG_DIS_SETUP;
//------------------------------------------------------------------------------
/**
* @brief OSD Font Color Index
*/
typedef enum
{
	IMG_DS_FONT_BLACK = IMG_DS_FONT_PAT_SFT,		//!< OSD font black color index
	IMG_DS_FONT_WHITE,						//!< DS font white color index
	IMG_DS_FONT_RED,							//!< DS font red color index
	IMG_DS_FONT_GREEN,						//!< DS font green color index
	IMG_DS_FONT_BLUE,							//!< DS font blue color index
	IMG_DS_FONT_YELLOW,						//!< DS font yellow color index
	IMG_DS_FONT_CYAN,							//!< DS font cyan color index
	IMG_DS_FONT_GRAY,							//!< DS font gray color index
}IMG_DS_FONT_COLOR_TYP;

typedef enum
{
	IMG_DS_QUEUE,									//!< DS display queue type
	IMG_DS_UPDATE									//!< DS display update type
}IMG_DS_UPDATE_TYP;

typedef enum
{
	IMG_DS_BUF_EMPTY,							//!< DS buffer empty
	IMG_DS_BUF_CUR,								//!< DS buffer used with LCD controller
	IMG_DS_BUF_READY
}IMG_DS_BUF_STA_TYP;

typedef struct
{
	uint32_t 		ulBufAddr;				//!< DS buffer address
	IMG_DS_BUF_STA_TYP tBufState;	//!< DS buffer state	
}IMG_DS_BUF_TYP;

typedef struct
{
	uint16_t uwWidht;							//!< DS width
	uint16_t uwHeight;						//!< DS height
	IMG_DS_BUF_TYP Buffer[2];			//!< DS ping pong buffer
}IMG_DS_INFO;

/*!
\brief Scaling down function setup 
*/
typedef struct 
{
	uint32_t ulWidth;							//!(must be mutiple of 16 byte)
	uint32_t ulHeight;						//!(must be mutiple of 8 byte)
	uint32_t ulDesAddr;						//!< Destination Address (must be mutiple of 4 byte)
	IMG_SCALING_DOWN_RATIO RATIO;	//!< Scaling down ratio
}IMG_SCALLING_DOWN_SETUP;
/*!
\brief JPEG function setup 
*/
typedef struct 
{
	uint32_t ulWidth;							//!(must be mutiple of 16 byte)
	uint32_t ulHeight;						//!(must be mutiple of 8 byte)
	uint32_t ulDesAddr;						//!< Destination Address (must be mutiple of 8 byte)
	uint32_t QP;									//!< QP for JPEG
	uint8_t* Q_Table;							//!< Q Table for JPEG
	IMG_JPEG_FIFO_Addr_t JPEG_RING_BUFFER_ADDR; 		//!< JPEG ring buffer setting
}IMG_JPEG_SETUP;

typedef void (*pvIMG_Codec)(struct IMG_RESULT);

//------------------------------------------------------------------------
/*!
\brief Initial H264&IMG ISR
\return(no)
\par [Example]
\code 
		IMG_Init();
\endcode
*/
void IMG_Init(void);

//------------------------------------------------------------------------
/*!
\brief Setup JPEG IP Parameter
\param JPEGSetup	Structure of JPEGSetup   
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
	JPEGSetup Jpegsetup;
	IMG_BOOL_RESULT = RESULT;
	JPEGSetup.ulWidth = Width;
	JPEGSetup.ulHeight = Height;
	JPEGSetup.QP = 32;
	JPEGSetup.Q_Table = QTable;
	JPEGSetup.ulDesAddr = 0x500000;
	JPEGSetup.JPEG_RING_BUFFER_ADDR.ulJpeg_Buf_Start= 0x100000;
	JPEGSetup.JPEG_RING_BUFFER_ADDR.ulJpeg_Buf_End= 0x200000;
	RESULT = IMG_JPEGSetup(&JPEGSetup);
\endcode	
*/
IMG_BOOL_RESULT IMG_JPEGSetup(IMG_JPEG_SETUP *JpegSetup);
//------------------------------------------------------------------------
/*!
\brief Setup ScalingDown IP Parameter
\param ScalingSetup	Structure of ScalingSetup   
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
	IMG_SCALLING_DOWN_SETUP ScalingSetup;
	IMG_BOOL_RESULT = RESULT;
	ScalingSetup.ulWidth = 160;
	ScalingSetup.ulHeight = 128;
	ScalingSetup.RATIO = SCALING_DOWN_1;
	ScalingSetup.ulDesAddr = 0x400000;
	RESULT = IMG_SCALLING_DOWN_SETUP(ScalingSetup);
\endcode
*/
IMG_BOOL_RESULT IMG_ScalingDownSetup(IMG_SCALLING_DOWN_SETUP *ScalingSetup);
//------------------------------------------------------------------------
/*!
\brief Setup Croping IP Parameter
\param STATUS ENABLE/DISABLE
\param ulHStartPixel Horizontal start pixel  (must be mutiple of 2 pixel)
\param ulVStartPixel Vertical start pixel		 (must be mutiple of 2 pixel)	
\param ubHSize 			 Horizontal size				 (must be mutiple of 16 pixel)	
\param ubVSize       Vertical size					 (must be mutiple of 8 pixel)	
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
	IMG_BOOL_RESULT = RESULT;
	RESULT = CropingSetup(IMG_ENABLE,0,0,80,128);
\endcode
*/
IMG_BOOL_RESULT IMG_CroppingSetup(IMG_ENABLE_STATUS STATUS,uint32_t ulHStartPixel,uint32_t ulVStartPixel,uint8_t ubHSize,uint8_t ubVSize);
//------------------------------------------------------------------------
/*!
\brief Setup Merge IP Parameter
\param ScalingSetup	Structure of Merge   
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
	struct MergeSetup MergeSetup;
	IMG_BOOL_RESULT = RESULT;
	MergeSetup.STATUS = ENABLE;
	MergeSetup.TYPE = COMBINE_3_IMAGE;
	MergeSetup.LOCATION = ISP_IMAGE_IN_1_LOCATION;
	MergeSetup.IMG_1_Addr = 0x250000;
	MergeSetup.IMG_2_Addr = 0x300000;
	RESULT = MergeSetup(MergeSetup);
\endcode
*/
IMG_BOOL_RESULT IMG_MergeSetup(IMG_MERGE_SETUP *MergeSetup);
//------------------------------------------------------------------------
/*!
\brief Start Encode/Decode/JPEG/Scalling Down
\param xQueueTask			Encode Task Queue Handle
\param Task						Image_Task Structure
\return(no)
\code 
		IMG_IMAGE_TASK ImageTask;
		struct H264_TASK H264Task;
		H264Task.SrcAddr = 0x200000;
		H264Task.DesAddr = 0x400000;
		H264Task.Stream = ENCODE_0;
		H264Task.Type = H264_ENCODE;	
		ImageTask.H264_Task = &H264Task;
		ImageTask.JPEGEnable = ENABLE;
		ImageTask.ScalingEnable = ENABLE;
		IMG_StartUp(ImageTask);
\endcode
*/
void IMG_StartUp(IMG_IMAGE_TASK Task);
//------------------------------------------------------------------------
/*!
\brief Get Adequate Buffer Size 
\param uwDS_Width			DS width
\param uwDS_Height		DS Height			
\return BufferSize
\par [Example]
\code 
		uint32_t Size;
		Size = IMG_DSGetBufferSize(320,240);
\endcode
*/
uint32_t IMG_DSGetBufferSize(uint16_t uwDS_Width,uint16_t uwDS_Height);
//------------------------------------------------------------------------
/*!
\brief Initial Date Stamp 
\param DS_NUM					Date stamp index
\param DS_WEIGHTING   Date stamp weight 
\param SCALE_UP_SIZE	Date stamp scale up size
\param uwIMGWidth 		Input image width
\param uwIMGHeight		Input image height
\param uwDS_HStart		Date stamp panel horizontal start point on image
\param uwDS_VStart 		Date stamp panel vertical start point on image
\param uwDS_HSize			Date stamp panel horizontal size
\param uwDS_VSize			Date stamp panel vertical size
\param ulBufAddr 			Allocate buffer start address
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
		IMG_BOOL_RESULT = RESULT;
		RESULT = IMG_DSInit(DS_1, DS_WT_7DIV8, SINGLE, 1280, 720, 0, 0, 640, 480,0x2200000);
\endcode
*/
IMG_BOOL_RESULT IMG_DSInit(IMG_DS_NUM DS_NUM,IMG_DS_WEIGHT DS_WEIGHTING,IMG_DS_SCALE_UP_SIZE SCALE_UP_SIZE,uint16_t uwIMGWidth,uint16_t uwIMGHeight,uint16_t uwDS_HStart,uint16_t uwDS_VStart, uint16_t uwDS_HSize,uint16_t uwDS_VSize,uint32_t ulBufAddr);
//------------------------------------------------------------------------
/*!
\brief Display Image From Serial Flash 
\param DS_NUM					Date stamp index
\param ulSFAddr				Serial flash address of image
\param uwHStart				Horizontal start point on date stamp panel
\param uwVStart				Vertical start point on date stamp panel
\param uwHSize				Horizontal size of loading image
\param uwVSize				Vertical size of loading image
\param UpdateType			Update immediately or later
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code
		IMG_BOOL_RESULT = RESULT;
		RESULT = IMG_DSLoadImageFromSF(DS_1,0x10000,0,0,320,240,IMG_DS_UPDATE);
\endcode
*/
IMG_BOOL_RESULT IMG_DSLoadImageFromSF(IMG_DS_NUM DS_NUM, uint32_t ulSFAddr, uint16_t uwHStart, uint16_t uwVStart,uint16_t uwHSize, uint16_t uwVSize, IMG_DS_UPDATE_TYP UpdateType);
//------------------------------------------------------------------------
/*!
\brief Display Font Date Stamp Function 
\param DS_NUM					Date stamp index
\param ulSFAddr				Serial flash address of font
\param tFontColor			Font color
\param uwHStart				Horizontal start point on date stamp panel
\param uwVStart				Vertical start point on date stamp panel
\param UpdateType 		Update immediately or later
\param UpdateType			Display font string	
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
		IMG_BOOL_RESULT = RESULT;
		RESULT = IMG_DSFontPrint(DS_1, 0x23230, IMG_DS_FONT_BLACK, 320, 240, IMG_DS_UPDATE, "%s\n%d\nFontSample\n", "SONiX", 5471);
\endcode
*/
//------------------------------------------------------------------------
IMG_BOOL_RESULT IMG_DSFontPrint(IMG_DS_NUM DS_NUM,uint32_t ulAddr, IMG_DS_FONT_COLOR_TYP tFontColor, uint16_t uwHStart, uint16_t uwVStart, IMG_DS_UPDATE_TYP UpdateType, char* pFmt, ...);
/*!
\brief Erase Date Stamp Function 
\param DS_NUM					Date stamp index
\param uwHStart				Horizontal start point on date stamp panel
\param uwVStart				Vertical start point on date stamp panel
\param uwHSize				Horizontal size 
\param uwVSize				Vertical size 
\par [Example]
\code 
		IMG_DSEraserBuf(DS_1, 320, 240, 320, 240);
\endcode
*/
//------------------------------------------------------------------------
void IMG_DSEraserBuf(IMG_DS_NUM DS_NUM,uint16_t uwHStart, uint16_t uwVStart,uint16_t uwHSize, uint16_t uwVSize);
//------------------------------------------------------------------------
/*!
\brief Upate Date Stamp Queue Buffer 
\param DS_NUM					Date stamp index
\par [Example]
\code 
		IMG_DSLoadImageFromSF(DS_1,0x10000,0,0,320,240,IMG_DS_QUEUE);
		IMG_DSLoadImageFromSF(DS_1,0x10000,320,240,320,240,IMG_DS_QUEUE);
		IMG_DSUpdate(DS_1);
		IMG_DSFontPrint(DS_1, 0x23230, IMG_DS_FONT_BLACK, 320, 240, IMG_DS_QUEUE, "%s\n%d\nFontSample\n", "SONiX", 5471);
		IMG_DSUpdate(DS_1);
\endcode
*/
void IMG_DSUpdate(IMG_DS_NUM DS_NUM);
//------------------------------------------------------------------------
/*!
\brief Disable Date Stamp Function 
\param DS_NUM					Date stamp index
\par [Example]
\code 
		IMG_DSDisable(DS_1);
\endcode
*/
void IMG_DSDisable(IMG_DS_NUM DS_NUM);
//------------------------------------------------------------------------
/*!
\brief Disable Date Stamp Function 
\param ulWidth					Width of source image
\param ulHeight					Height of source image
\param ubMergeNum				Number of merge image
\param pInputAddrArray 	Array of pointer of input image address;
\param DesAddr					Destination address
\return IMG_SUCESS/IMG_FAIL
\par [Example]
\code 
		IMG_BOOL_RESULT = RESULT;
		RESULT = IMG_VerticalMerge(320,240,3,0x150000,0x300000);
\endcode
*/
//------------------------------------------------------------------------
IMG_BOOL_RESULT IMG_VerticalMerge(uint32_t ulWidth,uint32_t ulHeight,uint8_t ubMergeNum ,uint8_t** pInputAddrArray,uint8_t *DesAddr);
//------------------------------------------------------------------------
/*!
\brief Get Adequate Buffer Size Of Vertical Merge 
\param ulWidth					Width of source image
\param ulHeight					Height of source image
\param ubMergeNum				Number of merge image
\return BufferSize
\par [Example]
\code 
		IMG_BOOL_RESULT = RESULT;
		RESULT = IMG_GetVerticalMergeBufferSize(320,240,3);
\endcode
*/
uint32_t IMG_GetVerticalMergeBufferSize(uint32_t ulWidth,uint32_t ulHeight,uint8_t ubMergeNum);
//------------------------------------------------------------------------
/*!
\brief Set RAW data for H264
\param EncodeIndex				H264 Encode information
\return no
*/
void IMG_SetH264RawImage(H264_ENCODE_INDEX EncodeIndex);
//------------------------------------------------------------------------
/*!
\brief H264 Codec Finish
\param pvCB						Callback function when Image codec is finish
\return no
*/
void IMG_SetCodecFinishCbFunc(pvIMG_Codec pvCB);
//------------------------------------------------------------------------
/*!
\brief Clear Message Queue
\return no
*/
void IMG_ClearImgMsgQueue(void);
//------------------------------------------------------------------------
/*!
\brief 	Get Image control Version	
\return	Version
*/
uint16_t uwIMG_GetVersion(void);
//------------------------------------------------------------------------
extern osMessageQId IMG_EventQueue;

#endif
