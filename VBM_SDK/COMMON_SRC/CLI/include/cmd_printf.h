#ifndef _CMD_I2C_H
#define _CMD_I2C_H

#include "_510PF.h"

int cmd_printf_hello(int argc, char* argv[]);
int cmd_printf_arg(int argc, char* argv[]);

#define CMD_TBL_PRINTF		CMD_TBL_ENTRY(		\
	"+printf",		7,      NULL,			\
	"+printf		- Printf Command Table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_printf_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_PRINTF_HELLO	CMD_TBL_ENTRY(		\
	"printf_hello",		12,      cmd_printf_hello,	\
	"printf_hello		- Printf Hello",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_PRINTF_ARGU	CMD_TBL_ENTRY(		\
	"printf_arg",		10,      cmd_printf_arg,	\
	"printf_arg		- Printf Argument",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
