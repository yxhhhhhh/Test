/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CQ_API.h
	\brief		Command Queue header file
	\author		Nick Huang
	\version	0.3
	\date		2017/07/14
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CQ_API_H_
#define _CQ_API_H_

#include "_510PF.h"
//------------------------------------------------------------------------------

#define CQ_OK						1
#define CQ_FAIL						255

#define CQ_SET_LABEL				0
#define CQ_GOTO_LABEL				1

#define CQ_LABEL(x)				CQ_LabelProcess(#x, CQ_SET_LABEL, 0)
#define CQ_LABEL_LBS(x)			CQ_LabelProcess(x, CQ_SET_LABEL, 0)			// LBS: label by C-string

#define CQ_CMD_ADD(x, ...)		x(CQ_ADD_CMD, 0, 0, __VA_ARGS__)
#define CQ_CMD_SET(x, ...)		x(CQ_SET_CMD, __VA_ARGS__)
#define CQ_CMD_WAIT(m, n)		((((m##L-1))<<10) + (n##L-1))

typedef enum
{
	CQ_Core0=0,
	CQ_Core1,
	CQ_Core2,
	CQ_Core3
} CQ_Core_t;

typedef enum
{
	CQ_NOT_ASSIGN,								// Not assigned to any CmdQ
	CQ_READY,									// Command Queue is ready to start
	CQ_ERROR,									// There is an error occurred
	CQ_CMD_ERROR,								// Command error (undefined command)
	CQ_RUNNING,									// Core is running
	CQ_END,										// Core is ending & stopped
	CQ_STOP										// Core is stopped
} CQ_CoreStatus_t;

typedef enum
{
	CQ_SW=0,									// Single Write
	CQ_MW,										// Multi Write
	CQ_PW,										// Partial Write
	CQ_BS,										// Bits Set
	CQ_BC,										// Bits Clear
	CQ_SP,										// Set Program Counter
	CQ_SI,										// Set Interrupt
	CQ_D,										// Delay
	CQ_PB,										// Polling Bits
	CQ_PBAC,									// Polling Bits and Clear
	CQ_CP,										// Compare
	CQ_PCP										// Partial Compare
} CQ_CMD_t;

typedef enum
{
	CQ_ADD_CMD,
	CQ_SET_CMD
} CQ_CmdQProcType_t;

// CQ_CMD_ADD(CQ_SINGLE_WRITE, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, uint32_t ulReturn);
// CQ_CMD_SET(CQ_SINGLE_WRITE, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, uint32_t ulReturn);
#define CQ_SINGLE_WRITE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, DATA, RET)									\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = DATA;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_SW, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_MULTI_WRITE, uint32_t ulDelay, uint32_t ulDestAddr, uint8_t ubNum, uint32_t* plData, uint32_t ulReturn);
// CQ_CMD_SET(CQ_MULTI_WRITE, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulDestAddr, uint8_t ubNum, uint32_t* plData, uint32_t ulReturn);
#define CQ_MULTI_WRITE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, NUM, DATA, RET)								\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = NUM;																							\
	for(ulCQ_Data[10]=0;ulCQ_Data[10]<NUM;ulCQ_Data[10]++)														\
	{																											\
		ulCQ_Data[ulCQ_Data[10]+2] = DATA[ulCQ_Data[10]];														\
	}																											\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_MW, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_PARTIAL_WRITE, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, uint32_t ulMask, uint32_t ulReturn);
// CQ_CMD_SET(CQ_PARTIAL_WRITE, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, uint32_t ulMask, uint32_t ulReturn);
#define CQ_PARTIAL_WRITE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, DATA, MASK, RET)							\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = MASK;																						\
	ulCQ_Data[2] = DATA;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PW, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_BITS_SET, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulBitSet, uint32_t ulReturn);
// CQ_CMD_SET(CQ_BITS_SET, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulBitSet, uint32_t ulReturn);
#define CQ_BITS_SET(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, SET_BIT, RET)									\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = SET_BIT;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_BS, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_BITS_CLEAR, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulBitClear, uint32_t ulReturn);
// CQ_CMD_SET(CQ_BITS_CLEAR, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulBitClear, uint32_t ulReturn);
#define CQ_BITS_CLEAR(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, CLEAR_BIT, RET)								\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = CLEAR_BIT;																					\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_BC, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_SET_PC, uint32_t ulDelay, label, uint32_t ulReturn);
#define CQ_SET_PC(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, L, RET)													\
{																												\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_SP, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(#L, CQ_GOTO_LABEL, RET+4);																	\
}

// CQ_CMD_ADD(CQ_SET_PC_LBS, uint32_t ulDelay, const char* label, uint32_t ulReturn);
#define CQ_SET_PC_LBS(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, L, RET)												\
{																												\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_SP, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(L, CQ_GOTO_LABEL, RET+4);																	\
}

// CQ_CMD_ADD(CQ_SET_INT, uint32_t ulDelay, uint32_t ulReturn);
// CQ_CMD_SET(CQ_SET_INT, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulReturn);
#define CQ_SET_INT(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, RET)													\
{																												\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_SI, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_DELAY, uint32_t ulDelay, uint32_t ulCycle, uint32_t ulReturn);
// CQ_CMD_SET(CQ_DELAY, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, uint32_t ulCycle, uint32_t ulReturn);
#define CQ_DELAY(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, CYCLE, RET)												\
{																												\
	ulCQ_Data[0] = CYCLE;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_D, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_POLLING_BITS, uint32_t ulDelay, uint32_t ulDestAddr, uint8_t ubBit, TimeoutLabel, uint32_t ulReturn);
#define CQ_POLLING_BITS(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, BIT, L, RET)								\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = (uint32_t)0x01 << BIT;																		\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PB, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(#L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_POLLING_BITS_LBS, uint32_t ulDelay, uint32_t ulDestAddr, uint8_t ubBit, const char* TimeoutLabel, uint32_t ulReturn);
#define CQ_POLLING_BITS_LBS(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, BIT, L, RET)							\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = (uint32_t)0x01 << BIT;																		\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PB, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_POLLING_BITS_AND_CLEAR, uint32_t ulDelay, uint32_t ulDestAddr, uint8_t ubBit, TimeoutLabel, uint32_t ulAddr2Clear, uint32_t ulBitClear, uint32_t ulReturn);
#define CQ_POLLING_BITS_AND_CLEAR(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, BIT, L, ADDR2, CLEAR_BIT, RET)	\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = (uint32_t)0x01 << BIT;																		\
	ulCQ_Data[3] = ADDR2;																						\
	ulCQ_Data[4] = CLEAR_BIT;																					\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PBAC, DELAY, ulCQ_Data);							\
	CQ_LabelProcess(#L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_POLLING_BITS_AND_CLEAR_LBS, uint32_t ulDelay, uint32_t ulDestAddr, uint8_t ubBit, const char* TimeoutLabel, uint32_t ulAddr2Clear, uint32_t ulBitClear, uint32_t ulReturn);
#define CQ_POLLING_BITS_AND_CLEAR_LBS(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, BIT, L, ADDR2, CLEAR_BIT, RET)\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = (uint32_t)0x01 << BIT;																		\
	ulCQ_Data[3] = ADDR2;																						\
	ulCQ_Data[4] = CLEAR_BIT;																					\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PBAC, DELAY, ulCQ_Data);							\
	CQ_LabelProcess(L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_COMPARE, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, NotEqualLabel, uint32_t ulReturn);
