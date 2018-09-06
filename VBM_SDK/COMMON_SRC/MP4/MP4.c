/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file       MP4.c
	\brief		MP4 File format
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
#include "MP4.h" 
#include "FS_API.h"

//------------------------------------------------------------------------------
//	DEFINITION                       |
//------------------------------------------------------------------------------
#define MP4_SRC_NUM             (REC_SRC_NUM)

#define MP4_STR_V1		(REC_STR_V1)
#define MP4_STR_A1		(REC_STR_A1)
#define MP4_STR_V2		(REC_STR_V2)
#define MP4_STR_MAX		(REC_STR_MAX)

#define MP4_FRMTYPE_VDO	(PLY_FRMTYPE_VDO)    //!< Bitstream type in avi file is video
#define MP4_FRMTYPE_ADO	(PLY_FRMTYPE_ADO)    //!< Bitstream type in avi file is audio

#define MP4_V_PFRM		(REC_P_VFRM)
#define MP4_V_IFRM		(REC_I_VFRM)
#define MP4_V_SFRM		(REC_SKIP_FRM)

#define MP4_RES_NONE		(REC_RES_NONE)
#define MP4_RES_FHD		(REC_RES_FHD)
#define MP4_RES_HD		(REC_RES_HD)
#define MP4_RES_WVGA	(REC_RES_WVGA)
#define MP4_RES_VGA		(REC_RES_VGA)

//------------------------------------------------------------------------------
//	MACRO DEFINITION
//------------------------------------------------------------------------------
// Video1 & Video2 memory Size = (BytesPerFrame*FramePerSec*60sec*10min)+ size of atom header. 
#define MP4_STTS_ATOM_SIZE(fps,Time)		(480*fps*Time + 16)	// Indicate video stbl-stts size=(8*Vfps*60*Time + 16)
#define MP4_STSS_ATOM_SIZE(fps,Time)		(240*fps*Time + 16)	// Indicate video stbl-stss size=(4*Vfps*60*Time + 16)
#define MP4_STSC_ATOM_SIZE(fps,Time)		(720*fps*Time + 16)	//Indicate video stbl-stsc size=(12*Vfps*60*Time + 16)
#define MP4_STSZ_ATOM_SIZE(fps,Time)		(240*fps*Time + 20)	// Indicate video stbl-stsz size=(4*Vfps*60*Time + 20)
#define MP4_STCO_ATOM_SIZE(fps,Time)		(240*fps*Time + 16)	// Indicate video stbl-stco size=(4*Vfps*60*Time + 16)


//------------------------------------------------------------------------------
//	GLOBAL VARIABLE
//------------------------------------------------------------------------------
/*
MP4 Tree
ROOT
 -Ftyp
 -Free
 -mdat
 -moov
 	-mvhd
 	-trak
 		-tkhd
 		-edts->elst
		-mdia
			-mdhd
			-hdlr
			-minf
				-vmhd
				-dinf->dref->url
				-stbl
					-stsd->avc1->avcC
					-stts
					-stss
					-stsc
					-stsz
					-stco
	-trak
	-trak
	-udta
		-data
*/

// For MP4 Record
osSemaphoreId SEM_MP4_WRFrm;

uint8_t ubMP4_CfgSrcNum;		// Indicate configuration source number
uint32_t ulMP4_RecTime=0;		// Indicate max record time
uint8_t ubMP4_MaxVFPS=0;
uint8_t ubMP4_MaxAFPS=0;

// File Type Atom, Size = 0x20
uint8_t MP4_FtypAtom[0x20] = {
	0x00, 0x00, 0x00, 0x20,  'f',  't',  'y',  'p',  'i',  's',  'o',  'm', 0x00, 0x00, 0x02, 0x00,
	 'i', 	 's',  'o',  'm',  'i',  's',  'o',  '2',  'a',  'v',  'c',  '1',  'm',  'p',  '4',  '1',
};

// Free Atom, Size = 0x08
uint8_t MP4_FreeAtom[8] = {
	0x00, 0x00, 0x00, 0x08,  'f',  'r',  'e',  'e',
};

// mdat Atom, Size Update finially.
uint8_t MP4_MdatAtom[8] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'd',  'a',  't',
};

// Movie Atom, Size Update finially. Size = 8 + size(mvhd) + size(trak1) + size(trak2) + size(trak3)
uint8_t MP4_MoovAtom[8] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'o',  'o',  'v',
};

// Movie Header Atom, Size = 0x6C(108)
uint8_t MP4_MvhdAtom[] = {
	0x00, 0x00, 0x00, 0x6c,  'm',  'v',  'h',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//------------------------------------------------------------------------------------------
// Video1 Part
//------------------------------------------------------------------------------------------
// Video 1 Track Atom, Size Update finially. Size = 8 + size(tkhd) + size(edit) + size(mdia)
uint8_t MP4_V1TrakAtom[] = {
	0x00, 0x00, 0x00, 0x08,  't',  'r',  'a',  'k',
};

// Video 1 Track Header Atom, Size = 0x5C(92)
uint8_t MP4_V1TkhdAtom[] = {
	0x00, 0x00, 0x00, 0x5c,  't',  'k',  'h',  'd', 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5e, 0x38,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00,
};

// Video 1 Edit atoms, Size Update finially. Size = 8 + size(elst) = 24 + 12*n
uint8_t MP4_V1EdtsAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'e',  'd',  't',  's',
};

// Video 1 Edit List Atoms, Size = 16 + 12*n
uint8_t MP4_V1ElstAtom[] = {
	0x00, 0x00, 0x00, 0x1c,  'e',  'l',  's',  't', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
};

// 
uint8_t MP4_V1MdiaAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'd',  'i',  'a',
};

uint8_t MP4_V1MdhdAtom[] = {
	0x00, 0x00, 0x00, 0x20,  'm',  'd',  'h',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x05, 0xe3, 0x80, 0x55, 0xc4, 0x00, 0x00,
};

uint8_t MP4_V1HdlrAtom[] = {
	0x00, 0x00, 0x00, 0x2d,  'h',  'd',  'l',  'r', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 'v',  'i',  'd',  'e', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 'V',  'i',  'd',  'e',  'o',  'H',  'a',  'n',  'd',  'l',  'e',  'r', 0x00
};

uint8_t MP4_V1MinfAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'i',  'n',  'f',
};

uint8_t MP4_V1VmhdAtom[] = {
	0x00, 0x00, 0x00, 0x14,  'v',  'm',  'h',  'd', 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

uint8_t MP4_V1DinfAtom[] = {
	0x00, 0x00, 0x00, 0x24,  'd',  'i',  'n',  'f', 0x00, 0x00, 0x00, 0x1c,  'd',  'r',  'e',  'f',
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x75, 0x72, 0x6c, 0x20,
	0x00, 0x00, 0x00, 0x01,
};

uint8_t MP4_V1StblAtom[] = {
	0x00, 0x00, 0x00, 0x08,  's',  't',  'b',  'l',
};

uint8_t MP4_V1StsdAtom[] = {
	0x00, 0x00, 0x00, 0x87,  's',  't',  's',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

uint8_t MP4_V1Avc1Atom[] = {
	0x00, 0x00, 0x00, 0x77,  'a',  'v',  'c',  '1', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x80, 0x04, 0x40, 0x00, 0x48, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00, 0x21,  'a',  'v',  'c',  'C', 0x01, 0x4d,
	0x40, 0x29, 0xff, 0xe1, 0x00, 0x0a, 0x67, 0x4d, 0x40, 0x29, 0x96, 0x54, 0x03, 0xc0, 0x11, 0x32,
	0x01, 0x00, 0x04, 0x68, 0xce, 0x38, 0x80, 
};

//------------------------------------------------------------------------------------------
// Video2 Part
//------------------------------------------------------------------------------------------
uint8_t MP4_V2TrakAtom[] = {
	0x00, 0x00, 0x00, 0x08,  't',  'r',  'a',  'k',
};

uint8_t MP4_V2TkhdAtom[] = {
	0x00, 0x00, 0x00, 0x5c,  't',  'k',  'h',  'd', 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5e, 0x38,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x04, 0x40, 0x00, 0x00,
};

uint8_t MP4_V2EdtsAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'e',  'd',  't',  's',
};

uint8_t MP4_V2ElstAtom[] = {
	0x00, 0x00, 0x00, 0x1c,  'e',  'l',  's',  't', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
};

uint8_t MP4_V2MdiaAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'd',  'i',  'a',
};

uint8_t MP4_V2MdhdAtom[] = {
	0x00, 0x00, 0x00, 0x20,  'm',  'd',  'h',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x05, 0xe3, 0x80, 0x55, 0xc4, 0x00, 0x00,
};

uint8_t MP4_V2HdlrAtom[] = {
	0x00, 0x00, 0x00, 0x2d,  'h',  'd',  'l',  'r', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 'v',  'i',  'd',  'e', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 'V',  'i',  'd',  'e',  'o',  'H',  'a',  'n',  'd',  'l',  'e',  'r', 0x00
};

uint8_t MP4_V2MinfAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'i',  'n',  'f',
};

