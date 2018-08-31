/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OSD.c
	\brief		LCD OSD Funcations
	\author		Pierce
	\version	1.0
	\date		2018/07/25
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <INTC.h>
#include "OSD.h"
#include "SF_API.h"
#include "PROFILE_API.h"
#include "DMAC_API.h"
#include "BUF.h"
//------------------------------------------------------------------------------
#define OSD_MAJORVER    1        //!< Major version
#define OSD_MINORVER    0        //!< Minor version
//------------------------------------------------------------------------------
//const uint16_t pFontPat[OSD_FONT_PAT_NUM] = {OSD_FONTC_BLACK, OSD_FONTC_WHITE, OSD_FONTC_RED, OSD_FONTC_GREEN,
//											   OSD_FONTC_BLUE,  OSD_FONTC_YELLOW,OSD_FONTC_CYAN,OSD_FONTC_GRAY};
const uint8_t ubOSD_BatL[8] =
{
	OSD_BAT_L_0,
	OSD_BAT_L_1,
	OSD_BAT_L_2,
	OSD_BAT_L_3,
	OSD_BAT_L_4,
	OSD_BAT_L_4,
	OSD_BAT_L_4,
	OSD_BAT_L_4
};
const uint8_t ubOSD_BatR[8] =
{
	OSD_BAT_R_0,
	OSD_BAT_R_0,
	OSD_BAT_R_0,
	OSD_BAT_R_0,
	OSD_BAT_R_4,
	OSD_BAT_R_5,
	OSD_BAT_R_6,
	OSD_BAT_R_7
};
const uint8_t ubOSD_AntL[6] =
{
	OSD_ANT_L_0,
	OSD_ANT_L_1,
	OSD_ANT_L_2,
	OSD_ANT_L_2,
	OSD_ANT_L_2,
	OSD_ANT_L_2
};
const uint8_t ubOSD_AntR[6] =
{
	OSD_ANT_R_0,
	OSD_ANT_R_0,
	OSD_ANT_R_2,
	OSD_ANT_R_3,
	OSD_ANT_R_4,
	OSD_ANT_R_5
};
const uint8_t ubOSD_VdoModeL[4] = 
{
	OSD_VDOMODE_L_0,
	OSD_VDOMODE_L_1,
	OSD_VDOMODE_L_2,
	OSD_VDOMODE_L_3
};
const uint8_t ubOSD_VdoModeR[4] = 
{
	OSD_VDOMODE_R_0,
	OSD_VDOMODE_R_1,
	OSD_VDOMODE_R_2,
	OSD_VDOMODE_L_3
};
const uint8_t ubOSD_Rec[4] =
{
 	OSD_REC_0,
 	OSD_REC_1,
 	OSD_REC_2,
	OSD_REC_3
};
const uint8_t ubOSD_Cam[9] =
{
	OSD_CAM_0,
 	OSD_CAM_1,
 	OSD_CAM_2,
	OSD_CAM_3,
	OSD_CAM_4,
	OSD_CAM_5,
	OSD_CAM_6,
	OSD_CAM_7,
	OSD_CAM_8
};
const uint8_t ubOSD_Bar5A[5] =
{
	OSD_BAR5_A_0,				
	OSD_BAR5_A_1,				
	OSD_BAR5_A_1,				
	OSD_BAR5_A_1,				
	OSD_BAR5_A_1
};
const uint8_t ubOSD_Bar5B[5] =
{
	OSD_BAR5_B_0,				
	OSD_BAR5_B_0,				
	OSD_BAR5_B_2,				
	OSD_BAR5_B_2,				
	OSD_BAR5_B_2
};
const uint8_t ubOSD_Bar5C[5] =
{
	OSD_BAR5_B_0,				
	OSD_BAR5_B_0,				
	OSD_BAR5_B_0,				
	OSD_BAR5_B_2,				
	OSD_BAR5_B_2
};
const uint8_t ubOSD_Bar5D[5] =
{
	OSD_BAR5_D_0,				
	OSD_BAR5_D_0,				
	OSD_BAR5_D_0,				
	OSD_BAR5_D_0,				
	OSD_BAR5_D_4
};
const uint8_t ubOSD_Bar7A[7] = 
{
	OSD_BAR5_A_0,
	OSD_BAR5_A_1,
	OSD_BAR5_A_1,
	OSD_BAR5_A_1,
	OSD_BAR5_A_1,
	OSD_BAR5_A_1,
	OSD_BAR5_A_1,
};
const uint8_t ubOSD_Bar7B[7] = 
{
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
};
const uint8_t ubOSD_Bar7C[7] = 
{
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
};
const uint8_t ubOSD_Bar7D[7] = 
{
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
};
const uint8_t ubOSD_Bar7E[7] = 
{
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_0,
	OSD_BAR5_B_2,
	OSD_BAR5_B_2,
};
const uint8_t ubOSD_Bar7F[7] = 
{
	OSD_BAR5_D_0,
	OSD_BAR5_D_0,
	OSD_BAR5_D_0,
	OSD_BAR5_D_0,
	OSD_BAR5_D_0,
	OSD_BAR5_D_0,
	OSD_BAR5_D_4,
};
//------------------------------------------------------------------------------
static OSD_TOL_BUF_TYP 		 	 tOSD_BufInfor;
static OSD_SFADDR_INFOR_TYP 	 tOSD_SfAddr;											
static OSD_PAT_INFOR_TYP 		 tOSD_PatInfor;
static OSD_MENU_STRING_INFOR_TYP tOSD_MenuStringInfor;
static uint32_t			 		 ulOSD_BufSize;
static uint16_t					 uwOSD_HSize, uwOSD_VSize;
static uint8_t					 ubOSD_BufFlag;
static uint8_t 					 ubOSD_FontLangMax;
static osMutexId 				 OSD_Mutex;
//------------------------------------------------------------------------------
uint16_t uwOSD_GetVersion (void)
{
    return ((OSD_MAJORVER << 8) + OSD_MINORVER);
}
//------------------------------------------------------------------------------
void OSD_Weight (OSD_WEIGHT_TYP tWeight)
{
	LCD->LCD_OSD_WT = tWeight;
}
//------------------------------------------------------------------------------
void OSD_Enable (void)
{
	LCD->OSD_EN = 1;
}
//------------------------------------------------------------------------------
void OSD_Disable (void)
{
	LCD->OSD_EN = 0;
}
//------------------------------------------------------------------------------
uint16_t uwOSD_GetHSize (void)
{
	return uwOSD_HSize;
}
//------------------------------------------------------------------------------
uint16_t uwOSD_GetVSize (void)
{
	return uwOSD_VSize;
}
//------------------------------------------------------------------------------
void OSD_SetBufEmpty (void)
{
	tOSD_BufInfor.tBuf[0].tBufState = OSD_BUF_EMPTY;
	tOSD_BufInfor.tBuf[1].tBufState = OSD_BUF_EMPTY;
}
//------------------------------------------------------------------------------
void OSD_UpdatePattern (OSD_BUF_TYP *pOsdBuf)
{
	uint8_t ubi, ubPatStart, ubPatLen = 0;

	if (OSD_BUF_PAT_MASK & ubOSD_BufFlag)
	{
		ubPatStart = OSD_PAT_CR_NUM;
		ubPatLen = tOSD_PatInfor.ubOsdImgPatNum;
		ubOSD_BufFlag = (ubOSD_BufFlag & (~OSD_BUF_PAT_MASK)) | OSD_BUF_IMG_STATE;
	}
	if (ubPatLen)
	{
		uint16_t *pAddr;

		OSD_Disable();
		pAddr = (uint16_t*)pOsdBuf->ulBufAddr;
		//LCD->TV_LCD_EN = 0;
		LCD->LCD_PALETTE_A = ubPatStart;
		LCD->LCD_AUTO_INC_PALETTE_A = 1;
		for (ubi=0; ubi<ubPatLen; ++ubi)
			LCD->LCD_PALETTE_D = pAddr[ubPatStart + ubi];
		//LCD->TV_LCD_EN = 1;
	}
	OSD_Enable();
	pOsdBuf->tBufState = OSD_BUF_CUR;
}
//------------------------------------------------------------------------------
bool bOSD_UpdateAtVsync (void)
{
	switch(tOSD_BufInfor.tBuf[0].tBufState)
	{
		case OSD_BUF_CUR:
			if (FONT_BUF_FULL <= tOSD_BufInfor.tBuf[1].tBufState)
			{
				OSD_UpdatePattern(&tOSD_BufInfor.tBuf[1]);
				LCD->OSD_STR_A = (tOSD_BufInfor.tBuf[1].ulBufAddr + (OSD_PAT_NUM << 1)) >> 2;
				tOSD_BufInfor.tBuf[0].tBufState = OSD_BUF_EMPTY;
				return true;
			}
		case OSD_BUF_EMPTY:
			return false;
		default:
			OSD_UpdatePattern(&tOSD_BufInfor.tBuf[0]);
			LCD->OSD_STR_A = (tOSD_BufInfor.tBuf[0].ulBufAddr + (OSD_PAT_NUM << 1)) >> 2;			
			if (OSD_BUF_CUR == tOSD_BufInfor.tBuf[1].tBufState)
				tOSD_BufInfor.tBuf[1].tBufState = OSD_BUF_EMPTY;
			return true;
	}
}
//------------------------------------------------------------------------------
uint32_t ulOSD_GetCurrentBufAddr (void)
{
	return (OSD_BUF_CUR == tOSD_BufInfor.tBuf[0].tBufState)?tOSD_BufInfor.tBuf[0].ulBufAddr:tOSD_BufInfor.tBuf[1].ulBufAddr;
}
//------------------------------------------------------------------------------
OSD_BUF_TYP *pOSD_GetOsdBufInfor (void)
{
	return (OSD_BUF_CUR == tOSD_BufInfor.tBuf[0].tBufState)?&tOSD_BufInfor.tBuf[1]:&tOSD_BufInfor.tBuf[0];
}
//------------------------------------------------------------------------------
uint32_t ulOSD_CalBufSize (uint16_t uwHSize, uint16_t uwVSize)
{
	ulOSD_BufSize = uwHSize * uwVSize + (OSD_PAT_NUM << 1);
	if (3 & ulOSD_BufSize)
		ulOSD_BufSize = ((ulOSD_BufSize >> 2) + 1) << 2;
	return (ulOSD_BufSize << 2) + OSD_FONT_MAX_SIZE * OSD_FONT_MAX_SIZE + (OSD_MENU_STRING_MAX_NUM << 1);
}
//------------------------------------------------------------------------------
void OSD_ClearImg1Buf (void)
{
	osMutexWait(OSD_Mutex, osWaitForever);
	pOSD_GetOsdBufInfor()->tBufState = OSD_BUF_EMPTY;
	ubOSD_BufFlag = OSD_BUF_INIT;
	//memset((uint8_t*)tOSD_BufInfor.ulOsdImg1BufAddr, 0, ulOSD_BufSize);
	//memset((uint8_t*)tOSD_BufInfor.ulOsdImg2BufAddr, 0, ulOSD_BufSize);
	tDMAC_MemSet(0, tOSD_BufInfor.ulOsdImg1BufAddr, ulOSD_BufSize, NULL);	
	tDMAC_MemSet(0, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize, NULL);
	osMutexRelease(OSD_Mutex);
}
//------------------------------------------------------------------------------
void OSD_ClearImg2Buf (void)
{
	OSD_BUF_STA_TYP	 tBufState;
	OSD_BUF_TYP	*pOsdBuf;
	OSD_RESULT tOsdCpyRet = OSD_OK;

	osMutexWait(OSD_Mutex, osWaitForever);
	INTC_IrqDisable(INTC_LCD_IRQ);
	pOsdBuf = pOSD_GetOsdBufInfor();
	tBufState = pOsdBuf->tBufState;
	pOsdBuf->tBufState = OSD_BUF_EMPTY;
	INTC_IrqEnable(INTC_LCD_IRQ);
////	memcpy((uint8_t*)tOSD_BufInfor.ulOsdImg2BufAddr, (uint8_t*)tOSD_BufInfor.ulOsdImg1BufAddr, ulOSD_BufSize);
//	tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg1BufAddr, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize, NULL);
	tOsdCpyRet = tOSD_DataCopy(tOSD_BufInfor.ulOsdImg1BufAddr, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize);
	if(OSD_OK == tOsdCpyRet)
	{
		switch(tBufState)
		{
			case OSD_BUF_FULL:
			case OSD1_BUF_FULL:
////			memcpy((uint8_t*)pOsdBuf->ulBufAddr, (uint8_t*)tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize);
//				tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg2BufAddr, pOsdBuf->ulBufAddr, ulOSD_BufSize, NULL);
				tOsdCpyRet = tOSD_DataCopy(tOSD_BufInfor.ulOsdImg2BufAddr, pOsdBuf->ulBufAddr, ulOSD_BufSize);
				if(OSD_OK != tOsdCpyRet)
					break;
				pOsdBuf->tBufState = OSD1_BUF_FULL;
				ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
				break;
			default:
				ubOSD_BufFlag &= OSD_BUF_STATE_MASK;
				break;
		}
	}
	if(OSD_OK != tOsdCpyRet)
	{
		INTC_IrqDisable(INTC_LCD_IRQ);
		pOsdBuf->tBufState = tBufState;
		INTC_IrqEnable(INTC_LCD_IRQ);
	}
	osMutexRelease(OSD_Mutex);
}
//------------------------------------------------------------------------------
void OSD_SetOsdBufAddr (uint32_t ulAddr)
{
	tOSD_BufInfor.tBuf[0].tBufState = OSD_BUF_EMPTY;
	tOSD_BufInfor.tBuf[1].tBufState = OSD_BUF_EMPTY;
	tOSD_BufInfor.tBuf[0].ulBufAddr = ulAddr;
	tOSD_BufInfor.tBuf[1].ulBufAddr = ulAddr + ulOSD_BufSize;
	tOSD_BufInfor.ulOsdImg1BufAddr = tOSD_BufInfor.tBuf[1].ulBufAddr + ulOSD_BufSize;
	tOSD_BufInfor.ulOsdImg2BufAddr = tOSD_BufInfor.ulOsdImg1BufAddr + ulOSD_BufSize;
	tOSD_BufInfor.ulOsdFontBufAddr = tOSD_BufInfor.ulOsdImg2BufAddr + ulOSD_BufSize;
	tOSD_BufInfor.ulOsdMenuStringBufAddr = tOSD_BufInfor.ulOsdFontBufAddr + OSD_FONT_MAX_SIZE * OSD_FONT_MAX_SIZE; 
	pOSD_GetOsdBufInfor()->tBufState = OSD_BUF_EMPTY;
	ubOSD_BufFlag = OSD_BUF_INIT;
//	memset((uint8_t*)tOSD_BufInfor.ulOsdImg1BufAddr, 0, ulOSD_BufSize);
//	memset((uint8_t*)tOSD_BufInfor.ulOsdImg2BufAddr, 0, ulOSD_BufSize);
	tDMAC_MemSet(0, tOSD_BufInfor.ulOsdImg1BufAddr, ulOSD_BufSize, NULL);	
	tDMAC_MemSet(0, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize, NULL);
}
//------------------------------------------------------------------------------
void OSD_SfUpdateData (uint32_t ulSfAddr, uint32_t ulBufAddr, uint32_t ulLen)
{
	SF_DMA_Info_t tInfo;
	uint32_t	  ulLenRemaider;

	if (0x20000 < ulLen && (ulLenRemaider = ulLen & 0x7FF) > 0)
		ulLen -= ulLenRemaider;
	else
		ulLenRemaider = 0;
	
	tInfo.ulSFStaAddr 	  = ulSfAddr;
	tInfo.ulRAMStaAddr 	  = ulBufAddr;
	tInfo.ulLength 		  = ulLen;
	tInfo.tRAMType 		  = SF_DDR_DMA;
	tInfo.tDir 			  = SF_DMA_READ;
	tInfo.ubWaitRdy 	  = TRUE;
	tInfo.ubCrcOrder 	  = 16;
    tInfo.ulCrcPolynomial = 0xA001;
    ulSF_DMA(&tInfo, NULL);
	if(ulLenRemaider)
	{
		tInfo.ulSFStaAddr 	  = ulSfAddr + ulLen;
		tInfo.ulRAMStaAddr 	  = ulBufAddr + ulLen;
		tInfo.ulLength 		  = ulLenRemaider;
		tInfo.tRAMType 		  = SF_DDR_DMA;
		tInfo.tDir 			  = SF_DMA_READ;
		tInfo.ubWaitRdy 	  = TRUE;
		tInfo.ubCrcOrder 	  = 16;
		tInfo.ulCrcPolynomial = 0xA001;
		ulSF_DMA(&tInfo, NULL);
	}
}
//------------------------------------------------------------------------------
void OSD_LogoJpeg (uint32_t ulLogo_Index)
{
	uint32_t 	   ulLogoSFAddr;
	uint32_t	   ulQTableSFAddr;
	uint32_t 	   ulImgMaxSize;
	uint32_t	   ulImgAddr;
	uint32_t	   ulImgSize;
	LCD_BUF_TYP   *pLcdCh0Buf;
	LCD_INFOR_TYP  sLcdInfor;
	uint16_t 		uwLogoH,uwLogoW;

	osMutexWait(OSD_Mutex, osWaitForever);
	if(LCD_JPEG_DISABLE == tLCD_GetJpegDecoderStatus())
	{
		LCD_ChDisable(LCD_CH0);
		LCD_ChDisable(LCD_CH1);
		LCD_ChDisable(LCD_CH2);
		LCD_ChDisable(LCD_CH3);
		tLCD_JpegDecodeDisable();
		LCD_SetOsdLogoJpegBufAddr();
	}
	pLcdCh0Buf   = pLCD_GetLcdChBufInfor(LCD_CH0);
	ulLogoSFAddr = *((uint32_t *)(pbPROF_GetParam(SYSPARAM) + LOGO_ADDR));
	SF_Read(ulLogoSFAddr + OSD_LOGO_MAXSIZE_SFT, OSD_LOGO_JPEG_MAXSIZE_LEN, (uint8_t*)&ulImgMaxSize);
	if(!ulImgMaxSize)
	{
		printf("Logo Max Size error !\n");
		osMutexRelease(OSD_Mutex);
		return;
	}
	ulImgMaxSize *= 1024;
	if(LCD_JPEG_DISABLE == tLCD_GetJpegDecoderStatus())
	{
		ulQTableSFAddr = ulLogoSFAddr + ((((ulLogo_Index + 1) * ulImgMaxSize) - 0xA0) + OSD_LOGO_HEADER_LEN);
		OSD_SfUpdateData(ulQTableSFAddr, pLcdCh0Buf->ulBufAddr, OSD_LOGO_JPEG_Q0_LEN);
		ulQTableSFAddr += OSD_LOGO_JPEG_Q0_LEN;
		OSD_SfUpdateData(ulQTableSFAddr, pLcdCh0Buf->ulBufAddr + OSD_LOGO_JPEG_Q0_LEN, OSD_LOGO_JPEG_Q1_LEN);
		LCD_JpegRwQTab(LCD_JPEG_QW, OSD_LOGO_JPEG_QADDR, OSD_LOGO_JPEG_QLEN, (uint8_t*)pLcdCh0Buf->ulBufAddr);

		SF_Read(ulQTableSFAddr+OSD_LOGO_JPEG_Q1_LEN+5, 2, (uint8_t*)&uwLogoW);
		SF_Read(ulQTableSFAddr+OSD_LOGO_JPEG_Q1_LEN+7, 2, (uint8_t*)&uwLogoH);
		uwLogoW = SWAP16(uwLogoW);
		uwLogoH = SWAP16(uwLogoH);

		sLcdInfor.tDispType = LCD_DISP_1T;
		sLcdInfor.ubChNum = 1;	
		sLcdInfor.tChRes[0].uwCropHstart = 0;
		sLcdInfor.tChRes[0].uwCropVstart = 0;
		sLcdInfor.uwLcdOutputHsize = uwLCD_GetLcdHoSize();
		sLcdInfor.uwLcdOutputVsize = uwLCD_GetLcdVoSize();
		sLcdInfor.tChRes[0].uwChInputHsize = uwLogoW;
		sLcdInfor.tChRes[0].uwChInputVsize = uwLogoH;
		sLcdInfor.tChRes[0].uwCropHsize = sLcdInfor.tChRes[0].uwChInputHsize;
		sLcdInfor.tChRes[0].uwCropVsize = sLcdInfor.tChRes[0].uwChInputVsize;
		tLCD_CropScale(&sLcdInfor);
	}
	ulImgAddr = ulLogoSFAddr + (ulLogo_Index * ulImgMaxSize) + OSD_LOGO_HEADER_LEN;
	SF_Read(ulLogoSFAddr + ((ulLogo_Index * 4) + OSD_LOGO_BSLEN_SFT), OSD_LOGO_JPEG_FILESIZE_LEN, (uint8_t*)&ulImgSize);
	printd(DBG_Debug3Lvl, "Image Size 0x%X[(0x%X)(0x%X)]\n", ulLogoSFAddr, ulImgAddr, ulImgSize);
	if((ulLogoSFAddr >= pSF_Info->ulSize) || (ulImgAddr >= pSF_Info->ulSize) || (ulImgSize >= pSF_Info->ulSize))
	{
		osMutexRelease(OSD_Mutex);
		return;
	}
	OSD_SfUpdateData(ulImgAddr, pLcdCh0Buf->ulBufAddr, ulImgSize);
	pLcdCh0Buf->tBufState = LCD_BUF_FULL;
	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
	{
		LCD_SetChBufReady(pLcdCh0Buf);
		osMutexRelease(OSD_Mutex);
		return;
	}
	tLCD_JpegDecodeEnable(LCD_CH0);
	LCD_Start();
	osMutexRelease(OSD_Mutex);
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_Init (OSD_WEIGHT_TYP tOsdWeight, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwXStart, uint16_t uwYStart, OSD_SCALE_TYP tXScale, OSD_SCALE_TYP tYScale)
{
	osMutexDef(OSD_Mutex);
	OSD_Mutex = osMutexCreate(osMutex(OSD_Mutex));
	if (!(ulHSize & 3))
	{
		uint32_t ulXEnd, ulYEnd;
		
		ulXEnd = uwXStart + ulHSize * (tXScale + 1);
		ulYEnd = uwYStart + ulVSize * (tYScale + 1);
		if (uwLCD_GetLcdHoSize() >= ulXEnd && uwLCD_GetLcdVoSize() >= ulYEnd)
		{
			uint16_t *pFontPat;
			uint8_t   ubi;

			uwOSD_HSize = ulHSize;
			uwOSD_VSize = ulVSize;
			LCD->LCD_OSD_WT = tOsdWeight;
			LCD->OSD_LINE_SIZE = ulHSize >> 2;
			LCD->LCD_OSD_H_SIZE = tXScale;	
			LCD->LCD_OSD_V_SIZE = tYScale;
			LCD->OSD_X_START = uwXStart;
			LCD->OSD_X_END = ulXEnd;
			LCD->OSD_Y_START = uwYStart;
			LCD->OSD_Y_END = ulYEnd;

			//!
			tOSD_SfAddr.ulOsdMenuStringSfStartAddr = *((uint32_t *)(pbPROF_GetParam(OSDPARAM)+OSD_MENU_ADDR));	//OSD_MEMU_STRING_ADDR;
			tOSD_SfAddr.ulOsdImgSfStartAddr = *((uint32_t *)(pbPROF_GetParam(OSDPARAM)+OSD_IMAGE_ADDR));			//OSD_IMG_ADDR;
			tOSD_SfAddr.ulOsdFontSfStartAddr = *((uint32_t *)(pbPROF_GetParam(OSDPARAM)+OSD_FONT_ADDR));		//OSD_FONT_ADDR;
			printd(DBG_Debug3Lvl, "OSD Start Addr: 0x%X\n", tOSD_SfAddr.ulOsdImgSfStartAddr);
			tOSD_PatInfor.ulOsdPatSft = OSD_GROUP_TOP_LEN + OSD_PAT_SFT;
			//tOSD_PatInfor.ubOsdImgPatNum = OSD_IMG_PAT_NUM + OSD_PAT_BG_NUM;
			SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + OSD_OSD1_PAT_NUM_SFT, 1, &tOSD_PatInfor.ubOsdImgPatNum);			
			tOSD_PatInfor.ubOsdImgPatNum += OSD_PAT_BG_NUM;
			SF_Read(tOSD_SfAddr.ulOsdMenuStringSfStartAddr + OSD_MENU_HEADER_LANG_SFT, 3, (uint8_t*)&tOSD_MenuStringInfor);			
			++tOSD_MenuStringInfor.ubItemNum; 
			printf("%s",(OSD_MENU_STRING_MAX_NUM < tOSD_MenuStringInfor.ubWordNum)?"[OSD] OSD Menu String Word Number Overflow\n":"");
			//tOSD_PatInfor.ubOsdFontPatNum = OSD_FONT_PAT_NUM;
			SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + OSD_FONT_PAT_NUM_SFT, 1,	&tOSD_PatInfor.ubOsdFontPatNum);			
			SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + OSD_FONT_LANG_MAX_SFT, 1,	&ubOSD_FontLangMax);			
			pFontPat = (uint16_t*) malloc (sizeof(uint16_t) * tOSD_PatInfor.ubOsdFontPatNum);
			SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + sizeof(uint16_t) * OSD_PAT_CR_NUM, sizeof(uint16_t) * tOSD_PatInfor.ubOsdFontPatNum,
				(uint8_t*)pFontPat);			
			OSD_Disable();
			LCD->LCD_PALETTE_A = OSD_PAT_NUM - tOSD_PatInfor.ubOsdFontPatNum;
			LCD->LCD_AUTO_INC_PALETTE_A = 1;			
			for (ubi=0; ubi<tOSD_PatInfor.ubOsdFontPatNum; ++ubi)
				LCD->LCD_PALETTE_D = pFontPat[ubi];
			free(pFontPat);
			return OSD_OK;
		}
		printf("[OSD] OSD Location error\n");
		return OSD_LOCA_FAIL;
	}
	printf ("[OSD] OSD line size error\n");
	return OSD_HSIZE_FAIL;
}
//------------------------------------------------------------------------------
void OSD_UpdateImg1 (void)
{
	OSD_BUF_TYP	*pOsdBuf;
	OSD_BUF_STA_TYP	 tBufState;
	OSD_RESULT tOsdCpyRet = OSD_OK;
	
	INTC_IrqDisable(INTC_LCD_IRQ);
	pOsdBuf = pOSD_GetOsdBufInfor();
	tBufState = pOsdBuf->tBufState;
	pOsdBuf->tBufState = OSD_BUF_EMPTY;
	INTC_IrqEnable(INTC_LCD_IRQ);
////	memcpy((uint8_t *)pOsdBuf->ulBufAddr, (uint8_t *)tOSD_BufInfor.ulOsdImg1BufAddr, ulOSD_BufSize);
//	tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg1BufAddr, pOsdBuf->ulBufAddr, ulOSD_BufSize, NULL);
	tOsdCpyRet = tOSD_DataCopy(tOSD_BufInfor.ulOsdImg1BufAddr, pOsdBuf->ulBufAddr, ulOSD_BufSize);
	if(OSD_OK != tOsdCpyRet)
	{
		INTC_IrqDisable(INTC_LCD_IRQ);
		pOsdBuf->tBufState = tBufState;
		INTC_IrqEnable(INTC_LCD_IRQ);
		return;
	}
	pOsdBuf->tBufState = OSD1_BUF_FULL;
	ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
}
//------------------------------------------------------------------------------
void OSD_EraserImg1 (OSD_IMG_INFO *pInfor)
{
	OSD_RESULT tOsdCpyRet = OSD_OK;

	osMutexWait(OSD_Mutex, osWaitForever);
	if (ubOSD_BufFlag & OSD_BUF1_PATIMG_MASK)
	{
		if (uwOSD_HSize < pInfor->uwXStart + pInfor->uwHSize)
			pInfor->uwHSize = uwOSD_HSize - pInfor->uwXStart;
		if (uwOSD_VSize < pInfor->uwYStart + pInfor->uwVSize)
			pInfor->uwVSize = uwOSD_VSize - pInfor->uwYStart;
		if (uwOSD_HSize == pInfor->uwHSize)
	//		memset((uint8_t *)(tOSD_BufInfor.ulOsdImg1BufAddr + (OSD_PAT_NUM << 1) + pInfor->uwYStart * pInfor->uwHSize),
	//		0, pInfor->uwHSize * pInfor->uwVSize);
			tDMAC_MemSet(0, tOSD_BufInfor.ulOsdImg1BufAddr + (OSD_PAT_NUM << 1) + pInfor->uwYStart * pInfor->uwHSize,
			pInfor->uwHSize * pInfor->uwVSize, NULL);
		else
		{
			uint32_t ulAddrSft;
			uint16_t uwi;
			
			for (uwi=0; uwi<pInfor->uwVSize; ++uwi)
			{
				ulAddrSft = pInfor->uwXStart + (uwi + pInfor->uwYStart) * uwOSD_HSize + (OSD_PAT_NUM << 1);
				memset((uint8_t *)(tOSD_BufInfor.ulOsdImg1BufAddr + ulAddrSft), 0, pInfor->uwHSize);
			}
		}
		//memcpy((uint8_t*)tOSD_BufInfor.ulOsdImg2BufAddr, (uint8_t*)tOSD_BufInfor.ulOsdImg1BufAddr, ulOSD_BufSize);
		//tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg1BufAddr, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize, NULL);
		tOsdCpyRet = tOSD_DataCopy(tOSD_BufInfor.ulOsdImg1BufAddr, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize);
		if(OSD_OK == tOsdCpyRet)
			OSD_UpdateImg1();
	}
	osMutexRelease(OSD_Mutex);
}
//------------------------------------------------------------------------------
void OSD_UpdateImg2 (void)
{
	OSD_BUF_TYP		*pOsdBuf;
	OSD_BUF_STA_TYP	 tBufState;
	OSD_RESULT tOsdCpyRet = OSD_OK;
	
	INTC_IrqDisable(INTC_LCD_IRQ);
	pOsdBuf = pOSD_GetOsdBufInfor();
	tBufState = pOsdBuf->tBufState;
	pOsdBuf->tBufState = OSD_BUF_EMPTY;
	INTC_IrqEnable(INTC_LCD_IRQ);
//	tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg2BufAddr, pOsdBuf->ulBufAddr, ulOSD_BufSize, NULL);
	tOsdCpyRet = tOSD_DataCopy(tOSD_BufInfor.ulOsdImg2BufAddr, pOsdBuf->ulBufAddr, ulOSD_BufSize);
	if(OSD_OK == tOsdCpyRet)
	{
		switch(tBufState)
		{
			case OSD_BUF_EMPTY:
			case FONT_BUF_FULL:
			case OSD2_BUF_FULL:
				if (!(OSD_IMG1_Q & ubOSD_BufFlag))
				{
					pOsdBuf->tBufState = OSD2_BUF_FULL; 
					break;
				}
			case OSD_BUF_FULL:
			case OSD1_BUF_FULL:
				pOsdBuf->tBufState = OSD_BUF_FULL; 
				break;
			default:
				break;
		}
		ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
	}
	else
	{
		INTC_IrqDisable(INTC_LCD_IRQ);
		pOsdBuf->tBufState = tBufState;
		INTC_IrqEnable(INTC_LCD_IRQ);
	}
}
//------------------------------------------------------------------------------
void OSD_EraserImg2 (OSD_IMG_INFO *pInfor)
{
	osMutexWait(OSD_Mutex, osWaitForever);
	if (ubOSD_BufFlag & OSD_BUF_STATE_MASK)
	{
		if (uwOSD_HSize < pInfor->uwXStart + pInfor->uwHSize)	
			pInfor->uwHSize = uwOSD_HSize - pInfor->uwXStart;
		if (uwOSD_VSize < pInfor->uwYStart + pInfor->uwVSize)
			pInfor->uwVSize = uwOSD_VSize - pInfor->uwYStart;	
		if (uwOSD_HSize == pInfor->uwHSize)
		{
			uint32_t ulSft = (OSD_PAT_NUM << 1) + pInfor->uwYStart * pInfor->uwHSize;
			
//			memcpy((uint8_t *)(tOSD_BufInfor.ulOsdImg2BufAddr + ulSft), 
//			(uint8_t *)(tOSD_BufInfor.ulOsdImg1BufAddr + ulSft),	pInfor->uwHSize * pInfor->uwVSize);
			tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg1BufAddr + ulSft, tOSD_BufInfor.ulOsdImg2BufAddr + ulSft,
			pInfor->uwHSize * pInfor->uwVSize, NULL);
		}
		else
		{
			uint32_t ulAddrSft;
			uint16_t uwi;
			
			for (uwi=0; uwi<pInfor->uwVSize; ++uwi)
			{
				ulAddrSft = pInfor->uwXStart + (uwi + pInfor->uwYStart) * uwOSD_HSize + (OSD_PAT_NUM << 1);
				memcpy((uint8_t *)(tOSD_BufInfor.ulOsdImg2BufAddr + ulAddrSft),
				(uint8_t *)(tOSD_BufInfor.ulOsdImg1BufAddr + ulAddrSft), pInfor->uwHSize);
			}
		}
		OSD_UpdateImg2();
	}
	osMutexRelease(OSD_Mutex);
}
//------------------------------------------------------------------------------
void OSD_UpdateFont (OSD_BUF_STA_TYP tBufState)
{
	OSD_BUF_TYP	*pOsdBuf;
	
	pOsdBuf = pOSD_GetOsdBufInfor();
	if (OSD_BUF_FULL == tBufState || (OSD_IMG1_Q | OSD_IMG2_Q) == ((OSD_IMG1_Q | OSD_IMG2_Q) & ubOSD_BufFlag))
		pOsdBuf->tBufState = OSD_BUF_FULL;
	else if (OSD1_BUF_FULL == tBufState || OSD_IMG1_Q & ubOSD_BufFlag)
		pOsdBuf->tBufState = OSD1_BUF_FULL;
	else if (OSD2_BUF_FULL == tBufState || OSD_IMG2_Q & ubOSD_BufFlag)
		pOsdBuf->tBufState = OSD2_BUF_FULL;
	else
		pOsdBuf->tBufState = FONT_BUF_FULL;
	ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
}
//------------------------------------------------------------------------------
OSD_BUF_STA_TYP tOSD_EraserBufFont (uint16_t uwHSize, uint16_t uwVSize, uint16_t uwXStart, uint16_t uwYStart)
{
	OSD_BUF_TYP		*pOsdBuf;
	OSD_BUF_STA_TYP	 tBufState;
	
	INTC_IrqDisable(INTC_LCD_IRQ);
	pOsdBuf = pOSD_GetOsdBufInfor();
	tBufState = pOsdBuf->tBufState;
	pOsdBuf->tBufState = OSD_BUF_EMPTY;
	INTC_IrqEnable(INTC_LCD_IRQ);
	if (OSD_BUF_UPDATE & ubOSD_BufFlag || OSD_FONT_Q & ubOSD_BufFlag)
	{
		uint32_t ulAddrSft;
		uint16_t uwi;
		
		if (OSD_BUF_UPDATE & ubOSD_BufFlag && OSD_BUF_EMPTY == tBufState)
//			memcpy((uint8_t *)pOsdBuf->ulBufAddr, (uint8_t *)ulOSD_GetCurrentBufAddr(), ulOSD_BufSize);
			tDMAC_MemCopy (ulOSD_GetCurrentBufAddr(), pOsdBuf->ulBufAddr,	ulOSD_BufSize, NULL);
		for (uwi=0; uwi<uwVSize; ++uwi)
		{
			ulAddrSft = uwXStart + (uwi + uwYStart) * uwOSD_HSize + (OSD_PAT_NUM << 1);
			memcpy((uint8_t *)(pOsdBuf->ulBufAddr + ulAddrSft), (uint8_t *)(tOSD_BufInfor.ulOsdImg2BufAddr + ulAddrSft), uwHSize);						
		}
	}
	else
//		memcpy((uint8_t *)pOsdBuf->ulBufAddr, (uint8_t *)tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize);
		tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg2BufAddr, pOsdBuf->ulBufAddr,	ulOSD_BufSize, NULL);
	return tBufState;
}
//------------------------------------------------------------------------------
OSD_FONT_SWITCH_OUT_TYP *pOSD_FontSwitchLangColorRotation (uint8_t ubLang, OSD_FONT_COLOR_TYP tColor, OSD_FONT_RA_TYP tRotation)
{
	if (ubLang && ubOSD_FontLangMax >= ubLang)
	{
		static OSD_FONT_SWITCH_OUT_TYP tFontInfor;
		static uint8_t  ubLangIndexLast = 0xFF, ubRoLast;
		static OSD_FONT_COLOR_TYP tColorIndexLast;	
		
		if (ubLangIndexLast != ubLang || tColorIndexLast != tColor || ubRoLast != tRotation)
		{
			uint32_t ulFontSftNow;		
			uint8_t  ubi, ubRoNow, ubRoOk;
			
			//! Switch Language
			for (ulFontSftNow=OSD_FONT_PT_SFT,ubi=ubLang; ubi>1; --ubi)
				SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_NEXT_LANG_OFFSET_SFT, 4,	(uint8_t*)&ulFontSftNow);
			//! Switch Color
			for (ubi=0; ubi<tColor; ++ubi)
				SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_NEXT_COLOR_OFFSET_SFT, 4,	(uint8_t*)&ulFontSftNow);
			//! Support Rotation?
			SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow, 1, &ubRoNow);
			ubRoOk = ubRoNow >> 4;
			if (ubRoOk & (1 << tRotation))
			{
				uint16_t uwTempH, uwTempV=0;
				
				//! Switch Rotation
				for (ubRoNow&=0x7, ubi=1; ubRoNow!=tRotation; ++ubi)
				{
					if (4 == ubi)	return NULL;
					SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_NEXT_ROTATION_OFFSET_SFT, 4, (uint8_t*)&ulFontSftNow);
					SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow, 1, &ubRoNow);
					ubRoNow &= 0x7;			
				}
				SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_H_SFT, 2,	(uint8_t*)&uwTempH);
				SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_V_SFT, 1,	(uint8_t*)&uwTempV);
				if (OSD_FONT_MAX_SIZE * OSD_FONT_MAX_SIZE >= uwTempH * uwTempV)
				{
					if (1 & ubRoNow)
					{
						tFontInfor.uwFontH = uwTempV;
						tFontInfor.uwFontV = uwTempH;
					}
					else
					{
						tFontInfor.uwFontH = uwTempH;
						tFontInfor.uwFontV = uwTempV;
					}
					SF_Read(tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_NUM_MAX_SFT, 2,	(uint8_t*)&tFontInfor.uwFontMax);
					tFontInfor.ulFontAddr = tOSD_SfAddr.ulOsdFontSfStartAddr + ulFontSftNow + OSD_FONT_HEADER_LEN;
					ubLangIndexLast = ubLang;
					tColorIndexLast = tColor;
					ubRoLast = tRotation;	
					return &tFontInfor;
				}
			}			
		}
		else
			return &tFontInfor;
	}
	return NULL;
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_EraserFont (OSD_FONT_INFOR_TYP *pInfor, uint16_t uwHFontNum, uint16_t uwVFontNum)
{
	OSD_FONT_SWITCH_OUT_TYP *pFontInfor;
	OSD_RESULT				 tFlag = OSD_FONT_SWITCH_FAIL;
	
	osMutexWait(OSD_Mutex, osWaitForever);
	pFontInfor = pOSD_FontSwitchLangColorRotation(pInfor->ubLangIndex, pInfor->tColorIndex, pInfor->tRotation);
	if (NULL != pFontInfor)
	{
		uint16_t 	 	 uwHSize, uwVSize;	
		OSD_BUF_STA_TYP	 tBufState;	
		
		uwHSize = (uwOSD_HSize < pInfor->uwXStart + (uwHSize = uwHFontNum * pFontInfor->uwFontH))?uwOSD_HSize - pInfor->uwXStart:uwHSize;
		uwVSize = (uwOSD_VSize < pInfor->uwYStart + (uwVSize = uwVFontNum * pFontInfor->uwFontV))?uwOSD_VSize - pInfor->uwYStart:uwVSize;		
		tBufState = tOSD_EraserBufFont (uwHSize, uwVSize, pInfor->uwXStart, pInfor->uwYStart);
		OSD_UpdateFont (tBufState);
		tFlag = OSD_OK;
	}
	else
		printf("[OSD] OSD Switch error\n");
	osMutexRelease(OSD_Mutex);
	return tFlag;
}
//------------------------------------------------------------------------------
void OSD_GetImgData (uint32_t ulSfAddr, uint32_t ulDataSft, uint32_t ulMemAddr,	uint16_t uwHSize, uint16_t uwVSize, uint16_t uwXStart, uint16_t uwYStart)
{
	if (uwOSD_HSize == uwHSize)
		OSD_SfUpdateData(ulSfAddr + ulDataSft, ulMemAddr + (OSD_PAT_NUM << 1) + uwYStart * uwHSize, uwHSize * uwVSize);
	else
	{
		uint32_t ulSfAddrSft, ulMemAddrSft;
		uint16_t uwLineSize;
		uint16_t uwi;
		uint8_t	 ubRemaider;
		
		for (uwi=0, ulSfAddrSft = ulDataSft; uwi<uwVSize; ++uwi)
		{
			ulMemAddrSft = (uwYStart + uwi) * uwOSD_HSize + uwXStart;
			//! For Test
			//ulSfAddrSft = ulDataSft + uwi * uwOSD_HSize;
			if ((ubRemaider = (uint8_t)(3 & ulMemAddrSft)) > 0)
			{
				ubRemaider = 4 - ubRemaider;
				SF_Read(ulSfAddr + ulSfAddrSft, ubRemaider, (uint8_t*)(ulMemAddr + (OSD_PAT_NUM << 1) + ulMemAddrSft));
				ulMemAddrSft += ubRemaider;
				ulSfAddrSft += ubRemaider;
				uwLineSize = uwHSize - ubRemaider;
			}
			else
				uwLineSize = uwHSize;
//			if ((ubRemaider = (uint8_t)(3 & uwLineSize)) > 0)
//				uwLineSize &= 0xFFFC;
			OSD_SfUpdateData(ulSfAddr + ulSfAddrSft, ulMemAddr + (OSD_PAT_NUM << 1) + ulMemAddrSft, uwLineSize);
			ulSfAddrSft += uwLineSize;
//			if (ubRemaider)
//			{
//				ulMemAddrSft += uwLineSize;
//				SF_Read(ulSfAddr + ulSfAddrSft, ubRemaider, (uint8_t*)(ulMemAddr + (OSD_PAT_LEN << 1) + ulMemAddrSft));
//				ulSfAddrSft += ubRemaider;
//			}
		}
	}
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_Img1 (OSD_IMG_INFO *pInfor, OSD_UPDATE_TYP tMode)
{
	OSD_RESULT tFlag = OSD_LOCA_FAIL, tOsdCpyRet = OSD_OK;

	osMutexWait(OSD_Mutex, osWaitForever);
	if (uwOSD_HSize >= pInfor->uwXStart + pInfor->uwHSize && uwOSD_VSize >= pInfor->uwYStart + pInfor->uwVSize)
	{
		if (!(OSD_BUF1_PATIMG_MASK & ubOSD_BufFlag))
		{
			ubOSD_BufFlag |= OSD_BUF1_PAT_STATE;
			OSD_SfUpdateData(tOSD_SfAddr.ulOsdImgSfStartAddr + tOSD_PatInfor.ulOsdPatSft, 
			tOSD_BufInfor.ulOsdImg1BufAddr, (tOSD_PatInfor.ubOsdImgPatNum + OSD_PAT_CR_NUM) << 1);
		}
		OSD_GetImgData (tOSD_SfAddr.ulOsdImgSfStartAddr, pInfor->ulAddrSft, tOSD_BufInfor.ulOsdImg1BufAddr, 
		pInfor->uwHSize, pInfor->uwVSize, pInfor->uwXStart, pInfor->uwYStart);
////		memcpy((uint8_t*)tOSD_BufInfor.ulOsdImg2BufAddr, (uint8_t*)tOSD_BufInfor.ulOsdImg1BufAddr, ulOSD_BufSize);
//		tDMAC_MemCopy (tOSD_BufInfor.ulOsdImg1BufAddr, tOSD_BufInfor.ulOsdImg2BufAddr,	ulOSD_BufSize, NULL);
		tOsdCpyRet = tOSD_DataCopy(tOSD_BufInfor.ulOsdImg1BufAddr, tOSD_BufInfor.ulOsdImg2BufAddr, ulOSD_BufSize);
		if(OSD_OK == tOsdCpyRet)
		{
			if (OSD_UPDATE == tMode)
			{
				uint8_t	ubDelayCnt = 5;
				
				while(OSD_BUF_EMPTY != pOSD_GetOsdBufInfor()->tBufState)
				{
					osDelay(1);
					if (--ubDelayCnt)	break;
				}
				OSD_UpdateImg1();
			}
			else
				ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_IMG1_Q;
			tFlag = OSD_OK;
		}
	}
	else
		printf("[OSD] OSD Location error\n");
	osMutexRelease(OSD_Mutex);
	return tFlag;
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_Img2 (OSD_IMG_INFO *pInfor, OSD_UPDATE_TYP tMode)
{
	OSD_RESULT tFlag = OSD_LOCA_FAIL;
	
	osMutexWait(OSD_Mutex, osWaitForever);
	if (uwOSD_HSize >= pInfor->uwXStart + pInfor->uwHSize && uwOSD_VSize >= pInfor->uwYStart + pInfor->uwVSize)
	{	
		if (!(OSD_BUF_STATE_MASK & ubOSD_BufFlag))
		{
			ubOSD_BufFlag |= OSD_BUF2_PAT_STATE;
			OSD_SfUpdateData(tOSD_SfAddr.ulOsdImgSfStartAddr + tOSD_PatInfor.ulOsdPatSft, 
			tOSD_BufInfor.ulOsdImg2BufAddr, (tOSD_PatInfor.ubOsdImgPatNum + OSD_PAT_CR_NUM) << 1);
		}
		OSD_GetImgData (tOSD_SfAddr.ulOsdImgSfStartAddr, pInfor->ulAddrSft, tOSD_BufInfor.ulOsdImg2BufAddr,
		pInfor->uwHSize, pInfor->uwVSize, pInfor->uwXStart, pInfor->uwYStart);
		if (OSD_UPDATE == tMode)
		{
			uint8_t	ubDelayCnt = 5;
		
			while(OSD_BUF_EMPTY == pOSD_GetOsdBufInfor()->tBufState)
			{
				osDelay(1);
				if (--ubDelayCnt)	break;
			}
			OSD_UpdateImg2();
		}
		else
			ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | ((OSD_IMG1_Q & ubOSD_BufFlag)?OSD_IMG1_Q|OSD_IMG2_Q:OSD_IMG2_Q);
		tFlag = OSD_OK;
	}
	else
		printf("[OSD] OSD Location error\n");
	osMutexRelease(OSD_Mutex);
	return tFlag;
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_GetOsdImgInfor (uint8_t ubGroupNum, OSD_LAYER_TYP tOsdLayer, uint16_t uwStartImageIdx, uint16_t uwTotalImage, void *pbOsdImgData)
{
	OSD_RESULT tFlag = OSD_OK;
	uint8_t    ubGroupCnt;

	osMutexWait(OSD_Mutex, osWaitForever);
	SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + OSD_GROUP_COUNT_SFT, 1, &ubGroupCnt);	
	if (ubGroupNum <= ubGroupCnt)
	{
		uint32_t ulGroupSft = OSD_GROUP_TOP_LEN;
		uint16_t uwi, uwOsd1ImgNum, uwOsd2ImgNum;

		for (uwi=1; uwi<ubGroupNum; ++uwi)
		{
			SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + ulGroupSft + OSD_NEXT_GROUP_ADDR_SFT, 4, (uint8_t*)&ulGroupSft);
			ulGroupSft -= (uwi * OSD_GROUP_TOP_LEN);
		}
		tOSD_PatInfor.ulOsdPatSft = OSD_PAT_SFT + ulGroupSft;
		SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + ulGroupSft + OSD_OSD1_IMAGE_NUM_SFT, 2, (uint8_t *)&uwOsd1ImgNum);
		SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + ulGroupSft + OSD_OSD2_IMAGE_NUM_SFT, 2, (uint8_t *)&uwOsd2ImgNum);
		if (tOsdLayer == OSD_IMG1)
		{
			if ((!uwStartImageIdx) || ((uwStartImageIdx+uwTotalImage-1) > uwOsd1ImgNum))
			{
				tFlag = OSD_LOADIMAGE_FAIL;
				printf("[%s] OSD1 image Information fail\n", __MODULE__);
			}
			else
				SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + ulGroupSft + OSD_GROUP_HEADER_SFT + ((uwStartImageIdx - 1) * sizeof(OSD_IMG_INFO)),
				uwTotalImage*sizeof(OSD_IMG_INFO), (uint8_t *)pbOsdImgData);
		}
		else
		{
			if ((!uwStartImageIdx) || ((uwStartImageIdx+uwTotalImage-1) > uwOsd2ImgNum))
			{
				tFlag = OSD_LOADIMAGE_FAIL;
				printf("[%s] OSD2 image Information fail\n", __MODULE__);
			}
			else
				SF_Read(tOSD_SfAddr.ulOsdImgSfStartAddr + ulGroupSft + OSD_GROUP_HEADER_SFT + ((uwOsd1ImgNum + uwStartImageIdx - 1) * sizeof(OSD_IMG_INFO)),
				uwTotalImage*sizeof(OSD_IMG_INFO), (uint8_t *)pbOsdImgData);
		}
	}
	osMutexRelease(OSD_Mutex);
	return tFlag;
}
//------------------------------------------------------------------------------
uint8_t OSD_ImgString2Index(char c, uint16_t *pOsdImg_Idx)
{
#define isNumber(c, idx) 		if(c >= '0' && c <= '9') { idx = c - '0'; return 1; }
#define isUpperLetter(c, idx)	if(c >= 'A' && c <= 'Z') { idx = c - 'A'; return 2; }
#define isLowerLetter(c, idx)	if(c >= 'a' && c <= 'z') { idx = c - 'a'; return 3; }
#define isSymbol(c, idx)		if((c == ':') || (c == '-') || (c == '.') || (c == ' ')) { idx = (c == ':')?0:(c == '-')?1:(c == '.')?2:3; return 4; }
	isUpperLetter(c, *pOsdImg_Idx);
	isLowerLetter(c, *pOsdImg_Idx);
	isNumber(c, *pOsdImg_Idx);
	isSymbol(c, *pOsdImg_Idx);

	return 0;
}
//------------------------------------------------------------------------------
void OSD_ImagePrintf(OSD_IMG_RA_TYP tRotType, uint16_t uwXStart, uint16_t uwYStart, OSD_IMGIDXARRARY_t tImgIdxArray, OSD_UPDATE_TYP tMode, char* pFmt, ...)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwOsd_ImgIdx = 0, uwPrintfLen = 0, uwi;
	uint16_t *pOsdType[5] = {NULL, tImgIdxArray.pNumImgIdxArray, tImgIdxArray.pUpperLetterImgIdxArray, 
						           tImgIdxArray.pLowerLetterImgIdxArray, tImgIdxArray.pSymbolImgIdxArray};
	uint16_t uwTmpXStart, uwTmpYStart;
	uint8_t  ubPrintfString[64], ubType = 0;
	va_list  arg_ptr;

	uwTmpXStart = (tRotType == OSD_IMG_ROTATION_90)?uwYStart:uwXStart;
	uwTmpYStart = (tRotType == OSD_IMG_ROTATION_90)?(uwOSD_VSize - uwXStart):uwYStart;
	va_start(arg_ptr, pFmt);
	vsprintf((char *) ubPrintfString, pFmt, arg_ptr);
	va_end(arg_ptr);
	uwPrintfLen = strlen((char*)ubPrintfString);
	for(uwi = 0; uwi < uwPrintfLen; uwi++)
	{
		if((ubType = OSD_ImgString2Index((char)ubPrintfString[uwi], &uwOsd_ImgIdx)) != 0)
		{
			uint16_t uwOsdImgIdx = *(pOsdType[ubType] + uwOsd_ImgIdx);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);			
			switch(tRotType)
			{
				case OSD_IMG_ROTATION_90:
					tOsdImgInfo.uwXStart = uwTmpXStart;
					uwTmpYStart			-= tOsdImgInfo.uwVSize;	//!< ((uwi)?tOsdImgInfo.uwVSize:0);
					tOsdImgInfo.uwYStart = uwTmpYStart;
					if(uwTmpYStart < tOsdImgInfo.uwVSize)
					{
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
						uwi = uwPrintfLen;
						break;
					}
					tOSD_Img2(&tOsdImgInfo, ((uwi + 1) == uwPrintfLen)?tMode:OSD_QUEUE);
					break;
				case OSD_IMG_ROTATION_0:
					tOsdImgInfo.uwXStart = uwTmpXStart;
					tOsdImgInfo.uwYStart = uwTmpYStart;
					if((uwTmpXStart + tOsdImgInfo.uwHSize) >= uwOSD_HSize)
					{
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
						uwi = uwPrintfLen;
						break;
					}
					tOSD_Img2(&tOsdImgInfo, ((uwi + 1) == uwPrintfLen)?tMode:OSD_QUEUE);
					uwTmpXStart += ((uwi + 1) == uwPrintfLen)?0:tOsdImgInfo.uwHSize;
					break;
			}
		}
	}
}
//------------------------------------------------------------------------------
void OSD_FontPutc (OSD_FONT_PUT_INFOR_TYP *pInfor, uint32_t ulBufAddr)
{
	uint16_t  uwi, uwj;
	uint8_t	 *pBuf;
	uint8_t  *pFontBuf;	
	
	if (pInfor->pFontInfor->uwFontMax > pInfor->uwIndex)
		OSD_SfUpdateData(pInfor->pFontInfor->ulFontAddr + pInfor->uwIndex * pInfor->pFontInfor->uwFontH * pInfor->pFontInfor->uwFontV, tOSD_BufInfor.ulOsdFontBufAddr, pInfor->pFontInfor->uwFontH * pInfor->pFontInfor->uwFontV);
	else
		memset((uint8_t*)tOSD_BufInfor.ulOsdFontBufAddr, 0, pInfor->pFontInfor->uwFontH * pInfor->pFontInfor->uwFontV);
	pBuf = (uint8_t*)(ulBufAddr);
	pFontBuf = (uint8_t*)tOSD_BufInfor.ulOsdFontBufAddr;
	pBuf += pInfor->uwXStart + pInfor->uwYStart * uwOSD_HSize + (OSD_PAT_NUM << 1);
	for (uwi=0; uwi<pInfor->pFontInfor->uwFontV; ++uwi)
	{	
		for (uwj=0; uwj<pInfor->pFontInfor->uwFontH; ++uwj)
		{
			if (pFontBuf[uwj])
				pBuf[uwj] = OSD_PAT_NUM - tOSD_PatInfor.ubOsdFontPatNum + pFontBuf[uwj] - 1;
		}
		pBuf += uwOSD_HSize;
		pFontBuf += pInfor->pFontInfor->uwFontH;	
	}
}
//------------------------------------------------------------------------------
void OSD_FontQueueHandle (OSD_BUF_TYP *pOsdBuf)
{
	if ((OSD_IMG1_Q | OSD_IMG2_Q) == ((OSD_IMG1_Q | OSD_IMG2_Q) & ubOSD_BufFlag))
	{
		pOsdBuf->tBufState = OSD_BUF_FULL;
		ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
	}
	else if (OSD_IMG1_Q & ubOSD_BufFlag)
	{
		pOsdBuf->tBufState = OSD1_BUF_FULL;
		ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
	}
	else if (OSD_IMG2_Q & ubOSD_BufFlag)
	{
		pOsdBuf->tBufState = OSD2_BUF_FULL;
		ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
	}
	else
	{
		pOsdBuf->tBufState = OSD_BUF_EMPTY;
		ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_FONT_Q;
	}
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_MenuString (OSD_FONT_INFOR_TYP *pInfor, uint8_t ubItemIndex , OSD_UPDATE_TYP tMode)
{
	OSD_RESULT tFlag = OSD_OK;
	
	osMutexWait(OSD_Mutex, osWaitForever);
	if (uwOSD_HSize >= pInfor->uwXStart && uwOSD_VSize >= pInfor->uwYStart)
	{
		OSD_FONT_SWITCH_OUT_TYP *pFontInfor;
		
		pFontInfor = pOSD_FontSwitchLangColorRotation(pInfor->ubLangIndex, pInfor->tColorIndex, pInfor->tRotation);
		if (NULL != pFontInfor)
		{	
			if ((OSD_FONT_ROTATION_0   == pInfor->tRotation && uwOSD_HSize >= pInfor->uwXStart + pFontInfor->uwFontH && uwOSD_VSize >= pInfor->uwYStart + pFontInfor->uwFontV) ||
				(OSD_FONT_ROTATION_90  == pInfor->tRotation && uwOSD_HSize >= pInfor->uwXStart + pFontInfor->uwFontH && pInfor->uwYStart >= pFontInfor->uwFontV) ||
				(OSD_FONT_ROTATION_180 == pInfor->tRotation && pInfor->uwXStart >= pFontInfor->uwFontH && pInfor->uwYStart >= pFontInfor->uwFontV) ||
				(OSD_FONT_ROTATION_270 == pInfor->tRotation && pInfor->uwXStart >= pFontInfor->uwFontH && uwOSD_VSize >= pInfor->uwYStart + pFontInfor->uwFontV))
			{		
				OSD_BUF_STA_TYP  tBufState;
				OSD_BUF_TYP		*pOsdBuf;
				uint8_t   		 ubi;
				uint8_t   		 ubWordMax = (OSD_MENU_STRING_MAX_NUM < tOSD_MenuStringInfor.ubWordNum)?OSD_MENU_STRING_MAX_NUM:tOSD_MenuStringInfor.ubWordNum;
				uint16_t 		*pFontIndex;
				uint16_t		 uwHSize, uwVSize;
				uint32_t         ulCaseWordMax;
				uint32_t  		 ulMenuStringSft = OSD_MENU_STRING_SFT + (pInfor->ubLangIndex - 1) * tOSD_MenuStringInfor.ubItemNum * (tOSD_MenuStringInfor.ubWordNum << 1)
													+ ubItemIndex * (tOSD_MenuStringInfor.ubWordNum << 1);
				
				switch (pInfor->tRotation)
				{
					case OSD_FONT_ROTATION_0:
						ulCaseWordMax = (uwOSD_HSize - pInfor->uwXStart) / pFontInfor->uwFontH;
						if (ubWordMax > ulCaseWordMax)	ubWordMax = ulCaseWordMax;
						OSD_SfUpdateData(tOSD_SfAddr.ulOsdMenuStringSfStartAddr + ulMenuStringSft, tOSD_BufInfor.ulOsdMenuStringBufAddr, ubWordMax << 1);
						tBufState = tOSD_EraserBufFont (ubWordMax * pFontInfor->uwFontH, pFontInfor->uwFontV, pInfor->uwXStart, pInfor->uwYStart);
						break;
					case OSD_FONT_ROTATION_90:
						ulCaseWordMax = pInfor->uwYStart / pFontInfor->uwFontV;
						if (ubWordMax > ulCaseWordMax)	ubWordMax = ulCaseWordMax;
						OSD_SfUpdateData(tOSD_SfAddr.ulOsdMenuStringSfStartAddr + ulMenuStringSft, tOSD_BufInfor.ulOsdMenuStringBufAddr, ubWordMax << 1);
						uwVSize = ubWordMax * pFontInfor->uwFontV;
						tBufState = tOSD_EraserBufFont (pFontInfor->uwFontH, uwVSize, pInfor->uwXStart, pInfor->uwYStart - uwVSize);
						break;
					case OSD_FONT_ROTATION_180:
						ulCaseWordMax = pInfor->uwXStart / pFontInfor->uwFontH;
						if (ubWordMax > ulCaseWordMax)	ubWordMax = ulCaseWordMax;
						OSD_SfUpdateData(tOSD_SfAddr.ulOsdMenuStringSfStartAddr + ulMenuStringSft, tOSD_BufInfor.ulOsdMenuStringBufAddr, ubWordMax << 1);
						uwHSize = ubWordMax * pFontInfor->uwFontH;
						tBufState = tOSD_EraserBufFont (uwHSize, pFontInfor->uwFontV, pInfor->uwXStart - uwHSize, pInfor->uwYStart - pFontInfor->uwFontV);
						break;
					case OSD_FONT_ROTATION_270:
						ulCaseWordMax = (uwOSD_VSize - pInfor->uwYStart) / pFontInfor->uwFontV;
						if (ubWordMax > ulCaseWordMax)	ubWordMax = ulCaseWordMax;
						OSD_SfUpdateData(tOSD_SfAddr.ulOsdMenuStringSfStartAddr + ulMenuStringSft, tOSD_BufInfor.ulOsdMenuStringBufAddr, ubWordMax << 1);
						tBufState = tOSD_EraserBufFont (pFontInfor->uwFontH, ubWordMax * pFontInfor->uwFontV, pInfor->uwXStart - pFontInfor->uwFontH, pInfor->uwYStart);
						break;
				}			
				pFontIndex = (uint16_t*) tOSD_BufInfor.ulOsdMenuStringBufAddr;
				if (pFontIndex[ubi])
				{
					OSD_FONT_PUT_INFOR_TYP tPutInfor;			
					
					tPutInfor.pFontInfor = pFontInfor;
					tPutInfor.uwXStart = pInfor->uwXStart - ((OSD_FONT_ROTATION_90 < pInfor->tRotation)?pFontInfor->uwFontH:0);
					tPutInfor.uwYStart = pInfor->uwYStart - ((OSD_FONT_ROTATION_90 == pInfor->tRotation || OSD_FONT_ROTATION_180 == pInfor->tRotation)?pFontInfor->uwFontV:0);
					pOsdBuf = pOSD_GetOsdBufInfor();			
					for (ubi=0; ubi<ubWordMax && pFontIndex[ubi]; ++ubi)
					{
						tPutInfor.uwIndex = pFontIndex[ubi];
						OSD_FontPutc (&tPutInfor, pOsdBuf->ulBufAddr);
						switch (pInfor->tRotation)
						{
							case OSD_FONT_ROTATION_0:	
								tPutInfor.uwXStart += pFontInfor->uwFontH;
								break;
							case OSD_FONT_ROTATION_90:	
								tPutInfor.uwYStart -= pFontInfor->uwFontV;
								break;
							case OSD_FONT_ROTATION_180:	
								tPutInfor.uwXStart -= pFontInfor->uwFontH;
								break;
							case OSD_FONT_ROTATION_270:	
								tPutInfor.uwYStart += pFontInfor->uwFontV;
								break;
						}
					}
				}
				if (OSD_UPDATE == tMode)
					OSD_UpdateFont (tBufState);
				else
					OSD_FontQueueHandle(pOsdBuf);
			}
			else
			{
				printf("[OSD] OSD Location error\n");
				tFlag = OSD_LOCA_FAIL;
			}
		}
		else
		{
			printf("[OSD] OSD Switch error\n");
			tFlag = OSD_FONT_SWITCH_FAIL;
		}
	}
	else
	{
		printf("[OSD] OSD Location error\n");
		tFlag = OSD_LOCA_FAIL;
	}
	osMutexRelease(OSD_Mutex);
	return tFlag;
}
//------------------------------------------------------------------------------
uint16_t uwOSD_FontTranfer(uint8_t ubFont)
{
	if (('A' <= ubFont && 'Z' >= ubFont)
		|| ('a' <= ubFont && 'z' >= ubFont)
		|| ('-' <= ubFont && ':' >= ubFont))
		return ubFont;
	return 0xFFFF;
}
//------------------------------------------------------------------------------
bool bOSD_SpeString (uint16_t *pLen, uint8_t *pStr)
{
	uint8_t ubi;
	
	if ('%' == pStr[0] && 'B' == pStr[1] && 'A' == pStr[2] && 'T' == pStr[3])
	{
		ubi = atoi ((char*)&pStr[4]);
		if (ubi >= sizeof(ubOSD_BatL)) return false;
		pStr[0] = ubOSD_BatL[ubi];
		pStr[1] = ubOSD_BatR[ubi];
		*pLen = 2;
	}
	else if ('%' == pStr[0] && 'A' == pStr[1] && 'N' == pStr[2] && 'T' == pStr[3])
	{
		ubi = atoi ((char*)&pStr[4]);
		if (ubi >= sizeof(ubOSD_AntL)) return false;
		pStr[0] = ubOSD_AntL[ubi];
		pStr[1] = ubOSD_AntR[ubi];
		*pLen = 2;
	}
	else if ('%' == pStr[0] && 'V' == pStr[1] && 'D' == pStr[2] && 'O' == pStr[3])
	{
		ubi = atoi ((char*)&pStr[4]);
		if (ubi >= sizeof(ubOSD_VdoModeL)) return false;
		pStr[0] = ubOSD_VdoModeL[ubi];
		pStr[1] = ubOSD_VdoModeR[ubi];
		*pLen = 2;
	}
	else if ('%' == pStr[0] && 'R' == pStr[1] && 'E' == pStr[2] && 'C' == pStr[3])
	{
		ubi = atoi ((char*)&pStr[4]);
		if (ubi >= sizeof(ubOSD_Rec)) return false;
		pStr[0] = ubOSD_Rec[ubi];
		*pLen = 1;
	}
	else if ('%' == pStr[0] && 'C' == pStr[1] && 'A' == pStr[2] && 'M' == pStr[3])
	{
		ubi = atoi ((char*)&pStr[4]);
		if (ubi >= sizeof(ubOSD_Cam)) return false;
		pStr[0] = ubOSD_Cam[ubi];
		*pLen = 1;
	}
	else if ('%' == pStr[0] && 'B' == pStr[1] && 'A' == pStr[2] && 'R' == pStr[3] && '5' == pStr[4])
	{
		ubi = atoi ((char*)&pStr[5]);
		if (ubi >= sizeof(ubOSD_Bar5A)) return false;
		pStr[0] = ubOSD_Bar5A[ubi];
		pStr[1] = ubOSD_Bar5B[ubi];
		pStr[2] = ubOSD_Bar5C[ubi];
		pStr[3] = ubOSD_Bar5D[ubi];
		*pLen = 4;
	}
	else if ('%' == pStr[0] && 'B' == pStr[1] && 'A' == pStr[2] && 'R' == pStr[3] && '7' == pStr[4])
	{
		ubi = atoi ((char*)&pStr[5]);
		if (ubi >= sizeof(ubOSD_Bar7A)) return false;
		pStr[0] = ubOSD_Bar7A[ubi];
		pStr[1] = ubOSD_Bar7B[ubi];
		pStr[2] = ubOSD_Bar7C[ubi];
		pStr[3] = ubOSD_Bar7D[ubi];
		pStr[4] = ubOSD_Bar7E[ubi];
		pStr[5] = ubOSD_Bar7F[ubi];
		*pLen = 6;
	}
	else
		return false;
	return true;
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_FontPrintf (OSD_FONT_INFOR_TYP *pInfor, OSD_UPDATE_TYP tMode, char* pFmt, ...)
{
	OSD_FONT_SWITCH_OUT_TYP *pFontInfor;
	OSD_RESULT               tFlag = OSD_LOCA_FAIL;

	osMutexWait(OSD_Mutex, osWaitForever);
	pFontInfor = pOSD_FontSwitchLangColorRotation(pInfor->ubLangIndex, pInfor->tColorIndex, pInfor->tRotation);
	if (NULL != pFontInfor)
    {
		uint16_t   uwXStart, uwYStart;

		switch (pInfor->tRotation)
		{
				case OSD_FONT_ROTATION_0:
				   uwXStart = pInfor->uwXStart;
				   uwYStart = pInfor->uwYStart;
				   break;
				case OSD_FONT_ROTATION_90:
				   uwXStart = pInfor->uwYStart;
				   uwYStart = uwOSD_VSize - pInfor->uwXStart - pFontInfor->uwFontV;
				   break;
				case OSD_FONT_ROTATION_180:
				   uwXStart = uwOSD_HSize - pInfor->uwXStart - pFontInfor->uwFontH;
				   uwYStart = uwOSD_VSize - pInfor->uwYStart - pFontInfor->uwFontV;
				   break;
				case OSD_FONT_ROTATION_270:
				   uwXStart = uwOSD_HSize - pInfor->uwYStart - pFontInfor->uwFontH;
				   uwYStart = pInfor->uwXStart;
				default:
					break;
		}
		if (uwOSD_HSize >= uwXStart && uwOSD_VSize >= uwYStart)
		{
			uint16_t uwStringH=0, uwStringV=0, uwLen;
			uint16_t uwHSize, uwVSize;                          
			uint8_t  pString[OSD_MENU_STRING_MAX_NUM];
			va_list  arg_ptr;
			bool     bSpeStrFlag;

			va_start(arg_ptr, pFmt);
			vsprintf((char *) pString, pFmt, arg_ptr);
			va_end(arg_ptr);
			if (true == (bSpeStrFlag = bOSD_SpeString (&uwLen, pString)))
			{
				uwStringH = uwLen;
				uwStringV = 1;
			}
			else
			{
				char    *p, *q;

				uwLen = strlen((char*)pString);                                           
				for (p=(char*)pString, uwStringV=1;;p=q+1, ++uwStringV)
				{
					q = strchr(p, '\n');
					if (NULL == q)
					{
						if ('\0' == p[0])   --uwStringV;
						break;
					}
					uwStringH = LCDMAX(uwStringH, q - p);
				}
				uwStringH = LCDMAX(uwStringH, strlen(p));
			}
			if (OSD_FONT_ROTATION_90  == pInfor->tRotation || OSD_FONT_ROTATION_270  == pInfor->tRotation)
			{
			   uwHSize = uwStringV * pFontInfor->uwFontH;
			   uwVSize = uwStringH * pFontInfor->uwFontV;                                   
			}
			else
			{
			   uwHSize = uwStringH * pFontInfor->uwFontH;
			   uwVSize = uwStringV * pFontInfor->uwFontV;
			}                              
			if ((OSD_FONT_ROTATION_0   == pInfor->tRotation && uwOSD_HSize >= uwXStart + uwHSize && uwOSD_VSize >= uwYStart + uwVSize) ||
			    (OSD_FONT_ROTATION_90  == pInfor->tRotation && uwOSD_HSize >= uwXStart + uwHSize && uwYStart >= uwVSize) ||
			    (OSD_FONT_ROTATION_180 == pInfor->tRotation && uwXStart >= uwHSize && uwYStart >= uwVSize) ||
			    (OSD_FONT_ROTATION_270 == pInfor->tRotation && uwXStart >= uwHSize && uwOSD_VSize >= uwYStart + uwVSize))
			{
				OSD_FONT_PUT_INFOR_TYP tPutInfor;
				uint16_t                     uwi;
				OSD_BUF_STA_TYP           tBufState;
				OSD_BUF_TYP                    *pOsdBuf;
			   
				pOsdBuf = pOSD_GetOsdBufInfor();
				tPutInfor.pFontInfor = pFontInfor;
				switch (pInfor->tRotation)
				{
					case OSD_FONT_ROTATION_0:
						tBufState = tOSD_EraserBufFont (uwHSize, uwVSize, uwXStart, uwYStart);
						tPutInfor.uwXStart = uwXStart;
						tPutInfor.uwYStart = uwYStart;
						break;
					case OSD_FONT_ROTATION_90:
						tBufState = tOSD_EraserBufFont (uwHSize, uwVSize, uwXStart, uwYStart - uwVSize);
						tPutInfor.uwXStart = uwXStart;
						tPutInfor.uwYStart = uwYStart - pFontInfor->uwFontV;
						break;
					case OSD_FONT_ROTATION_180:        
						tBufState = tOSD_EraserBufFont (uwHSize, uwVSize, uwXStart - uwHSize, uwYStart - uwVSize);
						tPutInfor.uwXStart = uwXStart - pFontInfor->uwFontH;
						tPutInfor.uwYStart = uwYStart - pFontInfor->uwFontV;
						break;
					case OSD_FONT_ROTATION_270:
						tBufState = tOSD_EraserBufFont (uwHSize, uwVSize, uwXStart - uwHSize, uwYStart);
						tPutInfor.uwXStart = uwXStart - pFontInfor->uwFontH;
						tPutInfor.uwYStart = uwYStart;
						break;
				}
				for (uwi=0; uwi<uwLen; ++uwi)
				{
					switch(pString[uwi])
					{
						case '\n':
							switch (pInfor->tRotation)
							{
							case OSD_FONT_ROTATION_0:
								tPutInfor.uwXStart = uwXStart;
								tPutInfor.uwYStart += pFontInfor->uwFontV;
								break;
							case OSD_FONT_ROTATION_90:
								tPutInfor.uwXStart += pFontInfor->uwFontH;
								tPutInfor.uwYStart = uwYStart - pFontInfor->uwFontV;
								break;
							case OSD_FONT_ROTATION_180:
								tPutInfor.uwXStart = uwXStart - pFontInfor->uwFontH;
								tPutInfor.uwYStart -= pFontInfor->uwFontV;
								break;
							case OSD_FONT_ROTATION_270:
								tPutInfor.uwXStart -= pFontInfor->uwFontH;
								tPutInfor.uwYStart = uwYStart;
								break;
							}
							break;
						default:
							tPutInfor.uwIndex = (true == bSpeStrFlag)?pString[uwi]:uwOSD_FontTranfer(pString[uwi]);
							OSD_FontPutc (&tPutInfor, pOsdBuf->ulBufAddr);
						case ' ':
						case '\t':
							switch (pInfor->tRotation)
							{
								case OSD_FONT_ROTATION_0:
									tPutInfor.uwXStart += pFontInfor->uwFontH;
									break;
								case OSD_FONT_ROTATION_90:
									tPutInfor.uwYStart -= pFontInfor->uwFontV;
									break;
								case OSD_FONT_ROTATION_180:
									tPutInfor.uwXStart -= pFontInfor->uwFontH;
									break;
								case OSD_FONT_ROTATION_270:                                                                                              
									tPutInfor.uwYStart += pFontInfor->uwFontV;
									break;
							}
							break;
						case '\r':
							break;
					}
				}
				if (OSD_UPDATE == tMode)
					OSD_UpdateFont (tBufState);
				else
					OSD_FontQueueHandle(pOsdBuf);
				tFlag = OSD_OK;
			}
		}
        if (OSD_LOCA_FAIL == tFlag)
			printf("[OSD] OSD Location error\n");
    }
	else
	{
		printf("[OSD] OSD Switch error\n");
		tFlag =        OSD_FONT_SWITCH_FAIL;           
	}
	osMutexRelease(OSD_Mutex);
	return tFlag;

}
//------------------------------------------------------------------------------
void OSD_UpdateQueueBuf (void)
{
	osMutexWait(OSD_Mutex, osWaitForever);
	if (OSD_FONT_Q & ubOSD_BufFlag)
	{
		OSD_BUF_TYP	*pOsdBuf;
		
		pOsdBuf = pOSD_GetOsdBufInfor();
		pOsdBuf->tBufState = FONT_BUF_FULL;
		ubOSD_BufFlag = (ubOSD_BufFlag & OSD_BUF_STATE_MASK) | OSD_BUF_UPDATE;
	}
	else if (OSD_IMG2_Q & ubOSD_BufFlag)
		OSD_UpdateImg2();
	else if (OSD_IMG1_Q & ubOSD_BufFlag)
		OSD_UpdateImg1();
	osMutexRelease(OSD_Mutex);
}
//------------------------------------------------------------------------------
OSD_RESULT tOSD_DataCopy(uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulLen)
{
#define OSD_MEMLEN_MAX	 0x3FFFFF
	if(ulLen >= OSD_MEMLEN_MAX)
		return OSD_DATACPY_FAIL;
	return (tDMAC_MemCopy(ulSrcAddr, ulDestAddr, ulLen, NULL) == DMAC_OK)?OSD_OK:OSD_DATACPY_FAIL;
}
