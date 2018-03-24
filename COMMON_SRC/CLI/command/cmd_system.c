/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_system.c
	\brief		System control command line
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
#include "KNL.h"
#include "RTC_API.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_SYSTEM

static void system_date_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage : date <OPT> <Year> <Month> <Date> <Hour> <Minute> <Second>\n");
	printf(" OPT:\n");
	printf("	0: Get current date and time\n");
	printf("	1: Set current date and time\n");
	printf(" Example 1: Get date and time\n");
	printf("	date 0\n");
	printf(" Example 2: Set date and time 2017/10/19 12:00:00\n");
	printf("	date 1 2017 10 19 12 0 0\n");
	printf("###################################\n");
}  

int32_t cmd_system_date(int argc, char* argv[])
{
	uint8_t OPT;
	RTC_Calendar_t tSys_Calendar;
	
	if (argc < 2) {
		system_date_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);

	if (OPT == 0) {
		RTC_GetCalendar(&tSys_Calendar);
		printf("===================================\n");
		printf("Get Date & Time:\n");
		printf("%d/%d/%d,%d:%d:%d\n",
			tSys_Calendar.uwYear,
			tSys_Calendar.ubMonth,
			tSys_Calendar.ubDate,
			tSys_Calendar.ubHour,
			tSys_Calendar.ubMin,
			tSys_Calendar.ubSec);
		printf("===================================\n");		
	} else if (OPT == 1) {
		tSys_Calendar.uwYear = strtoul(argv[2], NULL, 0);
		tSys_Calendar.ubMonth = strtoul(argv[3], NULL, 0);
		tSys_Calendar.ubDate = strtoul(argv[4], NULL, 0);
		tSys_Calendar.ubHour = strtoul(argv[5], NULL, 0);
		tSys_Calendar.ubMin = strtoul(argv[6], NULL, 0);
		tSys_Calendar.ubSec = strtoul(argv[7], NULL, 0);
		RTC_SetCalendar(&tSys_Calendar);
		RTC_GetCalendar(&tSys_Calendar);
		printf("===================================\n");
		printf("Set Date & Time:\n");
		printf("%d/%d/%d,%d:%d:%d\n",
			tSys_Calendar.uwYear,
			tSys_Calendar.ubMonth,
			tSys_Calendar.ubDate,
			tSys_Calendar.ubHour,
			tSys_Calendar.ubMin,
			tSys_Calendar.ubSec);
		printf("===================================\n");		
	} else {
		system_date_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

static void phymem_rw_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage 1: phymem_rw <Reg>\n");
	printf(" Brief: read ASIC register(32-bit), return 32 items value\n");
	printf(" Example: \n");
	printf("	phymem_rw 0x900000a4\n");
	printf("###################################\n");
	printf(" Usage 2: phymem_rw <Reg> <Value>\n");
	printf(" Brief: write ASIC register(32-bit)\n");
	printf(" Example: \n");
	printf("	phymem_rw 0x900000a4 10\n");
	printf("###################################\n");
	printf(" Usage 3: phymem_rw <Reg> <NumItems> <Base>\n");
	printf(" Brief: continuous read ASIC register n items(8/16/32-bit)\n");
	printf(" Example: \n");
	printf("	phymem_rw 0x900000a4 4 8\n");
	printf("###################################\n");
	printf(" Usage 4: phymem_rw <Reg> <Value> <Base> <Mask>\n");
	printf(" Brief: write ASIC register with mask and base\n");
	printf(" Example: \n");
	printf("	phymem_rw 0x900000a4 0x1230 32 0x3ff0\n");
	printf("###################################\n");
}  

int32_t cmd_system_phymem_rw(int argc, char* argv[]) {
	uint8_t i;
	uint32_t addr = 0, value = 0, ulTemp = 0;

	if ((argc < 2) || (argc > 5)) {
		phymem_rw_usage();
		return cliFAIL; 
	}

	if (argc == 2) {
		// READ: phymem_rw address
		addr = strtoul(argv[1], NULL, 0) & (~3);
		printf("===================================\n");
		printf("ASIC REG READ (32-bit)\n");
		for (i=0; i<8; i+=1) {
			printf("Reg[0x%08X]:  0x%08X  0x%08X  0x%08X  0x%08X\n", addr + i*4*4,
				*((uint32_t*)(addr + i*4*4)),
				*((uint32_t*)(addr + (i*4 + 1)*4)),
				*((uint32_t*)(addr + (i*4 + 2)*4)),
				*((uint32_t*)(addr + (i*4 + 3)*4)));
		}
		printf("===================================\n");
	} else if (argc == 3) {
		// WRITE: phymem_rw address value
		addr = strtoul(argv[1], NULL, 0) & (~3);
		value = strtoul(argv[2], NULL, 0);
		printf("===================================\n");
		printf("ASIC REG WRITE (32-bit)\n");
		printf("old Reg[0x%08X]=0x%08X\n", addr, *((uint32_t*)addr));
		*(uint32_t*)addr = value;
		printf("new Reg[0x%08X]=0x%08X\n", addr, *((uint32_t*)addr));
		printf("===================================\n");
	} else if (argc == 4) {
		// Continuous read ASIC register n items
		if (strtoul(argv[3], NULL, 0) == 32) {
			addr = strtoul(argv[1], NULL, 0) & (~3);
			ulTemp = strtoul(argv[2], NULL, 0);
			printf("===================================\n");
			printf("ASIC 32-bit continuous read\n");
			if (ulTemp >= 4) {
				for (i=0; i<(ulTemp/4); i+=1) {
					printf("Reg[0x%08X]:  0x%08X  0x%08X  0x%08X  0x%08X\n", addr + i*4*4,
						*((uint32_t*)(addr + i*4*4)),
						*((uint32_t*)(addr + (i*4 + 1)*4)),
						*((uint32_t*)(addr + (i*4 + 2)*4)),
						*((uint32_t*)(addr + (i*4 + 3)*4)));
				}
			}
			if (ulTemp % 4) {
				printf("Reg[0x%08X]:", addr + (ulTemp/4)*4*4);
				
				for (i=0; i<(ulTemp%4); i+=1) {
					printf("  0x%08X", *((uint32_t*)(addr + ((ulTemp/4)*4 + i)*4)));
				}
				printf("\n");
			}
			printf("===================================\n");
		} else if (strtoul(argv[3], NULL, 0) == 16) {
			addr = strtoul(argv[1], NULL, 0) & (~1);
			ulTemp = strtoul(argv[2], NULL, 0);
			printf("===================================\n");
			printf("ASIC 16-bit continuous read\n");
			if (ulTemp >= 8) {
				for (i=0; i<(ulTemp/8); i+=1) {
					printf("Reg[0x%08X]:  0x%04X  0x%04X  0x%04X  0x%04X  0x%04X  0x%04X  0x%04X  0x%04X\n", addr + i*8*2,
						*((uint16_t*)(addr + i*8*2)),
						*((uint16_t*)(addr + (i*8 + 1)*2)),
						*((uint16_t*)(addr + (i*8 + 2)*2)),
						*((uint16_t*)(addr + (i*8 + 3)*2)),
						*((uint16_t*)(addr + (i*8 + 4)*2)),
						*((uint16_t*)(addr + (i*8 + 5)*2)),
						*((uint16_t*)(addr + (i*8 + 6)*2)),
						*((uint16_t*)(addr + (i*8 + 7)*2)));
				}
			}
			if (ulTemp % 8) {
				printf("Reg[0x%08X]:", addr + (ulTemp/8)*8*2);
				
				for (i=0; i<(ulTemp%8); i+=1) {
					printf("  0x%04X", *((uint16_t*)(addr + ((ulTemp/8)*8 + i)*2)));
				}
				printf("\n");
			}
			printf("===================================\n");
		} else if (strtoul(argv[3], NULL, 0) == 8) {
			addr = strtoul(argv[1], NULL, 0);
			ulTemp = strtoul(argv[2], NULL, 0);
			printf("===================================\n");
			printf("ASIC 8-bit continuous read\n");
			if (ulTemp >= 16) {
				for (i=0; i<(ulTemp/16); i+=1) {
					printf("Reg[0x%08X]:  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X  0x%02X\n", 
						addr + i*16,
						*((uint8_t*)(addr + i*16)),
						*((uint8_t*)(addr + (i*16 + 1))),
						*((uint8_t*)(addr + (i*16 + 2))),
						*((uint8_t*)(addr + (i*16 + 3))),
						*((uint8_t*)(addr + (i*16 + 4))),
						*((uint8_t*)(addr + (i*16 + 5))),
						*((uint8_t*)(addr + (i*16 + 6))),
						*((uint8_t*)(addr + (i*16 + 7))),
						*((uint8_t*)(addr + (i*16 + 8))),
						*((uint8_t*)(addr + (i*16 + 9))),
						*((uint8_t*)(addr + (i*16 + 10))),
						*((uint8_t*)(addr + (i*16 + 11))),
						*((uint8_t*)(addr + (i*16 + 12))),
						*((uint8_t*)(addr + (i*16 + 13))),
						*((uint8_t*)(addr + (i*16 + 14))),
						*((uint8_t*)(addr + (i*16 + 15))));
				}
			}
			if (ulTemp % 16) {
				printf("Reg[0x%08X]:", addr + (ulTemp/16)*16);
				
				for (i=0; i<(ulTemp%16); i+=1) {
					printf("  0x%02X", *((uint8_t*)(addr + ((ulTemp/16)*16 + i))));
				}
				printf("\n");
			}
			printf("===================================\n");
		} else {
			phymem_rw_usage();
			return cliFAIL; 
		}
	} else if (argc == 5) {
		// write ASIC register with mask and base
		if (strtoul(argv[3], NULL, 0) == 32) {
			addr = strtoul(argv[1], NULL, 0) & (~3);
			ulTemp = strtoul(argv[4], NULL, 0);
			value = strtoul(argv[2], NULL, 0);
			printf("===================================\n");
			printf("32-bit ASIC register write with mask\n");
			printf("old Reg[0x%08X]=0x%08X\n", addr, *((uint32_t*)addr));
			*((uint32_t*)addr) = (*((uint32_t*)addr) & (~ulTemp) | (ulTemp & value));
			printf("new Reg[0x%08X]=0x%08X\n", addr, *((uint32_t*)addr));
			printf("===================================\n");
		} else if (strtoul(argv[3], NULL, 0) == 16) {
			addr = strtoul(argv[1], NULL, 0) & (~1);
			ulTemp = strtoul(argv[4], NULL, 0);
			value = strtoul(argv[2], NULL, 0);
			printf("===================================\n");
			printf("16-bit ASIC register write with mask\n");
			printf("old Reg[0x%08X]=0x%04X\n", addr, *((uint16_t*)addr));
			*((uint16_t*)addr) = (*((uint16_t*)addr) & (~ulTemp) | (ulTemp & value));
			printf("new Reg[0x%08X]=0x%04X\n", addr, *((uint16_t*)addr));
			printf("===================================\n");
		} else if (strtoul(argv[3], NULL, 0) == 8) {
			addr = strtoul(argv[1], NULL, 0);
			ulTemp = strtoul(argv[4], NULL, 0);
			value = strtoul(argv[2], NULL, 0);
			printf("===================================\n");
			printf("8-bit ASIC register write with mask\n");
			printf("old Reg[0x%08X]=0x%02X\n", addr, *((uint8_t*)addr));
			*((uint8_t*)addr) = (*((uint8_t*)addr) & (~ulTemp) | (ulTemp & value));
			printf("new Reg[0x%08X]=0x%02X\n", addr, *((uint8_t*)addr));
			printf("===================================\n");
		} else {
			phymem_rw_usage();
			return cliFAIL; 
		}
	} else {
		phymem_rw_usage();
		return cliFAIL; 
	}
	return cliPASS;	
}

static void system_fps_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: fps <OPT> <Type> <Src>\n");
	printf(" OPT:\n");
	printf("	1: show current src fps\n");
	printf("	2: show all src fps\n");
	printf(" Type:\n");
	printf("	0: KNL_FPS_OUT\n");
	printf("	1: KNL_FPS_IN\n");
	printf("	2: KNL_BB_FRM_OK\n");
	printf(" Src:\n");
	printf("	0: KNL_SRC_1_MAIN\n");
	printf("	1: KNL_SRC_2_MAIN\n");
	printf("	2: KNL_SRC_3_MAIN\n");
	printf("	3: KNL_SRC_4_MAIN\n");
	printf(" Example 1: AP Get STA 1 fps\n");
	printf("	fps 1 1 0\n");
	printf(" Example 2: STA1 Get AP fps\n");
	printf("	fps 1 0 0\n");
	printf(" Example 3: AP Get All STA fps\n");
	printf("	fps 2\n");
	printf("###################################\n");
}  

