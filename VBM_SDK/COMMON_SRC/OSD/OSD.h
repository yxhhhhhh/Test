/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OSD.h
	\brief		LCD OSD Funcations Header
	\author		Pierce
	\version	1.0
	\date		2018/07/25
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _OSD_H_
#define _OSD_H_
#include "LCD.h"
/*!	\file OSD.h
OSD Menu Flow Chart:
	\dot
digraph OSD_LogoJpeg {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		edge [fontname=Verdana];
		LCD_Init -> ulLCD_CalLcdBufSize;
		ulLCD_CalLcdBufSize -> LCD_SetLcdBufAddr [fontcolor=blue, fontsize=12, label="Assign LCD Buffer"];
		LCD_SetLcdBufAddr -> OSD_LogoJpeg;
		OSD_LogoJpeg -> OSD_LogoJpeg [fontcolor=blue, fontsize=8, headport=ne, label="Change display Logo Jpeg"];
		OSD_LogoJpeg -> LCD_Stop [fontcolor=blue, fontsize=8, label="Display video"];
		LCD_Stop -> LCD_Init [tailport=e];
	}
	\enddot
OSD Display Flow Chart:	
	\dot
digraph OSD_DisplayFlow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		edge [fontname=Verdana];
		tOSD_Init -> ulOSD_CalBufSize -> OSD_SetOsdBufAddr;
		OSD_SetOsdBufAddr -> tOSD_Img1;
		tOSD_Img1 -> OSD_ClearImg1Buf [tailport=w];
		OSD_ClearImg1Buf -> tOSD_Img1[fontcolor=blue, fontsize=8, label="new palette"];
		tOSD_Img1 -> OSD_EraserImg1[headport=nw];
 		OSD_EraserImg1-> tOSD_Img1[fontcolor=blue, fontsize=8, label="Same palette"];
		tOSD_Img1 -> OSD_UpdateQueueBuf [fontcolor=blue, fontsize=8, label="Queue"];
		tOSD_Img1,OSD_SetOsdBufAddr -> tOSD_Img2 [fontcolor=blue, fontsize=8, label="Update / Queue"];
		tOSD_Img1 -> tOSD_Img1[headport=ne, tailport=e, fontcolor=blue, fontsize=8, label="Same palette"];		
		tOSD_Img2 -> tOSD_Img2[headport=ne, tailport=e, fontcolor=blue, fontsize=8, label="Same palette"];
		tOSD_Img2 -> "tOSD_FontPrintf\ntOSD_MenuString" [tailport=sw, fontcolor=blue, fontsize=8, label="Update / Queue"];
		tOSD_Img2 -> OSD_ClearImg2Buf [headport=nw];
		OSD_ClearImg2Buf -> tOSD_Img2[fontcolor=blue, fontsize=8, label="Same palette"];		
		tOSD_Img2 -> OSD_EraserImg2[headport=nw];
 		OSD_EraserImg2-> tOSD_Img2[tailport=n, fontcolor=blue, fontsize=8, label="Same palette"];
		tOSD_Img2 -> OSD_UpdateQueueBuf [tailport=w, fontcolor=blue, fontsize=8, label="Queue"];
		"tOSD_FontPrintf\ntOSD_MenuString" -> "tOSD_FontPrintf\ntOSD_MenuString" [headport=ne, tailport=e];
		"tOSD_FontPrintf\ntOSD_MenuString" -> OSD_EraserFont[headport=nw];
		OSD_EraserFont -> "tOSD_FontPrintf\ntOSD_MenuString";
		"tOSD_FontPrintf\ntOSD_MenuString" ->  OSD_UpdateQueueBuf [tailport=sw, fontcolor=blue, fontsize=8, label="Queue"];
		tOSD_Img1 -> "tOSD_FontPrintf\ntOSD_MenuString"  [ fontcolor=blue, fontsize=8, label="Update / Queue"];
	}
	\enddot
*/
//------------------------------------------------------------------------------
/*!
	\brief ConfigGen Font Sample Index
*/
#define OSD_ANT_L_0		(1)
#define OSD_ANT_L_1		(2)
#define OSD_ANT_L_2		(3)
//------------------------------------------------------------------------------
#define OSD_ANT_R_0		(32)
#define OSD_ANT_R_2		(4)
#define OSD_ANT_R_3		(5)
#define OSD_ANT_R_4		(6)
#define OSD_ANT_R_5		(7)
//------------------------------------------------------------------------------
#define OSD_BAT_L_0		(8)
#define OSD_BAT_L_1		(9)
#define OSD_BAT_L_2		(10)
#define OSD_BAT_L_3		(11)
#define OSD_BAT_L_4		(12)
//------------------------------------------------------------------------------
#define OSD_BAT_R_0		(13)
#define OSD_BAT_R_4		(14)
#define OSD_BAT_R_5		(15)
#define OSD_BAT_R_6		(16)
#define OSD_BAT_R_7		(17)
//------------------------------------------------------------------------------
#define OSD_VDOMODE_L_0	(27)
#define OSD_VDOMODE_L_1	(29)
#define OSD_VDOMODE_L_2	(33)
#define OSD_VDOMODE_L_3	(32)
//------------------------------------------------------------------------------
#define OSD_VDOMODE_R_0	(28)
#define OSD_VDOMODE_R_1	(30)
#define OSD_VDOMODE_R_2	(34)
//------------------------------------------------------------------------------
#define OSD_REC_0		(36)			//REC		
#define OSD_REC_1		(35)			//STOP
#define OSD_REC_2		(37)			//PLAY
#define OSD_REC_3		(31)			//PAUSE
//------------------------------------------------------------------------------
#define OSD_BAR5_A_0	(38)
#define OSD_BAR5_A_1	(41)
//------------------------------------------------------------------------------
#define OSD_BAR5_B_0	(39)
#define OSD_BAR5_B_2	(42)
//------------------------------------------------------------------------------
#define OSD_BAR5_D_0	(40)
#define OSD_BAR5_D_4	(43)
//------------------------------------------------------------------------------
#define OSD_CAM_0		(49)	//123			//STA0
#define OSD_CAM_1		(50)	//124			//STA1
#define OSD_CAM_2		(51)	//125			//STA2
#define OSD_CAM_3		(52)	//126			//STA3
#define OSD_CAM_4		(53)	//127			//STA4
#define OSD_CAM_5		(54)	//128			//STA5
#define OSD_CAM_6		(55)	//129			//STA6
#define OSD_CAM_7		(56)	//130			//STA7
#define OSD_CAM_8		(57)	//131			//STA8
//------------------------------------------------------------------------------
//#define OSD_MENU0_ADDR			(0x1000)	//!< OSD menu0 SF address
//#define OSD_MENU1_ADDR			(0x30000)	//!< OSD menu1 SF address
//#define OSD_IMG10_ADDR			(0x10000)	//!< OSD image 1_0 SF address
//#define OSD_IMG_ADDR			(0x58880)//(0x243880)  //(0x120000)	//!< OSD image SF address
//#define OSD_FONT_ADDR			(0x23230)	//!< OSD font SF address
//#define OSD_MEMU_STRING_ADDR	(0x63230)	//!< OSD Menu String SF address

