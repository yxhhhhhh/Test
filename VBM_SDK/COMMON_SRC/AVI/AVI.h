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
	\brief      AVI File format Head file
	\author     Wales
	\version    0.2
	\date       2017/03/21
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef _AVI_H_
#define _AVI_H_

#include "_510PF.h"

//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define AVI_MAJORVER            (0) // Major version = 0
#define AVI_MINORVER            (2) // Minor version = 2

// Audio Frame Type
#define ADO_MSADPCM             (1)		
#define ADO_ALAW                (6)
#define ADO_FRMTYPE             (ADO_MSADPCM)

#define AVI_THUMBNAIL_SIZE	    (0x10200)    //32.5K=(0x8200);64,5K=(0x10200)

//!< AVI Main header flag definition
#define AVI_HASINDEX        	(0x00000010)    //!< Had idx1 chunk
#define AVI_MUSTUSEINDEX    	(0x00000020)    //!< Must use idx1 chunk to determine order
#define AVI_ISINTERLEAVED   	(0x00000100)    //!< AVI file is interleaved
#define AVI_TRUSTCKTYPE     	(0x00000800) 	//!< Had idx1 chunk
#define AVI_WASCAPTUREFILE  	(0x00010000)    //!< Specially allocated used for capturing real time video
#define AVI_COPYRIGHTED     	(0x00020000) 	//!< Contains copyrighted data

//!< AVI Stream header flag definition
#define AVISF_DISABLED			(0x00000001)	//!< Indicates this stream should not be enabled by default
#define AVISF_VIDEO_PALCHANGES  (0x00010000)	//!< Indicates this video stream contains palette changes. This flag warns the playback software that it will need to animate the palette

#define AVI_BYTE_OF_FCC			(4)
#define AVI_BYTE_OF_CB			(4)
#define AVI_BYTE_OF_HEADER		(AVI_BYTE_OF_FCC+AVI_BYTE_OF_CB)

#define AVI_INDEX_1MINSZ(vfps,afps)	(( vfps + afps )*960+8)	// ( vfps + afps )*16*60+8
//------------------------------------------------------------------------------
//	Lable offset in Atom
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------
typedef uint32_t FOURCC;

enum 
{
	AVI_READ_NYET = 0,
	AVI_READ_OK = 1,
};

