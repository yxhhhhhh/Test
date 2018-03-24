/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       MP4.h
	\brief		MP4 File format Head file
	\author		Wales
	\version    0.2
	\date		2017/03/21
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef _MP4_H_
#define _MP4_H_

#include "_510PF.h"

//------------------------------------------------------------------------------
//	DEFINITION
//------------------------------------------------------------------------------
#define MP4_MAJORVER        (0)         // Major version = 0
#define MP4_MINORVER        (2)         // Minor version = 2

#define MP4_THUMBNAIL_SIZE  (0x10200)    //32.5K(0x8210)

//------------------------------------------------------------------------------
//	Lable offset in Atom
//------------------------------------------------------------------------------
//! Define the position of elements in mvhd atom 
#define MP4_MVHD_CREATETIME		(12)
#define MP4_MVHD_MODIFYTIME		(16)
#define MP4_MVHD_TIMESCALE		(20)
#define MP4_MVHD_DURATION		(24)
#define MP4_MVHD_RATE			(28)
#define MP4_MVHD_VOLUME			(32)
#define MP4_MVHD_RESERVE		(34)
#define MP4_MVHD_NEXT_THKID		(104)

//! Define the position of elements in tkhd atom
#define MP4_TKHD_CREATETIME		(12)
#define MP4_TKHD_MODIFYTIME		(16)
#define MP4_TKHD_ID				(20)
#define MP4_TKHD_DURATION		(28)
#define MP4_TKHD_LAYER			(40)
#define MP4_TKHD_ALTERNATEGP	(42)
#define MP4_TKHD_VOLUME			(44)
#define MP4_TKHD_H_SIZE			(84)
#define MP4_TKHD_V_SIZE			(88)

//! Define the position of elements in elst atom
#define MP4_ELST_ENTRY			(12)
#define MP4_ELST_DURATION		(16)
#define MP4_ELST_TIME			(20)
#define MP4_ELST_RATE			(24)

//! Define the position of elements in mdhd atom
#define MP4_MDHD_CREATETIME		(12)
#define MP4_MDHD_MODIFYTIME		(16)
#define MP4_MDHD_TIMESCALE		(20)
#define MP4_MDHD_DURATION		(24)
#define MP4_MDHD_LANGUAGE		(28)
#define MP4_MDHD_QUALITY		(30)

//! Define the position of elements in avc1 atom
#define MP4_AVC1_DATA_REFIDX	(14)
#define MP4_AVC1_H_SIZE			(32)
#define MP4_AVC1_V_SIZE			(34)
#define MP4_AVC1_HRES			(36)
#define MP4_AVC1_VRES			(40)

//! Define the position of elements in mp4a atom
#define MP4A_CHANNEL_COUNT		(24)
#define MP4A_SAMPLE_SIZE        (26)
#define MP4A_SAMPLE_RATE		(32)

//! Define the position of elements in esds atom
#define ESDS_TRACKID            (53)
#define ESDS_OBJTYPE            (61)
#define ESDS_STREAMTYPE			(62)
#define ESDS_BUFFERSIZEDB		(63)
#define ESDS_MAX_BITRATE        (66)
#define ESDS_AVG_BITRATE        (70)
#define ESDS_AUD_SPEC_CONF		(79)

#define VDO_FRAME_DURAITON		(0x21)	//! Video Frame duration. Ex.1000/30 = 33 ms

#define ADO_16K512_FRM_DURAITON	(0x7D)	//! Audio Frame duration. 
										//! Ex.8*1000*2/4 = 4KB Szie ,4KB/512 = 8 frame/s ,125 ms/frame. 16kbit sample rate, 4x compress, 512 blcokalgin
#define ADO_16K256_FRM_DURAITON	(0x7D)	//! Audio Frame duration. 
										//! Ex.8*1000*2/4 = 4KB Szie ,4KB/256 = 16 frame/s ,62.5ms/frame. 16kbit sample rate, 4x compress, 256 blcokalgin

