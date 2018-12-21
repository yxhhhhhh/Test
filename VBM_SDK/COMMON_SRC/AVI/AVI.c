/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       AVI.c
	\brief		AVI File format
	\author		Wales
	\version    0.2
	\date		2017/03/21
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "REC_API.h"
#include "PLY_API.h"
#include "MEDIA.h"
#include "AVI.h"
#include "FS_API.h"
#include "DMAC_API.h"
#if 1 // for 510pf compiler pass
#include "AVI_DUMY.h"
#endif

//------------------------------------------------------------------------------
//	DEFINITION                       |
//------------------------------------------------------------------------------
#define AVI_SRC_NUM             (REC_SRC_NUM)
#define AVI_STR_V1		(REC_STR_V1)
#define AVI_STR_A1		(REC_STR_A1)

#define AVI_FRMTYPE_VDO     (PLY_FRMTYPE_VDO)
#define AVI_FRMTYPE_ADO     (PLY_FRMTYPE_ADO)

#define AVI_V_PFRM		(REC_P_VFRM)
#define AVI_V_IFRM		(REC_I_VFRM)
#define AVI_V_SFRM		(REC_SKIP_FRM)

#define AVI_RES_NONE		(REC_RES_NONE)
#define AVI_RES_FHD		(REC_RES_FHD)
#define AVI_RES_HD		(REC_RES_HD)
#define AVI_RES_WVGA		(REC_RES_WVGA)
#define AVI_RES_VGA		(REC_RES_VGA)

//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------
#define AVI_INDEX_MAX(Vfps,Afps,Time)	((Vfps+Afps)*60*Time)		// AVI Index frame number.
#define AVI_INDEX_MAXSIZE(Vfps,Afps,Time)	(AVI_INDEX_MAX(Vfps,Afps,Time)*(sizeof(AVIOLDINDEX)-8)+8)	// AVI Index frame data size.

//------------------------------------------------------------------------------
//	GLOBAL VARIABLE
//------------------------------------------------------------------------------
osSemaphoreId SEM_AVI_WRFrm;

uint8_t ubAVI_CfgSrcNum;									// Indicate confiuration source
uint32_t ulAVI_RecTime = 0;									// Indicate Max record time
uint8_t ubAVI_MaxVFPS = 0;
uint8_t ubAVI_MaxAFPS = 0;

uint8_t ubAVI_AdoFrmType;

uint32_t	ulAVI_IdxStartAdr;
uint32_t	ulAVI_IdxSize[AVI_SRC_NUM];						// AVI Index frame data size.

AVISTRUCT AVIS_Rec[AVI_SRC_NUM];
AVISTRUCT AVIS_Play[AVI_SRC_NUM];
AVIOLDINDEX *AVI_Idx1[AVI_SRC_NUM];
AVICHUNK AVI_RecMovi[AVI_SRC_NUM];
AVICHUNK AVI_PlyMovi[AVI_SRC_NUM];

int32_t slAVI_Length[AVI_SRC_NUM] = {0};            // Indicate total file size.
int32_t slAVI_IdxOffset[AVI_SRC_NUM] = {0x4};       // Indicate frame offset position in AVI file.
int32_t slAVI_IndexNum[AVI_SRC_NUM] = {0};          // Indicate idx1 index, equal to frame number (video + audio frame).

uint32_t ulAVI_HSize[AVI_SRC_NUM];                  // Indicate video frame Horizontal resolution.
uint32_t ulAVI_VSize[AVI_SRC_NUM];                  // Indicate video frame Vertical resolution.
uint16_t uwAVI_VFrameInterval[AVI_SRC_NUM];

uint32_t ulAVI_AdoSampleRate[AVI_SRC_NUM];
uint16_t uwAVI_AdoBlockAlign[AVI_SRC_NUM];
uint16_t uwAVI_AdoSamplesPerBlock[AVI_SRC_NUM];
uint16_t uwAVI_AFrameInterval[AVI_SRC_NUM];

uint8_t ubAVI_H264PIC[AVI_SRC_NUM] = {0};           // for skip frame using
uint8_t ubAVI_H264POC[AVI_SRC_NUM] = {0};           // for skip frame using
uint32_t ulSkipFrameBuf;                            // for save skip frame data.
uint32_t ulAVI_SfTotalSize[AVI_SRC_NUM] = {0};      // Indicate Skip frame total Size.
uint32_t ulThumbnailAdr;

// video
uint8_t cbAVI_Strf1[]=
{
	 's',  't',  'r',  'f', 0x40, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, //width:0x500
	0xD0, 0x02, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, //height:0x2D0
	0x48, 0x32, 0x36, 0x34, 0x00, 0x30, 0x2a, 0x00, //SizeImage=0x2a3000(width * height * 3)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x67, 0x4D, 0x40, 0x29,
	0x96, 0x54, 0x06, 0x40, 0x9B, 0x20, 0x00, 0x00,
	0x00, 0x01, 0x68, 0xEE, 0x38, 0x80, 0x00, 0x00,
};
// audio
uint8_t cbAVI_Strf2[] = {
	 's',  't',  'r',  'f', 0x34, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x01, 0x00, 0x40, 0x1F, 0x00, 0x00, //1 channel, sample rate 0x1F40=8K bps
	0xCF, 0x0F, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00, //average byte per second:0x0FCF(8K bps).
	0x20, 0x00, 0xF4, 0x03, 0x07, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x02, 0x00, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0xC0, 0x00, 0x40, 0x00, 0xF0, 0x00,
	0x00, 0x00, 0xCC, 0x01, 0x30, 0xFF, 0x88, 0x01,
	0x18, 0xFF, 0x00, 0x00,
};

uint8_t cbAVI_SkipFrame_HD[] = {
	'0' , '0' , 'd' , 'c' , 0x24, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x01, 0x9A, 0x00, 0x01, 0xAF, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
	0x03, 0x00, 0x01, 0x03
};

