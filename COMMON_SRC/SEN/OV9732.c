/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OV9732.c
	\brief		Sensor OV9732 relation function
	\author		BoCun
	\version	1
	\date		2017-10-18
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "SEN.h"
#include "I2C.h"
#include "TIMER.h"
#include "IQ_PARSER_API.h"

#if (SEN_USE == SEN_OV9732)
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
I2C_TYP I2C_Sel = I2C_2;

uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
//	0x83, 0x01, 0x03, 0x01,	//sofeware reset
	0x83, 0x01, 0x00, 0x00,	//sofeware standby(sleep)
	0x83, 0x30, 0x01, 0x1f,
	0x83, 0x30, 0x02, 0xff,
	0x83, 0x30, 0x07, 0x40,
	0x83, 0x30, 0x08, 0x00,
	0x83, 0x30, 0x09, 0x03,
	0x83, 0x30, 0x10, 0x00,
	0x83, 0x30, 0x11, 0x00,
	0x83, 0x30, 0x14, 0x36,
	0x83, 0x30, 0x1e, 0x15,
	0x83, 0x30, 0x30, 0x09,
	// system clock control	
	// if SEN_CLK = 24MHz (limit 6~27MHz)
	// PIXEL_CLK = (SEN_CLK * PLL_MULTIPLIER) / (PRE_PLL_CLK_DIV * VT_SYS_CLK_DIV * VT_PIXEL_CLK_DIV * SYS_CLK_DIV)
	//			 = 36MHz
	0x83, 0x30, 0x80, 0x02,	// PRE_PLL_CLK_DIV = 2			note:Bit[2:0]=>1/1.5/2/2.5/3/4/5/6
	0x83, 0x30, 0x81, 0x3c,	// PLL_MULTIPLIER = 60
	0x83, 0x30, 0x82, 0x04,	// VT_SYS_CLK_DIV = 5			note:1+sys_div
	0x83, 0x30, 0x83, 0x00,	// VT_PIXEL_CLK_DIV = 2		    note:Bit[0]=>2/4
	0x83, 0x31, 0x03, 0x01,	// SYS_CLK_DIV = 2				note:Bit[6:4]([0:2]?)=>1~7
    
	0x83, 0x30, 0x84, 0x02,
	0x83, 0x30, 0x85, 0x01,
	0x83, 0x30, 0x86, 0x01,
	0x83, 0x30, 0x89, 0x01,
	0x83, 0x30, 0x8a, 0x00,
	0x83, 0x36, 0x00, 0xf6,
	0x83, 0x36, 0x01, 0x72,
	0x83, 0x36, 0x10, 0x0c,
	0x83, 0x36, 0x11, 0xf0, //////////////////////////
	0x83, 0x36, 0x12, 0x35,
	0x83, 0x36, 0x54, 0x10,
	0x83, 0x36, 0x55, 0x77,
	0x83, 0x36, 0x56, 0x77,
	0x83, 0x36, 0x57, 0x07,
	0x83, 0x36, 0x58, 0x22,
	0x83, 0x36, 0x59, 0x22,
	0x83, 0x36, 0x5a, 0x02,
	0x83, 0x37, 0x00, 0x1f,
	0x83, 0x37, 0x01, 0x10,
	0x83, 0x37, 0x02, 0x0c,
	0x83, 0x37, 0x03, 0x07, //////////////////////////
	0x83, 0x37, 0x04, 0x3c,
	0x83, 0x37, 0x05, 0x81, //////////////////////////
	0x83, 0x37, 0x0d, 0x20, 
	0x83, 0x37, 0x10, 0x0c, //////////////////////////
	0x83, 0x37, 0x82, 0x58, 
	0x83, 0x37, 0x83, 0x60, 
	0x83, 0x37, 0x84, 0x05,
	0x83, 0x37, 0x85, 0x55,
	0x83, 0x37, 0xc0, 0x07,	
	
	//	x_address_start = 4,	x_address_end = 1291
	//	y_address_start = 4,	y_address_end = 731	
	//	deltaX = (x_address_end - x_address_start + 1) =1288
	//	deltaY = (y_address_end - y_address_start + 1) =728
	0x83, 0x38, 0x00, 0x00,	// x start h
	0x83, 0x38, 0x01, 0x04,	// x start l	
	0x83, 0x38, 0x02, 0x00,	// y start h
	0x83, 0x38, 0x03, 0x04,	// y start l	
	0x83, 0x38, 0x04, 0x05,	// x end h
	0x83, 0x38, 0x05, 0x0b,	// x end l	
	0x83, 0x38, 0x06, 0x02,	// y end h
	0x83, 0x38, 0x07, 0xdb,	// y end l		
	// x_output_size = 0x500 = 1280
	// y_output_size = 0x2d0 = 720
	0x83, 0x38, 0x08, 0x05,	// x size h
	0x83, 0x38, 0x09, 0x00,	// x size l 
	0x83, 0x38, 0x0a, 0x02,	// y size h
	0x83, 0x38, 0x0b, 0xd0,	// y size l	
	// total pixel per line = 0x5c6 = 1478
	// total line per frame = 0x322 = 802
	0x83, 0x38, 0x0c, 0x05,	// HTS h
	0x83, 0x38, 0x0d, 0xc6,	// HTS l	
	0x83, 0x38, 0x0e, 0x03,	// VTS h
	0x83, 0x38, 0x0f, 0x22,	// VTS l	
	
	0x83, 0x38, 0x10, 0x00,	// ISP x window
	0x83, 0x38, 0x11, 0x04,	// ISP x window
	0x83, 0x38, 0x12, 0x00,	// ISP y window
	0x83, 0x38, 0x13, 0x04,	// ISP y window
	0x83, 0x38, 0x16, 0x00,	// ISP
	0x83, 0x38, 0x17, 0x00,	// ISP
	0x83, 0x38, 0x18, 0x00,	// ISP
	0x83, 0x38, 0x19, 0x01,	// ISP
	0x83, 0x38, 0x20, 0x10,	// mirror/flip
	//0x83, 0x38, 0x20, 0x18,	// mirror/flip
	0x83, 0x38, 0x21, 0x00,	
	0x83, 0x38, 0x2c, 0x06,
	
	// AEC enable => 0x3503 bit[0]
	// AGC enable => 0x3503 bit[1]
	// exposure time = 0x3100 = 12544
	// analog_gain = 0x40 = 64
	0x83, 0x35, 0x00, 0x00,
	0x83, 0x35, 0x01, 0x31,
	0x83, 0x35, 0x02, 0x00,
	0x83, 0x35, 0x03, 0x03,
	0x83, 0x35, 0x04, 0x00,
	0x83, 0x35, 0x05, 0x00,
	0x83, 0x35, 0x09, 0x10,
	0x83, 0x35, 0x0a, 0x00,
	0x83, 0x35, 0x0b, 0x40,
	0x83, 0x3d, 0x00, 0x00,
	0x83, 0x3d, 0x01, 0x00,
	0x83, 0x3d, 0x02, 0x00,
	0x83, 0x3d, 0x03, 0x00,
	0x83, 0x3d, 0x04, 0x00,
	0x83, 0x3d, 0x05, 0x00,
	0x83, 0x3d, 0x06, 0x00,
	0x83, 0x3d, 0x07, 0x00,
	0x83, 0x3d, 0x08, 0x00,
	0x83, 0x3d, 0x09, 0x00,
	0x83, 0x3d, 0x0a, 0x00,
	0x83, 0x3d, 0x0b, 0x00,
	0x83, 0x3d, 0x0c, 0x00,
	0x83, 0x3d, 0x0d, 0x00,
	0x83, 0x3d, 0x0e, 0x00,
	0x83, 0x3d, 0x0f, 0x00,
	0x83, 0x3d, 0x80, 0x00,
	0x83, 0x3d, 0x81, 0x00,
	0x83, 0x3d, 0x82, 0x38,
	0x83, 0x3d, 0x83, 0xa4,
	0x83, 0x3d, 0x84, 0x00,
	0x83, 0x3d, 0x85, 0x00,
	0x83, 0x3d, 0x86, 0x1f,
	0x83, 0x3d, 0x87, 0x03,
	0x83, 0x3d, 0x8b, 0x00,
	0x83, 0x3d, 0x8f, 0x00,
	0x83, 0x40, 0x01, 0xe0,
	0x83, 0x40, 0x04, 0x00,
	0x83, 0x40, 0x05, 0x02,
	0x83, 0x40, 0x06, 0x01,
	0x83, 0x40, 0x07, 0x40,
	0x83, 0x40, 0x09, 0x0b,
	0x83, 0x43, 0x00, 0x03,	// Y max h
	0x83, 0x43, 0x01, 0xff,	// Y max l		0x3ff = 1023
	0x83, 0x43, 0x04, 0x00,
	0x83, 0x43, 0x05, 0x00,
	0x83, 0x43, 0x09, 0x00,
	0x83, 0x46, 0x00, 0x00,
	0x83, 0x46, 0x01, 0x04,
	0x83, 0x48, 0x00, 0x04,	// MIPI // 04
	0x83, 0x48, 0x05, 0x00,	// MIPI
	0x83, 0x48, 0x21, 0x3c,	// MIPI //3c ........
	0x83, 0x48, 0x23, 0x3c,	// MIPI
	0x83, 0x48, 0x37, 0x2d,	// MIPI
	0x83, 0x4a, 0x00, 0x00,
	0x83, 0x4f, 0x00, 0x80,
	0x83, 0x4f, 0x01, 0x10,
	0x83, 0x4f, 0x02, 0x00,
	0x83, 0x4f, 0x03, 0x00,
	0x83, 0x4f, 0x04, 0x00,
	0x83, 0x4f, 0x05, 0x00,
	0x83, 0x4f, 0x06, 0x00,
	0x83, 0x4f, 0x07, 0x00,
	0x83, 0x4f, 0x08, 0x00,
	0x83, 0x4f, 0x09, 0x00,
	0x83, 0x50, 0x00, 0x0f,	// ISP  //////////////////////////
	0x83, 0x50, 0x0c, 0x00,	// ISP
	0x83, 0x50, 0x0d, 0x00,	// ISP
	0x83, 0x50, 0x0e, 0x00,	// ISP
	0x83, 0x50, 0x0f, 0x00,	// ISP
	0x83, 0x50, 0x10, 0x00,	// ISP
	0x83, 0x50, 0x11, 0x00,	// ISP
	0x83, 0x50, 0x12, 0x00,	// ISP
	0x83, 0x50, 0x13, 0x00,	// ISP
	0x83, 0x50, 0x14, 0x00,	// ISP
	0x83, 0x50, 0x15, 0x00,	// ISP
	0x83, 0x50, 0x16, 0x00,	// ISP
	0x83, 0x50, 0x17, 0x00,	// ISP
	0x83, 0x50, 0x80, 0x00,	// testpattern  NOTE: Bit[7]=>test pattern enable
	0x83, 0x51, 0x80, 0x01,
	0x83, 0x51, 0x81, 0x00,
	0x83, 0x51, 0x82, 0x01,
	0x83, 0x51, 0x83, 0x00,
	0x83, 0x51, 0x84, 0x01,
	0x83, 0x51, 0x85, 0x00,
	0x83, 0x57, 0x08, 0x06,
	0x83, 0x57, 0x81, 0x00, //////////////////////////
	0x83, 0x57, 0x83, 0x0f,
    //////////////////////////
	0x83, 0x01, 0x00, 0x01,	//sofeware standby(streaming)
	0x83, 0x37, 0x03, 0x0b,
	0x83, 0x37, 0x05, 0x51,	
};

