/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_SN9C271A.c
	\brief		LCD SN9C271A Funcation
	\author		Pierce
	\version	0.1
	\date		2017/03/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "LCD.h"
//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_SN9C271A_YUV422 || LCD_PANEL == LCD_TEST_PANEL)
void LCD_SN9C271A_Yuv422 (void)
{
	printf("LCD UVC SU5L271F\n");
	printf("YUV422 Sensor Interface Mode\n");
	printf("Screen Size 1920 x 1080\n");	
	LCD->LCD_MODE = LCD_YUV422;
	LCD->SEL_TV = 0;		

	LCD->LCD_HO_SIZE = 1920;
	LCD->LCD_VO_SIZE = 1080;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 402;
	LCD->LCD_VT_DM_SIZE = 0;
	LCD->LCD_HT_START = 2;
	LCD->LCD_VT_START = 10;
	
	LCD->LCD_HS_WIDTH = 1;
	LCD->LCD_VS_WIDTH = 1;
			
	LCD->LCD_PCK_SPEED = 6;	
	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;	
	LCD->LCD_PCK_RIS = 0;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;
}
#endif
