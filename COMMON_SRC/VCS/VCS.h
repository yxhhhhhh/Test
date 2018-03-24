/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VCS.h
	\brief		Version Control header file
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/08/01
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _VCS_H_
#define _VCS_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
	char *pLibName;
	uint16_t (*pvLibraryVcFunc)(void);	
}VCS_GetVersionFuncPtr_t;

//------------------------------------------------------------------------------
/*!
\brief 	List all library version
\return(no)
*/
void uwVCS_LibraryVersionList(char *LibraryName);
//------------------------------------------------------------------------------
/*!
\brief 	Obtain VCS Version	
\return	Version
*/
uint16_t uwVCS_GetVersion(void);

#endif

