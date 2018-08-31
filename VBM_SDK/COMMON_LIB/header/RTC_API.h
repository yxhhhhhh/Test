/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		RTC_API.h
	\brief		Real time clock control header file
	\author		Nick Huang
	\version	1.3
	\date		2018/01/25
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _RTC_API_H_
#define _RTC_API_H_
//------------------------------------------------------------------------------
#include "_510PF.h"

#define RTC_DATA_MAX_SIZE 	(6)
#define RTC_OK				0
#define RTC_ERR				-1

typedef struct
{
	uint16_t uwYear;
	uint8_t  ubMonth;
	uint8_t  ubDate;
	uint8_t  ubHour;
	uint8_t  ubMin;
	uint8_t  ubSec;
} RTC_Calendar_t;

//! RTC GPIO pull-down type
typedef enum
{
	RTC_PullDownEnable=0,
	RTC_PullDownDisable
} RTC_PullDown_t;

typedef enum
{
	RTC_TimerDisable=0,
	RTC_TimerEnable
} RTC_Timer_t;

uint16_t uwRTC_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief	RTC initialize
\param	tOption		1:Enable RTC Timer; 
					0:Disable RTC Timer
\return	(no)
*/
void RTC_Init(RTC_Timer_t tOption);
//------------------------------------------------------------------------------
/*!
\brief	Read RTC Key value
\return	0 or 1
*/
uint8_t ubRTC_GetKey(void);
void RTC_SetKeyIntHandler(void (*pfHandler)(void));
void RTC_KeyIntEnable(void);
void RTC_KeyIntDisable(void);
void RTC_KeyIntFlagClear(void);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC control pin(RTC_CTL) to high
\return	(no)
\note	RTC_CTL pin always used as system power control pin due to its wakeupable.
*/
void RTC_PowerEnable(void);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC control pin(RTC_CTL) to low
\return	(no)
\note	RTC_CTL pin always used as system power control pin due to its wakeupable.
*/
void RTC_PowerDisable(void);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC_GPIO0 pin output high or low
\param	ubValue		output value, 0 or 1
\param	tPullDown	internal pull down setting, can be RTC_PullDownEnable or\n
					RTC_PullDownDisable
\return	(no)
*/
void RTC_SetGPO_0(uint8_t ubValue, RTC_PullDown_t tPullDown);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC_GPIO0 pin in input mode and get the value
\param	tPullDown	internal pull down setting, can be RTC_PullDownEnable or\n
					RTC_PullDownDisable
\return	RTC_GPIO0 input value
*/
uint8_t ubRTC_GetGPI_0(RTC_PullDown_t tPullDown);
void RTC_WakeupByGPI0Enable(void);
void RTC_WakeupByGPI0Disable(void);
void RTC_SetGPI0IntHandler(void (*pfHandler)(void));
void RTC_GPI0IntEnable(void);
void RTC_GPI0IntDisable(void);
void RTC_GPI0IntFlagClear(void);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC_GPIO1 pin output high or low
\param	ubValue		output value, 0 or 1
\param	tPullDown	internal pull down setting, can be RTC_PullDownEnable or\n
					RTC_PullDownDisable
\return	(no)
*/
void RTC_SetGPO_1(uint8_t ubValue, RTC_PullDown_t tPullDown);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC_GPIO1 pin in input mode and get the value
\param	tPullDown	internal pull down setting, can be RTC_PullDownEnable or\n
					RTC_PullDownDisable