uint8_t cbAVI_SkipFrame_FHD[] = {
	'0' , '0' , 'd' , 'c' , 0x46, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x01, 0x9A, 0x00, 0x01, 0xAF, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
	0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
	0x00, 0x03, 0x00, 0x8D, 0x80, 0x00,
};

uint8_t cbAVI_SkipFrame[] = { '0' , '0' , 'd' , 'c' , 0x00, 0x00, 0x00, 0x00,};

#define AVI_SKIP_FRAME_DATA     cbAVI_SkipFrame_HD

#define AVI_SKIP_FRAME_PIECE    (100)
#define AVI_SF_MAX_DRAM_SIZE    (sizeof(AVI_SKIP_FRAME_DATA)*AVI_SKIP_FRAME_PIECE)//100 piece of skip frame
// RECORD PART

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//	AVI Write One Index
//------------------------------------------------------------------------------
static int32_t slAVI_WriteIndex(uint8_t ubCh, uint8_t ubStreamType, uint16_t uwFrameType, uint32_t ulSize)
{
	if (AVI_Idx1[ubCh]->cb > ulAVI_IdxSize[ubCh])						//don't continue.
	{
		printf("AVIWriteIdx,cb[%d] Over Spec = %x\n\r", ubCh, AVI_Idx1[ubCh]->cb);
		return -1;
	}
	
	if(ubStreamType == AVI_STR_V1)
		memcpy(&AVI_Idx1[ubCh]->aIndex[slAVI_IndexNum[ubCh]].ulChunkId, "00dc", 4);//stream1:video
	else
		memcpy(&AVI_Idx1[ubCh]->aIndex[slAVI_IndexNum[ubCh]].ulChunkId, "01wb", 4);//stream2:audio

	if(uwFrameType == AVI_V_IFRM || ubStreamType == AVI_STR_A1)	//Audio is Keyframe for each frame.
		AVI_Idx1[ubCh]->aIndex[slAVI_IndexNum[ubCh]].ulFlags = 0x10;//mark it as I frame
	else
		AVI_Idx1[ubCh]->aIndex[slAVI_IndexNum[ubCh]].ulFlags = 0x0;

	AVI_Idx1[ubCh]->aIndex[slAVI_IndexNum[ubCh]].ulOffset = slAVI_IdxOffset[ubCh];
	AVI_Idx1[ubCh]->aIndex[slAVI_IndexNum[ubCh]].ulSize = ulSize;	//real size, this can not be compensated odd byte to even byte
	if (ulSize % 2 == 1)									//compensate odd byte to even byte
		ulSize++;

	slAVI_IdxOffset[ubCh] += ulSize + AVI_BYTE_OF_HEADER;

	AVI_Idx1[ubCh]->cb += 16;								//16:index size of a frame
	slAVI_IndexNum[ubCh]++;
	
	if(slAVI_IndexNum[ubCh] >= AVI_INDEX_MAX((1000/uwAVI_VFrameInterval[ubCh]),(10000/uwAVI_AFrameInterval[ubCh]),ulAVI_RecTime))
	{
		printf("AVI Write Idx, Idx frm number Over. Src:%d Stream:%d\n\r", ubCh, ubStreamType);
		return -2;
	}
	return 0;
}

//------------------------------------------------------------------------------
//	AVI Update Header
//------------------------------------------------------------------------------
void AVI_UpdateHeader(uint8_t ubCh)
{
	uint32_t ulAddr;
	
	//save total length for RIFF
	AVIS_Rec[ubCh].RIFF.ulSize = slAVI_Length[ubCh] - AVI_BYTE_OF_HEADER + (AVI_Idx1[ubCh]->cb + AVI_BYTE_OF_HEADER);

	//update index
	if (AVI_Idx1[ubCh]->cb > ulAVI_IdxSize[ubCh])
	{
		printf("AVI Index DataSize OverFlow\r\n");
		//System_Stop("AVI Update Hd, Idx1->cb Over Spec = %x\n\r", AVI_Idx1[ubCh]->cb);
	}
	else
	{
		FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(AVI_Idx1[ubCh]), AVI_Idx1[ubCh]->cb + AVI_BYTE_OF_HEADER);//update index
		
		ulAddr = ((uint32_t)AVI_Idx1[ubCh] + AVI_Idx1[ubCh]->cb + AVI_BYTE_OF_HEADER - 16);
		
		if ((*(uint32_t *)(ulAddr) != AVI_FRMTYPE_VDO) && (*(uint32_t *)(ulAddr) != AVI_FRMTYPE_ADO))
		{
			printf("AVI Index Frm Type Err\r\n");
			//System_Stop("AVI_Idx1 StremType ERR!\n\r");
		}
	}
	printf("AVI Update Hd, Idx1 Ch:%d Sz:0x%x \n\r", ubCh, AVI_Idx1[ubCh]->cb + AVI_BYTE_OF_HEADER );
}

uint16_t uwAVI_GetVersion(void)
{
    return ((AVI_MAJORVER << 8) + AVI_MINORVER);
}

void AVI_SemaphoreCreate(void)
{
	osSemaphoreDef(SEM_AVI_WRFrm);
	SEM_AVI_WRFrm    = osSemaphoreCreate(osSemaphore(SEM_AVI_WRFrm), 1);	
}

void AVI_SetConfigedSrcNum(uint8_t ubSrcNum)
{
	ubAVI_CfgSrcNum = ubSrcNum;
}
	
uint32_t ulAVI_GetBufSz(void)
{
	uint8_t i;
	uint32_t ulSize = 0;
	
	for(i=0; i < ubAVI_CfgSrcNum; i++)
		ulSize += AVI_INDEX_MAXSIZE(ubAVI_MaxVFPS,ubAVI_MaxAFPS,ulAVI_RecTime);
	
	ulSize += (AVI_SF_MAX_DRAM_SIZE + AVI_THUMBNAIL_SIZE);
	return ulSize;
}

