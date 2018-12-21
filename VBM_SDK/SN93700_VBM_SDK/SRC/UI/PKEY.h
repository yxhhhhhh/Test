/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PKEY.h
	\brief		Power Key Scan Header File
	\author		Hanyi Chiu
	\version	1
	\date		2017/03/30
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _PKEY_H_
#define _PKEY_H_
//------------------------------------------------------------------------------
#include <stdint.h>
#include "KEY.h"

#define PKEY_DOWN_EVENT					0							//!<KEY pin key press down event
#define PKEY_CNT_EVENT					1							//!<KEY pin key continue press down event
#define PKEY_UP_EVENT					2							//!<KEY pin key release up event

#define PKEY_DET_TIME					(30 / KEY_THREAD_PERIOD)	// 30ms
#define PKEY_DEB_TIME					(10 / KEY_THREAD_PERIOD)	// 10ms
#define PKEY_CNT_TIME					(100 / KEY_THREAD_PERIOD)	// 100ms
#define PKEY_LOCK_TIME					(30 / KEY_THREAD_PERIOD)	// 30ms

#define GKEY_DOWN_EVENT					0							//!<KEY pin key press down event
#define GKEY_CNT_EVENT					1							//!<KEY pin key continue press down event
#define GKEY_UP_EVENT					2							//!<KEY pin key release up event

#define GKEY_DET_TIME					(30 / KEY_THREAD_PERIOD)	// 30ms
#define GKEY_DEB_TIME					(10 / KEY_THREAD_PERIOD)	// 10ms
#define GKEY_CNT_TIME					(100 / KEY_THREAD_PERIOD)	// 100ms

typedef enum
{
	PKEY_ID0,
}PKEY_ID_t;

typedef struct
{
	uint8_t ubPKEY_KeyScanState;
	uint16_t uwPKEY_KeyScanTime;
	uint16_t uwPKEY_KeyScanCnt;
}PKEY_SCAN_t;

#ifdef VBM_PU
typedef enum
{
	GKEY_UNKNOW,
	GKEY_ID0 = 0x11,
	GKEY_ID1 = 0x12,		
}GKEY_ID_t;
#endif

#ifdef VBM_BU
typedef enum
{
	GKEY_ID0 = 0x11,	
}GKEY_ID_t;
#endif

typedef struct
{
	uint8_t ubGKEY_KeyScanState;
	uint16_t uwGKEY_KeyScanTime;
	uint16_t uwGKEY_KeyScanCnt;
}GKEY_SCAN_t;

void PKEY_Init(void);
void PKEY_Thread(void);

void GKEY_Init(void);
void GKEY_Thread(void);

GKEY_ID_t GKEY_Detection(void);

#endif														// Define _PKEY_H_
