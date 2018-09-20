/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BUF.c
	\brief		Buffer access function
	\author		Justin Chen
	\version	0.3
	\date		2018/07/25
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#include "BUF.h"
#include "DDR_API.h"

//------------------------------------------------------------------------------
/*!	\file BUF.c	
BUF Initial Flow Chart:	
	\dot
		digraph G {
	node [shape=record,fontsize=10];
	"BUF_Init" -> "BUF_BufInit";		
	}
	\enddot
*/
//------------------------------------------------------------------------------
osSemaphoreId tBUF_AdcBufAcc;
osSemaphoreId tBUF_Dac0BufAcc;
osSemaphoreId tBUF_Dac1BufAcc;
osSemaphoreId tBUF_Dac2BufAcc;
osSemaphoreId tBUF_Dac3BufAcc;
osSemaphoreId tBUF_Dac4BufAcc;
osSemaphoreId tBUF_Dac5BufAcc;

//ISP Output (PATH1)
uint8_t ubBUF_Sen1YuvBufTag[BUF_NUM_SEN_YUV];
uint32_t ulBUF_Sen1YuvBufAddr[BUF_NUM_SEN_YUV];	
uint8_t ubBUF_Sen1YuvBufIdx;
uint32_t ulBUF_Sen1YuvBufSz;
uint8_t ubBUF_Sen1YuvBufNum;

//ISP Output (PATH2)
uint8_t ubBUF_Sen2YuvBufTag[BUF_NUM_SEN_YUV];
uint32_t ulBUF_Sen2YuvBufAddr[BUF_NUM_SEN_YUV];	
uint8_t ubBUF_Sen2YuvBufIdx;
uint32_t ulBUF_Sen2YuvBufSz;
uint8_t ubBUF_Sen2YuvBufNum;

//ISP Output (PATH3)
uint8_t ubBUF_Sen3YuvBufTag[BUF_NUM_SEN_YUV];
uint32_t ulBUF_Sen3YuvBufAddr[BUF_NUM_SEN_YUV];	
uint8_t ubBUF_Sen3YuvBufIdx;
uint32_t ulBUF_Sen3YuvBufSz;
uint8_t ubBUF_Sen3YuvBufNum;

//BS0 Main
uint8_t ubBUF_VdoMainBs0BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoMainBs0BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoMainBs0BufIdx;
uint32_t ulBUF_VdoMainBs0BufSz;
uint8_t ubBUF_VdoMainBs0BufNum;

//BS1 Main
uint8_t ubBUF_VdoMainBs1BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoMainBs1BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoMainBs1BufIdx;
uint32_t ulBUF_VdoMainBs1BufSz;
uint8_t ubBUF_VdoMainBs1BufNum;

//BS2 Main
uint8_t ubBUF_VdoMainBs2BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoMainBs2BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoMainBs2BufIdx;
uint32_t ulBUF_VdoMainBs2BufSz;
uint8_t ubBUF_VdoMainBs2BufNum;

//BS3 Main
uint8_t ubBUF_VdoMainBs3BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoMainBs3BufAddr[BUF_NUM_VDO_BS3];	
uint8_t ubBUF_VdoMainBs3BufIdx;
uint32_t ulBUF_VdoMainBs3BufSz;
uint8_t ubBUF_VdoMainBs3BufNum;

//BS0 Aux
uint8_t ubBUF_VdoAuxBs0BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoAuxBs0BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoAuxBs0BufIdx;
uint32_t ulBUF_VdoAuxBs0BufSz;
uint8_t ubBUF_VdoAuxBs0BufNum;

//BS1 Aux
uint8_t ubBUF_VdoAuxBs1BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoAuxBs1BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoAuxBs1BufIdx;
uint32_t ulBUF_VdoAuxBs1BufSz;
uint8_t ubBUF_VdoAuxBs1BufNum;

//BS2 Aux
uint8_t ubBUF_VdoAuxBs2BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoAuxBs2BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoAuxBs2BufIdx;
uint32_t ulBUF_VdoAuxBs2BufSz;
uint8_t ubBUF_VdoAuxBs2BufNum;

//BS3 Aux
uint8_t ubBUF_VdoAuxBs3BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoAuxBs3BufAddr[BUF_NUM_VDO_BS3];
uint8_t ubBUF_VdoAuxBs3BufIdx;
uint32_t ulBUF_VdoAuxBs3BufSz;
uint8_t ubBUF_VdoAuxBs3BufNum;

//Index 0
//BS0 Sub
uint8_t ubBUF_VdoSubBs00BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoSubBs00BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoSubBs00BufIdx;
uint32_t ulBUF_VdoSubBs00BufSz;
uint8_t ubBUF_VdoSubBs00BufNum;

//BS1 Sub
uint8_t ubBUF_VdoSubBs10BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoSubBs10BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoSubBs10BufIdx;
uint32_t ulBUF_VdoSubBs10BufSz;
uint8_t ubBUF_VdoSubBs10BufNum;

//BS2 Sub
uint8_t ubBUF_VdoSubBs20BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoSubBs20BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoSubBs20BufIdx;
uint32_t ulBUF_VdoSubBs20BufSz;
uint8_t ubBUF_VdoSubBs20BufNum;

//BS3 Sub
uint8_t ubBUF_VdoSubBs30BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoSubBs30BufAddr[BUF_NUM_VDO_BS3];	
uint8_t ubBUF_VdoSubBs30BufIdx;
uint32_t ulBUF_VdoSubBs30BufSz;
uint8_t ubBUF_VdoSubBs30BufNum;

//Index 1
//BS0 Sub
uint8_t ubBUF_VdoSubBs01BufTag[BUF_NUM_VDO_BS0];
uint32_t ulBUF_VdoSubBs01BufAddr[BUF_NUM_VDO_BS0];	
uint8_t ubBUF_VdoSubBs01BufIdx;
uint32_t ulBUF_VdoSubBs01BufSz;
uint8_t ubBUF_VdoSubBs01BufNum;

//BS1 Sub
uint8_t ubBUF_VdoSubBs11BufTag[BUF_NUM_VDO_BS1];
uint32_t ulBUF_VdoSubBs11BufAddr[BUF_NUM_VDO_BS1];	
uint8_t ubBUF_VdoSubBs11BufIdx;
uint32_t ulBUF_VdoSubBs11BufSz;
uint8_t ubBUF_VdoSubBs11BufNum;

