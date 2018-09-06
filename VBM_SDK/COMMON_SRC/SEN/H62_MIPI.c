/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		H62.c
	\brief		Sensor H62 relation function
	\author		BoCun
	\version	1
	\date		2018-07-06
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
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

#if (SEN_USE == SEN_H62_MIPI)
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
I2C_TYP I2C_Sel = I2C_1;

uint8_t ubSEN_InitTable[] = {
	0x82, 0x12, 0x40,
	0x82, 0x0E, 0x11,
	0x82, 0x0F, 0x09,
	0x82, 0x10, 0x2D,
	0x82, 0x11, 0x80,
	0x82, 0x19, 0x68,
	0x82, 0x20, 0x60,
	0x82, 0x21, 0x09,
	0x82, 0x22, 0xEE,
	0x82, 0x23, 0x02,
	0x82, 0x24, 0x00, //Image horizontal output window width LSBs: HWin[7:0]
	0x82, 0x25, 0xD0, //Image vertical output window high LSBs: VWin[7:0]
	0x82, 0x26, 0x25, //Image output window horizontal and vertical MSBs. { VWin[11:8],HWin[11:8]}
	0x82, 0x27, 0xF4,
	0x82, 0x28, 0x15,
	0x82, 0x29, 0x02,
	0x82, 0x2A, 0x70,
	0x82, 0x2B, 0x21,
	0x82, 0x2C, 0x0A,
	0x82, 0x2D, 0x01,
	0x82, 0x2E, 0xBB,
	0x82, 0x2F, 0xC0,
	0x82, 0x41, 0x88,
	0x82, 0x42, 0x12,
	0x82, 0x39, 0x90,
	0x82, 0x1D, 0x00,
	0x82, 0x1E, 0x00,
	0x82, 0x7A, 0x4C,
	0x82, 0x70, 0xA9,
	0x82, 0x71, 0x6A,
	0x82, 0x72, 0x88,
	0x82, 0x73, 0x43,
	0x82, 0x74, 0xD2,
	0x82, 0x75, 0x2B,
	0x82, 0x76, 0x40,
	0x82, 0x77, 0x06,
	0x82, 0x78, 0x20,
	0x82, 0x66, 0x08,
	0x82, 0x1F, 0x00,
	0x82, 0x31, 0x0C,
	0x82, 0x33, 0x0C,
	0x82, 0x34, 0x2F,
	0x82, 0x35, 0xA3,
	0x82, 0x36, 0x05,
	0x82, 0x38, 0x53,
	0x82, 0x3A, 0x08,
	0x82, 0x56, 0x02,
	0x82, 0x60, 0x02,
	0x82, 0x0D, 0x50,
	0x82, 0x57, 0x80,
	0x82, 0x58, 0x33,
	0x82, 0x5A, 0x04,
	0x82, 0x5B, 0xB6,
	0x82, 0x5C, 0x08,
	0x82, 0x5D, 0x67,
	0x82, 0x5E, 0x04,
	0x82, 0x5F, 0x08,
	0x82, 0x66, 0x28,
	0x82, 0x67, 0xF8,
	0x82, 0x68, 0x00,
	0x82, 0x69, 0x74,
	0x82, 0x6A, 0x3F,
	0x82, 0x63, 0x82,
	0x82, 0x6C, 0xC0,
	0x82, 0x6E, 0x5C,
	0x82, 0x82, 0x01,
	
	/* test pattern */
	0x82, 0x0C, 0x00,
	/*0x82, 0x0C, 0x01,*/
	
	0x82, 0x46, 0xC2,
	0x82, 0x48, 0x7E,
	0x82, 0x62, 0x40,
	0x82, 0x7D, 0x57,
	0x82, 0x7E, 0x28,
	0x82, 0x80, 0x00,
	0x82, 0x4A, 0x05,
	0x82, 0x49, 0x10,
	0x82, 0x13, 0x81,
	0x82, 0x59, 0x97,
	0x82, 0x12, 0x00,
	0xbb, 0x01, 0xF4,/*sleep 500*/
		//{0xC0, 0x74},
		//{0xC1, 0x52},
		//sleep 500
	0x82, 0x47, 0x44,
	0x82, 0x1F, 0x01,
		//{0x1F, 0x80},
};

