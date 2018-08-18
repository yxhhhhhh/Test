/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		system__510PF.h
	\brief		ARM926EJ-S Device Peripheral Access Layer Header File for 510PF
	\author		Nick Huang
	\version	1.1
	\date		2017/10/11
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SYSTEM__510PF_H_
#define _SYSTEM__510PF_H_
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
#include "Retarget.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines

    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

#ifndef TRUE
	#define TRUE	1
#endif

#ifndef FALSE
	#define FALSE	0
#endif

#define SET_BIT(val, num)	val |= (((uint32_t)1) << num)
#define CLR_BIT(val, num)	val &= ~(((uint32_t)1) << num)
#define MAX(a,b)			((a > b)?a:b)
#define MIN(a,b)			((a < b)?a:b)

typedef enum
{
	CPU_CLK_FS    = 2,
	CPU_CLK_DIV1  = 3,
	CPU_CLK_DIV2  = 4,
	CPU_CLK_DIV4  = 6,
	CPU_CLK_DIV6  = 8,
	CPU_CLK_DIV8  = 10,
	CPU_CLK_DIV10 = 12,
	CPU_CLK_DIV12 = 14,
} SYS_CoreClkSel;

typedef enum
{
	SYS_PS0,
	SYS_PS1,
} SYS_PowerState_t;

typedef enum
{
	SYS_STATE_IMG_ADDR = 0,
	SYS_HEADER_OFFSET  = 1
} SYS_DATA_BUF_ADDR_t;

typedef enum
{
	SYS_PwrOff,						//!< Previous system state is Power-off, same as NormalHs
	SYS_NormalHs,					//!< reset to boot-loader (run in High Speed Mode)
	SYS_NormalLp,					//!< reset to boot-loader (run in Low Power Mode)
	SYS_BypassBt,					//!< Bypass Boot-loader
	SYS_Error						//!< Error, Boot-loader will lead system into ISP mode
} SYS_STATE;

typedef enum
{
	DBG_OFF,
    DBG_CriticalLvl,
    DBG_ErrorLvl,
    DBG_InfoLvl,
    DBG_Debug1Lvl,
	DBG_Debug2Lvl,
	DBG_Debug3Lvl,
} SYS_PrintLevel_t;

#define Current_Test	1
#define Current_Mode	PS_VOX_MODE //A:PS_VOX_MODE / B: PS_WOR_MODE

#define Apk_DebugLvl	5 // 5 1
#define printd(level, ...)    ((level <= lSYS_DebugLvl) ? printf(__VA_ARGS__) : 0 )

#define SWAP16(x) \
    ((uint16_t) (\
	       (((uint16_t) (x) & (uint16_t) 0x00ffU) << 8) | \
	       (((uint16_t) (x) & (uint16_t) 0xff00U) >> 8))) 
 
#define SWAP32(x) \
    ((uint32_t) (\
	       (((uint32_t) (x) & (uint32_t) 0x000000ffUL) << 24) | \
	       (((uint32_t) (x) & (uint32_t) 0x0000ff00UL) << 8) | \
	       (((uint32_t) (x) & (uint32_t) 0x00ff0000UL) >> 8) | \
	       (((uint32_t) (x) & (uint32_t) 0xff000000UL) >> 24))) 

//------------------------------------------------------------------------------
void SystemStartup(void);
void SystemInit(void);
void SystemCoreClockUpdate(void);
void SYS_SetCoreClock(SYS_CoreClkSel CoreClkSel);
void SYS_SetPowerStates(SYS_PowerState_t tPowerState);
void SYS_SetState(SYS_STATE state);
SYS_STATE SYS_GetState(void);
void SYS_SetPrintLevel(SYS_PrintLevel_t tLevel);
void APP_Init(void);
uint16_t uwSYS_GetOsVersion(void);
//------------------------------------------------------------------------------
extern uint32_t SystemCoreClock;												//!< Variable to hold the system core clock value
extern int32_t lSYS_DebugLvl;                                                   //!< System Debug Level
#endif