#define ADO_8K512_FRM_DURAITON	(0xFA)	//! Audio Frame duration. 
										//! 8*1000*1/4 = 2KB Szie ,2KB/512 = 4 frame/s ,250 ms/frame. 8 kb sample rate 4x compress, 512 blcokalgin
#define ADO_8K256_FRM_DURAITON	(0xFA)	//! Audio Frame duration. 
										//! 8*1000*1/4 = 2KB Szie ,2KB/256 = 8 frame/s ,125 ms/frame. 8 kb sample rate, 4x compress, 256 blcokalgin
#define ADO_FRAME_DURAITON	    (ADO_8K256_FRM_DURAITON)

#define MP4_MOOV_HDSIZE		(0x400L)
#define MP4_STBL_HDSIZE		(0x98L)

//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------
#define MP4_STBL_1MINSZ(Vfps,Afps)	(((32*Vfps)+(28*Afps))*60)	// video-stts=(8*Vfps*60*Time + 16), Audio-stts=(8*Afps*60*Time + 16)
																// video-stss=(4*Vfps*60*Time + 16)
																// video-stsc=(12*Vfps*60*Time + 20), Audio-stsc=(12*Afps*60*Time + 20)
																// video-stsz=(4*Vfps*60*Time + 20), Audio-stsz=(4*Afps*60*Time + 20)
																// video-stco=(4*Vfps*60*Time + 16), Audio-stco=(4*Afps*60*Time + 16)
																// moov header = 1024, 
																
//------------------------------------------------------------------------------
//	DATA STRUCT DEFINITION
//------------------------------------------------------------------------------

//! ftyp atom - Atom header structure, Size = 0x08(8) Bytes
typedef struct _mp4atom{
	uint8_t ubAtomSize[4];
	uint8_t ubAtomType[4];
} MP4ATOM;
#define ATOMHD	8	// sizeof(MP4ATOM)

//! ftyp atom - File Type Atom structure, Size = 0x20(32) Bytes
typedef struct _file_type_atom{
	MP4ATOM	ftyp;					// 8 Bytes
	uint8_t ulMajorBrand[4];        // 4 Byte
	uint8_t ubMinorVersion[4];      // 4 Byte
	uint8_t ubCompatibleBrands[16];	// 16
} FILE_TYPE_ATOM;

//! mvhd atom - Movie Header Atom structure, Size = 0x6C(108) Bytes
typedef struct _movie_header_atom{
	MP4ATOM	mvhd;					// 8 Bytes
	uint8_t ubVersion;				// 1 Byte
	uint8_t ubFlag[3];				// 3 Bytes
	uint8_t ubCreateTime[4];        // 4 Bytes
	uint8_t ubModificationTime[4];  // 4 Bytes
	uint8_t ubTimeScale[4];			// 4 Bytes
	uint8_t ubDuration[4];			// 4 Bytes
	uint8_t ubRate[4];				// 4 Bytes
	uint8_t ubVolume[2];            // 2 Bytes
	uint8_t ubReserve[10];			// 10 Bytes,hidden delay time.
	uint8_t ubMaxtix[36];           // 36 Bytes
	uint8_t ubPreviewTime[4];       // 4 Bytes
	uint8_t ubPreviewDuration[4];   // 4 Bytes
	uint8_t ubPosterTime[4];        // 4 Bytes
	uint8_t ubSelectionTime[4];		// 4 Bytes
	uint8_t ubSelectionDuration[4]; // 4 Bytes
	uint8_t ubCuttentTime[4];       // 4 Bytes
	uint8_t ubNextTrackID[4];       // 4 Bytes
} MOVIE_HEADER_ATOM;