uint8_t ubSEN_PckSettingTable[] = {
	// 27MHz 15P
	0x82, 0x0E, 0x13,
	0x82, 0x20, 0x60,       // FrameH h
	0x82, 0x21, 0x09,		// FrameH l	
	// 54MHz 20P
	0x82, 0x0E, 0x12,
	0x82, 0x20, 0x60,       // FrameH h
	0x82, 0x21, 0x09,		// FrameH l	   
	// 54MHz 25P
	0x82, 0x0E, 0x11,
	0x82, 0x20, 0x40,       // FrameH h
	0x82, 0x21, 0x0B,		// FrameH l	  
	// 54MHz 30P
	0x82, 0x0E, 0x11,
	0x82, 0x20, 0x60,       // FrameH h
	0x82, 0x21, 0x09,		// FrameH l				
};

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Read(uint8_t ubAddress, uint8_t *pValue)
{
	
	return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &ubAddress, 1, pValue, 1);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Write(uint8_t ubAddress, uint8_t ubValue)
{		
	uint8_t ubData, pBuf[2];
	
	pBuf[0] = ubAddress;
	pBuf[1] = ubValue;	

	// write data to sensor register
	bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
	
	// check if write success
	bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 1, &ubData, 1);	
	if (ubData != ubValue)
	{
		//printf("wr \r\n");
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
			sensor_cfg.ulSensorPclk = 27000000;  
			sensor_cfg.ulSensorPclkPerLine = 2400;
			sensor_cfg.ulSensorFrameRate = 15;
			break;
		case SEN_FPS20:
			ubIdx = 9;
			sensor_cfg.ulSensorPclk = 36000000;  
			sensor_cfg.ulSensorPclkPerLine = 2400;
			sensor_cfg.ulSensorFrameRate = 20;
			break; 
		case SEN_FPS25:
			ubIdx = 18;
			sensor_cfg.ulSensorPclk = 54000000;  
			sensor_cfg.ulSensorPclkPerLine = 2880;
			sensor_cfg.ulSensorFrameRate = 25;
			break;        
		case SEN_FPS30:
			ubIdx = 27;
			sensor_cfg.ulSensorPclk = 54000000;  
			sensor_cfg.ulSensorPclkPerLine = 2400;
			sensor_cfg.ulSensorFrameRate = 30;
			break;
		default:
			ubIdx = 27;
			sensor_cfg.ulSensorPclk = 54000000;  
			sensor_cfg.ulSensorPclkPerLine = 2400;
			sensor_cfg.ulSensorFrameRate = 30;
			break;
	}
	
	for (i=ubIdx; i<(ubIdx+9); i+=3)
	{
		if (ubSEN_PckSettingTable[i] == 0x82)	// write
		{
			ulSEN_I2C_Write(ubSEN_PckSettingTable[i+1], ubSEN_PckSettingTable[i+2]);
		}
	}
    //Please set this trigger to frame end
    ulSEN_I2C_Write(0x1F, 0x80); 
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
	uint8_t     *pBuf,temp;	
	uint16_t 	uwPID = 0;
	uint32_t 	i;
	//uint8_t ubDelayCellSelForClk;
	
	pI2C_type = pI2C_MasterInit (I2C_2, I2C_SCL_400K);
	IQ_SetI2cType(pI2C_type);
