/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CQI2C.h
	\brief		I2C controlled by Command Queue
	\author		Nick Huang
	\version	0.2
	\date		2017/05/22
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CQI2C_H_
#define _CQI2C_H_

#include "_510PF.h"
//------------------------------------------------------------------------------
#define CQI2C_MAJOR_VER    0                       // Major version = 0
#define CQI2C_MINOR_VER    2                       // Minor version = 2

#define CQI2C_CQ_POLLING_PERIOD			50										// 2us @25MHz
#define CQI2C_DLY_1US					CQ_CMD_WAIT(5, 5)						// 1us @25MHz
#define CQI2C_DLY_10US					CQ_CMD_WAIT(25, 10)						// 10us @25MHz
#define CQI2C_DLY_20US					CQ_CMD_WAIT(50, 10)						// 20us @25MHz
enum
{
	CQI2C_DO_ACK,
	CQI2C_DO_NACK
};

enum
{
	CQI2C_DATA_NEXT,
	CQI2C_DATA_STOP
};

#define CQI2C_MASK(x)		(1L << x)

#define CQI2C_CR			0x92400000
#define CQI2C_SCL_EN		2						// bit 2 of CQI2C_CR
#define CQI2C_START			4						// bit 4 of CQI2C_CR
#define CQI2C_STOP			5						// bit 5 of CQI2C_CR
#define CQI2C_NACK			6						// bit 6 of CQI2C_CR
#define CQI2C_TB_EN			7						// bit 7 of CQI2C_CR

#define CQI2C_SR			0x92400004
#define CQI2C_BB			3						// bit 3 of CQI2C_SR
#define CQI2C_DT_FLAG		4						// bit 4 of CQI2C_SR
#define CQI2C_DR_FLAG		5						// bit 5 of CQI2C_SR
#define CQI2C_STOP_FLAG		7						// bit 7 of CQI2C_SR

#define CQI2C_DR			0x9240000C

typedef struct
{
	uint8_t		ubCmdNum;
	uint32_t	ulCmdAddr[8];
} CQI2C_WrDataSet_t;
//------------------------------------------------------------------------------
void CQI2C_Init(void);
CQI2C_WrDataSet_t* CQI2C_CreateCQ_I2C_Write(uint8_t ubSlvAddr, uint8_t* pWrBuf, uint32_t ulWrSz);
void CQI2C_ModifyCQ_I2C_Write(CQI2C_WrDataSet_t* pDataSet, uint8_t* pbData);
void CQI2C_Complete(void);
void CQI2C_Start(void);
void CQI2C_MasterWrAddr(uint8_t ubAddr);
uint32_t ulCQI2C_MasterWrData(uint8_t ubData, uint8_t ubMode);
void CQI2C_MasterWaitReady(void);
void CQI2C_MasterStopDone(void);
void CQI2C_MasterTxDone(void);
void CQI2C_CqEndIsr(void);
uint16_t uwCQI2C_GetVersion(void);
//------------------------------------------------------------------------------
#endif