//! tkhd atom - Track Header Atom, Size = 0x5C(92) Bytes
typedef struct _track_header_atom{
	MP4ATOM	tkhd;					// 8 Bytes
	uint8_t ubVersion;				// 1 Byte
	uint8_t ubFlag[3];				// 3 Bytes
	uint8_t ubCreateTime[4];        // 4 Bytes
	uint8_t ubModificationTime[4];  // 4 Bytes
	uint8_t ubTrackID[4];           // 4 Bytes
	uint8_t ubReserve1[4];			// 4 Bytes
	uint8_t ubDuration[4];			// 4 Bytes
	uint8_t ubReserve2[8];			// 8 Bytes
	uint8_t ubLayer[2];				// 2 Bytes
	uint8_t ubAlternateGroup[2];    // 2 Bytes
	uint8_t ubVolume[2];            // 2 Bytes
	uint8_t ubReserve3[2];			// 2 Bytes
	uint8_t ubMaxtix[36];           // 36 Bytes
	uint8_t ubTrackWidth[4];        // 4 Bytes
	uint8_t ubTrackHeight[4];       // 4 Bytes
} TRACK_HEADER_ATOM;

//! Element of elst entry, Size = 0x0C(12)
typedef struct _edit_list_entry{
	uint8_t ubTrackDuration[4];		// 4 Bytes
	uint8_t ubTime[4];				// 4 Bytes
	uint8_t ubRate[4];				// 4 Bytes
}EDIT_LIST_ENTRY;

//! elst atom - Edit List Atom, Size = 0x10(16) + 12*entry number Bytes
typedef struct _edit_list_atom{
	MP4ATOM elst;                   // 8 Bytes
	uint8_t ubVersion;			    // 1 Byte
	uint8_t ubFlag[3];			    // 3 Bytes
	uint8_t ubEditListEntry[4];     // 4 Bytes
	EDIT_LIST_ENTRY	EditList[1];    // 12 * entry number
} EDIT_LIST_ATOM;

//! mdhd atom - Media Header Atom, Size = 0x20(32) Bytes
typedef struct _media_header_atom{
	MP4ATOM mdhd;				    // 8 Bytes
	uint8_t ubVersion;			    // 1 Byte
	uint8_t ubFlag[3];			    // 3 Bytes
	uint8_t ubCreateTime[4];		// 4 Bytes
	uint8_t ubModificationTime[4];	// 4 Bytes
	uint8_t ubTimeScale[4];		    // 4 Bytes
	uint8_t ubDuration[4];		    // 4 Bytes
	uint8_t ubLanguage[2];		    // 2 Bytes
	uint8_t ubQuality[2];			// 2 Bytes
} MEDIA_HEADER_ATOM;

//! hdlr atom - Handler Reference Atom, Size = 0x2D(45) Bytes
typedef struct _handler_reference_atom{
	MP4ATOM hdlr;               // 8 Bytes
	uint8_t ubVersion;			// 1 Byte
	uint8_t ubFlag[3];			// 3 Bytes
	uint8_t ubPreDefined[4];    // 4 Bytes
	uint8_t ubHandlerType[4];   // 4 Bytes
	uint8_t ubReserve[12];		// 12 Bytes
	uint8_t ubName[13];			// 13 Bytes
} HANDLER_REFERENCE_ATOM;

//! vmhd atom - Video Media Header Atom, Size = 0x14(20) Bytes
typedef struct _video_media_header_atom{
	MP4ATOM vmhd;				// 8 Bytes
	uint8_t ubVersion;			// 1 Byte
	uint8_t ubFlag[3];			// 3 Bytes
	uint8_t ubGraphicsMode[2];	// 2 Bytes
	uint8_t ubOpcolor[6];       // 6 Bytes
} VIDEO_MEDIA_HEADER_ATOM;