//------------------------------------------------------------------------------
//	AVI Initial
//------------------------------------------------------------------------------
void AVI_Init(uint32_t ulStartAdr)
{
	uint16_t i;
	uint32_t ulBufIdxAdr = 0;
	uint32_t ulBufAdr = 0;

	ulAVI_IdxStartAdr = ulStartAdr;

	for(i=0; i < ubAVI_CfgSrcNum; i++)
	{
		AVI_Idx1[i] = (void *)(ulAVI_IdxStartAdr + ulBufIdxAdr);
		ulAVI_IdxSize[i] = AVI_INDEX_MAXSIZE(ubAVI_MaxVFPS,ubAVI_MaxAFPS,ulAVI_RecTime);
		ulBufIdxAdr += ulAVI_IdxSize[i];
	}

	ulSkipFrameBuf = (uint32_t)(ulAVI_IdxStartAdr + ulBufIdxAdr);

	for(i=0; i<AVI_SKIP_FRAME_PIECE; i++)					//fill skip frame into dram
	{
		tDMAC_MemCopy((uint32_t)AVI_SKIP_FRAME_DATA,(ulSkipFrameBuf+ulBufAdr),sizeof(AVI_SKIP_FRAME_DATA),NULL);
		ulBufAdr += sizeof(AVI_SKIP_FRAME_DATA);
	}

	// Thumbnail start address
	ulThumbnailAdr = ulSkipFrameBuf + AVI_SF_MAX_DRAM_SIZE;
	
	ubAVI_AdoFrmType = ADO_FRMTYPE;
}

//------------------------------------------------------------------------------
//	AVI Set Resolution for Video
//------------------------------------------------------------------------------
void AVI_Vdo1SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval)
{
	ulAVI_HSize[ubCh] = ulHSize;
	ulAVI_VSize[ubCh] = ulVSize;
	uwAVI_VFrameInterval[ubCh] = uwFrmInterval;
}

//------------------------------------------------------------------------------
//	AVI Set Format for Audio
//------------------------------------------------------------------------------
void AVI_AdoSetFormat(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval)
{
	ulAVI_AdoSampleRate[ubCh] = ulSampleRate;
	uwAVI_AdoBlockAlign[ubCh] = uwBlockAlign;
	uwAVI_AdoSamplesPerBlock[ubCh] = (((uwBlockAlign-7)*8)/4) + 2;
	uwAVI_AFrameInterval[ubCh] = uwFrmInterval;
}

uint32_t ulAVI_GetFixedMemorySize(void)
{
	return (AVI_SF_MAX_DRAM_SIZE+AVI_THUMBNAIL_SIZE);	
}

void AVI_SetRecMaxTime(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute)
{
	ubAVI_MaxVFPS = ubVFPS;
	ubAVI_MaxAFPS = ubAFPS;
	ulAVI_RecTime = ulMinute;
}