//BS2 Sub
uint8_t ubBUF_VdoSubBs21BufTag[BUF_NUM_VDO_BS2];
uint32_t ulBUF_VdoSubBs21BufAddr[BUF_NUM_VDO_BS2];	
uint8_t ubBUF_VdoSubBs21BufIdx;
uint32_t ulBUF_VdoSubBs21BufSz;
uint8_t ubBUF_VdoSubBs21BufNum;

//BS3 Sub
uint8_t ubBUF_VdoSubBs31BufTag[BUF_NUM_VDO_BS3];
uint32_t ulBUF_VdoSubBs31BufAddr[BUF_NUM_VDO_BS3];	
uint8_t ubBUF_VdoSubBs31BufIdx;
uint32_t ulBUF_VdoSubBs31BufSz;
uint8_t ubBUF_VdoSubBs31BufNum;

//ADC Buf Information
uint8_t ubBUF_AdcBufTag[BUF_NUM_ADC];
uint32_t ulBUF_AdcBufAddr[BUF_NUM_ADC];	
uint8_t ubBUF_AdcBufIdx;
uint32_t ulBUF_AdcBufSz;
uint8_t ubBUF_AdcBufNum;

//DAC Buf Information
//0
uint8_t ubBUF_Dac0BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac0BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac0BufIdx;
uint32_t ulBUF_Dac0BufSz;
uint8_t ubBUF_Dac0BufNum;

//1
uint8_t ubBUF_Dac1BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac1BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac1BufIdx;
uint32_t ulBUF_Dac1BufSz;
uint8_t ubBUF_Dac1BufNum;

//2
uint8_t ubBUF_Dac2BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac2BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac2BufIdx;
uint32_t ulBUF_Dac2BufSz;
uint8_t ubBUF_Dac2BufNum;

//3
uint8_t ubBUF_Dac3BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac3BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac3BufIdx;
uint32_t ulBUF_Dac3BufSz;
uint8_t ubBUF_Dac3BufNum;

//4
uint8_t ubBUF_Dac4BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac4BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac4BufIdx;
uint32_t ulBUF_Dac4BufSz;
uint8_t ubBUF_Dac4BufNum;

//5
uint8_t ubBUF_Dac5BufTag[BUF_NUM_DAC];
uint32_t ulBUF_Dac5BufAddr[BUF_NUM_DAC];	
uint8_t ubBUF_Dac5BufIdx;
uint32_t ulBUF_Dac5BufSz;
uint8_t ubBUF_Dac5BufNum;

//IP Buffer (Used for IP/Function Internal Only)
uint32_t ulBUF_ImgEncBufAddr[4];
uint32_t ulBUF_ImgDecBufAddr[4];
uint32_t ulBUF_AdoIpBufAddr;
uint32_t ulBUF_LcdIpBufAddr;
uint32_t ulBUF_BBIpBufAddr;
uint32_t ulBUF_ImgMergeIpBufAddr;
uint32_t ulBUF_SenIpBufAddr;
uint32_t ulBUF_UsbdIpBufAddr;
uint32_t ulBUF_UsbhIpBufAddr;
uint32_t ulBUF_3DNRIpBufAddr;
uint32_t ulBUF_MDw0IpBufAddr;
uint32_t ulBUF_MDw1IpBufAddr;
uint32_t ulBUF_MDw2IpBufAddr;
uint32_t ulBUF_IQbinBufAddr;

//JPG
uint32_t ulBUF_JpgBsBuffAddr[4];
uint32_t ulBUF_JpgRawBuffAddr;
uint32_t ulBUF_ResvYuvBuffAddr;

//FS
uint32_t ulBUF_FsBufAddr;
//REC
uint32_t ulBUF_RecBufAddr;

uint32_t ulBUF_FreeBufAddr;
uint32_t ulBUF_InitFreeBufAddr;

#define BUF_MAJORVER    0        //!< Major version = 0
#define BUF_MINORVER    2        //!< Minor version = 2
uint16_t uwBUF_GetVersion (void)
{
    return ((BUF_MAJORVER << 8) + BUF_MINORVER);
}

void BUF_Init(uint32_t ulBUF_StartAddress)
{
	ulBUF_FreeBufAddr = ulBUF_InitFreeBufAddr = ulBUF_StartAddress;
	osSemaphoreDef(tBUF_AdcBufAcc);
	tBUF_AdcBufAcc	= osSemaphoreCreate(osSemaphore(tBUF_AdcBufAcc), 1);
	osSemaphoreDef(tBUF_Dac0BufAcc);
	tBUF_Dac0BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac0BufAcc), 1);		
	osSemaphoreDef(tBUF_Dac1BufAcc);
	tBUF_Dac1BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac1BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac2BufAcc);
	tBUF_Dac2BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac2BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac3BufAcc);
	tBUF_Dac3BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac3BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac4BufAcc);
	tBUF_Dac4BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac4BufAcc), 1);	
	osSemaphoreDef(tBUF_Dac5BufAcc);
	tBUF_Dac5BufAcc	= osSemaphoreCreate(osSemaphore(tBUF_Dac5BufAcc), 1);
}

void BUF_ResetFreeAddr(void)
{
	ulBUF_FreeBufAddr = ulBUF_InitFreeBufAddr;
}

uint32_t ulBUF_GetFreeAddr(void)
{
	return ulBUF_FreeBufAddr;
}

