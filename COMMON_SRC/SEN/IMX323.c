/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMX323.c
	\brief		Sensor IMX323 relation function
	\author		BoCun
	\version	1.3
	\date		2018-07-25
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
#include "IQ_PARSER_API.h"

#if (SEN_USE == SEN_IMX323)
#define TRY_COUNTS 3
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
I2C_TYP I2C_Sel = I2C_2;

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

#define PIX_H 2300
#define PIX_V 1125

#define PIX_H2 (PIX_H/2)
#define PIX_H4 (PIX_H/4)
uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
    // IMX323_1080p@30fps
    // For 40MHz input
	//  Pixel clock = 40Mhz * 2 = 80MHz
	
	0x83, 0x30, 0x00, 0x31, // Standby
	0x83, 0x01, 0x00, 0x00, // Mode_Sel Standby
	0x83, 0x30, 0x2C, 0x01, // Master mode standby

	0x83, 0x00, 0x08, 0x00,
	0x83, 0x00, 0x09, 0x00,
	0x83, 0x01, 0x01, 0x00,
	0x83, 0x01, 0x04, 0x00,
	0x83, 0x01, 0x12, 0x0A,
	0x83, 0x01, 0x13, 0x0A,
	0x83, 0x02, 0x02, 0x00,
	0x83, 0x02, 0x03, 0x00,


	0x83, 0x03, 0x40, (PIX_V>>8),
	0x83, 0x03, 0x41, (PIX_V&0xff),
	0x83, 0x03, 0x42, (PIX_H2>>8),
	0x83, 0x03, 0x43, (PIX_H2&0xff),

	0x83, 0x30, 0x01, 0x00,
	0x83, 0x30, 0x02, 0x0F,	
	0x83, 0x30, 0x03, (PIX_H2&0xff),
	0x83, 0x30, 0x04, (PIX_H2>>8),
	0x83, 0x30, 0x05, (PIX_V&0xff),
	0x83, 0x30, 0x06, (PIX_V>>8),

	0x83, 0x30, 0x07, 0x00,
	0x83, 0x30, 0x11, 0x00,
	0x83, 0x30, 0x12, 0x80,
	0x83, 0x30, 0x16, 0x3C,
	0x83, 0x30, 0x1F, 0x73,
	0x83, 0x30, 0x20, 0x3C,
	0x83, 0x30, 0x21, 0x80,
	0x83, 0x30, 0x22, 0x01, //0x00
	
	0x83, 0x30, 0x4F, 0x47,
	0x83, 0x30, 0x54, 0x14, //0x10
	0x83, 0x31, 0x1E, 0x00,
	
	0x83, 0x30, 0x27, 0x20,
	0x83, 0x30, 0x7A, 0x40,
	0x83, 0x30, 0x7B, 0x02,
	0x83, 0x30, 0x98, (PIX_H4&0xff),
	0x83, 0x30, 0x99, (PIX_H4>>8),
	0x83, 0x30, 0x9A, 0x4c,
	0x83, 0x30, 0x9B, 0x04,
	0x83, 0x30, 0xCE, 0x16,
	0x83, 0x30, 0xCF, 0x82,
	0x83, 0x30, 0xD0, 0x00,
    // 17-09-08
    0x83, 0x30, 0x3f, 0x0a, // imx323 only 

	// address	data
	0x83, 0x31, 0x17, 0x0D,
	
	0xbb, 0x00, 0x64, 0x00, // delay 100ms
	0x83, 0x30, 0x2C, 0x00,	// Master mode operation start
	0xbb, 0x00, 0x64, 0x00, // delay 100ms
	0x83, 0x30, 0x00, 0x30, // Register write:Valid
	0x83, 0x01, 0x00, 0x01,
};

#define cbAE_SensorVTblColSZ		(sizeof(ctSensor_SensorVTbl[0]))
#define cbAE_SensorVTblRowSZ		(sizeof(ctSensor_SensorVTbl)/sizeof(ctSensor_SensorVTbl[0]))
    
struct Sen_GainTblObj{
	uint16_t    uwAlgGain;
	uint8_t     ubSenGain;
};

