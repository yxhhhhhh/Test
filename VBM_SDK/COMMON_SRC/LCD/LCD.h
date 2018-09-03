/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD.h
	\brief		LCD Function Header
	\author		Pierce
	\version	1.3
	\date		2018/07/25
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_H_
#define _LCD_H_
#include <stdint.h>
#include <stdbool.h>
#include "LCD_TYPE.h"
#include "INTC.h"
/*!	\file LCD.h
LCD Display FlowChart:
	\dot
digraph LCD_Flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		edge [fontname=Verdana];
		"No Video" [shape=polygon, sides=9,color=red, fontcolor=red];
		"Window No Shift" [shape=polygon, sides=9, color=red, fontcolor=red];
		LCD_Init ->	tLCD_CropScale;
		tLCD_CropScale -> "No Video" [color=red, fontcolor=red, fontsize=8, tailport=w,label="Fail", style = dashed]; 
		tLCD_CropScale -> tLCD_JpegDecodeEnable [fontcolor=blue, fontsize=8, label="Jpeg 422"]; 
		tLCD_JpegDecodeEnable -> "No Video" [color=red, fontcolor=red, fontsize=8, tailport=w, label="Fail", style = dashed];
		tLCD_JpegDecodeEnable -> LCD_JpegRwQTab -> ulLCD_CalLcdBufSize [fontcolor=blue, fontsize=8, label="Jpeg 422"];
		tLCD_CropScale -> ulLCD_CalLcdBufSize [fontcolor=blue, fontsize=9, headport=ne, label="Raw"];
		ulLCD_CalLcdBufSize -> LCD_SetLcdBufAddr [fontcolor=blue, fontsize=12, label="Assign LCD Buffer"];
		LCD_SetLcdBufAddr -> pLCD_GetLcdChBufInfor;
		pLCD_GetLcdChBufInfor -> LCD_SetChBufReady [fontcolor=blue, fontsize=8, label="CHx"];
		LCD_SetChBufReady -> LCD_ChEnable [fontcolor=blue, fontsize=8, label="CHx"];					
		LCD_ChEnable -> pLCD_GetLcdChBufInfor [fontcolor=blue, fontsize=8, headport=sw, label="CHy"];
		LCD_ChEnable -> LCD_Start;
		LCD_ChEnable -> tLCD_WinSft;
		tLCD_WinSft -> "Window No Shift"  [color=red, fontcolor=red, fontsize=8, tailport=sw, label="Fail", style = dashed]; 
		tLCD_WinSft  -> LCD_Start [headport=nw];
		LCD_Start -> LCD_Stop [fontcolor=blue, fontsize=8, label="Change resolution or display type"];
		LCD_Stop -> tLCD_CropScale [headport=se];
		LCD_Stop -> LCD_UnInit [fontcolor=blue, fontsize=8, label="Change LCD type"];
		LCD_Start -> LCD_UnInit [fontcolor=blue, fontsize=8, headport=w, tailport=sw,label="Change LCD type"];
		LCD_Init ->LCD_UnInit[dir=back, headport=ne, tailport=se];
	}
	\enddot
*/
//------------------------------------------------------------------------------
#define LCD_MAX_CH					(4)		//!< MAX Channels of LCD HW
#define LCD_PINGPONG_BUF_MAX		(2)		//!< Max PINGPONG Buffer

#define LCD_CH_NOIMG_MAX_BOUNDARY	(0xFF)	//!< Max boundary of LCD channel no detect signal

//------------------------------------------------------------------------------
#define LCD_HVIEW_CAMF_HSIZE		(160)	//!< LCD H-Type view Cam F(Ch0) Hsize
//------------------------------------------------------------------------------
#define LCDMAX(a,b)					((a)>(b))?a:b
#define LCDMIN(a,b)					((a)<(b))?a:b
//------------------------------------------------------------------------------
#define LCD_MODE_QUEUE_SZ			(1)
//------------------------------------------------------------------------------

#ifdef _510PF_SDK_

#define LCD_WaitMutex                        \
if(osLCD_Mutex != NULL)                        \
    osMutexWait(osLCD_Mutex, osWaitForever);

#define LCD_ReleaseMutex                     \
if(osLCD_Mutex != NULL)                        \
    osMutexRelease(osLCD_Mutex);

#else

#define LCD_WaitMutex
#define LCD_ReleaseMutex

#endif

#define LCD_FuncEnable				(GLB->LCD_FUNC_DIS = 0)
#define LCD_FuncDisable				(GLB->LCD_FUNC_DIS = 1)

