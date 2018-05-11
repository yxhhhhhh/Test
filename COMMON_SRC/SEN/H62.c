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
	\date		2017-11-23
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "SEN.h"
#include "I2C.h"
#include "TIMER.h"
#include "IQ_PARSER_API.h"

#if (SEN_USE == SEN_H62)
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
I2C_TYP I2C_Sel = I2C_2;

#define TRY_COUNTS 3
#define cbAE_MaxExpLTblColSZ		(sizeof(ctAE_MaxExpLTbl[0]))
#define cbAE_MaxExpLTblRowSZ		(sizeof(ctAE_MaxExpLTbl)/sizeof(ctAE_MaxExpLTbl[0]))

struct AE_ExpLineTblObj {
	unsigned char ubExpIdx;
	unsigned int swExpLineOffset;
};

static struct AE_ExpLineTblObj ctAE_MaxExpLTbl[] = {
	//FPS(DEC),	Max Exposure Offset(DEC, Sign),
	{30,    0},
	{29,	0},
	{28,	0},
	{27,	0},
	{26,	0},
	{25,	0},
	{24,	0},
	{23,	0},
	{22,	0},
	{21,	0},
	{20,	0},
	{19,	0},
	{18,	0},
	{17,	0},
	{16,	0},
	{15,	0},
	{14,	0},
	{13,	0},
	{12,	0},
	{11,	0},
	{10,	0},
	{9, 0},
	{8,	0},
	{7,	0},
	{6,	0},
	{5,	0},
	{4,	0},
	{3,	0},
	{2,	0},
	{1,	0},
};

uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
    // H62_24Minput_30fps_54MPclk_720p
    
	0x82, 0x12, 0x40,
    // VCO = (SenClk * 0x10[7:0]) / PLL_Pre_Ratio = (24MHz * 36) / (1+1) = 432MHz
    // where PLL_Pre_Ratio = 1 + 0x0E[1:0]
    // DAC VCO = VCO / 0x0F[5:3] 
	0x82, 0x0E, 0x11,//
	0x82, 0x0F, 0x08,
	0x82, 0x10, 0x24,
	0x82, 0x11, 0x80,
	0x82, 0x19, 0x68,
    // PClk = FrameW * FrameH * fps
    //      = 2400 * 750 * fps    
	0x82, 0x20, 0x60,
	0x82, 0x21, 0x09,
	0x82, 0x22, 0xEE,
	0x82, 0x23, 0x02,
    // window output
    // image window width = (0x26[3:0]<<8)+0x24[7:0] = 1288
    // image window high  = (0x26[7:4]<<8)+0x25[7:0] = 720
    // horizontal window start = (0x29[3:0]<<4)+0x27[7:0] = 750
    // vertical window start   = (0x29[7:4]<<4)+0x28[7:0] = 21     
	0x82, 0x24, 0x08,
	0x82, 0x25, 0xD0,
	0x82, 0x26, 0x25,
	0x82, 0x27, 0xEE,
	0x82, 0x28, 0x15,
	0x82, 0x29, 0x02,
    
	0x82, 0x2A, 0x70,
	0x82, 0x2B, 0x21,
	0x82, 0x2C, 0x08,
	0x82, 0x2D, 0x01,
	0x82, 0x2E, 0xBB,
	0x82, 0x2F, 0xC0,
	0x82, 0x41, 0x88,
	0x82, 0x42, 0x12,
	0x82, 0x39, 0x90,
	0x82, 0x1D, 0xFF,
	0x82, 0x1E, 0x9F,
	0x82, 0x7A, 0x80,
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
	0x82, 0x67, 0xF5,   //0xF8
	0x82, 0x68, 0x04,
	0x82, 0x69, 0x74,
	0x82, 0x6A, 0x3F,
	0x82, 0x63, 0x80,   //0x82
	0x82, 0x6C, 0xC0,
	0x82, 0x6E, 0x5C,
	0x82, 0x82, 0x01,
	0x82, 0x0C, 0x00,
	0x82, 0x46, 0xC2,    
    0x82, 0x47, 0x47,   //    
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
	//0x82, 0x47, 0x47, //
