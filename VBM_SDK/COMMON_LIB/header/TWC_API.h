/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TWC_API.h
	\brief		Two Way Command API header
	\author		Bing
	\version	0.1
	\date		2017/02/16
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef __TWC_API_H
#define __TWC_API_H

#include <stdint.h>

typedef enum
{
	TWC_STA1		= 0,
	TWC_STA2		= 1,
	TWC_STA3		= 2,
	TWC_STA4		= 3,
	TWC_AP_SLAVE	= 4,
	TWC_AP_MASTER	= 0xF,
}TWC_TAG;

typedef enum
{
	TWC_RESEV	= 0,
	TWC_Test1	= 1,
	TWC_Test2	= 2,
	TWC_APP,	
	TWC_EN,
	TWC_PRP,
    TWC_PAP,
    TWC_PAAP,
	TWC_UI_SETTING,
	TWC_RESEND_I,
	TWC_VDORES_SETTING,
//	............
	TWC_USE63	= 63,	 
}TWC_OPC;

typedef enum
{
	TWC_SUCCESS	= 0,
	TWC_FAIL,
	TWC_BUSY,
}TWC_STATUS;

typedef void(*pvRptTxStsFunc)(TWC_TAG, TWC_STATUS);
typedef void(*pvRecvDataFunc)(TWC_TAG, uint8_t *);

//------------------------------------------------------------------------
/*!
\brief TWC Initial
\param (No)
\return(No)
*/
void TWC_Init(void);
//------------------------------------------------------------------------
/*!
\brief TWC Start
\return (No)
*/
void TWC_Start(void);
//------------------------------------------------------------------------
/*!
\brief Register transaction callback function of TWC
\param Opc				(input) TWC Operating Code
\param ReportSts_cb 	(input) Callback function: Report transmit status (ACK mode)
\param RecvData_cb 		(input) Callback function: Receive data
\return TWC_STATUS \n
		(TWC_SUCCESS)	Get TWC,Reqistered Success \n
		(TWC_FAIL)		Get TWC,Reqistered Fail \n
*/
TWC_STATUS tTWC_RegTransCbFunc(TWC_OPC Opc, pvRptTxStsFunc ReportSts_cb, pvRecvDataFunc RecvData_cb);
//------------------------------------------------------------------------
/*!
\brief Send TWC Command
\param Tag 		(input) Send TWC Target
\param Opc		(input) Send TWC Operating Code
\param *Data 	(input) Send TWC Data Address Point
\param ubLen 	(input) Send TWC Data Length
\param uRetry 	(input) Send TWC Retry Time
\par Note
	1. Operating Code = 1 ~ 63 \n
	2. Data Length Max = 32Bytes \n
\return TWC_STATUS \n
		(TWC_SUCCESS)	Set TWC In Buffer Success \n
		(TWC_FAIL)		Set TWC In Buffer Fail \n
		(TWC_BUSY)		Set TWC In Buffer Busy \n	
*/
TWC_STATUS tTWC_Send(TWC_TAG Tag,TWC_OPC Opc,uint8_t *Data,uint8_t ubLen,uint8_t ubRetry);
//------------------------------------------------------------------------
/*!
\brief Stop Send TWC Command
\param Tag 		(input) Stop Send TWC Target
\param Opc		(input) Stop Send TWC Operating Code
\return TWC_STATUS \n
		(TWC_SUCCESS)	TWC Operating Code Exist \n
		(TWC_FAIL)		TWC Operating Code Not Exist \n
*/
TWC_STATUS tTWC_StopTwcSend(TWC_TAG Tag,TWC_OPC Opc);
//------------------------------------------------------------------------
/*!
\brief 	Get TWC Version	
\return	Version
*/
uint16_t uwTWC_GetVersion(void);
#endif	/*__ST53510_TWC_API_H*/