uint8_t MP4_V2VmhdAtom[] = {
	0x00, 0x00, 0x00, 0x14,  'v',  'm',  'h',  'd', 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

uint8_t MP4_V2DinfAtom[] = {
	0x00, 0x00, 0x00, 0x24,  'd',  'i',  'n',  'f', 0x00, 0x00, 0x00, 0x1c,  'd',  'r',  'e',  'f',
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x75, 0x72, 0x6c, 0x20,
	0x00, 0x00, 0x00, 0x01,
};

uint8_t MP4_V2StblAtom[] = {
	0x00, 0x00, 0x00, 0x08,  's',  't',  'b',  'l',
};

uint8_t MP4_V2StsdAtom[] = {
	0x00, 0x00, 0x00, 0x87,  's',  't',  's',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

uint8_t MP4_V2Avc1Atom[] = {
	0x00, 0x00, 0x00, 0x77,  'a',  'v',  'c',  '1', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x80, 0x04, 0x40, 0x00, 0x48, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00, 0x21,  'a',  'v',  'c',  'C', 0x01, 0x4d,
	0x40, 0x29, 0xff, 0xe1, 0x00, 0x0a, 0x67, 0x4d, 0x40, 0x29, 0x96, 0x54, 0x03, 0xc0, 0x11, 0x32,
	0x01, 0x00, 0x04, 0x68, 0xce, 0x38, 0x80, 
};

uint8_t MP4_VSkipFrame[] = {
	0x00, 0x00, 0x00, 0x20, 0x01, 0x9A, 0x04, 0x07, 0xAF, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00,
	0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
	0x03, 0x00, 0x01, 0x03
};

//------------------------------------------------------------------------------------------
// Audio Part
//------------------------------------------------------------------------------------------
uint8_t MP4_ATrakAtom[] = {
	0x00, 0x00, 0x00, 0x08,  't',  'r',  'a',  'k',
};

uint8_t MP4_ATkhdAtom[] = {
	0x00, 0x00, 0x00, 0x5c,  't',  'k',  'h',  'd', 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x2c,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8_t MP4_AEdtsAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'e',  'd',  't',  's',
};

uint8_t MP4_AElstAtom[] = {
	0x00, 0x00, 0x00, 0x1c,  'e',  'l',  's',  't', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x5e, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
};

uint8_t MP4_AMdiaAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'd',  'i',  'a',
};

// 0x14 0x40:Time Scale ; 0xd1 0x60:Duration ; Real Time = Duration / Time Scale = 6s
uint8_t MP4_AMdhdAtom[] = {
	0x00, 0x00, 0x00, 0x20,  'm',  'd',  'h',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x40, 0x00, 0x00, 0xd1, 0x60, 0x55, 0xc4, 0x00, 0x00,
};

uint8_t MP4_AHdlrAtom[] = {
	0x00, 0x00, 0x00, 0x2d,  'h',  'd',  'l',  'r', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 's',  'o',  'u',  'n', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	 'S',  'o',  'u',  'n',  'd',  'H',  'a',  'n',  'd',  'l',  'e',  'r', 0x00
};

uint8_t MP4_AMinfAtom[] = {
	0x00, 0x00, 0x00, 0x08,  'm',  'i',  'n',  'f',
};

uint8_t MP4_SmhdAtom[] = {
	0x00, 0x00, 0x00, 0x10,  's',  'm',  'h',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8_t MP4_ADinfAtom[] = {
	0x00, 0x00, 0x00, 0x24,  'd',  'i',  'n',  'f', 0x00, 0x00, 0x00, 0x1c,  'd',  'r',  'e',  'f',
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x75, 0x72, 0x6c, 0x20,
	0x00, 0x00, 0x00, 0x01,
};

uint8_t MP4_AStblAtom[] = {
	0x00, 0x00, 0x00, 0x08,  's',  't',  'b',  'l',
};

uint8_t MP4_AStsdAtom[] = {
	0x00, 0x00, 0x00, 0x67,  's',  't',  's',  'd', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};

uint8_t MP4_Mp4aAtom[] = {
	0x00, 0x00, 0x00, 0x57,  'm',  'p',  '4',  'a', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x1F, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33,  'e',  's',  'd',  's', 0x00, 0x00, 0x00, 0x00,
	0x03, 0x80, 0x80, 0x80, 0x22, 0x00, 0x02, 0x00, 0x04, 0x80, 0x80, 0x80, 0x14, 0x40, 0x15, 0x00,
	0x00, 0x00, 0x00, 0x01, 0xf4, 0x00, 0x00, 0x00, 0x2a, 0x2e, 0x05, 0x80, 0x80, 0x80, 0x02, 0x15,
	0x88, 0x06, 0x80, 0x80, 0x80, 0x01, 0x02,
};

uint8_t MP4_ASkipFrame[32] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

uint8_t MP4_UdtaAtom[] = {
	0x00, 0x00, 0x82, 0x10,  'u',  'd',  't',  'a',
};

uint8_t MP4_DataAtom[] = {
	0x00, 0x00, 0x82, 0x08,  'd',  'a',  't',  'a',
};

uint8_t stts_str[] = {'s','t','t','s'};
uint8_t stss_str[] = {'s','t','s','s'};
uint8_t stsc_str[] = {'s','t','s','c'};
uint8_t stsz_str[] = {'s','t','s','z'};
uint8_t stco_str[] = {'s','t','c','o'};

uint32_t ulMP4_MemStartAddr;
uint32_t ulMP4_StartAddr[MP4_SRC_NUM];

// Video 1
uint32_t MP4_V1SttsAtom[MP4_SRC_NUM];
uint32_t MP4_V1StssAtom[MP4_SRC_NUM];
uint32_t MP4_V1StscAtom[MP4_SRC_NUM];
uint32_t MP4_V1StszAtom[MP4_SRC_NUM];
uint32_t MP4_V1StcoAtom[MP4_SRC_NUM];

// Audio
uint32_t MP4_ASttsAtom[MP4_SRC_NUM];
uint32_t MP4_AStscAtom[MP4_SRC_NUM];
uint32_t MP4_AStszAtom[MP4_SRC_NUM];
uint32_t MP4_AStcoAtom[MP4_SRC_NUM];

// Video2
uint32_t MP4_V2SttsAtom[MP4_SRC_NUM];
uint32_t MP4_V2StssAtom[MP4_SRC_NUM];
uint32_t MP4_V2StscAtom[MP4_SRC_NUM];
uint32_t MP4_V2StszAtom[MP4_SRC_NUM];
uint32_t MP4_V2StcoAtom[MP4_SRC_NUM];

// Thumbnail
uint32_t ulMP4ThumbnailAdr;

uint32_t ulMP4_FileSize[MP4_SRC_NUM];
uint32_t ulMP4_AtomSize[MP4_SRC_NUM];
uint32_t ulMP4_ThumbnailOffset[MP4_SRC_NUM];

uint8_t ubMP4_PIC[MP4_SRC_NUM] = {0};
uint8_t ubMP4_POC[MP4_SRC_NUM] = {0};

REC_CHUNK_CONTROLLER sRec_MP4Controller[MP4_SRC_NUM];

uint8_t ubMP4_SourceType[MP4_SRC_NUM];

uint32_t ulMP4_DelayTime[MP4_SRC_NUM];

uint32_t ulMP4_V1HSize[MP4_SRC_NUM];
uint32_t ulMP4_V1VSize[MP4_SRC_NUM];

uint32_t ulMP4_V2HSize[MP4_SRC_NUM];
uint32_t ulMP4_V2VSize[MP4_SRC_NUM];

uint32_t ulMP4_AdoSampleRate[MP4_SRC_NUM];
uint16_t uwMP4_AdoBlockAlign[MP4_SRC_NUM];
uint16_t uwMP4_AdoSamplesPerBlock[MP4_SRC_NUM];

uint32_t ulMvhdTimeScale[MP4_SRC_NUM];

uint32_t ulTrackDuration[MP4_SRC_NUM][MP4_STR_MAX];
uint32_t ulMdhdTimeScale[MP4_SRC_NUM][MP4_STR_MAX];
uint32_t ulMdhdDuration[MP4_SRC_NUM][MP4_STR_MAX];
uint16_t uwMP4_FrmDuration[MP4_SRC_NUM][MP4_STR_MAX];
//End of For MP4 Record
//------------------------------------------------------------------------------

// For MP4 Play
MP4ATOM *pMoov[MP4_SRC_NUM];
MOVIE_HEADER_ATOM *pMvhd[MP4_SRC_NUM];

// Play Video
MP4ATOM *pTrakV[MP4_SRC_NUM];
TRACK_HEADER_ATOM *pTkhdV[MP4_SRC_NUM];
MP4ATOM *pEdtsV[MP4_SRC_NUM];
EDIT_LIST_ATOM  *pElstV[MP4_SRC_NUM];
MP4ATOM *pMdiaV[MP4_SRC_NUM];
MEDIA_HEADER_ATOM  *pMdhdV[MP4_SRC_NUM];
HANDLER_REFERENCE_ATOM *pHdlrV[MP4_SRC_NUM];
MP4ATOM *pMinfV[MP4_SRC_NUM];
VIDEO_MEDIA_HEADER_ATOM *pVmhdV[MP4_SRC_NUM];
DATA_INFORMATION_ATOM *pDinfV[MP4_SRC_NUM];
MP4ATOM *pStblV[MP4_SRC_NUM];
VIDEO_SAMPLE_DESCRIPTION_ATOM *pStsdV[MP4_SRC_NUM];

// Play Audio
MP4ATOM *pTrakA[MP4_SRC_NUM];
TRACK_HEADER_ATOM *pTkhdA[MP4_SRC_NUM];
MP4ATOM *pEdtsA[MP4_SRC_NUM];
EDIT_LIST_ATOM  *pElstA[MP4_SRC_NUM];
MP4ATOM *pMdiaA[MP4_SRC_NUM];
MEDIA_HEADER_ATOM  *pMdhdA[MP4_SRC_NUM];
HANDLER_REFERENCE_ATOM *pHdlrA[MP4_SRC_NUM];
MP4ATOM *pMinfA[MP4_SRC_NUM];
SOUND_MEDIA_HEADER_ATOM *pSmhdA[MP4_SRC_NUM];
DATA_INFORMATION_ATOM *pDinfA[MP4_SRC_NUM];
MP4ATOM *pStblA[MP4_SRC_NUM];
AUDIO_SAMPLE_DESCRIPTION_ATOM *pStsdA[MP4_SRC_NUM];

STBL_INFO sStbl_Info[MP4_SRC_NUM];
// End of For MP4 Play

//------------------------------------------------------------------------------
//	FUNCTION PROTOTYPE
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int32_t ulMP4_GetLong(uint8_t *ptr, uint32_t ulOffset)
{
	int32_t result;
	result = ((int32_t)ptr[ulOffset] << 24) + ((int32_t)ptr[ulOffset+1] << 16) + ((int32_t)ptr[ulOffset+2] << 8) + ptr[ulOffset+3];
	return result;
}

static void vMP4_SetLong(uint8_t *ptr, uint32_t ulOffset, int32_t ulValue)
{
	ptr[ulOffset] = (uint8_t)(ulValue >> 24);
	ptr[ulOffset+1] = (uint8_t)(ulValue >> 16);
	ptr[ulOffset+2] = (uint8_t)(ulValue >> 8);
	ptr[ulOffset+3] = (uint8_t)ulValue;
}

static void vMP4_SetWord(uint8_t *ptr, uint32_t ulOffset, int16_t slValue)
{
	ptr[ulOffset] = (uint8_t)(slValue >> 8);
	ptr[ulOffset+1] = (uint8_t)slValue;
}

#if 0
static void vMP4_SetByte(uint8_t *ptr, uint32_t ulOffset, uint8_t ubValue)
{
	ptr[ulOffset] = ubValue;
}
#endif

static int32_t ulMP4_GetAtomSize(uint8_t *ptr)
{
	return (ulMP4_GetLong(ptr, 0));
}

static void vMP4_SaveAtom(uint8_t ubCh,uint8_t* ptr, uint32_t ulSize)
{
	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh] + ulMP4_AtomSize[ubCh]), ptr, ulSize);
	ulMP4_AtomSize[ubCh] += ulSize;
	ulMP4_FileSize[ubCh] += ulSize;
}

static void vMP4_Stbl_Init(uint8_t ubCh, uint8_t ubStreamType)
{
	switch (ubStreamType )
	{
		case MP4_STR_V1:
			// STTS
			memset((uint8_t *)MP4_V1SttsAtom[ubCh], 0, 24);
			*((uint8_t *)(MP4_V1SttsAtom[ubCh] + 3)) = 0x18;
			memcpy((uint8_t *)(MP4_V1SttsAtom[ubCh] + 4),stts_str, 4);
			*((uint8_t *)(MP4_V1SttsAtom[ubCh] + 15)) = 0x1;
			*((uint8_t *)(MP4_V1SttsAtom[ubCh] + 19)) = 0x1;
			*((uint8_t *)(MP4_V1SttsAtom[ubCh] + 23)) = uwMP4_FrmDuration[ubCh][ubStreamType];

			// STSS
			memset((uint8_t *)MP4_V1StssAtom[ubCh], 0, 16);
			memcpy((uint8_t *)(MP4_V1StssAtom[ubCh] + 4),stss_str, 4);

			// STSC
			memset((uint8_t *)MP4_V1StscAtom[ubCh], 0, 28);
			*((uint8_t *)(MP4_V1StscAtom[ubCh] + 3)) = 0x1c;
			memcpy((uint8_t *)(MP4_V1StscAtom[ubCh] + 4),stsc_str, 4);
			*((uint8_t *)(MP4_V1StscAtom[ubCh] + 15)) = 0x1;
			*((uint8_t *)(MP4_V1StscAtom[ubCh] + 19)) = 0x1;
			*((uint8_t *)(MP4_V1StscAtom[ubCh] + 23)) = 0x1;
			*((uint8_t *)(MP4_V1StscAtom[ubCh] + 27)) = 0x1;

			// STSZ
			memset((uint8_t *)MP4_V1StszAtom[ubCh], 0, 20);
			memcpy((uint8_t *)(MP4_V1StszAtom[ubCh] + 4),stsz_str, 4);

			// STCO
			memset((uint8_t *)MP4_V1StcoAtom[ubCh], 0, 16);
			memcpy((uint8_t *)(MP4_V1StcoAtom[ubCh] + 4),stco_str, 4);
		break;
		
		case MP4_STR_A1:
			// STTS
			memset((uint8_t *)MP4_ASttsAtom[ubCh], 0, 24);
			*((uint8_t *)(MP4_ASttsAtom[ubCh] + 3)) = 0x18;
			memcpy((uint8_t *)(MP4_ASttsAtom[ubCh] + 4),stts_str, 4);
			*((uint8_t *)(MP4_ASttsAtom[ubCh] + 15)) = 0x1;
			*((uint8_t *)(MP4_ASttsAtom[ubCh] + 19)) = 0x1;
			*((uint8_t *)(MP4_ASttsAtom[ubCh] + 23)) = uwMP4_FrmDuration[ubCh][ubStreamType];;

			// STSC
			memset((uint8_t *)MP4_AStscAtom[ubCh], 0, 28);
			*((uint8_t *)(MP4_AStscAtom[ubCh] + 3)) = 0x1c;
			memcpy((uint8_t *)(MP4_AStscAtom[ubCh] + 4),stsc_str, 4);
			*((uint8_t *)(MP4_AStscAtom[ubCh] + 15)) = 0x1;
			*((uint8_t *)(MP4_AStscAtom[ubCh] + 19)) = 0x1;
			*((uint8_t *)(MP4_AStscAtom[ubCh] + 23)) = 0x1;
			*((uint8_t *)(MP4_AStscAtom[ubCh] + 27)) = 0x1;

			// STSZ
			memset((uint8_t *)MP4_AStszAtom[ubCh], 0, 20);
			memcpy((uint8_t *)(MP4_AStszAtom[ubCh] + 4),stsz_str, 4);

			// STCO
			memset((uint8_t *)MP4_AStcoAtom[ubCh], 0, 16);
			memcpy((uint8_t *)(MP4_AStcoAtom[ubCh] + 4),stco_str, 4);
		break;
		
		case MP4_STR_V2:
			// STTS
			memset((uint8_t *)MP4_V2SttsAtom[ubCh], 0, 24);
			*((uint8_t *)(MP4_V2SttsAtom[ubCh] + 3)) = 0x18;
			memcpy((uint8_t *)(MP4_V2SttsAtom[ubCh] + 4),stts_str, 4);
			*((uint8_t *)(MP4_V2SttsAtom[ubCh] + 15)) = 0x1;
			*((uint8_t *)(MP4_V2SttsAtom[ubCh] + 19)) = 0x1;
			*((uint8_t *)(MP4_V2SttsAtom[ubCh] + 23)) = uwMP4_FrmDuration[ubCh][ubStreamType];

			// STSS
			memset((uint8_t *)MP4_V2StssAtom[ubCh], 0, 16);
			memcpy((uint8_t *)(MP4_V2StssAtom[ubCh] + 4),stss_str, 4);

			// STSC
			memset((uint8_t *)MP4_V2StscAtom[ubCh], 0, 28);
			*((uint8_t *)(MP4_V2StscAtom[ubCh] + 3)) = 0x1c;
			memcpy((uint8_t *)(MP4_V2StscAtom[ubCh] + 4),stsc_str, 4);
			*((uint8_t *)(MP4_V2StscAtom[ubCh] + 15)) = 0x1;
			*((uint8_t *)(MP4_V2StscAtom[ubCh] + 19)) = 0x1;
			*((uint8_t *)(MP4_V2StscAtom[ubCh] + 23)) = 0x1;
			*((uint8_t *)(MP4_V2StscAtom[ubCh] + 27)) = 0x1;

			// STSZ
			memset((uint8_t *)MP4_V2StszAtom[ubCh], 0, 20);
			memcpy((uint8_t *)(MP4_V2StszAtom[ubCh] + 4),stsz_str, 4);

			// STCO
			memset((uint8_t *)MP4_V2StcoAtom[ubCh], 0, 16);
			memcpy((uint8_t *)(MP4_V2StcoAtom[ubCh] + 4),stco_str, 4);
		break;
	}
}

