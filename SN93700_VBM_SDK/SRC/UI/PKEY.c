/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PKEY.c
	\brief		Power KEY Function
	\author		Hanyi Chiu
	\version	1.1
	\date		2017/11/28
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "PKEY.h"
#include "RTC_API.h"
//----------------------------------------------------------
KEY_Event_t tPKEY_Event;
PKEY_SCAN_t tPKEY_Scan;

KEY_Event_t tGKEY_Event;
GKEY_SCAN_t tGKEY_Scan;
GKEY_ID_t   tGKEY_LastID;

//----------------------------------------------------------
void PKEY_Init(void)
{
	memset(&tPKEY_Scan, 0, sizeof(PKEY_SCAN_t));
	tPKEY_Scan.ubPKEY_KeyScanState = KEY_UNKNOW_STATE;
}
//----------------------------------------------------------
void PKEY_Thread(void)
{
	static uint16_t uwPKEY_ThreadCnt = 0;

	if(uwPKEY_ThreadCnt % tPKEY_Scan.uwPKEY_KeyScanTime == 0)
	{
		uwPKEY_ThreadCnt = 0;
		switch(tPKEY_Scan.ubPKEY_KeyScanState)
		{
			case KEY_UNKNOW_STATE:
				if(ubRTC_GetKey() == 0)
					tPKEY_Scan.ubPKEY_KeyScanState = KEY_DET_STATE;
				tPKEY_Scan.uwPKEY_KeyScanTime = PKEY_DET_TIME;
				break;
			case KEY_DET_STATE:
				if(ubRTC_GetKey() == 1)
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_DEBONC_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DEB_TIME;
				}
				else
				{
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DET_TIME;
				}
				break;
			case KEY_DEBONC_STATE:
				if(ubRTC_GetKey() == 1)
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_CNT_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_CNT_TIME;
					tPKEY_Event.ubKeyAction = KEY_DOWN_ACT;
					tPKEY_Event.ubKeyID	  	= PKEY_ID0;
					tPKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(PKEY, &tPKEY_Event);
				}
				else
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_DET_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DET_TIME;
				}
				break;
			case KEY_CNT_STATE:
				if(ubRTC_GetKey() == 1)
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_CNT_STATE; 
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_CNT_TIME;
					tPKEY_Event.ubKeyAction = KEY_CNT_ACT;
					tPKEY_Event.ubKeyID	  	= PKEY_ID0;
					tPKEY_Event.uwKeyCnt  	= ++tPKEY_Scan.uwPKEY_KeyScanCnt;
					KEY_QueueSend(PKEY, &tPKEY_Event);
				}
				else
				{
					tPKEY_Scan.ubPKEY_KeyScanState 	= KEY_DET_STATE;
					tPKEY_Scan.uwPKEY_KeyScanTime 	= PKEY_DET_TIME;
					tPKEY_Event.ubKeyAction = KEY_UP_ACT;
					tPKEY_Event.ubKeyID	  	= PKEY_ID0;
					tPKEY_Event.uwKeyCnt  	= tPKEY_Scan.uwPKEY_KeyScanCnt;
					KEY_QueueSend(PKEY, &tPKEY_Event);
					tPKEY_Scan.uwPKEY_KeyScanCnt = 0;
				}
				break;
			default:
				break;
		}
	}
	uwPKEY_ThreadCnt++;
}

//--------------------------------------------------------------
void GKEY_Init(void)
{	
#ifdef VBM_BU
	GLB->PADIO34   = 0;
	GPIO->GPIO_OE6 = 0;
#endif

#ifdef VBM_PU
	tGKEY_LastID = GKEY_UNKNOW;

#if 1
	GLB->PADIO51 = 0; 	//! AUDIO+
	GLB->PADIO52 = 0; 	//! AUDIO-	
	
	GPIO->GPIO_OE9  = 0; //! AUDIO+
	GPIO->GPIO_OE10 = 0; //! AUDIO-
#endif

#endif

	tGKEY_Scan.uwGKEY_KeyScanTime  = GKEY_DET_TIME;
	tGKEY_Scan.ubGKEY_KeyScanState = KEY_DET_STATE;
	tGKEY_Scan.uwGKEY_KeyScanCnt   = 0;
}

