/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       Media.c
	\brief      Media File format
	\author     Wales
	\version    0.2
	\date       2017/03/21
	\copyright  Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MEDIA.h"
#include "REC_API.h"
#include "PLY_API.h"
#include "REC.h"

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
uint16_t uwMedia_GetVersion(void)
{
	uint16_t uwVer;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			uwVer = uwAVI_GetVersion();
		break;
		case REC_FILE_MP4:
			uwVer = uwMP4_GetVersion();
		break;
	}
	return uwVer;
}

void Media_SemaphoreCreate(void)
{
	AVI_SemaphoreCreate();
	MP4_SemaphoreCreate();
	return;
}

// REC
void Media_FileFormatConfigInit(void)
{
	REC_FILEFORMAT sRecFileFormat;

	// Source 1
	memset(&sRecFileFormat, 0x00, sizeof(REC_FILEFORMAT));
	sRecFileFormat.ubCh = 0;
	sRecFileFormat.ubConfiged = REC_SRC_CONFIGED;
	sRecFileFormat.ubSourceType = MEDIA_SRC_1V1A;
	sRecFileFormat.uwVFrmInterval = 33;
	sRecFileFormat.ulHoSize1 = 1280;
	sRecFileFormat.ulVoSize1 = 720;
	sRecFileFormat.ulHoSize2 = 1280;
	sRecFileFormat.ulVoSize2 = 720;

#if 1		// PCM for AVI
	sRecFileFormat.uwAFrmInterval = 625;
	sRecFileFormat.ulSampleRate =  8000;
	sRecFileFormat.uwBlockAlign = 256;
#else	// AAC for MP4
	sRecFileFormat.uwAFrmInterval = 1250;	// 1250 // Key Word: MP4 AAC_TEST
	sRecFileFormat.ulSampleRate =  8192;
	sRecFileFormat.uwBlockAlign = 256;
#endif
	REC_FileFormatConfigure(&sRecFileFormat);

	// Source 2
	memset(&sRecFileFormat, 0x00, sizeof(REC_FILEFORMAT));
	sRecFileFormat.ubCh = 1;
	sRecFileFormat.ubConfiged = REC_SRC_UNCONFIGED;//REC_SRC_CONFIGED;
	sRecFileFormat.ubSourceType = MEDIA_SRC_1V1A;
	sRecFileFormat.uwVFrmInterval = 33;
	sRecFileFormat.ulHoSize1 = 1280;
	sRecFileFormat.ulVoSize1 = 720;
	sRecFileFormat.ulHoSize2 = 1280;
	sRecFileFormat.ulVoSize2 = 720;
#if 1		// PCM for AVI
	sRecFileFormat.uwAFrmInterval = 625;
	sRecFileFormat.ulSampleRate =  8000;
	sRecFileFormat.uwBlockAlign = 256;
#else	// AAC for MP4
	sRecFileFormat.uwAFrmInterval = 1250;	// 1250 // Key Word: MP4 AAC_TEST
	sRecFileFormat.ulSampleRate =  8192;
	sRecFileFormat.uwBlockAlign = 256;
#endif
	REC_FileFormatConfigure(&sRecFileFormat);

	// Source 3
	memset(&sRecFileFormat, 0x00, sizeof(REC_FILEFORMAT));
	sRecFileFormat.ubCh = 2;
	sRecFileFormat.ubConfiged = REC_SRC_UNCONFIGED;//REC_SRC_CONFIGED;
	sRecFileFormat.ubSourceType = MEDIA_SRC_1V1A;
	sRecFileFormat.uwVFrmInterval = 33;
	sRecFileFormat.ulHoSize1 = 1280;
	sRecFileFormat.ulVoSize1 = 720;
	sRecFileFormat.ulHoSize2 = 1280;
	sRecFileFormat.ulVoSize2 = 720;
#if 1		// PCM for AVI
	sRecFileFormat.uwAFrmInterval = 625;
	sRecFileFormat.ulSampleRate =  8000;
	sRecFileFormat.uwBlockAlign = 256;
#else	// AAC for MP4
	sRecFileFormat.uwAFrmInterval = 1250;	// 1250 // Key Word: MP4 AAC_TEST
	sRecFileFormat.ulSampleRate =  8192;
	sRecFileFormat.uwBlockAlign = 256;
#endif
	REC_FileFormatConfigure(&sRecFileFormat);	

	// Source 4
	memset(&sRecFileFormat, 0x00, sizeof(REC_FILEFORMAT));
	sRecFileFormat.ubCh = 3;
	sRecFileFormat.ubConfiged = REC_SRC_UNCONFIGED;//REC_SRC_CONFIGED;
	sRecFileFormat.ubSourceType = MEDIA_SRC_1V1A;
	sRecFileFormat.uwVFrmInterval = 33;
	sRecFileFormat.ulHoSize1 = 1280;
	sRecFileFormat.ulVoSize1 = 720;
	sRecFileFormat.ulHoSize2 = 1280;
	sRecFileFormat.ulVoSize2 = 720;
#if 1		// PCM for AVI
	sRecFileFormat.uwAFrmInterval = 625;
	sRecFileFormat.ulSampleRate =  8000;
	sRecFileFormat.uwBlockAlign = 256;
#else	// AAC for MP4
	sRecFileFormat.uwAFrmInterval = 1250;	// 1250 // Key Word: MP4 AAC_TEST
	sRecFileFormat.ulSampleRate =  8192;
	sRecFileFormat.uwBlockAlign = 256;
#endif
	REC_FileFormatConfigure(&sRecFileFormat);
}

