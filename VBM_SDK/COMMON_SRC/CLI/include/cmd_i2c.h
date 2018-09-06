/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		cmd_i2c.h
	\brief		I2C control command line header file
	\author		Ocean
	\version	0.1
	\date		2017/10/12
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CMD_I2C_H_
#define _CMD_I2C_H_

#include <stdint.h>

#define SNX_I2C_R8D8_MODE		0
#define SNX_I2C_R8D16_MODE		1
#define SNX_I2C_R16D8_MODE		2
#define SNX_I2C_R16D16_MODE		3

#define SNX_I2C_OP_WRITE		0
#define SNX_I2C_OP_READ			1


int32_t cmd_i2c_ctrl(int argc, char* argv[]);


#define CMD_TBL_I2C		CMD_TBL_ENTRY(		\
	"i2c",		3,      NULL,			\
	"i2c		- Enter I2C control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_i2c_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_I2C_CTRL	CMD_TBL_ENTRY(		\
	"i2c_ctrl",		8,      cmd_i2c_ctrl,	\
	"i2c_ctrl	- I2C Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
