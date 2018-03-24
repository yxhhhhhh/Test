/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SC2235.c
	\brief		Sensor SC2235 relation function
	\author		BoCun
	\version	1.1
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

#if (SEN_USE == SEN_SC2235)
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
//SC2235_24Minput_30fps_81MPclk_1080p       
	0x83, 0x01, 0x03, 0x01,
	0x83, 0x01, 0x00, 0x00,
	
	0x83, 0x36, 0x21, 0x28,
	
	0x83, 0x33, 0x09, 0x60,
	0x83, 0x33, 0x1f, 0x4d,
	0x83, 0x33, 0x21, 0x4f,
	0x83, 0x33, 0xb5, 0x10,
	
	0x83, 0x33, 0x03, 0x20,
	0x83, 0x33, 0x1e, 0x0d,
	0x83, 0x33, 0x20, 0x0f,
	
	0x83, 0x36, 0x22, 0x02,
	0x83, 0x36, 0x33, 0x42,
	0x83, 0x36, 0x34, 0x42,
	
	0x83, 0x33, 0x06, 0x66,
	0x83, 0x33, 0x0b, 0xd1,
	
	0x83, 0x33, 0x01, 0x0e,

	0x83, 0x32, 0x0c, 0x08,
	0x83, 0x32, 0x0d, 0x98,
	
	0x83, 0x33, 0x64, 0x05,// [2] 1: write at sampling ending
	
	0x83, 0x36, 0x3c, 0x28, //bypass nvdd
	0x83, 0x36, 0x3b, 0x0a, //HVDD
	0x83, 0x36, 0x35, 0xa0, //TXVDD
	
	0x83, 0x45, 0x00, 0x59,
	0x83, 0x3d, 0x08, 0x01, //0x00 -> 0x01 
	0x83, 0x36, 0x40, 0x00, //0x00 -> 0x01 add driven capability
	0x83, 0x39, 0x08, 0x11,
	
	0x83, 0x36, 0x3c, 0x08,
	
	0x83, 0x3e, 0x03, 0x03,
	0x83, 0x3e, 0x01, 0x46,
	
	
	//0x3e08,0x7f,
	//0x3e09,0x1f,
	0x83, 0x50, 0x00,0x00,//Disable Sensor DPC 
	//0x3908,0x31,
	
	//0703
	0x83, 0x33, 0x81, 0x0a,
	0x83, 0x33, 0x48, 0x09,
	0x83, 0x33, 0x49, 0x50,
	0x83, 0x33, 0x4a, 0x02,
	0x83, 0x33, 0x4b, 0x60,
	
	0x83, 0x33, 0x80, 0x04,
	0x83, 0x33, 0x40, 0x06,
	0x83, 0x33, 0x41, 0x50,
	0x83, 0x33, 0x42, 0x02,
	0x83, 0x33, 0x43, 0x60,
	
	//0707
	
	0x83, 0x36, 0x32, 0x88, //anti sm
	0x83, 0x33, 0x09, 0xa0,
	0x83, 0x33, 0x1f, 0x8d,
	0x83, 0x33, 0x21, 0x8f,
	
	
	0x83, 0x33, 0x5e, 0x01,  //ana dithering
	0x83, 0x33, 0x5f, 0x03,
	0x83, 0x33, 0x7c, 0x04,
	0x83, 0x33, 0x7d, 0x06,
	0x83, 0x33, 0xa0, 0x05,
	0x83, 0x33, 0x01, 0x05,
	
	//atuo logic
	
	0x83, 0x36, 0x70, 0x08, //[3]:3633 logic ctrl  real value in 3682
	0x83, 0x36, 0x7e, 0x07,  //gain0
	0x83, 0x36, 0x7f, 0x0f,  //gain1
	0x83, 0x36, 0x77, 0x2f,  //<gain0
	0x83, 0x36, 0x78, 0x22,  //gain0 - gain1
	0x83, 0x36, 0x79, 0x43,  //>gain1
	
	0x83, 0x33, 0x7f, 0x03, //new auto precharge  330e in 3372   [7:6] 11: close div_rst 00:open div_rst
	0x83, 0x33, 0x68, 0x02,
	0x83, 0x33, 0x69, 0x00,
	0x83, 0x33, 0x6a, 0x00,
	0x83, 0x33, 0x6b, 0x00,
	0x83, 0x33, 0x67, 0x08,
	0x83, 0x33, 0x0e, 0x30,
	
	0x83, 0x33, 0x66, 0x7c, // div_rst gap
	
	0x83, 0x36, 0x35, 0xc1,
	0x83, 0x36, 0x3b, 0x09,
	0x83, 0x36, 0x3c, 0x07,
	
	0x83, 0x39, 0x1e, 0x00,
	
	0x83, 0x36, 0x37, 0x14, //fullwell 7K
	
	0x83, 0x33, 0x06, 0x54,
	0x83, 0x33, 0x0b, 0xd8,
	0x83, 0x36, 0x6e, 0x08,  // ofs auto en [3]
	0x83, 0x36, 0x6f, 0x2f,  // ofs+finegain  real ofs in 0x3687[4:0]
	
	0x83, 0x36, 0x31, 0x84,
	0x83, 0x36, 0x30, 0x48,
	0x83, 0x36, 0x22, 0x06,
	
	
	//ramp by sc
	0x83, 0x36, 0x38, 0x1f,
	0x83, 0x36, 0x25, 0x02,
	0x83, 0x36, 0x36, 0x24,
	
	//0714
	0x83, 0x33, 0x48, 0x08,
	0x83, 0x3e, 0x03, 0x0b,//0x0b -> 0x03
	
	//7.17 fpn
	0x83, 0x33, 0x42, 0x03,
	0x83, 0x33, 0x43, 0xa0,
	0x83, 0x33, 0x4a, 0x03,
	0x83, 0x33, 0x4b, 0xa0,
	
	//0718
	0x83, 0x33, 0x43, 0xb0,
	0x83, 0x33, 0x4b, 0xb0,
	
	//0720	
	//digital ctrl
	0x83, 0x38, 0x02, 0x00,
	0x83, 0x32, 0x35, 0x04,
	0x83, 0x32, 0x36, 0x63, // vts-2
	
	//fpn
	0x83, 0x33, 0x43, 0xd0,
	0x83, 0x33, 0x4b, 0xd0,
	0x83, 0x33, 0x48, 0x07,
	0x83, 0x33, 0x49, 0x80,
	
	//0724
	0x83, 0x39, 0x1b, 0x4d,

	0x83, 0x33, 0x42, 0x04,
	0x83, 0x33, 0x43, 0x20,
	0x83, 0x33, 0x4a, 0x04,
	0x83, 0x33, 0x4b, 0x20,
	
	//0804
	0x83, 0x32, 0x22, 0x29,
	0x83, 0x39, 0x01, 0x02,
	
	//0808	
	//digital ctrl
	0x83, 0x3f, 0x00, 0x07,  // bit[2] = 1

	0x83, 0x3f, 0x04, 0x08,
	0x83, 0x3f, 0x05, 0x74,  // hts - 0x24
	
	//0809
	0x83, 0x33, 0x0b, 0xc8,
	//0817
	0x83, 0x33, 0x06, 0x4a,
	0x83, 0x33, 0x0b, 0xca,
	0x83, 0x36, 0x39, 0x09,
	
	//manual DPC
	0x83, 0x57, 0x80, 0xff,
	0x83, 0x57, 0x81, 0x04,  //
	0x83, 0x57, 0x85, 0x18,  //
	
	//0822
	0x83, 0x30, 0x39, 0x35,
	0x83, 0x30, 0x3a, 0x2e,
	0x83, 0x30, 0x34, 0x05,
	0x83, 0x30, 0x35, 0x2a,
	
	0x83, 0x32, 0x0c, 0x08,
	0x83, 0x32, 0x0d, 0xca,
	0x83, 0x32, 0x0e, 0x04,
	0x83, 0x32, 0x0f, 0xb0,
	
	0x83, 0x3f, 0x04, 0x08,
	0x83, 0x3f, 0x05, 0xa6, // hts - 0x24

	0x83, 0x32, 0x35, 0x04,
	0x83, 0x32, 0x36, 0xae, // vts-2
	
	//0825
    0x83, 0x33, 0x13, 0x05,
    0x83, 0x36, 0x78, 0x42,

    //for AE control per frame
    0x83, 0x36, 0x70, 0x00,
    0x83, 0x36, 0x33, 0x42,

    0x83, 0x38, 0x02, 0x00,	
    
    // output window height((1080 to 1088
    0x83, 0x32, 0x0a, 0x04,		
    0x83, 0x32, 0x0b, 0x40,		
	//0x83, 0x01, 0x00, 0x01,
};

