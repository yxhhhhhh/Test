/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AWB_API.h
	\brief		Auto white balance API header
	\author			
	\version	1
	\date		2018-01-19
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#ifndef _AWB_API_H_
#define _AWB_API_H_

#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
#define AWB_EVENT_SUSPEND			(0)
#define AWB_EVENT_RESUME			(1)
#define AWB_EVENT_VSYNC				(2)
#define AWB_EVENT_START				(3)
#define AWB_EVENT_STOP				(4)
//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define AWB_Suspend()				{ AWB_Statemachine(AWB_EVENT_SUSPEND); }
#define AWB_Resume()				{ AWB_Statemachine(AWB_EVENT_RESUME); }
#define AWB_VSync()					{ AWB_Statemachine(AWB_EVENT_VSYNC); }
#define AWB_Start()					{ AWB_Statemachine(AWB_EVENT_START); }
#define AWB_Stop()					{ AWB_Statemachine(AWB_EVENT_STOP); }

//------------------------------------------------------------------------
/*!
\brief Set AWB default state.
\return(no)
*/
void AWB_Init(void);
//------------------------------------------------------------------------
/*!
\brief AWB operating process.
\param ubEvent 	AWB current state.
\return(no)
*/
void AWB_Statemachine(uint8_t ubEvent);
//------------------------------------------------------------------------
/*!
\brief AWB Frame End ISR Handler.
\return(no)
*/
void AWB_FrmEndIsr_Handler(void);
//------------------------------------------------------------------------
/*!
\brief 	Get AWB library version	
\return	AWB lib version.
\par [Example]
\code		 
	 uwAWB_GetVersion();
\endcode
*/
uint16_t uwAWB_GetVersion (void);
//------------------------------------------------------------------------
/*!
\brief Set AWB ctrl table value.
\return(no)
*/
void AWB_SetCtrlTable(void);
#endif
