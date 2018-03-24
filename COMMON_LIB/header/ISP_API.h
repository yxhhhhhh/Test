/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ISP_API.h
	\brief		ISP API header
	\author			
	\version	0.3
	\date		2017-08-31
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _ISP_API_H_
#define _ISP_API_H_
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
#define DS_TEXT_NUM 32

#define ISP_FuncEnable											\
						{										\
							GLB->ISP_FUNC_DIS 	 = 0;			\
							GLB->ISP_MD_FUNC_DIS = 0;			\
							GLB->ISP_DIS_FUNC_DIS = 0;			\
							GLB->ISP_YUV_PATH1_FUNC_DIS = 0;	\
							GLB->ISP_YUV_PATH2_FUNC_DIS = 0;	\
							GLB->ISP_YUV_PATH3_FUNC_DIS = 0;	\
						}
#define ISP_FuncDisable											\
						{										\
							GLB->ISP_FUNC_DIS 	 = 1;			\
							GLB->ISP_MD_FUNC_DIS = 1;			\
							GLB->ISP_DIS_FUNC_DIS = 1;			\
							GLB->ISP_YUV_PATH1_FUNC_DIS = 1;	\
							GLB->ISP_YUV_PATH2_FUNC_DIS = 1;	\
							GLB->ISP_YUV_PATH3_FUNC_DIS = 1;	\
						}

enum
{
	DS_OFF = 0,
	DS_ON,
};

enum
{
	DS_DS1 = 1,
	DS_DS2,
};

enum
{
	DS_8x16 = 0,
	DS_12x16,
	DS_16x16,	
};

enum
{
	PM_OFF = 0,
	PM_ON,
};

enum
{
	PM_8x6 = 1,
	PM_16x12,
};

enum _IMG_FORMAT
{
	SONIX420 = 0,
	Y_ONLY,
	YCBCR_YUY2,
	YCBCR_SIGN,
	RGB888,
	BGR888,
	RGB565,
	BGR565,
	RGB888_PLANAR,
	BGR888_PLANAR,
	I420_YCBCR,    
    I420_YUV, 
};

enum
{
	ISP_NORMAL = 0,
	ISP_RAW_DATA,
};

//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!
\brief Set ISP report initial state
\return(no)
\par [Example]
\code 
		ISP_Init();
\endcode
*/
void ISP_Init(void);

//------------------------------------------------------------------------------//
//                               Date stamp                                     //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Date stamp switch.
\param ubSelect     Select DS1/2.
\param ubFlag       DS on/off.
\return(no)
\par [Example]
\code 
		ISP_SetOsdSwitch(DS_DS1, DS_OFF);
