/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PAIR.c
	\brief		Pairing Function
	\author		Bing
	\version	2018/4/17
	\date		0.4
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include "PAIR.h"
#include "BB_API.h"
#include "TWC_API.h"
#include "SF_API.h"
#include "APP_CFG.h"

#define PAIR_MAJORVER	0
#define PAIR_MINORVER	4

uint32_t        ulPAIR_Timeout,ulPAIR_TimeCnt;
uint8_t         ubPAIR_State,ubPAIR_Number,ubPAIR_PaapCnt;
osMessageQId    *xAppEventQueue;
PAIR_RRP_Hdr    PAIR_PrpPket;
PAIR_RAP_Hdr    PAIR_PapPket;
PAIR_ID_TABLE   PAIR_IdTable;
PAIR_Info_t 	tPAIR_Info;
osThreadId      PAIR_ThreadId;
static uint32_t ulPAIR_SFAddr;
uint8_t ubFixChTable[3] =
{
	20, 82, 154
};
uint8_t ubTestChTable[3] = 		
{
	20, 82, 154
};
//------------------------------------------------------------------------------
static void PAIR_Thread(void const *argument)
{
    PAIR_EventMsg_t tPair_EvtMsg = {0};
    uint16_t uwDelayMs;
	while(1)
	{
		if(ubPAIR_State == PAIR_NULL)
		{
			 osThreadSuspend(PAIR_ThreadId);
		}
#if (OP_STA || OP_AP_SLAVE)
		if(ubPAIR_State == PAIR_START)
		{
			PAIR_PreparePrp();
			ubPAIR_PaapCnt = 0;
			ulPAIR_TimeCnt = 0;
			ubPAIR_State = PAIR_PRP;
			uwDelayMs = PAIR_PRP_DELAY;
		}
		else if(ubPAIR_State == PAIR_PRP)
		{
//			printf("Sent TWC_PRP");
			if(tTWC_Send(TWC_AP_MASTER, TWC_PRP, (uint8_t *)&PAIR_PrpPket,sizeof(PAIR_PrpPket), 8) == TWC_SUCCESS)
				printf("Sent TWC_PRP_ok \n");

			uwDelayMs = PAIR_PRP_DELAY;
		}
		else if(ubPAIR_State == PAIR_PAAP)
		{
			printf("Sent TWC_PAAP\n");
			tTWC_Send(TWC_AP_MASTER, TWC_PAAP, (uint8_t *)&PAIR_PapPket, sizeof(PAIR_PapPket), 8);
			uwDelayMs = PAIR_PAAP_DELAY;
			ubPAIR_PaapCnt++;
			if(ubPAIR_PaapCnt == PAIR_PAAP_SENT_CNT)
			{
				ubPAIR_PaapCnt = 0;
				ubPAIR_State = PAIR_END;
			}
		}
		else if(ubPAIR_State == PAIR_END)
		{
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PRP);
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PAAP);
			BB_HoppingPairingEnd();
			printf("PAIR_END\n");
			tPair_EvtMsg.ubPAIR_Event 		= APP_PAIRING_SUCCESS_EVENT;
			tPair_EvtMsg.ubPAIR_Message[0] 	= PAIR_GetStaNumber();
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 5000);
			ulPAIR_TimeCnt = 0;
			osThreadSuspend(PAIR_ThreadId);
		}
		ulPAIR_TimeCnt += uwDelayMs;
		if(ulPAIR_TimeCnt > ulPAIR_Timeout)
		{
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PRP);
			tTWC_StopTwcSend(TWC_AP_MASTER, TWC_PAAP);
			BB_HoppingPairingEnd();
			PAIR_LoadId();
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 5000);
			printf("PAIR_TIMEOUT\n");
			osThreadSuspend(PAIR_ThreadId);
		}
		osDelay(uwDelayMs);
