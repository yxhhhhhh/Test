/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_i2c.c
	\brief		I2C control command line
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
#include "I2C.h"

//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_I2C

static void i2c_ctrl_usage() {   
	printf(" pls check uasge!\n");
	printf("###################################\n");
	printf(" Usage: i2c_ctrl <Device> <OPT> <SlaveID> <Mode> <Reg> <Value/NumItem>\n");
	printf(" Device:\n");
	printf("	0: I2C1\n");
	printf("	1: I2C2\n");
	printf(" OPT:\n");
	printf("	0: Write operation\n");
	printf("	1: Read operation\n");
	printf(" SlaveID: Device slave ID\n");
	printf(" Mode:\n");
	printf("	0: SNX_I2C_R8D8_MODE\n");
	printf("	1: SNX_I2C_R8D16_MODE\n");
	printf("	2: SNX_I2C_R16D8_MODE\n");
	printf("	3: SNX_I2C_R16D16_MODE\n");
	printf(" Reg: register address\n");
	printf(" Value: register value for write opt\n");
	printf(" NumItem: Number of item for contimnue read opt\n");
	printf(" Example:\n");
	printf("	i2c_ctrl 0 1 0x60 0 0x01\n");
	printf("	i2c_ctrl 0 0 0x60 0 0x01 0x23\n");
	printf("###################################\n");
}  

