/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_ado.h
	\brief		Audio control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/30
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_ADO_H_
#define _CMD_ADO_H_

#include <stdint.h>

int32_t cmd_ado_wav(int argc, char* argv[]);
int32_t cmd_ado_buzzer(int argc, char* argv[]);

#define CMD_TBL_ADO    CMD_TBL_ENTRY(          \
	"ado",		3,	NULL,       \
	"ado		- Enter Audio control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_ado_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_ADO_WAV    CMD_TBL_ENTRY(          \
	"wav",		3,	cmd_ado_wav,       \
	"wav		- Wav control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_ADO_BUZZER    CMD_TBL_ENTRY(          \
	"buzzer",	6,	cmd_ado_buzzer,       \
	"buzzer		- Buzzer control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