//------------------------------------------------------------------------------
//! brief AVI Main Header Structure
typedef struct _avimainheader {
	FOURCC fcc;						//!< Must Set 'avih' 
	uint32_t cb;					//!< size of header structure,but not include fcc and cb
	uint32_t ulMicroSecPerFrame;	//!< Frame interval time
	uint32_t ulMaxBytesPerSec;		//!< Max data rate in the avi file
	uint32_t ulPaddingGranularity;	//!< Specifies the alignment for data, in bytes. Pad the data to multiples of this value
	uint32_t ulFlags;				//!< Contains a bitwise combination of zero or more of the following flags
	uint32_t ulTotalFrames;			//!< Specifies the total number of frames of data in the file
	uint32_t ulInitialFrames;		//!< Specifies the initial frame for interleaved files. Noninterleaved files should specify zero
	uint32_t ulStreams;				//!< Specifies the number of streams in the file. For example, a file with audio and video has two streams
	uint32_t ulSuggestedBufferSize;	//!< Specifies the suggested buffer size for reading the file
	uint32_t ulWidth;				//!< Specifies the width of the AVI file in pixels
	uint32_t ulHeight;				//!< Specifies the height of the AVI file in pixels
	uint32_t ulReserved[4];			//!< Reserved. Set this array to zero
} AVIMAINHEADER;
//------------------------------------------------------------------------------
//! brief AVI Stream Header Structure
typedef struct _avistreamheader {
	FOURCC fcc;						//!< Must Set 'strh' 
	uint32_t cb;					//!< size of header structure,but not include fcc and cb
	FOURCC fccType;					//!< Contains a FOURCC that specifies the type of the data contained in the stream.'auds' or 'vids' or 'mids' or 'txts'
	FOURCC fccHandler;				//!< Optionally, contains a FOURCC that identifies a specific data handler. The data handler is the preferred handler for the stream
	uint32_t ulFlags;				//!< Contains any flags for the data stream
	uint16_t uwPriority;			//!< Priority of a stream type. In a file with multiple audio streams, the one with the highest priority might be the default stream
	uint16_t uwLanguage;			//!< Set language
	uint32_t ulInitialFrames;		//!< How far audio data is skewed ahead of the video frames in interleaved files
	uint32_t ulScale;				//!< Used with dwRate to specify the time scale that this stream will use.Dividing dwRate by dwScale gives the number of samples per second
	uint32_t ulRate;				//!< For video streams, this rate should be the frame rate
									//!< For audio streams, this rate should correspond to the time needed for nBlockAlign bytes of audio, which for PCM audio simply reduces to the sample rate.
	uint32_t ulStart;				//!< The starting time of the AVI file. The units are defined by the dwRate and dwScale members in the main file header
									//!< Usually, this is zero, but it can specify a delay time for a stream that does not start concurrently with the file
	uint32_t ulLength;				//!< The length of this stream. The units are defined by the dwRate and dwScale members of the stream's header
	uint32_t ulSuggestedBufferSize;	//!< How large a buffer should be used to read this stream. 
									//!< Typically, this contains a value corresponding to the largest chunk present in the stream. 
									//!< Using the correct buffer size makes playback more efficient. Use zero if you do not know the correct buffer size.
	uint32_t ulQuality;				//!< An indicator of the quality of the data in the stream
	uint32_t ulSampleSize;			//!< The size of a single sample of data
	struct {
		int16_t left;
		int16_t top;
		int16_t right;
		int16_t bottom;
	}  rcFrame;						//!< The destination rectangle for a text or video stream within the movie rectangle specified by the dwWidth and dwHeight members of the AVI main header structure.
} AVISTREAMHEADER;
//------------------------------------------------------------------------------
//! brief AVI Index Structure
typedef struct _avioldindex {
	FOURCC fcc;						//!< Must Set 'idx1' 
	uint32_t cb;					//!< size of header structure,but not include fcc and cb
	struct _avioldindex_entry {
		uint32_t ulChunkId;			// 表徵本資料塊的四字元碼
		uint32_t ulFlags;			// 說明本資料塊是不是關鍵幀、是不是‘rec ’列表等資訊
		uint32_t ulOffset;			// 本資料塊在文件中的偏移量
		uint32_t ulSize;			// 本資料塊的大小
	} aIndex[1];					// 這是一個陣列！為每個媒體資料塊都定義一個索引資訊
} AVIOLDINDEX;
//------------------------------------------------------------------------------
//! brief AVI stream1(video) format Structure
typedef struct {
	FOURCC fcc;
	uint32_t cb;
	struct {
		uint32_t ulSize;
		uint32_t ulWidth;
		uint32_t ulHeight;
		uint16_t uwPlanes;
		uint16_t uwBitCount;
		uint32_t ulCompression;
		uint32_t ulSizeImage;
		uint32_t ulXPelsPerMeter;
		uint32_t ulYPelsPerMeter;
		uint32_t ulClrUsed;
		uint8_t exdata[28];
	}info;
} AVISTREAMFORMAT1;
//------------------------------------------------------------------------------
//! brief AVI stream2(Audio) format Structure
typedef struct {
	FOURCC fcc;
	uint32_t cb;
	struct {
		uint16_t uwFormatTag;
		uint16_t uwChannels;
		uint32_t ulSamplesPerSec;
		uint32_t ulAvgBytesPerSec;
		uint16_t uwBlockAlign;
		uint16_t uwBitsPerSample;
		uint16_t cbSize;
		uint16_t uwSamplesPerBlock;
		uint8_t exdata[32];
	}format;
} AVISTREAMFORMAT2;
/*
Code　　　　Description 
0 (0x0000) Unknown 
1 (0x0001) PCM/uncompressed 
2 (0x0002) Microsoft ADPCM 
6 (0x0006) ITU G.711 a-law 
7 (0x0007) ITU G.711 Aμ-law 
17 (0x0011) IMA ADPCM 
20 (0x0016) ITU G.723 ADPCM (Yamaha) 
49 (0x0031) GSM 6.10 
64 (0x0040) ITU G.721 ADPCM 
80 (0x0050) MPEG 
65,536 (0xFFFF) Experimental
*/
//------------------------------------------------------------------------------
//! brief  AVI Information Structure
typedef struct {
	FOURCC fcc;
	uint32_t cb;
	uint8_t data[108];				//stuff to 512 byte alignment for update header of FS.
} AVIISFT;