_RETRY:    
	pBuf = (uint8_t*)&uwPID;  
	// I2C by Read Sensor ID
	ulSEN_I2C_Read (H62_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	ulSEN_I2C_Read (H62_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (H62_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "This is not H62 MIPI Sensor!! 0x%x 0x%x\n", H62_CHIP_ID, uwPID);
        TIMER_Delay_us(10000);
        goto _RETRY;
	}	
    printf("chip ID[0x%04X] check ok!\r\n",uwPID);

	//software reset
	ulSEN_I2C_Read (0x12, &temp);
	temp |= (0x1<<7);
	ulSEN_I2C_Write(0x12, temp);
	
    //stream off
    ulSEN_I2C_Read (0x74, &temp);
    temp |= (0x01<<7);
	ulSEN_I2C_Write(0xCE, 0x74);
	ulSEN_I2C_Write(0xCF, temp);
	
    //Please set this trigger to frame end
    ulSEN_I2C_Write(0x1F, 0x80); 
    
	//wait 33ms
    TIMER_Delay_ms(33);
	
	//fill initial table
	for (i=0; i<sizeof(ubSEN_InitTable); i+=3)
	{
		if (ubSEN_InitTable[i] == 0x82)	// write
		{
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2]);
		}else if (ubSEN_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
        }
	}
	
	//wait 5*33.3ms
    TIMER_Delay_ms(167);
	
    //stream on
    ulSEN_I2C_Read (0x74, &temp);
    temp &= ~(0x01<<7);
	ulSEN_I2C_Write(0xCE, 0x74);
    ulSEN_I2C_Write(0xCF, temp);
	
    //Please set this trigger to frame end
    ulSEN_I2C_Write(0x1F, 0x80);
    
	//FPS setting	
	SEN_PclkSetting(ubFPS);
	
	//-------------------------------------------------------
	// chinwei add
	//-------------------------------------------------------
#if 0
	if( ubFPS==15 )
	{
		ubDelayCellSelForClk = 7;
		MIPI->CLK_SEL0 = (ubDelayCellSelForClk & 0x01)>>0;
		MIPI->CLK_SEL1 = (ubDelayCellSelForClk & 0x02)>>1;
		MIPI->CLK_SEL2 = (ubDelayCellSelForClk & 0x04)>>2;
		MIPI->CLK_SEL3 = (ubDelayCellSelForClk & 0x08)>>3;
		MIPI->CLK_SEL4 = (ubDelayCellSelForClk & 0x10)>>4;
		MIPI->DLAN_PDD_LSCALE_SEL0 = 16;
	}
	else if( ubFPS==30 )
	{
		ubDelayCellSelForClk = 14;
		MIPI->CLK_SEL0 = (ubDelayCellSelForClk & 0x01)>>0;
		MIPI->CLK_SEL1 = (ubDelayCellSelForClk & 0x02)>>1;
		MIPI->CLK_SEL2 = (ubDelayCellSelForClk & 0x04)>>2;
		MIPI->CLK_SEL3 = (ubDelayCellSelForClk & 0x08)>>3;
		MIPI->CLK_SEL4 = (ubDelayCellSelForClk & 0x10)>>4;
		MIPI->DLAN_PDD_LSCALE_SEL0 = 6;
	}
	else
	{
		MIPI->DLAN_PDD_LSCALE_SEL0 = 4;
	}
	printf("-------------------------------\n");
	printf("CLK select:%d\n",ubDelayCellSelForClk);
	printf("PDD select:%d\n",MIPI->DLAN_PDD_LSCALE_SEL0);
	printf("-------------------------------\n");
#else
	MIPI_PowerDownTimingSelect(ubFPS);
	MIPI_AutoPhaseDetect();
#endif	
	
	TIMER_Delay_ms(100);	// wait 100 ms to become stable	
	//-------------------------------------------------------
	
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting,uint8_t ubFPS)
{	
	// Set ISP clock
	SEN_SetISPRate(4);
	// Set mipi mode
	SEN->MIPI_MODE = 1;
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
	SEN->SYNC_MODE = 0;

	// Change ISP_LH_SEL to 0, for 10bit raw input
	SEN->ISP_LH_SEL = 0;																		// change ISP_LH_SEL to 1, for D2-D9 input
	SEN->ISP_HSTART_OFFSET = 0;
	SEN->ISP_MODE = 0;																			// Bayer pat mode

	// change SN_SEN->RAW_REORDER to 2, start from Gr
	SEN->RAW_REORDER = 0;
	SEN->SENSOR_MODE = 1;																		// Buffer sync mode
	// Change to 10bit RAW
	SEN->RAW_BP_MODE = 1;
	SEN->RAW_BP_EN = 0;		
	// Input YUV data, and bypass to PostColor
	SEN->ISP_BRIG = 0;

	// set dummy line & pixel
	SEN->DMY_DIV = 2;
	SEN->NUM_DMY_LN = 16;
	SEN->NUM_DMY_DSTB = 256;
	SEN->DMY_INSERT = 1;

	//-------------------------------------------------------
	// chinwei add
	//-------------------------------------------------------
	MIPI->MIPI_EN = 0;
	MIPI->MIPI_PD = 0;
	printf("-------MIPI_EN=%d-------\n",MIPI->MIPI_EN);
	
	MIPI->DLAN_NUM = 0;
	MIPI->DATA_CHANNEL0_SEL = 0;
	MIPI->LANE0_CLK_SEL = 0;
	
	TIMER_Delay_us(5000);;	// wait 5 ms to become stable
	MIPI->MIPI_EN = 1;
	printf("-------MIPI_EN=%d-------\n",MIPI->MIPI_EN);
	//-------------------------------------------------------

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
		printf("H62 mipi startup failed! \n\r");
		return 0;
	}			
	// enable interrupt
	SEN->HW_END_INT_EN = 1;
	SEN->SEN_VSYNC_INT_EN = 1;
	SEN->SEN_HSYNC_INT_EN = 1;

 //msut last beacause SEN_DSTB_TSEL is write only.
	//SEN->SEN_DSTB_TSEL = 1;	
	return 1;
}