uint8_t ubSEN_PckSettingTable[] = {
	// 18MHz 15P
	0x83, 0x30, 0x83, 0x01,
	0x83, 0x38, 0x0c, 0x05,	// HTS h
	0x83, 0x38, 0x0d, 0xd8,	// HTS l 
	// 36MHz 20P
	0x83, 0x30, 0x83, 0x00,
	0x83, 0x38, 0x0c, 0x08,	// HTS h
	0x83, 0x38, 0x0d, 0xc4,	// HTS l    
	// 36MHz 25P
	0x83, 0x30, 0x83, 0x00,
	0x83, 0x38, 0x0c, 0x07,	// HTS h
	0x83, 0x38, 0x0d, 0x03,	// HTS l    
	// 36MHz 30P
	0x83, 0x30, 0x83, 0x00,
	0x83, 0x38, 0x0c, 0x05,	// HTS h
	0x83, 0x38, 0x0d, 0xd8,	// HTS l						
};

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Read(uint16_t uwAddress, uint8_t *pValue)
{
	uint8_t *pAddr,pBuf[2];
	
	pAddr = (uint8_t*)&uwAddress;
	pBuf[0] = pAddr[1];
	pBuf[1] = pAddr[0];
	
	return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, pValue, 1);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Write(uint8_t ubAddress1, uint8_t ubAddress2, uint8_t ubValue)
{	
	uint8_t ubData, pBuf[3];
	
	pBuf[0] = ubAddress1;
	pBuf[1] = ubAddress2;
	pBuf[2] = ubValue;	

	// write data to sensor register
	bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);
	
	// check if write success
	bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, &ubData, 1);	
	if (ubData != ubValue)
	{
		//printf("Sensor I2C write REG=0x%x with value=0x%x failed!\n", uwAddress, ubValue);
		return 0;
	}	
	
	return 1;	
}