void Media_FileFormatSet(uint8_t ubFmt)
{
	vREC_FileFormatSet(ubFmt);
	printf("MaxRECTime %d\r\n",ulREC_ModeSet(0x1400000));
	vREC_AdoEnableSet(1);
	Media_Init(0x154A000);
	printf("ulMedia_GetBufSz:0x%x\r\n", ulMedia_GetBufSz());
	PLY_PinPonBuffInit(0x5600000);
	printf("ulPLY_GetBufSz:0x%x\r\n", ulPLY_GetBufSz());
}

uint8_t ubMedia_FileFormatGet(void)
{
	return ubREC_FileFormatGet();
}

void Media_FileFormatConfigure(REC_FILEFORMAT *FileFormat)
{
	REC_FileFormatConfigure(FileFormat);
	printf("MaxRECTime %d\r\n",ulREC_ModeSet(0x1400000));
	vREC_AdoEnableSet(1);
	Media_Init(0x154A000);
	printf("ulMedia_GetBufSz:0x%x\r\n", ulMedia_GetBufSz());
	PLY_PinPonBuffInit(0x5600000);
	printf("ulPLY_GetBufSz:0x%x\r\n", ulPLY_GetBufSz());

	ubREC_PrecordTime(30,15,0x64000,(0x200000*4));
	REC_PreMemInit(0x5600000);
	printf("ulREC_PreGetBufSz:0x%x\r\n", ulREC_PreGetBufSz());
	ubREC_PreStart();	
}

void Media_ConfigedSrcNum(uint8_t ubSrcNum)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_SetConfigedSrcNum( ubSrcNum);
		break;
		case REC_FILE_MP4:
			MP4_SetConfigedSrcNum( ubSrcNum);
		break;	
	}
	return;
}

uint32_t ulMedia_GetBufSz(void)
{
	uint32_t ulSize;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulSize = ulAVI_GetBufSz();
		break;
		case REC_FILE_MP4:
			ulSize = ulMP4_GetBufSz();
		break;	
	}
	return ulSize;
}

void Media_Init(uint32_t ulStartAdr)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_Init(ulStartAdr);
		break;
		case REC_FILE_MP4:
			MP4_Init(ulStartAdr);
		break;
	}
	return;
}

void Media_SetSourceType(uint8_t ubCh, uint8_t ubSourceType)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_MP4:
			MP4_SetSourceType( ubCh,  ubSourceType);
		break;	
	}
	return;
}

void Media_Vdo1SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_Vdo1SetFormat(ubCh,ulHSize,ulVSize,uwFrmInterval);
		break;
		case REC_FILE_MP4:
			MP4_Vdo1SetFormat(ubCh,ulHSize,ulVSize,uwFrmInterval);
		break;
	}
	return;
}

void Media_Vdo2SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_MP4:
			MP4_Vdo2SetFormat(ubCh,ulHSize,ulVSize,uwFrmInterval);
		break;
	}
	return;
}

void Media_AdoSetFormat(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_AdoSetFormat(ubCh,ulSampleRate,uwBlockAlign,uwFrmInterval);
		break;
		case REC_FILE_MP4:
			MP4_AdoSetFormat(ubCh,ulSampleRate,uwBlockAlign,uwFrmInterval);
		break;
	}
	return;
}