void GKEY_Thread(void)
{
	static uint16_t uwGKEY_ThreadCnt = 0;

#ifdef VBM_BU
	if(uwGKEY_ThreadCnt % tGKEY_Scan.uwGKEY_KeyScanTime == 0)
	{
		uwGKEY_ThreadCnt = 0;
		switch(tGKEY_Scan.ubGKEY_KeyScanState)
		{
			case KEY_UNKNOW_STATE:
				if(GPIO->GPIO_I6 == 1)
					tGKEY_Scan.ubGKEY_KeyScanState = KEY_DET_STATE;
				tGKEY_Scan.uwGKEY_KeyScanTime = GKEY_DET_TIME;
				break;
			case KEY_DET_STATE:
				if(GPIO->GPIO_I6 == 0)
				{
					tGKEY_Scan.ubGKEY_KeyScanState 	= KEY_DEBONC_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime 	= GKEY_DEB_TIME;
				}
				else
				{
					tGKEY_Scan.uwGKEY_KeyScanTime 	= GKEY_DET_TIME;
				}
				break;
			case KEY_DEBONC_STATE:
				if(GPIO->GPIO_I6 == 0)
				{
					tGKEY_Scan.ubGKEY_KeyScanState 	= KEY_CNT_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime 	= GKEY_CNT_TIME;
					tGKEY_Event.ubKeyAction = KEY_DOWN_ACT;
					tGKEY_Event.ubKeyID	  	= GKEY_ID0;
					tGKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(GKEY, &tGKEY_Event);
				}
				else
				{
					tGKEY_Scan.ubGKEY_KeyScanState 	= KEY_DET_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime 	= GKEY_DET_TIME;
				}
				break;
			case KEY_CNT_STATE:
				if(GPIO->GPIO_I6 == 0)
				{
					tGKEY_Scan.ubGKEY_KeyScanState 	= KEY_CNT_STATE; 
					tGKEY_Scan.uwGKEY_KeyScanTime 	= GKEY_CNT_TIME;
					tGKEY_Event.ubKeyAction = KEY_CNT_ACT;
					tGKEY_Event.ubKeyID	  	= GKEY_ID0;
					tGKEY_Event.uwKeyCnt  	= ++tGKEY_Scan.uwGKEY_KeyScanCnt;
					KEY_QueueSend(GKEY, &tGKEY_Event);
				}
				else
				{
					tGKEY_Scan.ubGKEY_KeyScanState 	= KEY_DET_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime 	= GKEY_DET_TIME;
					tGKEY_Event.ubKeyAction = KEY_UP_ACT;
					tGKEY_Event.ubKeyID	  	= GKEY_ID0;
					tGKEY_Event.uwKeyCnt  	= tGKEY_Scan.uwGKEY_KeyScanCnt;
					KEY_QueueSend(GKEY, &tGKEY_Event);
					tGKEY_Scan.uwGKEY_KeyScanCnt = 0;
				}
				break;
			default:
				break;
		}
	}
#endif

#ifdef VBM_PU
	GKEY_ID_t tGKEY_DetID;

	tGKEY_DetID = GKEY_Detection();
	if(uwGKEY_ThreadCnt % tGKEY_Scan.uwGKEY_KeyScanTime == 0)
	{
		uwGKEY_ThreadCnt = 0;
		switch(tGKEY_Scan.ubGKEY_KeyScanState)
		{
			case KEY_DET_STATE:
				if(tGKEY_DetID != GKEY_UNKNOW)
				{
					tGKEY_Scan.ubGKEY_KeyScanState 	= KEY_DEBONC_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime  	= GKEY_DEB_TIME;
					tGKEY_Scan.uwGKEY_KeyScanCnt 	= 0;
				}
				else
				{
					tGKEY_Scan.uwGKEY_KeyScanTime  = GKEY_DET_TIME;
				}
				break;
			case KEY_DEBONC_STATE:
				if(tGKEY_DetID == tGKEY_LastID)
				{
					tGKEY_Scan.ubGKEY_KeyScanState = KEY_CNT_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime  = GKEY_CNT_TIME;
					tGKEY_Event.ubKeyAction = KEY_DOWN_ACT;
					tGKEY_Event.ubKeyID	  	= tGKEY_DetID;
					tGKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(GKEY, &tGKEY_Event);
				}
				else
				{
					tGKEY_Scan.ubGKEY_KeyScanState = KEY_DET_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime  = GKEY_DET_TIME;
				}
				break;
			case KEY_CNT_STATE:
				if(tGKEY_DetID == tGKEY_LastID)
				{
					tGKEY_Scan.uwGKEY_KeyScanCnt = (tGKEY_Scan.uwGKEY_KeyScanCnt < 255)?
					                               (++tGKEY_Scan.uwGKEY_KeyScanCnt):tGKEY_Scan.uwGKEY_KeyScanCnt;
					tGKEY_Event.ubKeyAction = KEY_CNT_ACT;
					tGKEY_Event.ubKeyID	  	= tGKEY_DetID;
					tGKEY_Event.uwKeyCnt  	= tGKEY_Scan.uwGKEY_KeyScanCnt;
					KEY_QueueSend(GKEY, &tGKEY_Event);
				}
				else
				{
					tGKEY_Scan.uwGKEY_KeyScanCnt   = 0;
					tGKEY_Scan.ubGKEY_KeyScanState = KEY_DET_STATE;
					tGKEY_Scan.uwGKEY_KeyScanTime  = GKEY_DET_TIME;
					tGKEY_Event.ubKeyAction = KEY_UP_ACT;
					tGKEY_Event.ubKeyID	  	= tGKEY_LastID;
					tGKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(GKEY, &tGKEY_Event);
					tGKEY_DetID = GKEY_UNKNOW;
				}
				break;
			}
		tGKEY_LastID = tGKEY_DetID;
	}
#endif
	
	uwGKEY_ThreadCnt++;
}

GKEY_ID_t GKEY_Detection(void)
{
#ifdef VBM_PU
	if((GPIO->GPIO_I9 == 1)&&(GPIO->GPIO_I10 == 1))
	{
		return GKEY_UNKNOW;
	}

	if((GPIO->GPIO_I9 == 0)&&(GPIO->GPIO_I10 == 1))
	{
		return GKEY_ID0;
	}

	if((GPIO->GPIO_I10 == 0)&&(GPIO->GPIO_I9 == 1))
	{
		return GKEY_ID1;
	}
	return GKEY_UNKNOW;
	#endif

	return 0;
}
