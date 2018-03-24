/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SF_API.h
	\brief		Serial Flash header file
	\author		Nick Huang
	\version	0.3
	\date		2017/09/05
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SF_API_H_
#define _SF_API_H_
//------------------------------------------------------------------------------
#include "_510PF.h"

//------------------------------------------------------------------------------
//! SF Erase Command
#define SF_SE		0x20				//!< Sector erase
#define SF_SBE		0x52				//!< Small block erase
#define SF_LBE		0xD8				//!< Large block erase
#define SF_CE		0x60				//!< Chip erase

#ifdef FPGA
//! SF_CLK = 24MHz / (MS_SPEED + 2)
typedef enum
{
	SF_12M_CLK=0,
	SF_8M_CLK=1,
	SF_6M_CLK=2,
	SF_4M_CLK=4,
	SF_3M_CLK=6
} SF_CLK_t;
#endif

#ifdef ASIC
//! SF_CLK = 96MHz / (MS_SPEED + 2)
typedef enum
{
	SF_48M_CLK=0,
	SF_32M_CLK=1,
	SF_24M_CLK=2,
	SF_16M_CLK=4,
	SF_12M_CLK=6,
	SF_8M_CLK=10,
	SF_6M_CLK=14,
	SF_4M_CLK=22,
	SF_3M_CLK=30
} SF_CLK_t;
#endif

typedef enum
{
	SF_MXIC=0,
	SF_WINBOND,
	SF_GIGADEV,
	SF_AMIC,
	SF_ESMT,
	SF_PFLASH,
	SF_SST,
	SF_SSTPCT,
	SF_UNKNOWN
} SF_Manuf_t;

typedef struct
{
	SF_Manuf_t Manufacturer;
	uint32_t ulSize;
	uint32_t ulSBlkSize;
	uint32_t ulLBlkSize;
	uint32_t ulSecSize;
	SF_CLK_t HighSpeedClk;
	SF_CLK_t LowSpeedClk;
	uint8_t ubPhase;
	uint8_t ubMId;
	uint8_t ubDId;
	char cPartNum[16];
	uint8_t ubCeCmd;
	uint8_t ubSbeCmd;
	uint8_t ubLbeCmd;
	uint8_t ubAaiCmd;
} SF_Info_t;

typedef enum
{
	SF_DMA_WRITE=0,
	SF_DMA_READ=1
} SF_DmaDir_t;

typedef enum
{
	SF_DDR_DMA=0,
	SF_SRAM_DMA=1						//!< SRAM for code/data, Read: 1T/Write: 2T for 1st byte, 1T for other bytes
} SF_DmaRamType_t;

typedef enum
{
	SF_ADDR_NOT_ALIGN,
	SF_LENGTH_NOT_ALIGN,
	SF_LENGTH_OVER_128KB,
	SF_OK
} SF_Error_t;

typedef struct
{
	uint32_t ulSFStaAddr;
	uint32_t ulRAMStaAddr;
	uint32_t ulLength;
	SF_DmaRamType_t tRAMType;
	SF_DmaDir_t tDir;
	uint8_t ubWaitRdy;
	uint32_t ulCrcPolynomial;
	uint8_t ubCrcOrder;
	SF_Error_t tError;
} SF_DMA_Info_t;

typedef struct
{
	uint8_t* SBox;
	uint8_t RC4_Active:1;
	uint8_t SBox_Size:2;
} SF_RC4_Info_t;

extern SF_Info_t* pSF_Info;
//------------------------------------------------------------------------------
uint16_t uwSF_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief Serial Flash initialize
\return(no)
*/
void SF_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Show SF information (through printf)
\return(no)
*/
#ifdef _510PF_SDK_
void SF_ShowInfo(void);
#endif
//------------------------------------------------------------------------------
/*!
\brief Read data from serial flash
\param ulStaAddr	Start address to read in serial flash (in bytes)
\param ulLength		Data length to read (in bytes)
\param pbData		Data pointer to a byte array allocated for read-in data from SF
\return(no)
*/
void SF_Read(uint32_t ulStaAddr, uint32_t ulLength, uint8_t* pbData);
//------------------------------------------------------------------------------
/*!
\brief Write data to serial flash
\param ulStaAddr	Start address to write in serial flash (in bytes)
\param ulLength		Data length to write (in bytes)
\param pbData		Data pointer to a byte array that will be write-out to SF
\return(no)
*/
void SF_Write(uint32_t ulStaAddr, uint32_t ulLength, uint8_t* pbData);
//------------------------------------------------------------------------------
/*!
\brief SF DMA Read/Write
\param info			A structure contain necessary information for SF DMA\n
					operation, for more details about info please refer to\n
					SF_DMA_Info_t.
\param SF_RC4_Info	RC4 information for RC4 encryption, for more details about\n
					SF_RC4_Info please refer to SF_RC4_Info_t.
\return CRC, SF_SUCCESS or SF_FAIL
*/
uint32_t ulSF_DMA(SF_DMA_Info_t* info, SF_RC4_Info_t* SF_RC4_Info);
//------------------------------------------------------------------------------
void SF_EnableWrProtect(void);
//------------------------------------------------------------------------------
void SF_DisableWrProtect(void);
//------------------------------------------------------------------------------
/*!
\brief SF erase funtion
\param ubEraseCmd	Erase command, can be SF_SE, SF_SBE, SF_LBE and SF_CE
\par Note:
		SF_SE	Sector Erase (4KB)
		SF_SBE	Small Block Erase (different size in different part number)
		SF_LBE	Large Block Erase (different size in different part number)
		SF_CE	Chip Erase (Whole chip)
\param ulStaAddr	Start address to erase (in bytes, auto extend alignment to\n
					sector or block boundary, ignored if SF_CE)
\param ulLength		Length to erase (in bytes, auto extend alignment to sector\n
					or block boundary, ignored if SF_CE)
\return(no)
*/
void SF_Erase(uint8_t ubEraseCmd, uint32_t ulStaAddr, uint32_t ulLength);
//------------------------------------------------------------------------------

#endif