//#define OSD_MENU0_LEN		(0xE9C6)	//!< OSD menu0 length
//#define OSD_MENU1_LEN		(0xE4B60)	//!< OSD menu1 length
//#define OSD_IMG10_LEN		(0x12E50)	//!< OSD image 1_0 length
//#define OSD_IMG_LEN			(0xE75B9)	//!< OSD image length
//#define OSD_FONT_LEN		(0x9830)	//!< OSD font SF length
//------------------------------------------------------------------------------
////#define OSD_IMG1_PAT_LEN		(159)		//!< OSD image 1 palette length
////#define OSD_IMG2_PAT_LEN		(88)		//!< OSD image 2 palette length
//#define OSD_IMG_PAT_NUM			(246)		//!< OSD image palette length
//#define OSD_FONT_PAT_NUM		(8)			//!< OSD font palette length

////#define OSD_IMG_PAT_SFT		(1)			//!< OSD image 1 palette index shift
////#define OSD_IMG1_PAT_SFT		(1)			//!< OSD image 1 palette index shift
////#define OSD_IMG2_PAT_SFT		(0xA0)		//!< OSD image 2 palette index shift
//#define OSD_FONT_PAT_SFT		(0xF8)		//!< OSD font palette index shift
//------------------------------------------------------------------------------
/*!
	\brief Logo Jpeg File Offset by ConfigGen
*/
#define OSD_LOGO_JPEG_FILESIZE_SFT			(0)
//! Jpeg0
//#define OSD_LOGO_JPEG_Q0_ADDR_SFT			(0x1A + OSD_LOGO_JPEG_FILESIZE_LEN)
//#define OSD_LOGO_JPEG_Q1_ADDR_SFT			(0x5B + OSD_LOGO_JPEG_FILESIZE_LEN)
//! Jpeg1
#define OSD_LOGO_HEADER_LEN					(0x800)//(0x400)
#define OSD_LOGO_MAXSIZE_SFT				(0x20)
#define OSD_LOGO_BSLEN_SFT					(0x140)
#define OSD_LOGO_JPEG_Q0_ADDR_SFT			(0x1A)//(0x1E)//(0x1D)
#define OSD_LOGO_JPEG_Q1_ADDR_SFT			(0x5B)//(0x5F)//(0x5E)
#define OSD_JPEG_HEADER_LEN					(0x24D)
#define OSD_LOGO_JPEG_STREAM_ADDR_SFT		(OSD_JPEG_HEADER_LEN)//(0x24D + OSD_LOGO_JPEG_FILESIZE_LEN)

