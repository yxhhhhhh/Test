/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_lcd.c
	\brief		LCD control command line
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "LCD_TYPE.H"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_LCD

static void lcd_ctrl_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: lcd_ctrl <OPT> <Type> <Reg> <Data>\n");
	printf(" OPT:\n");
	printf("	1: LCD register write\n");
	printf(" Type:\n");
	printf("	1: Reg write\n");
	printf("	2: Data write\n");
	printf(" Example: write MIPI driver ic reg 0xB1\n");
	printf("          //lcd_init_B1[]={0xB1,0x6C,0x12,0x12}\n");
	printf("          //LCD_SSD2828_RegWr(0xBC,0x0004)\n");
	printf("          //LCD_SSD2828_RegWr(0xBF,0x6CB1)\n");
	printf("          //LCD_SSD2828_DatWr(0x1212)\n");
	printf("	lcd_ctrl 1 1 0xbc 0x0004\n");
	printf("	lcd_ctrl 1 1 0xbf 0x6cb1\n");
	printf("	lcd_ctrl 1 2 0x1212\n");
	printf("###################################\n");
}  

int32_t cmd_lcd_ctrl(int argc, char* argv[]) {
	uint8_t OPT = 0, Type = 0;
	uint8_t ubReg = 0;
	uint16_t uwData = 0;

	if (argc < 2) {
		lcd_ctrl_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);
	Type = strtoul(argv[2], NULL, 0);

	if (OPT == 1) {
		if (Type == 1) {
			//RegWr
			ubReg = strtoul(argv[3], NULL, 0);
			uwData = strtoul(argv[4], NULL, 0);
			LCD_SSD2828_RegWr(ubReg, uwData);
			printf("===================================\n");
			printf("LCD Reg Wr: 0x%02x, 0x%04x\n", ubReg, uwData);
			printf("===================================\n");
		} else if (Type == 2) {
			//DatWr
			uwData = strtoul(argv[3], NULL, 0);
			LCD_SSD2828_DatWr(uwData);
			printf("===================================\n");
			printf("LCD Data Wr: 0x%04x\n", uwData);
			printf("===================================\n");
		}
	} else {
		lcd_ctrl_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

#endif