static void vMP4_UpdateStsc(uint8_t ubCh, uint8_t ubStreamType)
{
	if(ubStreamType == MP4_STR_V1)
	{
		vMP4_SetLong((uint8_t *)MP4_V1StscAtom[ubCh], 0, (uint32_t)(16 + 12 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry));
		vMP4_SetLong((uint8_t *)MP4_V1StscAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry);
		vMP4_SetLong((uint8_t *)MP4_V1StscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk);
		vMP4_SetLong((uint8_t *)MP4_V1StscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk);
		vMP4_SetLong((uint8_t *)MP4_V1StscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 8, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID);
	}
	else if (ubStreamType == MP4_STR_A1)
	{
		vMP4_SetLong((uint8_t *)MP4_AStscAtom[ubCh], 0, (uint32_t)(16 + 12 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry));
		vMP4_SetLong((uint8_t *)MP4_AStscAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry);
		vMP4_SetLong((uint8_t *)MP4_AStscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk);
		vMP4_SetLong((uint8_t *)MP4_AStscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk);
		vMP4_SetLong((uint8_t *)MP4_AStscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 8, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID);	
	}
	else if (ubStreamType == MP4_STR_V2)
	{
		vMP4_SetLong((uint8_t *)MP4_V2StscAtom[ubCh], 0, (uint32_t)(16 + 12 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry));
		vMP4_SetLong((uint8_t *)MP4_V2StscAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry);
		vMP4_SetLong((uint8_t *)MP4_V2StscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk);
		vMP4_SetLong((uint8_t *)MP4_V2StscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk);
		vMP4_SetLong((uint8_t *)MP4_V2StscAtom[ubCh], 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 8, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID);
	}
}

static void vMP4_CheckUpdateStts(uint8_t ubCh,uint8_t *pStts,uint8_t ubStreamType,uint32_t ulDuration)
{
	if(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0 )
		return;

	if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration != ulDuration)
	{
		printf("ts %d, %d, %d\r\n",ubStreamType,sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount,sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration);

		vMP4_SetLong(pStts, 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount);
		vMP4_SetLong(pStts, 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration);
		
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry++;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount = 1;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration = ulDuration;
	}
	else
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount++;
}

static void vMP4_CheckUpdateStsc(uint8_t ubCh,uint8_t *pStsc,uint8_t *pStco,uint8_t ubStreamType,uint32_t ulDesID)
{
	if(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0 )
		return;

	if( sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk == 0 )
	{
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk++;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID = ulDesID;
		return;
	}

	if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID != ulDesID)
	{
		// Update video stsc atom
		printf("Usc %d, %x, %x, %x, %x\r\n",	ubStreamType,
											sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry,
											sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk,
											sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry,
											sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID);
		
		vMP4_SetLong(pStsc, 0, (uint32_t)(16 + 12 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry));
		vMP4_SetLong(pStsc, 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry);
		vMP4_SetLong(pStsc, 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk);
		vMP4_SetLong(pStsc, 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk);
		vMP4_SetLong(pStsc, 16 + 12*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry - 1) + 8, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID);
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry++;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscPreSamplePerChunk = sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscPreSampeDesID = sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID;
		
		// Update video stco atom
		printf("Usc %d, %x, %x \r\n",	ubStreamType,
									sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry,
									sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset);
		
		vMP4_SetLong(pStco, 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset);
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry++;			// stco

		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk++;				 // stsc
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk = 1;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID = ulDesID;
		
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset = ulMP4_FileSize[ubCh];
	}
	else
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk++;			// stsc
}

static void vMP4_CheckSourceSWUpdateChunk(uint8_t ubCh,uint8_t ubStreamType)
{
	uint8_t ubPreStreamType;

	ubPreStreamType = sRec_MP4Controller[ubCh].ubStreamType;
	
	if(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0 )
		return;

	// Inturrupt video contineous, add chunk to next.
	if(ubPreStreamType != ubStreamType)
	{
		//printf("CSW:%d PST:%d CST:%d\r\n",sRec_MP4Controller[ubCh].ubChunkStartFlag,ubPreStreamType,ubStreamType);
		
		if(	(sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSamplePerChunk != sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscPreSamplePerChunk) ||
			(sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSampeDesID != sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscPreSampeDesID) )
		{
			//printf("PSC(ST:%d EN:%x FC:%x SPC:%x SSD:%x)\r\n",	ubPreStreamType,
			//									sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscEntry,
			//									sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscFirstChunk,
			//									sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSamplePerChunk,
			//									sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSampeDesID);
			
			// Update video stsc atom
			vMP4_UpdateStsc(ubCh, ubPreStreamType);

			sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscEntry++;
			sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscPreSamplePerChunk = sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSamplePerChunk;
			sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscPreSampeDesID = sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSampeDesID;
		}

		//printf("PCO(ST:%d EN:%x OF:%x)\r\n",	ubPreStreamType,
		//							sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffsetEntry,
		//							sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffset);
		
		// Update Video stco atom
		if(ubPreStreamType == MP4_STR_V1)
			vMP4_SetLong((uint8_t *)MP4_V1StcoAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffset);
		if(ubPreStreamType == MP4_STR_A1)
			vMP4_SetLong((uint8_t *)MP4_AStcoAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffset);
		if(ubPreStreamType == MP4_STR_V2)
			vMP4_SetLong((uint8_t *)MP4_V2StcoAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffset);

		sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStcoChunkOffsetEntry++;

		// Jump to next video stsc stco
		sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscFirstChunk++;
		sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSamplePerChunk = 0;
		sRec_MP4Controller[ubCh].stStbl[ubPreStreamType].ulStscSampeDesID = 0;
		
		sRec_MP4Controller[ubCh].ubStreamType = ubStreamType;
		sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset = ulMP4_FileSize[ubCh];
	}
}

