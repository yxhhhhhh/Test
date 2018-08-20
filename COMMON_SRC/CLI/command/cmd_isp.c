/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_isp.c
	\brief		ISP control command line
	\author		Ocean
	\version	0.2
	\date		2017/11/31
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "IQ_API.h"
#include "IQ_PARSER_API.h"
#include "AE_API.h"
#include "AWB_API.h"
#include "AF_API.h"
#include "APP_HS.h"
#include "WDT.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_ISP

static void isp_ctrl_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: isp_ctrl <OPT> <Reg/Mode> <Value>\n");
	printf(" OPT:\n");
	printf("	0: ISP XDATA Register Read Operation\n");
	printf("	1: ISP XDATA Register Write Operation\n");
	printf("	2: ISP tuning mode read / write\n");
	printf("	3: ISP Get Alg Version\n");
	printf(" Example: write ISP xdata 0x2004, value 1\n");
	printf("	isp_ctrl 1 0x2004 1\n");
	printf(" Example: read ISP xdata 0x2004\n");
	printf("	isp_ctrl 0 0x2004\n");
	printf(" Example: set ISP tuning mode on\n");
	printf("	isp_ctrl 2 1 1\n");
	printf(" Example: get ISP tuning mode\n");
	printf("	isp_ctrl 2 0\n");
	printf(" Example: get Alg version\n");
	printf("	isp_ctrl 3\n");
	printf("###################################\n");
}  

int32_t cmd_isp_ctrl(int argc, char* argv[])
{
	uint8_t opt = 0;
	uint32_t addr = 0, value = 0;
	uint8_t mode = 0;

	if ((argc < 2) || (argc > 4)) {
		isp_ctrl_usage();
		return cliFAIL; 
	}
	
	opt = strtoul(argv[1], NULL, 0);

	if (opt == 0) {
		//xdata read
		addr = strtoul(argv[2], NULL, 0);
		
		if (addr == 0x2004) {				
			value = ulIQ_GetIspFuncFlag();
		} else if (addr == 0x2008) {
			value = ubIQ_GetDynFrameRate();
		} else if (addr == 0x200C) {
			value = ulIQ_GetSuppressGain();
		} else if (addr == 0x201C) {
			value = ulIQ_GetMclk();
		} else if (addr == 0x2020) {
			value = ulIQ_GetHStart();
		} else if (addr == 0x2024) {
			value = ulIQ_GetVStart();
		} else if (addr == 0x2028) {
			value = ulIQ_GetHSize();
		} else if (addr == 0x202C) {
			value = ulIQ_GetVSize();
		} else if (addr == 0x2030) {
			value = ulIQ_GetIspReorderPattern();
		//} else if (addr == 0x2038) {
			//value = 0x10000;		//IQ version v1.0
		} else if ((addr >= 0x203C) && (addr <= 0x205C)) {
			value = ulIQ_GetNrTurningVal(addr, 0xffff);
		} else if (addr == 0x2060) {
			//value = (UINT32)ubH264_EncQp;
		} else {
			value = 0;
			printf("R XDATA unknow 0x%x\r\n", addr);	
			return cliFAIL; 
		}
		printf("===================================\n");
		printf("ISP xdata read\n");
		printf("Reg[0x%04X]=0x%02X\n", addr, value);
		printf("===================================\n");
	} else if (opt == 1) {
		//xdata write
		addr = strtoul(argv[2], NULL, 0);
		value = strtoul(argv[3], NULL, 0);
		
		if (addr == 0x2004) {
			if(value & 0x01)
			{
				IQ_DynamicStart();
			}
			else
			{
				IQ_DynamicStop();
			}
			if(value & 0x02)
			{	 
				AF_Start();
			}
			else
			{
				AF_Stop();
			}
			if(value & 0x04)
			{	 
				AE_Start();
			}
			else
			{
				AE_Stop();
			}
			if(value & 0x08)
			{	 
				AWB_Start();	
			}
			else
			{	 
				AWB_Stop();
			}
			IQ_SetIspFuncFlag(value);
		} else if (addr == 0x2008) {
			IQ_SetDynFrameRate((uint8_t)value);		
		} else if (addr == 0x200C) {
			IQ_SetSuppressGain(value);		
		} else if (addr == 0x201C) {
			IQ_SetMclk(value);
		} else if (addr == 0x2020) {
			IQ_SetHStart(value);
		} else if (addr == 0x2024) {
			IQ_SetVStart(value);
		} else if (addr == 0x2028) {
			IQ_SetHSize(value);
		} else if (addr == 0x202C) {
			IQ_SetVSize(value);
		} else if (addr == 0x2030) {
			IQ_SetIspReorderPattern(value);
		} else if ((addr >= 0x203C) && (addr <= 0x205C)) {
			ubIQ_SetNrTurningVal(addr, value);
		} else if (addr == 0x2060) {
			//APP_H264SetQp((uint8_t)value);			
		} else {
			printf("W XDATA unknow 0x%x\r\n", addr);	
			return cliFAIL; 
		}
		printf("===================================\n");
		printf("ISP xdata write\n");
		printf("Reg[0x%04X]=0x%02X\n", addr, value);
		printf("===================================\n");
	} else if (opt == 2) {		
		//ISP tuning mode switch
		mode = strtoul(argv[2], NULL, 0);
		value = strtoul(argv[3], NULL, 0);

		if (mode == 1) {
			printf("===================================\n");
			if (value == 1) {				
				printf("Set ISP tuning mode on\n");
			} else {				
				printf("Set ISP tuning mode off\n");
			}
			APP_SetTuningToolMode((APP_TuningMode_t)value);
			printf("Reboot ...\n");
			printf("===================================\n");
			WDT_Disable(WDT_RST);
			WDT_RST_Enable(WDT_CLK_EXTCLK, 1);
			while(1);
		} else if (mode == 0) {
			printf("===================================\n");
			printf("Get ISP tuning mode:%d\n", APP_GetTuningToolMode());
			printf("===================================\n");
		} else {
			isp_ctrl_usage();
			return cliFAIL; 
		}
	} else if (opt == 3) {		
		//Get Alg Version
		printf("===================================\n");
		printf( "AE Alg Ver=%s\n" ,pbAE_GetAlgVerID());    
		printf( "AWB Alg Ver=%s\n" ,pbAWB_GetAlgVerID());    
		printf("===================================\n");
	} else {
		isp_ctrl_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

#endif
