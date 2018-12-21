/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ADO.h
	\brief		Audio Process Header file for VBM
	\author		Hanyi Chiu
	\version	0.4
	\date		2017/12/25
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _ADO_H_
#define _ADO_H_

#include "APP_CFG.h"
#include "KNL.h"

typedef enum
{
	ADO_MAIN_SRC,
	ADO_PTT_SRC,
	ADO_SRC_MAX,
}ADO_SrcType_t;

typedef struct
{
	void (*ADO_tPttFunPtr)(void);
}ADO_PttFuncPtr_t;

typedef struct
{
	KNL_SRC		tKNL_SrcNum;
	KNL_SRC		tKNL_SubSrcNum;
}ADO_KNLRoleInfo_t;

void ADO_Init(void);
void ADO_DataPathSetup(KNL_ROLE tADO_KNLRole, ADO_SrcType_t tADO_SrcType);
#ifdef VBM_BU
void ADO_KNLSysInfoSetup(KNL_ROLE tADO_KNLRole);
#endif
#ifdef VBM_PU
void ADO_RemoveDataPath(KNL_ROLE tADO_Role);
void ADO_PTTStart(void);
void ADO_PTTStop(void);
#endif
KNL_SRC ADO_GetSourceNumber(KNL_VA_DATAPATH tADO_Path, KNL_ROLE tADO_KNLRole);
void ADO_Start(KNL_ROLE tADO_Role);
void ADO_Stop(void);
void ADO_KNLParamSetup(void);

#endif