#define OSD_LOGO_JPEG_MAXSIZE_LEN			(4)
#define OSD_LOGO_JPEG_FILEADDR_LEN			(4)
#define OSD_LOGO_JPEG_FILESIZE_LEN			(4)
#define OSD_LOGO_JPEG_Q0_LEN				(0x40)
#define OSD_LOGO_JPEG_Q1_LEN				(0x40)
#define OSD_LOGO_JPEG_STREAM_LEN			(0xE779)
//------------------------------------------------------------------------------
/*!
	\brief For OSD HW
*/
#define OSD_PAT_NUM							(0x100)		//!< OSD HW palette length
#define OSD_PAT_CR_NUM						(1)			//!< OSD palette Transparent color length
#define OSD_PAT_BG_NUM						(1)			//!< OSD palette background color length
//------------------------------------------------------------------------------
/*!
	\brief OSD Library Buffer Length
*/
#define OSD_FONT_MAX_SIZE					(64)		//!< OSD font size nxn
#define OSD_MENU_STRING_MAX_NUM 			(50)		//!< OSD menu string font max number
//------------------------------------------------------------------------------
/*!
	\brief OSD Menu String Bin File Offset by ConfigGen
*/
#define OSD_MENU_HEADER_LANG_SFT			(0x3)
#define OSD_MENU_STRING_SFT					(0x710)
//------------------------------------------------------------------------------
/*!
	\brief OSD Image File Offset by ConfigGen
*/
#define OSD_PAT_SFT							(0x20)		//!< Palette table offset 
//#define OSD_DATA_ADDR_SFT					(0x222)
//------------------------------------------------------------------------------
//! OSD Image Bin File Offset
#define OSD_GROUP_TOP_LEN					(0x10)
#define OSD_GROUP_COUNT_SFT					(0xA)
#define OSD_NEXT_GROUP_ADDR_SFT				(0x1)
#define OSD_OSD1_PAT_NUM_SFT				(0xC)
#define OSD_OSD1_IMAGE_NUM_SFT				(0xD)
#define OSD_OSD2_IMAGE_NUM_SFT				(0xF)
#define OSD_GROUP_HEADER_SFT				(0x220)
//------------------------------------------------------------------------------
/*!
	\brief OSD Font File Offset by ConfigGen
*/
#define OSD_FONT_PT_SFT						(0x200)
#define OSD_FONT_H_SFT						(1)
#define OSD_FONT_V_SFT						(3)
#define OSD_FONT_LANG_MAX_SFT				(OSD_FONT_PT_SFT + 0x6)
#define OSD_FONT_NEXT_LANG_OFFSET_SFT		(0x8)
#define OSD_FONT_NUM_MAX_SFT				(0xC)
#define OSD_FONT_PAT_NUM_SFT				(OSD_FONT_PT_SFT + 0xE)
#define OSD_FONT_NEXT_ROTATION_OFFSET_SFT	(0x12)
#define OSD_FONT_NEXT_COLOR_OFFSET_SFT		(0x16)

#define OSD_FONT_HEADER_LEN					(0x30)
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define OSD_LOGO_JPEG_QADDR					(0)
#define OSD_LOGO_JPEG_QLEN					(0x80)
//------------------------------------------------------------------------------
#define OSD_BUF_INIT						(0)
#define OSD_BUF1_PAT_STATE					(1)
#define OSD_BUF2_PAT_STATE					(2)
#define OSD_BUF_IMG_STATE					(4)
#define OSD_BUF_PAT_MASK					(OSD_BUF1_PAT_STATE | OSD_BUF2_PAT_STATE)
#define OSD_BUF1_PATIMG_MASK				(OSD_BUF1_PAT_STATE | OSD_BUF_IMG_STATE)
#define OSD_BUF_STATE_MASK					(OSD_BUF_PAT_MASK | OSD_BUF_IMG_STATE)
//#define OSD_BUF_IMG1_STATE				(1)
//#define OSD_BUF_IMG2_STATE				(2)
#define OSD_IMG1_Q							(8)
#define OSD_IMG2_Q							(0x10)
#define OSD_FONT_Q							(0x20)
#define OSD_BUF_UPDATE						(0x80)

