/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_sf.c
	\brief		SF control command line
	\author		Ocean
	\version	0.1
	\date		2017/10/13
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "cmd.h"
#include "CLI.h"
#include "SF_API.h"
#include "APP_CFG.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_SF

static void sf_info_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : sf_info <mode>\n");
	printf(" mode:\n");
	printf("	1: show current SF message\n");
	printf("	2: show SF UI start address\n");
	printf("	3: show SF Kernel start address\n");
	printf("	4: show SF Pair start address\n");
	printf(" Example: \n");
	printf("	sf_info 1\n");
	printf("###################################\n");
}

int32_t cmd_sf_info(int argc, char* argv[])
{
	uint8_t mode = 0;
	uint32_t ulSFAddr = 0;

	if (argc < 2) {
		sf_info_usage();
		return cliFAIL; 
	}
	
	mode = strtoul(argv[1], NULL, 0);

	if (mode == 1) {
		SF_ShowInfo();
	} else if (mode == 2) {
		ulSFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
		printf("===================================\n");
		printf("SF UI Start Addr=0x%x\n", ulSFAddr);
		printf("===================================\n");
	} else if (mode == 3) {
		ulSFAddr = pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);
		printf("===================================\n");
		printf("SF KNL Start Addr=0x%x\n", ulSFAddr);
		printf("===================================\n");
	} else if (mode == 4) {
		ulSFAddr = pSF_Info->ulSize - (PAIR_SF_START_SECTOR * pSF_Info->ulSecSize);
		printf("===================================\n");
		printf("SF Pair Start Addr=0x%x\n", ulSFAddr);
		printf("===================================\n");
	} else {
		sf_info_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

static void sf_ctrl_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : sf_ctrl <OPT> <addr> <offset> <length> <data 0>.... <data N>\n");
	printf(" OPT:\n");
	printf("	1: SF Sector erase\n");
	printf("	2: SF Read\n");
	printf("	3: SF Write\n");
	printf("	4: SF Chip Erase\n");
	printf(" addr: SF address\n");
	printf(" offset: SF buffer offset\n");
	printf(" length: data length\n");
	printf(" data 0~N: wtite data\n");
	printf(" Example 1: erase sector, address = 0x1000\n");
	printf("	sf_ctrl 1 0x1000\n");
	printf(" Example 2: read flash, address = 0x1100, length = 5\n");
	printf("	sf_ctrl 2 0x1000 0x100 5\n");
	printf(" Example 3: write flash, address = 0x1100, length = 5\n");
	printf("	sf_ctrl 3 0x1000 0x100 5 0x11 0x22 0x33 0x44 0x55\n");
	printf(" Example 4: SF chip erase\n");
	printf("	sf_ctrl 4\n");
	printf("###################################\n");
}  

int32_t cmd_sf_ctrl(int argc, char* argv[])
{
	uint8_t opt = 0;
	uint32_t addr = 0, offset = 0, length = 0;
	uint8_t ubBuf[4096];
	uint16_t uwTemp = 0;
	uint32_t ulSectorAddr = 0;

	if (argc < 2) {
		sf_ctrl_usage();
		return cliFAIL; 
	}
	
	opt = strtoul(argv[1], NULL, 0);
	addr = strtoul(argv[2], NULL, 0);
	
	if (opt == 1) {
		SF_DisableWrProtect();
		SF_Erase(SF_SE, addr, pSF_Info->ulSecSize);
		SF_EnableWrProtect();
		printf("===================================\n");
		printf("SF Sector erase ok: addr=0x%x\n", addr);
		printf("===================================\n");
	} else if (opt == 2) {
		offset = strtoul(argv[3], NULL, 0);
		length = strtoul(argv[4], NULL, 0);
		SF_Read(addr + offset, length, ubBuf);
		printf("===================================\n");
		printf("SF Read: addr=0x%x, length=0x%x\n", addr + offset, length);
		
		if (length >= 16) {
			for (uwTemp=0; uwTemp<(length/16*16); uwTemp+=16) {
				printf("Reg[0x%08X]:  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X\n", 
					addr + offset + uwTemp,
					ubBuf[uwTemp],
					ubBuf[uwTemp + 1],
					ubBuf[uwTemp + 2],
					ubBuf[uwTemp + 3],
					ubBuf[uwTemp + 4],
					ubBuf[uwTemp + 5],
					ubBuf[uwTemp + 6],
					ubBuf[uwTemp + 7],
					ubBuf[uwTemp + 8],
					ubBuf[uwTemp + 9],
					ubBuf[uwTemp + 10],
					ubBuf[uwTemp + 11],
					ubBuf[uwTemp + 12],
					ubBuf[uwTemp + 13],
					ubBuf[uwTemp + 14],
					ubBuf[uwTemp + 15]);
			}
		}
		
		if (length % 16) {
			addr = addr + offset + uwTemp;
			printf("Reg[0x%08X]:", addr);
			addr = uwTemp;

			for (uwTemp=0; uwTemp<(length%16); uwTemp+=1) {
				printf("  0x%02X", ubBuf[addr + uwTemp]);
			}
			printf("\n");
		}

		printf("===================================\n");
	} else if (opt == 3) {
		offset = strtoul(argv[3], NULL, 0);
		length = strtoul(argv[4], NULL, 0);
		ulSectorAddr = addr / 4096 * 4096;
		
		SF_Read(ulSectorAddr, pSF_Info->ulSecSize, ubBuf);

		printf("===================================\n");
		printf("SF Write: addr=0x%x, length=0x%x\n", addr + offset, length);
		printf("data:\n");
		
		for (uwTemp=0; uwTemp<length; uwTemp+=1) {
			ubBuf[addr + offset - ulSectorAddr + uwTemp] = strtoul(argv[5 + uwTemp], NULL, 0),
			printf("0x%02X\n", ubBuf[addr + offset - ulSectorAddr + uwTemp]);
		}
		SF_DisableWrProtect();
		SF_Erase(SF_SE, ulSectorAddr, pSF_Info->ulSecSize);
		SF_Write(ulSectorAddr, pSF_Info->ulSecSize, ubBuf);
		SF_EnableWrProtect();
		printf("===================================\n");
	} else if (opt == 4) {
		printf("===================================\n");
		printf("SF Chip Erase...\n");
		SF_DisableWrProtect();
		SF_Erase(SF_CE, 0, 0);
		SF_EnableWrProtect();
		printf("SF Chip Erase ok\n");
		printf("===================================\n");
	} else {
		sf_ctrl_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

#endif