enum
{
	LCD_GPIO,
	LCD_AU_UPS051_8,
	LCD_AU_UPS051_6,
	LCD_AT3210,
	LCD_ST7528,
	LCD_8080_8_2BYTE,
	LCD_IST3101,
	LCD_RGB_DUMMY,
	LCD_DE,
	LCD_8080_16,
	LCD_AU,
	LCD_8080_8_3BYTE,
	LCD_YUV422,
	LCD_BT656_BT601,
	LCD_8080_9,
	TV=15
};
enum
{
	NTSC,
	NTSC_443,
	PAL,
	PAL_M
};
enum
{
	INTERLACE,
	PROGRESSIVE,
};
typedef struct{
	uint16_t uwGamma;
	int16_t  swBrit;
	uint16_t uwCntrs;
	uint16_t uwGain;
}LCD_GAMMA_SET_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Controller Output Type
*/
typedef enum
{
	LCD_UNINIT,								//!< UnInitial
	LCD_LCD_PANEL,							//!< Display on LCD
	LCD_TV,									//!< TV output
	LCD_HDMI								//!< HDMI output
}LCD_OUTPUT_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Display Type
*/
typedef enum
{
	LCD_DISP_4T,							//!< Display four channels
	LCD_DISP_3TC,							//!< Display three channels, column list
	LCD_DISP_3T_1U2D,						//!< Display three channels, one top and two bottom
	LCD_DISP_3T_2U1D,						//!< Display three channels, two top and one bottom
	LCD_DISP_3T_2L1R,						//!< Display three channels, two left and one right
	LCD_DISP_3T_1L2R,						//!< Display three channels, two left and one right
	LCD_DISP_3T_2PIP,						//!< Display three channels, two PIPs, inset picture at middle
	LCD_DISP_2T_H,							//!< Display two channels, one left and one right
	LCD_DISP_2T_H13,						//!< Display two channels, one left and one right, right is triple left size
	LCD_DISP_2T_H12,						//!< Display two channels, one left and one right, right is dual left size
	LCD_DISP_2T_V,							//!< Display two channels, one top and one bottom
	LCD_DISP_2T_PIP_0,						//!< Display PIP, inset picture at upper left
	LCD_DISP_2T_PIP_1,						//!< Display PIP, inset picture at upper right
	LCD_DISP_2T_PIP_2,						//!< Display PIP, inset picture at lower left
	LCD_DISP_2T_PIP_3,						//!< Display PIP, inset picture at lower right
	LCD_DISP_1T								//!< Display one channel
}LCD_DISP_TYPE;
//------------------------------------------------------------------------------
/*!
	\brief LCD Function Return Result Type
*/
typedef enum
{
	LCD_OK,									//!< LCD setting ok
	LCD_DISP_FAIL,							//!< LCD display type fail
	LCD_CROP_FAIL,							//!< LCD channels cropping fail
	LCD_SCALE_FAIL,							//!< LCD channels scaler fail
	LCD_BUF_FAIL,							//!< LCD buffer address fail because not four alignment
	LCD_WINXY_FAIL,							//!< LCD channel XY shift Fail
	LCD_WINX_FAIL,							//!< LCD channel X shift Fail
	LCD_WINY_FAIL,							//!< LCD channel Y shift Fail
	LCD_OSD_FONT_FAIL,						//!< LCD OSD string special font not support
	LCD_OSD_OF_FAIL,						//!< LCD OSD buffer overflow
	LCD_OSD_SCALE_FAIL,						//!< LCD OSD scaler fail
	LCD_OSD_HSIZE_FAIL, 					//!< LCD OSD horizontal size fail because not quadruple
	LCD_OSD_LOCA_FAIL,						//!< LCD OSD loaction fail because out of LCD display range
	LCD_JPEG_DECEN_FAIL,					//!< LCD Jpeg decode enable fail
	LCD_JPEG_DECDIS_FAIL = 0,				//!< LCD Jpeg decode disable fail
	LCD_JPEG_DECDIS_OK = 1,					//!< LCD Jpeg decode disable ok
}LCD_RESULT;
//------------------------------------------------------------------------------
/*!
	\brief LCD Channels
*/
typedef enum
{
	LCD_CH0,								//!< LCD channel0
	LCD_CH1,								//!< LCD channel1
	LCD_CH2,								//!< LCD channel2
	LCD_CH3,								//!< LCD channel3
}LCD_CH_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Back Light Level
*/
typedef enum
{
	LCD_BL_OFF,								//!< LCD back light off
	LCD_BL_LV1,								//!< LCD back light level 1
	LCD_BL_LV2,								//!< LCD back light level 2
	LCD_BL_LV3,								//!< LCD back light level 3
	LCD_BL_LV4,								//!< LCD back light level 4
	LCD_BL_LV5,								//!< LCD back light level 5
	LCD_BL_LV6,								//!< LCD back light level 6
	LCD_BL_LV7,								//!< LCD back light level 7
	LCD_BL_LV8,								//!< LCD back light level 8
}LCD_BL_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Buffer States
*/
typedef enum
{
	LCD_BUF_EMPTY,							//!< LCD buffer empty
	LCD_BUF_CUR,							//!< LCD buffer used with LCD controller
	LCD_BUF_FULL,							//!< LCD buffer full	
}LCD_BUF_STA_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Jpeg QTable Type (R/W)
*/
typedef enum
{
	LCD_JPEG_QW,							//!< Write LCD Jpeg QTable
	LCD_JPEG_QR								//!< Read  LCD Jpeg QTable
}LCD_JPEG_QRW_TYP;