#define CQ_COMPARE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, DATA, L, RET)									\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = DATA;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_CP, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(#L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_COMPARE_LBS, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, const char* NotEqualLabel, uint32_t ulReturn);
#define CQ_COMPARE_LBS(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, DATA, L, RET)								\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = DATA;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_CP, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_PARTIAL_COMPARE, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, uint32_t ulMask, NotEqualLabel, uint32_t ulReturn);
#define CQ_PARTIAL_COMPARE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, DATA, MASK, L, RET)						\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = DATA;																						\
	ulCQ_Data[3] = MASK;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PCP, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(#L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_PARTIAL_COMPARE_LBS, uint32_t ulDelay, uint32_t ulDestAddr, uint32_t ulData, uint32_t ulMask, const char* NotEqualLabel, uint32_t ulReturn);
#define CQ_PARTIAL_COMPARE_LBS(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, ADDR, DATA, MASK, L, RET)					\
{																												\
	ulCQ_Data[0] = ADDR;																						\
	ulCQ_Data[1] = DATA;																						\
	ulCQ_Data[3] = MASK;																						\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_PCP, DELAY, ulCQ_Data);								\
	CQ_LabelProcess(#L, CQ_GOTO_LABEL, RET+12);																	\
}

// CQ_CMD_ADD(CQ_START_CORE, uint32_t ulDelay, CQ_Core_t core, uint32_t ulReturn);
// CQ_CMD_SET(CQ_START_CORE, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, CQ_Core_t core, uint32_t ulReturn);
#define CQ_START_CORE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, CORE, RET)											\
{																												\
	ulCQ_Data[0] = 0x9A01000C + CORE * 0x20;																	\
	ulCQ_Data[1] = 0x00000001;																					\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_BS, DELAY, ulCQ_Data);								\
}

// CQ_CMD_ADD(CQ_STOP_CORE, uint32_t ulDelay, CQ_Core_t core, uint32_t ulReturn);
// CQ_CMD_SET(CQ_STOP_CORE, uint8_t ubCmdQIdx, uint32_t ulCmdAddr, uint32_t ulDelay, CQ_Core_t core, uint32_t ulReturn);
#define CQ_STOP_CORE(CMD_TYPE, CMDQ_IDX, CMDQ_ADDR, DELAY, CORE, RET)											\
{																												\
	ulCQ_Data[0] = 0x9A01000C + CORE * 0x20;																	\
	ulCQ_Data[1] = 0x00000001;																					\
	RET = ulCQ_ProcessCmdQ(CMDQ_IDX, CMD_TYPE, CMDQ_ADDR, CQ_BC, DELAY, ulCQ_Data);								\
}
//------------------------------------------------------------------------------
void CQ_Init(void);
uint16_t uwCQ_GetVersion(void);
void CQ_CoreStart(CQ_Core_t core);
void CQ_CoreStop(CQ_Core_t core);
uint32_t ulCQ_Malloc(uint8_t ubLength);
uint8_t ubCQ_GetCmdQNum(void);
CQ_CoreStatus_t tCQ_CoreGetStatus(CQ_Core_t core);
uint8_t ubCQ_CoreGetCmdQIdx(CQ_Core_t core);
uint8_t ubCQ_CreateCmdQ(void (*pfISR)(void), uint16_t uwCQ_PollPeriod);
void CQ_LabelProcess(const char* str, uint8_t ubProcess, uint32_t ulGotoAddr);
uint32_t ulCQ_ProcessCmdQ(uint8_t ubCmdQIdx, CQ_CmdQProcType_t tProcess, uint32_t ulCmdQAddr, CQ_CMD_t tCmd, uint32_t ulTime, uint32_t* plData);
void CQ_CompleteCmdQ(void);
uint8_t ubCQ_CoreAssign(CQ_Core_t core, uint8_t ubCmdQIdx);
uint16_t uwCQ_GetCmdQSz(uint8_t ubCmdQIdx);
//------------------------------------------------------------------------------
extern uint32_t ulCQ_Data[11];
#endif
