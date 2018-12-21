/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TM035KDH03.c
	\brief		LCD TM035KDH03 Funcation
	\author		Pierce
	\version	0.3
	\date		2017/03/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "TIMER.h"
//------------------------------------------------------------------------------
#if ((LCD_PANEL >= LCD_TM035KDH03_24HV && LCD_PANEL <= LCD_TM035KDH03_BT656) || LCD_PANEL == LCD_TEST_PANEL)
uint16_t uwLCD_TM035KDH03_Read(uint8_t ubAddr)
{
	uint16_t uwValue=0;
	uint8_t  ubi;
	
	TM035KDH03_SDAO = 1;
	TM035KDH03_SCL = 1;
	TM035KDH03_CS = 1;
	TM035KDH03_SDAIO = 1;
	TM035KDH03_SCLIO = 1;
	TM035KDH03_CSIO = 1;	
	TIMER_Delay_us(1);
	
	TM035KDH03_CS = 0;
	for (ubi=0; ubi<LCD_TM035KDH03_ADDR_MAX; ++ubi)
	{
		TM035KDH03_SCL = 0;
		TM035KDH03_SDAO = (ubAddr & (1 << (LCD_TM035KDH03_ADDR_SFT - ubi)))?1:0;		
		TIMER_Delay_us(1);
		TM035KDH03_SCL = 1;
		TIMER_Delay_us(1);
	}
	
	//! Read
	TM035KDH03_SCL = 0;
	TM035KDH03_SDAO = 0;
	TIMER_Delay_us(1);
	TM035KDH03_SCL = 1;
	TIMER_Delay_us(1);
	//! Data input
	TM035KDH03_SCL = 0;
	TM035KDH03_SDAIO = 0;	
	TIMER_Delay_us(1);
	TM035KDH03_SCL = 1;
	TIMER_Delay_us(1);
	
	for (ubi=0; ubi<LCD_TM035KDH03_DATA_MAX; ++ubi)
	{
		TM035KDH03_SCL = 0;
		TIMER_Delay_us(1);
		TM035KDH03_SCL = 1;
		if (TM035KDH03_SDAI)
			uwValue |= (1 << (LCD_TM035KDH03_DATA_SFT - ubi));
		TIMER_Delay_us(1);
	}
	TIMER_Delay_us(1);
	TM035KDH03_CS = 1;
	return uwValue;
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_Rw(uint16_t uwSetting, uint8_t ubAns)
{
	uint8_t ubRdData;
	uint8_t  ubAddr, ubi;
	
	TM035KDH03_SDAO = 1;
	TM035KDH03_SCL = 1;
	TM035KDH03_CS = 1;
	TM035KDH03_SDAIO = 1;
	TM035KDH03_SCLIO = 1;
	TM035KDH03_CSIO = 1;	
	TIMER_Delay_us(1);
	
	TM035KDH03_CS = 0;
	for (ubi=0; ubi<LCD_TM035KDH03_SETTING_MAX; ++ubi)
	{
		TM035KDH03_SCL = 0;
		TM035KDH03_SDAO = (uwSetting & (1 << (LCD_TM035KDH03_SETTING_SFT - ubi)))?1:0;
		TIMER_Delay_us(1);
		TM035KDH03_SCL = 1;
		TIMER_Delay_us(1);
	}
	TIMER_Delay_us(1);
	TM035KDH03_CS = 1;		
	if (ubAns !=  (ubRdData = uwLCD_TM035KDH03_Read(ubAddr = (uwSetting >> LCD_TM035KDH03_ADDR_SB))))
	{
		printf ("Write TM035KDH03 R%d Fail\n", ubAddr);
		printf ("Write Data = 0x%X\n", ubAns);
		printf ("Read Data = 0x%X\n", ubRdData);
		return false;
	}
	return true;
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_Write(uint16_t uwSetting)
{
	return bLCD_TM035KDH03_Rw(uwSetting, uwSetting & LCD_TM035KDH03_DATA_MASK);	
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_WriteValue(uint16_t uwSetting, uint8_t ubValue)
{
	return bLCD_TM035KDH03_Rw(uwSetting, ubValue);	
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_24RgbHv (void)
{
	//! TM035KDH03
	printf("LCD TM035KDH03\n");
	printf("24 HV Mode\n");
	printf("Screen Size 320 x 240\n");			
	LCD->LCD_MODE = LCD_DE;
	LCD->SEL_TV = 0;			
	if (false == bLCD_TM035KDH03_WriteValue(0x202, 7)) return false;
	if (false == bLCD_TM035KDH03_Write(0xECC))	  return false;	
	if (false == bLCD_TM035KDH03_Write(0x2644))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3210))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3610))	  return false;

	LCD->LCD_HO_SIZE = 320;
	LCD->LCD_VO_SIZE = 240;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 27;
	LCD->LCD_VT_DM_SIZE = 17;
	LCD->LCD_HT_START = 69;
	LCD->LCD_VT_START = 13;
	
	LCD->LCD_HS_WIDTH = 1;
	LCD->LCD_VS_WIDTH = 1;
			
	//LCD->LCD_PCK_SPEED = 24;
	LCD->LCD_PCK_SPEED = 0xB0;//Justin 2016.11.15
	LCD->LCD_RGB_REVERSE = 0;
	LCD->LCD_EVEN_RGB = 0;
	LCD->LCD_ODD_RGB = 0;
	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;
	LCD->LCD_DE_HIGH = 0;
	LCD->LCD_PCK_RIS = 1;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;
	return true;
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_24RgbDe (void)
{
	//! TM035KDH03
	printf("LCD TM035KDH03\n");
	printf("24 Den Mode\n");
	printf("Screen Size 320 x 240\n");			
	LCD->LCD_MODE = LCD_DE;
	LCD->SEL_TV = 0;								
	if (false == bLCD_TM035KDH03_WriteValue(0x202, 7)) return false;
	if (false == bLCD_TM035KDH03_Write(0xECD))	  return false;	
	if (false == bLCD_TM035KDH03_Write(0x2644))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3210))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3610))	  return false;	

	LCD->LCD_HO_SIZE = 320;
	LCD->LCD_VO_SIZE = 240;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 27;
	LCD->LCD_VT_DM_SIZE = 17;
	LCD->LCD_HT_START = 69;
	LCD->LCD_VT_START = 13;
	
	LCD->LCD_HS_WIDTH = 1;
	LCD->LCD_VS_WIDTH = 1;
			
	LCD->LCD_PCK_SPEED = 24;
	LCD->LCD_RGB_REVERSE = 0;
	LCD->LCD_EVEN_RGB = 0;
	LCD->LCD_ODD_RGB = 0;
	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;
	LCD->LCD_DE_HIGH = 1;
	LCD->LCD_PCK_RIS = 1;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;
	return true;
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_8RgbHv (void)
{
	//! TM035KDH03
	printf("LCD TM035KDH03\n");
	printf("8 HV Mode\n");
	printf("Screen Size 320 x 240\n");						
	LCD->LCD_MODE = LCD_AU;
	LCD->SEL_TV = 0;									
	if (false == bLCD_TM035KDH03_WriteValue(0x202, 7)) return false;
	if (false == bLCD_TM035KDH03_Write(0xEC8))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x2644))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3210))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3610))	  return false;

	LCD->LCD_HO_SIZE = 320;
	LCD->LCD_VO_SIZE = 240;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 13;
	LCD->LCD_VT_DM_SIZE = 6;
	LCD->LCD_HT_START = 69;
	LCD->LCD_VT_START = 13;
	
	LCD->LCD_HS_WIDTH = 1;
	LCD->LCD_VS_WIDTH = 1;
			
	LCD->LCD_PCK_SPEED = 10;
	LCD->LCD_RGB_REVERSE = 0;
	LCD->LCD_EVEN_RGB = 2;
	LCD->LCD_ODD_RGB = 2;
	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;	
	LCD->LCD_PCK_RIS = 1;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;
	return true;
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_IturBt601 (void)
{
	//! TM035KDH03
	printf("LCD TM035KDH03\n");
	printf("BT601 Mode\n");
	printf("Screen Size 720 x 480\n");						
	LCD->LCD_MODE = LCD_BT656_BT601;
	LCD->SEL_TV = 0;						
	if (false == bLCD_TM035KDH03_WriteValue(0x202, 7)) return false;
	if (false == bLCD_TM035KDH03_Write(0xEC2))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x2644))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3210))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3610))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x2E01))	  return false;
	//! BT Mode
	LCD->BT_MODE = 0;
	LCD->BT_PROG = 1;
	LCD->BT_VS1_ST = 0;
	LCD->BT_VS1_END = 13;
	LCD->BT_VS2_ST = 0;
	LCD->BT_VS2_END = 13;
	LCD->BT_FLD1 = 3;
	LCD->BT_FLD2 = 3;		

	LCD->LCD_HO_SIZE = 720;
	LCD->LCD_VO_SIZE = 480;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 0;
	LCD->LCD_VT_DM_SIZE = 0;
	LCD->LCD_HT_START = 234;
	LCD->LCD_VT_START = 0;
	
	LCD->LCD_HS_WIDTH = 1;
	LCD->LCD_VS_WIDTH = 1;
			
	LCD->LCD_PCK_SPEED = 4;
	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;			
	LCD->LCD_PCK_RIS = 1;
	LCD->LCD_FS0_FIELD_MODE = 1;
	LCD->LCD_FS1_FIELD_MODE = 1;
	return true;
}
//------------------------------------------------------------------------------
bool bLCD_TM035KDH03_IturBt656 (void)
{
	//! TM035KDH03
	printf("LCD TM035KDH03\n");
	printf("BT656 Mode\n");
	printf("Screen Size 720 x 480\n");						
	LCD->LCD_MODE = LCD_BT656_BT601;
	LCD->SEL_TV = 0;						
	if (false == bLCD_TM035KDH03_WriteValue(0x202, 7)) return false;
	if (false == bLCD_TM035KDH03_Write(0xEC4))	  return false;	
	if (false == bLCD_TM035KDH03_Write(0x2644))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3210))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x3610))	  return false;
	if (false == bLCD_TM035KDH03_Write(0x2E01))	  return false;
	//! BT Mode
	LCD->BT_MODE = 2;
	LCD->BT_PROG = 1;
	LCD->BT_VS1_ST = 0;
	LCD->BT_VS1_END = 19;
	LCD->BT_VS2_ST = 0;
	LCD->BT_VS2_END = 19;
	LCD->BT_FLD1 = 3;
	LCD->BT_FLD2 = 3;		

	LCD->LCD_HO_SIZE = 720;
	LCD->LCD_VO_SIZE = 480;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 0;
	LCD->LCD_VT_DM_SIZE = 0;
	LCD->LCD_HT_START = 0;
	LCD->LCD_VT_START = 0;
	
	LCD->LCD_HS_WIDTH = 134;
	LCD->LCD_VS_WIDTH = 0;
			
	LCD->LCD_PCK_SPEED = 4;		
	LCD->LCD_PCK_RIS = 1;
	LCD->LCD_FS0_FIELD_MODE = 1;
	LCD->LCD_FS1_FIELD_MODE = 1;
	return true;
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