//------------------------------------------------------------------------------
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime)
{
	//Transfor the ExpLine of Algorithm	to ExpLine and DmyLine of Sensor here
	xtSENInst.xtSENCtl.ulExpTime = ulAlgExpTime;
    // calc exposure lines
	xtSENInst.xtSENCtl.uwExpLine = uwSEN_CalExpLine(ulAlgExpTime);

	if(xtSENInst.xtSENCtl.uwExpLine < 1)
	{
		xtSENInst.xtSENCtl.uwExpLine = 1;
	}	

	//Calculate Dummy line
	if(xtSENInst.xtSENCtl.uwExpLine > SEN_MAX_EXPLINE)
	{
		xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.xtSENCtl.uwExpLine;
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
	
	ulSEN_I2C_Write(0xc6, 0x23);
	ulSEN_I2C_Write(0xc7, (uint8_t)((uwDL>>8) & 0xff));
	TIMER_Delay_us(20);
	ulSEN_I2C_Write(0xc8, 0x22);
	ulSEN_I2C_Write(0xc9, (uint8_t)(uwDL & 0xff));	
	
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
    //Set long exposure
	
	ulSEN_I2C_Write(0xc0, 0x02);
	ulSEN_I2C_Write(0xc1, (uint8_t)((uwExpLine>>8) & 0xff));
	
	ulSEN_I2C_Write(0xc2, 0x01);
	ulSEN_I2C_Write(0xc3, (uint8_t)(uwExpLine & 0xff));
	
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	uint8_t  ubGaintmp;
		
	//avoid warning
	ubGaintmp = ubGaintmp;
	
	//	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}		
		
    if(ulGainX1024 < 2048)							            //analog 1x~2x
    {
        ubGaintmp = 0x00+((ulGainX1024-1024)>>6);
    }
    else if(ulGainX1024 < 4096)					                //analog 2x~4x
    {		
        ubGaintmp = 0x10+((ulGainX1024-2048)>>7);
    }
    else if(ulGainX1024 < 8192)					                //analog 4x~8x
    {
        ubGaintmp = 0x20+((ulGainX1024-4096)>>8);
    }
    else if(ulGainX1024 < 16384)					            //analog 8x~16x
    {
        ubGaintmp = 0x30+((ulGainX1024-8192)>>9);
    }
    else if(ulGainX1024 < 32768)					            //digital*analog 16x~32x
    {			 
        ubGaintmp = 0x40+((ulGainX1024-16384)>>9);
    }		
    else
    {														    //digital*analog 32x~64x
        ubGaintmp = 0x4f;							
    }
	// gain	
	ulSEN_I2C_Write(0xc4, 0x00);
	ulSEN_I2C_Write(0xc5, (ubGaintmp & 0xff));
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  H62_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~H62_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  H62_FLIP;
    else
        xtSENInst.ubImgMode &=  ~H62_FLIP;
    
    ulSEN_I2C_Write(0x12, xtSENInst.ubImgMode);
    //
    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{

}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{
	
	ulSEN_I2C_Write(0x1f, 0x80);
	
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
    sensor_cfg.ulSensorType = SEN_H62_MIPI;
    printd(DBG_Debug1Lvl, "sensor type is H62 mipi\n");	
}
#endif