uint32_t ulMedia_GetFixedMemorySize(void)
{
	uint32_t ulSize;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulSize = ulAVI_GetFixedMemorySize();
		break;
		case REC_FILE_MP4:
			ulSize = ulMP4_GetFixedMemorySize();
		break;
	}
	return ulSize;
}

void Media_SetRecMaxTime(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_SetRecMaxTime(ubVFPS, ubAFPS, ulMinute);
		break;
		case REC_FILE_MP4:
			MP4_SetRecMaxTime(ubVFPS, ubAFPS, ulMinute);
		break;
	}
	return;
}

int32_t slMedia_WriteOneFrame(uint8_t ubCh, uint8_t ubStreamType, uint16_t uwFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID)
{
	int32_t slRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			slRtn = slAVI_WriteOneFrame(ubCh, ubStreamType, uwFrameType, ulAddr, ulSize, NULL);
		break;
		case REC_FILE_MP4:
			slRtn = slMP4_WriteOneFrame(ubCh, ubStreamType, uwFrameType, ulAddr, ulSize, ulDesID);
		break;
	}
	return slRtn;
}

void Media_CreateFile(uint8_t ubCh)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_CreateFile(ubCh);
		break;
		case REC_FILE_MP4:
			MP4_CreateFile(ubCh);
		break;
	}
	return;
}

void Media_Close(uint8_t ubCh)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_Close(ubCh);
		break;
		case REC_FILE_MP4:
			MP4_Close(ubCh);
		break;
	}
	return;
}

uint16_t uwMedia_MovieLength(uint8_t ubCh)
{
	uint16_t uwLen;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			uwLen = uwAVI_MovieLength(ubCh);
		break;
		case REC_FILE_MP4:
			uwLen = uwMP4_MovieLength(ubCh);
		break;
	}
	return uwLen;
}

uint32_t ulMedia_GetSkipFrameSize(void)
{
	uint32_t ulSize;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulSize = ulAVI_GetSkipFrameSize();
		break;
		case REC_FILE_MP4:
			ulSize = ulMP4_GetSkipFrameSize();
		break;
	}
	return ulSize;
}

uint32_t ulMedia_GetStrMaxFrmCnt(uint8_t ubStreamType)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = ulAVI_GetStrMaxFrmCnt(ubStreamType);
		break;
		case REC_FILE_MP4:
			ulRtn = 1;
		break;
	}
	return ulRtn;
}

void ulMedia_WriteInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_WriteInfoData(ubCh, ubOffset, ptr, ubSize);
		break;
		case REC_FILE_MP4:
			MP4_WriteInfoData(ubCh, ubOffset, ptr, ubSize);
		break;
	}
	return;
}

void Media_AssignmentStructure(uint8_t ubCh, uint32_t ulAdr, uint32_t ulSize)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_MP4:
			MP4_AssignmentStructure(ubCh, ulAdr, ulSize);
		break;
	}
	return;
}

uint32_t ulMedia_GetStreamDelayTime(uint8_t ubCh)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 0;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_GetStreamDelayTime(ubCh);
		break;
	}
	return ulRtn;
}

uint32_t ulMedia_GetStreamTimeScale(uint8_t ubCh, uint8_t ubStreamType)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 0;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_GetStreamTimeScale(ubCh, ubStreamType);
		break;
	}
	return ulRtn;
}

void Media_ReadInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_ReadInfoData(ubCh, ubOffset, ptr, ubSize);
		break;
	}
	return;
}

uint8_t ubMedia_AudioFrameTypeGet(void)
{
	uint8_t ubRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ubRtn = ubAVI_AudioFrameTypeGet();
		break;
		case REC_FILE_MP4:
			ubRtn  = 1;
		break;
	}
	return ubRtn;
}

// PLAY
void Media_LoadAviHeader(uint8_t ubCh, uint32_t ulAddr)
{
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			AVI_LoadAviHeader(ubCh, ulAddr);
		break;
	}
	return;
}

uint32_t ulMedia_GetMoviLength(uint8_t ubCh)
{
	uint32_t ulLen;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulLen =  ulAVI_GetMoviLength(ubCh);
		break;
		case REC_FILE_MP4:
			ulLen  = 1;
		break;
	}
	return ulLen;
}

uint16_t uwMedia_GetFilmLength(uint8_t ubCh)
{
	uint32_t uwLen;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			uwLen =  uwAVI_GetFilmLength(ubCh);
		break;
		case REC_FILE_MP4:
			uwLen = 1;
		break;
	}
	return uwLen;
}

