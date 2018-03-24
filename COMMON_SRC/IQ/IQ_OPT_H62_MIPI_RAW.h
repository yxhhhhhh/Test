/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IQ_OPT_H62_RAW.c
	\brief		H62 image quality function header
	\author		Tomas
	\version	0.1
	\date		2017-03-10
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _IQ_OPT_H_
#define _IQ_OPT_H_
//==============================================================================
// FILE INCLUSION
//==============================================================================
#include "_510PF.h"

//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ lens shading compensattion(LSC).
\return(no)
*/
void IQ_DynamicLSC_4G5(void);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ CCM.
\return(no)
*/
void IQ_DynamicCCM_4G5(void);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ denoise.
\return(no)
*/
void IQ_DynamicDenoise_4G5(void); 

#endif
