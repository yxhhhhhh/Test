/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SADC.h
	\brief		SADC Header File
	\author		Hancyi Chiu
	\version	0.2
	\date		2017/04/26
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SADC_H_
#define _SADC_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------

typedef enum
{
	SADC_DISABLE,
	SADC_ENABLE
}SADC_Status_t;

typedef enum
{
	SADC_CH1 = 0,
	SADC_CH2,
	SADC_CH3,
	SADC_CH4,
	SADC_CH5
}SADC_CH_t;

//------------------------------------------------------------------------------
/*!
\brief SADC initialization function
\return(no)
*/
void SADC_Init(void);
//------------------------------------------------------------------------------
/*!
\brief SADC un-initialization function
\return(no)
*/
void SADC_UnInit(void);
//------------------------------------------------------------------------------
/*!
\brief SADC enable function
\return(no)
*/
void SADC_Enable(void);
//------------------------------------------------------------------------------
/*!
\brief SADC disable function
\return(no)
*/
void SADC_Disable(void);
//------------------------------------------------------------------------------
/*!
\brief Get 10bit value converted from voltage
\param tCH		SADC channel
\return(no)
*/
uint16_t uwSADC_GetReport(SADC_CH_t tCH);
//------------------------------------------------------------------------------
/*!
\brief 	Get SADC Version	
\return	Version
*/
uint16_t uwSADC_GetVersion(void);
#endif
