/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AKEY.h
	\brief		Analog Key Scan Header File
	\author		Hanyi Chiu
	\version	1
	\date		2016/09/20
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _AKEY_H_
#define _AKEY_H_
//------------------------------------------------------------------------------
#include <stdint.h>
#include "KEY.h"

#define AKEY_DOWN_EVENT					0								//!< analog key press down event
#define AKEY_CNT_EVENT					1								//!< analog key continue press down event
#define AKEY_UP_EVENT					2								//!< analog key release up event

#define AKEY_DET_TIME					(20 / KEY_THREAD_PERIOD)		//!< 20ms
#define AKEY_DEB_TIME					(10 / KEY_THREAD_PERIOD)		//!< 10ms
#define AKEY_CNT_TIME					(100 / KEY_THREAD_PERIOD)		//!< 100ms

typedef enum
{
	AKEY_UNKNOW,
	AKEY_MENU,
	AKEY_UP,
	AKEY_DOWN,
	AKEY_LEFT,
	AKEY_RIGHT,
	AKEY_ENTER,
	AKEY_PTT,			//! Push To Talk Key
	AKEY_PS,			//! Power Saving Key
}AKEY_ID_t;

typedef struct
{
	uint8_t ubAKEY_KeyScanState;
	uint16_t uwAKEY_KeyScanTime;
	uint16_t uwAKEY_KeyScanCnt;
}AKEY_SCAN_t;

typedef struct
{
	AKEY_ID_t	tKeyID;
	uint16_t 	uwKeyLvl;
}AKEY_MAP_t;

//------------------------------------------------------------------------------
void AKEY_Init(void);
void AKEY_Thread(void);
AKEY_ID_t AKEY_Detection(void);

#endif														// Define _AKEY_H_