struct Sen_GainTblObj ctSensor_SensorVTbl[] = {
	{512 ,   0 },
	{529 ,   1 },
	{548 ,   2 },
	{567 ,   3 },
	{587 ,   4 },
	{608 ,   5 },
	{629 ,   6 },
	{652 ,   7 },
	{674 ,   8 },
	{698 ,   9 },
	{723 ,   10},
	{748 ,   11},
	{774 ,   12},
	{802 ,   13},
	{830 ,   14},
	{859 ,   15},
	{889 ,   16},
	{921 ,   17},
	{953 ,   18},
	{986 ,   19},
	{1021,   20},
	{1057,   21},
	{1094,   22},
	{1133,   23},
	{1172,   24},
	{1214,   25},
	{1256,   26},
	{1300,   27},
	{1346,   28},
	{1394,   29},
	{1443,   30},
	{1493,   31},
	{1546,   32},
	{1600,   33},
	{1656,   34},
	{1715,   35},
	{1775,   36},
	{1837,   37},
	{1902,   38},
	{1969,   39},
	{2038,   40},
	{2109,   41},
	{2184,   42},
	{2260,   43},
	{2340,   44},
	{2422,   45},
	{2507,   46},
	{2595,   47},
	{2687,   48},
	{2781,   49},
	{2879,   50},
	{2980,   51},
	{3085,   52},
	{3193,   53},
	{3305,   54},
	{3421,   55},
	{3542,   56},
	{3666,   57},
	{3795,   58},
	{3928,   59},
	{4066,   60},
	{4209,   61},
	{4357,   62},
	{4510,   63},
	{4669,   64},
	{4833,   65},
	{5003,   66},
	{5179,   67},
	{5361,   68},
	{5549,   69},
	{5744,   70},
	{5946,   71},
	{6155,   72},
	{6371,   73},
	{6595,   74},
	{6827,   75},
	{7067,   76},
	{7315,   77},
	{7573,   78},
	{7839,   79},
	{8114,   80},
	{8399,   81},
	{8695,   82},
	{9000,   83},
	{9316,   84},
	{9644,   85},
	{9983,   86},
	{10334,  87},
	{10697,  88},
	{11073,  89},
	{11462,  90},
	{11865,  91},
	{12282,  92},
	{12713,  93},
	{13160,  94},
	{13622,  95},
	{14101,  96},
	{14597,  97},
	{15110,  98},
	{15641,  99},
	{16190, 100},
	{16759, 101},
	{17348, 102},
	{17958, 103},
	{18589, 104},
	{19242, 105},
	{19919, 106},
	{20619, 107},
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
	uint8_t pBuf[3];
	
	pBuf[0] = ubAddress1;
	pBuf[1] = ubAddress2;
	pBuf[2] = ubValue;	