void BUF_Reset(uint8_t ubBufMode)
{
	uint8_t i;
	
	if(ubBufMode == BUF_IMG_ENC)
	{		
	}
	else if(ubBufMode == BUF_IMG_DEC)
	{		
	}
	else if(ubBufMode == BUF_SEN_1_YUV)
	{
		for(i=0;i<ubBUF_Sen1YuvBufNum;i++)		
		{
			ubBUF_Sen1YuvBufTag[i] = BUF_FREE;						
		}
		ubBUF_Sen1YuvBufIdx = ubBUF_Sen1YuvBufNum-1;		
	}
	else if(ubBufMode == BUF_SEN_2_YUV)
	{
		for(i=0;i<ubBUF_Sen2YuvBufNum;i++)		
		{
			ubBUF_Sen2YuvBufTag[i] = BUF_FREE;						
		}
		ubBUF_Sen2YuvBufIdx = ubBUF_Sen2YuvBufNum-1;		
	}
	else if(ubBufMode == BUF_SEN_3_YUV)
	{
		for(i=0;i<ubBUF_Sen3YuvBufNum;i++)		
		{
			ubBUF_Sen3YuvBufTag[i] = BUF_FREE;						
		}
		ubBUF_Sen3YuvBufIdx = ubBUF_Sen3YuvBufNum-1;		
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS0)
	{
		for(i=0;i<ubBUF_VdoMainBs0BufNum;i++)
		{
			ubBUF_VdoMainBs0BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs0BufIdx 	= ubBUF_VdoMainBs0BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS1)
	{
		for(i=0;i<ubBUF_VdoMainBs1BufNum;i++)
		{
			ubBUF_VdoMainBs1BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs1BufIdx 	= ubBUF_VdoMainBs1BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS2)
	{
		for(i=0;i<ubBUF_VdoMainBs2BufNum;i++)
		{
			ubBUF_VdoMainBs2BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs2BufIdx 	= ubBUF_VdoMainBs2BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS3)
	{
		for(i=0;i<ubBUF_VdoMainBs3BufNum;i++)
		{
			ubBUF_VdoMainBs3BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoMainBs3BufIdx 	= ubBUF_VdoMainBs3BufNum-1;
	}
	
	else if(ubBufMode == BUF_VDO_AUX_BS0)
	{
		for(i=0;i<ubBUF_VdoAuxBs0BufNum;i++)
		{
			ubBUF_VdoAuxBs0BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs0BufIdx 	= ubBUF_VdoAuxBs0BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS1)
	{
		for(i=0;i<ubBUF_VdoAuxBs1BufNum;i++)
		{
			ubBUF_VdoAuxBs1BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs1BufIdx = ubBUF_VdoAuxBs1BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS2)
	{
		for(i=0;i<ubBUF_VdoAuxBs2BufNum;i++)
		{
			ubBUF_VdoAuxBs2BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs2BufIdx = ubBUF_VdoAuxBs2BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS3)
	{
		for(i=0;i<ubBUF_VdoAuxBs3BufNum;i++)
		{
			ubBUF_VdoAuxBs3BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoAuxBs3BufIdx = ubBUF_VdoAuxBs3BufNum-1;
	}
	
	else if(ubBufMode == BUF_VDO_SUB_BS00)
	{
		for(i=0;i<ubBUF_VdoSubBs00BufNum;i++)
		{
			ubBUF_VdoSubBs00BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs00BufIdx 	= ubBUF_VdoSubBs00BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS10)
	{
		for(i=0;i<ubBUF_VdoSubBs10BufNum;i++)
		{
			ubBUF_VdoSubBs10BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs10BufIdx 	= ubBUF_VdoSubBs10BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS20)
	{
		for(i=0;i<ubBUF_VdoSubBs20BufNum;i++)
		{
			ubBUF_VdoSubBs20BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs20BufIdx = ubBUF_VdoSubBs20BufNum-1;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS30)
	{
		for(i=0;i<ubBUF_VdoSubBs30BufNum;i++)
		{
			ubBUF_VdoSubBs30BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs30BufIdx = ubBUF_VdoSubBs30BufNum-1;
	}	
	else if(ubBufMode == BUF_VDO_SUB_BS01)
	{
		for(i=0;i<ubBUF_VdoSubBs01BufNum;i++)
		{
			ubBUF_VdoSubBs01BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs01BufIdx = ubBUF_VdoSubBs01BufNum-1;	
	}
	else if(ubBufMode == BUF_VDO_SUB_BS11)
	{
		for(i=0;i<ubBUF_VdoSubBs11BufNum;i++)
		{
			ubBUF_VdoSubBs11BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs11BufIdx = ubBUF_VdoSubBs11BufNum-1;	
	}
	else if(ubBufMode == BUF_VDO_SUB_BS21)
	{
		for(i=0;i<ubBUF_VdoSubBs21BufNum;i++)
		{
			ubBUF_VdoSubBs21BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs21BufIdx = ubBUF_VdoSubBs21BufNum-1;	
	}
	else if(ubBufMode == BUF_VDO_SUB_BS31)
	{
		for(i=0;i<ubBUF_VdoSubBs31BufNum;i++)
		{
			ubBUF_VdoSubBs31BufTag[i] = BUF_FREE;					
		}
		ubBUF_VdoSubBs31BufIdx = ubBUF_VdoSubBs31BufNum-1;
	}		
	else if(ubBufMode == BUF_ADO_ADC)
	{
		for(i=0;i<ubBUF_AdcBufNum;i++)
		{
			ubBUF_AdcBufTag[i] = BUF_FREE;			
		}
		ubBUF_AdcBufIdx = ubBUF_AdcBufNum-1;		
	}
	
	else if(ubBufMode == BUF_ADO_DAC0)
	{
		for(i=0;i<ubBUF_Dac0BufNum;i++)
		{
			ubBUF_Dac0BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac0BufIdx = ubBUF_Dac0BufNum-1;		
	}
	else if(ubBufMode == BUF_ADO_DAC1)
	{
		for(i=0;i<ubBUF_Dac1BufNum;i++)
		{
			ubBUF_Dac1BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac1BufIdx = ubBUF_Dac1BufNum-1;		
	}
	else if(ubBufMode == BUF_ADO_DAC2)
	{
		for(i=0;i<ubBUF_Dac2BufNum;i++)
		{
			ubBUF_Dac2BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac2BufIdx = ubBUF_Dac2BufNum-1;		
	}
	else if(ubBufMode == BUF_ADO_DAC3)
	{
		for(i=0;i<ubBUF_Dac3BufNum;i++)
		{
			ubBUF_Dac3BufTag[i]	= BUF_FREE;			
		}
		ubBUF_Dac3BufIdx = ubBUF_Dac3BufNum-1;		
	}
}

//ubIndex is useful for BUF_IMG_ENC/BUF_IMG_DEC Mode
//-----------------------------------------------------------------------
void BUF_BufInit(uint8_t ubBufMode,uint8_t ubBufNum,uint32_t ulUnitSz,uint8_t ubIndex)
{
	uint8_t i;

	printd(DBG_InfoLvl, "BufInit Mode=%d,Num=%d,Sz=%d,Index=%d,Adr=%d\n",ubBufMode,ubBufNum,ulUnitSz,ubIndex,ulBUF_FreeBufAddr); 

	//IP or Function Buffer
	if(ubBufMode == BUF_IMG_ENC)
	{
		ulBUF_ImgEncBufAddr[ubIndex] = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_IMG_DEC)
	{
		ulBUF_ImgDecBufAddr[ubIndex] = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_ADO_IP)
	{
		ulBUF_AdoIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_LCD_IP)
	{
		ulBUF_LcdIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_BB_IP)
	{
		ulBUF_BBIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_IMG_MERGE)
	{
		ulBUF_ImgMergeIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_SEN_IP)
	{
		ulBUF_SenIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_USBD_IP)
	{
		ulBUF_UsbdIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
    else if(ubBufMode == BUF_USBH_IP)
	{
		ulBUF_UsbhIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "USBH--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\r\n",ulBUF_FreeBufAddr); 
	}
    else if(ubBufMode == BUF_ISP_3DNR_IP)
	{
		ulBUF_3DNRIpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_ISP_MD_W0_IP)
	{
		ulBUF_MDw0IpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_ISP_MD_W1_IP)
	{
		ulBUF_MDw1IpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_ISP_MD_W2_IP)
	{
		ulBUF_MDw2IpBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	} 	
    else if(ubBufMode == BUF_IQ_BIN_FILE)
	{
		ulBUF_IQbinBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}    
	//Data Buffer
	else if(ubBufMode == BUF_SEN_1_YUV)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Sen1YuvBufTag[i]	= BUF_FREE;				
			ulBUF_Sen1YuvBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}			
		ubBUF_Sen1YuvBufIdx	= ubBufNum-1;		
		ulBUF_Sen1YuvBufSz	= ulUnitSz;
		ubBUF_Sen1YuvBufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_SEN_2_YUV)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Sen2YuvBufTag[i]	= BUF_FREE;				
			ulBUF_Sen2YuvBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}			
		ubBUF_Sen2YuvBufIdx 	= ubBufNum-1;		
		ulBUF_Sen2YuvBufSz	= ulUnitSz;
		ubBUF_Sen2YuvBufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_SEN_3_YUV)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Sen3YuvBufTag[i]	= BUF_FREE;				
			ulBUF_Sen3YuvBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}			
		ubBUF_Sen3YuvBufIdx = ubBufNum-1;		
		ulBUF_Sen3YuvBufSz  = ulUnitSz;
		ubBUF_Sen3YuvBufNum = ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS0)
	{
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs0BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs0BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs0BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs0BufSz 	= ulUnitSz;
		ubBUF_VdoMainBs0BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_MAIN_BS1)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs1BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs1BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs1BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs1BufSz	= ulUnitSz;
		ubBUF_VdoMainBs1BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS2)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs2BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs2BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs2BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs2BufSz	= ulUnitSz;
		ubBUF_VdoMainBs2BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_MAIN_BS3)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoMainBs3BufTag[i]	= BUF_FREE;			
			ulBUF_VdoMainBs3BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoMainBs3BufIdx 	= ubBufNum-1;
		ulBUF_VdoMainBs3BufSz	= ulUnitSz;
		ubBUF_VdoMainBs3BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS0)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs0BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs0BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs0BufIdx 	= ubBufNum-1;
		ulBUF_VdoAuxBs0BufSz	= ulUnitSz;
		ubBUF_VdoAuxBs0BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_AUX_BS1)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs1BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs1BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs1BufIdx 	= ubBufNum-1;
		ulBUF_VdoAuxBs1BufSz	= ulUnitSz;
		ubBUF_VdoAuxBs1BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS2)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs2BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs2BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs2BufIdx = ubBufNum-1;
		ulBUF_VdoAuxBs2BufSz  = ulUnitSz;
		ubBUF_VdoAuxBs2BufNum = ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_AUX_BS3)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoAuxBs3BufTag[i]	= BUF_FREE;			
			ulBUF_VdoAuxBs3BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoAuxBs3BufIdx = ubBufNum-1;
		ulBUF_VdoAuxBs3BufSz  = ulUnitSz;
		ubBUF_VdoAuxBs3BufNum = ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS00)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs00BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs00BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs00BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs00BufSz	= ulUnitSz;
		ubBUF_VdoSubBs00BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_SUB_BS10)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs10BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs10BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs10BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs10BufSz	= ulUnitSz;
		ubBUF_VdoSubBs10BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS20)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs20BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs20BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs20BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs20BufSz	= ulUnitSz;
		ubBUF_VdoSubBs20BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS30)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs30BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs30BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs30BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs30BufSz	= ulUnitSz;
		ubBUF_VdoSubBs30BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS01)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs01BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs01BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs01BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs01BufSz	= ulUnitSz;
		ubBUF_VdoSubBs01BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_VDO_SUB_BS11)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs11BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs11BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs11BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs11BufSz	= ulUnitSz;
		ubBUF_VdoSubBs11BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS21)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs21BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs21BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs21BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs21BufSz	= ulUnitSz;
		ubBUF_VdoSubBs21BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_VDO_SUB_BS31)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_VdoSubBs31BufTag[i]	= BUF_FREE;			
			ulBUF_VdoSubBs31BufAddr[i] 	= ulBUF_FreeBufAddr;	
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_VdoSubBs31BufIdx 	= ubBufNum-1;
		ulBUF_VdoSubBs31BufSz	= ulUnitSz;
		ubBUF_VdoSubBs31BufNum	= ubBufNum;
	}	
	else if(ubBufMode == BUF_ADO_ADC)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_AdcBufTag[i]	= BUF_FREE;			
			ulBUF_AdcBufAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr	= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 	= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_AdcBufIdx 	= ubBufNum-1;
		ulBUF_AdcBufSz		= ulUnitSz;
		ubBUF_AdcBufNum		= ubBufNum;
	}
	
	else if(ubBufMode == BUF_ADO_DAC0)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac0BufTag[i]		= BUF_FREE;			
			ulBUF_Dac0BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_Dac0BufIdx 	= ubBufNum-1;
		ulBUF_Dac0BufSz		= ulUnitSz;
		ubBUF_Dac0BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_ADO_DAC1)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac1BufTag[i]		= BUF_FREE;			
			ulBUF_Dac1BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_Dac1BufIdx 	= ubBufNum-1;
		ulBUF_Dac1BufSz		= ulUnitSz;
		ubBUF_Dac1BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_ADO_DAC2)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac2BufTag[i]		= BUF_FREE;			
			ulBUF_Dac2BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}		
		ubBUF_Dac2BufIdx 	= ubBufNum-1;
		ulBUF_Dac2BufSz		= ulUnitSz;
		ubBUF_Dac2BufNum	= ubBufNum;
	}
	else if(ubBufMode == BUF_ADO_DAC3)
	{		
		for(i=0;i<ubBufNum;i++)
		{
			ubBUF_Dac3BufTag[i]		= BUF_FREE;			
			ulBUF_Dac3BufAddr[i] 	= ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr		= ulBUF_FreeBufAddr+ulUnitSz;	
			ulBUF_FreeBufAddr 		= ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);			
		}
		ubBUF_Dac3BufIdx 	= ubBufNum-1;
		ulBUF_Dac3BufSz		= ulUnitSz;
		ubBUF_Dac3BufNum	= ubBufNum;
	}
    else if(ubBufMode == BUF_FS)
    {
		ulBUF_FsBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "FS--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
    else if(ubBufMode == BUF_REC)
    {
		ulBUF_RecBufAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
        printd(DBG_Debug3Lvl, "REC--->BUF_BufInit->ulBUF_FreeBufAddr:0x%X\n",ulBUF_FreeBufAddr); 
    }
	else if(ubBufMode == BUF_JPG_BS)
	{
		for(i = 0; i < ubBufNum; i++)
		{
			ulBUF_JpgBsBuffAddr[i] = ulBUF_FreeBufAddr;
			ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
			ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
		}
	}
	else if(ubBufMode == BUF_JPG_RAW)
	{
		ulBUF_JpgRawBuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}
	else if(ubBufMode == BUF_RESV_YUV)
	{
		ulBUF_ResvYuvBuffAddr = ulBUF_FreeBufAddr;
		ulBUF_FreeBufAddr = ulBUF_FreeBufAddr+ulUnitSz;
		ulBUF_FreeBufAddr = ulBUF_AlignAddrTo1K(ulBUF_FreeBufAddr);
	}

	if (ulBUF_FreeBufAddr >= DDR_BSZ_MAX) {
        printd(DBG_ErrorLvl, "Buffer full:%d\n",ulBUF_FreeBufAddr); 
    }
}

