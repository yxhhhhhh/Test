/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CQI2C.c
	\brief		I2C controlled by Command Queue
	\author		Nick Huang
	\version	0.2
	\date		2017/05/22
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "CQI2C.h"
#include "CQ_API.h"
#include "INTC.h"
#include <stdlib.h>

//------------------------------------------------------------------------------
uint8_t ubCQI2C_CqHandle;
//------------------------------------------------------------------------------
void CQI2C_Init(void)
{
	uint32_t ulReturn;
	(void)ulReturn;

	ubCQI2C_CqHandle = ubCQ_CreateCmdQ(&CQI2C_CqEndIsr, CQI2C_CQ_POLLING_PERIOD);
	if(ubCQI2C_CqHandle == CQ_FAIL)
	{
		printf("Create CmdQ for CQI2C fail\n");
		return;
	}

	CQ_LABEL(WAIT_VSYNC);
	CQ_CMD_ADD(CQ_POLLING_BITS, CQI2C_DLY_10US, (uint32_t)&(INTC->IRQ0_SCR), INTC_SEN_VSYNC_IRQ, WAIT_VSYNC, ulReturn);
}
//------------------------------------------------------------------------------
CQI2C_WrDataSet_t* CQI2C_CreateCQ_I2C_Write(uint8_t ubSlvAddr, uint8_t* pWrBuf, uint32_t ulWrSz)
{
	int i;
	CQI2C_WrDataSet_t *pDataSet;
	uint8_t  ubWrAddr = ubSlvAddr << 1;

	if(ulWrSz > 8)
	{
		printf("CQI2C Write size is too large...\n");
		return NULL;
	}
	if(ulWrSz)
	{
		pDataSet = malloc(sizeof(CQI2C_WrDataSet_t));
		//pDataSet = pvPortMalloc(sizeof(CQI2C_WrDataSet_t));
		pDataSet->ubCmdNum = ulWrSz;
		CQI2C_MasterWrAddr(ubWrAddr);
		for(i=0;i<ulWrSz;i++)
			#if ((SEN_USE == SEN_OV9715)	|| (SEN_USE == SEN_OV9732))
				// address1, data1
				pDataSet->ulCmdAddr[i] = ulCQI2C_MasterWrData(pWrBuf[i], (i == ulWrSz - 1) ? CQI2C_DATA_STOP : CQI2C_DATA_NEXT);
			#elif ((SEN_USE == SEN_AR0330) 	|| (SEN_USE == SEN_AR0130))
				// address1, address2, data1, data2
				pDataSet->ulCmdAddr[i] = ulCQI2C_MasterWrData(pWrBuf[i], (i == ulWrSz - 2) ? CQI2C_DATA_STOP : CQI2C_DATA_NEXT);		
			#endif
		return pDataSet;
	}

	return NULL;
}
//------------------------------------------------------------------------------
void CQI2C_ModifyCQ_I2C_Write(CQI2C_WrDataSet_t* pDataSet, uint8_t* pbData)
{
	int i;
	uint32_t ulReturn;
	(void)ulReturn;

	if(pDataSet == NULL)
		return;
	//CQ_CMD_ADD(CQ_SINGLE_WRITE, CQI2C_DLY_1US, CQI2C_DR, ubData, ulReturn);
	for(i=0;i<pDataSet->ubCmdNum;i++)
		CQ_CMD_SET(CQ_SINGLE_WRITE, ubCQI2C_CqHandle, pDataSet->ulCmdAddr[i], CQI2C_DLY_1US, CQI2C_DR, pbData[i], ulReturn);
}
//------------------------------------------------------------------------------
void CQI2C_Complete(void)
{
	uint32_t ulReturn;
	(void)ulReturn;
	
	CQ_CMD_ADD(CQ_SET_PC, CQI2C_DLY_1US, WAIT_VSYNC, ulReturn);
	// Complete CmdQ Creation
	CQ_CompleteCmdQ();

	if(ubCQ_CoreAssign(CQ_Core1, ubCQI2C_CqHandle) != CQ_OK)
		printf("CQI2C assign to Core1 fail\n");
	
	printf("CQI2C size = %d\n", uwCQ_GetCmdQSz(ubCQI2C_CqHandle));
}
//------------------------------------------------------------------------------
void CQI2C_Start(void)
{
	CQ_CoreStart(CQ_Core1);
}
//------------------------------------------------------------------------------
void CQI2C_MasterWrAddr(uint8_t ubAddr)
{
	uint32_t ulReturn;
	(void)ulReturn;

	CQI2C_MasterWaitReady();
	CQ_CMD_ADD(CQ_SINGLE_WRITE, CQI2C_DLY_1US, CQI2C_DR, ubAddr, ulReturn);
	CQ_CMD_ADD(CQ_PARTIAL_WRITE, CQI2C_DLY_1US, CQI2C_CR, 0xFFFFFFFF, CQI2C_MASK(CQI2C_START), ulReturn);
	CQ_CMD_ADD(CQ_PARTIAL_WRITE, CQI2C_DLY_1US, CQI2C_CR, 0xFFFFFFFF, CQI2C_MASK(CQI2C_SCL_EN), ulReturn);
	CQI2C_MasterTxDone();
}
//------------------------------------------------------------------------------
uint32_t ulCQI2C_MasterWrData(uint8_t ubData, uint8_t ubMode)
{
	uint32_t ulReturn, ulRetVal;
	(void)ulReturn;

	CQI2C_MasterWaitReady();
	CQ_CMD_ADD(CQ_SINGLE_WRITE, CQI2C_DLY_1US, CQI2C_DR, ubData, ulRetVal);
	if(ubMode == CQI2C_DATA_STOP)
	{
		CQ_CMD_ADD(CQ_PARTIAL_WRITE, CQI2C_DLY_1US, CQI2C_CR, 0xFFFFFFFF, CQI2C_MASK(CQI2C_STOP), ulReturn);
		CQI2C_MasterStopDone();
	}
	else
	{
		CQ_CMD_ADD(CQ_PARTIAL_WRITE, CQI2C_DLY_1US, CQI2C_CR, 0xFFFFFFFF, CQI2C_MASK(CQI2C_TB_EN), ulReturn);
		CQI2C_MasterTxDone();
	}
	
	return ulRetVal;
}
//------------------------------------------------------------------------------
void CQI2C_MasterWaitReady(void)
{
	static uint8_t ubWaitRdyCnt;
	char str[20];
	uint32_t ulReturn;
	(void)ulReturn;

	if(sprintf(str, "WaitRdy_%i", ubWaitRdyCnt++) >= 20) {
		printf("WaitRdy_%i label is too long...\n", ubWaitRdyCnt - 1);
		return;
	}
	CQ_LABEL_LBS(str);
	CQ_CMD_ADD(CQ_PARTIAL_COMPARE_LBS, CQI2C_DLY_20US, CQI2C_SR, 0, CQI2C_MASK(CQI2C_BB), str, ulReturn);
}
//------------------------------------------------------------------------------
void CQI2C_MasterStopDone(void)
{
	static uint8_t ubWaitStopDoneCnt;
	char str[20];
	uint32_t ulReturn;
	(void)ulReturn;
	
	if(sprintf(str, "WaitStopDone_%i", ubWaitStopDoneCnt++) >= 20) {
		printf("WaitStopDone_%i label is too long...\n", ubWaitStopDoneCnt - 1);
		return;
	}
	CQ_LABEL_LBS(str);
	CQ_CMD_ADD(CQ_POLLING_BITS_LBS, CQI2C_DLY_20US, CQI2C_SR, CQI2C_STOP_FLAG, str, ulReturn);
	CQ_CMD_ADD(CQ_PARTIAL_WRITE, CQI2C_DLY_1US, CQI2C_CR, 0, CQI2C_MASK(CQI2C_SCL_EN), ulReturn);
}
//------------------------------------------------------------------------------
void CQI2C_MasterTxDone(void)
{
	static uint8_t ubWaitTxDoneCnt;
	char str[20];
	uint32_t ulReturn;
	(void)ulReturn;

	if(sprintf(str, "WaitTxDone_%i", ubWaitTxDoneCnt++) >= 20) {
		printf("WaitTxDone_%i label is too long...\n", ubWaitTxDoneCnt - 1);
		return;
	}
	CQ_LABEL_LBS(str);
	CQ_CMD_ADD(CQ_POLLING_BITS_LBS, CQI2C_DLY_20US, CQI2C_SR, CQI2C_DT_FLAG, str, ulReturn);
}
//------------------------------------------------------------------------------
void CQI2C_CqEndIsr(void)
{
	printf("Error! CQI2C CQ_Core1 Stopped!\n");
}
//------------------------------------------------------------------------------
uint16_t uwCQI2C_GetVersion(void)
{
	return ((CQI2C_MAJOR_VER << 8) + CQI2C_MINOR_VER);
}
//------------------------------------------------------------------------------
