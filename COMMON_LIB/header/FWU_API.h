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
	\version	1.1
	\date		2018/06/12
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _FWU_API_H_
#define _FWU_API_H_

#include <stdint.h>

//! SN937XX Firmware Version
#define SN937XX_FW_VERSION				"01.00.01.027"

typedef enum
{
	FWU_USBDMSC,
	FWU_DISABLE,
}FWU_MODE_t;

typedef struct
{
	char cVolumeLable[11];
	char cFileName[8];			//! Length must be less than or equal to 8.(File system only support 8.3 filename)
	char cFileNameExt[3];		//! Length must be less than or equal to 3.(File system only support 8.3 filename)
}FWU_MSCParam_t;

//------------------------------------------------------------------------------
/*!
\brief Firmware upgrade initialization
\return (no)
*/
void FWU_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Setup Firmware upgrade parameter.
\param FWU_MODE_t	Setup upgrade mode.
\param pvModeParam	Setup upgrade mode parameter setup.
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
\brief 	Get FWU Version	
\return	Version
*/
uint16_t uwFWU_GetVersion(void);

#endif
