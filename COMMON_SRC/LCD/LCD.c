/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD.c
	\brief		LCD Funcation
	\author		Pierce
	\version	1.2
	\date		2017/11/22
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "LCD.h"
#include "OSD.h"
#include "TIMER.h"
//------------------------------------------------------------------------------
#define LCD_MAJORVER    1        //!< Major version
#define LCD_MINORVER    2        //!< Minor version
//------------------------------------------------------------------------------
static osMessageQId 	 	LCD_SwitchModeQueue;
static osMessageQId 	 	LCD_GpioQueue;
static LCD_DYN_SETTING_TYP tLCD_DynSetting;
static LCD_TOL_BUF_TYP	   tLCD_BufInfor;
static LCD_MODE_TYP  	   tLCD_Mode;
//static LCD_ROTATE_TYP	   tLcdRotate;
static uint32_t 		  ulLCD_ChBufSize[LCD_MAX_CH];
static uint32_t 	  	  ulLCD_TotalChBufSize;
static uint8_t 			  ubLCD_ChNoImgCount[LCD_MAX_CH];
static uint8_t			  ubLCD_ChNoImgBoundary;
static uint8_t			  ubLCD_MaxChNow;
static LCD_JPEG_STATUS	  tLCD_JpegEn;
static LCD_CHANNEL_EN_t   tLCD_ChEnable;
static bool               bLCD_IsrEnable;
static uint8_t			  ubLCD_StartFlag;
static LCD_OUTPUT_TYP tOutputNow = LCD_UNINIT;
osMutexId osLCD_Mutex;
osMutexDef(osLCD_Mutex);
//------------------------------------------------------------------------------
static void LCD_FrameUpdate (void);
static void LCD_SetLcdChBufAddr (LCD_CH_TYP tCh, uint32_t ulAddr);
//------------------------------------------------------------------------------
const uint8_t ubLCD_GammaCurve[13][17] = 
{
           {128,147,159,168,183,193,202,210,222,232,241,245,247,249,251,253,255},    // 0.2
           { 90,111,125,137,155,168,180,190,207 ,222,234,240,245,248,250,253,255},     // 0.3
           { 64, 84, 99,111,131,147,160,173,194 ,212,228,235,242,246,249,252,254},       // 0.4
           { 45, 64, 78, 90,111,128,143,156,181 ,202,221,230,239,243,247,251,254},         // 0.5
           { 32, 48, 62, 73, 94,111,127,142,169 ,193,215,226,236,241,246,251,254},          // 0.6
           { 23, 37, 49, 60, 79, 97,113,129,157 ,184,209,221,233,239,244,250,254}, // 0.7
           { 16, 28, 39, 48, 67, 84,101,117,147 ,176,203,217,230,236,243,249,253}, // 0.8
           { 11, 21, 30, 39, 57, 73, 90,106,137,168,198,212,227,234,241,249,253},   // 0.9
           {  8, 16, 24, 32, 48, 64, 80, 96,128,160,192,208,224,232,240,248,252},   // 1.0
           {  6, 12, 19, 26, 41, 56, 71, 87,119,153,187,204,221,230,239,247,251},   // 1.1
           {  4,  9, 15, 21, 34, 49, 63, 79,112,146,181,200,218,228,237,247,251},  // 1.2
           {  3,  7, 12, 17, 29, 42, 57, 72,104,139,176,196,215,226,236,246,250},  // 1.3
           {  2,  5,  9, 14, 25, 37, 50, 65, 97,133,171,192,213,223,234,245,249}   // 1.4
};