//------------------------------------------------------------------------------
void SEN_PclkSetting(uint8_t ubPclkIdx)
{
	uint8_t i;	
	uint8_t ubIdx;
	
	switch (ubPclkIdx)
	{
		case SEN_FPS15:
			ubIdx = 0;
			sensor_cfg.ulSensorPclk = 18000000;  
			sensor_cfg.ulSensorPclkPerLine = 1478;
			sensor_cfg.ulSensorFrameRate = 15;
			break;
		case SEN_FPS20:
			ubIdx = 12;
			sensor_cfg.ulSensorPclk = 36000000;  
			sensor_cfg.ulSensorPclkPerLine = 2244;
			sensor_cfg.ulSensorFrameRate = 20;
			break; 
		case SEN_FPS25:
			ubIdx = 24;
			sensor_cfg.ulSensorPclk = 36000000;  
			sensor_cfg.ulSensorPclkPerLine = 1795;
			sensor_cfg.ulSensorFrameRate = 25;
			break;        
		case SEN_FPS30:
			ubIdx = 36;
			sensor_cfg.ulSensorPclk = 36000000;  
			sensor_cfg.ulSensorPclkPerLine = 1496;
			sensor_cfg.ulSensorFrameRate = 30;
			break;
		default:
			ubIdx = 36;
			sensor_cfg.ulSensorPclk = 36000000;  
			sensor_cfg.ulSensorPclkPerLine = 1496;
			sensor_cfg.ulSensorFrameRate = 30;
			break;
	}
	
	for (i=ubIdx; i<(ubIdx+12); i+=4)
	{
		if (ubSEN_PckSettingTable[i] == 0x83)	// write
		{
			ulSEN_I2C_Write(ubSEN_PckSettingTable[i+1], ubSEN_PckSettingTable[i+2], ubSEN_PckSettingTable[i+3]);
		}
	}
}

