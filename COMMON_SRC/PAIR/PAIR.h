/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PAIR.h
	\brief		Pairing header
	\author		Bing
	\version	0.4
	\date		2017/11/30
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef _PAIR_H_
#define _PAIR_H_

#include "_510PF.h"
#include "TWC_API.h"

/*_____ I N C L U D E S ____________________________________________________*/

#define PAIR_SIGN				0x5105046

typedef enum
{
	PAIR_STA1		= 0,
	PAIR_STA2		= 1,
	PAIR_STA3		= 2,
	PAIR_STA4		= 3,
	PAIR_AP_SLAVE	= 4,
	PAIR_AP_ASSIGN	= 0xF,
}PAIR_TAG;	

typedef enum 
{
	PAIR_NULL = 0,
    PAIR_END,
    PAIR_TIMEOUT,
	PAIR_START,
	PAIR_PRP,
	PAIR_PAP,					
	PAIR_PAAP	
}PAIR_STATE;

typedef struct
{
	uint8_t						ubTxNumber; 
	uint8_t						ubIdCheckKey;
}PAIR_RRP_Hdr;

typedef struct
{
	uint8_t						ubTxNumber; 
	uint32_t					ulAp_ID;
    uint32_t					ulSTA_ID[4];
}PAIR_RAP_Hdr, PAIR_ID_TABLE;

typedef struct
{
	uint32_t 					ulPAIR_Sign;
	PAIR_ID_TABLE				tPAIR_Table;
}PAIR_Info_t;

typedef struct
{
	uint8_t ubPAIR_Event;
	uint8_t ubPAIR_Message[7];
}PAIR_EventMsg_t;

#define PAIR_TIMEOUT_DELAY			0xFF00
#define PAIR_START_DELAY	    	200  	//!< 50 msec
#define PAIR_PRP_DELAY				300  	//!< 50 msec
#define PAIR_PAP_DELAY				500  	//!< 20 msec
#define PAIR_PAAP_DELAY				100  	//!< 20 msec
#define PAIR_END_DELAY				100  	//!< 50 msec
#define PAIR_PAAP_SENT_CNT      	10

//------------------------------------------------------------------------------
/*!
\brief Pairing function initialize
\param pvMsgQId			Message Queue handle
\return(no)
*/
void PAIR_Init(osMessageQId *pvMsgQId);
//------------------------------------------------------------------------------
/*!
\brief Pairing start function
\param tPair_StaNum		Station Number
\param ulPair_Timeout	Pairing timeout, Unit: seconds
\return(no)
*/
void PAIR_Start(PAIR_TAG tPair_StaNum, uint32_t ulPair_Timeout);
//------------------------------------------------------------------------------
/*!
\brief Pairing stop function
\return(no)
*/
void PAIR_Stop(void);
//------------------------------------------------------------------------------
/*!
\brief Pairing stop function
\return Station number
*/
PAIR_TAG PAIR_GetStaNumber(void);
//------------------------------------------------------------------------------
/*!
\brief Delete TX ID
\param tStaNum			Station Number
\return(no)
*/
void PAIR_DeleteTxId(PAIR_TAG tStaNum);
void PAIR_SetDevInvaildId(PAIR_TAG tTag);

void PAIR_LoadId(void);
void PAIR_SaveId(void); 
uint8_t *PAIR_GetId(PAIR_TAG tPair_SrcNum);
void PAIR_Task(void* pdata);
PAIR_STATE ubPAIR_GetPairState(void);
#if (OP_STA||OP_AP_SLAVE)	
void PAIR_PreparePrp(void);
void PAIR_Pap(TWC_TAG GetSta,uint8_t *pData);
void PAIR_LoadPairingResult(uint8_t *pRole);
#endif
#if OP_AP
void PAIR_PreparePap(void);
void PAIR_Prp(TWC_TAG GetSta,uint8_t *pData);
void PAIR_Paap(TWC_TAG GetSta,uint8_t *pData);
void PAIR_CheckIdTable(void);
#endif
void PAIR_ShowDeviceID(void);
//------------------------------------------------------------------------
/*!
\brief 	Get Pairing Version	
\return	Version
*/
uint16_t uwPAIR_GetVersion(void);
#endif
