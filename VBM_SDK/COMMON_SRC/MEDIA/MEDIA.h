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
	\brief		Media File format header file
	\author		Wales
	\version    0.2
	\date		2017/03/21
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#ifndef _MEDIA_H_
#define _MEDIA_H_
#include "_510PF.h"
#include "AVI.h"
#include "MP4.h" 
#include "REC_API.h"
//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define MEDIA_MSADPCM	(1)
#define MEDIA_ALAW		(6)

//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------
typedef enum _MediaSourceType
{
	MEDIA_SRC_1V0A,
	MEDIA_SRC_1V1A,
	MEDIA_SRC_2V1A,
	MEDIA_SRC_MAX,
}MEDIA_SOURCE_TYPE;

//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Get Media Version
\return	(no)
*/
uint16_t uwMedia_GetVersion(void);

void Media_SemaphoreCreate(void);

void Media_FileFormatConfigInit(void);

void Media_FileFormatSet(uint8_t ubFmt);
uint8_t ubMedia_FileFormatGet(void);

void Media_FileFormatConfigure(REC_FILEFORMAT *FileFormat);


//------------------------------------------------------------------------
/*!
\brief	Set configured source number in record mode.
\param	ubSrcNum Total source number
\return	(no)
*/
void Media_ConfigedSrcNum(uint8_t ubSrcNum);
//------------------------------------------------------------------------
/*!
\brief	Calculate requested memory buffer size
\return	Memory size
*/
uint32_t ulMedia_GetBufSz(void);

//------------------------------------------------------------------------
/*!
\brief	Media file format initial
\param	ulStartAdr Memory start address
\return	(no)
*/
void Media_Init(uint32_t ulStartAdr);

//------------------------------------------------------------------------
/*!
\brief	Only for MP4. Set source type for media file format. 1video,1video1audio,2video1audio.
\param	ubCh Source Channel
\param	ubSourceType Source Type
\return	(no)
*/
void Media_SetSourceType(uint8_t ubCh, uint8_t ubSourceType);

//------------------------------------------------------------------------
/*!
\brief	Set Video1 frame information
\param	ubCh Source Channel
\param	ulHSize Horizontal resolution
\param	ulVSize Vertical resolution
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)*/
void Media_Vdo1SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval);

//------------------------------------------------------------------------
/*!
\brief	Only for MP4. Set Video2 frame information
\param	ubCh Source Channel
\param	ulHSize Horizontal resolution
\param	ulVSize Vertical resolution
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void Media_Vdo2SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval);

//------------------------------------------------------------------------
/*!
\brief	Set Audio frame information
\param	ubCh Source Channel
\param	ulSampleRate Sample rate
\param	uwBlockAlign Block Alignment
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void Media_AdoSetFormat(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval);

//------------------------------------------------------------------------------
/*!
\brief Get memory size which is not increased by source number
\return	memory size
*/
uint32_t ulMedia_GetFixedMemorySize(void);

//------------------------------------------------------------------------------
/*!
\brief 	Set support max video fps, audio fps, and record time. used to caculate memory size.
\param	ubVFPS supported max video fps
\param	ubAFPS supported max audio fps
\param	ulMinute supported max record time
\return	(no)
*/
void Media_SetRecMaxTime(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute);

//------------------------------------------------------------------------
/*!
\brief	Write video or audio frame data
\param	ubCh Source Channel
\param	ubStreamType Stream Type VIDEO, AUDIO
\param	ulAddr Start address of one frame
\param	ulSize Frame size
\param	ulDesID Use for MP4, fill null here
\return	Check MP4.h or AVI.h
*/
int32_t slMedia_WriteOneFrame(uint8_t ubCh, uint8_t ubStreamType, uint16_t uwFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID);