//! brief AVI Chunk Structure
typedef struct {
	FOURCC RIFF;
	uint32_t ulSize;
	FOURCC ID;
} AVICHUNK;

//! brief AVI File Format Structure
typedef struct {
	AVICHUNK RIFF;
	struct{
		AVICHUNK hdrl;				//LIST
		AVIMAINHEADER avih;
		struct{
			AVICHUNK strl;			//LIST
			AVISTREAMHEADER strh;
			AVISTREAMFORMAT1 strf;
		} s1;
		struct{
			AVICHUNK strl;			//LIST
			AVISTREAMHEADER strh;
			AVISTREAMFORMAT2 strf;
		} s2;
	} hl;
	struct{
		AVICHUNK INFO;				//LIST
		AVIISFT ISFT;
	} info;
//	AVICHUNK movi;					//LIST
//	AVIOLDINDEX idx1;
} AVISTRUCT;
//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Get MP4 Version
\return	(no)
*/
uint16_t uwAVI_GetVersion(void);

//------------------------------------------------------------------------
/*!
\brief	Create write frame samaphore
\return	(no)
*/
void AVI_SemaphoreCreate(void);

//------------------------------------------------------------------------
/*!
\brief	Set configured source number in record mode.
\param	ubSrcNum Total source number
\return	(no)
*/
void AVI_SetConfigedSrcNum(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief	Calculate requested memory buffer size
\return	Memory size
*/
uint32_t ulAVI_GetBufSz(void);

//------------------------------------------------------------------------
/*!
\brief	AVI file format initial
\param	ulStartAdr Memory start address
\return	(no)
*/
void AVI_Init(uint32_t ulStartAdr);

//------------------------------------------------------------------------
/*!
\brief	Get Audio Frame Type PCM,ADPCM,ALAW
\return	Frame Type
*/
uint8_t ubAVI_AudioFrameTypeGet(void);

//RECORD
//------------------------------------------------------------------------
/*!
\brief	Create AVI file format stucture
\param	ubCh Source Channel
\return	(no)
*/
void AVI_CreateFile(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Write video or audio frame data
\param	ubCh Source Channel
\param	ubStreamType Stream Type VIDEO, AUDIO
\param	ulAddr Start address of one frame
\param	ulSize Frame size
\param	ulDesID Use for MP4, fill null here
\return	0 success
		-1 Index size out of range
		-2 Index number out of range
		-3 AVI Video frame tag error
		-4 AVI Audio frame tag error
		-5 Video frame data header error before write to file system
		-6 Video frame data header error after write to file system
		-10 samaphore busy
*/
int32_t slAVI_WriteOneFrame(uint8_t ubCh, uint8_t ubStreamType, uint16_t uwFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID);

//------------------------------------------------------------------------
/*!
\brief	Close AVI file format
\param	ubCh Source Channel
\return	(no)
*/
void AVI_Close(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get movie time length
\param	ubCh Source Channel
\return	Unit second
*/
uint16_t uwAVI_MovieLength(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Set Video1 frame information
\param	ubCh Source Channel
\param	ulHSize Horizontal resolution
\param	ulVSize Vertical resolution
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void AVI_Vdo1SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval);

//------------------------------------------------------------------------
/*!
\brief	Set Audio frame information
\param	ubCh Source Channel
\param	ulSampleRate Sample rate
\param	uwBlockAlign Block Alignment
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void AVI_AdoSetFormat(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval);

//------------------------------------------------------------------------------
/*!
\brief Get memory size which is not increased by source number
\return	memory size
*/
uint32_t ulAVI_GetFixedMemorySize(void);

//------------------------------------------------------------------------------
/*!
\brief 	Set support max video fps, audio fps, and record time. used to caculate memory size.
\param	ubVFPS supported max video fps
\param	ubAFPS supported max audio fps
\param	ulMinute supported max record time
\return	(no)
*/
void AVI_SetRecMaxTime(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute);
//------------------------------------------------------------------------
/*!
\brief	Get skip Frame Size
\return	Frame Size
*/
uint32_t ulAVI_GetSkipFrameSize(void);

//------------------------------------------------------------------------
/*!
\brief	Write information data to AVI file format
\param	ubCh Source Channel
\param	ubOffset Offset position of AVI information structure
\param	ptr A pointer point to input data
\param	ubSize write size
\return	(no)
*/
void AVI_WriteInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize);

//------------------------------------------------------------------------
/*!
\brief	Read information data to AVI file format
\param	ubCh Source Channel
\param	ubOffset Offset position of AVI information structure
\param	ptr A pointer point to output data
\param	ubSize write size
\return	(no)
*/
void AVI_ReadInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize);

//------------------------------------------------------------------------
/*!
\brief	Get frame numbers be source type
\param	stream Stream Type
\return	Frame numbers
*/
uint32_t ulAVI_GetStrMaxFrmCnt(uint8_t ubStream);

//PLAYBACK
//------------------------------------------------------------------------
/*!
\brief	Load AVI Header from memory to Header structure
\param	ubCh Source Channel
\param	ulAddr Input start address
\return	(no)
*/
void AVI_LoadAviHeader(uint8_t ubCh, uint32_t ulAddr);

//------------------------------------------------------------------------
/*!
\brief	Get movie data size
\param	ubCh Source Channel
\return	Data size
*/
uint32_t ulAVI_GetMoviLength(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get movie time length
\param	ubCh Source Channel
\return	Time length
*/
uint16_t uwAVI_GetFilmLength(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief	Get video or audio frame number
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame number
*/
uint32_t ulAVI_GetMoviFrmCnt(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get video or audio frame Scale
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	frame Scale, fps = rate/scale
*/
uint32_t ulAVI_GetAviScale(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get video or audio frame rate
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	frame rate, fps = rate/scale
*/
uint32_t ulAVI_GetAviRate(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------
/*!
\brief	Get video resolution
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	frame resolution
*/
uint8_t ubAVI_GetResolution(uint8_t ubCh, uint8_t ubStreamType);
//------------------------------------------------------------------------
/*!
\brief	Get audio block alignment
\param	ubCh Source Channel
\return	block alignment
*/
uint32_t ulAVI_GetAdoBlockAlign(uint8_t ubCh); 

//------------------------------------------------------------------------
/*!
\brief	Get audio samples per block
\param	ubCh Source Channel
\return	samples
*/
uint32_t ulAVI_GetAdoSamplesPerBlock(uint8_t ubCh); 

//------------------------------------------------------------------------
/*!
\brief	Get audio samples rate
\param	ubCh Source Channel
\return	samples rate
*/
uint32_t ulAVI_GetAdoSampleRate(uint8_t ubCh); 

//------------------------------------------------------------------------
/*!
\brief	Get audio average bytes per second
\param	ubCh Source Channel
\return	average byte numbers
*/
uint32_t ulAVI_GetAdoAvgBytesPerSec(uint8_t ubCh);

#endif

