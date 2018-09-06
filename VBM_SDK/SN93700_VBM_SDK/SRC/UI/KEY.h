/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		KEY.h
	\brief		KEY Header file
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/02/24
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _KEY_H_
#define _KEY_H_																   
//------------------------------------------------------------------------------
#include "_510PF.h"
#include "PKEY.h"
#include "AKEY.h"

#define KEY_THREAD_PERIOD		10 //! Unit: ms
#define PKEY_ENABLE				1
#define AKEY_ENABLE				1

typedef enum
{
	PKEY,
	AKEY,
	GKEY,	
}KEY_Type_t;

typedef enum
{
	KEY_DOWN_ACT,
	KEY_CNT_ACT,
	KEY_UP_ACT
}KEY_Action_t;

typedef enum
{
	KEY_UNKNOW_STATE,
	KEY_DET_STATE,
	KEY_DEBONC_STATE,
	KEY_CNT_STATE
}KEY_ScanState_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{	
	uint8_t  ubKeyAction;
	uint8_t  ubKeyID;
	uint16_t uwKeyCnt;
}KEY_Event_t;
#pragma pack(pop)

//----------------------------------------------------------
void KEY_Init(osMessageQId *pEventQueueHandle);
void KEY_QueueSend(KEY_Type_t tKeyType, void *pvKeyEvent);
//----------------------------------------------------------
#endif
