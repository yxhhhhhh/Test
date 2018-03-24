/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TV.c
	\brief		LCD TV Funcation
	\author		Pierce
	\version	0.3
	\date		2017/03/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "LCD.h"
//------------------------------------------------------------------------------
#if (LCD_TV_OUT != LCD_NO_TV)
void LCD_TV_Init (uint8_t ubMode, uint8_t ubProg)
{
	//! TV
	printf("TV Output\n");
	printf("Screen Size 640 x 480\n");	
	
	LCD->LCD_MODE = TV;
	LCD->SEL_TV = 1;						
	
	LCD->LCD_HO_SIZE = 640;
	LCD->LCD_VO_SIZE = 480;

	LCD->LCD_HT_DM_SIZE = 0;
	LCD->LCD_VT_DM_SIZE = 0;
	LCD->LCD_HT_START = 0;
	LCD->LCD_VT_START = 0;
	
	switch (ubMode)
	{
		case NTSC:
		case NTSC_443:
			LCD->LCD_HT_START = 0;
			//LCD->LCD_VT_START = 2;
			LCD->TV_W_CLIP = 0x3F;
			LCD->TV_BURST = 0x44;		
			LCD->TV_SETUP  = 0x44;
			LCD->TV_BLANK  = 0x3A;
			LCD->TV_Y_GAIN  = 0x4C;
			LCD->TV_UV_GAIN  = 0x15;
			break;
		case PAL:
			LCD->LCD_VO_SIZE = 576;
			LCD->LCD_HT_START = 0;
			//LCD->LCD_VT_START = 3;
			LCD->TV_W_CLIP = 0x29;
			LCD->TV_BURST = 0x50;		
			LCD->TV_SETUP  = 0x41;
			LCD->TV_BLANK  = 0x37;
			LCD->TV_Y_GAIN  = 0x58;
			LCD->TV_UV_GAIN  = 21;
			break;
		case PAL_M:
			LCD->LCD_HT_START = 2;
			//LCD->LCD_VT_START = 6;
			LCD->TV_W_CLIP = 0x29;
			LCD->TV_BURST = 0x50;		
			LCD->TV_SETUP  = 0x41;
			LCD->TV_BLANK  = 0x37;
			LCD->TV_Y_GAIN  = 0x58;
			LCD->TV_UV_GAIN  = 21;
			break;
	}	
	LCD->TV_INV  = 0;
	LCD->TV_LPF_EN = 1;
	
	LCD->TEST_TV = 0;
	LCD->TV_MODE = ubMode;
	LCD->TV_PROG = ubProg;														//!<0:interlace , 1:progressive
	LCD->TV_SINC_EN = 0;
	LCD->TV_PATTERN = 0;
	LCD->TV_MONO_Display = 0;
	LCD->TV_MONO_Burst = 0;
	
	LCD->TV_FLT_EN = 0;
	LCD->TV_FLT_GAIN = 3;
	LCD->TV_FLT_EDGE = 0;
	LCD->TV_LPF_EN = 1;
	
	LCD->VDO_DAC_EN = 1;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;
}
#endif