//------------------------------------------------------------------------------
void SEN_InitBySF(void)
{
	I2C1_Type *pI2C;
	uint32_t ulTemp = 0;
	uint8_t *ulAddr;
	uint8_t pBuf[4];
	
	pI2C = I2C1;
	
	// Sensor initial table address
	ulAddr = (uint8_t *)0x030C0000;
	// tabale length(maximun is 1kByte)
	while(ulTemp < 1024)
	{
		if(*(uint8_t *)(ulAddr+ulTemp) == 0x82)
		{
			pBuf[0] = *(uint8_t *)(ulAddr+ulTemp+1);
			pBuf[1] = *(uint8_t *)(ulAddr+ulTemp+2);	
		
			bI2C_MasterProcess (pI2C, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);	
			ulTemp+=3;
		}else if(*(uint8_t *)(ulAddr+ulTemp) == 0x83){
			pBuf[0] = *(uint8_t *)(ulAddr+ulTemp+1);
			pBuf[1] = *(uint8_t *)(ulAddr+ulTemp+2);	
			pBuf[2] = *(uint8_t *)(ulAddr+ulTemp+3);
		
			bI2C_MasterProcess (pI2C, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);			
			ulTemp+=4;	
		}else if(*(uint8_t *)(ulAddr+ulTemp) == 0x84){
			pBuf[0] = *(uint8_t *)(ulAddr+ulTemp+1);
			pBuf[1] = *(uint8_t *)(ulAddr+ulTemp+2);	
			pBuf[2] = *(uint8_t *)(ulAddr+ulTemp+3);
			pBuf[3] = *(uint8_t *)(ulAddr+ulTemp+4);

			bI2C_MasterProcess (pI2C, SEN_SLAVE_ADDR, &pBuf[0], 4, NULL, 0);
			ulTemp+=5;				
		}else if(*(uint8_t *)(ulAddr+ulTemp) == 0xbb){
			TIMER_Delay_us(1000);	// delay 1ms
			ulTemp+=2;
		}else if(*(uint8_t *)(ulAddr+ulTemp) == 0xFF ){
			if(*(uint8_t *)(ulAddr+ulTemp+1) == 0xFF && *(uint8_t *)(ulAddr+ulTemp+2) == 0xFF && *(uint8_t *)(ulAddr+ulTemp+3) == 0xFF)
			{
				ulTemp += 4;
				break;
			}		
		}else{
			ulTemp+=1;
		}		
	}
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Start(struct SENSOR_SETTING *setting, uint8_t ubFPS)
{
	uint8_t     *pBuf;	
	uint16_t 	uwPID = 0;
	uint32_t 	i;	
	
	pI2C_type = pI2C_MasterInit (I2C_2, I2C_SCL_400K);
	IQ_SetI2cType(pI2C_type);

_RETRY:
	pBuf = (uint8_t*)&uwPID;    
	// I2C by Read Sensor ID
	ulSEN_I2C_Read (OV9732_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	ulSEN_I2C_Read (OV9732_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (OV9732_CHIP_ID != uwPID)
	{
		printf("This is not OV9732 Sensor!! 0x%x 0x%x\n", OV9732_CHIP_ID, uwPID);
        TIMER_Delay_us(10000);
        goto _RETRY;
	}	
    printf("chip ID check ok!\r\n");	
	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
		}
	}	
    
    //SEN_PclkSetting(SEN_FPS30);
	SEN_PclkSetting(ubFPS);
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting, uint8_t ubFPS)
{		
		// Set ISP clock
		SEN_SetISPRate(8);		
		// Set parallel mode
		SEN->MIPI_MODE = 0;
		// Set sensor clock
		SEN_SetSensorRate(SENSOR_96MHz, 4);	

		// Set change frame at Vsync rising/falling edge
		SEN->VSYNC_RIS = 0;
		// Set change frame at Hsync rising/falling edge
		SEN->HSYNC_RIS = 1;
		// Set Vsync high/low active
		SEN->VSYNC_HIGH = 1;
		// Set data latch at PCLK rising/falling edge
		SEN->PCK_DLH_RIS = 1;
		// Free run mode for PCLK
		SEN->SYNC_MODE = 1;		
	
		// Change ISP_LH_SEL to 0, for 10bit raw input
		SEN->ISP_LH_SEL = 0;																		// change ISP_LH_SEL to 1, for D2-D9 input
		SEN->ISP_HSTART_OFFSET = 0;
		SEN->ISP_MODE = 0;																			// Bayer pat mode

		// change SN_SEN->RAW_REORDER to 0, start from B
		SEN->RAW_REORDER = 0;
		SEN->SENSOR_MODE = 1;																		// Buffer sync mode
		// Change to 10bit RAW
		SEN->RAW_BP_MODE = 1;
		SEN->RAW_BP_EN = 0;		
		// Input YUV data, and bypass to PostColor
		SEN->ISP_BRIG = 0;

		// set dummy line & pixel
		SEN->DMY_DIV = 1; // dummy clock=sysclk/2
		SEN->NUM_DMY_LN = 24;
		SEN->NUM_DMY_DSTB = 1536;
		SEN->DMY_INSERT = 1;

		// Unable write to dram.
		SEN->IMG_TX_EN = 0;

		// clear all debug flag
		SEN->REG_0x1300 = 0x1ff;
		SEN->REG_0x1304 = 0x3;
		// enable sensor PCLK
		SEN->SEN_CLK_EN = 1;	

		//delay 1ms
		TIMER_Delay_us(1000);		
		// Initial dummy sensor
		if (ubSEN_Start(setting,ubFPS) != 1)
		{
			printf("Ov9732 startup failed! \n\r");
			return 0;
		}			
		// enable interrupt
		SEN->HW_END_INT_EN = 1;
		SEN->SEN_VSYNC_INT_EN = 1;
		SEN->SEN_HSYNC_INT_EN = 1;

//	 //msut last beacause SEN_DSTB_TSEL is write only.
//		SEN->SEN_DSTB_TSEL = 1;	
		return 1;
}

//------------------------------------------------------------------------------
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime)
{
	//Transfor the ExpLine of Algorithm	to ExpLine and DmyLine of Sensor here

	xtSENInst.xtSENCtl.ulExpTime = ulAlgExpTime;
	xtSENInst.xtSENCtl.uwExpLine = uwSEN_CalExpLine(ulAlgExpTime);

	if(xtSENInst.xtSENCtl.uwExpLine < 5)
	{
		xtSENInst.xtSENCtl.uwExpLine = 5;
	}	

	//Calculate Dummy line
	if(xtSENInst.xtSENCtl.uwExpLine > (SEN_MAX_EXPLINE - 4))
	{
		xtSENInst.xtSENCtl.uwDmyLine = (xtSENInst.xtSENCtl.uwExpLine + 4);
	}
	else
	{
		xtSENInst.xtSENCtl.uwDmyLine = SEN_MAX_EXPLINE;
	}
}