//------------------------------------------------------------------------------
void LCD_SetGammaBritCtst(LCD_GAMMA_SET_TYP *pParam)
{
	uint8_t 	ubIdx;
	uint8_t 	ubMod;
	uint8_t 	ubLoop;
	uint8_t 	swGammaTmp;
	uint8_t 	ubCntrsCtr;
	uint32_t 	ulGammaTemp[5];
	uint8_t 	*ptr = (uint8_t*)ulGammaTemp;
	
	ubCntrsCtr 			   = 1;
	LCD->LCD_Y_GAMMA_EN = 1;
	//fix point gamma x 100 = (100.0f/uwGamma)x100 = 10000/uwGamma
	pParam->uwGamma = 10000 / pParam->uwGamma;

	//==========================================================
	// output = center point + contrast*(input - center point)
	// zero point offset = (1 - contrast)*center point
	// = (64 - (CONTRAST_VALUE + 64))*center point / 64
	// set brightness = original bright +  zero point offset
	//==========================================================
	pParam->swBrit = pParam->swBrit - (uint16_t) (((((int32_t) (pParam->uwCntrs)) - 32L) * ((int32_t) ubCntrsCtr)) >> 6);
	pParam->uwGain = ((pParam->uwCntrs) + 32) * ((pParam->uwGain) + 64);

	*ptr++  = (pParam->swBrit< 0) ? 0:pParam->swBrit;
	ubIdx = pParam->uwGamma / 10;
	ubMod = pParam->uwGamma % 10;	
	for(ubLoop = 1; ubLoop < 18; ubLoop++)	{
		swGammaTmp = ((((int16_t) ubLCD_GammaCurve[ubIdx - 2][ubLoop - 1]) * (10 - ubMod)) +  (((int16_t) ubLCD_GammaCurve[ubIdx - 1][ubLoop - 1]) * ubMod)) / 10;
		swGammaTmp = (((uint32_t) swGammaTmp) * pParam->uwGain) >> 12;
		swGammaTmp = ((swGammaTmp + pParam->swBrit) < 0) ? 0:(swGammaTmp + pParam->swBrit);
		*ptr++  = (swGammaTmp > 255) ? 255:swGammaTmp;
	}		
	swGammaTmp = pParam->uwGain >> 4;
	*ptr++  = ((swGammaTmp + pParam->swBrit) > 255)? 255:(swGammaTmp + pParam->swBrit);

	// copy from register shadow buffer to register, cause register access unit is 32bit
	LCD->REG_0x9800_0138 = ulGammaTemp[0];									//LCD_Y_GAMMA0~3
	LCD->REG_0x9800_013C = ulGammaTemp[1];									//LCD_Y_GAMMA4~7
	LCD->REG_0x9800_0140 = ulGammaTemp[2];                  //LCD_Y_GAMMA8~11
	LCD->REG_0x9800_0144 = ulGammaTemp[3];                  //LCD_Y_GAMMA12~15
	LCD->REG_0x9800_0148 = ulGammaTemp[4];                  //LCD_Y_GAMMA16~17
}
//------------------------------------------------------------------------------
void LCD_SetGammaLevel(uint8_t ubGammaLevel) 
{
	LCD_GAMMA_SET_TYP tParam;
	tParam.uwGamma 	= 100;
	tParam.swBrit	= 0;
	tParam.uwCntrs	= 32;
	tParam.uwGain	= 0;
	
	switch(ubGammaLevel) {
		case 0:	
			tParam.uwGamma  = 72;
		   	break;
		case 1:
			tParam.uwGamma  = 85;
			break;
		case 2:	
			tParam.uwGamma  = 100;
			break;
		case 3:	
			tParam.uwGamma  = 125;
			break;
		case 4:	
			tParam.uwGamma  = 150;
			break;
		case 5:	
			tParam.uwGamma  = 175;
			break;
		case 6:	
		default:
			tParam.uwGamma  = 200;
			break;
	}
	LCD_SetGammaBritCtst(&tParam);
}
//------------------------------------------------------------------------------
uint16_t uwLCD_GetVersion (void)
{
    return ((LCD_MAJORVER << 8) + LCD_MINORVER);
}
//------------------------------------------------------------------------------
LCD_BUF_TYP *pLCD_GetLcdChBufInfor (LCD_CH_TYP tCh)
{
	return (LCD_BUF_CUR == tLCD_BufInfor.tChBuf[tCh].tBuf[0].tBufState)?
				&tLCD_BufInfor.tChBuf[tCh].tBuf[1]:&tLCD_BufInfor.tChBuf[tCh].tBuf[0];
}
//------------------------------------------------------------------------------
uint16_t uwLCD_GetLcdHoSize (void)
{
	return LCD->LCD_HO_SIZE;
}
//------------------------------------------------------------------------------
uint16_t uwLCD_GetLcdVoSize (void)
{
	return LCD->LCD_VO_SIZE;
}
//------------------------------------------------------------------------------
uint32_t ulLCD_CalLcdBufSize (LCD_CALBUF_TYP *pInfor)
{
	uint32_t ulOsdMenuBufSize;
	uint8_t  ubi;
	
	for (ubi=0; ubi<pInfor->ubChMax; ++ubi)
	{
		ulLCD_ChBufSize[ubi] = pInfor->tInput[ubi].uwHSize * (pInfor->tInput[ubi].uwVSize + 
		(pInfor->tInput[ubi].uwVSize >> 1) + ((pInfor->tInput[ubi].uwVSize & 1)?1:0));
		if (true == pInfor->tInput[ubi].bJpegEn)
			ulLCD_ChBufSize[ubi] >>= 1;
		//if (LCD_BUF_4BYTES_ALIG < pInfor->tAlign)
		if ((LCD_BUF_4BYTES_ALIG < pInfor->tAlign) && (pInfor->tAlign <= LCD_BUF_1024BYTES_ALIG))
		{
			if (((1 << pInfor->tAlign) - 1) & ulLCD_ChBufSize[ubi])
				ulLCD_ChBufSize[ubi] = ((ulLCD_ChBufSize[ubi] >> pInfor->tAlign) + 1) << pInfor->tAlign;
		}
		else if (3 & ulLCD_ChBufSize[ubi])
			ulLCD_ChBufSize[ubi] = ((ulLCD_ChBufSize[ubi] >> 2) + 1) << 2;
	}
	for (;ubi<LCD_MAX_CH; ++ubi)
		ulLCD_ChBufSize[ubi] = 0;
	ulOsdMenuBufSize = (pInfor->uwLcdHSize * pInfor->uwLcdVSize * 3) >> 2;
	if (3 & ulOsdMenuBufSize)
		ulOsdMenuBufSize = ((ulOsdMenuBufSize >> 2) + 1) << 2;
	ulOsdMenuBufSize <<= 1;
	for (ulLCD_TotalChBufSize=ubi=0; ubi<LCD_MAX_CH; ++ubi)	
		ulLCD_TotalChBufSize += ulLCD_ChBufSize[ubi];	
	ulLCD_TotalChBufSize <<= 1;
	return (ulLCD_TotalChBufSize = LCDMAX(ulLCD_TotalChBufSize, ulOsdMenuBufSize));
}
//------------------------------------------------------------------------------
void LCD_SetLcdBufAddr (uint32_t ulAddr)
{
	uint8_t ubi;
	
	for (ubi=0; ubi<LCD_MAX_CH; ++ubi)
	{	
		tLCD_BufInfor.tChBuf[ubi].tBuf[0].ulBufAddr = (ubi)?tLCD_BufInfor.tChBuf[ubi - 1].tBuf[1].ulBufAddr + ulLCD_ChBufSize[ubi - 1]:ulAddr;
		tLCD_BufInfor.tChBuf[ubi].tBuf[1].ulBufAddr = tLCD_BufInfor.tChBuf[ubi].tBuf[0].ulBufAddr + ulLCD_ChBufSize[ubi];
		tLCD_BufInfor.tChBuf[ubi].tBuf[0].tBufState = LCD_BUF_EMPTY;
		tLCD_BufInfor.tChBuf[ubi].tBuf[1].tBufState = LCD_BUF_EMPTY;
	}
}
//------------------------------------------------------------------------------
void LCD_SetOsdLogoJpegBufAddr (void)
{
	tLCD_BufInfor.tChBuf[0].tBuf[0].tBufState = LCD_BUF_EMPTY;
	tLCD_BufInfor.tChBuf[0].tBuf[1].tBufState = LCD_BUF_EMPTY;	
	tLCD_BufInfor.tChBuf[0].tBuf[1].ulBufAddr = tLCD_BufInfor.tChBuf[0].tBuf[0].ulBufAddr + (ulLCD_TotalChBufSize >> 1);
}
//------------------------------------------------------------------------------
void LCD_SetChBufReady (LCD_BUF_TYP *pBufInfor)
{
	pBufInfor->tBufState = LCD_BUF_FULL;
}
//------------------------------------------------------------------------------
void LCD_UpdateDynCropping (void)
{
	if (LCD->IMG0_EN && !LCD->IMG1_EN && !LCD->IMG2_EN && !LCD->IMG3_EN &&
		true == tLCD_DynSetting.bDyn)
	{				
		LCD->DISP0_HI_START   = tLCD_DynSetting.uwHiStart;
		LCD->DISP0_VI_START   = tLCD_DynSetting.uwViStart;
		LCD->DISP0_HI_SIZE    = tLCD_DynSetting.uwHiSize;
		LCD->DISP0_H_SIZE     = tLCD_DynSetting.uwHsize;
		LCD->DISP0_V_SIZE     = tLCD_DynSetting.uwVsize;
		LCD->LCD_FS0_HO_SIZE  = tLCD_DynSetting.uwHoSize;
		LCD->LCD_FS0_VO_SIZE  = tLCD_DynSetting.uwVoSize;
		LCD->LCD_FS0_HDUP_EN  = tLCD_DynSetting.ubFsHdupEN;
		LCD->LCD_FS0_HRATIO   = tLCD_DynSetting.uwHratio;
		LCD->LCD_FS0_HO_START = tLCD_DynSetting.uwHoStart;
		
		LCD->LCD_FS0_VDUP_EN  = tLCD_DynSetting.ubFsVdupEN;
		LCD->LCD_FS0_VRATIO   = tLCD_DynSetting.uwVratio;
		LCD->LCD_FS0_VO_START = tLCD_DynSetting.uwVoStart;
		LCD->LCD_FS0_EN       = tLCD_DynSetting.ubFsEN;
		LCD->LCD_FS0_FIR_EN   = tLCD_DynSetting.ubFirEN;
	}
	tLCD_DynSetting.bDyn = false;
}
//------------------------------------------------------------------------------
void LCD_VsyncIsr (void)
{
	uint8_t ubi, ubIndex;
	bool    bUpdate = false;

	LCD->CLR_LCD_INT = 1;
	INTC_IrqClear(INTC_LCD_IRQ);
	if((LCD_JPEG_ENABLE == tLCD_JpegEn) && (!LCD->LCD_JPEG_DEC_EN))
		LCD->LCD_JPEG_DEC_EN = tLCD_JpegEn;
	LCD_UpdateDynCropping();
	LCD->IMG0_EN = tLCD_ChEnable.bCh0;
	LCD->IMG1_EN = tLCD_ChEnable.bCh1;
	LCD->IMG2_EN = tLCD_ChEnable.bCh2;
	LCD->IMG3_EN = tLCD_ChEnable.bCh3;
	for (ubi=0; ubi<ubLCD_MaxChNow; ++ubi)
	{
		if (LCD_BUF_FULL == tLCD_BufInfor.tChBuf[ubi].tBuf[0].tBufState)
		{
			ubIndex = 0;
			bUpdate = true;
		}
		else if (LCD_BUF_FULL == tLCD_BufInfor.tChBuf[ubi].tBuf[1].tBufState)
		{
			ubIndex = 1;
			bUpdate = true;
		}
		else
		{
			if (LCD_CH_NOIMG_MAX_BOUNDARY > ubLCD_ChNoImgBoundary)
			{
				if (ubLCD_ChNoImgBoundary ==  ubLCD_ChNoImgCount[ubi])
				{
					LCD_ChDisable((LCD_CH_TYP)ubi);
					++(ubLCD_ChNoImgCount[ubi]);
				}
				else if (ubLCD_ChNoImgBoundary > ubLCD_ChNoImgCount[ubi])
					++(ubLCD_ChNoImgCount[ubi]);				
			}
			continue;
		}
		LCD_SetLcdChBufAddr ((LCD_CH_TYP)ubi, tLCD_BufInfor.tChBuf[ubi].tBuf[ubIndex].ulBufAddr);
		tLCD_BufInfor.tChBuf[ubi].tBuf[ubIndex].tBufState = LCD_BUF_CUR;
		tLCD_BufInfor.tChBuf[ubi].tBuf[LCD_PINGPONG_BUF_MAX - ubIndex - 1].tBufState = LCD_BUF_EMPTY;
		if (ubLCD_ChNoImgCount[ubi] > ubLCD_ChNoImgBoundary)
			LCD_ChEnable((LCD_CH_TYP)ubi);
		ubLCD_ChNoImgCount[ubi] = 0;
	}
	if (true == bOSD_UpdateAtVsync())	bUpdate = true;
	if (true == bUpdate)	LCD_FrameUpdate();
}
//------------------------------------------------------------------------------
void LCD_SetChannelDisableBoundary(uint8_t ubBoundary)
{
	ubLCD_ChNoImgBoundary = ubBoundary;
}
//------------------------------------------------------------------------------
void LCD_Start (void)
{
	if(ubLCD_StartFlag)
		return;

	memset(ubLCD_ChNoImgCount, 0, sizeof(ubLCD_ChNoImgCount));
	ubLCD_ChNoImgBoundary = LCD_CH_NOIMG_MAX_BOUNDARY;
	bLCD_IsrEnable = true;
	INTC_IrqSetup(INTC_LCD_IRQ, INTC_LEVEL_TRIG, LCD_VsyncIsr);
	INTC_IrqEnable(INTC_LCD_IRQ);
	LCD->CLR_LCD_INT = 1;
	LCD->LCD_INT_EN  = 1;
	LCD->TV_LCD_EN   = 1;
#if (LCD_PANEL == LCD_SSD2828_Y50019N00N)
	LCD_MIPI_SSD2828_Start();
#endif
	ubLCD_StartFlag = 1;
}
//------------------------------------------------------------------------------
void LCD_Stop(void)
{
	uint8_t ubi;
	
	bLCD_IsrEnable = false;
	LCD->TV_LCD_EN = 0;
	INTC_IrqDisable(INTC_LCD_IRQ);
	LCD->LCD_INT_EN = 0;
	LCD->CLR_LCD_INT = 1;
	for (ubi=0; ubi<LCD_MAX_CH; ++ubi)
	{
		tLCD_BufInfor.tChBuf[ubi].tBuf[0].tBufState = LCD_BUF_EMPTY;
		tLCD_BufInfor.tChBuf[ubi].tBuf[1].tBufState = LCD_BUF_EMPTY;
	}
	OSD_SetBufEmpty();
	ubLCD_StartFlag = 0;
}
//------------------------------------------------------------------------------
void LCD_Suspend (void)
{
#if (LCD_PANEL == LCD_SSD2828_Y50019N00N)
	SSP->SSP_GPIO_MODE = 1; //1:GPIO Mode  ???
	/*
	SSP->SSP_CLK_GPIO_OE = 1;
	SSP->SSP_FS_GPIO_OE = 1;
	SSP->SSP_TX_GPIO_OE = 1;
	SSP->SSP_RX_GPIO_OE = 1;

	SSP->SSP_CLK_GPIO_O = 0;
	SSP->SSP_FS_GPIO_O = 0;
	SSP->SSP_TX_GPIO_O = 0;
	SSP->SSP_RX_GPIO_O = 0;
	*/
	LCD_MIPI_SSD2828_Sleep();
#endif
	LCD->TV_LCD_EN 		= 0;
	GLB->LCDPLL_PD_N 	= 0;
	GLB->LCDPLL_LDO_EN 	= 0;
	GLB->LCD_RATE 		= 15;
	LCD->LCD_PCK_SPEED  = 255;
	ubLCD_StartFlag 	= 0;
}
//------------------------------------------------------------------------------
void LCD_Resume (void)
{
	GLB->LCD_RATE = 1;	
	//! LCD PLL
	if (!GLB->LCDPLL_PD_N)
	{
		GLB->LCDPLL_LDO_EN = 1;
		GLB->LCDPLL_PD_N = 1;
		TIMER_Delay_us(400);
		GLB->LCDPLL_SDM_EN = 1;
		TIMER_Delay_us(1);
	}
	LCD_PixelPllSetting();
	LCD->TV_LCD_EN = 1;
#if (LCD_PANEL == LCD_SSD2828_Y50019N00N)
	LCD_MIPI_SSD2828_Wakeup();
#endif
	ubLCD_StartFlag = 1;
}
//------------------------------------------------------------------------------
void LCD_ChEnable(LCD_CH_TYP tCh)
{
	LCD_WaitMutex;
	*((uint8_t*)(&tLCD_ChEnable)) |= (1 << tCh);
	if (false == bLCD_IsrEnable)
	{
		switch (tCh)
		{
			case LCD_CH0:
				LCD->IMG0_EN = 1;
				break;
			case LCD_CH1:
				LCD->IMG1_EN = 1;
				break;
			case LCD_CH2:
				LCD->IMG2_EN = 1;
				break;
			case LCD_CH3:
				LCD->IMG3_EN = 1;
				break;				
		}
	}
	LCD_ReleaseMutex;
}
//------------------------------------------------------------------------------
void LCD_ChDisable(LCD_CH_TYP tCh)
{
	if(!ubLCD_StartFlag)
		return;
	LCD_WaitMutex;
	*((uint8_t*)(&tLCD_ChEnable)) &= (0xF & (~(1 << tCh)));
	if (true == bLCD_IsrEnable)
	{
		switch (tCh)
		{
			case LCD_CH0:
				while(LCD->IMG0_EN)
					tLCD_ChEnable.bCh0 = 0;
				break;
			case LCD_CH1:
				while(LCD->IMG1_EN)
					tLCD_ChEnable.bCh1 = 0;
				break;
			case LCD_CH2:
				while(LCD->IMG2_EN)
					tLCD_ChEnable.bCh2 = 0;
				break;
			case LCD_CH3:
				while(LCD->IMG3_EN)
					tLCD_ChEnable.bCh3 = 0;
				break;				
		}
	}
	else
	{
		switch (tCh)
		{
			case LCD_CH0:
				LCD->IMG0_EN = 0;
				break;
			case LCD_CH1:
				LCD->IMG1_EN = 0;
				break;
			case LCD_CH2:
				LCD->IMG2_EN = 0;
				break;
			case LCD_CH3:
				LCD->IMG3_EN = 0;
				break;				
		}
	}
	LCD_ReleaseMutex;
}
//------------------------------------------------------------------------------
void LCD_JpegRwQTab (LCD_JPEG_QRW_TYP tType, uint8_t ubAddr, uint8_t ubLen, uint8_t *pData)
{
	uint8_t ubi;
	
	LCD->LCD_JPEG_DEC_EN = 0;
	LCD->LCD_QTAB_RAM_EN = 1;
	LCD->LCD_QTAB_RAM_A = ubAddr;
	LCD->LCD_AUTO_INC_QTAB_RAM_A = 1;		
	for (ubi=0; ubi<ubLen; ++ubi)
	{
		if (tType == LCD_JPEG_QW)
			LCD->LCD_QTAB_DAT = pData[ubi];
		else
		{
			LCD->LCD_QTAB_RD_TRG = 1;
			pData[ubi] = LCD->LCD_QTAB_DAT;
		}
	}
	LCD->LCD_QTAB_RAM_EN = 0;
	LCD->LCD_JPEG_DEC_EN = 1;
}
//------------------------------------------------------------------------------
LCD_RESULT tLCD_JpegDecodeEnable (LCD_CH_TYP tCh)
{
	if (LCD->LCD_JPEG_DEC_EN && tCh != LCD->LCD_JPEG_CH)
	{
		printf ("[LCD] LCD Jpeg Decode used by Ch%d\n", LCD->LCD_JPEG_CH);
		return LCD_JPEG_DECEN_FAIL;
	}
	LCD->LCD_JPEG_CH = tCh;
	tLCD_JpegEn = LCD_JPEG_ENABLE;
	LCD_ChEnable(tCh);

	return LCD_OK;
}
//------------------------------------------------------------------------------
LCD_RESULT tLCD_JpegDecodeDisable (void)
{
	if(LCD_JPEG_DISABLE == tLCD_JpegEn)
		return LCD_JPEG_DECDIS_FAIL;

	tLCD_JpegEn = LCD_JPEG_DISABLE;
	LCD_ChDisable((LCD_CH_TYP)LCD->LCD_JPEG_CH);
	LCD->LCD_JPEG_DEC_EN = 0;
	return LCD_JPEG_DECDIS_OK;
}
//------------------------------------------------------------------------------
LCD_JPEG_STATUS tLCD_GetJpegDecoderStatus(void)
{
	return tLCD_JpegEn;
}
//------------------------------------------------------------------------------
void LCD_UnInit (void)
{
	tOutputNow = LCD_UNINIT;
	bLCD_IsrEnable = false;
	*((uint8_t*)(&tLCD_ChEnable)) = 0;
	LCD_Stop();
	LCD_ChDisable(LCD_CH0);
	LCD_ChDisable(LCD_CH1);
	LCD_ChDisable(LCD_CH2);
	LCD_ChDisable(LCD_CH3);	
	OSD_Disable();	
	tLCD_JpegDecodeDisable();
	//! Clear Buffer State;
}
//------------------------------------------------------------------------------
void LCD_FrameUpdate (void)
{
#if (LCD_PANEL == LCD_TM024HDH03_8080_8Bit2Byte && LCD_PANEL == LCD_TM024HDH03_8080_8Bit3Byte)
	LCD->TV_LCD_EN = 0;
	TM024HDH03_Cmd(0x22);
	LCD->TV_LCD_EN = 1;
#elif (LCD_PANEL == LCD_TM024HDH03_8080_9Bit)
	LCD->TV_LCD_EN = 0;
	TM024HDH03_Cmd(0x22 << 1);
	LCD->TV_LCD_EN = 1;
#elif (LCD_PANEL >= LCD_TM023KDH03_8080_8Bit2Byte && LCD_PANEL <= LCD_TM023KDH03_8080_16Bit)
	LCD->TV_LCD_EN = 0;
	TM023KDH03_CMD(0x2C);
	LCD->TV_LCD_EN = 1;
	
//! Add New LCD Panel Tranmit Image
	
#endif
}
//------------------------------------------------------------------------------
void LCD_LcdPanelInit (void)
{
#if (LCD_PANEL == LCD_GPM1125A0_AU_UPS051)
	bLCD_GPM1125A0_SerialRgbData();
#elif (LCD_PANEL == LCD_GPM1125A0_RGB_DUMMY)
	bLCD_GPM1125A0_RgbDummy();
#elif (LCD_PANEL == LCD_GPM1125A0_BT601)
	bLCD_GPM1125A0_CCIR601();
#elif (LCD_PANEL == LCD_GPM1125A0_BT656)
	bLCD_GPM1125A0_CCIR656();
#elif (LCD_PANEL == LCD_TM035KDH03_24HV)
	bLCD_TM035KDH03_24RgbHv();
#elif (LCD_PANEL == LCD_TM035KDH03_24DE)
	bLCD_TM035KDH03_24RgbDe();	
#elif (LCD_PANEL == LCD_TM035KDH03_8HV)
	bLCD_TM035KDH03_8RgbHv();
#elif (LCD_PANEL == LCD_TM035KDH03_BT601)
	bLCD_TM035KDH03_IturBt601();
#elif (LCD_PANEL == LCD_TM035KDH03_BT656)
	bLCD_TM035KDH03_IturBt656();
#elif (LCD_PANEL == LCD_SN9C271A_YUV422)
	LCD_SN9C271A_Yuv422();
#elif (LCD_PANEL == LCD_SSD2828_ || LCD_PANEL == LCD_SSD2828_Y50019N00N)
	if (true == bLCD_MIPI_SSD2828_Init())
		LCD_MIPI_SSD2828();	
#elif (LCD_PANEL == LCD_TM024HDH03_8080_8Bit2Byte)
	LCD_TM024HDH03_CPU8(LCD_8080_8_2BYTE);
	LCD_TM024HDH03_Init(0x55, 0);
#elif (LCD_PANEL == LCD_TM024HDH03_8080_8Bit3Byte)
	LCD_TM024HDH03_CPU8(LCD_8080_8_3BYTE);
	LCD_TM024HDH03_Init(0xE6, 0);	
#elif (LCD_PANEL == LCD_TM024HDH03_8080_9Bit)
	LCD_TM024HDH03_CPU8(LCD_8080_9);
	LCD_TM024HDH03_Init(0x66, 1);
#elif (LCD_PANEL == LCD_TM023KDH03_8080_8Bit2Byte)
	LCD_TM023KDH03_CPU(LCD_8080_8_2BYTE);
	LCD_TM023KDH03_Init(0x55);
#elif (LCD_PANEL == LCD_TM023KDH03_8080_8Bit3Byte)
	LCD_TM023KDH03_CPU(LCD_8080_8_3BYTE);
	LCD_TM023KDH03_Init(0x66);
#elif (LCD_PANEL == LCD_TM023KDH03_8080_16Bit)
	LCD_TM023KDH03_CPU(LCD_8080_16);
	LCD_TM023KDH03_Init(0x55);	
//! Add New LCD Panel Initial
#elif (LCD_PANEL == LCD_TEST_PANEL)
	if (true == bLCD_MIPI_SSD2828_Init())
		LCD_MIPI_SSD2828();		
#elif (LCD_PANEL == LCD_HSD070IDW1_24DE)
	LCD_HSD070IDW1_Init();	
#endif	
}
//------------------------------------------------------------------------------
void LCD_LcdTvInit(void)
{
#if (LCD_TV_OUT == LCD_TV_NTSC_P)
	LCD_TV_Init(NTSC, PROGRESSIVE);
#elif (LCD_TV_OUT == LCD_TV_NTSC_I)
	LCD_TV_Init(NTSC, INTERLACE);
#elif (LCD_TV_OUT == LCD_TV_NTSC443_P)	
	LCD_TV_Init(NTSC_443, PROGRESSIVE);
#elif (LCD_TV_OUT == LCD_TV_NTSC443_I)	
	LCD_TV_Init(NTSC_443, INTERLACE);
#elif (LCD_TV_OUT == LCD_TV_PAL_P)	
	LCD_TV_Init(PAL, PROGRESSIVE);
#elif (LCD_TV_OUT == LCD_TV_PAL_I)	
	LCD_TV_Init(PAL, INTERLACE);	
#elif (LCD_TV_OUT == LCD_TV_PALM_P)	
	LCD_TV_Init(PAL_M, PROGRESSIVE);
#elif (LCD_TV_OUT == LCD_TV_PALM_I)	
	LCD_TV_Init(PAL_M, INTERLACE);	
#endif	
}
//------------------------------------------------------------------------------
void LCD_LcdHdmiInit (void)
{
#if (LCD_HDMI_OUT == LCD_HDMI_IT66121)	
	HDMI_LcdTimingInit();
#endif
}
//------------------------------------------------------------------------------
void LCD_BackLightCtrl (LCD_BL_TYP tLevel)
{
}
//------------------------------------------------------------------------------
void LCD_Init (LCD_OUTPUT_TYP tLcdOutput)
{
	tLCD_JpegEn          = LCD_JPEG_DISABLE;
	tLCD_DynSetting.bDyn = false;

	if (tOutputNow != tLcdOutput)
	{
		//LCD_UnInit();
		
		//!	memset(ulLCD_ChBufSize, 0, sizeof(ulLCD_ChBufSize));
		ubLCD_MaxChNow = 0;
		//! LCD PLL
		if (!GLB->LCDPLL_PD_N)
		{
			GLB->LCDPLL_LDO_EN = 1;
			GLB->LCDPLL_PD_N = 1;
			TIMER_Delay_us(400);
			GLB->LCDPLL_SDM_EN = 1;
			TIMER_Delay_us(1);
		}

		//reset pin 20180704
		#if 1
		GPIO->GPIO_OE3= 1;
		GPIO->GPIO_O3 = 1;
		TIMER_Delay_ms(20);
		GPIO->GPIO_O3 = 0;
		TIMER_Delay_ms(100);
		GPIO->GPIO_O3 = 1;
		TIMER_Delay_ms(100);
		#endif

		switch (tLcdOutput)
		{
			case LCD_LCD_PANEL:
				LCD_LcdPanelInit();
				break;
			case LCD_TV:
				LCD_LcdTvInit();			
				break;
			case LCD_HDMI:
				LCD_LcdHdmiInit();	
			default:
				break;		
		}
		tOutputNow = tLcdOutput;
		
		LCD->LCD_FS0_FIR4 = 2;
		LCD->LCD_FS0_FIR3 = 4;
		LCD->LCD_FS0_FIR2 = 8;
		LCD->LCD_FS0_FIR1 = 16;
		LCD->LCD_FS0_FIR0 = 256 - ((16 + 8 + 4 + 2) << 1);
		
		LCD->LCD_FS1_FIR4 = 2;
		LCD->LCD_FS1_FIR3 = 4;
		LCD->LCD_FS1_FIR2 = 8;
		LCD->LCD_FS1_FIR1 = 16;
		LCD->LCD_FS1_FIR0 = 256 - ((16 + 8 + 4 + 2) << 1);
		LCD_PixelPllSetting();
	}
	ubLCD_StartFlag = 0;
	if(osLCD_Mutex == NULL) 
	{
		osLCD_Mutex = osMutexCreate(osMutex(osLCD_Mutex));
		if(osLCD_Mutex == NULL)
			printf("LCD Init: create mutex fail!\n");
	}
}
//------------------------------------------------------------------------------
void LCD_WinSet(LCD_DISP_TYPE tDispType, uint16_t uwLcdOutputHsize, uint16_t uwLcdOutputVsize)
{
	uint16_t uwWinXStart, uwWinYStart;
	
	if (uwLCD_GetLcdHoSize() > uwLcdOutputHsize)
		uwWinXStart = (uwLCD_GetLcdHoSize() - uwLcdOutputHsize) >> 1;
	else
	{
		uwWinXStart = 0;
		uwLcdOutputHsize = uwLCD_GetLcdHoSize();
	}
	if (uwLCD_GetLcdVoSize() > uwLcdOutputVsize)
		uwWinYStart = (uwLCD_GetLcdVoSize() - uwLcdOutputVsize) >> 1;
	else
	{
		uwWinYStart = 0;
		uwLcdOutputVsize = uwLCD_GetLcdVoSize();
	}
	switch (tDispType)
	{
		case LCD_DISP_4T:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND1_Y_START = uwWinYStart;
			LCD->WND2_X_START = uwWinXStart;
			LCD->WND2_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			LCD->WND3_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND3_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_3TC:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart + (uwLcdOutputVsize >> 2);
			LCD->WND2_X_START = uwWinXStart;
			LCD->WND2_Y_START = LCD->WND1_Y_START + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_3T_1U2D:
			LCD->WND0_X_START = uwWinXStart + (uwLcdOutputHsize >> 2);
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			LCD->WND2_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND2_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_3T_2U1D:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND1_Y_START = uwWinYStart;
			LCD->WND2_X_START = uwWinXStart + (uwLcdOutputHsize >> 2);
			LCD->WND2_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_3T_2L1R:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND1_Y_START = uwWinYStart;
			LCD->WND2_X_START = uwWinXStart;
			LCD->WND2_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_3T_1L2R:
			LCD->WND0_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart;
			LCD->WND2_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND2_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_3T_2PIP:
			LCD->WND0_X_START = uwWinXStart + ((uwLcdOutputHsize - LCD_HVIEW_CAMF_HSIZE) >> 1);
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart;
			LCD->WND2_X_START = LCD->WND0_X_START;
			LCD->WND2_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_2T_H:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_2T_H13:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart + (uwLcdOutputHsize >> 2);
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_2T_H12:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart + uwLcdOutputHsize / 3;
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_2T_V:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			break;
		case LCD_DISP_2T_PIP_0:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_2T_PIP_1:
			LCD->WND0_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND0_Y_START = uwWinYStart;
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_2T_PIP_2:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_2T_PIP_3:
			LCD->WND0_X_START = uwWinXStart + (uwLcdOutputHsize >> 1);
			LCD->WND0_Y_START = uwWinYStart + (uwLcdOutputVsize >> 1);
			LCD->WND1_X_START = uwWinXStart;
			LCD->WND1_Y_START = uwWinYStart;
			break;
		case LCD_DISP_1T:
			LCD->WND0_X_START = uwWinXStart;
			LCD->WND0_Y_START = uwWinYStart;
			break;
	}
}
//------------------------------------------------------------------------------
LCD_RESULT tLCD_DynamicOneChCropScale (LCD_DYN_INFOR_TYP *pDynInfor)
{
	if (LCD->IMG0_EN && !LCD->IMG1_EN && !LCD->IMG2_EN && !LCD->IMG3_EN)
	{
		if (ulLCD_ChBufSize[0] >= 
		   (pDynInfor->tChRes.uwChInputHsize * 
		   ((pDynInfor->tChRes.uwCropVstart + pDynInfor->tChRes.uwCropVsize) >> 1) * 3))
		{
			if ((pDynInfor->tChRes.uwCropHsize != 0) && (pDynInfor->tChRes.uwCropVsize != 0))
			{
				if (pDynInfor->tChRes.uwCropHstart + pDynInfor->tChRes.uwCropHsize <= 
					pDynInfor->tChRes.uwChInputHsize &&
					pDynInfor->tChRes.uwCropVstart + pDynInfor->tChRes.uwCropVsize <=
					pDynInfor->tChRes.uwChInputVsize)
				{
					if (uwLCD_GetLcdHoSize() < pDynInfor->uwLcdOutputHsize)
						pDynInfor->uwLcdOutputHsize = uwLCD_GetLcdHoSize();
					if (uwLCD_GetLcdVoSize() < pDynInfor->uwLcdOutputVsize)
						pDynInfor->uwLcdOutputVsize = uwLCD_GetLcdVoSize();
					if (pDynInfor->tChRes.uwCropHsize <= (pDynInfor->uwLcdOutputHsize << 2) &&
					   (pDynInfor->tChRes.uwCropHsize << 3) >= pDynInfor->uwLcdOutputHsize &&
						pDynInfor->tChRes.uwCropVsize <= (pDynInfor->uwLcdOutputVsize << 2) &&
					   (pDynInfor->tChRes.uwCropVsize << 3) >= pDynInfor->uwLcdOutputVsize)
					{
						tLCD_DynSetting.bDyn       = false;
						tLCD_DynSetting.uwHiSize   = pDynInfor->tChRes.uwChInputHsize >> 2;
						tLCD_DynSetting.uwHiStart  = pDynInfor->tChRes.uwCropHstart >> 2;
						tLCD_DynSetting.uwViStart  = pDynInfor->tChRes.uwCropVstart;
						tLCD_DynSetting.uwHsize    = pDynInfor->tChRes.uwCropHsize >> 2;
						tLCD_DynSetting.uwVsize    = pDynInfor->tChRes.uwCropVsize;						
						tLCD_DynSetting.uwHoSize   = pDynInfor->uwLcdOutputHsize;
						tLCD_DynSetting.uwVoSize   = pDynInfor->uwLcdOutputVsize;
						tLCD_DynSetting.ubFsHdupEN = (pDynInfor->uwLcdOutputHsize > (pDynInfor->tChRes.uwCropHsize << 2))?1:0;
						tLCD_DynSetting.uwHratio   = 0x80 * pDynInfor->tChRes.uwCropHsize / 
												     (pDynInfor->uwLcdOutputHsize >> tLCD_DynSetting.ubFsHdupEN);
						tLCD_DynSetting.uwHoStart  = (((((uint32_t)(0x80 * pDynInfor->tChRes.uwCropHsize / 
													 tLCD_DynSetting.uwHratio)) << tLCD_DynSetting.ubFsHdupEN) - 
													 pDynInfor->uwLcdOutputHsize) >> 1) & ~1;						
						
						tLCD_DynSetting.ubFsVdupEN = (pDynInfor->uwLcdOutputVsize > (pDynInfor->tChRes.uwCropVsize << 2))?1:0;						
						tLCD_DynSetting.uwVratio   = 0x80 * pDynInfor->tChRes.uwCropVsize /
												     (pDynInfor->uwLcdOutputVsize >> tLCD_DynSetting.ubFsVdupEN);
						tLCD_DynSetting.uwVoStart  = (((((uint32_t)(0x80 * pDynInfor->tChRes.uwCropVsize /
													 tLCD_DynSetting.uwVratio)) << tLCD_DynSetting.ubFsVdupEN) - 
													 pDynInfor->uwLcdOutputVsize) >> 1) & ~1;						
						tLCD_DynSetting.ubFsEN     = (0x80 == tLCD_DynSetting.uwHratio && 0x80 == tLCD_DynSetting.uwVratio)?0:1;
						tLCD_DynSetting.ubFirEN    = (tLCD_DynSetting.ubFsEN)?1:0;	
						tLCD_DynSetting.bDyn = true;
						return LCD_OK;
					}
					return LCD_SCALE_FAIL;
				}
				return LCD_CROP_FAIL;
			}
		}
	}
	return LCD_DISP_FAIL;
}
//------------------------------------------------------------------------------
LCD_RESULT tLCD_CropScale(LCD_INFOR_TYP *pLcdInfor)
{
	uint16_t uwHoSize, uwVoSize;
	uint8_t ubi;
	
	if (uwLCD_GetLcdHoSize() < pLcdInfor->uwLcdOutputHsize)
		pLcdInfor->uwLcdOutputHsize = uwLCD_GetLcdHoSize();
	if (uwLCD_GetLcdVoSize() < pLcdInfor->uwLcdOutputVsize)
		pLcdInfor->uwLcdOutputVsize = uwLCD_GetLcdVoSize();
	if (LCD_MAX_CH < pLcdInfor->ubChNum)	pLcdInfor->ubChNum = LCD_MAX_CH;
	ubLCD_MaxChNow = pLcdInfor->ubChNum;
	for (ubi=0; ubi<pLcdInfor->ubChNum; ++ubi)
	{
		if (!pLcdInfor->tChRes[ubi].uwCropHsize || !pLcdInfor->tChRes[ubi].uwCropVsize)
        continue;
		
		if (pLcdInfor->tChRes[ubi].uwCropHstart + pLcdInfor->tChRes[ubi].uwCropHsize > pLcdInfor->tChRes[ubi].uwChInputHsize
			|| pLcdInfor->tChRes[ubi].uwCropVstart + pLcdInfor->tChRes[ubi].uwCropVsize > pLcdInfor->tChRes[ubi].uwChInputVsize)
		{
			printf ("[LCD] Channel %d Cropping error\n", ubi);
			return LCD_CROP_FAIL;
		}
		switch (pLcdInfor->tDispType)
		{
			case LCD_DISP_4T:
			case LCD_DISP_3T_1U2D:
			case LCD_DISP_3T_2U1D:
				uwHoSize = pLcdInfor->uwLcdOutputHsize >> 1;
				uwVoSize = pLcdInfor->uwLcdOutputVsize >> 1;
				break;
			case LCD_DISP_3T_2L1R:
			case LCD_DISP_3T_1L2R:
				uwHoSize = pLcdInfor->uwLcdOutputHsize >> 1;
				uwVoSize = pLcdInfor->uwLcdOutputVsize >> ((1 == ubi)?0:1);
				break;
			case LCD_DISP_3TC:
				uwHoSize = pLcdInfor->uwLcdOutputHsize;
				uwVoSize = pLcdInfor->uwLcdOutputVsize >> ((1 == ubi)?1:2);
				break;
			case LCD_DISP_3T_2PIP:
				if (pLcdInfor->uwLcdOutputHsize > LCD_HVIEW_CAMF_HSIZE)
				{
					uwHoSize = (1 == ubi)?pLcdInfor->uwLcdOutputHsize:LCD_HVIEW_CAMF_HSIZE;
					uwVoSize = pLcdInfor->uwLcdOutputVsize >> ((1 == ubi)?0:1);
					break;
				}
				else
				{
					printf ("[LCD] Channel %d Display Type error\n", ubi);
					return LCD_DISP_FAIL;
				}					
			case LCD_DISP_2T_H:
				uwHoSize = pLcdInfor->uwLcdOutputHsize >> 1;
				uwVoSize = pLcdInfor->uwLcdOutputVsize;
				break;
			case LCD_DISP_2T_H13:
				uwHoSize = (pLcdInfor->uwLcdOutputHsize >> 2) + ((ubi)?(pLcdInfor->uwLcdOutputHsize >> 1):0);
				uwVoSize = pLcdInfor->uwLcdOutputVsize;
				break;
			case LCD_DISP_2T_H12:
				uwHoSize = (pLcdInfor->uwLcdOutputHsize << ubi) / 3;
				uwVoSize = pLcdInfor->uwLcdOutputVsize;
				break;
			case LCD_DISP_2T_V:
				uwHoSize = pLcdInfor->uwLcdOutputHsize;
				uwVoSize = pLcdInfor->uwLcdOutputVsize >> 1;
				break;
			case LCD_DISP_2T_PIP_0:
			case LCD_DISP_2T_PIP_1:
			case LCD_DISP_2T_PIP_2:
			case LCD_DISP_2T_PIP_3:
				if (ubi)
				{
					uwHoSize = pLcdInfor->uwLcdOutputHsize;
					uwVoSize = pLcdInfor->uwLcdOutputVsize;
				}
				else
				{
					uwHoSize = pLcdInfor->uwLcdOutputHsize >> 1;
					uwVoSize = pLcdInfor->uwLcdOutputVsize >> 1;
				}
				break;
			case LCD_DISP_1T:
				uwHoSize = pLcdInfor->uwLcdOutputHsize;
				uwVoSize = pLcdInfor->uwLcdOutputVsize;
				break;
		}
		if (pLcdInfor->tChRes[ubi].uwCropHsize > (uwHoSize << 2) || uwHoSize > (pLcdInfor->tChRes[ubi].uwCropHsize << 3)
			|| pLcdInfor->tChRes[ubi].uwCropVsize > (uwVoSize << 2) || uwVoSize > (pLcdInfor->tChRes[ubi].uwCropVsize << 3))
		{
			printf ("[LCD] Channel %d Scale error\n", ubi);
			return LCD_SCALE_FAIL;
		}
		switch (ubi)
		{
			case LCD_CH0:
				LCD->LCD_FS0_HDUP_EN = (uwHoSize > (pLcdInfor->tChRes[ubi].uwCropHsize << 2))?1:0;
				LCD->DISP0_HI_SIZE = pLcdInfor->tChRes[ubi].uwChInputHsize >> 2;
				LCD->DISP0_HI_START = pLcdInfor->tChRes[ubi].uwCropHstart >> 2;
				LCD->DISP0_VI_START = pLcdInfor->tChRes[ubi].uwCropVstart;
				LCD->DISP0_H_SIZE = pLcdInfor->tChRes[ubi].uwCropHsize >> 2;
				LCD->DISP0_V_SIZE = pLcdInfor->tChRes[ubi].uwCropVsize;
				LCD->LCD_FS0_EN = 1;
				LCD->LCD_FS0_HO_SIZE = uwHoSize;
				LCD->LCD_FS0_HRATIO = 0x80 * pLcdInfor->tChRes[ubi].uwCropHsize / (uwHoSize >> LCD->LCD_FS0_HDUP_EN);
				LCD->LCD_FS0_HO_START = (((((uint32_t)(0x80 * pLcdInfor->tChRes[ubi].uwCropHsize / LCD->LCD_FS0_HRATIO)) << LCD->LCD_FS0_HDUP_EN) - uwHoSize) >> 1) & ~1;
				
				LCD->LCD_FS0_VDUP_EN = (uwVoSize > (pLcdInfor->tChRes[ubi].uwCropVsize << 2))?1:0;
				LCD->LCD_FS0_VO_SIZE = uwVoSize;
				LCD->LCD_FS0_VRATIO = 0x80 * pLcdInfor->tChRes[ubi].uwCropVsize / (uwVoSize >> LCD->LCD_FS0_VDUP_EN);
				LCD->LCD_FS0_VO_START = (((((uint32_t)(0x80 * pLcdInfor->tChRes[ubi].uwCropVsize / LCD->LCD_FS0_VRATIO)) << LCD->LCD_FS0_VDUP_EN) - uwVoSize) >> 1) & ~1;
				LCD->LCD_FS0_FIR_EN = (LCD->LCD_FS0_HRATIO != 0x80 || LCD->LCD_FS0_VRATIO != 0x80)?1:0;				
				break;
			case LCD_CH1:
				LCD->LCD_FS1_HDUP_EN = (uwHoSize > (pLcdInfor->tChRes[ubi].uwCropHsize << 2))?1:0;
				LCD->DISP1_HI_SIZE = pLcdInfor->tChRes[ubi].uwChInputHsize >> 2;
				LCD->DISP1_HI_START = pLcdInfor->tChRes[ubi].uwCropHstart >> 2;
				LCD->DISP1_VI_START = pLcdInfor->tChRes[ubi].uwCropVstart;
				LCD->DISP1_H_SIZE = pLcdInfor->tChRes[ubi].uwCropHsize >> 2;
				LCD->DISP1_V_SIZE = pLcdInfor->tChRes[ubi].uwCropVsize;
				LCD->LCD_FS1_EN = 1;
				LCD->LCD_FS1_HO_SIZE = uwHoSize;
				LCD->LCD_FS1_HRATIO = 0x80 * pLcdInfor->tChRes[ubi].uwCropHsize / (uwHoSize >> LCD->LCD_FS1_HDUP_EN);
				LCD->LCD_FS1_HO_START = (((((uint32_t)(0x80 * pLcdInfor->tChRes[ubi].uwCropHsize / LCD->LCD_FS1_HRATIO)) << LCD->LCD_FS1_HDUP_EN) - uwHoSize) >> 1) & ~1;
				
				LCD->LCD_FS1_VDUP_EN = (uwVoSize > (pLcdInfor->tChRes[ubi].uwCropVsize << 2))?1:0;
				LCD->LCD_FS1_VO_SIZE = uwVoSize;
				LCD->LCD_FS1_VRATIO = 0x80 * pLcdInfor->tChRes[ubi].uwCropVsize / (uwVoSize >> LCD->LCD_FS1_VDUP_EN);
				LCD->LCD_FS1_VO_START = (((((uint32_t)(0x80 * pLcdInfor->tChRes[ubi].uwCropVsize / LCD->LCD_FS1_VRATIO)) << LCD->LCD_FS1_VDUP_EN) - uwVoSize) >> 1) & ~1;
				LCD->LCD_FS1_FIR_EN = (LCD->LCD_FS1_HRATIO != 0x80 || LCD->LCD_FS1_VRATIO != 0x80)?1:0;
				break;
			case LCD_CH2:
				if (pLcdInfor->tChRes[ubi].uwCropHsize != pLcdInfor->tChRes[0].uwCropHsize || pLcdInfor->tChRes[ubi].uwCropVsize != pLcdInfor->tChRes[0].uwCropVsize)
				{
					printf ("[LCD] Channel 0 and %d are not the same Scale\n", ubi);
					return LCD_SCALE_FAIL;
				}
				LCD->DISP2_HI_SIZE = pLcdInfor->tChRes[ubi].uwChInputHsize >> 2;
				LCD->DISP2_HI_START = pLcdInfor->tChRes[ubi].uwCropHstart >> 2;
				LCD->DISP2_VI_START = pLcdInfor->tChRes[ubi].uwCropVstart;
				LCD->DISP2_H_SIZE = pLcdInfor->tChRes[ubi].uwCropHsize >> 2;
				LCD->DISP2_V_SIZE = pLcdInfor->tChRes[ubi].uwCropVsize;
				break;
			case LCD_CH3:
				if (pLcdInfor->tChRes[ubi].uwCropHsize != pLcdInfor->tChRes[1].uwCropHsize || pLcdInfor->tChRes[ubi].uwCropVsize != pLcdInfor->tChRes[1].uwCropVsize)
				{
					printf ("[LCD] Channel 1 and %d are not the same Scale\n", ubi);
					return LCD_SCALE_FAIL;
				}
				LCD->DISP3_HI_SIZE = pLcdInfor->tChRes[ubi].uwChInputHsize >> 2;
				LCD->DISP3_HI_START = pLcdInfor->tChRes[ubi].uwCropHstart >> 2;
				LCD->DISP3_VI_START = pLcdInfor->tChRes[ubi].uwCropVstart;
				LCD->DISP3_H_SIZE = pLcdInfor->tChRes[ubi].uwCropHsize >> 2;
				LCD->DISP3_V_SIZE = pLcdInfor->tChRes[ubi].uwCropVsize;
				break;
		}
//		ulLCD_ChBufSize[ubi] = pLcdInfor->tChRes[ubi].uwChInputHsize * (pLcdInfor->tChRes[ubi].uwChInputVsize + 
//		(pLcdInfor->tChRes[ubi].uwChInputVsize >> 1) + ((pLcdInfor->tChRes[ubi].uwChInputVsize & 1)?1:0));
//		if (3 & ulLCD_ChBufSize[ubi])
//			ulLCD_ChBufSize[ubi] = ((ulLCD_ChBufSize[ubi] >> 2) + 1) << 2;
	}
//	for (;ubi<LCD_MAX_CH; ++ubi)
//		ulLCD_ChBufSize[ubi] = 0;
	LCD_WinSet(pLcdInfor->tDispType, pLcdInfor->uwLcdOutputHsize, pLcdInfor->uwLcdOutputVsize);
	return LCD_OK;
}
//------------------------------------------------------------------------------
LCD_RESULT tLCD_WinSft(LCD_CH_TYP tCh, short swWinXSft, short swWinYSft)
{
	switch (tCh)
	{
		case LCD_CH0:
			swWinXSft += LCD->WND0_X_START;
			swWinYSft += LCD->WND0_Y_START;
			LCD->WND0_X_START = (0x1000 > swWinXSft)?(0 > swWinXSft)?0:swWinXSft:0xFFF;
			LCD->WND0_Y_START = (0x1000 > swWinYSft)?(0 > swWinYSft)?0:swWinYSft:0xFFF;
			break;
		case LCD_CH1:
			swWinXSft += LCD->WND1_X_START;
			swWinYSft += LCD->WND1_Y_START;
			LCD->WND1_X_START = (0x1000 > swWinXSft)?(0 > swWinXSft)?0:swWinXSft:0xFFF;
			LCD->WND1_Y_START = (0x1000 > swWinYSft)?(0 > swWinYSft)?0:swWinYSft:0xFFF;
			break;
		case LCD_CH2:
			swWinXSft += LCD->WND2_X_START;
			swWinYSft += LCD->WND2_Y_START;
			LCD->WND2_X_START = (0x1000 > swWinXSft)?(0 > swWinXSft)?0:swWinXSft:0xFFF;
			LCD->WND2_Y_START = (0x1000 > swWinYSft)?(0 > swWinYSft)?0:swWinYSft:0xFFF;
			break;
		case LCD_CH3:
			swWinXSft += LCD->WND3_X_START;
			swWinYSft += LCD->WND3_Y_START;
			LCD->WND3_X_START = (0x1000 > swWinXSft)?(0 > swWinXSft)?0:swWinXSft:0xFFF;
			LCD->WND3_Y_START = (0x1000 > swWinYSft)?(0 > swWinYSft)?0:swWinYSft:0xFFF;
			break;				
	}
	return ((0x1000 < swWinXSft || 0 > swWinXSft) && (0x1000 < swWinYSft || 0 > swWinYSft))?LCD_WINXY_FAIL:
		(0x1000 < swWinXSft || 0 > swWinXSft)?LCD_WINX_FAIL:(0x1000 < swWinYSft || 0 > swWinYSft)?LCD_WINY_FAIL:LCD_OK;
}
//------------------------------------------------------------------------------
void LCD_SetLcdChBufAddr (LCD_CH_TYP tCh, uint32_t ulAddr)
{
	switch (tCh)
	{
		case LCD_CH0:
			LCD->DISP0_STR_A = ulAddr >> 2;
			break;
		case LCD_CH1:
			LCD->DISP1_STR_A = ulAddr >> 2;
			break;
		case LCD_CH2:
			LCD->DISP2_STR_A = ulAddr >> 2;
			break;
		case LCD_CH3:
			LCD->DISP3_STR_A = ulAddr >> 2;
			break;				
	}	
}
//------------------------------------------------------------------------------
void LCD_GpioIsr (void)
{
    uint8_t ubIndex;

	printf("LCD int\n");
	if (GPIO->GPIO_INTR_FLAG1)
	{
		ubIndex = 1;
		GPIO->GPIO_INTR_MSK1 = 1;
        if(osMessagePut(LCD_GpioQueue, &ubIndex, 0) == osErrorResource)
		{
			printf("LCD GPIO Queue Reset\n");
            osMessageReset(LCD_GpioQueue);
		}
	}
	else if	(GPIO->GPIO_INTR_FLAG6)
	{
		GPIO->GPIO_TRG_LEVEL6 = (GPIO->GPIO_I6)?1:0;	
		if (LCD_HDMI_INIT != tLCD_Mode && LCD_HDMI_MODE == tLCD_Mode)
		{
			ubIndex = 6;
            if(osMessagePut(LCD_GpioQueue, &ubIndex, 0) == osErrorResource)
                printf("LCD TV -> HDMI event queue full!\n");
		}
	}
	GPIO->CLR_GPIO_INTR = 0x1FFF;
	INTC_IrqClear(INTC_GPIO_IRQ);	
}
//------------------------------------------------------------------------------
static void LCD_Switch_Thread(void const *argument)
{
	LCD_ROTATE_TYP tLcdRotate;	
	uint8_t 	   ubIndex = 1;
	
    osMessagePut(LCD_GpioQueue, &ubIndex, 0);
	while(1)
	{
        osMessageGet(LCD_GpioQueue, &ubIndex, osWaitForever);
		switch (ubIndex)
		{
			case 1:
				printf("Test GPIO 1\n");
				switch (tHDMI_HdmiDetect())
				{
					case HDMI_PLUG_IN:
						printf("HDMI plug in\n");
						tLCD_Mode = LCD_HDMI_INIT;
						osMessagePut(LCD_GpioQueue, &ubIndex, osWaitForever);
						break;
					case HDMI_OUTPUT:
						GPIO->GPIO_INTR_MSK1 = 0;
						printf("HDMI output\n");
						tLCD_Mode = LCD_HDMI_MODE;
						LCD_Init (LCD_HDMI);
						tLcdRotate = LCD_NO_ROTATE;
						osMessagePut(LCD_SwitchModeQueue, &tLcdRotate, 0);
						break;
					case HDMI_PLUG_OUT:
						GPIO->GPIO_INTR_MSK1 = 0;
						printf("HDMI plug out\n");
						ubIndex = 6;
						osMessagePut(LCD_GpioQueue, &ubIndex, 0);
						break;
				}
				break;
			case 6:
				if (GPIO->GPIO_I6)
				{
					printf("TV\n");
					tLCD_Mode = LCD_TV_MODE;				
					LCD_Init (LCD_TV);
					tLcdRotate = LCD_NO_ROTATE;
				}
				else
				{
					printf("LCD\n");
					tLCD_Mode = LCD_PANEL_MODE;
					//! Power On LCD;
					//! Enable Back Light
					LCD_Init (LCD_LCD_PANEL);
					tLcdRotate = LCD_ROTATE_90;
				}
				osMessagePut(LCD_SwitchModeQueue, &tLcdRotate, 0);
			default:
				break;
		}
	}
}
//------------------------------------------------------------------------------
osMessageQId* pLCD_SwitchModeInit(uint32_t ulStackSize, osPriority priority)
{
	tLCD_Mode = LCD_UNKNOW;
	HDMI_Init();
	GPIO->GPIO_PULL_HIGH1 = 1;
	GPIO->GPIO_PULL_EN1 = 1;
	GPIO->GPIO_OE1 = 0;	
//	GPIO->GPIO_PULL_HIGH6 = 1;
//	GPIO->GPIO_PULL_EN6 = 1;
//	GPIO->GPIO_OE6 = 0;		
	
	INTC_IrqSetup(INTC_GPIO_IRQ, INTC_LEVEL_TRIG, LCD_GpioIsr);
	INTC_IrqEnable(INTC_GPIO_IRQ);
	GPIO->GPIO_TRG_MODE1 = 0;
	GPIO->GPIO_TRG_EDGE1 = 0;
	GPIO->GPIO_TRG_LEVEL1 = 1;	
	GPIO->CLR_GPIO_INTR1 = 1;
	GPIO->GPIO_INTR_MSK1 = 0;	
//	GPIO->GPIO_TRG_MODE6 = 0;	
//	GPIO->GPIO_TRG_EDGE6 = 0;	
//	GPIO->GPIO_TRG_LEVEL6 = 0;	
//	GPIO->CLR_GPIO_INTR6 = 1;	
//	GPIO->GPIO_INTR_MSK6 = 0;
	
    osMessageQDef(LCD_SwitchMode, LCD_MODE_QUEUE_SZ, LCD_ROTATE_TYP);
    LCD_SwitchModeQueue = osMessageCreate(osMessageQ(LCD_SwitchMode), NULL);
    osMessageQDef(LCD_Gpio, LCD_MODE_QUEUE_SZ, uint8_t);
    LCD_GpioQueue = osMessageCreate(osMessageQ(LCD_Gpio), NULL);
    osThreadDef(LCD_Switch, LCD_Switch_Thread, priority, 1, ulStackSize);
    osThreadCreate(osThread(LCD_Switch), NULL);
	
	GPIO->GPIO_INTR_EN1 = 1;
//	GPIO->GPIO_INTR_EN6 = 1;
	return &LCD_SwitchModeQueue;
}

