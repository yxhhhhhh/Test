/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		DDR_API.h
	\brief		DDR Self-Refresh Funcation Header
	\author		Pierce
	\version	0.3
	\date		2017/03/10
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _DDR_API_H_
#define _DDR_API_H_
#include <stdint.h>
//------------------------------------------------------------------------------
//! DDR Infor
#define DDR_MB_SZ					(32)					//!< DDR memory size (N MB)
#define DDR_ADDR_START				(0)						//!< DDR memory start address
#define DDR_BSZ_MAX					(DDR_MB_SZ*1024*1024)
#define DDR_ADDR_END				(0 + DDR_BSZ_MAX - 1)	//!< DDR memory end address
//------------------------------------------------------------------------------
/*!
	\brief 		DDR Self-Refresh Function	
	\par [Example]
	\code    
		 DDR_SelfRefresh();
	\endcode
*/
void DDR_SelfRefresh (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get DDR Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwDDR_GetVersion();
		 printf("DDR Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwDDR_GetVersion (void);
#endif
