/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		QR_API.h
	\brief		QR Decode API 
	\author		Bruce Hsu	
	\version	0.1
	\date		2018/09/11
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __QR_API__
#define __QR_API__

#include "_510PF.h"


#define MAX_RESULT_LENGTH (2061+1)

typedef enum {
	
	QR_OK = 0,
	QR_Fail
}QR_Result;

QR_Result QRDecode(uint32_t ulInputAddr,uint32_t ulWidth,uint32_t ulHeight,int8_t * pDecodeText);


uint32_t QRCode_GetBufSize(uint32_t Width,uint32_t Height);


#endif