typedef enum
{
	LCD_JPEG_DISABLE,
	LCD_JPEG_ENABLE,
}LCD_JPEG_STATUS;
//------------------------------------------------------------------------------
/*!
	\brief LCD Buffer Alignment
*/
typedef enum
{
	LCD_BUF_NO_ALIG = 0,					//!< LCD Buffer no Alignment
	LCD_BUF_2BYTES_ALIG,					//!< LCD Buffer 2 Bytes Alignment
	LCD_BUF_4BYTES_ALIG,					//!< LCD Buffer 4 Bytes Alignment
	LCD_BUF_8BYTES_ALIG,					//!< LCD Buffer 8 Bytes Alignment
	LCD_BUF_16BYTES_ALIG,					//!< LCD Buffer 16 Bytes Alignment
	LCD_BUF_32BYTES_ALIG,					//!< LCD Buffer 32 Bytes Alignment
	LCD_BUF_64BYTES_ALIG,					//!< LCD Buffer 64 Bytes Alignment
	LCD_BUF_128BYTES_ALIG,					//!< LCD Buffer 128 Bytes Alignment
	LCD_BUF_256BYTES_ALIG,					//!< LCD Buffer 256 Bytes Alignment
	LCD_BUF_512BYTES_ALIG,					//!< LCD Buffer 512 Bytes Alignment
	LCD_BUF_1024BYTES_ALIG,					//!< LCD Buffer 1024 Bytes Alignment
}LCD_BUF_ALIG_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Channel Resolution Information
*/
typedef struct
{
	uint16_t uwChInputHsize;				//!< Channel input resolutions
	uint16_t uwChInputVsize;				//!< Channel input resolutions
	uint16_t uwCropHstart;					//!< Channel output start point
	uint16_t uwCropVstart;					//!< Channel output start point
	uint16_t uwCropHsize;					//!< Channel output resolutions
 	uint16_t uwCropVsize;					//!< Channel output resolutions
}LCD_CH_RES_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Display Information
*/
typedef struct
{
	LCD_DISP_TYPE tDispType;				//!< LCD display type
	uint8_t ubChNum;						//!< LCD display channel number
	LCD_CH_RES_TYP tChRes[LCD_MAX_CH];		//!< LCD channel information
	uint16_t uwLcdOutputHsize;				//!< LCD display resolution
	uint16_t uwLcdOutputVsize;				//!< LCD display resolution
}LCD_INFOR_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Dynamic Setting Information
*/
typedef struct
{	
	bool     bDyn;							//!< LCD dynamic display enable
	uint16_t uwHiStart;						//!< H input Start Setting
	uint16_t uwHiSize;						//!< H input Size Setting
	uint16_t uwHsize;						//!< H crop out size Setting
	uint16_t uwViStart;						//!< V input Start Setting
	uint16_t uwVsize;						//!< V crop out size Setting
	uint8_t  ubFsEN;						//!< Fine scaler enable
	uint16_t uwHratio;						//!< Fine scaler H ratio Setting
	uint16_t uwVratio;						//!< Fine scaler V ratio Setting
	uint8_t  ubFsHdupEN;					//!< H duplicate enable
	uint8_t  ubFsVdupEN;					//!< V duplicate enable
	uint16_t uwHoStart;						//!< Fine scaler crop H start Setting
	uint16_t uwVoStart;						//!< Fine scaler crop V start Setting
	uint16_t uwHoSize;						//!< Fine scaler crop H size Setting
	uint16_t uwVoSize;						//!< Fine scaler crop V size Setting
	uint8_t  ubFirEN;						//!< FIR filter enable	
}LCD_DYN_SETTING_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD One Channel Dynamic Display Information
*/
typedef struct
{	
	LCD_CH_RES_TYP tChRes;					//!< LCD channel information
	uint16_t uwLcdOutputHsize;				//!< LCD display resolution
	uint16_t uwLcdOutputVsize;				//!< LCD display resolution
}LCD_DYN_INFOR_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Buffer Information
*/
typedef struct
{
	uint32_t 		ulBufAddr;				//!< LCD buffer address
	LCD_BUF_STA_TYP tBufState;				//!< LCD buffer state	
}LCD_BUF_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Channel Buffer Information
*/
typedef struct
{
	LCD_BUF_TYP tBuf[LCD_PINGPONG_BUF_MAX];	//!< LCD channel buffer
}LCD_CH_BUF_TYP;
//------------------------------------------------------------------------------
/*!
	\brief LCD Total Buffer Information
*/
typedef struct
{
	LCD_CH_BUF_TYP  tChBuf[LCD_MAX_CH];		//!< LCD total channels buffer 
}LCD_TOL_BUF_TYP;
//------------------------------------------------------------------------------
typedef struct
{
	bool	 bJpegEn;						//!< LCD Jpeg decode for the channel
	uint16_t uwHSize;						//!< LCD horizontal size for the channel
	uint16_t uwVSize;						//!< LCD vertical size for the channel
}LCD_CALBUF_CH_TYP;
//------------------------------------------------------------------------------
typedef struct
{
	uint16_t uwLcdHSize;					//!< LCD output horizontal size
	uint16_t uwLcdVSize;					//!< LCD output vertical size
	uint8_t	 ubChMax;						//!< LCD output n channels
	LCD_CALBUF_CH_TYP  tInput[LCD_MAX_CH];	//!< LCD channels input information
	LCD_BUF_ALIG_TYP tAlign;				//!< LCD buffer alignment
}LCD_CALBUF_TYP;
//------------------------------------------------------------------------------
typedef enum 
{
    LCD_UNKNOW,								//!< LCD output unknow state
    LCD_HDMI_INIT,							//!< LCD output HDMI initial state
    LCD_HDMI_MODE,							//!< LCD output HDMI mode state
	LCD_TV_MODE,							//!< LCD output TV state
	LCD_PANEL_MODE							//!< LCD output panel state
}LCD_MODE_TYP;
typedef enum 
{
    LCD_NO_ROTATE,
    LCD_ROTATE_90,
}LCD_ROTATE_TYP;
//------------------------------------------------------------------------------
typedef struct
{
	uint8_t bCh0:1;
	uint8_t bCh1:1;
	uint8_t bCh2:1;
	uint8_t bCh3:1;
}LCD_CHANNEL_EN_t;
//------------------------------------------------------------------------------
/*!
	\brief 		LCD Controller Initial Function
	\param[in]	tLcdOutput	LCD Output Type
	\par [Example]
	\code  
		 //! Initial LCD
		 LCD_Init(LCD_LCD_PANEL);                  
	\endcode
	\par [Example]
	\code  
		 //! Initial TV
		 LCD_Init(LCD_TV);                  
	\endcode
	\par [Example]
	\code  
		 //! Initial HDMI
		 LCD_Init(LCD_HDMI);                  
	\endcode
*/
void LCD_Init (LCD_OUTPUT_TYP tLcdOutput);
//------------------------------------------------------------------------------
/*!
	\brief	LCD Controller Uninitial Function
	\par [Example]                                                                        
	\code                                                                         
		 LCD_UnInit();                  
	\endcode
*/
void LCD_UnInit (void);
//------------------------------------------------------------------------------
/*!
	\brief		LCD Cropping and Scaler Function
	\param[in]	*pLcdInfor	   LCD Information pointer
	\return		LCD_OK 		   LCD channels cropping and scaler ok!
	\return		LCD_CROP_FAIL  LCD channels cropping fail!
	\return		LCD_SCALE_FAIL LCD channels scaler fail!
	\par [Example]
	\code
		 //! LCD displays 1T
		 LCD_INFOR_TYP sLcdInfor;
		 
		 sLcdInfor.tDispType = LCD_DISP_1T;
		 sLcdInfor.ubChNum = 1;
		 sLcdInfor.uwLcdOutputHsize = 320;
		 sLcdInfor.uwLcdOutputVsize = 240;
		 sLcdInfor.tChRes[0].uwChInputHsize = 640;
		 sLcdInfor.tChRes[0].uwChInputVsize = 480;
		 sLcdInfor.tChRes[0].uwCropHstart = 0;
		 sLcdInfor.tChRes[0].uwCropVstart = 0;
		 sLcdInfor.tChRes[0].uwCropHsize = 640;
		 sLcdInfor.tChRes[0].uwCropVsize = 480; 
		 tLCD_CropScale(&sLcdInfor);
	\endcode
	\par [Example]
	\code
		 //! LCD displays 2T PIP (Tx0 is inset windows and Tx1 is full screen)
		 LCD_INFOR_TYP sLcdInfor;

		 sLcdInfor.tDispType = LCD_DISP_2T_PIP_0;
		 sLcdInfor.ubChNum = 2;
		 sLcdInfor.uwLcdOutputHsize = 320;
		 sLcdInfor.uwLcdOutputVsize = 240;
		 sLcdInfor.tChRes[0].uwChInputHsize = 640;
		 sLcdInfor.tChRes[0].uwChInputVsize = 480;
		 sLcdInfor.tChRes[0].uwCropHstart = 0;
		 sLcdInfor.tChRes[0].uwCropVstart = 0;
		 sLcdInfor.tChRes[0].uwCropHsize = 640;
		 sLcdInfor.tChRes[0].uwCropVsize = 480; 
		 sLcdInfor.tChRes[1].uwChInputHsize = 640;
		 sLcdInfor.tChRes[1].uwChInputVsize = 480;
		 sLcdInfor.tChRes[1].uwCropHstart = 0;
		 sLcdInfor.tChRes[1].uwCropVstart = 0;
		 sLcdInfor.tChRes[1].uwCropHsize = 640;
		 sLcdInfor.tChRes[1].uwCropVsize = 480; 
		 tLCD_CropScale(&sLcdInfor);
	\endcode
	\par [Example]
	\code
		 //! LCD displays 2T dual horizontal (Tx0 is left and Tx1 is right)
		 LCD_INFOR_TYP sLcdInfor;

		 sLcdInfor.tDispType = LCD_DISP_2T_H;
		 sLcdInfor.ubChNum = 2;
		 sLcdInfor.uwLcdOutputHsize = 320;
		 sLcdInfor.uwLcdOutputVsize = 240;
		 sLcdInfor.tChRes[0].uwChInputHsize = 640;
		 sLcdInfor.tChRes[0].uwChInputVsize = 480;
		 sLcdInfor.tChRes[0].uwCropHstart = 0;
		 sLcdInfor.tChRes[0].uwCropVstart = 0;
		 sLcdInfor.tChRes[0].uwCropHsize = 640;
		 sLcdInfor.tChRes[0].uwCropVsize = 480; 
		 sLcdInfor.tChRes[1].uwChInputHsize = 640;
		 sLcdInfor.tChRes[1].uwChInputVsize = 480;
		 sLcdInfor.tChRes[1].uwCropHstart = 0;
		 sLcdInfor.tChRes[1].uwCropVstart = 0;
		 sLcdInfor.tChRes[1].uwCropHsize = 640;
		 sLcdInfor.tChRes[1].uwCropVsize = 480; 
		 tLCD_CropScale(&sLcdInfor);
	\endcode
	\par [Example]
	\code
		 //! LCD displays 3T (Tx1 is left, Tx0 is upper right and Tx2 is lower right)
		 LCD_INFOR_TYP sLcdInfor;

		 sLcdInfor.tDispType = LCD_DISP_3T_1L2R;
		 sLcdInfor.ubChNum = 3;
		 sLcdInfor.uwLcdOutputHsize = 320;
		 sLcdInfor.uwLcdOutputVsize = 240;
		 sLcdInfor.tChRes[0].uwChInputHsize = 640;
		 sLcdInfor.tChRes[0].uwChInputVsize = 480;
		 sLcdInfor.tChRes[0].uwCropHstart = 0;
		 sLcdInfor.tChRes[0].uwCropVstart = 0;
		 sLcdInfor.tChRes[0].uwCropHsize = 640;
		 sLcdInfor.tChRes[0].uwCropVsize = 480; 
		 sLcdInfor.tChRes[1].uwChInputHsize = 640;
		 sLcdInfor.tChRes[1].uwChInputVsize = 480;
		 sLcdInfor.tChRes[1].uwCropHstart = 0;
		 sLcdInfor.tChRes[1].uwCropVstart = 0;
		 sLcdInfor.tChRes[1].uwCropHsize = 640;
		 sLcdInfor.tChRes[1].uwCropVsize = 480;
		 sLcdInfor.tChRes[2].uwChInputHsize = 640;
		 sLcdInfor.tChRes[2].uwChInputVsize = 480;
		 sLcdInfor.tChRes[2].uwCropHstart = 0;
		 sLcdInfor.tChRes[2].uwCropVstart = 0;
		 sLcdInfor.tChRes[2].uwCropHsize = 640;
		 sLcdInfor.tChRes[2].uwCropVsize = 480; 
		 tLCD_CropScale(&sLcdInfor);
	\endcode
	\par [Example]
	\code
		 //! LCD displays 4T Quad (Tx0 is upper left, Tx1 is upper right, Tx2 is lower left and Tx3 is lower right)
		 LCD_INFOR_TYP sLcdInfor;

		 sLcdInfor.tDispType = LCD_DISP_4T;
		 sLcdInfor.ubChNum = 4;
		 sLcdInfor.uwLcdOutputHsize = 320;
		 sLcdInfor.uwLcdOutputVsize = 240;
		 sLcdInfor.tChRes[0].uwChInputHsize = 640;
		 sLcdInfor.tChRes[0].uwChInputVsize = 480;
		 sLcdInfor.tChRes[0].uwCropHstart = 0;
		 sLcdInfor.tChRes[0].uwCropVstart = 0;
		 sLcdInfor.tChRes[0].uwCropHsize = 640;
		 sLcdInfor.tChRes[0].uwCropVsize = 480; 
		 sLcdInfor.tChRes[1].uwChInputHsize = 640;
		 sLcdInfor.tChRes[1].uwChInputVsize = 480;
		 sLcdInfor.tChRes[1].uwCropHstart = 0;
		 sLcdInfor.tChRes[1].uwCropVstart = 0;
		 sLcdInfor.tChRes[1].uwCropHsize = 640;
		 sLcdInfor.tChRes[1].uwCropVsize = 480; 
		 sLcdInfor.tChRes[2].uwChInputHsize = 640;
		 sLcdInfor.tChRes[2].uwChInputVsize = 480;
		 sLcdInfor.tChRes[2].uwCropHstart = 0;
		 sLcdInfor.tChRes[2].uwCropVstart = 0;
		 sLcdInfor.tChRes[2].uwCropHsize = 640;
		 sLcdInfor.tChRes[2].uwCropVsize = 480; 
		 sLcdInfor.tChRes[3].uwChInputHsize = 640;
		 sLcdInfor.tChRes[3].uwChInputVsize = 480;
		 sLcdInfor.tChRes[3].uwCropHstart = 0;
		 sLcdInfor.tChRes[3].uwCropVstart = 0;
		 sLcdInfor.tChRes[3].uwCropHsize = 640;
		 sLcdInfor.tChRes[3].uwCropVsize = 480; 
		 tLCD_CropScale(&sLcdInfor);
	\endcode
*/
LCD_RESULT tLCD_CropScale (LCD_INFOR_TYP *pLcdInfor);
//------------------------------------------------------------------------------
/*!
	\brief 		LCD Jpeg Decode Enable Function
	\param[in]	tCh Selet Nth Channel
	\return		LCD_OK LCD Jpeg decode enable ok!
	\return		LCD_JPEG_DECEN_FAIL LCD Jpeg decode enable fail because other channel uses Jpeg decode!
	\par [Example]
	\code    
		 tLCD_JpegDecodeEnable(LCD_CH0);
	\ndcode
*/
LCD_RESULT tLCD_JpegDecodeEnable (LCD_CH_TYP tCh);
//------------------------------------------------------------------------------
/*!
	\brief	LCD Jpeg Decode Disable Function
	\return	Disable Jpeg decode status
	\par [Example]
	\code    
		 tLCD_JpegDecodeDisable();
	\endcode
*/
LCD_RESULT tLCD_JpegDecodeDisable (void);
//------------------------------------------------------------------------------
/*!
	\brief	LCD Jpeg Decode Stop Function
	\return	Stop Jpeg decode
	\par [Example]
	\code    
		 LCD_JpegDecodeStop();
	\endcode
*/
void LCD_JpegDecodeStop (void);
//------------------------------------------------------------------------------
/*!
	\brief	Get status of LCD Jpeg Decoder.
	\return	Jpeg decoder status
	\par [Example]
	\code
		LCD_JPEG_STATUS tLCD_JpgSts;
		tLCD_JpgSts = tLCD_GetJpegDecoderStatus();
	\endcode
*/
LCD_JPEG_STATUS tLCD_GetJpegDecoderStatus(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Calculate LCD Total Buffer Size Function
	\param[in]	*pInfor	  LCD input information pointer
	\return		LCD total buffer size
	\par [Example]
	\code
		 LCD_CALBUF_TYP	sInfor;
		 uint32_t 		ulBufSz;

		 sInfor.ubChMax = 4;
		 sInfor.tInput[0].bJpegEn = false;
		 sInfor.tInput[0].uwHSize = 320;
		 sInfor.tInput[0].uwVSize = 240;
		 sInfor.tInput[1].bJpegEn = false;
		 sInfor.tInput[1].uwHSize = 640;
		 sInfor.tInput[1].uwVSize = 480;
		 sInfor.tInput[2].bJpegEn = false;
		 sInfor.tInput[2].uwHSize = 320;
		 sInfor.tInput[2].uwVSize = 240;
		 sInfor.tInput[3].bJpegEn = false;
		 sInfor.tInput[3].uwHSize = 640;
		 sInfor.tInput[3].uwVSize = 480;
		 sInfor.tAlign = LCD_BUF_4BYTES_ALIG;
		 ulBufSz = ulLCD_CalLcdBufSize(&sInfor);
	\endcode
*/
uint32_t ulLCD_CalLcdBufSize (LCD_CALBUF_TYP *pInfor);
//------------------------------------------------------------------------------
/*!
	\brief 		Set LCD Total Buffer Address Function
	\param[in]	ulAddr Total buffer address
	\note		LCD buffer address must be four alignment
	\par [Example]
	\code    
		 LCD_SetLcdBufAddr(0x200000);
	\endcode
*/
void LCD_SetLcdBufAddr (uint32_t ulAddr);
//------------------------------------------------------------------------------
/*!
	\brief 		Get LCD Channel Buffer Information Function
	\param[in]	tCh Selet Nth channel (LCD_CH0~3)
	\return     LCD Nth channel idle buffer information, include buffer address and state
	\par [Example]
	\code    
		 LCD_BUF_TYP *pLcdCh0Buf;

		 pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
	\endcode
*/
LCD_BUF_TYP *pLCD_GetLcdChBufInfor (LCD_CH_TYP tCh);
//------------------------------------------------------------------------------
/*!
	\brief 		Set LCD Channel Buffer Ready Function
	\param[in]	pBufInfor Selet buffer information
	\par [Example]
	\code    
		 LCD_BUF_TYP *pLcdCh0Buf;

		 pLcdCh0Buf = pLCD_GetLcdChBufInfor(LCD_CH0);
		 if (LCD_BUF_EMPTY == pLcdCh0Buf->tBufState)
		 {
     		//! Move data to pLcdCh0Buf->ulBufAddr
     		LCD_SetChBufReady(pLcdCh0Buf);
		 }
	\endcode
*/
void LCD_SetChBufReady (LCD_BUF_TYP *pBufInfor);
//------------------------------------------------------------------------------
/*!
	\brief 			R/W LCD Jpeg QTable Function
	\param[in]		tType LCD_JPEG_QW (Write) or LCD_JPEG_QR (Read)
	\param[in]		ubAddr Access LCD Jpeg Qtable address (0x00~0x7F)
	\param[in]		ubLen  R/W LCD Jpeg Qtable N bytes (1~128)
	\param[in,out]	pData  R/W LCD Jpeg Qtable data
	\par [Example]
	\code    
		 uint8_t pBuf[0x80]={0};

		 LCD_JpegRwQTab(LCD_JPEG_QW, 0, 0x80, pBuf);
	\endcode
*/
void LCD_JpegRwQTab (LCD_JPEG_QRW_TYP tType, uint8_t ubAddr, uint8_t ubLen, uint8_t *pData);
//------------------------------------------------------------------------------
/*!
	\brief 		Enable LCD Channel Function
	\param[in]	tCh Selet Nth Channel
	\par [Example]
	\code    
		 LCD_ChEnable(LCD_CH0);
	\endcode
*/
void LCD_ChEnable (LCD_CH_TYP tCh);
//------------------------------------------------------------------------------
/*!
	\brief 		Disable LCD Channel Function
	\param[in]	tCh Selet Nth Channel
	\par [Example]
	\code    
		 LCD_ChDisable(LCD_CH0);
	\endcode
*/
void LCD_ChDisable (LCD_CH_TYP tCh);
//------------------------------------------------------------------------------
/*!
	\brief 		LCD Display Channel Window Shift Function
	\param[in]	tCh Selet Nth Channel
	\param[in]	swWinXSft	 	   Channel display X axis shift
	\param[in]	swWinYSft	 	   Channel display Y axis shift
	\return		LCD_OK 		 	   LCD channel shift ok!
	\return		LCD_WINXY_FAIL	   LCD channel XY shift Fail!
	\return		LCD_WINX_FAIL	   LCD channel X  shift Fail!
	\returrn	LCD_WINY_FAIL	   LCD channel Y  shift Fail!
	\par [Example]
	\code    
		 tLCD_WinSft(LCD_CH0, 1, -1);
	\endcode
*/
LCD_RESULT tLCD_WinSft (LCD_CH_TYP tCh, short swWinXSft, short swWinYSft);
//------------------------------------------------------------------------------	
/*!
	\brief 		LCD Start Display Function
	\param[in]	ubBoundary When LCD_CH0~3 has no new signal over ubBoundary/60 seconds, LCD auto disable LCD_CH0~3\n
						   When ubBounary >= LCD_CH_NOIMG_MAX_BOUNDARY, LCD has no auto disable any LCD_CH0~3 feature 
	\par [Example]
	\code
		 LCD_SetChannelDisableBoundary(60);
	\endcode
*/
void LCD_SetChannelDisableBoundary(uint8_t ubBoundary);
//------------------------------------------------------------------------------	
/*!
	\brief 		LCD Start Display Function
	\par [Example]
	\code
		 LCD_Start();
	\endcode
*/
void LCD_Start (void);
//------------------------------------------------------------------------------
/*!
	\brief 	LCD Stop Display Function
	\par [Example]
	\code    
		 LCD_Stop();
	\endcode
*/
void LCD_Stop (void);
//------------------------------------------------------------------------------
/*!
	\brief 	LCD Suspend state
	\par [Example]
	\code    
		 LCD_Stop();
	\endcode
*/
void LCD_Suspend (void);
//------------------------------------------------------------------------------
/*!
	\brief 	LCD Resume Display Function
	\par [Example]
	\code    
		 LCD_Stop();
	\endcode
*/
void LCD_Resume (void);
//------------------------------------------------------------------------------	
/*!
	\brief 		LCD Back Light Control Function
	\param[in]	tLevel LCD_BL_OFF is disable LCD back light
					   LCD_BL_LV has eight levels
					   LCD_BL_LV8 is the highest level of LCD back light
	\par [Example]
	\code    	
		  LCD_BackLightCtrl(LCD_BL_LV4);
	\endcode
*/
void LCD_BackLightCtrl (LCD_BL_TYP tLevel);
//------------------------------------------------------------------------------	
/*!
	\brief 	Set LCD Logo Jpeg Buffer Address Function for OSD Library
	\note	No Display Image and Video
	\par [Example]
	\code    	
		  LCD_SetOsdLogoJpegBufAddr();
	\endcode
*/
void LCD_SetOsdLogoJpegBufAddr (void);
//------------------------------------------------------------------------------	
/*!
	\brief 	Get LCD Ouput Horizontal Size
	\return	LCD ouput horizontal size
	\par [Example]
	\code    	
		  uint16_t uwHSize;
		  
		  uwHSize = uwLCD_GetLcdHoSize();
	\endcode
*/
uint16_t uwLCD_GetLcdHoSize (void);
//------------------------------------------------------------------------------	
/*!
	\brief 	Get LCD Ouput Vertical Size
	\return	LCD ouput vertical size
	\par [Example]
	\code    	
		  uint16_t uwVSize;
		  
		  uwVSize = uwLCD_GetLcdHoSize();
	\endcode
*/
uint16_t uwLCD_GetLcdVoSize (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get LCD Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwLCD_GetVersion();
		 printf("LCD Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwLCD_GetVersion (void);
//------------------------------------------------------------------------------	
/*!
	\brief 		LCD Pixel Clock Setting Function
	\par [Example]
	\code    	
		  LCD_PixelPllSetting();
	\endcode
*/
void LCD_PixelPllSetting (void);
//------------------------------------------------------------------------------	
/*!
	\brief 		Setting LCD Gamma Level Function
	\param[in]	ubGammaLevel LCD gamma level (0~6)
					  
	\par [Example]
	\code    	
		  LCD_SetGammaLevel(4);
	\endcode
*/
void LCD_SetGammaLevel(uint8_t ubGammaLevel);
//------------------------------------------------------------------------------
/*!
	\brief		LCD Dynamic Cropping and Scaler Function (for one Channel [1T])
	\param[in]	*pDynInfor	   LCD Dynamic Information pointer
	\return		LCD_OK 		   LCD channels cropping and scaler ok!
	\return		LCD_DISP_FAIL  LCD 1T display fail!
	\return		LCD_CROP_FAIL  LCD channels cropping fail!
	\return		LCD_SCALE_FAIL LCD channels scaler fail!
	\par [Example]
	\code
		 //! LCD displays 1T (Dynamic)
		 LCD_DYN_INFOR_TYP sDynInfor;
		 
		 sDynInfor.uwLcdOutputHsize = 320;
		 sDynInfor.uwLcdOutputVsize = 240;
		 sDynInfor.tChRes.uwChInputHsize = 640;
		 sDynInfor.tChRes.uwChInputVsize = 480;
		 sDynInfor.tChRes.uwCropHstart = 0;
		 sDynInfor.tChRes.uwCropVstart = 0;
		 sDynInfor.tChRes.uwCropHsize = 640;
		 sDynInfor.tChRes.uwCropVsize = 480; 
		 tLCD_CropScale(&sDynInfor);
	\endcode	
*/
LCD_RESULT tLCD_DynamicOneChCropScale (LCD_DYN_INFOR_TYP *pDynInfor);
//osMessageQId* pLCD_SwitchModeInit(uint32_t ulStackSize, osPriority priority);
#endif