//------------------------------------------------------------------------------
//	AVI Write One Frame
//	ubStreamType: ulVideo: Is video frame. AVI_STR_V1:Video, AVI_STR_A1:Audio
//	uwFrameType: Is Key frame.
//	ulAddr: Data address of dram be read to write into AVI.
//	ulSize: Data size.
//------------------------------------------------------------------------------
int32_t slAVI_WriteOneFrame(uint8_t ubCh, uint8_t ubStreamType, uint16_t uwFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID)
{
	uint8_t ubFrameTag[8];
	//uint8_t ubPICSave = 0, ubPOCSave = 1;
	uint32_t ulTmpSize = 0, ulCalSize;
	int32_t slVal;

	ulTmpSize = ulSize;
	
	if(osSemaphoreWait(SEM_AVI_WRFrm ,10) != osOK)
	{
		printf("SEM_AVI_WRFrm busy\r\n");
		return -10;
	}
	
	//write index
	slVal = slAVI_WriteIndex(ubCh, ubStreamType, uwFrameType, ulSize);
	if (slVal < 0)											//some error
	{
		osSemaphoreRelease(SEM_AVI_WRFrm);
		return slVal;
	}
	
	//write stream tag to Dram
	if (ubStreamType == AVI_STR_V1)
		memcpy(ubFrameTag, "00dc", 4);
	else
		memcpy(ubFrameTag, "01wb", 4);

	slAVI_Length[ubCh] += 4;								//length of chunk ID
	memcpy(&ubFrameTag[4], &ulSize, 4);

	if (ulAVI_SfTotalSize[ubCh] >= AVI_SF_MAX_DRAM_SIZE)			//if dram length of skip frame is full, then save it.
	{
		FS_WriteFile((FS_SRC_NUM)ubCh, ulSkipFrameBuf, ulAVI_SfTotalSize[ubCh]);
		ulAVI_SfTotalSize[ubCh] = 0;
	}
	
	if ((uwFrameType != AVI_V_SFRM))
	{
		if (ulAVI_SfTotalSize[ubCh] > 0)							//if there is skip frame in dram
		{
			FS_WriteFile((FS_SRC_NUM)ubCh, ulSkipFrameBuf, ulAVI_SfTotalSize[ubCh]);
			ulAVI_SfTotalSize[ubCh] = 0;
		}
		//=== write frame size to FS ===
		FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(ubFrameTag), 8);
		//=== check data after FS wrtie ===
		ulCalSize = *(uint8_t *)(ubFrameTag+4) + (((uint32_t)*(uint8_t *)(ubFrameTag+5)) << 8) + (((uint32_t)*(uint8_t *)(ubFrameTag+6)) << 16) + (((uint32_t)*(uint8_t *)(ubFrameTag+7)) << 24);
		if (ubStreamType == AVI_STR_V1)
		{
			if (*(uint8_t *)ubFrameTag != '0' || (*(uint8_t *)(ubFrameTag+1)) != '0' || (*(uint8_t *)(ubFrameTag+2)) != 'd' || (*(uint8_t *)(ubFrameTag+3)) != 'c'
				|| (ulTmpSize != ulSize) || (ulCalSize != ulSize))
			{
				printf("AVI DRAM Tag ERR!\n\r");
				osSemaphoreRelease(SEM_AVI_WRFrm);
				return -3;
			}
		}
		else
		{
			if (*(uint8_t *)ubFrameTag != '0' || (*(uint8_t *)(ubFrameTag+1)) != '1' || (*(uint8_t *)(ubFrameTag+2)) != 'w' || (*(uint8_t *)(ubFrameTag+3)) != 'b'
				|| (ulTmpSize != ulSize) || (ulCalSize != ulSize))
			{
				printf("AVI DRAM Tag ERR!\n\r");
				osSemaphoreRelease(SEM_AVI_WRFrm);
				return -4;
			}
		}
	}

	slAVI_Length[ubCh] += 4;								//length of chunk size
	if(ulSize % 2 == 1)										//compensate odd byte to even byte
		ulSize++;

	// update PIC and POC
	if (ubStreamType == AVI_STR_V1)
	{
		if(uwFrameType == AVI_V_IFRM)
		{
			ubAVI_H264PIC[ubCh] = 0+2;					//I frame, reset PIC and POC.
			ubAVI_H264POC[ubCh] = 1+2;
		}
		else if (uwFrameType == AVI_V_PFRM)
		{
			//ubPICSave = *(unsigned char *)(ulAddr+6);		//backup original PIC
			//ubPOCSave = *(unsigned char *)(ulAddr+7);		//backup original POC
			*(unsigned char *)(ulAddr+6) = ubAVI_H264PIC[ubCh];
			*(unsigned char *)(ulAddr+7) = ubAVI_H264POC[ubCh];
			ubAVI_H264PIC[ubCh] += 2;
			ubAVI_H264POC[ubCh] += 2;
		}
		else	//Skip frame
		{
			*(unsigned char *)(ulSkipFrameBuf+ulAVI_SfTotalSize[ubCh]+8+6) = ubAVI_H264PIC[ubCh];
			*(unsigned char *)(ulSkipFrameBuf+ulAVI_SfTotalSize[ubCh]+8+7) = ubAVI_H264POC[ubCh];
			ubAVI_H264POC[ubCh] += 2;
			
			if (ubAVI_H264POC[ubCh] == 0x1)				//POC is overflow.
			{
				if ((ubAVI_H264PIC[ubCh] % 2) == 0)		//PIC is even value normally.
					ubAVI_H264PIC[ubCh] += 1;			//if POC overflow was odd times(first, third...), PIC change to odd value.
				else
					ubAVI_H264PIC[ubCh] -= 1;			//if POC overflow was even times(second, fourth...), PIC back to even value.
			}
		}
	}
	
	//write frame data to FS
	if (uwFrameType != AVI_V_SFRM)
	{
		//=== check data before FS wrtie ===
		if (ubStreamType == AVI_STR_V1)
		{
			if (*(uint8_t *)ulAddr != 0x00 || (*(uint8_t *)(ulAddr+1)) != 0x00 || (*(uint8_t *)(ulAddr+2)) != 0x00 || (*(uint8_t *)(ulAddr+3)) != 0x01)
			{
				//apENTER_CRITICAL();
				printf("AVI DRAM Data ERR, Addr=%x , Data= %x %x %x %x \n\r", ulAddr, *(uint8_t *)ulAddr, *(uint8_t *)(ulAddr+1), *(uint8_t *)(ulAddr+2), *(uint8_t *)(ulAddr+3));
				osDelay(10);	//reserved a lot time for UART print.
				osSemaphoreRelease(SEM_AVI_WRFrm);
				return -5;
			}
		}
		FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)ulAddr, ulSize);

#if 0	// Mask CRC check 
		//=== check data after FS wrtie ===
		if (ubStreamType == AVI_STR_V1)
		{
			uint32_t ulDbgSize;
			uint8_t	ubData;
			uint16_t	uwCrc;
			uint16_t	uwData;
			
			if (*(uint8_t *)ulAddr != 0x00 || (*(uint8_t *)(ulAddr+1)) != 0x00 || (*(uint8_t *)(ulAddr+2)) != 0x00 || (*(uint8_t *)(ulAddr+3)) != 0x01)
			{
				printf("AVI DRAM Data ERR!\n\r");
				osSemaphoreRelease(SEM_AVI_WRFrm);
				return -6;
			}
			
			ulDbgSize = ulSize + ulH264_GetStreamOffset();
			
			if (ulDbgSize <= (MAX_PKT_LEN - (HDR_WIFI_LEN + HDR_VDOP_LEN)))
				ulDbgSize = (MAX_PKT_LEN - (HDR_WIFI_LEN + HDR_VDOP_LEN)) + 8;

			if (uwFrameType == AVI_V_PFRM)
			{
				*(unsigned char *)(ulAddr+6) = ubPICSave;	//restore original PIC
				*(unsigned char *)(ulAddr+7) = ubPOCSave;	//restore original POC
			}
			ulDbgSize = (ulDbgSize +15)&(~15);				//16byte alignment.

			if (ulCrc16_Compute(ulAddr - ulH264_GetStreamOffset(), ulDbgSize, 0) == SUCCESS)
			{
				uwCrc = uwCrc16_Get();
				ubData = *(uint8_t *)(ulAddr - ulH264_GetStreamOffset() + ulDbgSize + 0);
				uwData = (uint16_t)ubData<<8;
				ubData = *(uint8_t *)(ulAddr - ulH264_GetStreamOffset() + ulDbgSize + 1);
				uwData += ubData;
				if (uwData != uwCrc)
					printf("ERR %d, AVI CRC:%x <-> %x\n\r", ubCh, uwData, uwCrc);
			}
			else
				printf("AVI CRC TimeOut\n\r");
		}
