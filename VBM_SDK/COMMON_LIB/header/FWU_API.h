/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		FWU_API.h
	\brief		Firmware upgrade function header file
	\author		Hanyi Chiu
	\version	1.9
	\date		2018/08/21
	\copyright	Copyright (C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _FWU_API_H_
#define _FWU_API_H_

#include <stdint.h>

//! SN937XX Firmware Version
#define SN937XX_FW_VERSION				"01.00.01.032"

typedef enum
{
	FWU_USBDMSC,
	FWU_SDCARD,
	FWU_DISABLE,
}FWU_MODE_t;

typedef enum
{
	FWU_UPG_FAIL 		= 120,
	FWU_UPG_DEVTAG_FAIL,
	FWU_UPG_SUCCESS,
	FWU_UPG_INPROGRESS,
}FWU_UpgResult_t;

typedef void (*pvFWU_StsRptFunc)(uint8_t);

typedef struct
{
	char cVolumeLable[11];
	char cFileName[8];					//! Length must be less than or equal to 8.(File system only support 8.3 filename)
	char cFileNameExt[3];				//! Length must be less than or equal to 3.(File system only support 8.3 filename)
	pvFWU_StsRptFunc pStsRptCbFunc;
}FWU_MSCParam_t;

typedef struct
{
	char cTargetFileName[32];
	uint8_t ubTargetFileNameLen;
	pvFWU_StsRptFunc pStsRptCbFunc;
	uint8_t ubIncrementProgressBy;		//! Progress bar scale, Unit: percent
}FWU_SDParam_t;

//------------------------------------------------------------------------------
/*!
\brief Firmware upgrade initialization
\return (no)
*/
void FWU_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Setup Firmware upgrade parameter.
\param FWU_MODE_t		Setup upgrade mode.
\param pvModeParam		Setup upgrade mode parameter setup.
\return (no)
*/
void FWU_Setup(FWU_MODE_t tModeSel, void *pvModeParam);
//------------------------------------------------------------------------------
/*!
\brief Enable firmware upgrade function
\return (no)
*/
void FWU_Enable(void);
//------------------------------------------------------------------------
/*!
\brief Start upgrade function use SD card
\param ulBUF_StartAddr	Buffer start address
\return upgrade result
*/
FWU_UpgResult_t FWU_SdUpgradeStart(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------
/*!
\brief 	Get FWU Version	
\return	Version
*/
uint16_t uwFWU_GetVersion(void);

#endif