static void vMP4_UpdateVideoAtom(uint8_t ubCh,uint8_t ubStreamType)
{
	int32_t slLength = 0;

	if( ubStreamType == MP4_STR_V1)
	{
		// update tkhd atom
		vMP4_SetLong(MP4_V1TkhdAtom, MP4_TKHD_ID,1);
		vMP4_SetLong(MP4_V1TkhdAtom, MP4_TKHD_H_SIZE, (int32_t) (ulMP4_V1HSize[ubCh] << 16));
		vMP4_SetLong(MP4_V1TkhdAtom, MP4_TKHD_V_SIZE, (int32_t) (ulMP4_V1VSize[ubCh] << 16));

		// update stbl-stsd-avc1 atom
		vMP4_SetWord(MP4_V1Avc1Atom, MP4_AVC1_H_SIZE, (int16_t)ulMP4_V1HSize[ubCh]);
		vMP4_SetWord(MP4_V1Avc1Atom, MP4_AVC1_V_SIZE, (int16_t)ulMP4_V1VSize[ubCh]);
		vMP4_SetWord(MP4_V1Avc1Atom, 0x6C, ((int16_t)ulMP4_V1HSize[ubCh])>>1);
		vMP4_SetWord(MP4_V1Avc1Atom, 0x6E, (((ulMP4_V1VSize[ubCh])<<4) & 0xC000) ? ((ulMP4_V1VSize[ubCh]<<2)+0x32) : ((ulMP4_V1VSize[ubCh]<<4)+0xC8));

		// update stts atom, Duration all the same, Sample count = total frame
		vMP4_SetLong((uint8_t *)MP4_V1SttsAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry);
		vMP4_SetLong((uint8_t *)MP4_V1SttsAtom[ubCh], 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount);
		vMP4_SetLong((uint8_t *)MP4_V1SttsAtom[ubCh], 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration);

		// Update stss atom, save total I frame
		vMP4_SetLong((uint8_t *)MP4_V1StssAtom[ubCh], 0, (uint32_t)(16 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample));
		vMP4_SetLong((uint8_t *)MP4_V1StssAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample);		

		// Update stsc atom, save total chunk and last chunk id, sample pre chunk.
		if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk != 0)
		{
			if(	(sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscSamplePerChunk != sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscPreSamplePerChunk) ||
				(sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscSampeDesID != sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscPreSampeDesID) )
				{
					vMP4_UpdateStsc(ubCh, ubStreamType);
				}
		}

		// Update stsz atom, save total frame.
		vMP4_SetLong((uint8_t *)MP4_V1StszAtom[ubCh], 0, (uint32_t)(20 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry));
		vMP4_SetLong((uint8_t *)MP4_V1StszAtom[ubCh], 16, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry);

		// Update stco atom, save last chunk offset
		if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk != 0)
		{
			vMP4_SetLong((uint8_t *)MP4_V1StcoAtom[ubCh], 0, (uint32_t)(16 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry));
			vMP4_SetLong((uint8_t *)MP4_V1StcoAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry);
			vMP4_SetLong((uint8_t *)MP4_V1StcoAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset);
		}
		else
		{
			vMP4_SetLong((uint8_t *)MP4_V1StcoAtom[ubCh], 0, (uint32_t)(16 + 4 * (sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1)));
			vMP4_SetLong((uint8_t *)MP4_V1StcoAtom[ubCh], 12, (sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1));
		}

		vMP4_SetLong(MP4_V1MdhdAtom, MP4_MDHD_TIMESCALE, ulMdhdTimeScale[ubCh][ubStreamType]);
		vMP4_SetLong(MP4_V1MdhdAtom, MP4_MDHD_DURATION, ulMdhdDuration[ubCh][ubStreamType]);

		ulTrackDuration[ubCh][ubStreamType] = ulMdhdDuration[ubCh][ubStreamType];	
		vMP4_SetLong(MP4_V1ElstAtom, MP4_ELST_DURATION, ulTrackDuration[ubCh][ubStreamType]);
		vMP4_SetLong(MP4_V1TkhdAtom, MP4_TKHD_DURATION, ulTrackDuration[ubCh][ubStreamType]);

		slLength = ATOMHD;	// Stbl head
		slLength += ulMP4_GetAtomSize(MP4_V1StsdAtom);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V1SttsAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V1StssAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V1StscAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V1StszAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V1StcoAtom[ubCh]);
		vMP4_SetLong(MP4_V1StblAtom, 0, slLength);					//update size of stbl atom

		slLength = ATOMHD;	// Minf head
		slLength += ulMP4_GetAtomSize(MP4_V1VmhdAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1DinfAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1StblAtom);
		vMP4_SetLong(MP4_V1MinfAtom, 0, slLength);					//update size of minf atom

		slLength = ATOMHD;	// Mdia head. // ulMP4_GetAtomSize(MP4_V1MdiaAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1MdhdAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1HdlrAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1MinfAtom);
		vMP4_SetLong(MP4_V1MdiaAtom, 0, slLength);					//update size of mdia atom

		slLength = ATOMHD; // Edts head. // ulMP4_GetAtomSize(MP4_V1EdtsAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1ElstAtom);
		vMP4_SetLong(MP4_V1EdtsAtom, 0, slLength);					//update size of edts atom

		slLength = ATOMHD; // trak head. // ulMP4_GetAtomSize(MP4_V1TrakAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1TkhdAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1EdtsAtom);
		slLength += ulMP4_GetAtomSize(MP4_V1MdiaAtom);
		vMP4_SetLong(MP4_V1TrakAtom, 0, slLength);					//update size of trak atom
	}
	else if( ubStreamType == MP4_STR_V2)
	{
		// update tkhd atom
		vMP4_SetLong(MP4_V2TkhdAtom, MP4_TKHD_ID,3);
		vMP4_SetLong(MP4_V2TkhdAtom, MP4_TKHD_H_SIZE, (int32_t) (ulMP4_V2HSize[ubCh] << 16));
		vMP4_SetLong(MP4_V2TkhdAtom, MP4_TKHD_V_SIZE, (int32_t) (ulMP4_V2VSize[ubCh] << 16));

		// update stbl-stsd-avc1 atom
		vMP4_SetWord(MP4_V2Avc1Atom, MP4_AVC1_H_SIZE, (int16_t)ulMP4_V2HSize[ubCh]);
		vMP4_SetWord(MP4_V2Avc1Atom, MP4_AVC1_V_SIZE, (int16_t)ulMP4_V2VSize[ubCh]);
		vMP4_SetWord(MP4_V2Avc1Atom, 0x6C, ((int16_t)ulMP4_V2HSize[ubCh])>>1);
		vMP4_SetWord(MP4_V2Avc1Atom, 0x6E, (((ulMP4_V2VSize[ubCh])<<4) & 0xC000) ? ((ulMP4_V2VSize[ubCh]<<2)+0x32) : ((ulMP4_V2VSize[ubCh]<<4)+0xC8));

		// update stts atom, Duration all the same, Sample count = total frame
		vMP4_SetLong((uint8_t *)MP4_V2SttsAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry);
		vMP4_SetLong((uint8_t *)MP4_V2SttsAtom[ubCh], 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount);
		vMP4_SetLong((uint8_t *)MP4_V2SttsAtom[ubCh], 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration);

		// Update stss atom, save total I frame
		vMP4_SetLong((uint8_t *)MP4_V2StssAtom[ubCh], 0, (uint32_t)(16 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample));
		vMP4_SetLong((uint8_t *)MP4_V2StssAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample);			

		// Update stsc atom, save total chunk and last chunk id, sample pre chunk.
		if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk != 0)
		{
			if(	(sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscSamplePerChunk != sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscPreSamplePerChunk) ||
				(sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscSampeDesID != sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscPreSampeDesID) )
				{
					vMP4_UpdateStsc(ubCh, ubStreamType);
				}
		}

		// Update stsz atom, save total frame.
		vMP4_SetLong((uint8_t *)MP4_V2StszAtom[ubCh], 0, (uint32_t)(20 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry));
		vMP4_SetLong((uint8_t *)MP4_V2StszAtom[ubCh], 16, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry);

		// Update stco atom, save last chunk offset
		if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk != 0)
		{
			vMP4_SetLong((uint8_t *)MP4_V2StcoAtom[ubCh], 0, (uint32_t)(16 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry));
			vMP4_SetLong((uint8_t *)MP4_V2StcoAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry);
			vMP4_SetLong((uint8_t *)MP4_V2StcoAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset);
		}
		else
		{
			vMP4_SetLong((uint8_t *)MP4_V2StcoAtom[ubCh], 0, (uint32_t)(16 + 4 * (sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1)));
			vMP4_SetLong((uint8_t *)MP4_V2StcoAtom[ubCh], 12, (sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1));
		}

		vMP4_SetLong(MP4_V2MdhdAtom, MP4_MDHD_TIMESCALE, ulMdhdTimeScale[ubCh][ubStreamType]);
		vMP4_SetLong(MP4_V2MdhdAtom, MP4_MDHD_DURATION, ulMdhdDuration[ubCh][ubStreamType]);

		ulTrackDuration[ubCh][ubStreamType] = ulMdhdDuration[ubCh][ubStreamType];	
		vMP4_SetLong(MP4_V2ElstAtom, MP4_ELST_DURATION, ulTrackDuration[ubCh][ubStreamType]);
		vMP4_SetLong(MP4_V2TkhdAtom, MP4_TKHD_DURATION, ulTrackDuration[ubCh][ubStreamType]);

		slLength = ATOMHD;	// Stbl head
		slLength += ulMP4_GetAtomSize(MP4_V2StsdAtom);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V2SttsAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V2StssAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V2StscAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V2StszAtom[ubCh]);
		slLength += ulMP4_GetAtomSize((uint8_t *)MP4_V2StcoAtom[ubCh]);
		vMP4_SetLong(MP4_V2StblAtom, 0, slLength);					//update size of stbl atom

		slLength = ATOMHD;	// minf head
		slLength += ulMP4_GetAtomSize(MP4_V2VmhdAtom);
		slLength += ulMP4_GetAtomSize(MP4_V2DinfAtom);
		slLength += ulMP4_GetAtomSize(MP4_V2StblAtom);
		vMP4_SetLong(MP4_V2MinfAtom, 0, slLength);					//update size of minf atom

		slLength = ATOMHD;	// mdia head
		slLength += ulMP4_GetAtomSize(MP4_V2MdhdAtom);
		slLength += ulMP4_GetAtomSize(MP4_V2HdlrAtom);
		slLength += ulMP4_GetAtomSize(MP4_V2MinfAtom);
		vMP4_SetLong(MP4_V2MdiaAtom, 0, slLength);					//update size of mdia atom

		slLength = ATOMHD;	// edts head
		slLength += ulMP4_GetAtomSize(MP4_V2ElstAtom);
		vMP4_SetLong(MP4_V2EdtsAtom, 0, slLength);					//update size of edts atom

		slLength = ATOMHD;	// trak head
		slLength += ulMP4_GetAtomSize(MP4_V2TkhdAtom);
		slLength += ulMP4_GetAtomSize(MP4_V2EdtsAtom);
		slLength += ulMP4_GetAtomSize(MP4_V2MdiaAtom);
		vMP4_SetLong(MP4_V2TrakAtom, 0, slLength);					//update size of trak atom
	}
}

static void vMP4_UpdateAudioAtom(uint8_t ubCh,uint8_t ubStreamType)
{
	int32_t slLength = 0;
	// update tkhd atom

	// update stbl-stsd-mp4a atom

	// update stts atom, Duration all the same, Sample count = total frame
	vMP4_SetLong((uint8_t *)MP4_ASttsAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry);
	vMP4_SetLong((uint8_t *)MP4_ASttsAtom[ubCh], 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount);
	vMP4_SetLong((uint8_t *)MP4_ASttsAtom[ubCh], 16 + 8*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry - 1) + 4, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration);

	// Update stsc atom, save total chunk and last chunk id, sample pre chunk.
	if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk != 0)
	{
		if(	(sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscSamplePerChunk != sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscPreSamplePerChunk) ||
			(sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscSampeDesID != sRec_MP4Controller[ubCh].stStbl[sRec_MP4Controller[ubCh].ubStreamType].ulStscPreSampeDesID) )
		{
			vMP4_UpdateStsc(ubCh, ubStreamType);
		}
	}

	// Update stsz atom, save total frame.
	vMP4_SetLong((uint8_t *)MP4_AStszAtom[ubCh], 0, (uint32_t)(20 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry));
	vMP4_SetLong((uint8_t *)MP4_AStszAtom[ubCh], 16, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry);

	// Update stco atom, save last chunk offset
	if(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk != 0)
	{
		vMP4_SetLong((uint8_t *)MP4_AStcoAtom[ubCh], 0, (uint32_t)(16 + 4 * sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry));
		vMP4_SetLong((uint8_t *)MP4_AStcoAtom[ubCh], 12, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry);
		vMP4_SetLong((uint8_t *)MP4_AStcoAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset);
	}
	else
	{
		vMP4_SetLong((uint8_t *)MP4_AStcoAtom[ubCh], 0, (uint32_t)(16 + 4 * (sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry - 1)));
		vMP4_SetLong((uint8_t *)MP4_AStcoAtom[ubCh], 12, (sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry -1 ));
	}

	ulTrackDuration[ubCh][ubStreamType] = ((ulMdhdDuration[ubCh][ubStreamType]*ulMvhdTimeScale[ubCh])/ulMdhdTimeScale[ubCh][ubStreamType]);
	
	vMP4_SetLong(MP4_ATkhdAtom, MP4_TKHD_ID,2);
	vMP4_SetLong(MP4_ATkhdAtom, MP4_TKHD_DURATION, ulTrackDuration[ubCh][ubStreamType]);
	vMP4_SetLong(MP4_AElstAtom, MP4_ELST_DURATION, ulTrackDuration[ubCh][ubStreamType]);
	vMP4_SetLong(MP4_AMdhdAtom, MP4_MDHD_TIMESCALE, ulMdhdTimeScale[ubCh][ubStreamType]);
	vMP4_SetLong(MP4_AMdhdAtom, MP4_MDHD_DURATION, ulMdhdDuration[ubCh][ubStreamType]);

	slLength = ATOMHD;	// Stbl head
	slLength += ulMP4_GetAtomSize(MP4_AStsdAtom);
	slLength += ulMP4_GetAtomSize((uint8_t *)MP4_ASttsAtom[ubCh]);
	slLength += ulMP4_GetAtomSize((uint8_t *)MP4_AStscAtom[ubCh]);
	slLength += ulMP4_GetAtomSize((uint8_t *)MP4_AStszAtom[ubCh]);
	slLength += ulMP4_GetAtomSize((uint8_t *)MP4_AStcoAtom[ubCh]);
	vMP4_SetLong(MP4_AStblAtom, 0, slLength);					//update size of stbl atom

	slLength = ATOMHD;	// minf head
	slLength += ulMP4_GetAtomSize(MP4_SmhdAtom);
	slLength += ulMP4_GetAtomSize(MP4_ADinfAtom);
	slLength += ulMP4_GetAtomSize(MP4_AStblAtom);
	vMP4_SetLong(MP4_AMinfAtom, 0, slLength);					//update size of minf atom

	slLength = ATOMHD;	// mdia head
	slLength += ulMP4_GetAtomSize(MP4_AMdhdAtom);
	slLength += ulMP4_GetAtomSize(MP4_AHdlrAtom);
	slLength += ulMP4_GetAtomSize(MP4_AMinfAtom);
	vMP4_SetLong(MP4_AMdiaAtom, 0, slLength);					//update size of mdia atom

	slLength = ATOMHD;	// edts head
	slLength += ulMP4_GetAtomSize(MP4_AElstAtom);
	vMP4_SetLong(MP4_AEdtsAtom, 0, slLength);					//update size of edts atom

	slLength = ATOMHD;	// trak head
	slLength += ulMP4_GetAtomSize(MP4_ATkhdAtom);
	slLength += ulMP4_GetAtomSize(MP4_AEdtsAtom);
	slLength += ulMP4_GetAtomSize(MP4_AMdiaAtom);
	vMP4_SetLong(MP4_ATrakAtom, 0, slLength);					//update size of trak atom
}

static uint32_t ulMP4_AllocateMemBuf(uint32_t ulSize)
{
	uint32_t ulAdr = 0;
	ulAdr = ulMP4_MemStartAddr;
	ulMP4_MemStartAddr += ulSize;
	return ulAdr;
}

uint16_t uwMP4_GetVersion(void)
{
    return ((MP4_MAJORVER << 8) + MP4_MINORVER);
}

void MP4_SemaphoreCreate(void)
{
	osSemaphoreDef(SEM_MP4_WRFrm);
	SEM_MP4_WRFrm    = osSemaphoreCreate(osSemaphore(SEM_MP4_WRFrm), 1);
}

void MP4_SetConfigedSrcNum(uint8_t ubSrcNum)
{
	ubMP4_CfgSrcNum = ubSrcNum;
}