#endif

	}
	else
		ulAVI_SfTotalSize[ubCh] += sizeof(AVI_SKIP_FRAME_DATA);	//The Size of a Skip Frame.

	slAVI_Length[ubCh] += ulSize;						//frame size
	
	if (ubStreamType == AVI_STR_V1)
	{
		AVIS_Rec[ubCh].hl.avih.ulTotalFrames += 1;		//update video frame count
		AVIS_Rec[ubCh].hl.s1.strh.ulLength += 1;
	}
	else
		AVIS_Rec[ubCh].hl.s2.strh.ulLength += 1;			//update audio frame count

	//update movi size
	AVI_RecMovi[ubCh].ulSize += (AVI_BYTE_OF_HEADER+ulSize);

	osSemaphoreRelease(SEM_AVI_WRFrm);
	return 0;
}

//------------------------------------------------------------------------------
//	AVI Create File
//------------------------------------------------------------------------------
void AVI_CreateFile(uint8_t ubCh)
{
	slAVI_Length[ubCh] = 0x0;										//first chunk offset.
	slAVI_IdxOffset[ubCh] = sizeof(AVISTRUCT) + AVI_THUMBNAIL_SIZE + sizeof(AVICHUNK);	//this value can been playing by WinXp's media-play. 20140529
	slAVI_IndexNum[ubCh] = 0;
	ubAVI_H264PIC[ubCh] = 0;										//for skip frame using
	ubAVI_H264POC[ubCh] = 0;										//for skip frame using
	ulAVI_SfTotalSize[ubCh] = 0;											//for skip frame buffer using
	
	//RIFF
	memcpy(&AVIS_Rec[ubCh].RIFF.RIFF, "RIFF", 4);
	AVIS_Rec[ubCh].RIFF.ulSize = 0;
	memcpy(&AVIS_Rec[ubCh].RIFF.ID, "AVI ", 4);

	memcpy(&AVIS_Rec[ubCh].hl.hdrl.RIFF, "LIST", 4);
	memcpy(&AVIS_Rec[ubCh].hl.hdrl.ID, "hdrl", 4);

	////hdrl size
	AVIS_Rec[ubCh].hl.hdrl.ulSize = sizeof(AVIS_Rec[ubCh].hl) - AVI_BYTE_OF_HEADER;
	
	memset(&AVIS_Rec[ubCh].hl.avih, 0, sizeof(AVIS_Rec[ubCh].hl.avih));				//clear AVI avih
	memset(&AVIS_Rec[ubCh].hl.s1.strh, 0, sizeof(AVIS_Rec[ubCh].hl.s1.strh));			//clear AVI strh1
	memset(&AVIS_Rec[ubCh].hl.s2.strh, 0, sizeof(AVIS_Rec[ubCh].hl.s2.strh));			//clear AVI strh2
	memset(AVI_Idx1[ubCh], 0, AVI_BYTE_OF_HEADER);								//clear AVI_Idx1
	memset(&AVIS_Rec[ubCh].info.ISFT.data, 0, sizeof(AVIS_Rec[ubCh].info.ISFT.data));	//clear AVI info data

	memcpy(&AVIS_Rec[ubCh].hl.avih.fcc, "avih", 4);
	AVIS_Rec[ubCh].hl.avih.cb = 0x38;
	AVIS_Rec[ubCh].hl.avih.ulMicroSecPerFrame = uwAVI_VFrameInterval[ubCh] * 1000;	// 33ms
	AVIS_Rec[ubCh].hl.avih.ulMaxBytesPerSec = 16000;
	AVIS_Rec[ubCh].hl.avih.ulPaddingGranularity = 0x0;
	AVIS_Rec[ubCh].hl.avih.ulFlags = AVI_TRUSTCKTYPE | AVI_ISINTERLEAVED | AVI_HASINDEX;
	AVIS_Rec[ubCh].hl.avih.ulTotalFrames = 0x0;
	AVIS_Rec[ubCh].hl.avih.ulInitialFrames = 0x0;
	AVIS_Rec[ubCh].hl.avih.ulStreams = 0x2;										// 2 streams, video + audio
	AVIS_Rec[ubCh].hl.avih.ulSuggestedBufferSize = 0x100000;
	AVIS_Rec[ubCh].hl.avih.ulWidth = ulAVI_HSize[ubCh];
	AVIS_Rec[ubCh].hl.avih.ulHeight = ulAVI_VSize[ubCh];

	//------------Video---------------
	// strh
	memcpy(&AVIS_Rec[ubCh].hl.s1.strl.RIFF, "LIST", 4);
	memcpy(&AVIS_Rec[ubCh].hl.s1.strl.ID, "strl", 4);
	AVIS_Rec[ubCh].hl.s1.strl.ulSize = sizeof(AVIS_Rec[ubCh].hl.s1.strh) + sizeof(AVIS_Rec[ubCh].hl.s1.strf) + AVI_BYTE_OF_FCC;
	memcpy(&AVIS_Rec[ubCh].hl.s1.strh.fcc, "strh", 4);
	AVIS_Rec[ubCh].hl.s1.strh.cb = 0x38;
	memcpy(&AVIS_Rec[ubCh].hl.s1.strh.fccType, "vids", 4);
	memcpy(&AVIS_Rec[ubCh].hl.s1.strh.fccHandler, "H264", 4);
	AVIS_Rec[ubCh].hl.s1.strh.ulFlags = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.uwPriority = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.uwLanguage = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.ulInitialFrames = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.ulScale = uwAVI_VFrameInterval[ubCh];
 	AVIS_Rec[ubCh].hl.s1.strh.ulRate = 1000;
	AVIS_Rec[ubCh].hl.s1.strh.ulStart = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.ulLength = 0x00;
	AVIS_Rec[ubCh].hl.s1.strh.ulSuggestedBufferSize = 0x100000;
	AVIS_Rec[ubCh].hl.s1.strh.ulQuality = 0xffffffff;
	AVIS_Rec[ubCh].hl.s1.strh.ulSampleSize = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.rcFrame.left = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.rcFrame.top = 0x0;
	AVIS_Rec[ubCh].hl.s1.strh.rcFrame.right = ulAVI_HSize[ubCh];
	AVIS_Rec[ubCh].hl.s1.strh.rcFrame.bottom = ulAVI_VSize[ubCh];
	// strf
	memcpy(&AVIS_Rec[ubCh].hl.s1.strf, cbAVI_Strf1, sizeof(cbAVI_Strf1));
	AVIS_Rec[ubCh].hl.s1.strf.info.ulWidth = ulAVI_HSize[ubCh];
	AVIS_Rec[ubCh].hl.s1.strf.info.ulHeight = ulAVI_VSize[ubCh];
	AVIS_Rec[ubCh].hl.s1.strf.info.ulSizeImage = ulAVI_HSize[ubCh]*ulAVI_VSize[ubCh]*3;

	//------------Audio---------------
	// strh
	memcpy(&AVIS_Rec[ubCh].hl.s2.strl.RIFF, "LIST", 4);
	memcpy(&AVIS_Rec[ubCh].hl.s2.strl.ID, "strl", 4);
	AVIS_Rec[ubCh].hl.s2.strl.ulSize = sizeof(AVIS_Rec[ubCh].hl.s2.strh) + sizeof(AVIS_Rec[ubCh].hl.s2.strf) + AVI_BYTE_OF_FCC;
	memcpy(&AVIS_Rec[ubCh].hl.s2.strh.fcc, "strh", 4);
	AVIS_Rec[ubCh].hl.s2.strh.cb = 0x38;
	memcpy(&AVIS_Rec[ubCh].hl.s2.strh.fccType, "auds", 4);
	if( ubAVI_AdoFrmType == ADO_ALAW)
	{
		AVIS_Rec[ubCh].hl.s2.strh.fccHandler = 0x1;
		AVIS_Rec[ubCh].hl.s2.strh.ulFlags = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.uwPriority = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.uwLanguage = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.ulInitialFrames = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.ulScale = 1;
		AVIS_Rec[ubCh].hl.s2.strh.ulRate = 48000;// 16000;
		AVIS_Rec[ubCh].hl.s2.strh.ulStart = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.ulLength = 0x00;
		AVIS_Rec[ubCh].hl.s2.strh.ulSuggestedBufferSize = 0x3000;
		AVIS_Rec[ubCh].hl.s2.strh.ulQuality = 0xffffffff;
		AVIS_Rec[ubCh].hl.s2.strh.ulSampleSize = 1;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.left = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.top = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.right = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.bottom = 0x0;
	}
	else
	{
		AVIS_Rec[ubCh].hl.s2.strh.fccHandler = 0x1;
		AVIS_Rec[ubCh].hl.s2.strh.ulFlags = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.uwPriority = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.uwLanguage = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.ulInitialFrames = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.ulScale = uwAVI_AdoSamplesPerBlock[ubCh];
		AVIS_Rec[ubCh].hl.s2.strh.ulRate = ulAVI_AdoSampleRate[ubCh];
		AVIS_Rec[ubCh].hl.s2.strh.ulStart = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.ulLength = 0x00;
		AVIS_Rec[ubCh].hl.s2.strh.ulSuggestedBufferSize = 0x3000;
		AVIS_Rec[ubCh].hl.s2.strh.ulQuality = 0xffffffff;
		AVIS_Rec[ubCh].hl.s2.strh.ulSampleSize = uwAVI_AdoBlockAlign[ubCh];
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.left = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.top = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.right = 0x0;
		AVIS_Rec[ubCh].hl.s2.strh.rcFrame.bottom = 0x0;
	}
	
	//strf
	memcpy(&AVIS_Rec[ubCh].hl.s2.strf, cbAVI_Strf2, sizeof(cbAVI_Strf2));
	if( ubAVI_AdoFrmType == ADO_ALAW)
	{
		AVIS_Rec[ubCh].hl.s2.strf.format.uwFormatTag = 6;
		AVIS_Rec[ubCh].hl.s2.strf.format.uwChannels = 1;
		AVIS_Rec[ubCh].hl.s2.strf.format.ulSamplesPerSec = 48000;// 16000;
		AVIS_Rec[ubCh].hl.s2.strf.format.ulAvgBytesPerSec = 48000;// 16000;
		AVIS_Rec[ubCh].hl.s2.strf.format.uwBlockAlign = 1;
		AVIS_Rec[ubCh].hl.s2.strf.format.uwBitsPerSample = 8;
	}
	else
	{
		AVIS_Rec[ubCh].hl.s2.strf.format.uwFormatTag = 2;
		AVIS_Rec[ubCh].hl.s2.strf.format.uwChannels = 1;
		AVIS_Rec[ubCh].hl.s2.strf.format.ulSamplesPerSec = ulAVI_AdoSampleRate[ubCh];
		AVIS_Rec[ubCh].hl.s2.strf.format.ulAvgBytesPerSec = (ulAVI_AdoSampleRate[ubCh]*uwAVI_AdoBlockAlign[ubCh])/uwAVI_AdoSamplesPerBlock[ubCh];
		AVIS_Rec[ubCh].hl.s2.strf.format.uwBlockAlign = uwAVI_AdoBlockAlign[ubCh];
		AVIS_Rec[ubCh].hl.s2.strf.format.uwBitsPerSample = 4;
		AVIS_Rec[ubCh].hl.s2.strf.format.uwSamplesPerBlock = uwAVI_AdoSamplesPerBlock[ubCh];
	}

	//INFO
	memcpy(&AVIS_Rec[ubCh].info.INFO.RIFF, "LIST", 4);
	memcpy(&AVIS_Rec[ubCh].info.INFO.ID, "INFO", 4);
	AVIS_Rec[ubCh].info.INFO.ulSize = sizeof(AVIS_Rec[ubCh].info.ISFT) + AVI_BYTE_OF_FCC + AVI_THUMBNAIL_SIZE;
	
	//ISFT
	memcpy(&AVIS_Rec[ubCh].info.ISFT.fcc, "ISFT", 4);
	AVIS_Rec[ubCh].info.ISFT.cb = sizeof(AVIS_Rec[ubCh].info.ISFT) - AVI_BYTE_OF_HEADER + AVI_THUMBNAIL_SIZE;
	memcpy(&AVIS_Rec[ubCh].info.ISFT.data, "Lavf55.2.100", 13);

	// MOVI
	memcpy(&AVI_RecMovi[ubCh].RIFF, "LIST", 4);
	memcpy(&AVI_RecMovi[ubCh].ID, "movi", 4);
	AVI_RecMovi[ubCh].ulSize = AVI_BYTE_OF_FCC;				//initial value	

	memcpy(&AVI_Idx1[ubCh]->fcc, "idx1", 4);

	slAVI_Length[ubCh] = sizeof(AVIS_Rec[ubCh]) + AVI_THUMBNAIL_SIZE + sizeof(AVICHUNK);
	slAVI_IndexNum[ubCh] = 0;

	//write avi header to FS
	FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(&AVIS_Rec[ubCh].RIFF), sizeof(AVIS_Rec[0].RIFF) + sizeof(AVIS_Rec[0].hl) + sizeof(AVIS_Rec[0].info));	// 500 Bytes
	FS_WriteFile((FS_SRC_NUM)ubCh, ulThumbnailAdr, AVI_THUMBNAIL_SIZE);		// 32.5K Bytes
	FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(&AVI_RecMovi[ubCh]), sizeof(AVICHUNK));	//12 Bytes
}

