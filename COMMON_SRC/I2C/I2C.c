/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		I2C.c
	\brief		I2C funcations
	\author		Pierce
	\version	0.4
	\date		2017/10/18
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "TIMER.h"
#include "INTC.h"
#include "I2C.h"
//------------------------------------------------------------------------------
#define I2C_MAJORVER    0        //!< Major version
#define I2C_MINORVER    3        //!< Minor version
//------------------------------------------------------------------------------
static osMutexId 		I2C_Mutex1;
static osMutexId 		I2C_Mutex2;
static osSemaphoreId 	I2C_Sem1;
static osSemaphoreId 	I2C_Sem2;
static uint32_t	 		*pulI2C_SizeIsr1;
static uint32_t	 		*pulI2C_SizeIsr2;
static uint32_t  		ulI2C_CountIsr1;
static uint32_t  		ulI2C_CountIsr2;
static uint8_t   		*pubI2C_BufIsr1;
static uint8_t   		*pubI2C_BufIsr2;
static I2C_SLAVE_RW_TYP tI2C_RWIsr1;
static I2C_SLAVE_RW_TYP tI2C_RWIsr2;
static uint8_t  		ubI2C_IsrFlag1;
static uint8_t  		ubI2C_IsrFlag2;
//------------------------------------------------------------------------------
uint16_t uwI2C_GetVersion (void)
{
    return ((I2C_MAJORVER << 8) + I2C_MINORVER);
}
//------------------------------------------------------------------------------
void I2C_Reset (I2C1_Type *pI2C)
{
	pI2C->I2C_RST = 1;
	while(pI2C->I2C_RST);
}
//------------------------------------------------------------------------------
void I2C_Init (void)
{
	static bool bInitFlag = false;
	
	if (false == bInitFlag)
	{
		osMutexDef(I2C_Mutex1);
		I2C_Mutex1 = osMutexCreate(osMutex(I2C_Mutex1));		
		osMutexDef(I2C_Mutex2);
		I2C_Mutex2 = osMutexCreate(osMutex(I2C_Mutex2));
		bInitFlag = true;
	}
}
//------------------------------------------------------------------------------
I2C1_Type *pI2C_MasterInit (I2C_TYP tNth, I2C_SCL_SPEED_TYP tSclSpeed)
{
	I2C1_Type *pI2C;
	osMutexId *pMutex;
	
	I2C_Init();
	pMutex = (I2C_1 == tNth)?&I2C_Mutex1:&I2C_Mutex2;	
	osMutexWait(*pMutex, osWaitForever);
	if (I2C_1 != tNth)
	{
		GLB->I2C_SEL = tNth - 1;
		pI2C = I2C2;
	}
	else
		pI2C = I2C1;
	I2C_Sem1 = NULL;	
	I2C_Sem2 = NULL;	
	pubI2C_BufIsr1 = NULL;
	pubI2C_BufIsr2 = NULL;
	pulI2C_SizeIsr1 = NULL;	
	pulI2C_SizeIsr2 = NULL;	
	I2C_Reset(pI2C);
	pI2C->I2C_SCL_COUNT = ((4800 >> GLB->SYS_RATE) / GLB->APBC_RATE / tSclSpeed - pI2C->I2C_GSR) / 2 - 2;
	pI2C->I2C_EN = 1;	
	osMutexRelease(*pMutex);
	return pI2C;
}
//------------------------------------------------------------------------------
bool bI2C_MasterReady (I2C1_Type *pI2C)
{
	uint32_t ulTo = I2C_TIME_OUT;
	
	while(pI2C->I2C_BB)
	{
		--ulTo;
		if (!ulTo)	return false;
	}
	return true;
}
//------------------------------------------------------------------------------
bool bI2C_MasterTxDone (I2C1_Type *pI2C)
{
	uint32_t ulTo = I2C_TIME_OUT;

	while (!pI2C->I2C_DT_FLAG)
	{
		--ulTo;
		if (!ulTo)	return false;
	}
	return true;
}
//------------------------------------------------------------------------------
bool bI2C_MasterRxDone (I2C1_Type *pI2C)
{
	uint32_t ulTo = I2C_TIME_OUT;
	
	while(!pI2C->I2C_DR_FLAG)
	{
		--ulTo;
		if (!ulTo)	return false;
	}
	return true;
}
//------------------------------------------------------------------------------
bool bI2C_MasterStopDone (I2C1_Type *pI2C)
{
	uint32_t ulTo = I2C_TIME_OUT;
	
	while(!pI2C->I2C_STOP_FLAG)
	{
		--ulTo;
		if (!ulTo)	return false;
	}
	pI2C->I2C_SCL_EN = 0;
	return true;
}
//------------------------------------------------------------------------------
bool bI2C_MasterStop (I2C1_Type *pI2C)
{
	bool bFlag = true;
	
	if (false == bI2C_MasterReady(pI2C))	bFlag = false;	
	pI2C->I2C_STOP = 1;
	if (false == bI2C_MasterStopDone(pI2C))	
	{
		I2C_Reset(pI2C);	
		bFlag = false;
	}		
	return bFlag;
}
//------------------------------------------------------------------------------
bool bI2C_MasterWrite (I2C1_Type *pI2C, uint8_t ubData, uint8_t ubMode)
{
	if (true == bI2C_MasterReady(pI2C))
	{	
		pI2C->I2C_DR = ubData;
		if (I2C_DATA_STOP == ubMode)
		{
			pI2C->I2C_STOP = 1;
			return bI2C_MasterStopDone(pI2C);
		}
		else
		{
			pI2C->I2C_TB_EN = 1;
			return bI2C_MasterTxDone(pI2C);
		}
	}
	return false;
}
//------------------------------------------------------------------------------
bool bI2C_MasterWriteAddr (I2C1_Type *pI2C, uint8_t ubAddr)
{
	if (true == bI2C_MasterReady(pI2C))
	{
		pI2C->I2C_DR = ubAddr;
		pI2C->I2C_START = 1;
		pI2C->I2C_SCL_EN = 1;
		return bI2C_MasterTxDone(pI2C);
	}
	return false;		
}
//------------------------------------------------------------------------------
bool bI2C_MasterRead (I2C1_Type *pI2C, uint8_t *pData, uint8_t ubAck)
{
	if (true == bI2C_MasterReady(pI2C))
	{
		pI2C->I2C_NACK = ubAck;
		pI2C->I2C_TB_EN = 1;
		if (false == bI2C_MasterRxDone(pI2C))	return false;
		*pData = pI2C->I2C_DR;		
		return true;
	}
	return false;	
}
//------------------------------------------------------------------------------
bool bI2C_MasterProcessInternal (I2C1_Type *pI2C, uint8_t ubSlvAddr, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz)
{
	uint32_t uli;
	uint8_t  ubWrAddr = ubSlvAddr << 1;
	uint8_t  ubRdAddr = (ubSlvAddr << 1) | 1;	
	
	if (ulWrSz)
	{
		if (false == bI2C_MasterWriteAddr(pI2C, ubWrAddr))	goto I2C_FAIL_STOP;
		for (uli=0; uli<ulWrSz; ++uli)
			if (false == bI2C_MasterWrite(pI2C, pWrBuf[uli], (uli == ulWrSz - 1)?I2C_DATA_STOP:I2C_DATA_NEXT))	goto I2C_FAIL_STOP;
	}
	if (ulRdSz)
	{
		if (false == bI2C_MasterWriteAddr(pI2C, ubRdAddr))	goto I2C_FAIL_STOP;
		for (uli=0; uli<ulRdSz; ++uli)
			if (false == bI2C_MasterRead(pI2C, &pRdBuf[uli], ((ulRdSz == uli + 1)?I2C_DO_NACK:I2C_DO_ACK)))	goto I2C_FAIL_STOP;
		return bI2C_MasterStop(pI2C);
	}
	return true;
I2C_FAIL_STOP:
	bI2C_MasterStop(pI2C);
	return false;
}
//------------------------------------------------------------------------------
bool bI2C_MasterProcess (I2C1_Type *pI2C, uint8_t ubSlvAddr, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz)
{
	bool bFlag;
	osMutexId *pMutex;
	
	pMutex = (I2C1 == pI2C)?&I2C_Mutex1:&I2C_Mutex2;
	osMutexWait(*pMutex, osWaitForever);	
	bFlag = bI2C_MasterProcessInternal (pI2C, ubSlvAddr, pWrBuf, ulWrSz, pRdBuf, ulRdSz);
	osMutexRelease(*pMutex);
	return bFlag;
	
}
//------------------------------------------------------------------------------
void I2C_Master1Isr (void)
{
	uint16_t uwFlag;

	uwFlag = I2C1->REG_0x0004;
	INTC_IrqClear (INTC_I2C1_IRQ);
	if (I2C_START_F & uwFlag)
	{
		if (I2C1->I2C_START_INTR_EN)
		{
			if (I2C_RW_F & uwFlag)
				I2C1->I2C_DR_INTR_EN =1;
			else
				I2C1->I2C_DT_INTR_EN = 1;				
			I2C1->I2C_BERR_INTR_EN = 1;
			I2C1->I2C_STOP_INTR_EN = 1;
//			GPIO->GPIO_O1 = ~GPIO->GPIO_O1;
		}
	}
	if (I2C_STOP_F & uwFlag)
	{
		if (I2C_TX_DONE & uwFlag)	--(*pulI2C_SizeIsr1);
		I2C1->I2C_SCL_EN = 0;
		osSemaphoreRelease(I2C_Sem1);		
	}
	else if (I2C_RX_DONE & uwFlag)
	{
		pubI2C_BufIsr1[ulI2C_CountIsr1] = I2C1->I2C_DR;
		++ulI2C_CountIsr1;
		--(*pulI2C_SizeIsr1);
		if (*pulI2C_SizeIsr1)
		{
			I2C1->I2C_NACK = (*pulI2C_SizeIsr1 > 1)?0:1;
			I2C1->I2C_TB_EN = 1;
		}
		else
			I2C1->I2C_STOP = 1;
	}
	else if ((I2C_BERR_F & uwFlag) && !I2C1->I2C_BB)
		I2C1->I2C_STOP = 1;
	else if (I2C_TX_DONE & uwFlag)
	{
		if (I2C_RW_F & uwFlag)
		{
			I2C1->I2C_NACK = (*pulI2C_SizeIsr1 > 1)?0:1;
			I2C1->I2C_TB_EN = 1;
		}
		else
		{
			if (ulI2C_CountIsr1)	--(*pulI2C_SizeIsr1);
			if (*pulI2C_SizeIsr1)
			{
				I2C1->I2C_DR = pubI2C_BufIsr1[ulI2C_CountIsr1];
				++ulI2C_CountIsr1;
				if (*pulI2C_SizeIsr1 > 1)
					I2C1->I2C_TB_EN = 1;
				else
					I2C1->I2C_STOP = 1;
			}
		}
	}
	GPIO->GPIO_O0 = ~GPIO->GPIO_O0;
}
//------------------------------------------------------------------------------
void I2C_Master2Isr (void)
{
	uint16_t uwFlag;

	uwFlag = I2C2->REG_0x0004;
	INTC_IrqClear (INTC_I2C2_IRQ);
	if (I2C_START_F & uwFlag)
	{
		if (I2C2->I2C_START_INTR_EN)
		{
			if (I2C_RW_F & uwFlag)
				I2C2->I2C_DR_INTR_EN =1;
			else
				I2C2->I2C_DT_INTR_EN = 1;				
			I2C2->I2C_BERR_INTR_EN = 1;
			I2C2->I2C_STOP_INTR_EN = 1;
//			GPIO->GPIO_O1 = ~GPIO->GPIO_O1;
		}
	}
	if (I2C_STOP_F & uwFlag)
	{
		if (I2C_TX_DONE & uwFlag)	--(*pulI2C_SizeIsr2);
		I2C2->I2C_SCL_EN = 0;
		osSemaphoreRelease(I2C_Sem2);
	}
	else if (I2C_RX_DONE & uwFlag)
	{
		pubI2C_BufIsr2[ulI2C_CountIsr2] = I2C2->I2C_DR;
		++ulI2C_CountIsr2;
		--(*pulI2C_SizeIsr2);
		if (*pulI2C_SizeIsr2)
		{
			I2C2->I2C_NACK = (*pulI2C_SizeIsr2 > 1)?0:1;
			I2C2->I2C_TB_EN = 1;
		}
		else
			I2C2->I2C_STOP = 1;
	}
	else if ((I2C_BERR_F & uwFlag) && !I2C2->I2C_BB)
		I2C2->I2C_STOP = 1;
	else if (I2C_TX_DONE & uwFlag)
	{
		if (I2C_RW_F & uwFlag)
		{
			I2C2->I2C_NACK = (*pulI2C_SizeIsr2 > 1)?0:1;
			I2C2->I2C_TB_EN = 1;
		}
		else
		{
			if (ulI2C_CountIsr2)	--(*pulI2C_SizeIsr2);
			if (*pulI2C_SizeIsr2)
			{
				I2C2->I2C_DR = pubI2C_BufIsr2[ulI2C_CountIsr2];
				++ulI2C_CountIsr2;
				if (*pulI2C_SizeIsr2 > 1)
					I2C2->I2C_TB_EN = 1;
				else
					I2C2->I2C_STOP = 1;
			}
		}
	}
	GPIO->GPIO_O0 = ~GPIO->GPIO_O0;
}
//------------------------------------------------------------------------------
bool bI2C_MasterIntRwInternal (I2C1_Type *pI2C, uint8_t ubMode, uint8_t ubAddr, uint8_t *pBuf, uint32_t ulSz)
{
	if (!ulSz)	return true;
	if (true == bI2C_MasterReady(pI2C))
	{
		if (I2C_START_INT == ubMode)
			pI2C->I2C_START_INTR_EN = 1;
		else
		{
			pI2C->I2C_DT_INTR_EN = 1;
			pI2C->I2C_DR_INTR_EN = (ubAddr & 1)?1:0;
			pI2C->I2C_BERR_INTR_EN = 1;
			pI2C->I2C_STOP_INTR_EN = 1;
		}
		if (I2C1 == pI2C)
		{
			osSemaphoreDef(I2C_Sem1);
			I2C_Sem1    = osSemaphoreCreate(osSemaphore(I2C_Sem1), 1);
			osSemaphoreWait(I2C_Sem1, osWaitForever);
			INTC_IrqSetup (INTC_I2C1_IRQ, INTC_LEVEL_TRIG, I2C_Master1Isr);
			INTC_IrqClear (INTC_I2C1_IRQ);
			INTC_IrqEnable (INTC_I2C1_IRQ);
			pubI2C_BufIsr1 = pBuf;
			pulI2C_SizeIsr1 = &ulSz;
			ulI2C_CountIsr1 = 0;
			pI2C->I2C_DR = ubAddr;
			pI2C->I2C_START = 1;
			pI2C->I2C_SCL_EN = 1;
			osSemaphoreWait(I2C_Sem1, 100);
			
			INTC_IrqDisable (INTC_I2C1_IRQ);
			pI2C->I2C_START_INTR_EN = 0;
			pI2C->I2C_DT_INTR_EN = 0;
			pI2C->I2C_DR_INTR_EN = 0;
			pI2C->I2C_BERR_INTR_EN = 0;
			pI2C->I2C_STOP_INTR_EN = 0;
			INTC_IrqClear (INTC_I2C1_IRQ);
			pI2C->I2C_SCL_EN = 0;
			osSemaphoreDelete(I2C_Sem1);
		}
		else
		{
			osSemaphoreDef(I2C_Sem2);
			I2C_Sem2    = osSemaphoreCreate(osSemaphore(I2C_Sem2), 1);
			osSemaphoreWait(I2C_Sem2, osWaitForever);
			INTC_IrqSetup (INTC_I2C2_IRQ, INTC_LEVEL_TRIG, I2C_Master2Isr);
			INTC_IrqClear (INTC_I2C2_IRQ);
			INTC_IrqEnable (INTC_I2C2_IRQ);
			pubI2C_BufIsr2 = pBuf;
			pulI2C_SizeIsr2 = &ulSz;
			ulI2C_CountIsr2 = 0;
			pI2C->I2C_DR = ubAddr;
			pI2C->I2C_START = 1;
			pI2C->I2C_SCL_EN = 1;
			osSemaphoreWait(I2C_Sem2, 100);
			
			INTC_IrqDisable (INTC_I2C2_IRQ);
			pI2C->I2C_START_INTR_EN = 0;
			pI2C->I2C_DT_INTR_EN = 0;
			pI2C->I2C_DR_INTR_EN = 0;
			pI2C->I2C_BERR_INTR_EN = 0;
			pI2C->I2C_STOP_INTR_EN = 0;
			INTC_IrqClear (INTC_I2C2_IRQ);
			pI2C->I2C_SCL_EN = 0;
			osSemaphoreDelete(I2C_Sem2);
		}
		return (ulSz)?false:true;
	}
	return false;
}
//------------------------------------------------------------------------------
bool bI2C_MasterIntRw (I2C1_Type *pI2C, uint8_t ubMode, uint8_t ubAddr, uint8_t *pBuf, uint32_t ulSz)
{
	bool bFlag;
	osMutexId *pMutex;
	
	pMutex = (I2C1 == pI2C)?&I2C_Mutex1:&I2C_Mutex2;
	osMutexWait(*pMutex, osWaitForever);
	bFlag = bI2C_MasterIntRwInternal (pI2C, ubMode, ubAddr, pBuf, ulSz);
	osMutexRelease(*pMutex);
	return bFlag;
}
//------------------------------------------------------------------------------
bool bI2C_MasterInt (I2C1_Type *pI2C, uint8_t ubMode, uint8_t ubSlvAddr, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz)
{
	if (true == bI2C_MasterIntRw (pI2C, ubMode, ubSlvAddr << 1, pWrBuf, ulWrSz))
		return bI2C_MasterIntRw (pI2C, ubMode, (ubSlvAddr << 1) | 1, pRdBuf, ulRdSz);
	return false;
}
//------------------------------------------------------------------------------
I2C1_Type *pI2C_SlaveInit (I2C_TYP tNth, I2C_SLAVE_TYP tGc, I2C_SLAVE_ADDR_TYP tAddrTyp, uint16_t uwAddr)
{
	I2C1_Type *pI2C;
	osMutexId *pMutex;
	
	I2C_Init();
	pMutex = (I2C1 == pI2C)?&I2C_Mutex1:&I2C_Mutex2;
	osMutexWait(*pMutex, osWaitForever);
	if (I2C_1 != tNth)
	{
		GLB->I2C_SEL = tNth - 1;
		pI2C = I2C2;
	}
	else
		pI2C = I2C1;
	I2C_Reset(pI2C);
	pI2C->I2C_EN10 = tAddrTyp;
	pI2C->I2C_SAR = uwAddr;
	pI2C->I2C_GC_EN = tGc;
	pI2C->I2C_EN = 1;
	osMutexRelease(*pMutex);
	return pI2C;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveAddrMatch (I2C1_Type *pI2C)
{
	uint32_t ulTo = I2C_TIME_OUT;
	uint16_t uwFlag;
	
	while(--ulTo)
	{
		uwFlag = pI2C->REG_0x0004;
		if ((uwFlag & I2C_SAM_F) || (uwFlag & I2C_GC_F))	return true;
	}
	return false;	
}
//------------------------------------------------------------------------------
bool bI2C_SlaveWriteByte (I2C1_Type *pI2C, uint8_t ubData, uint32_t *ulSz)
{
	if (!pI2C->I2C_STOP_FLAG)
	{
		uint16_t uwFlag;
		uint32_t ulTo = I2C_TIME_OUT;		
			
		pI2C->I2C_DR = ubData;
		pI2C->I2C_TB_EN = 1;
		while (!((uwFlag = pI2C->REG_0x0004) & I2C_TX_DONE)) 
		{
			--ulTo;
			if (!ulTo || uwFlag & I2C_STOP_F)	return false;
		}		
		++(*ulSz);
		if (uwFlag & I2C_STOP_F)	return false;
		else if (uwFlag & I2C_NACK_F)
		{
			ulTo = I2C_TIME_OUT;
			while (!pI2C->I2C_STOP_FLAG)
			{
				--ulTo;
				if (!ulTo)	break;
			}
			return false;
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveReadByte (I2C1_Type *pI2C, uint8_t *ubData, uint32_t *ulSz)
{
	if (!pI2C->I2C_STOP_FLAG)
	{
		uint16_t uwFlag;
		uint32_t ulTo = I2C_TIME_OUT;
		
		pI2C->I2C_NACK = 0;
		pI2C->I2C_TB_EN = 1;
		while (!((uwFlag = pI2C->REG_0x0004) & I2C_RX_DONE)) 
		{
			--ulTo;
			if (!ulTo || uwFlag & I2C_STOP_F)	return false;
		}
		*ubData = pI2C->I2C_DR;
		if (pI2C->I2C_RW && !(*ulSz) && (I2C_SLAVE_ADDR10_HMASK | (I2C_SLAVE_ADDR10 >> 7) | 1) == *ubData)	return true;
		++(*ulSz);
		if (uwFlag & I2C_STOP_F)	return false;		
		return true;		
	}
	return false;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveProcessInternal (I2C1_Type *pI2C, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz)
{
	uint32_t ulWrCnt, ulRdCnt;
	
	ulWrCnt = ulRdCnt = 0;	
	if (true == bI2C_SlaveAddrMatch(pI2C))
	{
		while(1)
		{
			if (!pI2C->I2C_RW)
			{
				uint8_t ubTemp;
				
				if (false == bI2C_SlaveReadByte(pI2C, (ulRdSz > ulRdCnt)?&pRdBuf[ulRdCnt]:&ubTemp, &ulRdCnt))	break;
			}
			else
				if (false == bI2C_SlaveWriteByte(pI2C, (ulWrSz > ulWrCnt)?pWrBuf[ulWrCnt]:0xFF, &ulWrCnt))	break;
		}
		return (ulWrSz == ulWrCnt && ulRdSz == ulRdCnt)?true:false;
	}
	return false;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveProcess (I2C1_Type *pI2C, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz)
{
	bool bFlag;
	osMutexId *pMutex;
	
	pMutex = (I2C1 == pI2C)?&I2C_Mutex1:&I2C_Mutex2;
	osMutexWait(*pMutex, osWaitForever);
	bFlag = bI2C_SlaveProcessInternal (pI2C, pWrBuf, ulWrSz, pRdBuf, ulRdSz);
	osMutexRelease(*pMutex);
	return bFlag;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveWrite (I2C1_Type *pI2C, uint8_t *pBuf, uint32_t ulSz)
{
	return bI2C_SlaveProcess (pI2C, pBuf, ulSz, NULL, 0);
}
//------------------------------------------------------------------------------
bool bI2C_SlaveRead (I2C1_Type *pI2C, uint8_t *pBuf, uint32_t ulSz)
{
	return bI2C_SlaveProcess (pI2C, NULL, 0, pBuf, ulSz);
}
//------------------------------------------------------------------------------
void I2C_Slave1Isr (void)
{

	uint16_t uwFlag;

	uwFlag = I2C1->REG_0x0004;
	INTC_IrqClear (INTC_I2C1_IRQ);
	if (I2C_START_F & uwFlag && I2C1->I2C_START_INTR_EN)
	{
		I2C1->I2C_SAM_INTR_EN = 1;	
//		GPIO->GPIO_O1 = ~GPIO->GPIO_O1;
	}
	if (I2C_GC_F & uwFlag)
	{
		I2C1->I2C_STOP_INTR_EN = 1;
		if (I2C_RW_F & uwFlag)
			I2C1->I2C_DR = 0xFF;
		I2C1->I2C_TB_EN = 1;			
	}
	else if (I2C_SAM_F & uwFlag)
	{
		I2C1->I2C_DT_INTR_EN = 1;
		I2C1->I2C_DR_INTR_EN = 1;
		I2C1->I2C_BERR_INTR_EN = 0;
		I2C1->I2C_STOP_INTR_EN = 1;
		if (I2C_RW_F & uwFlag)
		{
			I2C1->I2C_DR = pubI2C_BufIsr1[ulI2C_CountIsr1];
			I2C1->I2C_TB_EN = 1;
			tI2C_RWIsr1 = I2C_SLAVE_WR;
		}
		else
		{
			I2C1->I2C_NACK = 0;
			I2C1->I2C_TB_EN = 1;			
			tI2C_RWIsr1 = I2C_SLAVE_RD;
		}
	}
	else if (I2C_RX_DONE & uwFlag)
	{
		if (!ulI2C_CountIsr1 && I2C_RW_F & uwFlag)
		{
			uint8_t ubTemp;
			
			ubTemp = I2C1->I2C_DR;
			if ((I2C_SLAVE_ADDR10_HMASK | (I2C_SLAVE_ADDR10 >> 7) | 1) != ubTemp)
			{
				pubI2C_BufIsr1[ulI2C_CountIsr1] = ubTemp;
				++ulI2C_CountIsr1;
			}
		}
		else
		{
			pubI2C_BufIsr1[ulI2C_CountIsr1] = I2C1->I2C_DR;
			++ulI2C_CountIsr1;
			I2C1->I2C_NACK = 0;
			I2C1->I2C_TB_EN = 1;
		}
	}
	else if (I2C_TX_DONE & uwFlag)
	{
		++ulI2C_CountIsr1;
		if (I2C_STOP_F & uwFlag)
			ubI2C_IsrFlag1 = 1;
		else if (!(I2C_NACK_F & uwFlag))
		{
			I2C1->I2C_DR = pubI2C_BufIsr1[ulI2C_CountIsr1];
			I2C1->I2C_TB_EN = 1;
		}
	}
	else if ((I2C_STOP_F & uwFlag) && !((I2C_START_F & uwFlag) && !(I2C_GC_F & uwFlag) && !(I2C_SAM_F & uwFlag)))
		ubI2C_IsrFlag1 = 1;
	GPIO->GPIO_O0 = ~GPIO->GPIO_O0;
}
//------------------------------------------------------------------------------
void I2C_Slave2Isr (void)
{

	uint16_t uwFlag;

	uwFlag = I2C2->REG_0x0004;
	INTC_IrqClear (INTC_I2C2_IRQ);
	if (I2C_START_F & uwFlag && I2C2->I2C_START_INTR_EN)
	{
		I2C2->I2C_SAM_INTR_EN = 1;	
//		GPIO->GPIO_O1 = ~GPIO->GPIO_O1;
	}
	if (I2C_GC_F & uwFlag)
	{
		I2C2->I2C_STOP_INTR_EN = 1;
		if (I2C_RW_F & uwFlag)
			I2C2->I2C_DR = 0xFF;
		I2C2->I2C_TB_EN = 1;			
	}
	else if (I2C_SAM_F & uwFlag)
	{
		I2C2->I2C_DT_INTR_EN = 1;
		I2C2->I2C_DR_INTR_EN = 1;
		I2C2->I2C_BERR_INTR_EN = 0;
		I2C2->I2C_STOP_INTR_EN = 1;
		if (I2C_RW_F & uwFlag)
		{
			I2C2->I2C_DR = pubI2C_BufIsr2[ulI2C_CountIsr2];
			I2C2->I2C_TB_EN = 1;
			tI2C_RWIsr2 = I2C_SLAVE_WR;
		}
		else
		{
			I2C2->I2C_NACK = 0;
			I2C2->I2C_TB_EN = 1;			
			tI2C_RWIsr2 = I2C_SLAVE_RD;
		}
	}
	else if (I2C_RX_DONE & uwFlag)
	{
		if (!ulI2C_CountIsr2 && I2C_RW_F & uwFlag)
		{
			uint8_t ubTemp;
			
			ubTemp = I2C2->I2C_DR;
			if ((I2C_SLAVE_ADDR10_HMASK | (I2C_SLAVE_ADDR10 >> 7) | 1) != ubTemp)
			{
				pubI2C_BufIsr2[ulI2C_CountIsr2] = ubTemp;
				++ulI2C_CountIsr2;
			}
		}
		else
		{
			pubI2C_BufIsr2[ulI2C_CountIsr2] = I2C2->I2C_DR;
			++ulI2C_CountIsr2;
			I2C2->I2C_NACK = 0;
			I2C2->I2C_TB_EN = 1;
		}
	}
	else if (I2C_TX_DONE & uwFlag)
	{
		++ulI2C_CountIsr2;
		if (I2C_STOP_F & uwFlag)
			ubI2C_IsrFlag2 = 1;
		else if (!(I2C_NACK_F & uwFlag))
		{
			I2C2->I2C_DR = pubI2C_BufIsr2[ulI2C_CountIsr2];
			I2C2->I2C_TB_EN = 1;
		}
	}
	else if ((I2C_STOP_F & uwFlag) && !((I2C_START_F & uwFlag) && !(I2C_GC_F & uwFlag) && !(I2C_SAM_F & uwFlag)))
		ubI2C_IsrFlag2 = 1;
	GPIO->GPIO_O0 = ~GPIO->GPIO_O0;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveIntInternal (I2C1_Type *pI2C, uint8_t ubMode, I2C_SLAVE_RW_TYP tRw, uint8_t *pBuf, uint32_t ulSz)
{
	uint32_t ulTo = I2C_TIME_OUT;

	if (I2C_START_INT == ubMode)
		pI2C->I2C_START_INTR_EN = 1;
	else
		pI2C->I2C_SAM_INTR_EN = 1;	
	if (I2C1 == pI2C)
	{
		pubI2C_BufIsr1 = pBuf;	
		ulI2C_CountIsr1 = 0;		
		INTC_IrqSetup (INTC_I2C1_IRQ, INTC_LEVEL_TRIG, I2C_Slave1Isr);
		INTC_IrqClear (INTC_I2C1_IRQ);
		INTC_IrqEnable (INTC_I2C1_IRQ);
		ubI2C_IsrFlag1 = 0;
		while (!ubI2C_IsrFlag1 && --ulTo);
		
		INTC_IrqDisable (INTC_I2C1_IRQ);
		pI2C->I2C_START_INTR_EN = 0;
		pI2C->I2C_SAM_INTR_EN = 0;
		pI2C->I2C_DT_INTR_EN = 0;
		pI2C->I2C_DR_INTR_EN = 0;
		pI2C->I2C_BERR_INTR_EN = 0;
		pI2C->I2C_STOP_INTR_EN = 0;
		INTC_IrqClear (INTC_I2C1_IRQ);
		return (tRw == tI2C_RWIsr1 && ulI2C_CountIsr1 == ulSz)?true:false;
	}
	else
	{
		pubI2C_BufIsr2 = pBuf;	
		ulI2C_CountIsr2 = 0;
		INTC_IrqSetup (INTC_I2C2_IRQ, INTC_LEVEL_TRIG, I2C_Slave2Isr);
		INTC_IrqClear (INTC_I2C2_IRQ);
		INTC_IrqEnable (INTC_I2C2_IRQ);
		ubI2C_IsrFlag2 = 0;
		while (!ubI2C_IsrFlag2 && --ulTo);
		
		INTC_IrqDisable (INTC_I2C2_IRQ);
		pI2C->I2C_START_INTR_EN = 0;
		pI2C->I2C_SAM_INTR_EN = 0;
		pI2C->I2C_DT_INTR_EN = 0;
		pI2C->I2C_DR_INTR_EN = 0;
		pI2C->I2C_BERR_INTR_EN = 0;
		pI2C->I2C_STOP_INTR_EN = 0;
		INTC_IrqClear (INTC_I2C2_IRQ);
		return (tRw == tI2C_RWIsr2 && ulI2C_CountIsr2 == ulSz)?true:false;
	}
}
//------------------------------------------------------------------------------
bool bI2C_SlaveInt (I2C1_Type *pI2C, uint8_t ubMode, I2C_SLAVE_RW_TYP tRw, uint8_t *pBuf, uint32_t ulSz)
{
	bool bFlag;
	osMutexId *pMutex;
	
	pMutex = (I2C1 == pI2C)?&I2C_Mutex1:&I2C_Mutex2;
	osMutexWait(*pMutex, osWaitForever);
	bFlag = bI2C_SlaveIntInternal (pI2C, ubMode, tRw, pBuf, ulSz);
	osMutexRelease(*pMutex);
	return bFlag;
}
//------------------------------------------------------------------------------
bool bI2C_SlaveIntWrite (I2C1_Type *pI2C, uint8_t ubMode, uint8_t *pBuf, uint32_t ulSz)
{
	return bI2C_SlaveInt (pI2C, ubMode, I2C_SLAVE_WR, pBuf, ulSz);
}
//------------------------------------------------------------------------------
bool bI2C_SlaveIntRead (I2C1_Type *pI2C, uint8_t ubMode, uint8_t *pBuf, uint32_t ulSz)
{
	return bI2C_SlaveInt (pI2C, ubMode, I2C_SLAVE_RD, pBuf, ulSz);
}