#endif		
#if OP_AP
		if(ubPAIR_State == PAIR_START)
		{
			printf("Wait PRP\n");
			uwDelayMs = PAIR_START_DELAY;
		}
		else if(ubPAIR_State == PAIR_PAP)
		{
//			printf("Sent PAIR_PAP "); 
			if(tTWC_Send(TWC_STA1, TWC_PAP, (uint8_t *)&PAIR_PapPket, sizeof(PAIR_PapPket), 8)== TWC_SUCCESS)
				printf("Sent PAIR_PAP ok \n");
			uwDelayMs = PAIR_PAP_DELAY;
		}
		else if(ubPAIR_State == PAIR_END)
		{
			tTWC_StopTwcSend(TWC_STA1, TWC_PRP);
			BB_HoppingPairingEnd();
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_SUCCESS_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 5000);
			printf("PAIR_END\n");
			ulPAIR_TimeCnt = 0;			
			osThreadSuspend(PAIR_ThreadId);
		}
		ulPAIR_TimeCnt += uwDelayMs;
		if(ulPAIR_TimeCnt > ulPAIR_Timeout)
		{
			tTWC_StopTwcSend(TWC_STA1, TWC_PRP);
			BB_HoppingPairingEnd();
			PAIR_LoadId();
			ubPAIR_State = PAIR_TIMEOUT;
			tPair_EvtMsg.ubPAIR_Event = APP_PAIRING_FAIL_EVENT;
			osMessagePut(*xAppEventQueue, &tPair_EvtMsg, 5000);
			osThreadSuspend(PAIR_ThreadId);
		} 
		osDelay(uwDelayMs);
