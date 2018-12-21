/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AF_API.h
	\brief		Auto focus API header
	\author			
	\version	1
	\date		2017-03-15
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _AF_API_H_
#define _AF_API_H_

#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
enum
{
	AF_EVENT_SUSPEND,
	AF_EVENT_RESUME,
	AF_EVENT_VSYNC,
	AF_EVENT_START,
	AF_EVENT_STOP,
};
//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define AF_Suspend()				{ AF_Statemachine(AF_EVENT_SUSPEND); }
#define AF_Resume()					{ AF_Statemachine(AF_EVENT_RESUME); }
#define AF_VSync()					{ AF_Statemachine(AF_EVENT_VSYNC); }
#define AF_Start()					{ AF_Statemachine(AF_EVENT_START); }
#define AF_Stop()					{ AF_Statemachine(AF_EVENT_STOP); }
//------------------------------------------------------------------------
/*!
\brief Set AF default state.
\return(no)
*/
void AF_Init(void);
//------------------------------------------------------------------------
/*!
\brief AF operating process.
\param ubEvent 	AF current state.
\return(no)
*/
void AF_Statemachine(uint8_t ubEvent);
//------------------------------------------------------------------------
/*!
\brief AF Frame End ISR Handler.
\return(no)
*/
void AF_FrmEndIsr_Handler(void);
#endif