//------------------------------------------------------------------------------
//	AVI Close
//------------------------------------------------------------------------------
void AVI_Close(uint8_t ubCh)
{
	if (AVIS_Rec[ubCh].hl.avih.ulTotalFrames == 0)			//if have no any video frame, add a skip frame for Media Player can play normally.
	{
		printf("AVI_Close,Ch:%d No frame\n\r",ubCh);
		slAVI_WriteOneFrame(ubCh, AVI_STR_V1, AVI_V_SFRM, 0, ulAVI_GetSkipFrameSize(), NULL);
	}

	if (ulAVI_SfTotalSize[ubCh] != 0)								//if the last video frame is skip frame.
	{
		printf("AVI_Close,Fill SkipFrm Ch:%d Sz:0x%x\n\r",ubCh, ulAVI_SfTotalSize[ubCh]);
		FS_WriteFile((FS_SRC_NUM)ubCh, ulSkipFrameBuf, ulAVI_SfTotalSize[ubCh]);
		ulAVI_SfTotalSize[ubCh] = 0;
	}

	AVI_UpdateHeader(ubCh);							// Write Index table to file system

	FS_UpdateAviHeader1((FS_SRC_NUM)ubCh,(uint32_t)&AVIS_Rec[ubCh].RIFF, sizeof(AVIS_Rec[ubCh].RIFF) + sizeof(AVIS_Rec[ubCh].hl) + sizeof(AVIS_Rec[ubCh].info));
	FS_UpdateAviHeader2((FS_SRC_NUM)ubCh,(uint32_t)&AVI_RecMovi[ubCh], sizeof(AVICHUNK));
	
	FS_CloseFile((FS_SRC_NUM)ubCh);

	//---- clear parameter to avoid the value is invalid.-----
	//---- clear this value in AVI_CreateFile() --------------
	AVIS_Rec[ubCh].hl.s1.strh.ulLength = 0;
	AVIS_Rec[ubCh].hl.s2.strh.ulLength = 0;
	printf("AVI Close, Ch:%d TotalVFrm:%d FileSz:0x%x \n\r",	ubCh,
														AVIS_Rec[ubCh].hl.avih.ulTotalFrames,
														AVIS_Rec[ubCh].RIFF.ulSize+8);
}

