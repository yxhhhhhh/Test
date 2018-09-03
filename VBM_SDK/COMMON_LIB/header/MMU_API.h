/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		MMU_API.h
	\brief		MMU Function API Header
	\author		Pierce
	\version	0.3
	\date		2017/03/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _MMU_API_H_
#define _MMU_API_H_
#include "_510PF.h"
#include "DDR_API.h"
//------------------------------------------------------------------------------
//! Code range
#define MMU_RO_ADDR_START			(DDR_ADDR_START)		//!< System R0 start address
//#define MMU_RO_SZ_MAX				(0x100000)
#define MMU_RO_SZ_MAX				(0x200000)
//#define MMU_RO_ADDR_END			(MMU_RO_ADDR_START + MMU_RO_SZ_MAX - 1)
//------------------------------------------------------------------------------
//! Data range
//#define MMU_RW_ADDR_START			(MMU_RO_ADDR_END + 1)
//#define MMU_RW_SZ_MAX				(0xF0000)
//#define MMU_RW_SZ_MAX				(0x180000)
#define MMU_RW_SZ_MAX				(0x200000)

//#define MMU_RW_ADDR_END			(MMU_RW_ADDR_START + MMU_RW_SZ_MAX - 1)
//------------------------------------------------------------------------------
//! Other range
//#define MMU_R0RW_SZ_MAX			(MMU_RO_SZ_MAX + MMU_RW_SZ_MAX)
//#define MMU_TEST_MEM_ADDR_START	(MMU_RW_ADDR_END + 1)	
//#define MMU_TEST_MEM_SZ_MAX		(DDR_BSZ_MAX - MMU_RO_SZ_MAX - MMU_RW_SZ_MAX - MMU_TT_SZ_MAX)
//#define MMU_TEST_MEM_ADDRR_END	(MMU_TEST_MEM_ADDR_START + MMU_TEST_MEM_SZ_MAX - 1)
//------------------------------------------------------------------------------
//! MMU TT range
#define MMU_TT_SZ_MAX				(0x10000)
//#define MMU_TT_ADDR_START			(DDR_ADDR_END - MMU_TT_SZ_MAX + 1)
//#define MMU_TT_ADDR_END			(DDR_ADDR_END)
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Disable I-Cache Function	
	\par [Example]
	\code
		 MMU_DisableICache();
	\endcode
*/
void MMU_DisableICache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Disable D-Cache Function	
	\par [Example]
	\code
		 MMU_DisableDCache();
	\endcode
*/
void MMU_DisableDCache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Disable MMU Function	
	\par [Example]
	\code
		 MMU_DisableMMU();
	\endcode
*/
void MMU_DisableMMU(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Enable I-Cache Function	
	\par [Example]
	\code
		 MMU_EnableICache();
	\endcode
*/
void MMU_EnableICache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Enable D-Cache Function	
	\par [Example]
	\code
		 MMU_EnableDCache();
	\endcode
*/
void MMU_EnableDCache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Enable MMU Function	
	\par [Example]
	\code
		 MMU_EnableMMU();
	\endcode
*/
void MMU_EnableMMU(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Get CPU State Function	
	\return	CPU state vaule
	\note	The function prints the I-Cache state\n
			The function prints the D-Cache state\n
			The function prints the MMU state\n
	\par [Example]
	\code
		 ulMMU_CpuState();
	\endcode
*/
uint32_t ulMMU_CpuState(void);
//------------------------------------------------------------------------------
/*!
	\brief 		MMU Initial Function
	\par [Example]
	\code
		 MMU_Init(void);
	\endcode
*/
void MMU_Init(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get Buffer Start Address Function	
	\return	    Buffer start address
	\par [Example]
	\code
		 uint32_t ulBUF_InitFreeBufAddr;
		 
		 MMU_Init();
		 ulBUF_InitFreeBufAddr = ulMMU_GetBufStartAddr();
	\endcode
*/
uint32_t ulMMU_GetBufStartAddr(void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get MMU Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwMMU_GetVersion();
		 printf("MMU Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwMMU_GetVersion (void);
#endif