int32_t cmd_system_fps(int argc, char* argv[]) {
	uint8_t OPT = 0, Type = 0, Src = 0;

	if (argc < 2) {
		system_fps_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);
	
	if (OPT == 1) {
		Type = strtoul(argv[2], NULL, 0);
		Src = strtoul(argv[3], NULL, 0);
		
		printf("===================================\n");
		printf("Type:%d, Src:%d\n", Type, Src);
		printf("FPS=%d\n", ulKNL_GetFps((KNL_FPS_TYPE)Type, Src));
		printf("===================================\n");
	} else if (OPT == 2) {
		printf("===================================\n");
		printf("FPS STA 1~4:\n");
		printf("%d,%d,%d,%d\n", 
			ulKNL_GetFps(KNL_FPS_IN, KNL_SRC_1_MAIN),
			ulKNL_GetFps(KNL_FPS_IN, KNL_SRC_2_MAIN),
			ulKNL_GetFps(KNL_FPS_IN, KNL_SRC_3_MAIN),
			ulKNL_GetFps(KNL_FPS_IN, KNL_SRC_4_MAIN));
		printf("===================================\n");
	} else {
		system_fps_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

static void system_padio_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: padio <OPT>\n");
	printf(" OPT:\n");
	printf("	1: Get All PADIO Config\n");
	printf(" Example: Get All PADIO Config\n");
	printf("	padio 1\n");
	printf("###################################\n");
}  

int32_t cmd_system_padio(int argc, char* argv[]) {
	uint8_t OPT = 0;

	if (argc < 1) {
		system_padio_usage();
		return cliFAIL; 
	}

	OPT = strtoul(argv[1], NULL, 0);
	
	if (OPT == 1) {
		printf("===================================\n");
		printf("PADIO Config List\n");
		printf("  0~7: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO0,  GLB->PADIO1,  GLB->PADIO2,  GLB->PADIO3,  GLB->PADIO4,  GLB->PADIO5,  GLB->PADIO6,  GLB->PADIO7);
		printf(" 8~15: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO8,  GLB->PADIO9,  GLB->PADIO10, GLB->PADIO11, GLB->PADIO12, GLB->PADIO13, GLB->PADIO14, GLB->PADIO15);
		printf("16~23: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO16, GLB->PADIO17, GLB->PADIO18, GLB->PADIO19, GLB->PADIO20, GLB->PADIO21, GLB->PADIO22, GLB->PADIO23);
		printf("24~31: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO24, GLB->PADIO25, GLB->PADIO26, GLB->PADIO27, GLB->PADIO28, GLB->PADIO29, GLB->PADIO30, GLB->PADIO31);
		printf("32~39: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO32, GLB->PADIO33, GLB->PADIO34, GLB->PADIO35, GLB->PADIO36, GLB->PADIO37, GLB->PADIO38, GLB->PADIO39);
		printf("40~47: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO40, GLB->PADIO41, GLB->PADIO42, GLB->PADIO43, GLB->PADIO44, GLB->PADIO45, GLB->PADIO46, GLB->PADIO47);
		printf("48~55: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO48, GLB->PADIO49, GLB->PADIO50, GLB->PADIO51, GLB->PADIO52, GLB->PADIO53, GLB->PADIO54, GLB->PADIO55);
		printf("56~63: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO56, GLB->PADIO57, GLB->PADIO58, GLB->PADIO59, GLB->PADIO60, GLB->PADIO61, GLB->PADIO62, GLB->PADIO63);
		printf("64~71: %d,%d,%d,%d,%d,%d,%d,%d\n", GLB->PADIO64, GLB->PADIO65, GLB->PADIO66, GLB->PADIO67, GLB->PADIO68, GLB->PADIO69, GLB->PADIO70, GLB->PADIO71);
		printf("   72: %d\n", 					   GLB->PADIO72);
		printf("===================================\n");
	} else {
		system_padio_usage();
		return cliFAIL; 
	}
	return cliPASS; 
}

#endif
