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
#include "TIMER.h"
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
//------------------------------------------------------------------------------
/*
void LCD_PixelPllSetting(void)
{
	float fPixelClock;
	uint8_t ubFps = 60;
	float fFVCO;

	//! Calculation Pixel Clock
	switch (LCD->LCD_MODE)
	{
		case LCD_AU_UPS051_8:
		case LCD_AU_UPS051_6:
		case LCD_DE:
			fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
								   (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_RGB_DUMMY:
			fPixelClock = ((float)ubFps) * ((LCD->LCD_HO_SIZE << 2) + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
								            (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_AU:
			fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE * 3 + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
											(LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_YUV422:
			fPixelClock = ((float)ubFps) * ((LCD->LCD_HO_SIZE << 1) + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
											(LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_BT656_BT601:
			fPixelClock = ((float)ubFps) * LCD->LCD_VO_SIZE * ((LCD->LCD_HO_SIZE << 1) + 
								           LCD->LCD_HT_START + (LCD->LCD_HS_WIDTH << 1) + 8) / 1000000;
			break;
		default:
			return;
	}
	
	printd(DBG_InfoLvl, "LCD Pixel Clock = %f MHz\n", fPixelClock);					

	LCD->LCD_PCK_SPEED = 2;//2;
	GLB->LCDPLL_CK_SEL = 2;
	//! LCD PLL
	if (!GLB->LCDPLL_PD_N)
	{
		GLB->LCDPLL_LDO_EN = 1;
		GLB->LCDPLL_PD_N = 1;
		TIMER_Delay_us(400);
		GLB->LCDPLL_SDM_EN = 1;
		TIMER_Delay_us(1);
	}	
	GLB->LCDPLL_INT = 12;
	GLB->LCDPLL_FRA = 0xb0c34;
	TIMER_Delay_us(1);
	GLB->LCDPLL_INT_FRA_VLD = 1;
	TIMER_Delay_us(1);	
	//! LCD Controller rate
	GLB->LCD_RATE = 1;

	if (GLB->LCDPLL_CK_SEL == 0)
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 6;
	else if (GLB->LCDPLL_CK_SEL == 1)
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 2;
	else
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 1;
	
	if ((fFVCO < 148.3) || (fFVCO > 165)) { //range:148.3M ~ 165M
		printd(DBG_ErrorLvl, "fFVCO=%f MHz,Change PLL pls!\n", fFVCO);
		while(1);
	}
}
*/
#endif