/*
	//sleep 500
    0xbb, 0x01, 0xF4,     
	0x82, 0x47, 0x44,
	0x82, 0x1F, 0x01,
	//0x82, 0x1F,0x80},
*/    
};

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Read(uint8_t ubAddress, uint8_t *pValue)
{	
	return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &ubAddress, 1, pValue, 1);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Write(uint8_t ubAddress, uint8_t ubValue)
{		
	uint8_t pBuf[2];
	
	pBuf[0] = ubAddress;
	pBuf[1] = ubValue;	

	// write data to sensor register
	return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_WriteTry(uint8_t ubAddress, uint8_t ubValue, uint8_t ubTryCnt)
{	
    uint8_t ubData, pBuf[2], i = 0;
    
    pBuf[0] = ubAddress;        
    pBuf[1] = ubValue;
    
    do{
        bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
        // read sensor register and check wirte success
        bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 1, &ubData, 1);      
    }while((ubData != ubValue)  || ((i++) >= ubTryCnt));
       
    if(i >= ubTryCnt)
    {
        printf("Sensor I2C write REG=0x%x failed! 0x%x 0x%x\n", ubAddress, ubData, ubValue);
        return 0; 
    }    		
    
	return 1;
}

//------------------------------------------------------------------------------
void SEN_PclkSetting(uint8_t ubPclkIdx)
{    
    uint16_t uwPPL;
    uint32_t ulPCK; 
    // support 1-30 fps for 1280x800
    if(ubPclkIdx > 30)
    {
        ubPclkIdx = 30;
    }
    
    // set sensor struct value
    sensor_cfg.ulSensorPclk = 54000000;  
    sensor_cfg.ulSensorPclkPerLine = 2400;
	sensor_cfg.ulSensorFrameRate = ubPclkIdx;     
    sensor_cfg.ulMaximumSensorFrameRate = 30;
    //auto calculat Max Exposure
	ulPCK = sensor_cfg.ulSensorPclk;
	uwPPL = (unsigned short)(sensor_cfg.ulSensorPclkPerLine);
       
    if ((ubPclkIdx == 0) || (ubPclkIdx > 30))
    {
        xtSENInst.uwMaxExpLine = (ulPCK / 30 / (unsigned int)uwPPL)+ctAE_MaxExpLTbl[0].swExpLineOffset;
    }else{
        xtSENInst.uwMaxExpLine = (ulPCK / sensor_cfg.ulSensorFrameRate / (unsigned int)uwPPL) + ctAE_MaxExpLTbl[ubPclkIdx].swExpLineOffset;
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
	uint8_t     *pBuf, temp;	
	uint16_t 	uwPID = 0;
	uint32_t 	i;	
	
	pI2C_type = pI2C_MasterInit(I2C_2, I2C_SCL_400K);
	IQ_SetI2cType(pI2C_type);
    
//_RETRY:    
	pBuf = (uint8_t*)&uwPID;  
	// I2C by Read Sensor ID
	ulSEN_I2C_Read (H62_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	ulSEN_I2C_Read (H62_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (H62_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "This is not H62 Sensor!! 0x%x 0x%x\n", H62_CHIP_ID, uwPID);
        TIMER_Delay_us(10000);
//        goto _RETRY;
	}	
    printd(DBG_CriticalLvl, "H62 Sensor\n");
 
    //stream off
    ulSEN_I2C_Read (0x74, &temp);
    temp |= (0x01<<7);
    ulSEN_I2C_Write(0x74, temp);
    //Please set this trigger to frame end
    ulSEN_I2C_Write(0x1F, 0x80);     
	//wait 33ms
    TIMER_Delay_ms(33);     
	for (i=0; i<sizeof(ubSEN_InitTable); i+=3)
	{
		if (ubSEN_InitTable[i] == 0x82)	// write
		{
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2]);
		}else if (ubSEN_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
        }
	}   
    // Set mirror and flip
    ulSEN_I2C_Write(0x12, 0x30);
    
	//wait 5*33.3ms
    //TIMER_Delay_ms(167);
    
    //stream on
    ulSEN_I2C_Read (0x74, &temp);
    temp &= ~(0x01<<7);
    ulSEN_I2C_Write(0x74, temp);

    //Please set this trigger to frame end
    ulSEN_I2C_Write(0x1F, 0x80); 
    
	SEN_PclkSetting(ubFPS);    
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting,uint8_t ubFPS)
{		
		// Set ISP clock
		SEN_SetISPRate(6);		
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

		// change SN_SEN->RAW_REORDER to 2, start from Gr
		SEN->RAW_REORDER = 2;
		SEN->SENSOR_MODE = 1;																		// Buffer sync mode
		// Change to 10bit RAW
		SEN->RAW_BP_MODE = 1;
		SEN->RAW_BP_EN = 0;		
		// Input YUV data, and bypass to PostColor
		SEN->ISP_BRIG = 0;

		// set dummy line & pixel
		SEN->DMY_DIV = 3; // dummy clock=sysclk/2
		SEN->NUM_DMY_LN = 16;
		SEN->NUM_DMY_DSTB = 256;
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
			printf("H62 startup failed! \n\r");
			return 0;
		}			
		// enable interrupt
		SEN->HW_END_INT_EN = 1;
		SEN->SEN_VSYNC_INT_EN = 1;
		SEN->SEN_HSYNC_INT_EN = 1;
        
		return 1;
}