//------------------------------------------------------------------------------
uint32_t ulSEN_I2C_Read(uint16_t uwAddress, uint8_t *pValue)
{
	uint8_t *pAddr, pBuf[2];
	
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
    sensor_cfg.ulSensorPclk = 81000000;  
    sensor_cfg.ulSensorPclkPerLine = 2250;
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
	uint8_t     ubTemp;	
	uint16_t 	uwPID = 0;
	uint32_t 	i;	
	
	pI2C_type = pI2C_MasterInit (I2C_2, I2C_SCL_400K);
	IQ_SetI2cType(pI2C_type);    
    
    // software reset
    ulSEN_I2C_Read (0x0103, &ubTemp);
    ubTemp |= (0x1<<0);
    ulSEN_I2C_Write(0x01, 0x03, ubTemp);
    TIMER_Delay_ms(30);
    
_RETRY:   
	// I2C by read sensor ID
	pBuf = (uint8_t*)&uwPID;    
	ulSEN_I2C_Read (SC2235_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	ulSEN_I2C_Read (SC2235_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (SC2235_CHIP_ID != uwPID)
	{
		printf("This is not SC2235 Sensor!! 0x%x 0x%x\n", SC2235_CHIP_ID, uwPID);
        TIMER_Delay_ms(10);
        goto _RETRY;
	}	
    printf("chip ID check ok!\r\n");

	// stop streaming(sleep mode
	ulSEN_I2C_Write(0x01, 0x00, 0x00); 
	
    // initial table
	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
		}
	}
    TIMER_Delay_ms(50);
	// start streaming
	ulSEN_I2C_Write(0x01, 0x00, 0x01);     
    // set frame rate
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
		SEN_SetSensorRate(SENSOR_96MHz, 4);	

		// Set change frame at Vsync rising/falling edge
		SEN->VSYNC_RIS = 0;
		// Set change frame at Hsync rising/falling edge
		SEN->HSYNC_RIS = 1;
		// Set Vsync high/low active
		SEN->VSYNC_HIGH = 0;
		// Set data latch at PCLK rising/falling edge
		SEN->PCK_DLH_RIS = 0;
		// Free run mode for PCLK
		SEN->SYNC_MODE = 1;		
	
		// Change ISP_LH_SEL to 0, for 10bit raw input
		SEN->ISP_LH_SEL = 0;																		// change ISP_LH_SEL to 1, for D2-D9 input
		SEN->ISP_HSTART_OFFSET = 0;
		SEN->ISP_MODE = 0;																			// Bayer pat mode

		// change SN_SEN->RAW_REORDER to 0, start from B
		SEN->RAW_REORDER = 1;
		SEN->SENSOR_MODE = 0;	//																	// Buffer sync mode
		// Change to 10bit RAW
		SEN->RAW_BP_MODE = 1;
		SEN->RAW_BP_EN = 0;		
		// Input YUV data, and bypass to PostColor
		SEN->ISP_BRIG = 0;

		// set dummy line & pixel
		SEN->DMY_DIV = 3;       // dummy clock=sysclk/2
		SEN->NUM_DMY_LN = 24;
		SEN->NUM_DMY_DSTB = 256;
		SEN->DMY_INSERT = 1;

		// Unable write to dram.
		SEN->IMG_TX_EN = 0;

		// clear all debug flag
		SEN->REG_0x1300 = 0x1ff;
		SEN->REG_0x1304 = 0x3;
		// enable sensor PCLK
		SEN->SEN_CLK_EN = 1;	
	
		// Initial dummy sensor
		if (ubSEN_Start(setting,ubFPS) != 1)
		{
			printf("SC2235 startup failed! \n\r");
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
	if(xtSENInst.xtSENCtl.uwExpLine > (xtSENInst.uwMaxExpLine - 4))
	{
		xtSENInst.xtSENCtl.uwDmyLine = (xtSENInst.xtSENCtl.uwExpLine + 4);
	}else{
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
    uint16_t uwTemp;
	//Set dummy lines
	ulSEN_I2C_WriteTry(SC2235_FRAME_LENGTH_H, (uint8_t)((uwDL>>8) &0x00ff), TRY_COUNTS);
	ulSEN_I2C_WriteTry(SC2235_FRAME_LENGTH_L, (uint8_t)(uwDL &0x00ff), TRY_COUNTS);     
    
    // for smartsense suggestion
    uwTemp = uwDL - 0x02;   
    ulSEN_I2C_WriteTry(SC2235_D_FRAME_LENGTH_H, (uint8_t)((uwTemp>>8) &0x00ff), TRY_COUNTS);  
    ulSEN_I2C_WriteTry(SC2235_D_FRAME_LENGTH_L, (uint8_t)(uwTemp &0x00ff), TRY_COUNTS);   
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{    
    static uint32_t ulOldExpLine = 0;
    uint8_t ubTemp;
    
    if(uwExpLine == ulOldExpLine)
        return;
    
    // set exposure
    ulSEN_I2C_WriteTry(SC2235_EXPH, (uint8_t)((uwExpLine>>12)&0x001f), TRY_COUNTS);
    ulSEN_I2C_WriteTry(SC2235_EXPM, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    ulSEN_I2C_WriteTry(SC2235_EXPL, (uint8_t)((uwExpLine&0x0f) << 4), TRY_COUNTS);
    
    // set trigger
    //step1~3
	ulSEN_I2C_Write(0x39, 0x03, 0x84);
	ulSEN_I2C_Write(0x39, 0x03, 0x04);    
    //step5
    ubTemp = ((uwExpLine >> 4) & 0XFF); 
    if(ubTemp < 0x05)
    {
        ulSEN_I2C_Write(0x33, 0x14, 0x12);
    }else if(ubTemp > 0x0a){
        ulSEN_I2C_Write(0x33, 0x14, 0x02);    
    }
    
    ulOldExpLine = uwExpLine;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
    static uint32_t ulOldGainValue = 0;
    uint16_t uwGain_H = 0;
    uint8_t ubGain_3e08 = 0x03;
    uint8_t ubGain_3e09 = 0x10;
    
    //	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}else if(ulGainX1024 > 63488){
        ulGainX1024 = 63488;
    }
    //
    if(ulGainX1024 == ulOldGainValue)
        return;

    //
    if(ulGainX1024 < 2048)              // x1~x2
    {
        uwGain_H = 1;
        ubGain_3e08 = 0x03;
    }else if(ulGainX1024 < 4096){       // x2~x4
        uwGain_H = 2;
        ubGain_3e08 = 0x07;    
    }else if(ulGainX1024 < 8192){       // x4~x8
        uwGain_H = 4;
        ubGain_3e08 = 0x0F;    
    }else if(ulGainX1024 < 16384){      // x8~x16
        uwGain_H = 8;
        ubGain_3e08 = 0x1F;    
    }else if(ulGainX1024 < 32768){      // x16~x32
        uwGain_H = 16;
        ubGain_3e08 = 0x3F;    
    }else if(ulGainX1024 < 65536){      // x32~x64
        uwGain_H = 32;
        ubGain_3e08 = 0x7F;    
    }
    ubGain_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<6)) | 0x10);
    
    // set gain
    ulSEN_I2C_WriteTry(SC2235_GAIN_H, ubGain_3e08, TRY_COUNTS);
    ulSEN_I2C_WriteTry(SC2235_GAIN_L, ubGain_3e09, TRY_COUNTS);
    
    // set trigger
    // step4
    if(((ubGain_3e08>>2)&0x07)==0x00)             // gain < 2
    {
        ulSEN_I2C_Write(0x33, 0x01, 0x0b);
        ulSEN_I2C_Write(0x36, 0x31, 0x84);
        ulSEN_I2C_Write(0x36, 0x6f, 0x2f);                                   
    }else if((((ubGain_3e08>>2)&0x07))<=0x03){	
        ulSEN_I2C_Write(0x33, 0x01, 0x14);
        ulSEN_I2C_Write(0x36, 0x31, 0x88);
        ulSEN_I2C_Write(0x36, 0x6f, 0x2f);      		   
    }else if((((ubGain_3e08>>2)&0x07)==0x07) && ubGain_3e09<=0x1e){
        ulSEN_I2C_Write(0x33, 0x01, 0x1c);
        ulSEN_I2C_Write(0x36, 0x31, 0x88);
        ulSEN_I2C_Write(0x36, 0x6f, 0x2f);  
    }else if((((ubGain_3e08>>2)&0x07)==0x07) && ubGain_3e09==0x1f){ 
        ulSEN_I2C_Write(0x33, 0x01, 0xff);
        ulSEN_I2C_Write(0x36, 0x31, 0x88);
        ulSEN_I2C_Write(0x36, 0x6f, 0x3c);            
    }    
    // save gain value    
    ulOldGainValue = ulGainX1024;
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
	sensor_cfg.ulHOSize = 1920;
	sensor_cfg.ulVOSize = 1088;
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ulSensorType = SEN_SC2235;	
    printf("sensor type is SC2235\r\n");
}
#endif
