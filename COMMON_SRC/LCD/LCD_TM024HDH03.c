/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TM024HDH03.c
	\brief		LCD TM024HDH03 Funcation
	\author		Pierce
	\version	0.1
	\date		2017/03/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "TIMER.h"
//------------------------------------------------------------------------------
#if ((LCD_PANEL >= LCD_TM024HDH03_8080_8Bit2Byte && LCD_PANEL <= LCD_TM024HDH03_8080_9Bit) || LCD_PANEL == LCD_TEST_PANEL)
void LCD_TM024HDH03_Init(uint8_t ubValue, uint8_t ubSft)
{	
	printf("LCD TM024HDH03 Initial\n");
	LCD->LCD_REG_RW = 0;
	TM024HDH03_CSO = 1;
	TM024HDH03_CSIO = 1;
	TM024HDH03_RSTO = 1;
	TM024HDH03_RSTIO = 1;
	TIMER_Delay_ms(1);
	TM024HDH03_RSTO = 0;
	TIMER_Delay_us(10);
	TM024HDH03_RSTO = 1;
	TIMER_Delay_ms(120);
	
	TM024HDH03_CSO = 0;	
	TIMER_Delay_us(1);
	
	//! Set Page 0
	TM024HDH03_WrReg(0xFF << ubSft, 0x00);
	//! ----- Driving ability Setting ----- //
	TM024HDH03_WrReg(0xEA << ubSft, 0x00);
	TM024HDH03_WrReg(0xEB << ubSft, 0x20 << ubSft);
	TM024HDH03_WrReg(0xEC << ubSft, 0x3C << ubSft);
	TM024HDH03_WrReg(0xED << ubSft, 0xC4 << ubSft);
	TM024HDH03_WrReg(0xE8 << ubSft, 0x48 << ubSft);
	TM024HDH03_WrReg(0xE9 << ubSft, 0x38 << ubSft);
	
	//! ----- Gamma 2.2 Setting ----- //
	TM024HDH03_WrReg(0x40 << ubSft, 0x01 << ubSft);
	TM024HDH03_WrReg(0x41 << ubSft, 0x07 << ubSft);
	TM024HDH03_WrReg(0x42 << ubSft, 0x09 << ubSft);
	TM024HDH03_WrReg(0x43 << ubSft, 0x18 << ubSft);
	TM024HDH03_WrReg(0x44 << ubSft, 0x15 << ubSft);
	TM024HDH03_WrReg(0x45 << ubSft, 0x2F << ubSft);
	TM024HDH03_WrReg(0x46 << ubSft, 0x13 << ubSft);
	TM024HDH03_WrReg(0x47 << ubSft, 0x62 << ubSft);
	TM024HDH03_WrReg(0x48 << ubSft, 0x04 << ubSft);
	TM024HDH03_WrReg(0x49 << ubSft, 0x15 << ubSft);
	TM024HDH03_WrReg(0x4A << ubSft, 0x19 << ubSft);
	TM024HDH03_WrReg(0x4B << ubSft, 0x19 << ubSft);
	TM024HDH03_WrReg(0x4C << ubSft, 0x18 << ubSft);

	TM024HDH03_WrReg(0x50 << ubSft, 0x10 << ubSft);
	TM024HDH03_WrReg(0x51 << ubSft, 0x2A << ubSft);
	TM024HDH03_WrReg(0x52 << ubSft, 0x27 << ubSft);
	TM024HDH03_WrReg(0x53 << ubSft, 0x36 << ubSft);
	TM024HDH03_WrReg(0x54 << ubSft, 0x38 << ubSft);
	TM024HDH03_WrReg(0x55 << ubSft, 0x3E << ubSft);
	TM024HDH03_WrReg(0x56 << ubSft, 0x1D << ubSft);
	TM024HDH03_WrReg(0x57 << ubSft, 0x6C << ubSft);
	TM024HDH03_WrReg(0x58 << ubSft, 0x07 << ubSft);
	TM024HDH03_WrReg(0x59 << ubSft, 0x06 << ubSft);
	TM024HDH03_WrReg(0x5A << ubSft, 0x06 << ubSft);
	TM024HDH03_WrReg(0x5B << ubSft, 0x0A << ubSft);
	TM024HDH03_WrReg(0x5C << ubSft, 0x1B << ubSft);
	TM024HDH03_WrReg(0x5D << ubSft, 0xCC << ubSft);	
	
	//! ----- VCOM offset -----//
	TM024HDH03_WrReg(0x24 << ubSft, 0x70 << ubSft);
    TM024HDH03_WrReg(0x25 << ubSft, 0x58 << ubSft);
	TM024HDH03_WrReg(0x23 << ubSft, 0x6E << ubSft);
	// ----- Power on Setting ----- //
    TM024HDH03_WrReg(0x18 << ubSft, 0x36 << ubSft);
	TM024HDH03_WrReg(0x19 << ubSft, 0x01 << ubSft);
    TM024HDH03_WrReg(0x01 << ubSft, 0x00);
	TM024HDH03_WrReg(0x1F << ubSft, 0x88 << ubSft);
	TIMER_Delay_us(1);	
	TM024HDH03_WrReg(0x1F << ubSft, 0x80 << ubSft);
	TIMER_Delay_us(1);
	TM024HDH03_WrReg(0x1F << ubSft, 0x90 << ubSft);
	TIMER_Delay_us(1);
	TM024HDH03_WrReg(0x1F << ubSft, 0xD0 << ubSft);
	//! ----- 262k/65k color selection -----//
	TM024HDH03_WrReg(0x17 << ubSft, ubValue << ubSft);		//!< CPU-n Mode
	
	//! ----- SET PANEL ----- // 
   	TM024HDH03_WrReg(0x36 << ubSft, 0x09 << ubSft);
	//! ----- Display ON Setting ----- //
	TM024HDH03_WrReg(0x26 << ubSft, 0x0E << ubSft);		//!< frame rate
	TM024HDH03_WrReg(0x27 << ubSft, 0x00);
   	TM024HDH03_WrReg(0x28 << ubSft, 0x38 << ubSft);
	TIMER_Delay_ms(40);
	TM024HDH03_WrReg(0x28 << ubSft, 0x3C << ubSft);		//!< Display on
	
	TM024HDH03_WrReg(0x29 << ubSft, 0x00);					
	TM024HDH03_WrReg(0x2B << ubSft, 0x04 << ubSft);					
	TM024HDH03_WrReg(0x2C << ubSft, 0x04 << ubSft);	

	TM024HDH03_Cmd(0x22 << ubSft);
}
//------------------------------------------------------------------------------
void LCD_TM024HDH03_CPU8(uint8_t ubMode)
{	
	//! TM024HDH03
	printf("LCD TM024HDH03\n");
	switch(ubMode)
	{
		case LCD_8080_9:
			printf("8080 9 Bits Mode\n");
			LCD->DITHER_EN = 1;
			LCD->DITHER_MODE = 0;
			break;
		case LCD_8080_8_2BYTE:
			printf("8080 8 Bits 2 Bytes Mode\n");
			LCD->DITHER_EN = 1;
			LCD->DITHER_MODE = 1;
			break;
		case LCD_8080_8_3BYTE:			
			printf("8080 8 Bits 3 Bytes Mode\n");
			LCD->DITHER_EN = 0;
			break;
	}
	printf("Screen Size 240 x 320\n");
	LCD->LCD_MODE = ubMode;
	LCD->SEL_TV = 0;		
	
	LCD->LCD_HO_SIZE = 240;
	LCD->LCD_VO_SIZE = 320;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 8;
	LCD->LCD_VT_DM_SIZE = 4;
	LCD->LCD_HT_START = 4;
	LCD->LCD_VT_START = 3;
	
	LCD->LCD_WL_WIDTH = 1;
	LCD->LCD_WH_WIDTH = 1;
	LCD->LCD_RL_WIDTH = 3;
	LCD->LCD_RH_WIDTH = 3;
			
	LCD->LCD_PCK_SPEED = 5;
	LCD->DITHER_RAND_EN = 1;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;
}
#endif
