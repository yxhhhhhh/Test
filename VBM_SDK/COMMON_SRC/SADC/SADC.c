/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SADC.c
	\brief		SADC Control Funcation
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/04/26
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "SADC.h"
//------------------------------------------------------------------------------
#define SADC_MAJORVER	0
#define SADC_MINORVER	2

osMutexId SADC_Mutex;
SADC_Status_t tSADC_Status;
//------------------------------------------------------------------------------
void SADC_Init(void)
{
	tSADC_Status = SADC_DISABLE;
	osMutexDef(SADC_Mutex);
	SADC_Mutex = osMutexCreate(osMutex(SADC_Mutex));
}
//------------------------------------------------------------------------------
void SADC_UnInit(void)
{
	tSADC_Status = SADC_DISABLE;
	SARAD->SAR_CLK_EN = 0;
	SARAD->SAR_EN 	  = 0;
	if(SADC_Mutex != NULL)
		osMutexDelete(SADC_Mutex);
}
//------------------------------------------------------------------------------
void SADC_Enable(void)
{
	SARAD->SAR_CLK_EN 	= 1;
	SARAD->SAR_GPIO_EN 	= 0;
	SARAD->SAR_EN 	  	= 1;
	tSADC_Status 	  	= SADC_ENABLE;
}
//------------------------------------------------------------------------------
void SADC_Disable(void)
{
	SARAD->SAR_CLK_EN = 0;
	SARAD->SAR_EN 	  = 0;
}
//------------------------------------------------------------------------------
uint16_t uwSADC_GetReport(SADC_CH_t tCH)
{
	__I uint32_t *pSADC_RptMem = &(SARAD->CH_RPT);
	uint32_t ulSADC_RptValue   = 0xFFFF;
	uint16_t uwSADC_TimeOutVal = 0;

	if(tSADC_Status == SADC_DISABLE)
		return ulSADC_RptValue;

	osMutexWait(SADC_Mutex, osWaitForever);
	SARAD->CH_TRIGGER |= (1 << tCH);
	while(!(SARAD->CH_RDY & (1 << tCH)))
	{
		if(++uwSADC_TimeOutVal > 5000)
		{
			osMutexRelease(SADC_Mutex);
			printf("SADC Time Out \n");
			return ulSADC_RptValue;
		}
	}
	ulSADC_RptValue = (uint16_t)((*(pSADC_RptMem + tCH)) & 0x3FF);
	osMutexRelease(SADC_Mutex);
	return ulSADC_RptValue;
}
//------------------------------------------------------------------------------
uint16_t uwSADC_GetVersion(void)
{
    return ((SADC_MAJORVER << 8) + SADC_MINORVER);
}
