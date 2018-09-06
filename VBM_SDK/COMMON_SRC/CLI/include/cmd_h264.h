#ifndef _CMD_H264_H
#define _CMD_H264_H

#include "_510PF.h"

int32_t cmd_h264_status(int argc, char* argv[]);
int32_t cmd_h264_init(int argc, char* argv[]);

#define CMD_TBL_H264 CMD_TBL_ENTRY(		\
	"+h264",		5,      NULL,			\
	"+h264		- H264 command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_h264_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_H264_INIT	CMD_TBL_ENTRY(		\
	"h264_init",		9,      cmd_h264_init,	\
	"h264_init		- H264 Initial",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_H264_STATUS	CMD_TBL_ENTRY(		\
	"h264_status",		11,      cmd_h264_status,	\
	"h264_status		- Show H264 Channel Status",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),



#endif
