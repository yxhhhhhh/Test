/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AKEY.c
	\brief		Analog Key Scan Function
	\author		Hanyi Chiu
	\version	1
	\date		2016/09/20
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "AKEY.h" 
#include "SADC.h"

AKEY_MAP_t tAKEY_Map[] =
{
#if 0
	{AKEY_UNKNOW,	0x3F0},
	{AKEY_MENU,		0x06C},
	{AKEY_UP,		0x0E8},
	{AKEY_DOWN,		0x164},
	{AKEY_RIGHT,	0x1E0},
	{AKEY_LEFT,		0x258},	//! 0x25D
	{AKEY_ENTER,	0x2D0},
	{AKEY_PTT,		0x33F},
	{AKEY_PS,		0x3D0},
#else
	{AKEY_UNKNOW,	0x3F0},
	{AKEY_MENU,		0x010},	//0x070/0x045
	{AKEY_LEFT,		0x090},	//0x0EA/0x0C5
	{AKEY_UP,		0x120},	//0x15E/0x137
	{AKEY_DOWN,		0x1A5},	//0x1CA/0x1AA
	{AKEY_ENTER,	0x23B},	//0x258/0x22A
	{AKEY_RIGHT,	0x2CD},	//0x2DA/0x2AC
	{AKEY_PTT,		0x353}, //0x33F/0x33F
	{AKEY_PS,		0x3AA}, //0x3B0/0x3AA
#endif
};
KEY_Event_t tAKEY_Event;
AKEY_ID_t tAKEY_LastID;
AKEY_SCAN_t tAKEY_Scan;
//------------------------------------------------------------------------------
void AKEY_Init(void)
{
	tAKEY_LastID = AKEY_UNKNOW;
	memset(&tAKEY_Event, 0, sizeof(KEY_Event_t));
	memset(&tAKEY_Scan, 0, sizeof(AKEY_SCAN_t));
	tAKEY_Scan.ubAKEY_KeyScanState  = KEY_DET_STATE;
	tAKEY_Scan.uwAKEY_KeyScanTime	= AKEY_DET_TIME;
	SADC_Init();
	SADC_Enable();
}
//------------------------------------------------------------------------------
void AKEY_Thread(void)
{
	AKEY_ID_t tAKEY_DetID;
	static uint16_t uwAKEY_ThreadCnt = 0;

	tAKEY_DetID = AKEY_Detection();
	if((uwAKEY_ThreadCnt % tAKEY_Scan.uwAKEY_KeyScanTime) == 0)
	{
		uwAKEY_ThreadCnt = 0;
		switch(tAKEY_Scan.ubAKEY_KeyScanState)
		{
			case KEY_DET_STATE:
				if(tAKEY_DetID != AKEY_UNKNOW)
				{
					tAKEY_Scan.ubAKEY_KeyScanState 	= KEY_DEBONC_STATE;
					tAKEY_Scan.uwAKEY_KeyScanTime  	= AKEY_DEB_TIME;
					tAKEY_Scan.uwAKEY_KeyScanCnt 	= 0;
				}
				else
				{
					tAKEY_Scan.uwAKEY_KeyScanTime  = AKEY_DET_TIME;
				}
				break;
			case KEY_DEBONC_STATE:
				if(tAKEY_DetID == tAKEY_LastID)
				{
					tAKEY_Scan.ubAKEY_KeyScanState = KEY_CNT_STATE;
					tAKEY_Scan.uwAKEY_KeyScanTime  = AKEY_CNT_TIME;
					tAKEY_Event.ubKeyAction = KEY_DOWN_ACT;
					tAKEY_Event.ubKeyID	  	= tAKEY_DetID;
					tAKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(AKEY, &tAKEY_Event);
				}
				else
				{
					tAKEY_Scan.ubAKEY_KeyScanState = KEY_DET_STATE;
					tAKEY_Scan.uwAKEY_KeyScanTime  = AKEY_DET_TIME;
				}
				break;
			case KEY_CNT_STATE:
				if(tAKEY_DetID == tAKEY_LastID)
				{
					tAKEY_Scan.uwAKEY_KeyScanCnt = (tAKEY_Scan.uwAKEY_KeyScanCnt < 255)?
					                               (++tAKEY_Scan.uwAKEY_KeyScanCnt):tAKEY_Scan.uwAKEY_KeyScanCnt;
					tAKEY_Event.ubKeyAction = KEY_CNT_ACT;
					tAKEY_Event.ubKeyID	  	= tAKEY_DetID;
					tAKEY_Event.uwKeyCnt  	= tAKEY_Scan.uwAKEY_KeyScanCnt;
					KEY_QueueSend(AKEY, &tAKEY_Event);
				}
				else
				{
					tAKEY_Scan.uwAKEY_KeyScanCnt   = 0;
					tAKEY_Scan.ubAKEY_KeyScanState = KEY_DET_STATE;
					tAKEY_Scan.uwAKEY_KeyScanTime  = AKEY_DET_TIME;
					tAKEY_Event.ubKeyAction = KEY_UP_ACT;
					tAKEY_Event.ubKeyID	  	= tAKEY_LastID;
					tAKEY_Event.uwKeyCnt  	= 0;
					KEY_QueueSend(AKEY, &tAKEY_Event);
					tAKEY_DetID = AKEY_UNKNOW;
				}
				break;
		}
		tAKEY_LastID = tAKEY_DetID;
	}
	uwAKEY_ThreadCnt++;
}	
//------------------------------------------------------------------------------
AKEY_ID_t AKEY_Detection(void)
{
	uint16_t uwAKEY_DetLvl = 0x3FF;
	uint8_t ubAKEY_MaxID, ubAKEY_Idx;

	//SADC
	uwAKEY_DetLvl = uwSADC_GetReport(SADC_CH1);
	if(uwAKEY_DetLvl >= tAKEY_Map[AKEY_UNKNOW].uwKeyLvl)
		return AKEY_UNKNOW;

	printd(1,"AKEY_Detection  uwAKEY_DetLvl  %x.\n",uwAKEY_DetLvl);
	ubAKEY_MaxID = sizeof tAKEY_Map / sizeof(AKEY_MAP_t);
	for(ubAKEY_Idx = AKEY_MENU; ubAKEY_Idx < ubAKEY_MaxID; ubAKEY_Idx++)
	{
		if(uwAKEY_DetLvl < tAKEY_Map[ubAKEY_Idx].uwKeyLvl)
			return tAKEY_Map[ubAKEY_Idx].tKeyID;
	}
	return AKEY_UNKNOW;
}