\return	RTC_GPIO1 input value
*/
uint8_t ubRTC_GetGPI_1(RTC_PullDown_t tPullDown);
void RTC_WakeupByGPI1Enable(void);
void RTC_WakeupByGPI1Disable(void);
void RTC_SetGPI1IntHandler(void (*pfHandler)(void));
void RTC_GPI1IntEnable(void);
void RTC_GPI1IntDisable(void);
void RTC_GPI1IntFlagClear(void);
uint32_t ulRTC_RdRtcTime(void);
void RTC_WrRtcTime(uint32_t ulSec);
//------------------------------------------------------------------------------
/*!
\brief	Set base calendar
\param	Calendar	A pointer point to calendar data strcture
\return RTC_OK or RTC_ERR
*/
int iRTC_SetBaseCalendar(const RTC_Calendar_t* Calendar);
//------------------------------------------------------------------------------
/*!
\brief	Get base calendar
\param	Calendar	A pointer point to calendar data strcture
\return	(no)
*/
void RTC_GetBaseCalendar(RTC_Calendar_t* Calendar);
//------------------------------------------------------------------------------
/*!
\brief	Get RTC Wakeup time
\return	wakeup time (1/128 sec)
*/
uint32_t ulRTC_GetWakeupTime(void);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC Wakeup time
\param	ulTime		wakeup time (1/128 sec), over 24bits return RTC_ERR
\return	RTC_OK or RTC_ERR
*/
int iRTC_SetWakeupTime(uint32_t ulTime);
//------------------------------------------------------------------------------
/*!
\brief	Enable RTC Wakeup function 
\param
\return	(no)
*/
void RTC_EnableWakeup(void);
//------------------------------------------------------------------------------
/*!
\brief	Disable RTC Wakeup function
\return	(no)
*/
void RTC_DisableWakeup(void);
void RTC_SetWakeupIntHandler(void (*pfHandler)(void));
void RTC_WakeupIntEnable(void);
void RTC_WakeupIntDisable(void);
void RTC_WakeupIntFlagClear(void);
//------------------------------------------------------------------------------
/*!
\brief	Get RTC Alarm Calendar
\param	Calendar	A pointer point to calendar data strcture
\return	(no)
*/
void RTC_GetAlarmCalendar(RTC_Calendar_t* Calendar);
//------------------------------------------------------------------------------
/*!
\brief	Set RTC Alarm Calendar
\param	Calendar	A pointer point to calendar data strcture
\return	(no)
*/
void RTC_SetAlarmCalendar(const RTC_Calendar_t* Calendar);
//------------------------------------------------------------------------------
/*!
\brief	Get current calendar
\param	Calendar	A pointer point to calendar data strcture
\return	(no)
*/
void RTC_GetCalendar(RTC_Calendar_t* Calendar);
//------------------------------------------------------------------------------
/*!
\brief	Set current calendar
\param	Calendar	A pointer point to calendar data strcture
\return	(no)
*/
void RTC_SetCalendar(const RTC_Calendar_t* Calendar);
//------------------------------------------------------------------------------
/*!
\brief	Enable RTC Alarm function
\param Alarm Calendar porint, (6 byte unsigned char [s; min; hr; day; mon; year])
\par Example:
		unsigned char calendar[6] = { 0, 0, 8, 26, 6, 15}; ==>(Year: 2015 - 2000 = 15)
\return	(no)
*/
void RTC_EnableAlarm(void);
//------------------------------------------------------------------------------
/*!
\brief	Disable RTC Alarm function
\return	(no)
*/
void RTC_DisableAlarm(void);
//------------------------------------------------------------------------------
/*!
\brief	Read data from RTC Systerm ram
\return	value
*/
int16_t wRTC_ReadSysRam(uint8_t addr);
//------------------------------------------------------------------------------
/*!
\brief	Write data to RTC Systerm ram
\param
\return(no)
*/
void RTC_WriteSysRam(uint8_t addr, uint8_t data);
//------------------------------------------------------------------------------
/*!
\brief	Read data from RTC User ram
\param	addr		address, if address >= RTC_DATA_MAX_SIZE return RTC_ERR
\return	data
*/
int16_t wRTC_ReadUserRam(uint8_t addr);
//------------------------------------------------------------------------------
/*!
\brief Write data to RTC User ram
\param addr		address
\param data		data
\return(no)
*/
void RTC_WriteUserRam(uint8_t addr, uint8_t data);
//------------------------------------------------------------------------------

#endif