uint16_t uwAVI_MovieLength(uint8_t ubCh)
{
	return ((AVIS_Rec[ubCh].hl.avih.ulTotalFrames * uwAVI_VFrameInterval[ubCh]) / 1000);
}

//------------------------------------------------------------------------------
//	AVI H.264 one Skip Frame Size
//------------------------------------------------------------------------------
uint32_t ulAVI_GetSkipFrameSize(void)
{
	return (sizeof(AVI_SKIP_FRAME_DATA) - AVI_BYTE_OF_HEADER);
}

//------------------------------------------------------------------------------
//	AVI Get Avi Record frame count
//	INPUT(stream): AVI_STR_V1, AVI_STR_A1
//------------------------------------------------------------------------------
uint32_t ulAVI_GetStrMaxFrmCnt(uint8_t ubStream)
{
	uint8_t i;
	uint32_t ulMaxFrmCnt = 0;
	
	if (ubStream == AVI_STR_V1)				//VIDEO
	{
		for (i=0; i < AVI_SRC_NUM; i++)
		{
			if (AVIS_Rec[i].hl.s1.strh.ulLength > ulMaxFrmCnt)//Get MAX. Frame Count
				ulMaxFrmCnt = AVIS_Rec[i].hl.s1.strh.ulLength;
		}
	}
	else if (ubStream == AVI_STR_A1)			//AUDIO
	{
		for (i=0; i < AVI_SRC_NUM; i++)
		{
			if (AVIS_Rec[i].hl.s2.strh.ulLength > ulMaxFrmCnt)//Get MAX. Frame Count
				ulMaxFrmCnt = AVIS_Rec[i].hl.s2.strh.ulLength;
		}
	}
	return ulMaxFrmCnt;
}

//------------------------------------------------------------------------------
//	Write AVI Info Data
//------------------------------------------------------------------------------
void AVI_WriteInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize)
{
	memcpy(&AVIS_Rec[ubCh].info.ISFT.data[ubOffset], ptr, ubSize);
}

//------------------------------------------------------------------------------
//	Read AVI Info Data
//------------------------------------------------------------------------------
void AVI_ReadInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize)
{
	memcpy(ptr, &AVIS_Rec[ubCh].info.ISFT.data[ubOffset], ubSize);
}
//END OF RECORD PART

