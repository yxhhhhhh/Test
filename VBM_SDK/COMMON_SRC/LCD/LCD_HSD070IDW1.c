/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_GPM1125A0.c
	\brief		LCD GPM1125A0 Funcation
	\author		Pierce
	\version	0.1
	\date		2017/03/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "TIMER.h"
//------------------------------------------------------------------------------
#if ((LCD_PANEL == LCD_HSD070IDW1_24DE) || LCD_PANEL == LCD_TEST_PANEL)
void LCD_HSD070IDW1_Init(void) 
{
	printf("LCD HSD070IDW1\n");
	printf("Screen Size 800 x 480\n");      
	LCD->LCD_MODE = LCD_DE;
	LCD->SEL_TV = 0;
	LCD->LCD_PCK_RIS = 0;

	LCD->LCD_RGB_REVERSE = 0;
	LCD->LCD_EVEN_RGB = 1;
	LCD->LCD_ODD_RGB = 1;

	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;
	LCD->LCD_DE_HIGH = 1;

	LCD->LCD_HS_WIDTH = 40;
	LCD->LCD_VS_WIDTH = 5;
									
	LCD->LCD_PCK_SPEED = 2;

	LCD->LCD_HO_SIZE = 800;
	LCD->LCD_VO_SIZE = 480;     
					//! Timing
	LCD->LCD_HT_DM_SIZE = 88;
	LCD->LCD_VT_DM_SIZE = 32;
	LCD->LCD_HT_START = 80;
	LCD->LCD_VT_START = 13;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;

}
//------------------------------------------------------------------------------
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

	LCD->LCD_PCK_SPEED = 5;
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
	GLB->LCDPLL_FRA = 0xb47ae;
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

#endif