uint32_t ulMP4_GetBufSz(void)
{
	uint8_t i;
	uint8_t ubSourceType;
	uint32_t ulSize = 0;

	ulSize += MP4_STBL_1MINSZ(ubMP4_MaxVFPS,ubMP4_MaxAFPS)*ulMP4_RecTime +MP4_MOOV_HDSIZE;
	
	for(i=0;i<ubMP4_CfgSrcNum;i++)
	{
		ubSourceType = ubMP4_SourceType[i];

		if( ubSourceType == MEDIA_SRC_1V0A || ubSourceType == MEDIA_SRC_1V1A || ubSourceType == MEDIA_SRC_2V1A )
		{
			ulSize += MP4_STTS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STSS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STSC_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STSZ_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STCO_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
		}
		if( ubSourceType == MEDIA_SRC_1V1A || ubSourceType == MEDIA_SRC_2V1A )
		{
			ulSize += MP4_STTS_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime);
			ulSize += MP4_STSC_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime);
			ulSize += MP4_STSZ_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime);
			ulSize += MP4_STCO_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime);
		}
		if( ubSourceType == MEDIA_SRC_2V1A )
		{
			ulSize += MP4_STTS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STSS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STSC_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STSZ_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
			ulSize += MP4_STCO_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime);
		}
	}
	ulSize += MP4_THUMBNAIL_SIZE;
	return ulSize;
}

void MP4_Init(uint32_t ulStartAdr)
{
	uint8_t i;
	uint8_t ubSourceType;

	ulMP4_MemStartAddr = ulStartAdr;

	ubSourceType = ubMP4_SourceType[0];
	ulMP4_StartAddr[0] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STBL_1MINSZ(ubMP4_MaxVFPS,ubMP4_MaxAFPS)*ulMP4_RecTime+MP4_MOOV_HDSIZE);
	
	printf("== MP4_Init ==\r\n");
	for( i = 0 ; i < ubMP4_CfgSrcNum ; i++)
	{
		ubSourceType = ubMP4_SourceType[i];
		
		ulMP4_StartAddr[i] = ulMP4_StartAddr[0];
		printf("Ch:%d Adr: 0x%x \r\n",i, ulMP4_StartAddr[i]);

		if( ubSourceType == MEDIA_SRC_1V0A || ubSourceType == MEDIA_SRC_1V1A || ubSourceType == MEDIA_SRC_2V1A )
	 	{
	 		// video1
			MP4_V1SttsAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STTS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
	 		MP4_V1StssAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STSS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
	 		MP4_V1StscAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STSC_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
	 		MP4_V1StszAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STSZ_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
	 		MP4_V1StcoAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STCO_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
	 		printf("V1(ts:0x%x ss:0x%x sc:0x%x sz:0x%x co:0x%x)\r\n",MP4_V1SttsAtom[i], MP4_V1StssAtom[i],MP4_V1StscAtom[i],MP4_V1StszAtom[i],MP4_V1StcoAtom[i]);
		}
		if( ubSourceType == MEDIA_SRC_1V1A || ubSourceType == MEDIA_SRC_2V1A )
	 	{
	 		// audio
			MP4_ASttsAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STTS_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime));
			MP4_AStscAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STSC_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime));
			MP4_AStszAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STSZ_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime));
			MP4_AStcoAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STCO_ATOM_SIZE(ubMP4_MaxAFPS,ulMP4_RecTime));
			printf("A (ts:0x%x             sc:0x%x sz:0x%x co:0x%x)\r\n",MP4_ASttsAtom[i], MP4_AStscAtom[i],MP4_AStszAtom[i],MP4_AStcoAtom[i]);
		}
	 	if( ubSourceType == MEDIA_SRC_2V1A )
		{
			// video1
			MP4_V2SttsAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STTS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
			MP4_V2StssAtom[i] =(uint32_t)ulMP4_AllocateMemBuf(MP4_STSS_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
			MP4_V2StscAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STSC_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
			MP4_V2StszAtom[i] =(uint32_t)ulMP4_AllocateMemBuf(MP4_STSZ_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
			MP4_V2StcoAtom[i] = (uint32_t)ulMP4_AllocateMemBuf(MP4_STCO_ATOM_SIZE(ubMP4_MaxVFPS,ulMP4_RecTime));
			printf("V2(ts:0x%x ss:0x%x sc:0x%x sz:0x%x co:0x%x)\r\n",MP4_V2SttsAtom[i], MP4_V2StssAtom[i],MP4_V2StscAtom[i],MP4_V2StszAtom[i],MP4_V2StcoAtom[i]);
	 	}
		printf("\r\n");
	 	memset(&sRec_MP4Controller[i], 0, sizeof(REC_CHUNK_CONTROLLER));
	}
	ulMP4ThumbnailAdr = (uint32_t)ulMP4_AllocateMemBuf(MP4_THUMBNAIL_SIZE);
}

void MP4_SetSourceType(uint8_t ubCh, uint8_t ubSourceType)
{
	ubMP4_SourceType[ubCh] = ubSourceType;
}

void MP4_Vdo1SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval)
{
	ulMvhdTimeScale[ubCh] = 1000;
	ulMP4_V1HSize[ubCh] = ulHSize;
	ulMP4_V1VSize[ubCh] = ulVSize;
	ulMdhdTimeScale[ubCh][MP4_STR_V1] = ulMvhdTimeScale[ubCh];	
	uwMP4_FrmDuration[ubCh][MP4_STR_V1] = uwFrmInterval;	
}

void MP4_Vdo2SetFormat(uint8_t ubCh, uint32_t ulHSize, uint32_t ulVSize, uint16_t uwFrmInterval)
{
	ulMP4_V2HSize[ubCh] = ulHSize;
	ulMP4_V2VSize[ubCh] = ulVSize;
	//uwMP4_FrmDuration[ubCh][MP4_STR_V2] = uwFrmInterval;
	//ulMdhdTimeScale[ubCh][MP4_STR_V2] = ulMvhdTimeScale[ubCh];
}

void MP4_AdoSetFormat(uint8_t ubCh,uint32_t ulSampleRate, uint16_t uwBlockAlign, uint16_t uwFrmInterval)
{
	ulMP4_AdoSampleRate[ubCh] = ulSampleRate;
	uwMP4_AdoBlockAlign[ubCh] = uwBlockAlign;
	uwMP4_AdoSamplesPerBlock[ubCh] = (((uwBlockAlign-7)*8)/4) + 2;
	ulMdhdTimeScale[ubCh][MP4_STR_A1] = ulSampleRate;
	uwMP4_FrmDuration[ubCh][MP4_STR_A1] = (uwFrmInterval*ulSampleRate) / 10000;
}

uint32_t ulMP4_GetFixedMemorySize(void)
{
	return (MP4_STBL_HDSIZE*ubMP4_CfgSrcNum + MP4_MOOV_HDSIZE + MP4_THUMBNAIL_SIZE);
}

void MP4_SetRecMaxTime(uint8_t ubVFPS, uint8_t ubAFPS, uint32_t ulMinute)
{
	ubMP4_MaxVFPS = ubVFPS;	
	ubMP4_MaxAFPS = ubAFPS;
	ulMP4_RecTime = ulMinute;
}

void MP4_CreateFile(uint8_t ubCh)
{
	uint8_t ubSourceType;
	uint32_t ulSize = 0;
	
	memset(&sRec_MP4Controller[ubCh], 0, sizeof(REC_CHUNK_CONTROLLER));

	memset(&ulTrackDuration[ubCh][0], 0 , sizeof(uint32_t)*MP4_STR_MAX);
	memset(&ulMdhdDuration[ubCh][0], 0 , sizeof(uint32_t)*MP4_STR_MAX);

	ubSourceType = ubMP4_SourceType[ubCh];
	
	if(	ubSourceType == MEDIA_SRC_1V0A || ubSourceType == MEDIA_SRC_1V1A || ubSourceType == MEDIA_SRC_2V1A )
		vMP4_Stbl_Init(ubCh,MP4_STR_V1);
	
	if(	ubSourceType == MEDIA_SRC_1V1A || ubSourceType == MEDIA_SRC_2V1A )
		vMP4_Stbl_Init(ubCh,MP4_STR_A1);
	
	if(	ubSourceType == MEDIA_SRC_2V1A )
		vMP4_Stbl_Init(ubCh,MP4_STR_V2);

	ulMP4_FileSize[ubCh] = 0x30;
	ulMP4_AtomSize[ubCh] = 0;
	ulMP4_ThumbnailOffset[ubCh] = 0;

	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh]), MP4_FtypAtom, sizeof(MP4_FtypAtom));
	ulSize = sizeof(MP4_FtypAtom);
	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh]+ulSize), MP4_FreeAtom, sizeof(MP4_FreeAtom));
	ulSize = sizeof(MP4_FreeAtom);
	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh] + ulSize), MP4_MdatAtom, sizeof(MP4_MdatAtom));
	
	FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(ulMP4_StartAddr[ubCh]) , ulMP4_FileSize[ubCh]);
}

