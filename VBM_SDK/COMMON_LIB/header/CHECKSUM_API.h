/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CHECKSUM.h
	\brief		CheckSum Header File
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CHECKSUM_H_
#define _CHECKSUM_H_

#include "_510PF.h"

//------------------------------------------------------------------------
/*!
\brief CheckSum function
\param ulData_Addr 			Source address
\param ulData_Len			Data length
\return Checksum value
*/
uint16_t uwCHECKSUM_Calc(uint32_t ulData_Addr, uint32_t ulData_Len);
#endif