\endcode
*/
void ISP_SetOsdSwitch(uint8_t ubSelect, uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Set date stamp color.
\param ubSelect 	Select DS1/2.
\param ubIndex 		Select txt color1/2/3.
\param ubRed		Red value(0~255).
\param ubGreen		Green value(0~255).
\param ubBlue		Blue value(0~255).
\return(no)
\par [Example]
\code
	RGB -> YUV
	Y = 0.3 * R + 0.59 * G + 0.11 * B
	U = 0.493 * ( B - Y)
	V = 0.877 * ( R - Y)	
	
	ISP_SetOsdColor(DS_DS1, DS_COLOR1, 255, 0, 0);
\endcode
*/
void ISP_SetOsdColor(uint8_t ubSelect, uint8_t ubIndex, uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue);
//------------------------------------------------------------------------
/*!
\brief Set date stamp position.
\param ubSelect 	Select DS1/2.
\param ubGain 		Set DS mapping gain.
\param uwRow 		Horizontal size.
\param uwCol		Vertical size.
\return(no)
\par [Example]
\code
	ubGain range is 0~7 , 0 => x8 and 1-7 => x1-x7.
	DS column 0 and 1 window can't be overlapped.
	
	ISP_SetOsdPos(DS_DS1, DS_GAIN1, 0, 0);
\endcode
*/
void ISP_SetOsdPos(uint8_t ubSelect, uint8_t ubGain, uint16_t uwRow, uint16_t uwCol);
//------------------------------------------------------------------------
/*!
\brief Date stamp font table.
\param ubSelect 			Select DS1/2.
\param pllFontTable 		Point of font table.
\return(no)
\par [Example]
\code
		ullDS_FontTable[512] =
		{
			///////////////0////////////////////
			0x0022222222222200,
			0x0223333333333220,
			0x2233332222333322,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2233332222333322,
			0x0223333333333220,
			0x0022222222222200,	
			...
		};
		and SEN->OSD1_SIZE = DS_16x16;		
		// 16x16 (2 bits per pixel, 16 pixels per line, 16 line per char):
		
		ISP_LoadOsdFontTable(DS_DS1, &ullDS_FontTable[0]);		
\endcode
*/
void ISP_LoadOsdFontTable(uint8_t ubSelect,uint64_t *pllFontTable);
//------------------------------------------------------------------------
/*!
\brief Date stamp display.
\param ubSelect 				Select DS1/2.
\param ubDateStampLine1 		Point of DS1 data.
\param ubDateStampLine2 		Point of DS2 data.
\return(no)
\par [Example]
\code 
		uint8_t ubISP_DSLine1[32] = {	
			0,	 1,  2,	 3,	 4,	 5,  6,	 7,	 8,	 9,	10,	11,	12,	13, 14,	15,
			16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		};

		index value mapping font table value and font value show on image.
		ISP_OsdDisplay(DS_DS1 ,&ubISP_DSLine1[0] ,&ubISP_DSLine2[0]);
\endcode
*/
void ISP_OsdDisplay(uint8_t ubSelect, uint8_t *ubDateStampLine1, uint8_t *ubDateStampLine2);
//------------------------------------------------------------------------------//
//                               Private Mask                                   //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Private mask block size.
\param uwHsize 					Private mask horizontal size.
\param uwVsize 					Private mask vertical size.
\param ubCase 					Case 1:block 8x6 , Case2:block 16x12
\return(no)
\par [Example]
\code
		ISP_SetPrivateMaskSize(1280, 720, PM_8x6);

		sensor resolution is HD and select case1
		PRI_MASK_H_SIZE = H_SIZE/8 = 1280/8 = 160 = 0xA0
		PRI_MASK_V_SIZE = V_SIZE/6 = 720 /6 = 120 = 0x78
		each mask block is 160x120
		sensor resolution is HD and select case2
		PRI_MASK_H_SIZE = H_SIZE/16 = 1280/16 = 80 = 0x50
		PRI_MASK_V_SIZE = V_SIZE/12 = 720 /12 = 60 = 0x3C
		each mask block is 80x60
\endcode
*/
void ISP_SetPrivateMaskSize(uint16_t uwHsize, uint16_t uwVsize, uint8_t ubCase);
//------------------------------------------------------------------------
/*!
\brief Set private mask block color.
\param ubYColor 					Y[0 to 15]
\param ubUColor 					U[0 to 15]
\param ubVColor 					V[0 to 15]
\return(no)
\par [Example]
\code
	ISP_SetPrivateMaskColor(0 ,0x08 ,0x08);

	 0: select 8'h00,  1: select 8'h10,  2: select 8'h20,   3: select 8'h30,
	 4: select 8'h40,  5: select 8'h50,  6: select 8'h60,   7: select 8'h70,
	 8: select 9'h80,  9: select 8'h90, 10: select 8'ha0,  11: select 8'hb0,
	12: select 8'hc0, 13: select 8'hd0, 14: select 8'he0,  15: select 8'hf0.

	R = Y + 1.139 * (V - 128)
	G = Y - 0.394 * (U - 128) - 0.58 *(V - 128)
	B = Y + 2.032 * (U - 128)
\endcode
*/
void ISP_SetPrivateMaskColor(uint8_t ubYColor, uint8_t ubUColor, uint8_t ubVColor);
//------------------------------------------------------------------------
/*!
\brief Set private mask switch.
\param ubFlag 					Private mask on/off
\return(no)
\par [Example]
\code 
		ISP_SetPrivateMaskSwitch(PM_ON);
\endcode
*/
void ISP_SetPrivateMaskSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Private mask trigger.
\return(no)
\par [Example]
\code 
	***If user want to change PM register, must trigger ***	
		ISP_PrivateMaskTrigger();		
\endcode
*/
void ISP_PrivateMaskTrigger(void);
//------------------------------------------------------------------------
/*!
\brief Conversion private mask data.
\param ubType 						Case 1:block 8x6 , Case2:block 16x12
\param ubPrivateArea 			Point of array data.
\return(no)
\par [Example]
\code 
	ubPrivateMaskData1[24] = {	
		0xaa,
		0x55,
		0x33,
		0xcc,
		0x69,
		0x69,
	};
	and type is PM_8x6.

	ISP_ConversionPrivateMask(PM_8x6, &ubPrivateMaskData1[0]);	
	---------------------------------
	| 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 |		
	| 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 |	
	| 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 |	
	| 1 | 1 | 0 | 0 | 1 | 1 | 0 | 0 |	
	| 0 | 1 | 1 | 0 | 1 | 0 | 0 | 1 |	
	| 0 | 1 | 1 | 0 | 1 | 0 | 0 | 1 |
	|--------------------------------  1/0 => mask/normal area
\endcode
*/
void ISP_ConversionPrivateMask(uint8_t ubType, uint8_t * ubPrivateArea);
//------------------------------------------------------------------------------//
//                              ISP function switch                             //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Dynamic range control switch.
\param ubFlag 	DRC switch.
\return(no)
\par [Example]
\code 
		ISP_DRCSwitch(1);
\endcode
*/
void ISP_DRCSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Vertical lens distortion compensation switch.
\param ubFlag 	VLDC switch.
\return(no)
\par [Example]
\code 
		ISP_VLDCSwitch(1);
\endcode
*/
void ISP_VLDCSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Noise reduce 2D switch.
\param ubFlag 	NR2D switch.
\return(no)
\par [Example]
\code 
		ISP_NR2DSwitch(1);
\endcode
*/
void ISP_NR2DSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Noise reduce 3D switch.
\param ubFlag 	NR3D switch.
\return(no)
\par [Example]
\code 
		ISP_NR3DSwitch(1);
\endcode
*/
void ISP_NR3DSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------------//
//                         Scaler function application                          //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Scale-up setting, when scale-up happened need to set this function.
\param ubScaleUp 	SEN_SCALE_UP/SEN_SCALE_DOWN.
\return(no)
\par [Example]
\code 
		ISP_ScalerSetting(SEN_SCALE_UP);
\endcode
*/
void ISP_ScalerSetting(uint8_t ubScaleUp);
//------------------------------------------------------------------------
/*!
\brief Zoom to area.
\param uwZoomHsize 	Zoom horizontal size.
\param uwZoomVsize 	Zoom vertical size .
\param uwHStart 	Horizontal start point.
\param uwVStart 	Vertical start point.
\return(no)
\par [Example]
\code
		sensor output = 1280x720					ISP scaler = 1280x720
    -----------                       -----------	
    | A  | B  |                       |         |
    |---------| ------------------->  |    A    |
    | C  | D  |                       |         |
    -----------                       -----------
		ISP_Zoom2Area(640, 360, 0, 0); 		(A)
        ......
        ISP_Zoom2Area(640, 360, 1280, 720); (D)

		RATIO_H/V = 64.[scale up is 2]
		H/VSTART 	= 0/0(A), 1280/0(B), 0/720(C), 1280/720(D)			
		H/VSIZE 	= half sensor output H/V
    note:(H_START + (RATIO_H) *H_SIZE) <= 1280 * RATIO_H
		 (V_START + (RATIO_V) *V_SIZE) <= 720 * RATIO_V
		 uwZoomHsize & uwZoomVsize must bigger than half of sensor output resolution.
\endcode
*/
void ISP_Zoom2Area(uint16_t uwZoomHsize, uint16_t uwZoomVsize, uint16_t uwHStart, uint16_t uwVStart);
//------------------------------------------------------------------------
/*!
\brief Pan-tilt-zoom achieve.
\param ubPath 	Select path.
\param uwFrameHsize 	Horizontal size of frame.
\param uwFrameVsize 	Vertical size of frame.
\param uwHStart 	Horizontal start point.
\param uwVStart 	Vertical start point.
\return(no)
\par [Example]
\code
		sensor output = 1280x720					ISP scale = 320x240
		-----------                       -----------
		|  A |	  |                       |		    |
		|-----	  |	------------------->  |	   A	|
		|		  |                       |		    |
		-----------                       -----------
	ex:
		Block A are composed of any H/Vsize and H/VStart 
		If block A is 320x240,
			H/V_start limit are (0~(1280-320)) and (0~(720-240)).
		(RATIO_H/V = 128)
			
		If block A is 640x480,
			H/V_start limit are (0~640) and (0~240).
		(RATIO_H/V = 256)		
\endcode
*/
void ISP_PTZapplication(uint8_t ubPath, uint16_t uwFrameHsize, uint16_t uwFrameVsize, uint16_t uwHStart, uint16_t uwVStart);
//------------------------------------------------------------------------
/*!
\brief Cropping from ISP window.
\param ubPath 	Select path1/2/3.
\param uwHsize 		Horizontal size of cropping.
\param uwVsize 		Vertical size of cropping.
\param uwHStart 	Horizontal start point.
\param uwVStart 	Vertical start point.
\return(no)
\par [Example]
\code
		sensor output = 1280x720					ISP scaler size is 640x480
		-----------                       -----------
		|  A |	  |                       |		    |
		|-----	  |	------------------->  |	   A    |
		|		  |                       |		    |
		-----------                       -----------
		ISP_WindowCropping(SENSOR_PATH1, 640, 480, 0, 0);		
		
	ex:
		Block A are composed of any H/Vsize and H/VStart 
		If block A is 320x240,
			H/V_start limit are (0~960) and (0~480).
		(RATIO_H/V = 128)
			
		If block A is 640x480,
			H/V_start limit are (0~640) and (0~240).
		(RATIO_H/V = 128)	
\endcode
*/
void ISP_WindowCropping(uint8_t ubPath, uint16_t uwHsize, uint16_t uwVsize, uint16_t uwHStart, uint16_t uwVStart);
//------------------------------------------------------------------------
/*!
\brief Set ISP output scaler size.
\param ubPath 	Select path1/2/3.
\param uwScalerHsize 	Horizontal size of ISP output.
\param uwScalerVsize 	Vertical size of ISP output.
\param uwHsize 				Horizontal size of sensor output.
\param uwVsize 				Vertical size of sensor output.
\return(no)
\par [Example]
\code 
		sensor output=HsizexVsize  		ISP scale = ScalerHxScalerV
      uwHsize                         uwScalerHsize
    -----------                       -----------	
    |         |                       |         | 
    |         |	uwVsize               |         | uwScalerVsize
    |         |                       |         |
    -----------                       -----------
		***only path1 support scale-up and maximum is 2.
		
		ISP_SetScaler(SENSOR_PATH1, ulUpImgW, ulUpImgH, ulImgW, ulImgH);
\endcode
*/
void ISP_SetScaler(uint8_t ubPath, uint16_t uwScalerHsize, uint16_t uwScalerVsize, uint16_t uwHsize, uint16_t uwVsize);
//------------------------------------------------------------------------
/*!
\brief Set Saturation.
\param ubSatu 		Saturation value.
\return(no)
\par [Example]
\code 
        ubSatu = 0~255;
		ISP_SetIQSaturation(ubCont);
\endcode
*/
void ISP_SetIQSaturation(uint8_t ubSatu);
//------------------------------------------------------------------------
/*!
\brief Set Chroma.
\param ubChroma 		Chroma value.
\return(no)
\par [Example]
\code 
        ubChroma = 0~255;
		ISP_SetIQChroma(ubChroma);
\endcode
*/
void ISP_SetIQChroma(uint8_t ubChroma);
//------------------------------------------------------------------------
/*!
\brief Set Contrast.
\param ubCont 		Contrast value.
\return(no)
\par [Example]
\code 
        ubCont = 0~255;
		ISP_SetIQContrast(ubCont);
\endcode
*/
void ISP_SetIQContrast(uint8_t ubCont);
//------------------------------------------------------------------------
/*!
\brief Set Brightness.
\param ubBri 		Brightness value.
\return(no)
\par [Example]
\code 
        ubBri = 0~255;
		ISP_SetIQBrightness(ubBri);
\endcode
*/
void ISP_SetIQBrightness(uint8_t ubBri);
//------------------------------------------------------------------------------//
//                         Other ISP control                                    //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Set AE power frequency.
\param ubPwrFreq 		set 50/60Hz.
\return(no)
\par [Example]
\code 
		ISP_SetAePwrFreq(ubPowFreq);
\endcode
*/
void ISP_SetAePwrFreq(uint8_t ubPwrFreq);
//------------------------------------------------------------------------
/*!
\brief Set sensor output type.
\param ubType 	switch YUV420/raw data.
\return(no)
\par [Example]
\code 
		ISP_SenosrOutputType(1);
\endcode
*/
void ISP_SenosrOutputType(uint8_t ubType);
//------------------------------------------------------------------------
/*!
\brief Set path3 image format type.
\param ubType 	Formar type.
\return(no)
\par [Example]
\code 
		ISP_SetPath3ImgFormat(SONIX420);
\endcode
*/
void ISP_SetPath3ImgFormat(uint8_t ubType);
//------------------------------------------------------------------------
/*!
\brief Set 3DNR fame buffer compression.
\return(no)
\par [Example]
\code 
		ISP_Set3DNR_FBC();
\endcode
*/
void ISP_Set3DNR_FBC(void);
//------------------------------------------------------------------------------
/*!
\brief 	Get ISP function version	
\return	image signal process version.
\par [Example]
\code		 
	 uwISP_GetVersion();
\endcode
*/
uint16_t uwISP_GetVersion (void);
#endif