uint32_t ulMedia_GetMoviFrmCnt(uint8_t ubCh, uint8_t ubStreamType) 
{
	uint32_t ulNum;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulNum = ulAVI_GetMoviFrmCnt(ubCh, ubStreamType);
		break;
		case REC_FILE_MP4:
			ulNum = ulMP4_GetTotalFrameNumber(ubCh,ubStreamType);
		break;
	}
	return ulNum;
}

uint32_t ulMedia_GetTotalVideoIFrame(uint8_t ubCh, uint8_t ubStreamType) 
{
	uint32_t ulNum;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulNum = 1;
		break;
		case REC_FILE_MP4:
			ulNum = ulMP4_GetTotalVideoIFrame(ubCh, ubStreamType);
		break;
	}
	return ulNum;
}
	
uint8_t ubMedia_GetResolution(uint8_t ubCh, uint8_t ubStreamType) 
{
	uint8_t ubRes;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ubRes = ubAVI_GetResolution(ubCh,ubStreamType);
		break;
		case REC_FILE_MP4:
			ubRes = ubMP4_GetResolution(ubCh,ubStreamType);
		break;
	}
	return ubRes;
}

uint32_t ulMedia_GetScale(uint8_t ubCh, uint8_t ubStreamType)
{
	uint32_t ulValue;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulValue = ulAVI_GetAviScale(ubCh, ubStreamType);
		break;
		case REC_FILE_MP4:
			ulValue = ulMP4_GetFrameDuration(ubCh, ubStreamType);
		break;
	}
	return ulValue;
}

uint32_t ulMedia_GetRate(uint8_t ubCh, uint8_t ubStreamType)
{
	uint32_t ulRate;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRate = ulAVI_GetAviRate(ubCh, ubStreamType);
		break;
		case REC_FILE_MP4:
			ulRate = 1;
		break;
	}
	return ulRate;
}

uint32_t ulMedia_GetAdoBlockAlign(uint8_t ubCh)
{
	uint32_t ulAlign;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulAlign = ulAVI_GetAdoBlockAlign(ubCh);
		break;
		case REC_FILE_MP4:
			ulAlign = 1;
		break;
	}
	return ulAlign;
}

uint32_t ulMedia_GetAdoSamplesPerBlock(uint8_t ubCh)
{
	uint32_t ulSpB;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulSpB = ulAVI_GetAdoSamplesPerBlock(ubCh);
		break;
		case REC_FILE_MP4:
			ulSpB = 1;
		break;
	}
	return ulSpB;
}

uint32_t ulMedia_GetAdoSampleRate(uint8_t ubCh)
{
	uint32_t ulRate;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRate = ulAVI_GetAdoSampleRate(ubCh);
		break;
		case REC_FILE_MP4:
			ulRate = ulMP4_GetAudioSampleRate(ubCh);
		break;
	}
	return ulRate;
}

uint32_t ulMedia_GetAdoAvgBytesPerSec(uint8_t ubCh)
{
	uint32_t ulBps;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulBps = ulAVI_GetAdoAvgBytesPerSec(ubCh);
		break;
		case REC_FILE_MP4:
			ulBps = ulMP4_GetAudioAvgBR(ubCh);
		break;
	}
	return ulBps;
}

uint32_t ulMedia_ParseMdatFrameInfo(uint8_t ubCh,PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 1;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_ParseMdatFrameInfo(ubCh, PlyChkCtrl);
		break;
	}
	return ulRtn;
}

uint32_t ulMedia_JumpReload(uint8_t ubCh, PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 1;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_JumpReload(ubCh, PlyChkCtrl);
		break;
	}
	return ulRtn;
}

uint32_t ulMedia_ParseVideoIFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 1;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_ParseVideoIFrameTable(ubCh, ulAdr, PlyChkCtrl);
		break;
	}
	return ulRtn;
}

uint32_t ulMedia_ParseAudioFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 1;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_ParseAudioFrameTable(ubCh, ulAdr, PlyChkCtrl);
		break;
	}
	return ulRtn;
}

uint32_t ulMedia_GetVideoFrameSize( uint8_t ubCh , uint32_t ulFrm)
{
	uint32_t ulRtn;
	switch(ubREC_FileFormatGet())
	{
		case REC_FILE_AVI:
			ulRtn = 1;
		break;
		case REC_FILE_MP4:
			ulRtn = ulMP4_GetVideoFrameSize( ubCh , ulFrm);
		break;
	}
	return ulRtn;
}