	// write data to sensor register
	return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);		
}

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_WriteTry(uint16_t uwAddress, uint8_t ubValue, uint8_t ubTryCnt)
{	
    uint8_t ubData, pBuf[3], i = 0;
    
    pBuf[0] = (uint8_t)((uwAddress>>8) & 0x00ff);        
    pBuf[1] = (uwAddress & 0x00ff);
    pBuf[2] = ubValue;
    
    do{
        bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);
        // read sensor register and check wirte success
        bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, &ubData, 1);      
    }while((ubData != ubValue)  || ((i++) >= ubTryCnt));
       
    if(i >= ubTryCnt)
    {
        printf("Sensor I2C write REG=0x%x failed! 0x%x 0x%x\n", uwAddress, ubData, ubValue);
        return 0; 
    }    		
    
	return 1;
}
//------------------------------------------------------------------------------
void SEN_PclkSetting(uint8_t ubPclkIdx)
{
    uint16_t uwPPL;
    uint32_t ulPCK;   
    // support 1-30 fps for 1920x1080
    if(ubPclkIdx > 30)
    {
        ubPclkIdx = 30;
    }
    // set sensor struct value
    sensor_cfg.ulSensorPclk = 80000000;  
    sensor_cfg.ulSensorPclkPerLine = 2300;
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
uint8_t ubSEN_Start(struct SENSOR_SETTING *setting, uint8_t ubFPS)
{
	uint8_t     *pBuf;	
	uint8_t 	ubPID = 0;
	uint32_t 	i;	
	
    // Init I2C2
	pI2C_type = pI2C_MasterInit (I2C_2, I2C_SCL_400K);
    IQ_SetI2cType(pI2C_type);

_RETRY:
	pBuf = (uint8_t*)&ubPID;   
	ulSEN_I2C_Read (IMX323_CHIP_MODELID_ADDR, &pBuf[0]);
	if (IMX323_CHIP_MODELID != ubPID)
	{
		printf("This is not IMX323 Sensor!! 0x%x 0x%x\n", IMX323_CHIP_MODELID, ubPID);
        TIMER_Delay_us(10000);
        goto _RETRY;
	}	

	ulSEN_I2C_Write(0x30, 0x00, 0x31);	// Standby
	ulSEN_I2C_Write(0x01, 0x00, 0x00);	// Mode_Sel Standby
	ulSEN_I2C_Write(0x30, 0x2C, 0x01);	// Master mode standby

	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
		}else if (ubSEN_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
        }
	}	
    
    SEN_PclkSetting(ubFPS);
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting, uint8_t ubFPS)
{		
		// Set ISP clock
		SEN_SetISPRate(5);		
		// Set parallel mode
		SEN->MIPI_MODE = 0;
		// Set sensor clock
		SEN_SetSensorRate(SENSOR_120MHz, 3);	

		// Set change frame at Vsync rising/falling edge
		SEN->VSYNC_RIS = 0;
		// Set change frame at Hsync rising/falling edge
		SEN->HSYNC_RIS = 0;
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
		SEN->SENSOR_MODE = 0;																		// Buffer sync mode
		// Change to 10bit RAW
		SEN->RAW_BP_MODE = 1;
		SEN->RAW_BP_EN = 0;		
		// Input YUV data, and bypass to PostColor
		SEN->ISP_BRIG = 0;

		// Set dummy line & pixel
		SEN->DMY_DIV = 3; // dummy clock=sysclk/2
		SEN->NUM_DMY_LN = 16;
		SEN->NUM_DMY_DSTB = 256;
		SEN->DMY_INSERT = 0;

		// Unable write to dram.
		SEN->IMG_TX_EN = 0;

		// Clear all debug flag
		SEN->REG_0x1300 = 0x1ff;
		SEN->REG_0x1304 = 0x3;
		// Enable sensor PCLK
		SEN->SEN_CLK_EN = 1;	
	
		// Initial dummy sensor
		if (ubSEN_Start(setting,ubFPS) != 1)
		{
			printf("IMX323 startup failed! \n\r");
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

	//Calculate Dummy line
    if(xtSENInst.xtSENCtl.uwExpLine > xtSENInst.uwMaxExpLine)
	{
        xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.xtSENCtl.uwExpLine;
	}
	else
	{
        xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.uwMaxExpLine;
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
    //Set Dummy Line         
    ulSEN_I2C_WriteTry(IMX322_FRAME_LENGTH_H, (uint8_t)((uwDL>>8) & 0x00ff), TRY_COUNTS);		
    ulSEN_I2C_WriteTry(IMX322_FRAME_LENGTH_L, (uint8_t)((uwDL) & 0x00ff), TRY_COUNTS);	   
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	uint16_t uwTargetExpLine;
    static uint16_t uwOldExpLine = 0;
    
	if(uwExpLine <= xtSENInst.uwMaxExpLine)
	{
	   uwTargetExpLine = xtSENInst.uwMaxExpLine - uwExpLine;
	}else{
	   uwTargetExpLine = 0;
	}		
    
    if(uwTargetExpLine == uwOldExpLine)
        return;
    // set exposure
    ulSEN_I2C_WriteTry(IMX322_DUMMY_LINE_H, (uint8_t)(((uwTargetExpLine)>>8)&0x00ff), TRY_COUNTS);	
    ulSEN_I2C_WriteTry(IMX322_DUMMY_LINE_L, (uint8_t)((uwTargetExpLine)&0x00ff), TRY_COUNTS);     
    
    uwOldExpLine = uwTargetExpLine;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	uint8_t ubIdx;
    static uint32_t ulOldGainValue = 0;
    
	ulGainX1024 = ulGainX1024/2;
	if(ulGainX1024 < 512)
	{
		ulGainX1024 = 512;
	}

    if(ulGainX1024 == ulOldGainValue)
        return;

	for(ubIdx = 0; ubIdx<cbAE_SensorVTblRowSZ; ubIdx++)
	{
		if(ulGainX1024 <= ctSensor_SensorVTbl[ubIdx].uwAlgGain)
		{
			xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx].ubSenGain;
			break;
		}
	}

	if(ubIdx == cbAE_SensorVTblRowSZ)
	{
		xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx-1].ubSenGain;		
	}
    
    ulSEN_I2C_WriteTry(IMX322_GAIN, xtSENInst.ubBuf[2], TRY_COUNTS);
    //save gain value
    ulOldGainValue = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  IMX322_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~IMX322_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  IMX322_FLIP;
    else
        xtSENInst.ubImgMode &=  ~IMX322_FLIP;
    
    ulSEN_I2C_Write(0x01, 0x01, xtSENInst.ubImgMode);
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

}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
	// Image for ISP
	sensor_cfg.ulHSize = ISP_WIDTH;
	sensor_cfg.ulVSize = ISP_HEIGHT;	
	// Output size from sensor.
	sensor_cfg.ulHOSize = 1984;
	sensor_cfg.ulVOSize = 1152;	
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ulSensorType = SEN_IMX323;	
    printf("sensor type is IMX323\r\n");
}
#endif
