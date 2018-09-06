/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TM023KDH03.c
	\brief		LCD TM023KDH03 Funcation
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
#if ((LCD_PANEL >= LCD_TM023KDH03_8080_8Bit2Byte && LCD_PANEL <= LCD_TM023KDH03_8080_16Bit) || LCD_PANEL == LCD_TEST_PANEL)
void LCD_TM023KDH03_Init(uint8_t ubValue)
{	
	LCD->LCD_REG_RW = 0;	
	TM023KDH03_CSO = 1;
	TM023KDH03_CSIO = 1;
	//! RST
	TM023KDH03_RSTO = 1;
	TM023KDH03_RSTIO = 1;
	TIMER_Delay_ms(1);
	TM023KDH03_RSTO = 0;	
	TIMER_Delay_us(10);
	TM023KDH03_RSTO = 1;	
	TIMER_Delay_ms(120);
	//! CS
	TM023KDH03_CSO = 0;	
	TM023KDH03_CMD(0xB9);
	TM023KDH03_DATA(0xFF);
  	TM023KDH03_DATA(0x93);
  	TM023KDH03_DATA(0x42);
	
	TM023KDH03_CMD(0xB6);
  	TM023KDH03_DATA(0x0A);
  	TM023KDH03_DATA(0xE2);
  	TM023KDH03_DATA(0x1D);
  	TM023KDH03_DATA(0x08);
  
  	TM023KDH03_CMD(0x36);
  	TM023KDH03_DATA(0x08); 	

  	TM023KDH03_CMD(0x3A);
  	TM023KDH03_DATA(ubValue);
	
  	TM023KDH03_CMD(0xC0);
  	TM023KDH03_DATA(0x25);
  	TM023KDH03_DATA(0x0A);
  
  	TM023KDH03_CMD(0xC1);
  	TM023KDH03_DATA(0x01);
  
  	TM023KDH03_CMD(0xC5);
  	TM023KDH03_DATA(0x2F);
  	TM023KDH03_DATA(0x27);
  
  	TM023KDH03_CMD(0xC7);
  	TM023KDH03_DATA(0xD3);
  
  	TM023KDH03_CMD(0xB8);
  	TM023KDH03_DATA(0x04);
  
  	TM023KDH03_CMD(0xE0);
  	TM023KDH03_DATA(0x0F);
  	TM023KDH03_DATA(0x20);
  	TM023KDH03_DATA(0x1D);
  	TM023KDH03_DATA(0x0D);
  	TM023KDH03_DATA(0x10);
  	TM023KDH03_DATA(0x09);
  	TM023KDH03_DATA(0x4C);
  	TM023KDH03_DATA(0x99);
  	TM023KDH03_DATA(0x3C);
  	TM023KDH03_DATA(0x0B);
  	TM023KDH03_DATA(0x14);
  	TM023KDH03_DATA(0x07);
  	TM023KDH03_DATA(0x10);
  	TM023KDH03_DATA(0x09);
  	TM023KDH03_DATA(0x08);
  
  	TM023KDH03_CMD(0xE1);
  	TM023KDH03_DATA(0x08);
  	TM023KDH03_DATA(0x1F);
  	TM023KDH03_DATA(0x22);
  	TM023KDH03_DATA(0x02);
  	TM023KDH03_DATA(0x0F);
  	TM023KDH03_DATA(0x06);
  	TM023KDH03_DATA(0x33);
  	TM023KDH03_DATA(0x66);
  	TM023KDH03_DATA(0x43);
  	TM023KDH03_DATA(0x04);
  	TM023KDH03_DATA(0x0B);
  	TM023KDH03_DATA(0x08);
  	TM023KDH03_DATA(0x2F);
  	TM023KDH03_DATA(0x36);
  	TM023KDH03_DATA(0x0F);
  
  	TM023KDH03_CMD(0xF2);
  	TM023KDH03_DATA(0x00);

	TM023KDH03_CMD(0xB1);
	TM023KDH03_DATA(0x00);
	TM023KDH03_DATA(0x1b);

	TM023KDH03_CMD(0x35);
	TM023KDH03_DATA(0x00);
	TM023KDH03_CMD(0xB8);
	TM023KDH03_DATA(0x0E);			 

	TM023KDH03_CMD(0x11); 									// Exit Sleep
	TIMER_Delay_ms(120);    
	TM023KDH03_CMD(0x11); 									// Exit Sleep
	TIMER_Delay_ms(120); 					                                                                                                                                                                                                                                                                                                                                   
	TM023KDH03_CMD(0x29); 									// Display On
	TM023KDH03_CMD(0x2C);		 	 
}
//------------------------------------------------------------------------------
void LCD_TM023KDH03_CPU(uint8_t ubMode)
{
	//! TM023KDH03
	printf("LCD TM023KDH03\n");
	switch(ubMode)
	{
		case LCD_8080_16:
			printf("8080 16 Bits Mode\n");
			LCD->DITHER_EN = 1;
			break;
		case LCD_8080_8_2BYTE:
			printf("8080 8 Bits 2 Bytes Mode\n");
			LCD->DITHER_EN = 1;
			break;
		case LCD_8080_8_3BYTE:			
			printf("8080 8 Bits 3 Bytes Mode\n");
			LCD->DITHER_EN = 0;
			break;
	}	
	printf("Screen Size 320 x 240\n");	
	LCD->LCD_MODE = ubMode;
	LCD->SEL_TV = 0;		

	LCD->LCD_HO_SIZE = 320;
	LCD->LCD_VO_SIZE = 240;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 1;
	LCD->LCD_VT_DM_SIZE = 1;
	LCD->LCD_HT_START = 40;
	LCD->LCD_VT_START = 2;
	
	LCD->LCD_WL_WIDTH = 1;
	LCD->LCD_WH_WIDTH = 1;
	LCD->LCD_RL_WIDTH = 3;
	LCD->LCD_RH_WIDTH = 3;
			
	LCD->LCD_PCK_SPEED = 12;
	LCD->DITHER_MODE = 1;
	LCD->DITHER_RAND_EN = 0;
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
#endif
