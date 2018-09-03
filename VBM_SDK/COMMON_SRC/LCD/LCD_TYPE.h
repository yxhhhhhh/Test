/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TYPE.H
	\brief		LCD TYPE Header
	\author		Pierce
	\version	1
	\date		2016/11/23
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TYPE_H_
#define _LCD_TYPE_H_
//------------------------------------------------------------------------------
/**
* @brief LCD Panel Type (HW)
*/
#define	LCD_GPM1125A0_AU_UPS051	 		  (0)	//!< LCD GPM1125A0 AU UPS051 8-bit
#define LCD_GPM1125A0_RGB_DUMMY	 		  (1)	//!< LCD GPM1125A0 RGB Dummy
#define	LCD_GPM1125A0_BT601		 		  (2)	//!< LCD GPM1125A0 BT601
#define	LCD_GPM1125A0_BT656		 		  (3)	//!< LCD GPM1125A0 BT656
#define LCD_TM035KDH03_24HV		 		  (4)	//!< LCD TM035KDH03 RGB 24-bit
#define LCD_TM035KDH03_24DE      		  (5)	//!< LCD TM035KDH03 RGB 24-bit DE
#define LCD_TM035KDH03_8HV		 		  (6)	//!< LCD TM035KDH03 1 Pixel 3 Data
#define LCD_TM035KDH03_BT601	 		  (7)	//!< LCD TM035KDH03 BT601
#define LCD_TM035KDH03_BT656	 		  (8)	//!< LCD TM035KDH03 BT656
#define LCD_SN9C271A_YUV422		 		  (9)	//!< LCD SN9C271A YUV 422
#define LCD_SSD2828_					  (10)	//!< LCD SSD2828
#define LCD_SSD2828_Y50019N00N			  (11)	//!< LCD SSD2828
#define LCD_TM024HDH03_8080_8Bit2Byte 	 (19)	//!< LCD TM024HDH03 8080 Series 8-Bit 2-Byte
#define LCD_TM024HDH03_8080_8Bit3Byte	 (20)	//!< LCD TM024HDH03 8080 Series 8-Bit 3-Byte
#define LCD_TM024HDH03_8080_9Bit		 (21)	//!< LCD TM024HDH03 8080 Series 9-Bit 2-Byte
#define	LCD_TM023KDH03_8080_8Bit2Byte	 (22)	//!< LCD TM023KDH03 8080 Series 8-Bit 2-Byte
#define LCD_TM023KDH03_8080_8Bit3Byte	 (23)	//!< LCD TM023KDH03 8080 Series 8-Bit 3-Byte
#define	LCD_TM023KDH03_8080_16Bit		 (24)	//!< LCD TM023KDH03 8080 Series 16-Bit
#define LCD_HSD070IDW1_24DE				 (25)	//!< LCD_HSD070IDW1 RGB 24-bit DE

//! Add New LCD Panel Type	

#define LCD_TEST_PANEL 				    (254)	//!< For Test Code
#define LCD_NO_PANEL 					(255)	//!< HW setting no LCD panel
//------------------------------------------------------------------------------
/**
* @brief LCD TV Type
*/
#define LCD_TV_NTSC_P		 (11)	//!< TV NTSC Progressive
#define LCD_TV_NTSC_I		 (12)	//!< TV NTSC Interlace	
#define LCD_TV_NTSC443_P	 (13)	//!< TV NTSC 443 Progressive	
#define LCD_TV_NTSC443_I	 (14)	//!< TV NTSC 443 Interlace	
#define LCD_TV_PAL_P		 (15)	//!< TV PAL Progressive
#define LCD_TV_PAL_I		 (16)	//!< TV PAL Interlace	
#define LCD_TV_PALM_P		 (17)	//!< TV PAL M Progressive
#define	LCD_TV_PALM_I		 (18)	//!< TV PAL M Interlace
#define LCD_NO_TV			(255)   //!< HW no TV out
//------------------------------------------------------------------------------
/**
* @brief LCD HDMI Type (HW)
*/
#define LCD_HDMI_IT66121	(200)	//!< HDMI TX IT66121
#define LCD_NO_HDMI			(255)	//!< HW no HDMI out
//------------------------------------------------------------------------------
//#define LCD_PANEL		(LCD_TM035KDH03_24DE)
#define LCD_PANEL		(LCD_SSD2828_Y50019N00N)

#define LCD_TV_OUT		(LCD_TV_NTSC_P)
#define LCD_HDMI_OUT	(LCD_HDMI_IT66121)
//------------------------------------------------------------------------------
#if (LCD_PANEL <= LCD_GPM1125A0_BT656 || LCD_PANEL == LCD_TEST_PANEL)
#include "LCD_GPM1125A0.h"
#endif
#if ((LCD_PANEL >= LCD_TM035KDH03_24HV && LCD_PANEL <= LCD_TM035KDH03_BT656) || LCD_PANEL == LCD_TEST_PANEL)
#include "LCD_TM035KDH03.h"
#endif
#if (LCD_PANEL == LCD_SN9C271A_YUV422 || LCD_PANEL == LCD_TEST_PANEL)
#include "LCD_SN9C271A.h"
#endif
#if (LCD_PANEL == LCD_SSD2828_ || LCD_PANEL == LCD_SSD2828_Y50019N00N || LCD_PANEL == LCD_TEST_PANEL)
#include "LCD_SSD2828.h"
#endif
#if ((LCD_PANEL >= LCD_TM024HDH03_8080_8Bit2Byte && LCD_PANEL <= LCD_TM024HDH03_8080_9Bit) || LCD_PANEL == LCD_TEST_PANEL)
#include "LCD_TM024HDH03.h"
#endif
#if ((LCD_PANEL >= LCD_TM023KDH03_8080_8Bit2Byte && LCD_PANEL <= LCD_TM023KDH03_8080_16Bit) || LCD_PANEL == LCD_TEST_PANEL)
#include "LCD_TM023KDH03.h"
#endif
#if (LCD_TV_OUT != LCD_NO_TV)
#include "LCD_TV.h"
#endif
#if (LCD_HDMI_OUT == LCD_HDMI_IT66121)
#include "LCD_HDMI_IT66121_API.h"
#endif
#if (LCD_PANEL == LCD_HSD070IDW1_24DE)
#include "LCD_HSD070IDW1.h"
#endif
#endif