//------------------------------------------------------------------------------
uint32_t ulSEN_GetPixClk(void)
{
	return (sensor_cfg.ulSensorPclk);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_GetPckPerLine(void)
{
	return (sensor_cfg.ulSensorPclkPerLine);
}

//------------------------------------------------------------------------------
void SEN_WriteTotalLine(void)
{

}

//------------------------------------------------------------------------------
void SEN_WrDummyLine(uint16_t uwDL)
{
	//Set dummy lines
	ulSEN_I2C_Write(0x38, 0x0e, (uint8_t)((uwDL>>8) &0x00ff));
	TIMER_Delay_us(20);
	ulSEN_I2C_Write(0x38, 0x0f, (uint8_t)(uwDL &0x00ff));		
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
    //Set long exposure
	ulSEN_I2C_Write(0x35, 0x00, (uint8_t)((uwExpLine>>12)&0x000f));
	ulSEN_I2C_Write(0x35, 0x01, (uint8_t)((uwExpLine>>4)&0x00ff));
	ulSEN_I2C_Write(0x35, 0x02, (uint8_t)((uwExpLine<<4)&0x00f0));
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	uint8_t  ubAGaintmp;
	uint16_t uwDGaintmp;
		
	//avoid warning
	uwDGaintmp = uwDGaintmp;
	ubAGaintmp = ubAGaintmp;
	
	//	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}		
		
    if(ulGainX1024 < 2048)							            //analog 1x~2x
    {
        ubAGaintmp = 0x10+((ulGainX1024-1024)>>6);
        uwDGaintmp = 0x100;
    }
    else if(ulGainX1024 < 4096)					                //analog 2x~4x
    {		
        ubAGaintmp = 0x20+((ulGainX1024-2048)>>6);
        uwDGaintmp = 0x100;
    }
    else if(ulGainX1024 < 8192)					                //analog 4x~8x
    {
        ubAGaintmp = 0x40+((ulGainX1024-4096)>>6);
        uwDGaintmp = 0x100;
    }
    else if(ulGainX1024 < 16384)					            //analog 8x~16x
    {
        ubAGaintmp = 0x80+((ulGainX1024-8192)>>6);
        uwDGaintmp = 0x100;
    }
    else if(ulGainX1024 < 32768)					            //digital*analog 16x~32x
    {			 
        
        uwDGaintmp = 0x100+((ulGainX1024-16384)>>6);
        ubAGaintmp = 0xff;
    }		
    else if(ulGainX1024 < 65536)
    {														    //digital*analog 32x~64x

        uwDGaintmp = 0x200+((ulGainX1024-32768)>>6);
        ubAGaintmp = 0xff;							
    }
	// long gain	
	ulSEN_I2C_Write(0x35, 0x0B, (ubAGaintmp & 0xff));
	// R gain	
	ulSEN_I2C_Write(0x51, 0x80, ((uwDGaintmp>>8) &0x000f));
	ulSEN_I2C_Write(0x51, 0x81, (uwDGaintmp &0x00ff));
    // G gain
	ulSEN_I2C_Write(0x51, 0x82, ((uwDGaintmp>>8) &0x000f));
	ulSEN_I2C_Write(0x51, 0x83, (uwDGaintmp &0x00ff));
    // B gain
	ulSEN_I2C_Write(0x51, 0x84, ((uwDGaintmp>>8) &0x000f));
	ulSEN_I2C_Write(0x51, 0x85, (uwDGaintmp &0x00ff));
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{

}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{

}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
	// Image for ISP
	sensor_cfg.ulHSize = ISP_WIDTH;
	sensor_cfg.ulVSize = ISP_HEIGHT;	
	// Output size from sensor.
	sensor_cfg.ulHOSize = 1280;
	sensor_cfg.ulVOSize = 720;	
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ulSensorType = SEN_OV9732;	
    printf("sensor type is OV9732\r\n");
}
#endif