int32_t cmd_i2c_ctrl(int argc, char* argv[])
{                          
	int8_t device = 0;
	int8_t operation = 0;
	uint8_t slaveID = 0;
	int8_t mode = SNX_I2C_R8D8_MODE;
	uint16_t reg = 0; 
	uint16_t value = 0; 
	uint8_t ubI2cRegNum = 0;
	uint8_t ubI2cValueNum = 0;
	uint8_t pBuf[4];
	uint16_t uwNumItem = 0, uwTemp = 0; 
	
	if (argc < 6) {
		i2c_ctrl_usage();
		return cliFAIL; 
	}
	
	device = strtoul(argv[1], NULL, 0);
	if ((device < 0) || (device > 1)) {
		printf("Wrong device:%d\n", device);
		i2c_ctrl_usage();
		return cliFAIL;
	}

	operation = strtoul(argv[2], NULL, 0);
	if ((operation < 0) || (operation > 1))
	{
		printf("Wrong opt:%d\n", operation);
		i2c_ctrl_usage();
		return cliFAIL;
	}

	slaveID = strtoul(argv[3], NULL, 0);
	mode = strtoul(argv[4], NULL, 0);
	if ((mode < SNX_I2C_R8D8_MODE) || (mode > SNX_I2C_R16D16_MODE) ) {
		printf("Wrong MODE:%d\n", mode);
		i2c_ctrl_usage();
		return cliFAIL;
	}
	
	if (mode == SNX_I2C_R8D8_MODE) {
		ubI2cRegNum = 1;
		ubI2cValueNum = 1;
	} else if (mode == SNX_I2C_R8D16_MODE) {
		ubI2cRegNum = 1;
		ubI2cValueNum = 2;
	} else if (mode == SNX_I2C_R16D8_MODE) {
		ubI2cRegNum = 2;
		ubI2cValueNum = 1;
	} else if (mode == SNX_I2C_R16D16_MODE) {
		ubI2cRegNum = 2;
		ubI2cValueNum = 2;
	}
	
	reg = strtoul(argv[5], NULL, 0);	

	if (operation == SNX_I2C_OP_WRITE) {
		if (argc > 6) {
			value = strtoul(argv[6], NULL, 0);

			if (ubI2cRegNum == 2) {
				pBuf[0] = (reg >> 8);
				pBuf[1] = reg & 0xff;		
				
				if (ubI2cValueNum == 2) {
					pBuf[2] = (value >> 8);
					pBuf[3] = value & 0xff;	
				} else {
					pBuf[2] = value & 0xff;
				}						
			} else {
				pBuf[0] = (reg & 0xff);
				
				if (ubI2cValueNum == 2) {
					pBuf[1] = (value >> 8);
					pBuf[2] = value & 0xff;	
				} else {
					pBuf[1] = value & 0xff;
				}												
			}
			
			if (device == 0) {	
				bI2C_MasterProcess(I2C1, slaveID, pBuf, ubI2cRegNum + ubI2cValueNum, NULL, 0);
			} else {
				bI2C_MasterProcess(I2C2, slaveID, pBuf, ubI2cRegNum + ubI2cValueNum, NULL, 0);
			}
			printf("===================================\n");
			printf("Wr I2C\n");
			printf("Device=0x%x,SlaveId=0x%x,Reg_num=0x%x,Value_num=0x%x\n", device, slaveID, ubI2cRegNum, ubI2cValueNum);
			if (mode == SNX_I2C_R8D8_MODE) {
				printf("Reg[0x%02x]=0x%02x\n", reg, value);
			} else if (mode == SNX_I2C_R8D16_MODE) {
				printf("Reg[0x%02x]=0x%04x\n", reg, value);
			} else if (mode == SNX_I2C_R16D8_MODE) {
				printf("Reg[0x%04x]=0x%02x\n", reg, value);
			} else if (mode == SNX_I2C_R16D16_MODE) {
				printf("Reg[0x%04x]=0x%04x\n", reg, value);
			}
			printf("===================================\n");
		} else {
			printf("NO write value\n");
			return cliFAIL;
		}
	} else {
		uwNumItem = strtoul(argv[6], NULL, 0);
		
		if (uwNumItem < 2) {
			if (ubI2cRegNum == 2) {
				pBuf[0] = (reg >> 8);
				pBuf[1] = reg & 0xff;		
			} else {
				pBuf[0] = (reg & 0xff);
			}
			
			if (device == 0) {				
				bI2C_MasterProcess(I2C1, slaveID, pBuf, ubI2cRegNum, (uint8_t *)&value, ubI2cValueNum);
			} else {
				bI2C_MasterProcess(I2C2, slaveID, pBuf, ubI2cRegNum, (uint8_t *)&value, ubI2cValueNum);
			}
			printf("===================================\n");
			printf("Rd I2C\n");
			printf("Device=0x%x,SlaveId=0x%x,Reg_num=0x%x,Value_num=0x%x\n", device, slaveID, ubI2cRegNum, ubI2cValueNum);
			if (mode == SNX_I2C_R8D8_MODE) {
				printf("Reg[0x%02x]=0x%02x\n", reg, value);
			} else if (mode == SNX_I2C_R8D16_MODE) {
				printf("Reg[0x%02x]=0x%04x\n", reg, value);
			} else if (mode == SNX_I2C_R16D8_MODE) {
				printf("Reg[0x%04x]=0x%02x\n", reg, value);
			} else if (mode == SNX_I2C_R16D16_MODE) {
				printf("Reg[0x%04x]=0x%04x\n", reg, value);
			}
			printf("===================================\n");
		} else {
			printf("===================================\n");
			printf("I2C continue read %d Item\n", uwNumItem);
			printf("Device=0x%x,SlaveId=0x%x,Reg_num=0x%x,Value_num=0x%x\n", device, slaveID, ubI2cRegNum, ubI2cValueNum);
			for (uwTemp = 0; uwTemp < uwNumItem; uwTemp += 1) {
				if (ubI2cRegNum == 2) {
					pBuf[0] = (reg >> 8);
					pBuf[1] = reg & 0xff;		
				} else {
					pBuf[0] = (reg & 0xff);
				}
				
				if (device == 0) {				
					bI2C_MasterProcess(I2C1, slaveID, pBuf, ubI2cRegNum, (uint8_t *)&value, ubI2cValueNum);
				} else {
					bI2C_MasterProcess(I2C2, slaveID, pBuf, ubI2cRegNum, (uint8_t *)&value, ubI2cValueNum);
				}

				if (mode == SNX_I2C_R8D8_MODE) {
					printf("Reg[0x%02x]=0x%02x\n", reg, value);
				} else if (mode == SNX_I2C_R8D16_MODE) {
					printf("Reg[0x%02x]=0x%04x\n", reg, value);
				} else if (mode == SNX_I2C_R16D8_MODE) {
					printf("Reg[0x%04x]=0x%02x\n", reg, value);
				} else if (mode == SNX_I2C_R16D16_MODE) {
					printf("Reg[0x%04x]=0x%04x\n", reg, value);
				}
				reg ++;
			}
			printf("===================================\n");
		}
	}
	
	return cliPASS;	
}

#endif