//------------------------------------------------------------------------------
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime)
{
	//Transfor the ExpLine of Algorithm	to ExpLine and DmyLine of Sensor here
	xtSENInst.xtSENCtl.ulExpTime = ulAlgExpTime;
	xtSENInst.xtSENCtl.uwExpLine = uwSEN_CalExpLine(ulAlgExpTime);

	if(xtSENInst.xtSENCtl.uwExpLine < 1)
	{
		xtSENInst.xtSENCtl.uwExpLine = 1;
	}	
        
	//Calculate Dummy line
    if(xtSENInst.xtSENCtl.uwExpLine <= xtSENInst.uwMaxExpLine)
	{
        xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.uwMaxExpLine;
	}
	else
	{
        xtSENInst.xtSENCtl.uwDmyLine =xtSENInst.xtSENCtl.uwExpLine;
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
	ulSEN_I2C_WriteTry(0xc6, H62_FRAME_H, TRY_COUNTS);
	ulSEN_I2C_WriteTry(0xc7, (uint8_t)((uwDL>>8) & 0xff), TRY_COUNTS);
	TIMER_Delay_us(20);
	ulSEN_I2C_WriteTry(0xc8, H62_FRAME_L, TRY_COUNTS);
	ulSEN_I2C_WriteTry(0xc9, (uint8_t)(uwDL & 0xff), TRY_COUNTS);		
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	ulSEN_I2C_WriteTry(0xc0, H62_EXP_H, TRY_COUNTS);
	ulSEN_I2C_WriteTry(0xc1, (uint8_t)((uwExpLine>>8) & 0xff), TRY_COUNTS);
	
	ulSEN_I2C_WriteTry(0xc2, H62_EXP_L, TRY_COUNTS);
	ulSEN_I2C_WriteTry(0xc3, (uint8_t)(uwExpLine & 0xff), TRY_COUNTS);	
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
	ulSEN_I2C_WriteTry(0xc4, H62_GAIN, TRY_COUNTS);
	ulSEN_I2C_WriteTry(0xc5, (ubGaintmp & 0xff), TRY_COUNTS);
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
	sensor_cfg.ulHOSize = 1288;
	sensor_cfg.ulVOSize = 720;	
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ulSensorType = SEN_H62;
    printd(DBG_Debug1Lvl, "sensor type is H62\n");	
}
#endif
