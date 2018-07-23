/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OV9715.c
	\brief		Sensor OV9715 relation function
	\author		BoCun
	\version	1.1
	\date		2018-07-06
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "SEN.h"
#include "I2C.h"
#include "TIMER.h"
#include "CQ_API.h"
#include "CQI2C.h"
#include "USBD_API.h"
#include "APP_HS.h"

#if (SEN_USE == SEN_OV9715)
//CQI2C_WrDataSet_t* pWrDataSet[3];
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
I2C_TYP I2C_Sel = I2C_2;

uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
	0x82, 0x12, 0x80,	// set SRST 1 => init soft reset
	0x82, 0x1e, 0x07,
	0x82, 0x5f, 0x18,
	0x82, 0x69, 0x04,
	0x82, 0x65, 0x2a,
	0x82, 0x68, 0x0a,
	0x82, 0x39, 0x28,
	0x82, 0x4d, 0x90,
	0x82, 0xc1, 0x80,
	0x82, 0x0c, 0x30,
	0x82, 0x6d, 0x02,
	0x82, 0xbc, 0x68,
	0x82, 0x12, 0x00, // set SRST 0 => end soft reset
	0x82, 0x3b, 0x00,
	0x82, 0x97, 0x80,

	// output 1296x810
	0x82, 0x17, 0x25, // set HSTART[3:9] 0x25
	0x82, 0x18, 0xA2, // set HOUT[3:9] 0xa2
	0x82, 0x19, 0x01, // set VSTART[2:8] 0x1
	0x82, 0x1a, 0xCA, // set VOUT[2:8] 0xCA
	0x82, 0x03, 0x0A, // set VSTART[0:1] 0x2, VOUT[0:1] 0x2
	0x82, 0x32, 0x07, // set HSTART[0:2] 0x3, HOUT[0] 0x1

	0x82, 0x98, 0x00,
	0x82, 0x99, 0x00,
	0x82, 0x9a, 0x00,
	0x82, 0x57, 0x00,
	0x82, 0x58, 0xC8,
	0x82, 0x59, 0xA0,
	0x82, 0x4c, 0x13,
	0x82, 0x4b, 0x36,
	0x82, 0x3d, 0x3c,
	0x82, 0x3e, 0x03,
	0x82, 0xbd, 0xA0,
	0x82, 0xbe, 0xc8,
	0x82, 0x9e, 0x00,
	0x82, 0x9f, 0x80,
	0x82, 0xa0, 0xf1,
	0x82, 0xa1, 0x6a,
	0x82, 0xa2, 0x00,
	0x82, 0xa3, 0x80,
	0x82, 0xa4, 0x90,
	0x82, 0xa5, 0x12,
	0x82, 0xa6, 0x16,
	0x82, 0xa7, 0xc2,
	0x82, 0xa8, 0x84,
	0x82, 0xa9, 0x80,
	0x82, 0xaa, 0x90,
	0x82, 0xab, 0x12,
	0x82, 0xac, 0x1c,
	0x82, 0xad, 0xc2,
	0x82, 0xae, 0x85,
	0x82, 0xaf, 0x80,
	0x82, 0xb0, 0x90,
	0x82, 0xb1, 0x12,
	0x82, 0xb2, 0x18,
	0x82, 0xb3, 0xc2,
	0x82, 0xb4, 0x85,
	0x82, 0xb5, 0x00,
	0x82, 0x4e, 0x55,
	0x82, 0x4f, 0x55,
	0x82, 0x50, 0x55,
	0x82, 0x51, 0x55,
	0x82, 0x24, 0x55,
	0x82, 0x25, 0x40,
	0x82, 0x26, 0xa1,
	
	// CLK1=8.33MHz
	// CLK2=83.33MHz, 80MHz < CLK2 < 88MHz
	// CLK3=83.33MHz
	// SYSCLK=41.67MHz
	0x82, 0x5c, 0x16,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,	
	//total pixel per line = 0x69b = 1691
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	
	0x82, 0x2d, 0x00,
	0x82, 0x2e, 0x00,
	0x82, 0x14, 0x40,
	0x82, 0x13, 0x85,  // AEC & AGC
	0x82, 0x96, 0xf9,  //  AWB
	0x82, 0x38, 0x10,  //  AWB
	
	0x82, 0xC3, 0x20,
	0x82, 0x55, 0xff,  // data pin out
	0x82, 0x56, 0x1f,  // PCLK/ VSYC/ HREF out
};