#endif
	}
}
//------------------------------------------------------------------------------
void PAIR_Init(osMessageQId *pvMsgQId)
{
    xAppEventQueue = pvMsgQId;
	ulPAIR_SFAddr = pSF_Info->ulSize - (PAIR_SF_START_SECTOR * pSF_Info->ulSecSize);
#if (OP_STA||OP_AP_SLAVE)
	if(tTWC_RegTransCbFunc(TWC_PAP,NULL,PAIR_Pap) == TWC_FAIL)
	{
	}
#endif		
#if OP_AP
	if(tTWC_RegTransCbFunc(TWC_PRP,NULL,PAIR_Prp) == TWC_FAIL)
	{
	}	
    if(tTWC_RegTransCbFunc(TWC_PAAP,NULL,PAIR_Paap) == TWC_FAIL)
	{
	}
#endif
    ubPAIR_State  = PAIR_NULL;
	PAIR_LoadId();
	osThreadDef(PAIR_Thread, PAIR_Thread, THREAD_PRIO_PAIRING_HANDLER, 1, THREAD_STACK_PAIRING_HANDLER);
    PAIR_ThreadId = osThreadCreate(osThread(PAIR_Thread), NULL);
}    
//------------------------------------------------------------------------------
void PAIR_LoadId(void)
{
	SF_Read(ulPAIR_SFAddr, sizeof(PAIR_Info_t), (uint8_t *)&tPAIR_Info);
	memcpy(&PAIR_IdTable, &tPAIR_Info.tPAIR_Table, sizeof(PAIR_ID_TABLE));
#if OP_AP
	PAIR_CheckIdTable();
#endif
}
//------------------------------------------------------------------------------
void PAIR_SaveId(void)
{
    SF_DisableWrProtect();
    SF_Erase(SF_SE, ulPAIR_SFAddr, sizeof(PAIR_Info_t));
    #if OP_AP
    PAIR_IdTable.ubTxNumber = 0x0F;
    #endif
	memcpy(&tPAIR_Info.tPAIR_Table, &PAIR_IdTable, sizeof(PAIR_ID_TABLE));
    SF_Write(ulPAIR_SFAddr, sizeof(PAIR_Info_t), (uint8_t *)&tPAIR_Info);
	SF_EnableWrProtect();
}
//------------------------------------------------------------------------------
void PAIR_DeleteTxId(PAIR_TAG tStaNum)
{
	PAIR_SetDevInvaildId(tStaNum);
	PAIR_SaveId();
}
//------------------------------------------------------------------------------
void PAIR_SetDevInvaildId(PAIR_TAG tTag)
{
	switch(tTag)
	{
		case PAIR_AP_ASSIGN:
			PAIR_IdTable.ulAp_ID = 0xFEFFFFFE;	
			break;
		default:
			if(tTag <= PAIR_STA4)
				PAIR_IdTable.ulSTA_ID[tTag] = 0xFEFFFFFE;
			break;
	}
}
//------------------------------------------------------------------------------
uint8_t *PAIR_GetId(PAIR_TAG tPair_StaNum)
{
	static uint8_t ubPAIR_InvaildID[4] = {0xFE, 0xFF, 0xFF, 0xFE};

    if((tPair_StaNum == PAIR_AP_ASSIGN) || (tPair_StaNum == PAIR_AP_SLAVE))
        return (uint8_t *)&(PAIR_IdTable.ulAp_ID);
    else
        return (tPair_StaNum <= PAIR_STA4)?(uint8_t *)&(PAIR_IdTable.ulSTA_ID[tPair_StaNum]):(uint8_t *)&ubPAIR_InvaildID;
//    #if OP_STA	
//    return (uint8_t*)&(PAIR_IdTable.ulAp_ID);
//    #else
//    return (uint8_t*) &(PAIR_IdTable.ulSTA_ID[tPair_StaNum]);
//    #endif
}
//------------------------------------------------------------------------------
void PAIR_Start(PAIR_TAG tPair_StaNum, uint32_t ulPair_Timeout)
{
    if( ubPAIR_State > PAIR_TIMEOUT)
    {
        printf("Not in pairing mode PAP\n");
        return;
    }
    ulPAIR_Timeout = (ulPair_Timeout * 1000);
	BB_HoppingPairingStart();
    printf("PAIR_Start\n");
    ulPAIR_TimeCnt = 0;
    ubPAIR_Number  = tPair_StaNum;
    ubPAIR_State   = PAIR_START;
	osThreadResume(PAIR_ThreadId);
}
//------------------------------------------------------------------------------
void PAIR_Stop(void)
{
	ubPAIR_State 	= PAIR_TIMEOUT;	//! PAIR_END;
	ulPAIR_TimeCnt 	= ulPAIR_Timeout + 1;
}
//------------------------------------------------------------------------------
PAIR_STATE ubPAIR_GetPairState(void)
{
    if(ubPAIR_State == PAIR_TIMEOUT)
        return PAIR_TIMEOUT;
    else if((ubPAIR_State == PAIR_END) || (ubPAIR_State == PAIR_NULL))
        return PAIR_END;
    else 
        return PAIR_START;
}  
//------------------------------------------------------------------------------
#if (OP_STA||OP_AP_SLAVE)
void PAIR_PreparePrp(void)
{
    PAIR_PrpPket.ubTxNumber   = ubPAIR_Number;
    PAIR_PrpPket.ubIdCheckKey = Timer3->TM3_COUNTER;
}
//------------------------------------------------------------------------------
void PAIR_Pap(TWC_TAG GetSta,uint8_t *pData)
{
    uint8_t ubTemp;
    printf("Get PAP\n");
    if((ubPAIR_State < PAIR_PRP)&&(ubPAIR_State != PAIR_END))
    {
        printf("Not in pairing mode PAP %x\n",ubPAIR_State);
        return;
    }

    ubTemp = pData[0];
    if((pData[0] == PAIR_PrpPket.ubTxNumber) &&(PAIR_PrpPket.ubTxNumber == PAIR_AP_SLAVE)&& (ubPAIR_State == PAIR_PRP))
    {
        memcpy(&PAIR_PapPket, pData, sizeof(PAIR_PapPket));
		memcpy(&PAIR_IdTable, pData, sizeof(PAIR_ID_TABLE));
        PAIR_SaveId();
        ubPAIR_State = PAIR_PAAP;
    }
    else if(((pData[0] == PAIR_PrpPket.ubTxNumber)||(PAIR_PrpPket.ubTxNumber == PAIR_AP_ASSIGN))&&(PAIR_PrpPket.ubIdCheckKey == pData[ubTemp*4+8]) && (ubPAIR_State == PAIR_PRP))
    {
        memcpy(&PAIR_PapPket, pData, sizeof(PAIR_PapPket));
		memcpy(&PAIR_IdTable, pData, sizeof(PAIR_ID_TABLE));
        PAIR_SaveId();
        ubPAIR_State = PAIR_PAAP;
    }
}
//------------------------------------------------------------------------------
void PAIR_LoadPairingResult(uint8_t *pRole)
{
    *pRole = PAIR_GetStaNumber();
}
#endif
//------------------------------------------------------------------------------
PAIR_TAG PAIR_GetStaNumber(void)
{
    return (PAIR_TAG)PAIR_IdTable.ubTxNumber;
}
//------------------------------------------------------------------------------
#if OP_AP
void PAIR_PreparePap(void)
{
    uint32_t ulTemp;
    memcpy(&PAIR_PapPket , &PAIR_IdTable, sizeof(PAIR_RAP_Hdr));
    if(PAIR_PrpPket.ubTxNumber == PAIR_AP_ASSIGN)
    {
        ubPAIR_Number = ubPAIR_Number;
    }
    else
    {
        ubPAIR_Number = PAIR_PrpPket.ubTxNumber;
    }
#if (DISPLAY_MODE == DISPLAY_1T1R)
	tPAIR_Info.ulPAIR_Sign = 0;
#endif
    if(tPAIR_Info.ulPAIR_Sign != PAIR_SIGN)	//!< if(PAIR_PapPket.ubTxNumber != 0x0F)
    {
        ulTemp  = Timer3->TM3_COUNTER<<8;
        ulTemp &= 0x00FFFF00;
        PAIR_PapPket.ulAp_ID 	 = ulTemp + 0x0F000000;
        PAIR_PapPket.ulSTA_ID[0] = ulTemp + 0x01000000;
        PAIR_PapPket.ulSTA_ID[1] = ulTemp + 0x02000000;
        PAIR_PapPket.ulSTA_ID[2] = ulTemp + 0x03000000;
        PAIR_PapPket.ulSTA_ID[3] = ulTemp + 0x04000000;
		tPAIR_Info.ulPAIR_Sign   = PAIR_SIGN;
    }
    PAIR_PapPket.ubTxNumber = ubPAIR_Number; 
    if(PAIR_PrpPket.ubTxNumber != PAIR_AP_SLAVE)
    {
		ulTemp = Timer3->TM3_COUNTER;
		srand(ulTemp);
		ulTemp = rand();
		PAIR_PapPket.ulSTA_ID[ubPAIR_Number]  = ulTemp;
        PAIR_PapPket.ulSTA_ID[ubPAIR_Number] &= 0xFFFFFF00;
//        PAIR_PapPket.ulSTA_ID[ubPAIR_Number] += 0x100;
        PAIR_PapPket.ulSTA_ID[ubPAIR_Number] |= PAIR_PrpPket.ubIdCheckKey;
    }  
}
//------------------------------------------------------------------------------
void PAIR_Prp(TWC_TAG GetSta,uint8_t *pData)
{
    printf("Get PRP\n");
    if(ubPAIR_State < PAIR_START)
    {
        printf("Not in pairing mode PRP\n");
        return;
    }

    if(ubPAIR_State == PAIR_START)
    {
        memcpy(&PAIR_PrpPket, pData, sizeof(PAIR_RRP_Hdr));
        PAIR_PreparePap();
        ubPAIR_State = PAIR_PAP;
    }
}
//------------------------------------------------------------------------------
void PAIR_Paap(TWC_TAG GetSta,uint8_t *pData)
{
    printf("Get PAAP\n");
    if((ubPAIR_State < PAIR_PAP)&&(ubPAIR_State != PAIR_END))
    {
        printf("Not in pairing mode PAAP %d\n",ubPAIR_State);
        return;
    }
    if(ubPAIR_State == PAIR_PAP)
    {
        if(memcmp(&PAIR_PapPket , pData, sizeof(PAIR_RAP_Hdr))==0)
        {
			memcpy(&PAIR_IdTable, pData, sizeof(PAIR_ID_TABLE));
            PAIR_SaveId();
            ubPAIR_State = PAIR_END;
        }
        else
            printf("memcmp err\n");
    }
}
//------------------------------------------------------------------------------
void PAIR_CheckIdTable(void)
{
#if ((DISPLAY_MODE == DISPLAY_1T1R) || (DISPLAY_MODE == DISPLAY_2T1R))
	uint8_t i;

	for(i = DISPLAY_MODE; i < 4; i++)
		PAIR_IdTable.ulSTA_ID[i] = 0xFEFFFFFE;
	PAIR_SaveId();
#endif
}
#endif
//------------------------------------------------------------------------------
void PAIR_ShowDeviceID(void)
{
	uint8_t i;

    printf("ID Table: 0x%X\n", PAIR_IdTable.ubTxNumber);
	printf("    [%X] : %02X.%02X.%02X.%02X\n", 0xF, *((uint8_t *)&PAIR_IdTable.ulAp_ID), *((uint8_t *)&PAIR_IdTable.ulAp_ID + 1),
	                                                *((uint8_t *)&PAIR_IdTable.ulAp_ID + 2), *((uint8_t *)&PAIR_IdTable.ulAp_ID + 3));
	for(i = 0; i < 4; i++)
	{
		printf("    [%X] : %02X.%02X.%02X.%02X\n", i, *((uint8_t *)&PAIR_IdTable.ulSTA_ID[i]), *((uint8_t *)&PAIR_IdTable.ulSTA_ID[i] + 1),
	                                                  *((uint8_t *)&PAIR_IdTable.ulSTA_ID[i] + 2), *((uint8_t *)&PAIR_IdTable.ulSTA_ID[i] + 3));
	}
	printf("\n");
}
//------------------------------------------------------------------------------
uint16_t uwPAIR_GetVersion(void)
{
    return ((PAIR_MAJORVER << 8) + PAIR_MINORVER);
}
