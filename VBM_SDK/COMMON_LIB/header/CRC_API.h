/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CRC.h
	\brief		CRC Header File
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/04/05
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CRC_H_
#define _CRC_H_

#include "_510PF.h"

#define     CRC_P_16        0xA001
#define     CRC_P_32        0xEDB88320L     //! 0x04C11DB7L
#define     CRC_P_CCITT     0x1021
#define     CRC_P_T10DIF    0xEDD1			//! 0x8BB7
#define     CRC_P_DNP       0xA6BC
#define     CRC_P_KERMIT    0x8408
#define     CRC_P_SICK      0x8005
#define 	CRC_GP  		0x107
#define 	CRC_P_8  		0xE0			//! 0x07
#define 	CRC_P_24  		0x864CFB

typedef enum
{
	INIT_ALL_ZERO = 0,					//!< 32'h0000_0000
	INIT_ALL_FF,						//!< 32'hFFFF_FFFF
}CRC_INIT_VALUE_t;

typedef enum
{
	XOR_ALL_ZERO = 0,					//!< 32'h0000_0000
	XOR_ALL_FF,							//!< 32'hFFFF_FFFF
}CRC_FINAL_OUT_XOR_t;

typedef struct
{
	uint32_t CRC_INIT_VALUE:1;
	uint32_t CRC_FINAL_XOR_VALUE:1;
	uint32_t RESERVED1:2;
	uint32_t CRC_ORDER:5;				//!< Range:0 ~ 31, ex.0: CRC order = 1.... 31: CRC order = 32
	uint32_t RESERVED2:23;
}CRC_t;

//------------------------------------------------------------------------
/*!
\brief CRC function
\param tCRC_Param 			Setup parameters of CRC
\param ulPolynomial 		CRC polynomial
\param ulData_Addr 			Source address
\param ulData_Len			Data length
\return CRC value
*/
uint32_t ulCRC_Calc(CRC_t tCRC_Param, uint32_t ulPolynomial, uint32_t ulData_Addr, uint32_t ulData_Len);

#endif