uint8_t ubSEN_PckSettingTable[] = {
	// 1.5MHz 1fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x1B,	
	// 3MHz 2fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x0D,	
	// 4.2MHz 3fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x08,	
	// 6MHz 4fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x06,	
	// 7MHz 5fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x05,	
	// 8.4MHz 6fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x04,	
	// 10.5MHz 7fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x03,	
	// 14MHz 8fps
	0x82, 0x2a, 0xec,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x02,	
	// 14MHz 9fps
	0x82, 0x2a, 0x56,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x02,	
	// 14MHz 10fps
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x02,	
	// 21MHz 11P
	0x82, 0x2a, 0x01,
	0x82, 0x2b, 0x09,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x01,		
	// 21MHz 12P
	0x82, 0x2a, 0x41,
	0x82, 0x2b, 0x08,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x01,	
	// 21MHz 13P
	0x82, 0x2a, 0x9E,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x01,	
	// 21MHz 14P
	0x82, 0x2a, 0x13,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x01,	
	// 21MHz 15P
	0x82, 0x2a, 0x98,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x01,	
	// 42MHz 16P
	0x82, 0x2a, 0x62,
	0x82, 0x2b, 0x0C,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 17P
	0x82, 0x2a, 0xA7,
	0x82, 0x2b, 0x0B,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 18P
	0x82, 0x2a, 0x02,
	0x82, 0x2b, 0x0B,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 19P
	0x82, 0x2a, 0x6D,
	0x82, 0x2b, 0x0A,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 20P
	0x82, 0x2a, 0xe8,
	0x82, 0x2b, 0x09,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 20P
	0x82, 0x2a, 0xe8,
	0x82, 0x2b, 0x09,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 21P
	0x82, 0x2a, 0x6f,
	0x82, 0x2b, 0x09,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 22P
	0x82, 0x2a, 0x01,
	0x82, 0x2b, 0x09,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 23P
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x08,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 24P
	0x82, 0x2a, 0x41,
	0x82, 0x2b, 0x08,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 25P
	0x82, 0x2a, 0xec,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 26P
	0x82, 0x2a, 0x9e,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 27P
	0x82, 0x2a, 0x56,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 28P
	0x82, 0x2a, 0x13,
	0x82, 0x2b, 0x07,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 29P
	0x82, 0x2a, 0xd5,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,
	// 42MHz 30P
	0x82, 0x2a, 0x9b,
	0x82, 0x2b, 0x06,
	0x82, 0x5d, 0x00,
	0x82, 0x11, 0x00,		
};

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Read(uint8_t ubAddress, uint8_t *pValue)
{
	return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &ubAddress, 1, pValue, 1);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Write(uint8_t ubAddress, uint8_t ubValue)
{		
	uint8_t ubData,pBuf[2];
	
	pBuf[0] = ubAddress;
	pBuf[1] = ubValue;
	
	// write data to sensor register
	bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, pBuf, 2, NULL, 0);
	
	// check if write success
	bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &ubAddress, 1, &ubData, 1);
	if (ubData != ubValue)
	{
		//printf("Sensor I2C write REG=0x%x with value=0x%x failed!\n", ubAddress, ubValue);
		return 0;
	}	
	return 1;	
}