int32_t slMP4_WriteOneFrame(uint8_t ubCh, uint8_t ubStreamType,uint8_t ubFrameType, uint32_t ulAddr, uint32_t ulSize, uint32_t ulDesID)
{
	//printf("\r\nWF ST:%d FT:%d Sz:%d\r\n",ubStreamType,ubFrameType,ulSize);
	if(osSemaphoreWait(SEM_MP4_WRFrm ,3) != osOK)
	{
		printf("SEM_MP4_WRFrm Busy\r\n");
		return -10;
	}

	if(ubStreamType == MP4_STR_A1)
	{
		ulMdhdDuration[ubCh][ubStreamType] += uwMP4_FrmDuration[ubCh][ubStreamType];
		
		// Inturrupt video contineous, add chunk to next.
		vMP4_CheckSourceSWUpdateChunk(ubCh,ubStreamType);		
		
		if(	(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0) || 
			( (sRec_MP4Controller[ubCh].ubChunkStartFlag == 1) && (sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] == 0)))
		{
			if(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0)
				sRec_MP4Controller[ubCh].ubStreamType = ubStreamType;
			
			sRec_MP4Controller[ubCh].ubChunkStartFlag = 1;
			sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] = 1;
			
			// First Audio Input
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry = 1;								// Initial Stts
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount = 1;
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration = uwMP4_FrmDuration[ubCh][ubStreamType];
			
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry = 1;								// stsc
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk = 1;
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk = 1;
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID= ulDesID;
			
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry = 1;						//stsz
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize = ulSize;

			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry = 1;					// stco
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset = ulMP4_FileSize[ubCh];
			sRec_MP4Controller[ubCh].ubStreamType = ubStreamType;
			//printf("ST:%d LEN:0x%x\r\n",ubStreamType,ulMP4_FileSize[ubCh]);
		}
		else
		{
			vMP4_CheckUpdateStts(ubCh,(uint8_t *)MP4_ASttsAtom[ubCh],ubStreamType,uwMP4_FrmDuration[ubCh][ubStreamType]);	// stts
			vMP4_CheckUpdateStsc(ubCh,(uint8_t *)MP4_AStscAtom[ubCh],(uint8_t *)MP4_AStcoAtom[ubCh]	,ubStreamType,ulDesID);		// stsc,stco

			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry++;
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId++;
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize = ulSize;
		}
		
		//update audio stsz atom, save audio Sample Size
		vMP4_SetLong((uint8_t *)MP4_AStszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
		
		//save audio mdat data
		FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)ulAddr, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
		ulMP4_FileSize[ubCh] += sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize;
	}
	else if((ubStreamType == MP4_STR_V1) || (ubStreamType == MP4_STR_V2))
	{
		//printf("Z:%d %d %d\r\n",ubFrameType ,sRec_MP4Controller[ubCh].ubChunkStartFlag, sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType]);
		vMP4_CheckSourceSWUpdateChunk(ubCh,ubStreamType);

		if(ubFrameType == MP4_V_IFRM)	// I frame
		{
			ulMdhdDuration[ubCh][ubStreamType] += uwMP4_FrmDuration[ubCh][ubStreamType];
			
			if( 	(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0) || 
				( (sRec_MP4Controller[ubCh].ubChunkStartFlag == 1) && (sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] == 0)))
			{
				if(sRec_MP4Controller[ubCh].ubChunkStartFlag == 0)
					sRec_MP4Controller[ubCh].ubStreamType = ubStreamType;
				
				sRec_MP4Controller[ubCh].ubChunkStartFlag = 1;
				sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] = 1;
				
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry = 1;									// stts
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount = 1;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration = uwMP4_FrmDuration[ubCh][ubStreamType];

				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample = 1;								// stss
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId = 1;
				
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry = 1;									// stsc
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk = 1;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk = 1;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID = ulDesID;
				
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry = 1;							// stsz
									
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry = 1;						// stco		
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset = ulMP4_FileSize[ubCh];
				//printf("ST:%d LEN:0x%x\r\n",ubStreamType,ulMP4_FileSize[ubCh]);
			}
			else	
			{
				if(ubStreamType == MP4_STR_V1)
				{
					vMP4_CheckUpdateStts(ubCh,(uint8_t *)MP4_V1SttsAtom[ubCh],ubStreamType,uwMP4_FrmDuration[ubCh][ubStreamType]);			// stts
					vMP4_CheckUpdateStsc(ubCh,(uint8_t *)MP4_V1StscAtom[ubCh],(uint8_t *)MP4_V1StcoAtom[ubCh],ubStreamType,ulDesID);		// stsc,stco
				}
				else if (ubStreamType == MP4_STR_V2)
				{
					vMP4_CheckUpdateStts(ubCh,(uint8_t *)MP4_V2SttsAtom[ubCh],ubStreamType,uwMP4_FrmDuration[ubCh][ubStreamType]);					// stts
					vMP4_CheckUpdateStsc(ubCh,(uint8_t *)MP4_V2StscAtom[ubCh],(uint8_t *)MP4_V2StcoAtom[ubCh],ubStreamType,ulDesID);		// stsc,stco
				}
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample++;					// stss
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId++;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry++;				// stsz
			}
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize = ulSize;

			// mdat
			sRec_MP4Controller[ubCh].ulmdatSampleSize[ubStreamType] = ulSize-4-(0xa+0x4+8);

			if(ubStreamType == MP4_STR_V1)
			{
				//update stss atom, save I-Frame Sample ID
				vMP4_SetLong((uint8_t *)MP4_V1StssAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId);

				//update stsz atom, save Sample Size
				vMP4_SetLong((uint8_t *)MP4_V1StszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			}
			else if (ubStreamType == MP4_STR_V2)
			{
				//update stss atom, save I-Frame Sample ID
				vMP4_SetLong((uint8_t *)MP4_V2StssAtom[ubCh], 16 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId);
				
				//update stsz atom, save Sample Size
				vMP4_SetLong((uint8_t *)MP4_V2StszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			}
			
			//transfer bitstream to MP4 format
			//*(uint8_t *)(ulAddr+3) = 0x0A;	//0xA: SPS size
			//*(uint8_t *)(ulAddr+17) = 0x04;	//0x4: PPS size
			//vMP4_SetLong((uint8_t *)ulAddr, 22, sRec_MP4Controller[ubCh].ulmdatSampleSize[ubStreamType]); //4: size field, 0xa+0x4+4+4: SPS+PPS+two size fields
			
			ubMP4_PIC[ubCh] = 0 + 2;
			ubMP4_POC[ubCh] = 1 + 2;
			
			//save mdat data
			FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)ulAddr, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			ulMP4_FileSize[ubCh] += sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize;
		}
		else	if((ubFrameType == MP4_V_PFRM) && (sRec_MP4Controller[ubCh].ubChunkStartFlag == 1) && (sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] == 1))	// P frame
		{
			ulMdhdDuration[ubCh][ubStreamType] += uwMP4_FrmDuration[ubCh][ubStreamType];
			
			if(ubStreamType == MP4_STR_V1)
			{
				vMP4_CheckUpdateStts(ubCh,(uint8_t *)MP4_V1SttsAtom[ubCh],ubStreamType,uwMP4_FrmDuration[ubCh][ubStreamType]);			// stts
				vMP4_CheckUpdateStsc(ubCh,(uint8_t *)MP4_V1StscAtom[ubCh],(uint8_t *)MP4_V1StcoAtom[ubCh],ubStreamType,ulDesID);		// stsc,stco
			}
			else if(ubStreamType == MP4_STR_V2)
			{
				vMP4_CheckUpdateStts(ubCh,(uint8_t *)MP4_V2SttsAtom[ubCh],ubStreamType,uwMP4_FrmDuration[ubCh][ubStreamType]);			// stts
				vMP4_CheckUpdateStsc(ubCh,(uint8_t *)MP4_V2StscAtom[ubCh],(uint8_t *)MP4_V2StcoAtom[ubCh],ubStreamType,ulDesID);		// stsc,stco
			}

			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId++;				// stss
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry++;			// stsz
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize = ulSize;
			sRec_MP4Controller[ubCh].ulmdatSampleSize[ubStreamType] = ulSize-4;	// mdat

			//update stsz atom, save Sample Size
			if(ubStreamType == MP4_STR_V1)
				vMP4_SetLong((uint8_t *)MP4_V1StszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);			
			else if(ubStreamType == MP4_STR_V2)
				vMP4_SetLong((uint8_t *)MP4_V2StszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);	

			//transfer bitstream to MP4 format
			//vMP4_SetLong((uint8_t *)ulAddr, 0, sRec_MP4Controller[ubCh].ulmdatSampleSize[ubStreamType]);

			*(uint8_t *)(ulAddr+6) = ubMP4_PIC[ubCh];
			*(uint8_t *)(ulAddr+7) = ubMP4_POC[ubCh];
			ubMP4_PIC[ubCh] += 2;
			ubMP4_POC[ubCh] += 2;

			//save mdat data
			FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)ulAddr, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			ulMP4_FileSize[ubCh] += sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize;
		}
		else if((ubFrameType == MP4_V_SFRM) && (sRec_MP4Controller[ubCh].ubChunkStartFlag == 1))		//skip frame
		{
			ulMdhdDuration[ubCh][ubStreamType] += uwMP4_FrmDuration[ubCh][ubStreamType];
			
			ulSize = 36;
			ulAddr = (uint32_t)MP4_VSkipFrame;

			if(sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] == 0)
			{
				sRec_MP4Controller[ubCh].ubVideoAudioRunFg[ubStreamType] = 1;
				
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsEntry = 1;									// stts
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleCount = 1;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulSttsSampleDuration = uwMP4_FrmDuration[ubCh][ubStreamType];

				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSyncSample = 0;								// stss
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId = 1;
				
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscEntry = 1;									// stsc
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscFirstChunk = 1;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSamplePerChunk = 1;
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStscSampeDesID = ulDesID;
				
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry = 1;							// stsz
									
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffsetEntry = 1;						// stco		
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStcoChunkOffset = ulMP4_FileSize[ubCh];
			}
			else
			{
				vMP4_CheckUpdateStts(ubCh, (uint8_t *)MP4_V1SttsAtom[ubCh], ubStreamType,uwMP4_FrmDuration[ubCh][ubStreamType]);			// stts
				vMP4_CheckUpdateStsc(ubCh,(uint8_t *)MP4_V1StscAtom[ubCh],(uint8_t *)MP4_V1StcoAtom[ubCh],ubStreamType,ulDesID);		// stsc,stco

				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStssSampleId++;				// stss
				sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry++;		// stsz
			}
			
			sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize = ulSize;
			sRec_MP4Controller[ubCh].ulmdatSampleSize[ubStreamType] = ulSize - 4;		// mdat

			//update stsz atom
			if(ubStreamType == MP4_STR_V1)
				vMP4_SetLong((uint8_t *)MP4_V1StszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			else if(ubStreamType == MP4_STR_V2)
				vMP4_SetLong((uint8_t *)MP4_V2StszAtom[ubCh], 20 + 4*(sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSizeEntry - 1), sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			
			//transfer bitstream to MP4 format
			//vMP4_SetLong((uint8_t *)ulAddr, 0, sRec_MP4Controller[ubCh].ulmdatSampleSize[ubStreamType]);
			
			*(uint8_t *)(ulAddr+6) = ubMP4_PIC[ubCh];
			*(uint8_t *)(ulAddr+7) = ubMP4_POC[ubCh];
			ubMP4_POC[ubCh] += 2;
			if (ubMP4_POC[ubCh] == 0x1)			//POC is overflow.
			{
				if ((ubMP4_PIC[ubCh] % 2) == 0)		//PIC is even value normally.
					ubMP4_PIC[ubCh] += 1;		//if POC overflow was odd times(first, third...), PIC change to odd value.
				else
					ubMP4_PIC[ubCh] -= 1;			//if POC overflow was even times(second, fourth...), PIC back to even value.
			}			

			//save mdat data
			FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)ulAddr, sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize);
			ulMP4_FileSize[ubCh] += sRec_MP4Controller[ubCh].stStbl[ubStreamType].ulStszSampleSize;
		}
	}
	osSemaphoreRelease(SEM_MP4_WRFrm);
	return 0;
}

