/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_HDMI_IT66121_API.h
	\brief		LCD HDMI API Header
	\author		Pierce
	\version	0.4
	\date		2017/07/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_HDMI_IT66121_API_H_
#define _LCD_HDMI_IT66121_API_H_
#include <stdint.h>
//------------------------------------------------------------------------------
//#define	HDMI_TESTA
//#define	HDMI_EDID
//#define HDMI_HDCP
//#define HDMI_HDCP_SHA
//#define	HDMI_AUDIO
//------------------------------------------------------------------------------
#define HDMI_480P60 					(0)
#define HDMI_720P60 					(1)
#define HDMI_1080P60 					(2)
#define HDMI_OUTPUT_RES					(HDMI_1080P60)
/*!
	\brief 	HDMI Tx State
*/
typedef enum 
{
    HDMI_PLUG_IN,
    HDMI_OUTPUT,
	HDMI_PLUG_OUT
}HDMI_STATE_TYP;
//------------------------------------------------------------------------------
/*!
	\brief 	HDMI Tx Initial
	\par [Example]
	\code    
		 HDMI_Init();
	\endcode
*/
void HDMI_Init (void);
//------------------------------------------------------------------------------
/*!
	\brief 	LCD Output Timing Initial for HDMI Tx
	\par [Example]
	\code    
		 HDMI_LcdTimingInit();
	\endcode
*/
void HDMI_LcdTimingInit (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Detect HDMI Tx State
	\return	HDMI_PLUG_IN  Detect HDMI Rx plug in
	\return	HDMI_OUTPUT   HDMI Tx output now
	\return	HDMI_PLUG_OUT Detect HDMI Rx plug output
	\par [Example]
	\code
		 HDMI_STATE_TYP tState;
		 tState = tHDMI_HdmiDetect();
	\endcode
*/
HDMI_STATE_TYP tHDMI_HdmiDetect (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get HDMI IT66121 Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwHDMI_IT66121_GetVersion();
		 printf("HDMI IT66121 Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwHDMI_IT66121_GetVersion (void);
#endif