//! dinf atom - Data Information Atoms, Size = 0x24(36) Bytes
typedef struct _data_information_atom{
	MP4ATOM dinf;               // 8 Bytes
	MP4ATOM dref;				// 8 Bytes
	uint8_t ubVersion;			// 1 Byte
	uint8_t ubFlag[3];			// 3 Bytes
	uint8_t ubEntryCount[4];    // 4 Bytes
	uint8_t ubURLSize[4];       // 4 Bytes
	uint8_t ubURL[4];			// 4 Bytes
	uint8_t ubLocation[4];		// 4 Bytes
} DATA_INFORMATION_ATOM;

//! stsd atom - Sample Description Atom, Size = 0x77(119) Bytes
typedef struct _video_sample_description_atom{
	MP4ATOM stsd;					// 8 Bytes
	uint8_t ubVersion;				// 1 Byte
	uint8_t ubFlag[3];				// 3 Bytes
	uint8_t ubSampleDscripCount[4];	// 4 Bytes
	
	//avc1
	uint8_t ubDescriptionSize[4];   // 4 Bytes
	uint8_t ubDescriptionType[4];   // 4 Byte
	uint8_t ubReserve1[6];			// 6 Bytes
	uint8_t ubDataRefIndex[2];		// 2 Bytes
	uint8_t ubReserve2[16];			// 16 Bytes
	uint8_t ubWidth[2];				// 2 Bytes
	uint8_t ubHight[2];				// 2 Bytes
	uint8_t ubHResolution[4];       // 4 Bytes
	uint8_t ubVResolution[4];       // 4 Bytes
	uint8_t ubReserve3[4];			// 4 Bytes
	uint8_t ubFrameCount[2];        // 2 Bytes
	uint8_t ubCompressName[32];		// 32 Bytes
	uint8_t ubDepth[2];				// 2 Bytes
	uint8_t ubPreDefine[2];			// 2 Bytes
	// avcC
	uint8_t ubavcCSize[4];			// 4 Bytes
	uint8_t ubavcCType[4];			// 4 Bytes
	uint8_t ubVideoSample[6];       // 6 Bytes
	uint8_t ubSeqParaSetLen[2];		// 2 Bytes, SPS Length
	uint8_t ubSPSContent[10];		// 10 Bytes
	uint8_t ubPicParaSetNum;        // 1 Bytes
	uint8_t ubPicParaSetLen[2];		// 2 Bytes
	uint8_t ubPPSContent[4];        // 4 Bytes
} VIDEO_SAMPLE_DESCRIPTION_ATOM;

//! smhd atom - Sound Media Header Atom, Size = 0x10(16) Bytes
typedef struct _sound_media_header_atom{
	MP4ATOM smhd;           // 8 Bytes
	uint8_t ubVersion;      // 1 Byte
	uint8_t ubFlag[3];      // 3 Bytes
	uint8_t ubBalance[2];   // 2 Bytes
	uint8_t ubReserve[2];   // 2 Bytes
} SOUND_MEDIA_HEADER_ATOM;