//------------------------------------------------------------------------------
void SEN_PclkSetting(uint8_t ubPclkIdx)
{
	uint8_t i;	
	uint16_t uwIdx;
	
	switch (ubPclkIdx)
	{
		case SEN_FPS01:
			uwIdx = 0;
			sensor_cfg.ulSensorPclk = 1500000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 1;		
			break;
		case SEN_FPS02:
			uwIdx = 12;
			sensor_cfg.ulSensorPclk = 3000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 2;		
			break;
		case SEN_FPS03:
			uwIdx = 24;
			sensor_cfg.ulSensorPclk = 4200000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 3;		
			break;
		case SEN_FPS04:
			uwIdx = 36;
			sensor_cfg.ulSensorPclk = 6000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 4;		
			break;
		case SEN_FPS05:
			uwIdx = 48;
			sensor_cfg.ulSensorPclk = 7000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 5;		
			break;	
		case SEN_FPS06:
			uwIdx = 60;
			sensor_cfg.ulSensorPclk = 8400000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 6;		
			break;
		case SEN_FPS07:
			uwIdx = 72;
			sensor_cfg.ulSensorPclk = 10500000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 7;		
			break;
		case SEN_FPS08:
			uwIdx = 84;
			sensor_cfg.ulSensorPclk = 14000000;  
			sensor_cfg.ulSensorPclkPerLine = 2028;
			sensor_cfg.ulSensorFrameRate = 8;		
			break;
		case SEN_FPS09:
			uwIdx = 96;
			sensor_cfg.ulSensorPclk = 14000000;  
			sensor_cfg.ulSensorPclkPerLine = 1878;
			sensor_cfg.ulSensorFrameRate = 9;		
			break;
		case SEN_FPS10:
			uwIdx = 108;
			sensor_cfg.ulSensorPclk = 14000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 10;		
			break;	
		case SEN_FPS11:
			uwIdx = 120;
			sensor_cfg.ulSensorPclk = 21000000;  
			sensor_cfg.ulSensorPclkPerLine = 2305;
			sensor_cfg.ulSensorFrameRate = 11;		
			break;
		case SEN_FPS12:
			uwIdx = 132;
			sensor_cfg.ulSensorPclk = 21000000;  
			sensor_cfg.ulSensorPclkPerLine = 2113;
			sensor_cfg.ulSensorFrameRate = 12;		
			break;
		case SEN_FPS13:
			uwIdx = 144;
			sensor_cfg.ulSensorPclk = 21000000;  
			sensor_cfg.ulSensorPclkPerLine = 1950;
			sensor_cfg.ulSensorFrameRate = 13;		
			break;
		case SEN_FPS14:
			uwIdx = 156;
			sensor_cfg.ulSensorPclk = 21000000;  
			sensor_cfg.ulSensorPclkPerLine = 1811;
			sensor_cfg.ulSensorFrameRate = 14;		
			break;		
		case SEN_FPS15:
			uwIdx = 168;
			sensor_cfg.ulSensorPclk = 21000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 15;		
			break;
		case SEN_FPS16:
			uwIdx = 180;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 3170;
			sensor_cfg.ulSensorFrameRate = 16;		
			break;	
		case SEN_FPS17:
			uwIdx = 192;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2983;
			sensor_cfg.ulSensorFrameRate = 17;		
			break;
		case SEN_FPS18:
			uwIdx = 204;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2818;
			sensor_cfg.ulSensorFrameRate = 18;		
			break;
		case SEN_FPS19:
			uwIdx = 216;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2669;
			sensor_cfg.ulSensorFrameRate = 19;		
			break;		
		case SEN_FPS20:
			uwIdx = 228;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2536;
			sensor_cfg.ulSensorFrameRate = 20;				
			break;	
		case SEN_FPS21:
			uwIdx = 240;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2415;
			sensor_cfg.ulSensorFrameRate = 21;		
			break;	
		case SEN_FPS22:
			uwIdx = 252;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2305;
			sensor_cfg.ulSensorFrameRate = 22;		
			break;
		case SEN_FPS23:
			uwIdx = 264;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2205;
			sensor_cfg.ulSensorFrameRate = 23;		
			break;
		case SEN_FPS24:
			uwIdx = 276;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2113;
			sensor_cfg.ulSensorFrameRate = 24;		
			break;
		case SEN_FPS25:
			uwIdx = 288;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 2028;
			sensor_cfg.ulSensorFrameRate = 25;		
			break;	
		case SEN_FPS26:
			uwIdx = 300;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 1950;
			sensor_cfg.ulSensorFrameRate = 26;		
			break;
		case SEN_FPS27:
			uwIdx = 312;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 1878;
			sensor_cfg.ulSensorFrameRate = 27;		
			break;
		case SEN_FPS28:
			uwIdx = 324;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 1811;
			sensor_cfg.ulSensorFrameRate = 28;		
			break;	
		case SEN_FPS29:
			uwIdx = 336;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 1749;
			sensor_cfg.ulSensorFrameRate = 29;		
			break;			
		case SEN_FPS30:
			uwIdx = 348;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 30;		
			break;
		default:
			uwIdx = 348;
			sensor_cfg.ulSensorPclk = 42000000;  
			sensor_cfg.ulSensorPclkPerLine = 1691;
			sensor_cfg.ulSensorFrameRate = 30;				
			break;
	}
	
	for (i=uwIdx; i<(uwIdx+12); i+=3)
	{
		if (ubSEN_PckSettingTable[i] == 0x82)	// write
		{
			ulSEN_I2C_Write(ubSEN_PckSettingTable[i+1], ubSEN_PckSettingTable[i+2]);
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
uint8_t ubSEN_Start(struct SENSOR_SETTING *setting,uint8_t ubFPS)
{
	uint8_t   *pBuf;	
	uint16_t 	uwPID = 0;	
	uint32_t 	i;
	uint32_t 	ulValue;
		
	pI2C_type = pI2C_MasterInit (I2C_1, I2C_SCL_400K);
	
_RETRY:
	pBuf = (uint8_t*)&uwPID;    
	// I2C by read sensor ID
	ulSEN_I2C_Read (OV9715_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	ulSEN_I2C_Read (OV9715_CHIP_ID_LOW_ADDR, &pBuf[0]);   
	if (OV9715_CHIP_ID != uwPID)
	{
		printf("This is not OV9715 Sensor!! 0x%x 0x%x\n",OV9715_CHIP_ID, uwPID);
        TIMER_Delay_us(10000);
        goto _RETRY;
	}

	for (i=0; i<sizeof(ubSEN_InitTable); i+=3)
	{
		if (ubSEN_InitTable[i] == 0x82)	// write
		{
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2]);
		}
	}

	// set sensor output size
	ulValue = setting->ulVOSize / 4;
	ulSEN_I2C_Write(0x58, ulValue);
	ulValue = setting->ulHOSize / 8;
	ulSEN_I2C_Write(0x59, ulValue);	
	// set frame rate
	ulValue = 6;
	ulSEN_I2C_Write(0x11, ulValue);			

	// set pclk and frame rate
	SEN_PclkSetting(ubFPS);		
/*	
	CQ_Init();
	CQI2C_Init();

	// OV9715_WrTotalLine (MBS)
	ubBuf[0] = 0x3E;
	ubBuf[1] = 0x03;
	pWrDataSet[0] = CQI2C_CreateCQ_I2C_Write(SEN_SLAVE_ADDR, ubBuf, 2);

	// OV9715_WrTotalLine (LBS)
	ubBuf[0] = 0x3D;
	ubBuf[1] = 0x3D;
	pWrDataSet[1] = CQI2C_CreateCQ_I2C_Write(SEN_SLAVE_ADDR, ubBuf, 2);

	// SEN_GroupHoldOnVSync
	ubBuf[0] = 0x04;
	ubBuf[1] = 0x09;
	pWrDataSet[2] = CQI2C_CreateCQ_I2C_Write(SEN_SLAVE_ADDR, ubBuf, 2);

	CQI2C_Complete();
	CQI2C_Start();
*/
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting,uint8_t ubFPS)
{		
	// Set ISP clock
	SEN_SetISPRate(8);
	// Set parallel mode
	SEN->MIPI_MODE = 0;	
	// Set sensor clock
	SEN_SetSensorRate(SENSOR_96MHz, 12);	
	
	// Set change frame at Vsync rising/falling edge
	SEN->VSYNC_RIS = 1;
	// Set change frame at Hsync rising/falling edge
	SEN->HSYNC_RIS = 1;
	// Set Vsync high/low active
	SEN->VSYNC_HIGH = 1;
	// Set data latch at PCLK rising/falling edge
	SEN->PCK_DLH_RIS = 0;
	// Free run mode for PCLK
	SEN->SYNC_MODE = 1;		
	// Change ISP_LH_SEL to 0, for 10bit raw input
	SEN->ISP_LH_SEL = 0;																		// change ISP_LH_SEL to 1, for D2-D9 input
	SEN->ISP_HSTART_OFFSET = 0;
	SEN->ISP_MODE = 0;																			// Bayer pat mode	

	// Change RAW_REORDER to 0, start from B
	SEN->RAW_REORDER = 0;
	SEN->SENSOR_MODE = 1;																		// Buffer sync mode
	// Change to 10bit RAW
	SEN->RAW_BP_MODE = 1;
	SEN->RAW_BP_EN = 0;		
	// Input YUV data, and bypass to PostColor
	SEN->ISP_BRIG = 0;

	// Set dummy line & pixel
	SEN->DMY_DIV = 1; // dummy clock=sysclk/2
	SEN->NUM_DMY_LN = 24;
	SEN->NUM_DMY_DSTB = 1536;
	SEN->DMY_INSERT = 1;

	// Unable write to dram.
	SEN->IMG_TX_EN = 0;

	// Clear all debug flag
	SEN->REG_0x1300 = 0x1ff;
	SEN->REG_0x1304 = 0x3;
	// Enable sensor PCLK
	SEN->SEN_CLK_EN = 1;	

	// Delay 1ms
	TIMER_Delay_us(1000);
	// Initial dummy sensor
	if (ubSEN_Start(setting,ubFPS) != 1)
	{
		printf("OV9715 startup failed! \n\r");
		return 0;
	}			
	
	// Enable interrupt
	SEN->HW_END_INT_EN = 1;
	SEN->SEN_VSYNC_INT_EN = 1;
	SEN->SEN_HSYNC_INT_EN = 1;

	// Msut last beacause SEN_DSTB_TSEL is write only.
	SEN->SEN_DSTB_TSEL = 1;	
	return 1;
}

//------------------------------------------------------------------------------
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime)
{
	//Transfer the ExpLine of Algorithm	to ExpLine and DmyLine of Sensor here
	xtSENInst.xtSENCtl.ulExpTime = ulAlgExpTime;
	xtSENInst.xtSENCtl.uwExpLine = uwSEN_CalExpLine(ulAlgExpTime);

	if(xtSENInst.xtSENCtl.uwExpLine < 5)
	{
		xtSENInst.xtSENCtl.uwExpLine = 5;
	}	

	//Calculate Dummy line
	if(xtSENInst.xtSENCtl.uwExpLine > (xtSENInst.uwMaxExpLine - 2))
	{
		xtSENInst.xtSENCtl.uwDmyLine = (xtSENInst.xtSENCtl.uwExpLine - (xtSENInst.uwMaxExpLine - 2));
		xtSENInst.xtSENCtl.uwExpLine = (xtSENInst.uwMaxExpLine - 2);
	}
	else
	{
		xtSENInst.xtSENCtl.uwDmyLine = 0;
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
#if 1
	//MSB
	ulSEN_I2C_Write(0x3E, (uint8_t)((xtSENInst.uwMaxExpLine>>8) &0x00ff));
	TIMER_Delay_us(20);	
	//LSB
	ulSEN_I2C_Write(0x3D, (uint8_t)((xtSENInst.uwMaxExpLine) &0x00ff));
#else
	uint8_t ubData[2];
//	static uint8_t testCnt;
	
	// MSB
	ubData[0] = 0x3E;
	ubData[1] = (uint8_t)((xtSENInst.uwMaxExpLine>>8) &0x00ff);
	CQI2C_ModifyCQ_I2C_Write(pWrDataSet[0], ubData);

	// LSB
	ubData[0] = 0x3D;
	ubData[1] = (uint8_t)((xtSENInst.uwMaxExpLine) &0x00ff);
	//ubData[1] = testCnt++;
	CQI2C_ModifyCQ_I2C_Write(pWrDataSet[1], ubData);
#endif
}

//------------------------------------------------------------------------------
void SEN_WrDummyLine(uint16_t uwDL)
{
	//MSB
	ulSEN_I2C_Write(0x2E, (uint8_t)((uwDL>>8) &0x00ff));		
	TIMER_Delay_us(20);
	//LSB
	ulSEN_I2C_Write(0x2D, (uint8_t)(uwDL &0x00ff));		
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	//Set exposure line
	//MSB
	ulSEN_I2C_Write(0x16, (uint8_t)((uwExpLine>>8) & 0x00ff));
	TIMER_Delay_us(20);
	//LSB
	ulSEN_I2C_Write(0x10, (uint8_t)((uwExpLine) & 0x00ff));
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint16_t uwGainX1024)
{
	uint8_t ubAGaintmp;
	
	//Max globe gain is 16x gain
	if (uwGainX1024 < 1024)			                //Limit min value
	{
		uwGainX1024 = 1024;
	}	

	if(uwGainX1024 < 2048)							//analog 1x~2x
	{
		ubAGaintmp = 0x00+((uwGainX1024-1024)>>6);
	}
	else if(uwGainX1024 < 4096)					    //analog 2x~4x
	{		
		ubAGaintmp = 0x10+((uwGainX1024-2048)>>7);
	}	  
	else if(uwGainX1024 < 8192)					    //analog 4x~8x
	{
		ubAGaintmp = 0x30+((uwGainX1024-4096)>>8);
	}
	else if(uwGainX1024 < 16384)					//analog 8x~16x
	{
		ubAGaintmp = 0x70+((uwGainX1024-8192)>>9);
	}
	else if(uwGainX1024 < 32768)					//digital 16x~32x
	{			 
		ubAGaintmp = 0xf0+((uwGainX1024-16384)>>10);
	}
	else
	{
		ubAGaintmp = 0xff;
	}
	ulSEN_I2C_Write(0x00, ubAGaintmp);
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  OV9715_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~OV9715_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  OV9715_FLIP;
    else
        xtSENInst.ubImgMode &=  ~OV9715_FLIP;
    
    ulSEN_I2C_Write(0x04, xtSENInst.ubImgMode);
    //
    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{
#if 0
	uint8_t ubData[2];	
	ubData[0] = 0x04;
	ubData[1] = 0x09;
	CQI2C_ModifyCQ_I2C_Write(pWrDataSet[2], ubData);
#else
	uint8_t ubValue;
	ulSEN_I2C_Read(0x04, &ubValue);
	ubValue |= 0x09;
	ulSEN_I2C_Write(0x04, ubValue);	
#endif
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{
	uint8_t ubValue;

	ulSEN_I2C_Read(0x04, &ubValue);
	ubValue &= 0xFE;
	ulSEN_I2C_Write(0x04, ubValue);
	ulSEN_I2C_Write(0xFF, 0xFF);	
}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
	// Image for ISP
	sensor_cfg.ulHSize = ISP_WIDTH;
	sensor_cfg.ulVSize = ISP_HEIGHT;	
	// Output size from sensor.
	sensor_cfg.ulHOSize = 1280;
	sensor_cfg.ulVOSize = 800;	
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ulSensorType = SEN_OV9715;	
    printf("sensor type is OV9715\r\n");
}
#endif
