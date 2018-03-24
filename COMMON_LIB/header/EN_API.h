/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		EN_API.h
	\brief		Engineer Mode API header
	\author		Bing
	\version	0.3
	\date		2016/09/13
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef __EN_API_H
#define __EN_API_H

#include "_510PF.h"
#include "OSD.h"

void EN_Init(void);
void EN_UpKey(void);
void EN_DownKey(void);
void EN_EnterKey(void);
void EN_Start(uint32_t ulStackSize, osPriority priority);
void EN_SetupOsdImgInfo(OSD_IMGIDXARRARY_t *pEN_OsdImgIdx);
void EN_OpenEnMode(uint8_t ubEnMode);

#endif