//! stsd atom - Sample Description Atoms, Size = 0x67(103) Bytes
typedef struct _audio_sample_description_atom{
	MP4ATOM stsd;					// 8 Bytes
	uint8_t ubVersion;				// 1 Byte
	uint8_t ubFlag[3];				// 3 Bytes
	uint8_t ubSampleDscripCount[4];	// 4 Bytes
	//mp4a
	uint8_t ubDescriptionSize[4];   // 4 Bytes
	uint8_t ubDescriptionType[4];   // 4 Byte
	uint8_t ubReserve1[4];			// 4 Bytes
	uint8_t ubDataRefIndex[4];		// 4 Bytes
	uint8_t ubReserve2[8];			// 8 Bytes
	uint8_t ubChannelCnt[2];        // 2 Bytes
	uint8_t ubSampleSize[2];        // 2 Bytes
	uint8_t ubPreDefine[2];			// 2 Bytes
	uint8_t ubReserve3[2];			// 2 Bytes
	uint8_t ubSampleRate[4];        // 4 Bytes
	// esds
	uint8_t ubesdsSize[4];			// 4 Bytes
	uint8_t ubesdsType[4];			// 4 Bytes
	uint8_t ubesdsVersion[1];       // 1 Bytes
	uint8_t ubesdsFlag[3];			// 3 Bytes
	// esds - desc
	uint8_t ubDescTag[4];			// 4 Bytes
	uint8_t ubDescSize[1];			// 1 Bytes
	uint8_t ubDescID[2];            // 2 Bytes
	uint8_t ubDescFlag[1];			// 1 Bytes
	// esds - decode config desc 
	uint8_t ubDecConfDescTag[4];    // 4 Bytes
	uint8_t ubDecConfDescSize[1];   // 1 Bytes
	uint8_t ubDecConfDescObjType[1];// 1 Bytes
	uint8_t ubDecConfDescStrmType[1];// 1 Bytes
	uint8_t ubDecConfDescBuffSize[3];// 3 Bytes
	uint8_t ubDecConfDescMaxBR[4];  // 4 Bytes
	uint8_t ubDecConfDescAvgBR[4];  // 4 Bytes
	// esds - decode spec info desc 
	uint8_t ubDecInfoDescTag[4];    // 4 Bytes
	uint8_t ubDecInfoDescSize[1];   // 1 Bytes
	uint8_t ubDecInfoDescData[2];   // 2 Bytes
	// esds - decode spec info desc 
	uint8_t ubSLDescTag[4];         // 4 Bytes
	uint8_t ubSLDescSize[1];        // 1 Bytes
	uint8_t ubSLDescData[1];        // 1 Bytes
} AUDIO_SAMPLE_DESCRIPTION_ATOM;

//! Indicate start address of sub atom in Stbl atom
typedef struct _stbl_info{
	uint32_t ulSttsVAdr;
	uint32_t ulStssVAdr;
	uint32_t ulStscVAdr;
	uint32_t ulStszVAdr;
	uint32_t ulStcoVAdr;
	uint32_t ulSttsAAdr;
	uint32_t ulStscAAdr;
	uint32_t ulStszAAdr;
	uint32_t ulStcoAAdr;
} STBL_INFO;

//! Indicate stbl information, for rec used
typedef struct _mp4_stbl{
	uint32_t    ulSttsEntry;                // 4 Bytes, stts
	uint32_t    ulSttsSampleCount;
	uint32_t    ulSttsSampleDuration;
	uint32_t    ulStssSyncSample;           // 4 Bytes, stss
	uint32_t    ulStssSampleId;
	uint32_t    ulStscEntry;                // 4 Bytes, stsc
	uint32_t    ulStscFirstChunk;			
	uint32_t    ulStscSamplePerChunk;		
	uint32_t    ulStscSampeDesID;			
	uint32_t    ulStszSampleSizeEntry;      // 4 Bytes, stsz
	uint32_t    ulStszSampleSize;				
	uint32_t    ulStcoChunkOffsetEntry;     // 4 Bytes, stsc
	uint32_t    ulStcoChunkOffset;		
	uint32_t    ulStscPreSamplePerChunk;    // 4 Bytes,stsc
	uint32_t    ulStscPreSampeDesID;		
} MP4_STBL;		// 60 Bytes

//! Indicate chunk controller information, for rec used
typedef struct _rec_chunk_controller{
	volatile uint8_t    ubChunkStartFlag;   // 1 Bytes
	uint8_t ubStreamType;				    // 1 Bytes
	uint8_t ubVideoAudioRunFg[3];			// 3 Bytes
	uint8_t ubReserve[3];					// 3 Bytes
	MP4_STBL    stStbl[3];					// 180
	uint32_t    ulmdatSampleSize[3];        // 12 Byte,mdat
} REC_CHUNK_CONTROLLER;	// 200 Bytes