// PLAY PART
//------------------------------------------------------------------------------
//	AVI Load Avi Header 
//------------------------------------------------------------------------------
void AVI_LoadAviHeader(uint8_t ubCh, uint32_t ulAddr)
{
	memcpy(&AVIS_Play[ubCh], (void *)ulAddr, sizeof(AVISTRUCT));

	ulAddr = ulAddr + sizeof(AVISTRUCT) + AVI_THUMBNAIL_SIZE;

	memcpy(&AVI_PlyMovi[ubCh], (void *)ulAddr, sizeof(AVICHUNK));
	
}

//------------------------------------------------------------------------------
//	AVI Get Avi Movi Length 
//------------------------------------------------------------------------------
uint32_t ulAVI_GetMoviLength(uint8_t ubCh)
{
	//return AVIS_Play[ubCh].movi.ulSize;
	return AVI_PlyMovi[ubCh].ulSize;
}

//------------------------------------------------------------------------------
//	AVI Get Avi movie length
//------------------------------------------------------------------------------
uint16_t uwAVI_GetFilmLength(uint8_t ubCh)
{
	uint32_t ulFramePerSec;
	
	ulFramePerSec = 1000/((AVIS_Play[ubCh].hl.avih.ulMicroSecPerFrame) / 1000);	// 1s / (convet us to ms)ms per frame
	
	return (uint16_t)(AVIS_Play[ubCh].hl.avih.ulTotalFrames / ulFramePerSec);
}

//------------------------------------------------------------------------------
//	AVI Get Avi frame count
//------------------------------------------------------------------------------
uint32_t ulAVI_GetMoviFrmCnt(uint8_t ubCh, uint8_t ubStreamType) 
{
	if (ubStreamType == AVI_STR_V1)
		return AVIS_Play[ubCh].hl.s1.strh.ulLength;
	else if (ubStreamType == AVI_STR_A1) 
		return AVIS_Play[ubCh].hl.s2.strh.ulLength;
	else 
		return 0;
}

//------------------------------------------------------------------------------
//	AVI Get Avi Scale
//------------------------------------------------------------------------------
uint32_t ulAVI_GetAviScale(uint8_t ubCh, uint8_t ubStreamType)
{
	if (ubStreamType == AVI_STR_V1)
		return AVIS_Play[ubCh].hl.s1.strh.ulScale;
	else if (ubStreamType == AVI_STR_A1)
		return AVIS_Play[ubCh].hl.s2.strh.ulScale;
	else
		return 0;
}

//------------------------------------------------------------------------------
//	AVI Get Avi Rate
//------------------------------------------------------------------------------
uint32_t ulAVI_GetAviRate(uint8_t ubCh, uint8_t ubStreamType)
{
	if (ubStreamType == AVI_STR_V1)
		return AVIS_Play[ubCh].hl.s1.strh.ulRate;
	else if (ubStreamType == AVI_STR_A1)
		return AVIS_Play[ubCh].hl.s2.strh.ulRate;
	else
		return 0;
}
//------------------------------------------------------------------------------
//	AVI Get video resolution
//------------------------------------------------------------------------------
uint8_t ubAVI_GetResolution(uint8_t ubCh, uint8_t ubStreamType) 
{
	uint32_t ulWidth;
	uint32_t ulHeight;

	if(ubStreamType == AVI_STR_A1 )
		return AVI_RES_NONE;
		
	ulWidth = AVIS_Play[ubCh].hl.s1.strf.info.ulWidth;
	ulHeight = AVIS_Play[ubCh].hl.s1.strf.info.ulHeight;

	if( (1900 < ulWidth && 1940 > ulWidth) && (1060 < ulHeight && 1100 > ulHeight))
		return AVI_RES_FHD;
	else if ( (1260 < ulWidth && 1300 > ulWidth) && (700 < ulHeight && 740 > ulHeight))
		return AVI_RES_HD;
	else if ( (750 < ulWidth && 850 > ulWidth) && (450 < ulHeight && 500 > ulHeight))
		return AVI_RES_WVGA;
	else if ( (620 < ulWidth && 660 > ulWidth) && (340 < ulHeight && 380 > ulHeight))
		return AVI_RES_VGA;
	else
		return AVI_RES_NONE;
}

//------------------------------------------------------------------------------
//	AVI Get ADO BlockAlign
//------------------------------------------------------------------------------
uint32_t ulAVI_GetAdoBlockAlign(uint8_t ubCh)
{
	return AVIS_Play[ubCh].hl.s2.strf.format.uwBlockAlign;
}

//------------------------------------------------------------------------------
//	AVI Get ADO Samples Per Block
//------------------------------------------------------------------------------
uint32_t ulAVI_GetAdoSamplesPerBlock(uint8_t ubCh)
{
	return AVIS_Play[ubCh].hl.s2.strf.format.uwSamplesPerBlock;
}

//------------------------------------------------------------------------------
//	AVI Get ADO Sample Rate
//------------------------------------------------------------------------------
uint32_t ulAVI_GetAdoSampleRate(uint8_t ubCh)
{
	return AVIS_Play[ubCh].hl.s2.strf.format.ulSamplesPerSec;
}

//------------------------------------------------------------------------------
//	AVI Get ADO Avg Bytes Per Sec  
//------------------------------------------------------------------------------
uint32_t ulAVI_GetAdoAvgBytesPerSec(uint8_t ubCh)
{
	return AVIS_Play[ubCh].hl.s2.strf.format.ulAvgBytesPerSec;
}

//------------------------------------------------------------------------------
//	Get AVI Audio Frame Type
//------------------------------------------------------------------------------
uint8_t ubAVI_AudioFrameTypeGet(void)
{
	return ubAVI_AdoFrmType;
}

#if 1
// H264
uint32_t ulH264_GetStreamOffset(void)
{
	return 1;
}

// CRC
uint32_t ulCrc16_Compute(uint32_t ulDramAddr,uint32_t ulNumOfByte, uint8_t ubWaitTime)
{
	return 1;
}
uint16_t uwCrc16_Get(void)
{
	return 1;
}
#endif



