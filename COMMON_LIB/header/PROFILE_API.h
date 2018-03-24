/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PROFILE_API.h
	\brief		Profile API Header
	\author		Hanyi Chiu
	\version	0.3
	\date		2017/05/15
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _PROFILE_API_H_
#define _PROFILE_API_H_

#include <stdint.h>

typedef enum
{
	SYSPARAM,
	OSDPARAM,
	KEYPARAM,
	ADOPARAM,
	VDOPARAM,	
	RESERVED,
	PROFILE_TYPE_MAX
}PROF_TYPE_t;

typedef enum
{
	IQ_TABLE_ADDR   = 0,
	IMG_TABLE_ADDR  = 4,
	COMM_TABLE_ADDR = 8,
	LOGO_ADDR		= 12,
	LOSTLOGO_ADDR	= 16,
}PROF_SYSParam_t;

typedef enum
{
	OSD_MENU_ADDR   = 0,
	OSD_IMAGE_ADDR  = 4,
	OSD_FONT_ADDR   = 8,		
}PROF_OSDParam_t;

//------------------------------------------------------------------------------
/*!
\brief Profile initialize
\return(no)
*/
void PROF_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Get system profile parameter
\param tProf_Type		Profile type
\return Parameter
*/
uint8_t *pbPROF_GetParam(PROF_TYPE_t tProf_Type);
//------------------------------------------------------------------------------
/*!
\brief 	Get Profile Version	
\return	Version
*/
uint16_t uwPROFILE_GetVersion(void);

#endif