//! Indicate stbl information, for play used
typedef struct _ply_chunk_controller{
	uint32_t    ulFileOffset;
	uint32_t    ulFrmType;

	uint32_t    ulVDOTotalSTSC;
	uint32_t    ulVDOTotalChk;
	uint32_t    ulVDOChkIdx;
	uint32_t    ulVDOChkAdrOffset;
	uint32_t    ulVDOTotalFrm;
	uint32_t    ulVDOFrmIdxInFile;
	uint32_t    ulVDOFrmSize;

	uint32_t    ulADOTotalSTSC;
	uint32_t    ulADOTotalChk;
	uint32_t    ulADOChkIdx;
	uint32_t    ulADOChkAdrOffset;
	uint32_t    ulADOTotalFrm;
	uint32_t    ulADOFrmIdxInFile;
	uint32_t    ulADOFrmSize;
} PLY_CHUNK_CONTROLLER;

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*!
\brief Get MP4 Version
\return	(no)
*/
uint16_t uwMP4_GetVersion(void);

//------------------------------------------------------------------------------
/*!
\brief	Create write frame samaphore
\return	(no)
*/
void MP4_SemaphoreCreate(void);

//------------------------------------------------------------------------
/*!
\brief	Set configured source number in record mode.
\param	ubSrcNum Total source number
\return	(no)
*/
void MP4_SetConfigedSrcNum(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief	Calculate requested memory buffer size
\param	ubSrcNum Total source number
\return	Memory size
*/
uint32_t ulMP4_GetBufSz(void);

//------------------------------------------------------------------------------
/*!
\brief	file format initial
\param	ubSrcNum Total source number
\param	ulStartAdr Memory start address
\return	(no)
*/
void MP4_Init(uint32_t ulStartAdr);

//------------------------------------------------------------------------------
/*!
\brief	set source type
\param	ubCh Source Channel
\param	ubSourceType Set source type
\return	(no)
*/
void MP4_SetSourceType(uint8_t ubCh, uint8_t ubSourceType);

//------------------------------------------------------------------------------
/*!
\brief	Set Video1 frame information
\param	ubCh Source Channel
\param	ulHSize Horizontal resolution
\param	ulVSize Vertical resolution
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void MP4_Vdo1SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval);

//------------------------------------------------------------------------------
/*!
\brief	Set Video2 frame information
\param	ubCh Source Channel
\param	ulHSize Horizontal resolution
\param	ulVSize Vertical resolution
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void MP4_Vdo2SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval);

//------------------------------------------------------------------------------
/*!
\brief	Set Audio frame information
\param	ubCh Source Channel
\param	ulSampleRate Sample rate
\param	uwBlockAlign Block Alignment
\param	uwFrmInterval Frame interval = 1/fps
\return	(no)
*/
void MP4_AdoSetFormat(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval);

//------------------------------------------------------------------------------
/*!
\brief Get memory size which is not increased by source number
\return	memory size
*/
uint32_t ulMP4_GetFixedMemorySize(void);

//------------------------------------------------------------------------------
/*!
\brief 	Set support max video fps, audio fps, and record time. used to caculate memory size.
\param	ubVFPS supported max video fps
\param	ubAFPS supported max audio fps
\param	ulMinute supported max record time
\return	(no)
*/
void MP4_SetRecMaxTime(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute);

//------------------------------------------------------------------------------
/*!
\brief	Create MP4 file format stucture
\param	ubCh Source Channel
\return	(no)
*/
void MP4_CreateFile(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Write video or audio frame data
\param	ubCh Source Channel
\param	ubStreamType Stream Type VIDEO, AUDIO
\param	ulAddr Start address of one frame
\param	ulSize Frame size
\param	ulDesID Use for MP4, fill null here
\return	0 success
		-10 samaphore busy
*/
int32_t slMP4_WriteOneFrame(uint8_t ubCh, uint8_t ubStreamType,uint8_t ubFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID);

//------------------------------------------------------------------------------
/*!
\brief	Close AVI file format
\param	ubCh Source Channel
\return	(no)
*/
void MP4_Close(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get movie time length
\param	ubCh Source Channel
\return	Unit second
*/
uint16_t uwMP4_MovieLength(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get skip Frame Size
\return	Frame Size
*/
uint32_t ulMP4_GetSkipFrameSize(void);

//------------------------------------------------------------------------------
/*!
\brief	Write information data to AVI file format
\param	ubCh Source Channel
\param	ubOffset Offset position of AVI information structure
\param	ptr A pointer point to input data
\param	ubSize write size
\return	(no)
*/
void MP4_WriteInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize);


// Play
//------------------------------------------------------------------------------
/*!
\brief	Assignment and partition memory to commite MP4 moov data structure
\param	ubCh Source Channel
\param	ulAdr Memory start address
\param	ulSize moov entire size
\return	(no)
*/
void MP4_AssignmentStructure(uint8_t ubCh, uint32_t ulAdr, uint32_t ulSize);

//------------------------------------------------------------------------------
/*!
\brief	Get delay time between current file and first created file
\param	ubCh Source Channel
\return	Delay time. Unit=1ms
*/
uint32_t ulMP4_GetStreamDelayTime(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get stream time scale
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Time scale. No unit
*/
uint32_t ulMP4_GetStreamTimeScale(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------------
/*!
\brief	Get stream total frame number
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame Number. No unit
*/
uint32_t ulMP4_GetTotalFrameNumber(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------------
/*!
\brief	Get MP4 frame resolution
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame Duration. No unit
*/
uint8_t ubMP4_GetResolution(uint8_t ubCh, uint8_t ubStreamType);
//------------------------------------------------------------------------------
/*!
\brief	Get stream frame duration. frame duration/time scale = frame interval (unit=sec)
\param	ubCh Source Channel
\param	ubStreamType Stream Type
\return	Frame Duration. No unit
*/
uint32_t ulMP4_GetFrameDuration(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------------
/*!
\brief	Get stream I frame number
\param	ubCh Source Channel
\return	Frame Number. No unit
*/
uint32_t ulMP4_GetTotalVideoIFrame(uint8_t ubCh, uint8_t ubStreamType);

//------------------------------------------------------------------------------
/*!
\brief	Parse video stbl data structure. To store I frame information, address offset, frame index, chunk index.
\param	ubCh Source Channel
\param	ulAdr Inpur memory start address
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1 success.
*/
uint32_t ulMP4_ParseVideoIFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	Parse audio stbl data structure. To store frame information, address offset,chunk index.
\param	ubCh Source Channel
\param	ulAdr Inpur memory start address
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1 success.
*/
uint32_t ulMP4_ParseAudioFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	Get audio samples rate
\param	ubCh Source Channel
\return	samples rate
*/
uint32_t ulMP4_GetAudioSampleRate(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get audio average bytes per second
\param	ubCh Source Channel
\return	average byte numbers
*/
uint32_t ulMP4_GetAudioAvgBR(uint8_t ubCh);

//------------------------------------------------------------------------------
/*!
\brief	Get frame information, frame type, frame size from real raw data.
\param	ubCh Source Channel
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1
*/
uint32_t ulMP4_ParseMdatFrameInfo(uint8_t ubCh ,PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	For Jump used. Reload stbl infromation.
\param	ubCh Source Channel
\param	PlyChkCtrl A pointer point to PLY_CHUNK_CONTROLLER data structure
\return	1
*/
uint32_t ulMP4_JumpReload(uint8_t ubCh, PLY_CHUNK_CONTROLLER *PlyChkCtrl);

//------------------------------------------------------------------------------
/*!
\brief	Get video frame size
\param	ubCh Source Channel
\param	ulFrm Frame index
\return	frame size
*/
uint32_t ulMP4_GetVideoFrameSize( uint8_t ubCh , uint32_t ulFrm);

#endif

