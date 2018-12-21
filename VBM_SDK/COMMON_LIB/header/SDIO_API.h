/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SDIO_API.h
	\brief		SDIO Host header file
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/11/30
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef _SDIO_API_H_
#define _SDIO_API_H_

#include <stdint.h>

//------------------------------------------------------------------------
/*!
\brief SDIO Host initialize
\return(no)
*/
void SDIO_Init(void);
//------------------------------------------------------------------------
/*!
\brief 	Get SDIO Host Version	
\return	Version
*/
uint16_t uwSDIO_GetVersion(void);

#endif 