void MP4_Close(uint8_t ubCh)
{
	int32_t slLength = 0;
	uint32_t i = 0;
	uint32_t ulTemp = 0;
	uint32_t ulmdatSz = 0;
	uint32_t ulQuot = 0;
	uint32_t ulRem = 0;
		
	// Update mdata Size
	ulmdatSz = ulMP4_FileSize[ubCh] - 0x28;
	vMP4_SetLong(MP4_MdatAtom, 0, ulmdatSz);

	// Update stream info.
	vMP4_UpdateVideoAtom(ubCh,MP4_STR_V1);
	vMP4_UpdateAudioAtom(ubCh,MP4_STR_A1);
	if(	ubMP4_SourceType[ubCh] == MEDIA_SRC_2V1A )
		vMP4_UpdateVideoAtom(ubCh,MP4_STR_V2);
	
	// Update Moov info
	slLength = ATOMHD;	// Moov head
	slLength += ulMP4_GetAtomSize(MP4_MvhdAtom);
	slLength += ulMP4_GetAtomSize(MP4_V1TrakAtom);

	if(	ubMP4_SourceType[ubCh] == MEDIA_SRC_2V1A || ubMP4_SourceType[ubCh] == MEDIA_SRC_1V1A)
		slLength += ulMP4_GetAtomSize(MP4_ATrakAtom);
	if(	ubMP4_SourceType[ubCh] == MEDIA_SRC_2V1A )
		slLength += ulMP4_GetAtomSize(MP4_V2TrakAtom);

	// Thumbnail Size
	slLength += MP4_THUMBNAIL_SIZE;
	
	vMP4_SetLong(MP4_MoovAtom, 0, slLength);					//update size of moov atom
	vMP4_SaveAtom(ubCh,MP4_MoovAtom, sizeof(MP4_MoovAtom));

	// Update mvhd info
	for( i = 0 ; i < MP4_STR_MAX ; i++)
	{
		if( ulTrackDuration[ubCh][i] > ulTemp)
			ulTemp = ulTrackDuration[ubCh][i];
	}
	vMP4_SetLong(MP4_MvhdAtom, MP4_MVHD_TIMESCALE, ulMvhdTimeScale[ubCh]);
	vMP4_SetLong(MP4_MvhdAtom, MP4_MVHD_DURATION, ulTemp);
	vMP4_SetLong(MP4_MvhdAtom, MP4_MVHD_RESERVE, ulMP4_DelayTime[ubCh]);
	vMP4_SetLong(MP4_MvhdAtom, MP4_MVHD_NEXT_THKID, MP4_STR_MAX+1);
	vMP4_SaveAtom(ubCh,MP4_MvhdAtom, sizeof(MP4_MvhdAtom));	

	// Update Video1 track info
	vMP4_SaveAtom(ubCh,MP4_V1TrakAtom, sizeof(MP4_V1TrakAtom));
	vMP4_SaveAtom(ubCh,MP4_V1TkhdAtom, sizeof(MP4_V1TkhdAtom));
	vMP4_SaveAtom(ubCh,MP4_V1EdtsAtom, sizeof(MP4_V1EdtsAtom));
	vMP4_SaveAtom(ubCh,MP4_V1ElstAtom, sizeof(MP4_V1ElstAtom));
	vMP4_SaveAtom(ubCh,MP4_V1MdiaAtom, sizeof(MP4_V1MdiaAtom));
	vMP4_SaveAtom(ubCh,MP4_V1MdhdAtom, sizeof(MP4_V1MdhdAtom));
	vMP4_SaveAtom(ubCh,MP4_V1HdlrAtom, sizeof(MP4_V1HdlrAtom));
	vMP4_SaveAtom(ubCh,MP4_V1MinfAtom, sizeof(MP4_V1MinfAtom));
	vMP4_SaveAtom(ubCh,MP4_V1VmhdAtom, sizeof(MP4_V1VmhdAtom));
	vMP4_SaveAtom(ubCh,MP4_V1DinfAtom, sizeof(MP4_V1DinfAtom));
	vMP4_SaveAtom(ubCh,MP4_V1StblAtom, sizeof(MP4_V1StblAtom));
	vMP4_SaveAtom(ubCh,MP4_V1StsdAtom, sizeof(MP4_V1StsdAtom));
	vMP4_SaveAtom(ubCh,MP4_V1Avc1Atom, sizeof(MP4_V1Avc1Atom));
	vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V1SttsAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V1SttsAtom[ubCh]));
	vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V1StssAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V1StssAtom[ubCh]));
	vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V1StscAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V1StscAtom[ubCh]));
	vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V1StszAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V1StszAtom[ubCh]));
	vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V1StcoAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V1StcoAtom[ubCh]));

	// Update Audio track info
	if(	ubMP4_SourceType[ubCh] == MEDIA_SRC_1V1A || ubMP4_SourceType[ubCh] == MEDIA_SRC_2V1A)
	{
		vMP4_SaveAtom(ubCh,MP4_ATrakAtom, sizeof(MP4_ATrakAtom));
		vMP4_SaveAtom(ubCh,MP4_ATkhdAtom, sizeof(MP4_ATkhdAtom));
		vMP4_SaveAtom(ubCh,MP4_AEdtsAtom, sizeof(MP4_AEdtsAtom));
		vMP4_SaveAtom(ubCh,MP4_AElstAtom, sizeof(MP4_AElstAtom));
		vMP4_SaveAtom(ubCh,MP4_AMdiaAtom, sizeof(MP4_AMdiaAtom));
		vMP4_SaveAtom(ubCh,MP4_AMdhdAtom, sizeof(MP4_AMdhdAtom));
		vMP4_SaveAtom(ubCh,MP4_AHdlrAtom, sizeof(MP4_AHdlrAtom));
		vMP4_SaveAtom(ubCh,MP4_AMinfAtom, sizeof(MP4_AMinfAtom));
		vMP4_SaveAtom(ubCh,MP4_SmhdAtom, sizeof(MP4_SmhdAtom));
		vMP4_SaveAtom(ubCh,MP4_ADinfAtom, sizeof(MP4_ADinfAtom));
		vMP4_SaveAtom(ubCh,MP4_AStblAtom, sizeof(MP4_AStblAtom));
		vMP4_SaveAtom(ubCh,MP4_AStsdAtom, sizeof(MP4_AStsdAtom));
		vMP4_SaveAtom(ubCh,MP4_Mp4aAtom, sizeof(MP4_Mp4aAtom));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_ASttsAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_ASttsAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_AStscAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_AStscAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_AStszAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_AStszAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_AStcoAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_AStcoAtom[ubCh]));
	}

	// Update Video2 track info
	if(	ubMP4_SourceType[ubCh] == MEDIA_SRC_2V1A )
	{
		vMP4_SaveAtom(ubCh,MP4_V2TrakAtom, sizeof(MP4_V2TrakAtom));
		vMP4_SaveAtom(ubCh,MP4_V2TkhdAtom, sizeof(MP4_V2TkhdAtom));
		vMP4_SaveAtom(ubCh,MP4_V2EdtsAtom, sizeof(MP4_V2EdtsAtom));
		vMP4_SaveAtom(ubCh,MP4_V2ElstAtom, sizeof(MP4_V2ElstAtom));
		vMP4_SaveAtom(ubCh,MP4_V2MdiaAtom, sizeof(MP4_V2MdiaAtom));
		vMP4_SaveAtom(ubCh,MP4_V2MdhdAtom, sizeof(MP4_V2MdhdAtom));
		vMP4_SaveAtom(ubCh,MP4_V2HdlrAtom, sizeof(MP4_V2HdlrAtom));
		vMP4_SaveAtom(ubCh,MP4_V2MinfAtom, sizeof(MP4_V2MinfAtom));
		vMP4_SaveAtom(ubCh,MP4_V2VmhdAtom, sizeof(MP4_V2VmhdAtom));
		vMP4_SaveAtom(ubCh,MP4_V2DinfAtom, sizeof(MP4_V2DinfAtom));
		vMP4_SaveAtom(ubCh,MP4_V2StblAtom, sizeof(MP4_V2StblAtom));
		vMP4_SaveAtom(ubCh,MP4_V2StsdAtom, sizeof(MP4_V2StsdAtom));
		vMP4_SaveAtom(ubCh,MP4_V2Avc1Atom, sizeof(MP4_V2Avc1Atom));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V2SttsAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V2SttsAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V2StssAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V2StssAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V2StscAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V2StscAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V2StszAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V2StszAtom[ubCh]));
		vMP4_SaveAtom(ubCh,(uint8_t *)MP4_V2StcoAtom[ubCh], ulMP4_GetAtomSize((uint8_t *)MP4_V2StcoAtom[ubCh]));		
	}

	// write moov data to storeage.
	ulQuot = (ulMP4_AtomSize[ubCh] / 0x20000);
	ulRem = (ulMP4_AtomSize[ubCh] % 0x20000);
	printf("Q:0x%x R:0x%x Sz:0x%x\r\n",ulQuot,ulRem,ulMP4_AtomSize[ubCh]);
	
	for(i = 0; i < ulQuot ; i++)
		FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(ulMP4_StartAddr[ubCh] +  (0x20000 * i)), 0x20000);	// 128K
	
	if(ulRem > 0)
		FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)(ulMP4_StartAddr[ubCh] +  (0x20000 * i)), ulRem);	// 128K

	// write thumbnail to storeage.
	ulMP4_ThumbnailOffset[ubCh] = ulMP4_FileSize[ubCh];
	ulMP4_FileSize[ubCh] += MP4_THUMBNAIL_SIZE;
	vMP4_SetLong(MP4_UdtaAtom, 0, MP4_THUMBNAIL_SIZE);
	memcpy((uint8_t *)ulMP4ThumbnailAdr, MP4_UdtaAtom, sizeof(MP4_UdtaAtom));
	
	vMP4_SetLong(MP4_DataAtom, 0, (MP4_THUMBNAIL_SIZE - 0x08));
	memcpy((uint8_t *)(ulMP4ThumbnailAdr+0x08), MP4_DataAtom, sizeof(MP4_DataAtom));
	FS_WriteFile((FS_SRC_NUM)ubCh, (uint32_t)ulMP4ThumbnailAdr, MP4_THUMBNAIL_SIZE);
	

	// update mdate size.
	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh]), MP4_FtypAtom, sizeof(MP4_FtypAtom));
	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh]+0x20),MP4_FreeAtom, sizeof(MP4_FreeAtom));
	memcpy((uint8_t *)(ulMP4_StartAddr[ubCh] + 0x28), MP4_MdatAtom, sizeof(MP4_MdatAtom));
	FS_UpdateMp4Header((FS_SRC_NUM)ubCh,(uint32_t)ulMP4_StartAddr[ubCh], 0x30);
	FS_CloseFile((FS_SRC_NUM)ubCh);
	
	printf("MP4(FSz:0x%x ASz:0x%x)\n\r", ulMP4_FileSize[ubCh], ulMP4_AtomSize[ubCh]);
	printf("\r\n");
}

uint16_t uwMP4_MovieLength(uint8_t ubCh)
{
	return (ulMdhdDuration[ubCh][MP4_STR_V1] /ulMdhdTimeScale[ubCh][MP4_STR_V1]);
}

uint32_t ulMP4_GetSkipFrameSize(void)
{
	return sizeof(MP4_VSkipFrame);
}

void MP4_WriteInfoData(uint8_t ubCh, uint8_t ubOffset, uint8_t *ptr, uint8_t ubSize)
{
	ulMP4_DelayTime[ubCh] = *(uint32_t*)ptr;
}

void MP4_AssignmentStructure(uint8_t ubCh, uint32_t ulAdr, uint32_t ulSize)
{
	uint32_t ulAtomSz = 0;
	uint32_t ulOffset = 0;
	
	ulAtomSz = sizeof(MP4ATOM);
	
	pMoov[ubCh] = (MP4ATOM*) (ulAdr + ulOffset);
	pMvhd[ubCh] = (MOVIE_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(MOVIE_HEADER_ATOM));

	// Video 1
	pTrakV[ubCh] = (MP4ATOM*) (ulAdr + ulOffset);
	pTkhdV[ubCh] = (TRACK_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(TRACK_HEADER_ATOM));

	pEdtsV[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pElstV[ubCh] = (EDIT_LIST_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(EDIT_LIST_ATOM));

	pMdiaV[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pMdhdV[ubCh] = (MEDIA_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(MEDIA_HEADER_ATOM));

	pHdlrV[ubCh] = (HANDLER_REFERENCE_ATOM*) (ulAdr + ulOffset);
	ulOffset += sizeof(HANDLER_REFERENCE_ATOM);

	pMinfV[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pVmhdV[ubCh] = (VIDEO_MEDIA_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(VIDEO_MEDIA_HEADER_ATOM));
		
	pDinfV[ubCh] = (DATA_INFORMATION_ATOM*) (ulAdr + ulOffset);
	ulOffset += sizeof(DATA_INFORMATION_ATOM);

	pStblV[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pStsdV[ubCh] = (VIDEO_SAMPLE_DESCRIPTION_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(VIDEO_SAMPLE_DESCRIPTION_ATOM));

	sStbl_Info[ubCh].ulSttsVAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulSttsVAdr),0);
	sStbl_Info[ubCh].ulStssVAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStssVAdr),0);
	sStbl_Info[ubCh].ulStscVAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscVAdr),0);	
	sStbl_Info[ubCh].ulStszVAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszVAdr),0);	
	sStbl_Info[ubCh].ulStcoVAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),0);	

	// Audio
	pTrakA[ubCh] = (MP4ATOM*) (ulAdr + ulOffset);
	pTkhdA[ubCh] = (TRACK_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(TRACK_HEADER_ATOM));

	pEdtsA[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pElstA[ubCh] = (EDIT_LIST_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(EDIT_LIST_ATOM));	

	pMdiaA[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pMdhdA[ubCh] = (MEDIA_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(MEDIA_HEADER_ATOM));	

	pHdlrA[ubCh] = (HANDLER_REFERENCE_ATOM*) (ulAdr + ulOffset);
	ulOffset += sizeof(HANDLER_REFERENCE_ATOM);

	pMinfA[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pSmhdA[ubCh] = (SOUND_MEDIA_HEADER_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(SOUND_MEDIA_HEADER_ATOM));

	pDinfA[ubCh] = (DATA_INFORMATION_ATOM*) (ulAdr + ulOffset);
	ulOffset += sizeof(DATA_INFORMATION_ATOM);	

	pStblA[ubCh] =  (MP4ATOM*) (ulAdr + ulOffset);
	pStsdA[ubCh] = (AUDIO_SAMPLE_DESCRIPTION_ATOM*) (ulAdr + ulOffset + ulAtomSz);
	ulOffset += (ulAtomSz + sizeof(AUDIO_SAMPLE_DESCRIPTION_ATOM));	

	sStbl_Info[ubCh].ulSttsAAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulSttsAAdr),0);
	sStbl_Info[ubCh].ulStscAAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscAAdr),0);	
	sStbl_Info[ubCh].ulStszAAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszAAdr),0);	
	sStbl_Info[ubCh].ulStcoAAdr = ulAdr + ulOffset;
	ulOffset += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),0);	
}

uint32_t ulMP4_GetStreamDelayTime(uint8_t ubCh)
{
	return ulMP4_GetLong((uint8_t*)pMvhd[ubCh]->ubReserve,0);
}

uint32_t ulMP4_GetStreamTimeScale(uint8_t ubCh, uint8_t ubStreamType)
{
	if( ubStreamType == MP4_STR_V1)
		return ulMP4_GetLong((uint8_t*)pMdhdV[ubCh]->ubTimeScale,0);
	else if( ubStreamType == MP4_STR_A1)
		return ulMP4_GetLong((uint8_t*)pMdhdA[ubCh]->ubTimeScale,0);
	return 0;
}

uint32_t ulMP4_GetTotalFrameNumber(uint8_t ubCh, uint8_t ubStreamType)
{
	if( ubStreamType == MP4_STR_V1)
	 	return ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulSttsVAdr),16);
	else if( ubStreamType == MP4_STR_A1)
		return ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulSttsAAdr),16);
	return 0;
}

//------------------------------------------------------------------------------
//	MP4 Get video resolution
//------------------------------------------------------------------------------
uint8_t ubMP4_GetResolution(uint8_t ubCh, uint8_t ubStreamType) 
{
	uint32_t ulWidth;
	uint32_t ulHeight;
	
	if( ubStreamType == MP4_STR_A1 || ubStreamType == MP4_STR_V2)
		return MP4_RES_NONE;

	ulWidth = (ulMP4_GetLong((uint8_t*)pTkhdV[ubCh]->ubTrackWidth,0) >> 16);
	ulHeight = (ulMP4_GetLong((uint8_t*)pTkhdV[ubCh]->ubTrackHeight,0) >> 16);

	if( (1900 < ulWidth && 1940 > ulWidth) && (1060 < ulHeight && 1100 > ulHeight))
		return MP4_RES_FHD;
	else if ( (1260 < ulWidth && 1300 > ulWidth) && (700 < ulHeight && 740 > ulHeight))
		return MP4_RES_HD;
	else if ( (750 < ulWidth && 850 > ulWidth) && (450 < ulHeight && 500 > ulHeight))
		return MP4_RES_WVGA;
	else if ( (620 < ulWidth && 660 > ulWidth) && (340 < ulHeight && 380 > ulHeight))
		return MP4_RES_VGA;
	else
		return MP4_RES_NONE;
}

