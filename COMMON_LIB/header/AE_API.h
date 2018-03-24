/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AE_API.h
	\brief		Auto exposure API header
	\author			
	\version	1
	\date		2018-01-19
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _AE_API_H_
#define _AE_API_H_

#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
enum
{
	AE_EVENT_SUSPEND,
	AE_EVENT_RESUME,
	AE_EVENT_VSYNC,
	AE_EVENT_START,
	AE_EVENT_STOP
};
//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define AE_Suspend()				{ AE_Statemachine(AE_EVENT_SUSPEND); }
#define AE_Resume()					{ AE_Statemachine(AE_EVENT_RESUME); }
#define AE_VSync()					{ AE_Statemachine(AE_EVENT_VSYNC); }
#define AE_Start()					{ AE_Statemachine(AE_EVENT_START); }
#define AE_Stop()					{ AE_Statemachine(AE_EVENT_STOP); }
//------------------------------------------------------------------------
/*!
\brief Set AE default state.
\return(no)
*/
void AE_Init(void);
//------------------------------------------------------------------------
/*!
\brief AE operating process.
\param ubEvent 	AE current state.
\return(no)
*/
void AE_Statemachine(uint8_t ubEvent);
//------------------------------------------------------------------------
/*!
\brief AE Frame End ISR Handler.
\return(no)
*/
void AE_FrmEndIsr_Handler(void);
//------------------------------------------------------------------------
/*!
\brief 	Get AE library version	
\return	AE lib version.
\par [Example]
\code		 
	 uwAE_GetVersion();
\endcode
*/
uint16_t uwAE_GetVersion (void);
//------------------------------------------------------------------------
/*!
\brief 	Set AE PID	
\param ubLowFrame 	low frame rate set to 1.
\return	(no)
\par [Example]
\code		 
	 AE_SetPID(1);
\endcode
*/
void AE_SetPID(uint8_t ubLowFrame);
//------------------------------------------------------------------------
/*!
\brief Set AE ctrl table value.
\return(no)
*/
void AE_SetCtrlTable(void);
#endif