uint32_t ulBUF_GetSen1YuvFreeBuf(void)
{
	if(++ubBUF_Sen1YuvBufIdx >= BUF_NUM_SEN_YUV)
		ubBUF_Sen1YuvBufIdx = 0;
	if(ubBUF_Sen1YuvBufTag[ubBUF_Sen1YuvBufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get SEN YUV1 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_Sen1YuvBufTag[ubBUF_Sen1YuvBufIdx] = BUF_USED;
	return ulBUF_Sen1YuvBufAddr[ubBUF_Sen1YuvBufIdx];
}

uint32_t ulBUF_GetSen2YuvFreeBuf(void)
{
	if(++ubBUF_Sen2YuvBufIdx >= BUF_NUM_SEN_YUV)
		ubBUF_Sen2YuvBufIdx = 0;
	if(ubBUF_Sen2YuvBufTag[ubBUF_Sen2YuvBufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get SEN YUV2 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_Sen2YuvBufTag[ubBUF_Sen2YuvBufIdx] = BUF_USED;
	return ulBUF_Sen2YuvBufAddr[ubBUF_Sen2YuvBufIdx];
}

uint32_t ulBUF_GetSen3YuvFreeBuf(void)
{
	if(++ubBUF_Sen3YuvBufIdx >= BUF_NUM_SEN_YUV)
		ubBUF_Sen3YuvBufIdx = 0;
	if(ubBUF_Sen3YuvBufTag[ubBUF_Sen3YuvBufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get SEN YUV3 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_Sen3YuvBufTag[ubBUF_Sen3YuvBufIdx] = BUF_USED;
	return ulBUF_Sen3YuvBufAddr[ubBUF_Sen3YuvBufIdx];
}

uint32_t ulBUF_GetVdoMainBs0FreeBuf(void)
{
	if(++ubBUF_VdoMainBs0BufIdx >= BUF_NUM_VDO_BS0)
		ubBUF_VdoMainBs0BufIdx = 0;

	if(ubBUF_VdoMainBs0BufTag[ubBUF_VdoMainBs0BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Main BS0 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs0BufTag[ubBUF_VdoMainBs0BufIdx] = BUF_USED;
	return ulBUF_VdoMainBs0BufAddr[ubBUF_VdoMainBs0BufIdx];
}

uint32_t ulBUF_GetVdoMainBs1FreeBuf(void)
{
	if(++ubBUF_VdoMainBs1BufIdx >= BUF_NUM_VDO_BS1)
		ubBUF_VdoMainBs1BufIdx = 0;

	if(ubBUF_VdoMainBs1BufTag[ubBUF_VdoMainBs1BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Main BS1 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs1BufTag[ubBUF_VdoMainBs1BufIdx] = BUF_USED;
	return ulBUF_VdoMainBs1BufAddr[ubBUF_VdoMainBs1BufIdx];
}

uint32_t ulBUF_GetVdoMainBs2FreeBuf(void)
{
	if(++ubBUF_VdoMainBs2BufIdx >= BUF_NUM_VDO_BS2)
		ubBUF_VdoMainBs2BufIdx = 0;

	if(ubBUF_VdoMainBs2BufTag[ubBUF_VdoMainBs2BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Main BS2 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs2BufTag[ubBUF_VdoMainBs2BufIdx] = BUF_USED;
	return ulBUF_VdoMainBs2BufAddr[ubBUF_VdoMainBs2BufIdx];	
}

uint32_t ulBUF_GetVdoMainBs3FreeBuf(void)
{
	if(++ubBUF_VdoMainBs3BufIdx >= BUF_NUM_VDO_BS3)
		ubBUF_VdoMainBs3BufIdx = 0;

	if(ubBUF_VdoMainBs3BufTag[ubBUF_VdoMainBs3BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Main BS3 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoMainBs3BufTag[ubBUF_VdoMainBs3BufIdx] = BUF_USED;
	return ulBUF_VdoMainBs3BufAddr[ubBUF_VdoMainBs3BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs0FreeBuf(void)
{
	if(++ubBUF_VdoAuxBs0BufIdx >= BUF_NUM_VDO_BS0)
		ubBUF_VdoAuxBs0BufIdx = 0;

	if(ubBUF_VdoAuxBs0BufTag[ubBUF_VdoAuxBs0BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Aux BS0 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs0BufTag[ubBUF_VdoAuxBs0BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs0BufAddr[ubBUF_VdoAuxBs0BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs1FreeBuf(void)
{	
	if(++ubBUF_VdoAuxBs1BufIdx >= BUF_NUM_VDO_BS1)
		ubBUF_VdoAuxBs1BufIdx = 0;

	if(ubBUF_VdoAuxBs1BufTag[ubBUF_VdoAuxBs1BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Aux BS1 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs1BufTag[ubBUF_VdoAuxBs1BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs1BufAddr[ubBUF_VdoAuxBs1BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs2FreeBuf(void)
{
	if(++ubBUF_VdoAuxBs2BufIdx >= BUF_NUM_VDO_BS2)
		ubBUF_VdoAuxBs2BufIdx = 0;

	if(ubBUF_VdoAuxBs2BufTag[ubBUF_VdoAuxBs2BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Aux BS2 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs2BufTag[ubBUF_VdoAuxBs2BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs2BufAddr[ubBUF_VdoAuxBs2BufIdx];
}

uint32_t ulBUF_GetVdoAuxBs3FreeBuf(void)
{
	if(++ubBUF_VdoAuxBs3BufIdx >= BUF_NUM_VDO_BS3)
		ubBUF_VdoAuxBs3BufIdx = 0;

	if(ubBUF_VdoAuxBs3BufTag[ubBUF_VdoAuxBs3BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Aux BS3 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoAuxBs3BufTag[ubBUF_VdoAuxBs3BufIdx] = BUF_USED;
	return ulBUF_VdoAuxBs3BufAddr[ubBUF_VdoAuxBs3BufIdx];
}

uint32_t ulBUF_GetVdoSubBs00FreeBuf(void)
{
	if(++ubBUF_VdoSubBs00BufIdx >= BUF_NUM_VDO_BS0)
		ubBUF_VdoSubBs00BufIdx = 0;

	if(ubBUF_VdoSubBs00BufTag[ubBUF_VdoSubBs00BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS00 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs00BufTag[ubBUF_VdoSubBs00BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs00BufAddr[ubBUF_VdoSubBs00BufIdx];
}

uint32_t ulBUF_GetVdoSubBs10FreeBuf(void)
{
	if(++ubBUF_VdoSubBs10BufIdx >= BUF_NUM_VDO_BS1)
		ubBUF_VdoSubBs10BufIdx = 0;

	if(ubBUF_VdoSubBs10BufTag[ubBUF_VdoSubBs10BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS10 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs10BufTag[ubBUF_VdoSubBs10BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs10BufAddr[ubBUF_VdoSubBs10BufIdx];	
}

uint32_t ulBUF_GetVdoSubBs20FreeBuf(void)
{
	if(++ubBUF_VdoSubBs20BufIdx >= BUF_NUM_VDO_BS2)
		ubBUF_VdoSubBs20BufIdx = 0;

	if(ubBUF_VdoSubBs20BufTag[ubBUF_VdoSubBs20BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS20 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs20BufTag[ubBUF_VdoSubBs20BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs20BufAddr[ubBUF_VdoSubBs20BufIdx];
}

uint32_t ulBUF_GetVdoSubBs30FreeBuf(void)
{
	if(++ubBUF_VdoSubBs30BufIdx >= BUF_NUM_VDO_BS3)
		ubBUF_VdoSubBs30BufIdx = 0;

	if(ubBUF_VdoSubBs30BufTag[ubBUF_VdoSubBs30BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS30 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs30BufTag[ubBUF_VdoSubBs30BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs30BufAddr[ubBUF_VdoSubBs30BufIdx];
}

//-----------------------------------------------------------------------------------
uint32_t ulBUF_GetVdoSubBs01FreeBuf(void)
{
	if(++ubBUF_VdoSubBs01BufIdx >= BUF_NUM_VDO_BS0)
		ubBUF_VdoSubBs01BufIdx = 0;

	if(ubBUF_VdoSubBs01BufTag[ubBUF_VdoSubBs01BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS01 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs01BufTag[ubBUF_VdoSubBs01BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs01BufAddr[ubBUF_VdoSubBs01BufIdx];	
}

uint32_t ulBUF_GetVdoSubBs11FreeBuf(void)
{
	if(++ubBUF_VdoSubBs11BufIdx >= BUF_NUM_VDO_BS1)
		ubBUF_VdoSubBs11BufIdx = 0;

	if(ubBUF_VdoSubBs11BufTag[ubBUF_VdoSubBs11BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS11 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs11BufTag[ubBUF_VdoSubBs11BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs11BufAddr[ubBUF_VdoSubBs11BufIdx];	
}

uint32_t ulBUF_GetVdoSubBs21FreeBuf(void)
{
	if(++ubBUF_VdoSubBs21BufIdx >= BUF_NUM_VDO_BS2)
		ubBUF_VdoSubBs21BufIdx = 0;

	if(ubBUF_VdoSubBs21BufTag[ubBUF_VdoSubBs21BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS21 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs21BufTag[ubBUF_VdoSubBs21BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs21BufAddr[ubBUF_VdoSubBs21BufIdx];
}

uint32_t ulBUF_GetVdoSubBs31FreeBuf(void)
{
	if(++ubBUF_VdoSubBs31BufIdx >= BUF_NUM_VDO_BS3)
		ubBUF_VdoSubBs31BufIdx = 0;

	if(ubBUF_VdoSubBs31BufTag[ubBUF_VdoSubBs31BufIdx] != BUF_FREE)
	{
		printd(DBG_ErrorLvl, "Get Sub BS31 Err !\n");
		return BUF_FAIL;
	}
	ubBUF_VdoSubBs31BufTag[ubBUF_VdoSubBs31BufIdx] = BUF_USED;
	return ulBUF_VdoSubBs31BufAddr[ubBUF_VdoSubBs31BufIdx];
}

uint32_t ulBUF_GetAdcFreeBuf(void)
{
#if RTOS	
	osSemaphoreWait(tBUF_AdcBufAcc, osWaitForever);
#endif
	if(++ubBUF_AdcBufIdx >= BUF_NUM_ADC)
		ubBUF_AdcBufIdx = 0;

	if(ubBUF_AdcBufTag[ubBUF_AdcBufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get ADC Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_AdcBufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_AdcBufTag[ubBUF_AdcBufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_AdcBufAcc);
#endif
	return ulBUF_AdcBufAddr[ubBUF_AdcBufIdx];
}

uint32_t ulBUF_GetDac0FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac0BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac0BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac0BufIdx = 0;

	if(ubBUF_Dac0BufTag[ubBUF_Dac0BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get DAC0 Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac0BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac0BufTag[ubBUF_Dac0BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac0BufAcc);
#endif
	return ulBUF_Dac0BufAddr[ubBUF_Dac0BufIdx];
}

uint32_t ulBUF_GetDac1FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac1BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac1BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac1BufIdx = 0;

	if(ubBUF_Dac1BufTag[ubBUF_Dac1BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get DAC1 Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac1BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac1BufTag[ubBUF_Dac1BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac1BufAcc);
#endif
	return ulBUF_Dac1BufAddr[ubBUF_Dac1BufIdx];
}

uint32_t ulBUF_GetDac2FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac2BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac2BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac2BufIdx = 0;

	if(ubBUF_Dac2BufTag[ubBUF_Dac2BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get DAC2 Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac2BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac2BufTag[ubBUF_Dac2BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac2BufAcc);
#endif
	return ulBUF_Dac2BufAddr[ubBUF_Dac2BufIdx];
}

uint32_t ulBUF_GetDac3FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac3BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac3BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac3BufIdx = 0;

	if(ubBUF_Dac3BufTag[ubBUF_Dac3BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get DAC3 Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac3BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac3BufTag[ubBUF_Dac3BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac3BufAcc);
#endif
	return ulBUF_Dac3BufAddr[ubBUF_Dac3BufIdx];
}

uint32_t ulBUF_GetDac4FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac4BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac4BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac4BufIdx = 0;

	if(ubBUF_Dac4BufTag[ubBUF_Dac4BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get DAC4 Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac4BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac4BufTag[ubBUF_Dac4BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac4BufAcc);
#endif
	return ulBUF_Dac4BufAddr[ubBUF_Dac4BufIdx];
}

uint32_t ulBUF_GetDac5FreeBuf(void)
{
#if RTOS
	osSemaphoreWait(tBUF_Dac5BufAcc, osWaitForever);
#endif
	if(++ubBUF_Dac5BufIdx >= BUF_NUM_DAC)
		ubBUF_Dac5BufIdx = 0;

	if(ubBUF_Dac4BufTag[ubBUF_Dac5BufIdx] == BUF_USED)
	{
		printd(DBG_ErrorLvl, "Get DAC5 Buf Err !\n");
	#if RTOS
		osSemaphoreRelease(tBUF_Dac5BufAcc);
	#endif
		return BUF_FAIL;
	}
	ubBUF_Dac5BufTag[ubBUF_Dac5BufIdx] = BUF_USED;
#if RTOS
	osSemaphoreRelease(tBUF_Dac5BufAcc);
#endif
	return ulBUF_Dac5BufAddr[ubBUF_Dac5BufIdx];
}

uint8_t ubBUF_ReleaseSenYuvBuf(uint32_t ulBufAddr)
{
	uint8_t ubPath1Flg = 0;
	uint8_t ubPath2Flg = 0;
	uint8_t ubPath3Flg = 0;
	uint8_t ubBufIdx   = 0;

	//Path1
	for(ubBufIdx=0;ubBufIdx<ubBUF_Sen1YuvBufNum;ubBufIdx++)
	{
		if(ulBUF_Sen1YuvBufAddr[ubBufIdx] == ulBufAddr)
		{
			ubPath1Flg = 1;
			break;
		}
	}
	if(!ubPath1Flg)
	{
		//Path2
		for(ubBufIdx=0;ubBufIdx<ubBUF_Sen2YuvBufNum;ubBufIdx++)
		{
			if(ulBUF_Sen2YuvBufAddr[ubBufIdx] == ulBufAddr)
			{
				ubPath2Flg = 1;
				break;
			}
		}
		//Path3
		for(ubBufIdx=0;ubBufIdx<ubBUF_Sen3YuvBufNum;ubBufIdx++)
		{
			if(ulBUF_Sen3YuvBufAddr[ubBufIdx] == ulBufAddr)
			{
				ubPath3Flg = 1;
				break;
			}
		}
	}

	if(ubPath1Flg)
	{
		return ubBUF_ReleaseSen1YuvBuf(ubBufIdx);
	}
	else if(ubPath2Flg)
	{
		return ubBUF_ReleaseSen2YuvBuf(ubBufIdx);
	}
	else if(ubPath3Flg)
	{
		return ubBUF_ReleaseSen3YuvBuf(ubBufIdx);
	}
	else
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_SenReleaseBuf !!!!\r\n");
		return BUF_FAIL;
	}
}

uint8_t ubBUF_ReleaseSen1YuvBuf(uint8_t ubBufIdx)
{
	ubBUF_Sen1YuvBufTag[ubBufIdx] = BUF_FREE;		
	return BUF_OK;	
}

uint8_t ubBUF_ReleaseSen2YuvBuf(uint8_t ubBufIdx)
{
	ubBUF_Sen2YuvBufTag[ubBufIdx] = BUF_FREE;		
	return BUF_OK;
}

uint8_t ubBUF_ReleaseSen3YuvBuf(uint8_t ubBufIdx)
{
	ubBUF_Sen3YuvBufTag[ubBufIdx] = BUF_FREE;		
	return BUF_OK;	
}

uint8_t ubBUF_ReleaseVdoMainBs0Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS0;

	for(i=0;i<BUF_NUM_VDO_BS0;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_VDO_BS0)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs0ReleaseBuf !!!!\r\n");
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs0BufTag[ubBufIdx] = BUF_FREE;
		return BUF_OK;
	}
}

uint8_t ubBUF_ReleaseVdoMainBs1Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS1;
	
	for(i=0;i<BUF_NUM_VDO_BS1;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS1)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs1ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs1BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoMainBs2Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS2;

	for(i=0;i<BUF_NUM_VDO_BS2;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS2)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs2ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs2BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoMainBs3Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS3;
	
	for(i=0;i<BUF_NUM_VDO_BS3;i++)
	{
		if(ulBufAddr == ulBUF_VdoMainBs3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS3)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoMainBs3ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoMainBs3BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs0Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS0;

	for(i=0;i<BUF_NUM_VDO_BS0;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS0)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs0ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs0BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs1Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS1;

	for(i=0;i<BUF_NUM_VDO_BS1;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS1)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs1ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs1BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs2Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS2;

	for(i=0;i<BUF_NUM_VDO_BS2;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS2)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs2ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs2BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoAuxBs3Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS3;

	for(i=0;i<BUF_NUM_VDO_BS3;i++)
	{
		if(ulBufAddr == ulBUF_VdoAuxBs3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS3)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoAuxBs3ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoAuxBs3BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs00Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS0;

	for(i=0;i<BUF_NUM_VDO_BS0;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs00BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS0)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs00ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs00BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs10Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS1;

	for(i=0;i<BUF_NUM_VDO_BS1;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs10BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS1)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs10ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs10BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs20Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS2;

	for(i=0;i<BUF_NUM_VDO_BS2;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs20BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS2)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs20ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs20BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs30Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS3;

	for(i=0;i<BUF_NUM_VDO_BS3;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs30BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS3)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs30ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs30BufTag[ubBufIdx] = BUF_FREE;
		return BUF_OK;
	}	
}


//-------------------------------------------------------------------------------
uint8_t ubBUF_ReleaseVdoSubBs01Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS0;

	for(i=0;i<BUF_NUM_VDO_BS0;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs01BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS0)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs01ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs01BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs11Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS1;

	for(i=0;i<BUF_NUM_VDO_BS1;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs11BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS1)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs11ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs11BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs21Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS2;

	for(i=0;i<BUF_NUM_VDO_BS2;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs21BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS2)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs21ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs21BufTag[ubBufIdx] = BUF_FREE;		
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseVdoSubBs31Buf(uint32_t ulBufAddr)
{	
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_VDO_BS3;
	
	for(i=0;i<BUF_NUM_VDO_BS3;i++)
	{
		if(ulBufAddr == ulBUF_VdoSubBs31BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_VDO_BS3)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_VdoSubBs31ReleaseBuf !!!!\r\n");		
		return BUF_FAIL;
	}
	else
	{
		ubBUF_VdoSubBs31BufTag[ubBufIdx] = BUF_FREE;
		return BUF_OK;
	}	
}

uint8_t ubBUF_ReleaseAdcBuf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_ADC, ubAdcResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_AdcBufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_ADC;i++)
	{
		if(ulBufAddr == ulBUF_AdcBufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_ADC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_AdcReleaseBuf !!!!\r\n");
		ubAdcResult = BUF_FAIL;
	}
	else
	{
		ubBUF_AdcBufTag[ubBufIdx] = BUF_FREE;
	}	
#if RTOS
	osSemaphoreRelease(tBUF_AdcBufAcc);
#endif
	return ubAdcResult;
}

uint8_t ubBUF_ReleaseDac0Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac0BufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac0BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac0ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac0BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac0BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac1Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac1BufAcc, osWaitForever);
#endif
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac1BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac1ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac1BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac1BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac2Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;
	
#if RTOS
	osSemaphoreWait(tBUF_Dac2BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac2BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac2ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac2BufTag[ubBufIdx] = BUF_FREE;
	}	
#if RTOS
	osSemaphoreRelease(tBUF_Dac2BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac3Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac3BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac3BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac3ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac3BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac3BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac4Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS	
	osSemaphoreWait(tBUF_Dac4BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac4BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac4ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac4BufTag[ubBufIdx] = BUF_FREE;
	}	
#if RTOS
	osSemaphoreRelease(tBUF_Dac4BufAcc);
#endif
	return ubDacResult;
}

uint8_t ubBUF_ReleaseDac5Buf(uint32_t ulBufAddr)
{
	uint8_t i;
	uint8_t ubBufIdx = BUF_NUM_DAC, ubDacResult = BUF_OK;

#if RTOS
	osSemaphoreWait(tBUF_Dac5BufAcc, osWaitForever);
#endif	
	for(i=0;i<BUF_NUM_DAC;i++)
	{
		if(ulBufAddr == ulBUF_Dac5BufAddr[i])
		{
			ubBufIdx = i;
			break;
		}
	}	
	if(ubBufIdx == BUF_NUM_DAC)
	{
		printd(DBG_ErrorLvl, "Fail @ubBUF_Dac5ReleaseBuf !!!!\r\n");
		ubDacResult = BUF_FAIL;
	}
	else
	{
		ubBUF_Dac5BufTag[ubBufIdx] = BUF_FREE;
	}
#if RTOS
	osSemaphoreRelease(tBUF_Dac5BufAcc);
#endif
	return ubDacResult;
}

//Get IP or Function Buffer Address
uint32_t ulBUF_GetBlkBufAddr(uint8_t ubIndex,uint8_t ubBufMode)
{
	if(ubBufMode == BUF_IMG_ENC)
	{
		return ulBUF_ImgEncBufAddr[ubIndex];
	}
	else if(ubBufMode == BUF_IMG_DEC)
	{
		return ulBUF_ImgDecBufAddr[ubIndex];
	}
	else if(ubBufMode == BUF_ADO_IP)
	{
		return ulBUF_AdoIpBufAddr;
	}
	else if(ubBufMode == BUF_LCD_IP)
	{
		return ulBUF_LcdIpBufAddr;
	}
	else if(ubBufMode == BUF_BB_IP)
	{
		return ulBUF_BBIpBufAddr;
	}
	else if(ubBufMode == BUF_IMG_MERGE)
	{
		return ulBUF_ImgMergeIpBufAddr;
	}
	else if(ubBufMode == BUF_SEN_IP)
	{
		return ulBUF_SenIpBufAddr;
	}
    else if(ubBufMode == BUF_USBD_IP)
	{
		return ulBUF_UsbdIpBufAddr;
	}
    else if(ubBufMode == BUF_USBH_IP)
	{
		return ulBUF_UsbhIpBufAddr;
	}
    else if(ubBufMode == BUF_ISP_3DNR_IP)
	{
		return ulBUF_3DNRIpBufAddr;
	}	
    else if(ubBufMode == BUF_ISP_MD_W0_IP)
	{
		return ulBUF_MDw0IpBufAddr;
	}
    else if(ubBufMode == BUF_ISP_MD_W1_IP)
	{
		return ulBUF_MDw1IpBufAddr;
	}
    else if(ubBufMode == BUF_ISP_MD_W2_IP)
	{
		return ulBUF_MDw2IpBufAddr;
	}
    else if(ubBufMode == BUF_IQ_BIN_FILE)
	{
		return ulBUF_IQbinBufAddr;
	}    
    else if(ubBufMode == BUF_FS)
	{
		return ulBUF_FsBufAddr;
	}  
    else if(ubBufMode == BUF_REC)
	{
		return ulBUF_RecBufAddr;
	}
	else if(ubBufMode == BUF_JPG_BS)
	{
		return ulBUF_JpgBsBuffAddr[ubIndex];
	}
	else if(ubBufMode == BUF_JPG_RAW)
	{
		return ulBUF_JpgRawBuffAddr;
	}
	else if(ubBufMode == BUF_RESV_YUV)
	{
		return ulBUF_ResvYuvBuffAddr;
	}
	printd(DBG_ErrorLvl, "Err @ulBUF_GetBufAddr\r\n");
	return 0;	//Error Case
}

uint32_t ulBUF_AlignAddrTo1K(uint32_t ulAddr)
{
	if((ulAddr%1024) == 0)
	{
		return ulAddr;
	}
	else
	{
		return ((ulAddr/1024)*1024)+1024;
	}
}