uint32_t ulMP4_GetFrameDuration(uint8_t ubCh, uint8_t ubStreamType)
{
	if( ubStreamType == MP4_STR_V1)
	 	return ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulSttsVAdr),20);
	else if( ubStreamType == MP4_STR_A1)
		return ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulSttsAAdr),20);
	return 0;
}

uint32_t ulMP4_GetTotalVideoIFrame(uint8_t ubCh, uint8_t ubStreamType)
{
	if( ubStreamType == MP4_STR_V1)
	 	return ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStssVAdr),12);
	return 0;
}

uint32_t ulMP4_ParseVideoIFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulStcoEntryN = 0;	// total chunk
	uint32_t ulStcoIdx = 0;
	uint32_t ulStcoAdr = 0;
	
	uint32_t ulStscEntryN = 0;
	uint32_t ulStscIdx = 0;
	uint32_t ulStscChkID = 0;
	uint32_t ulStscFrmInChk = 0;
	
	uint32_t ulStszEntryN = 0;	// total frame
	uint32_t ulStszIdx = 0;
	
	//uint32_t ulStssEntryN = 0;	// total I frame
	uint32_t ulStssIdx = 0;
	uint32_t ulStssFrm = 0;
	
	uint32_t ulChunkCnt = 1;
	uint32_t ulFrmCnt = 1;
	uint32_t ulFrmInChkCnt = 1;
	
	ulStscEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscVAdr),12);
	ulStcoEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),12);
	//ulStssEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStssVAdr),12);
	ulStszEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszVAdr),16);

	for( ; ulChunkCnt <= ulStcoEntryN ; ulChunkCnt++)
	{
		ulStcoAdr = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*ulStcoIdx)));
			
		ulStscChkID = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscVAdr),(16 + (12*ulStscIdx)));

		if(ulChunkCnt == ulStscChkID)
		{
			ulStscFrmInChk = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscVAdr),(20 + (12*ulStscIdx)));
			ulStscIdx++;
		}

		for(ulFrmInChkCnt = 1 ; ulFrmInChkCnt <= ulStscFrmInChk ; ulFrmInChkCnt++)
		{
			ulStssFrm = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStssVAdr),(16 + (4*ulStssIdx)));

			if(ulFrmCnt == ulStssFrm)
			{
				memcpy((uint8_t *)(ulAdr + 12*ulStssIdx), (uint8_t *)&ulStcoAdr, 4);
				memcpy((uint8_t *)(ulAdr + 12*ulStssIdx + 4), (uint8_t *)&ulFrmCnt, 4);
				memcpy((uint8_t *)(ulAdr + 12*ulStssIdx + 8), (uint8_t *)&ulChunkCnt, 4);
				ulStssIdx++;
			}
			ulStcoAdr += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszVAdr),(20+ (4*ulStszIdx)));
			ulFrmCnt++;
			ulStszIdx++;
		}
		ulStcoIdx++;
	}

	PlyChkCtrl->ulVDOTotalSTSC = ulStscEntryN;
	PlyChkCtrl->ulVDOTotalChk =  ulStcoEntryN;
	PlyChkCtrl->ulVDOChkIdx = 0;	
	PlyChkCtrl->ulVDOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx)));

	PlyChkCtrl->ulVDOTotalFrm = ulStszEntryN;
	PlyChkCtrl->ulVDOFrmIdxInFile = 0;
	PlyChkCtrl->ulVDOFrmSize = 0;

	
	return 1;
}

uint32_t ulMP4_ParseAudioFrameTable(uint8_t ubCh, uint32_t ulAdr,PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulStcoEntryN = 0;	// total chunk
	uint32_t ulStcoIdx = 0;
	uint32_t ulStcoAdr = 0;

	uint32_t ulStscEntryN = 0;
	uint32_t ulStscIdx = 0;
	uint32_t ulStscChkID = 0;
	uint32_t ulStscFrmInChk = 0;

	uint32_t ulStszEntryN = 0;	// total frame
	uint32_t ulStszIdx = 0;
	
	uint32_t ulChunkCnt = 1;
	uint32_t ulFrmInChkCnt = 1;
	
	ulStcoEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),12);
	ulStscEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscAAdr),12);
	ulStszEntryN = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszAAdr),16);

	for( ; ulChunkCnt <= ulStcoEntryN ; ulChunkCnt++)
	{
		ulStcoAdr = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*ulStcoIdx)));
			
		ulStscChkID = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscAAdr),(16 + (12*ulStscIdx)));

		if(ulChunkCnt == ulStscChkID)
		{
			ulStscFrmInChk = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscAAdr),(20 + (12*ulStscIdx)));
			ulStscIdx++;
		}

		for(ulFrmInChkCnt = 1 ; ulFrmInChkCnt <= ulStscFrmInChk ; ulFrmInChkCnt++)
		{
			memcpy((uint8_t *)(ulAdr + 8*ulStszIdx), (uint8_t *)&ulStcoAdr, 4);
			memcpy((uint8_t *)(ulAdr + 8*ulStszIdx + 4), (uint8_t *)&ulChunkCnt, 4);
			
			ulStcoAdr += ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszAAdr),(20+ (4*ulStszIdx)));
			ulStszIdx++;
		}
		ulStcoIdx++;
	}

	PlyChkCtrl->ulADOTotalSTSC = ulStscEntryN;
	PlyChkCtrl->ulADOTotalChk =  ulStcoEntryN;
	PlyChkCtrl->ulADOChkIdx = 0;	
	PlyChkCtrl->ulADOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx)));

	PlyChkCtrl->ulADOTotalFrm = ulStszEntryN;
	PlyChkCtrl->ulADOFrmIdxInFile = 0;
	PlyChkCtrl->ulADOFrmSize = 0;
	
	return 1;
}

uint32_t ulMP4_GetAudioSampleRate(uint8_t ubCh)
{
	uint32_t ulSampleRate;
	ulSampleRate = ulMP4_GetLong((uint8_t*)(pStsdA[ubCh]->ubSampleRate),0);
	return (ulSampleRate>>16);
}

uint32_t ulMP4_GetAudioAvgBR(uint8_t ubCh)
{
	uint32_t ulAvgByteRate;
	ulAvgByteRate = (ulMP4_GetLong((uint8_t*)(pStsdA[ubCh]->ubDecConfDescAvgBR),0) / 8);
	return ulAvgByteRate;
}

uint32_t ulMP4_ParseMdatFrameInfo(uint8_t ubCh,PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	// Check Frame Type
	if( PlyChkCtrl->ulFileOffset == PlyChkCtrl->ulVDOChkAdrOffset)
	{
		PlyChkCtrl->ulFrmType = MP4_FRMTYPE_VDO;
		PlyChkCtrl->ulVDOChkIdx++;
		PlyChkCtrl->ulVDOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx)));
	}
	else if ( PlyChkCtrl->ulFileOffset == PlyChkCtrl->ulADOChkAdrOffset)
	{
		PlyChkCtrl->ulFrmType = MP4_FRMTYPE_ADO;
		PlyChkCtrl->ulADOChkIdx++;
		PlyChkCtrl->ulADOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*PlyChkCtrl->ulADOChkIdx)));
	}

	// Read Frame Size
	if( PlyChkCtrl->ulFrmType == MP4_FRMTYPE_VDO)
	{
		PlyChkCtrl->ulVDOFrmSize = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszVAdr),(20+ (4*PlyChkCtrl->ulVDOFrmIdxInFile)));
		PlyChkCtrl->ulFileOffset += PlyChkCtrl->ulVDOFrmSize;
		PlyChkCtrl->ulVDOFrmIdxInFile++;
	}
	else if( PlyChkCtrl->ulFrmType == MP4_FRMTYPE_ADO)
	{
		PlyChkCtrl->ulADOFrmSize = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszAAdr),(20+ (4*PlyChkCtrl->ulADOFrmIdxInFile)));
		PlyChkCtrl->ulFileOffset += PlyChkCtrl->ulADOFrmSize;
		PlyChkCtrl->ulADOFrmIdxInFile++;
	}
	return 1;
}

uint32_t ulMP4_JumpReload(uint8_t ubCh, PLY_CHUNK_CONTROLLER *PlyChkCtrl)
{
	uint32_t ulStscIdx = 0;
	uint32_t ulFrmNum = 0;
	uint32_t ulChkID=0;
	uint32_t ulFrm=0;
	uint32_t ulChkIDCnt=0;
	
	if(PlyChkCtrl->ulFrmType == MP4_FRMTYPE_ADO)
	{
		// Next Audio chunk ID and Offset
		PlyChkCtrl->ulADOChkIdx++;
		PlyChkCtrl->ulADOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*PlyChkCtrl->ulADOChkIdx)));

		if( PlyChkCtrl->ulFileOffset == 0x30)
		{
			PlyChkCtrl->ulVDOChkIdx = 0;
			PlyChkCtrl->ulVDOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx)));
			PlyChkCtrl->ulVDOFrmIdxInFile = 0;
		}
		else	
		{
			// Nex Video chunk ID and Offset
			for( ; PlyChkCtrl->ulVDOChkIdx > 0; PlyChkCtrl->ulVDOChkIdx--)
			{
				if(PlyChkCtrl->ulFileOffset > ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx))))
				{
					PlyChkCtrl->ulVDOChkIdx++;
					PlyChkCtrl->ulVDOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx)));
					break;
				}
			}
			// Video Frame Number
			for(ulChkIDCnt = 0 ; ulChkIDCnt < (PlyChkCtrl->ulVDOChkIdx+1) ; ulChkIDCnt++)
			{
				ulChkID =  ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscVAdr),(16 + (12*ulStscIdx)));
				
				if(ulChkIDCnt == ulChkID)
				{
					ulFrm = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscVAdr),(20 + (12*ulStscIdx)));
					ulFrmNum += ulFrm;
					ulStscIdx++;
				}
				else
					ulFrmNum += ulFrm;
			}
			PlyChkCtrl->ulVDOFrmIdxInFile = ulFrmNum;
		}
	}
	else if(PlyChkCtrl->ulFrmType == MP4_FRMTYPE_VDO)
	{
		PlyChkCtrl->ulVDOChkIdx++;
		PlyChkCtrl->ulVDOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoVAdr),(16 + (4*PlyChkCtrl->ulVDOChkIdx)));

		if( PlyChkCtrl->ulFileOffset == 0x30)
		{
			PlyChkCtrl->ulADOChkIdx = 0;
			PlyChkCtrl->ulADOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*PlyChkCtrl->ulADOChkIdx)));
			PlyChkCtrl->ulADOFrmIdxInFile = 0;
		}
		else	
		{
			// Nex Video chunk ID and Offset
			for( ; PlyChkCtrl->ulADOChkIdx > 0; PlyChkCtrl->ulADOChkIdx--)
			{
				if(PlyChkCtrl->ulFileOffset > ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*PlyChkCtrl->ulADOChkIdx))))
				{
					PlyChkCtrl->ulADOChkIdx++;
					PlyChkCtrl->ulADOChkAdrOffset = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStcoAAdr),(16 + (4*PlyChkCtrl->ulADOChkIdx)));
					break;
				}
			}

			// Video Frame Number
			for(ulChkIDCnt = 0 ; ulChkIDCnt < (PlyChkCtrl->ulADOChkIdx+1) ; ulChkIDCnt++)
			{
				ulChkID =  ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscAAdr),(16 + (12*ulStscIdx)));
				
				if(ulChkIDCnt == ulChkID)
				{
					ulFrm = ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStscAAdr),(20 + (12*ulStscIdx)));
					ulFrmNum += ulFrm;
					ulStscIdx++;
				}
				else
					ulFrmNum += ulFrm;
			}
			PlyChkCtrl->ulADOFrmIdxInFile = ulFrmNum;
		}
	}
	
	return 1;
}

uint32_t ulMP4_GetVideoFrameSize( uint8_t ubCh , uint32_t ulFrm)
{
	return ulMP4_GetLong((uint8_t*)(sStbl_Info[ubCh].ulStszVAdr),(20+ (4*(ulFrm-1))));
}