//#define OSD_BUF_STATE_MASK				(3)
//------------------------------------------------------------------------------
#define OSD_FONTC_BLACK 					(0x8400)
#define	OSD_FONTC_WHITE						(0x843F)
#define	OSD_FONTC_RED						(0xFA93)
#define	OSD_FONTC_GREEN						(0x1165)
#define	OSD_FONTC_BLUE						(0x6FC7)
#define OSD_FONTC_YELLOW					(0x9038)
#define	OSD_FONTC_CYAN						(0x056C)
#define	OSD_FONTC_GRAY						(0x7C2D)
//------------------------------------------------------------------------------
/*!
	\brief OSD Function Return Result Type
*/
typedef enum
{
	OSD_OK,									//!< OSD setting ok
	OSD_FONT_FAIL,							//!< OSD string special font not support
	OSD_FONT_SWITCH_FAIL,					//!< OSD font switch fail
	OSD_OF_FAIL,							//!< OSD buffer overflow
	OSD_SCALE_FAIL,							//!< OSD scaler fail
	OSD_HSIZE_FAIL, 						//!< OSD horizontal size fail because not quadruple
	OSD_LOCA_FAIL,							//!< OSD loaction fail because out of LCD display range
	OSD_LOADIMAGE_FAIL,
	OSD_DATACPY_FAIL,
}OSD_RESULT;
//------------------------------------------------------------------------------
/*!
	\brief OSD Buffer States
*/
typedef enum
{
	OSD_BUF_EMPTY,							//!< OSD buffer empty
	OSD_BUF_CUR,							//!< OSD buffer used with LCD controller
	FONT_BUF_FULL,							//!< Modify OSD font layer
	OSD_BUF_FULL,							//!< Modify OSD1 and OSD2 layer
	OSD1_BUF_FULL,							//!< Modify OSD1 layer
	OSD2_BUF_FULL							//!< Modify OSD2 layer	
}OSD_BUF_STA_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Update Type
*/
typedef enum
{
	OSD_QUEUE,								//!< OSD display queue type
	OSD_UPDATE								//!< OSD display update type
}OSD_UPDATE_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Update Type
*/
typedef enum
{
	OSD_SCALE_1X,							//!< OSD scale 1
	OSD_SCALE_2X,							//!< OSD scale 2
	OSD_SCALE_3X,							//!< OSD scale 3
	OSD_SCALE_4X							//!< OSD scale 4
}OSD_SCALE_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD and LCD Channels Weight Type
*/
typedef enum
{
	OSD_WEIGHT_8DIV8,						//!< OSD weight 8/8
	OSD_WEIGHT_1DIV8,						//!< OSD weight 1/8
	OSD_WEIGHT_2DIV8,						//!< OSD weight 2/8
	OSD_WEIGHT_3DIV8,						//!< OSD weight 3/8
	OSD_WEIGHT_4DIV8,						//!< OSD weight 4/8
	OSD_WEIGHT_5DIV8,						//!< OSD weight 5/8
	OSD_WEIGHT_6DIV8,						//!< OSD weight 6/8
	OSD_WEIGHT_7DIV8,						//!< OSD weight 7/8
}OSD_WEIGHT_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Font Color Index
*/
//typedef enum
//{
//	OSD_FONT_BLACK = OSD_FONT_PAT_SFT,		//!< OSD font black color index
//	OSD_FONT_WHITE,							//!< OSD font white color index
//	OSD_FONT_RED,							//!< OSD font red color index
//	OSD_FONT_GREEN,							//!< OSD font green color index
//	OSD_FONT_BLUE,							//!< OSD font blue color index
//	OSD_FONT_YELLOW,						//!< OSD font yellow color index
//	OSD_FONT_CYAN,							//!< OSD font cyan color index
//	OSD_FONT_GRAY,							//!< OSD font gray color index
//}OSD_FONT_COLOR_TYPE;
//------------------------------------------------------------------------------
/*!
	\brief OSD Font Color Index from ConfigGen
*/
typedef enum
{
	OSD_FONT_COLOR1,						//!< OSD Font color1 from Config Gen
	OSD_FONT_COLOR2,						//!< OSD Font color2 from Config Gen
	OSD_FONT_COLOR3,						//!< OSD Font color3 from Config Gen
}OSD_FONT_COLOR_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Font Rotation Angle from ConfigGen
*/
typedef enum
{
	OSD_FONT_ROTATION_0,					//!< OSD Font Rotation 0
	OSD_FONT_ROTATION_90,					//!< OSD Font Rotation 90
	OSD_FONT_ROTATION_180,					//!< OSD Font Rotation 180
	OSD_FONT_ROTATION_270					//!< OSD Font Rotation 270
}OSD_FONT_RA_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Layer Type
*/
typedef enum
{
	OSD_IMG1,								//!< OSD image layer1
	OSD_IMG2								//!< OSD image layer2
}OSD_LAYER_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Image Rotation Angle from ConfigGen
*/
typedef enum
{
	OSD_IMG_ROTATION_0,					//!< OSD Font Rotation 0
	OSD_IMG_ROTATION_90,					//!< OSD Font Rotation 90
}OSD_IMG_RA_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Buffer Information
*/
typedef struct
{
	uint32_t 		ulBufAddr;				//!< OSD buffer address
	OSD_BUF_STA_TYP tBufState;				//!< OSD buffer state	
}OSD_BUF_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Total Buffer Information
*/
typedef struct
{
	OSD_BUF_TYP tBuf[LCD_PINGPONG_BUF_MAX];	//!< OSD pingpong buffer
	uint32_t    ulOsdImg1BufAddr;			//!< OSD image 1 layer buffer address
	uint32_t    ulOsdImg2BufAddr;			//!< OSD image 2 layer buffer address
	uint32_t	ulOsdFontBufAddr;			//!< OSD font buffer address
	uint32_t	ulOsdMenuStringBufAddr;		//!< OSD menu string buffer address
}OSD_TOL_BUF_TYP;
//------------------------------------------------------------------------------
/*!
	\brief OSD Image Information from ConfigGen
*/
typedef struct
{
	uint32_t ulAddrSft;						//!< OSD image item shift address at SF
	uint16_t uwHSize;						//!< OSD image item horizontal size
	uint16_t uwVSize;						//!< OSD image item vertical size
	uint16_t uwXStart;						//!< OSD image item horizontal start point
	uint16_t uwYStart;						//!< OSD image item vertical start point
	uint32_t ulReserved;
}OSD_IMG_INFO;
//------------------------------------------------------------------------------
/**
* @brief OSD Font Input Information
*/
typedef struct
{
	uint8_t 		   ubLangIndex;			//!< OSD font language index
	OSD_FONT_COLOR_TYP tColorIndex;			//!< OSD font color index
	OSD_FONT_RA_TYP    tRotation;			//!< OSD font rotation index
	uint16_t 		   uwXStart;			//!< OSD font horizontal start point
	uint16_t 		   uwYStart;			//!< OSD font vertical start point
}OSD_FONT_INFOR_TYP;
//------------------------------------------------------------------------------
/**
* @brief OSD Font Switch Output Information
*/
typedef struct
{
	uint32_t 	ulFontAddr;					//!< OSD font table address at SF
	uint16_t	uwFontH;					//!< OSD font horizontal size
	uint16_t	uwFontV;					//!< OSD font vertical size
	uint16_t	uwFontMax;					//!< OSD font table max number
}OSD_FONT_SWITCH_OUT_TYP;
//------------------------------------------------------------------------------
/**
* @brief OSD Font Put Information
*/
typedef struct
{
	OSD_FONT_SWITCH_OUT_TYP *pFontInfor;	//!< OSD font table information
	uint16_t				 uwXStart;		//!< OSD font horizontal start point
	uint16_t				 uwYStart;		//!< OSD font vertical start point
	uint16_t				 uwIndex;		//!< OSD font table index
}OSD_FONT_PUT_INFOR_TYP;
//------------------------------------------------------------------------------
typedef struct
{
	uint32_t ulOsdFontSfStartAddr;
	uint32_t ulOsdMenuStringSfStartAddr;
	uint32_t ulOsdImgSfStartAddr;			
}OSD_SFADDR_INFOR_TYP;
//------------------------------------------------------------------------------
/**
* @brief OSD Menu String Information
*/
typedef struct
{
	uint8_t	 ubLangNum;						
	uint8_t	 ubItemNum;
	uint8_t	 ubWordNum;	
}OSD_MENU_STRING_INFOR_TYP;
//------------------------------------------------------------------------------
typedef struct
{
	uint32_t ulOsdPatSft;
	uint8_t  ubOsdImgPatNum;				
	uint8_t  ubOsdFontPatNum;				
}OSD_PAT_INFOR_TYP;
//------------------------------------------------------------------------------
/*!
	\brief 		OSD Logo Jpeg Function (No video or image)
	\param[in]	ulLogo_Index	Index of OSD Logo
	\par [Example]
	\code  
		 LCD_Init(LCD_LCD_PANEL);
		 ulLCD_CalLcdBufSize(&sInfor);
		 LCD_SetLcdBufAddr(0x300000);
		 OSD_LogoJpeg(LOGO_INDEX);
	\endcode
*/
void OSD_LogoJpeg (uint32_t ulLogo_Index);
//------------------------------------------------------------------------------
/*!
	\brief 		OSD Initial Function 
	\param[in]	tOsdWeight	   OSD and LCD channels weight
	\param[in]	ulHSize		   OSD horizontal size
	\param[in]	ulVSize		   OSD vertical size
	\param[in]	uwXStart	   OSD horizontal start point
	\param[in]	uwYStart	   OSD vertical start point
	\param[in]	tXScale		   OSD horizontal scale size
	\param[in]	tYScale		   OSD vertical scale size
	\return		OSD_OK 		   OSD initial OK
	\return		OSD_LOCA_FAIL  OSD location fail
	\return		OSD_HSIZE_FAIL OSD horizontal size fail
	\par [Example]
	\code  
		 LCD_Init(LCD_LCD_PANEL);
		 tOSD_Init(OSD_WEIGHT_6DIV8, uwLCD_GetLcdHoSize(), uwLCD_GetLcdVoSize(), 0, 0, OSD_SCALE_1X, OSD_SCALE_1X);          
	\endcode
*/
OSD_RESULT tOSD_Init (OSD_WEIGHT_TYP tOsdWeight, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwXStart, uint16_t uwYStart, OSD_SCALE_TYP tXScale, OSD_SCALE_TYP tYScale);
//------------------------------------------------------------------------------
/*!
	\brief 		Calculate OSD Total Buffer Size Function
	\param[in]	ulHSize OSD horizontal size
	\param[in]	ulVSize	OSD vertical size
	\return		OSD total buffer size
	\par [Example]
	\code    
		 uint32_t ulBufSz;

		 LCD_Init(LCD_LCD_PANEL);
		 ulBufSz = ulOSD_CalBufSize(uwLCD_GetLcdHoSize(), uwLCD_GetLcdVoSize());
	\endcode
*/
uint32_t ulOSD_CalBufSize (uint16_t uwHSize, uint16_t uwVSize);
//------------------------------------------------------------------------------
/*!
	\brief 		Set OSD Total Buffer Address Function
	\param[in]	ulAddr Total buffer address
	\note		OSD buffer address must be four alignment
	\par [Example]
	\code    
		 uint32_t ulBufSz;

		 LCD_Init(LCD_LCD_PANEL);
		 ulBufSz = ulOSD_CalBufSize(uwLCD_GetLcdHoSize(), uwLCD_GetLcdVoSize());
		 OSD_SetOsdBufAddr(0x200000);
	\endcode
*/
void OSD_SetOsdBufAddr (uint32_t ulAddr);
//------------------------------------------------------------------------------
/*!
	\brief Clear Image 1 Layer Buffer Function
	\note  When change new image palette table, user must clear image 1 layer buffer
*/
void OSD_ClearImg1Buf (void);
//------------------------------------------------------------------------------
/*!
	\brief Clear Image 2 Layer Buffer Function
*/
void OSD_ClearImg2Buf (void);
//------------------------------------------------------------------------------
/*!
	\brief 		Eraser Image 1 OSD Function
	\param[in]	pInfor	Eraser image 1 OSD information
	\par [Example]
	\code
		 OSD_IMG_INFO tInfor;
		 tInfor.uwHSize = 160;
	  	 tInfor.uwVSize = 80;
		 tInfor.uwXStart = 0;
		 tInfor.uwYStart = 0;
		 OSD_EraserImg1(&tInfor);
	\endcode
*/
void OSD_EraserImg1 (OSD_IMG_INFO *pInfor);
//------------------------------------------------------------------------------
/*!
	\brief 		Eraser Image 2 OSD Function
	\param[in]	pInfor	Eraser image 2 OSD information
	\par [Example]
	\code    
		 OSD_IMG_INFO tInfor;
		 tInfor.uwHSize = 80;
	  	 tInfor.uwVSize = 40;
		 tInfor.uwXStart = 10;
		 tInfor.uwYStart = 10;
		 OSD_EraserImg2(&tInfor);
	\endcode
*/
void OSD_EraserImg2 (OSD_IMG_INFO *pInfor);
//------------------------------------------------------------------------------
/*!
	\brief 		Eraser Font OSD Function
	\param[in]	uwHFontNum	Eraser font OSD horizontal size
	\param[in]	uwVFontNum	Eraser font OSD vertical size
	\param[in]	uwXStart	Eraser font OSD horizontal start point
	\param[in]	uwYStart	Eraser font OSD vertical start point
	\par [Example]
	\code    
		 OSD_EraserFont (2, 2, 17, 13);
	\endcode
*/
void OSD_EraserFont (uint16_t uwHFontNum, uint16_t uwVFontNum, uint16_t uwXStart, uint16_t uwYStart);
//------------------------------------------------------------------------------
/*!
	\brief 		Display Image 1 OSD Function
	\param[in]	pInfor		  Display image 1 OSD information
	\param[in]	tMode		  Image 1 OSD updates method
	\return		OSD_OK 		  Display image 1 OSD ok!
	\return		OSD_LOCA_FAIL Display image 1 OSD location fail!
	\par [Example]
	\code
		 OSD_IMG_INFO tInfor;
		 tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_VGASTATUSMASK, 1, &tInfor);
		 tOSD_Img1 (&tInfor, OSD_UPDATE);
	\endcode
*/
OSD_RESULT tOSD_Img1 (OSD_IMG_INFO *pInfor, OSD_UPDATE_TYP tMode);
//------------------------------------------------------------------------------
/*!
	\brief 		Display Image 2 OSD Function
	\param[in]	pInfor		  Display image 2 OSD information
	\param[in]	tMode		  Image 2 OSD updates method
	\return		OSD_OK 		  Display image 2 OSD ok!
	\return		OSD_LOCA_FAIL Display image 2 OSD location fail!
	\par [Example]
	\code
		 OSD_IMG_INFO tInfor;
		 tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOLDECNOR_ICON, 1, &tInfor);
		 tOSD_Img2 (&tInfor, OSD_UPDATE);
	\endcode
*/
OSD_RESULT tOSD_Img2 (OSD_IMG_INFO *pInfor, OSD_UPDATE_TYP tMode);
//------------------------------------------------------------------------------
/*!
	\brief 		Display Font OSD Function
	\param[in]	pInfor		  		 Display OSD font input information
	\param[in]	tMode		  		 OSD font updates method
	\return		OSD_OK 		  		 Display font OSD ok!
	\return		OSD_LOCA_FAIL 		 Display font OSD location fail!
	\return		OSD_FONT_SWITCH_FAIL Search OSD font table fail!
	\par [Example]
	\code    
		 OSD_FONT_INFOR_TYP tInfor;
		 tInfor.ubLangIndex = 1;
		 tInfor.tColorIndex = OSD_FONT_COLOR1;
		 tInfor.tRotation = OSD_FONT_ROTATION_0;
		 tInfor.uwXStart = 10;
		 tInfor.uwYStart = 10;
		 tOSD_FontPrintf (&tInfor, OSD_UPDATE, "%s\n%d\nTaiwan\n", "SONiX", 5471);
	\endcode
*/
OSD_RESULT tOSD_FontPrintf (OSD_FONT_INFOR_TYP *pInfor, OSD_UPDATE_TYP tMode, char* pFmt, ...);
//------------------------------------------------------------------------------
/*!
	\brief 		Update OSD Queue Buffer (Font or Image 1 and Image 2)
	\par [Example]
	\code
		 OSD_IMG_INFO tInfor;
		 OSD_FONT_INFOR_TYP tFontInfor;
		 tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_VGASTATUSMASK, 1, &tInfor);
		 tOSD_Img1 (&tInfor, OSD_QUEUE);
		 tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOLDECNOR_ICON, 1, &tInfor);
		 tOSD_Img2 (&tInfor, OSD_QUEUE);    
		 OSD_UpdateQueueBuf();		 
		 tFontInfor.ubLangIndex = 1;
		 tFontInfor.tColorIndex = OSD_FONT_COLOR1;
		 tFontInfor.tRotation = OSD_FONT_ROTATION_0;
		 tFontInfor.uwXStart = 10;
		 tFontInfor.uwYStart = 10;
		 tOSD_FontPrintf (&tFontInfor, OSD_QUEUE, "%s\n%d\n", "SONiX", 5471);
		 tFontInfor.uwXStart = 200;
		 tFontInfor.uwYStart = 200;
		 tOSD_FontPrintf (&tFontInfor, OSD_QUEUE, "Taiwan\n");
		 OSD_UpdateQueueBuf();
	\endcode
*/
void OSD_UpdateQueueBuf (void);
//------------------------------------------------------------------------------	
/*!
	\brief 	Get OSD Horizontal Size
	\return	OSD horizontal size
	\par [Example]
	\code    	
		  uint16_t uwHSize;
		  
		  uwHSize = uwOSD_GetHSize();
	\endcode
*/
uint16_t uwOSD_GetHSize (void);
//------------------------------------------------------------------------------	
/*!
	\brief 	Get LCD Ouput Vertical Size
	\return	OSD vertical size
	\par [Example]
	\code    	
		  uint16_t uwVSize;
		  
		  uwVSize = uwLCD_GetLcdHoSize();
	\endcode
*/
uint16_t uwOSD_GetVSize (void);
//------------------------------------------------------------------------------
/*!
	\brief 		Set OSD Weight
	\param[in]	tWeight OSD and LCD channels weight type	
	\par [Example]
	\code    
		 OSD_Weight(OSD_WEIGHT_7DIV8);
	\endcode
*/
void OSD_Weight (OSD_WEIGHT_TYP tWeight);
//------------------------------------------------------------------------------
/*!
	\brief 		Get OSD Image Information from SF
	\param[in]	ubGroupNum		  	 Nth group of OSD layer
	\param[in]	tOsdLayer		  	 OSD layer
	\param[in]	uwStartImageIdx		 Staert image index of OSD layer
	\param[in]	uwTotalImage		 Get N number image imformation
	\param[out]	pbOsdImgData		 Get image imformation pointer
	\return		OSD_OK 		  		 Get image imformation ok!
	\return		OSD_LOADIMAGE_FAIL 	 Get image imformation fail!
	\par [Example]
	\code    
		 OSD_IMG_INFO tInfor;
		 tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOLDECNOR_ICON, 1, &tInfor);
		 tOSD_Img2 (&tInfor, OSD_UPDATE);
	\endcode
*/
OSD_RESULT tOSD_GetOsdImgInfor (uint8_t ubGroupNum, OSD_LAYER_TYP tOsdLayer, uint16_t uwStartImageIdx, uint16_t uwTotalImage, void *pbOsdImgData);
//------------------------------------------------------------------------------
/*!
	\brief 		Display Menu String from ConfigGen
	\param[in]	pInfor		 		 OSD font input information pointer
	\param[in]	ubItemIndex	 		 OSD menu string index
	\param[in]	tMode		 		 OSD menu string updates method
	\return		OSD_OK 		  		 Get image imformation ok!
	\return		OSD_LOCA_FAIL 		 Display menu string location fail!
	\return		OSD_FONT_SWITCH_FAIL Search OSD font table fail!
	\par [Example]
	\code    
		 OSD_FONT_INFOR_TYP tInfor;
		 tFontInfor.ubLangIndex = 1;
		 tFontInfor.tColorIndex = OSD_FONT_COLOR1;
		 tFontInfor.tRotation = OSD_FONT_ROTATION_0;
		 tFontInfor.uwXStart = 10;
		 tFontInfor.uwYStart = 10;		 
		 tOSD_MenuString (&tInfor, 2, OSD_UPDATE);
	\endcode
*/
OSD_RESULT tOSD_MenuString (OSD_FONT_INFOR_TYP *pInfor, uint8_t ubItemIndex , OSD_UPDATE_TYP tMode);
//------------------------------------------------------------------------------
/*!
	\brief Enable Display OSD
	\par [Example]
	\code    
		 OSD_Enable();
	\endcode
*/
void OSD_Enable (void);
//------------------------------------------------------------------------------
/*!
	\brief Disable Display OSD
	\par [Example]
	\code    
		 OSD_Disable();
	\endcode
*/
void OSD_Disable (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get OSD Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwOSD_GetVersion();
		 printf("OSD Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwOSD_GetVersion (void);
//------------------------------------------------------------------------------
/*!
	\brief Clear all OSD Buffer for LCD Library
*/
void OSD_SetBufEmpty (void);
//------------------------------------------------------------------------------
/*!
	\brief  Update? OSD Pattern when occurs Vsync for LCD Library
	\return	true  Update OSD pattern when occurs Vsync for LCD Library!
	\return	false No Update OSD pattern!
*/
bool bOSD_UpdateAtVsync (void);
//------------------------------------------------------------------------------
typedef struct
{
	uint16_t *pNumImgIdxArray;			//! OSD image index array of Number.
	uint16_t *pUpperLetterImgIdxArray;	//! OSD image index array of Upper letter.
	uint16_t *pLowerLetterImgIdxArray;	//! OSD image index array of Lower letter.
	uint16_t *pSymbolImgIdxArray;		//! OSD image index array of Symbol(Only ":", "-", ".", " ").
}OSD_IMGIDXARRARY_t;
//------------------------------------------------------------------------------
/*!
	\brief 		Display Font OSD Function
	\param[in]	tRotType		 	 Rotation type
	\param[in]	uwXStart		  	 Display X position
	\param[in]	uwYStart		  	 Display Y position
	\param[in]	tImgIdxArray		 OSD image index array, include number, upper letter, lower letter and symbol(Only ":", "-", ".", " ").
	\param[in]	pFmt		  	 	 String
	\par [Example]
	\code
		uint16_t uwNumArray[10];
		uint16_t uwUpperLetterArray[26];
		uint16_t uwLowerLetterArray[26];
		uint16_t uwSymbolArray[4], i;
		OSD_IMGIDXARRARY_t tImgArray;
		for(i = 0; i < 10; i++)
			uwNumArray[i] = OSD2IMG_ENG_NUM0 + i;
		for(i = 0; i < 26; i++)
		{
			uwUpperLetterArray[i] = OSD2IMG_ENG_UPA + i;
			uwLowerLetterArray[i] = OSD2IMG_ENG_LWA + i;
		}
		for(i = 0; i < 4; i++)
			uwSymbolArray[i] = OSD2IMG_ENG_COLONSYM + i;
		tImgArray.pNumImgIdxArray = uwNumArray;
		tImgArray.pUpperLetterImgIdxArray = uwUpperLetterArray;
		tImgArray.pLowerLetterImgIdxArray = uwLowerLetterArray;
		tImgArray.pSymbolImgIdxArray = uwSymbolArray;
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 100, 150, tImgArray, "Sensitivity: %d dBm", 95);
	\endcode
*/
void OSD_ImagePrintf(OSD_IMG_RA_TYP tRotType, uint16_t uwXStart, uint16_t uwYStart, OSD_IMGIDXARRARY_t tImgIdxArray, OSD_UPDATE_TYP tMode, char* pFmt, ...);

OSD_RESULT tOSD_DataCopy(uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulLen);
#endif