//------------------------------------------------------------------------
/*!
\brief	Create Media file format stucture
\param	ubCh Source Channel
\return	(no)
*/
void Media_CreateFile(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Close Media file format
\param	ubCh Source Channel
\return	(no)
*/
void Media_Close(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get movie time length
\param	ubCh Source Channel
\return	Unit second
*/
uint16_t uwMedia_MovieLength(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get skip Frame Size
\return	Frame Size
*/
uint32_t ulMedia_GetSkipFrameSize(void);

//------------------------------------------------------------------------
/*!
\brief	Get frame numbers by source type, AVI only
\param	stream Stream Type
\return	Frame numbers
*/
uint32_t ulMedia_GetStrMaxFrmCnt(uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Only for AVI. Load AVI Header from memory to Header structure, AVI only
\param	ubCh Source Channel
\param	ulAddr Input start address
\return	(no)
*/
void Media_LoadAviHeader(uint8_t ubCh, uint32_t ulAddr);

//------------------------------------------------------------------------
/*!
\brief	Get movie data size, AVI only
\param	ubCh Source Channel
\return	Data size
*/
uint32_t ulMedia_GetMoviLength(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get movie time length, AVI only
\param	ubCh Source Channel
\return	Time length
*/
uint16_t uwMedia_GetFilmLength(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get video or audio frame number
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame number
*/
uint32_t ulMedia_GetMoviFrmCnt(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get video key frame number, MP4 only
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame number
*/
uint32_t ulMedia_GetTotalVideoIFrame(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get video frame resolution
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame number
*/
uint8_t ubMedia_GetResolution(uint8_t ubCh, uint8_t ubStreamType);
//------------------------------------------------------------------------
/*!
\brief	Get video or audio frame Scale
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	frame Scale, fps = rate/scale
*/
uint32_t ulMedia_GetScale(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get video or audio frame rate
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	frame rate, fps = rate/scale
*/
uint32_t ulMedia_GetRate(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get audio block alignment
\param	ubCh Source Channel
\return	block alignment
*/
uint32_t ulMedia_GetAdoBlockAlign(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get audio samples per block
\param	ubCh Source Channel
\return	samples
*/
uint32_t ulMedia_GetAdoSamplesPerBlock(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get audio samples rate
\param	ubCh Source Channel
\return	samples rate
*/
uint32_t ulMedia_GetAdoSampleRate(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get audio average bytes per second
\param	ubCh Source Channel
\return	average byte numbers
*/
uint32_t ulMedia_GetAdoAvgBytesPerSec(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Write information data to file format
\param	ubCh Source Channel
\param	ubOffset Offset position of information structure
\param	ptr A pointer point to input data
\param	ubSize write size
\return	(no)
*/
void ulMedia_WriteInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize);

//------------------------------------------------------------------------
/*!
\brief	Assignment each sub atoms in mp4 file, MP4 only
\param	ubCh Source Channel
\param	ulAdr moov start address
\param	ulSize fill moov size without thumbnail size
\return	(no)
*/
void Media_AssignmentStructure(uint8_t ubCh, uint32_t ulAdr, uint32_t ulSize);

//------------------------------------------------------------------------------
/*!
\brief	Get delay time between current file and first created file, MP4 only
\param	ubCh Source Channel
\return	Delay time. Unit=1ms
*/
uint32_t ulMedia_GetStreamDelayTime(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get stream time scale, MP4 only
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Time scale. No unit
*/
uint32_t ulMedia_GetStreamTimeScale(uint8_t ubCh, uint8_t ubStreamType);
//------------------------------------------------------------------------
/*!
\brief	Read information data to file format, AVI only
\param	ubCh Source Channel
\param	ubOffset Offset position of information structure
\param	ptr A pointer point to output data
\param	ubSize write size
\return	(no)
*/
void Media_ReadInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize);

//------------------------------------------------------------------------
/*!
\brief	Get Audio Frame Type PCM,ADPCM,ALAW
\return	Frame Type
*/
uint8_t ubMedia_AudioFrameTypeGet(void);

//------------------------------------------------------------------------------
/*!
\brief	Get frame information, frame type, frame size from real raw data. MP4 only
\param	ubCh Source Channel
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1
*/
uint32_t ulMedia_ParseMdatFrameInfo(uint8_t ubCh,PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	For Jump used. Reload stbl infromation. MP4 only
\param	ubCh Source Channel
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1
*/
uint32_t ulMedia_JumpReload(uint8_t ubCh, PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	Parse video stbl data structure. To store I frame information, address offset, frame index, chunk index.MP4 only
\param	ubCh Source Channel
\param	ulAdr Inpur memory start address
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1 success.
*/
uint32_t ulMedia_ParseVideoIFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	Parse audio stbl data structure. To store frame information, address offset,chunk index.MP4 only
\param	ubCh Source Channel
\param	ulAdr Inpur memory start address
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1 success.
*/
uint32_t ulMedia_ParseAudioFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	Get video frame size,MP4 only
\param	ubCh Source Channel
\param	ulFrm Frame index
\return	frame size
*/
uint32_t ulMedia_GetVideoFrameSize( uint8_t ubCh , uint32_t ulFrm);
#endif

