#ifndef _RTOSCLI_H_
#define _RTOSCLI_H_

#include "_510PF.h"

#define CMD_TBL_ENTRY(name, len_name, cmd, usage, uid_lv, next_lv, prev_lv)  {name, len_name, cmd, usage, uid_lv, next_lv, prev_lv}
#define CMDBUF_SIZE	128
#define PROMPT	"\nSONIX (%s)>:\n"
#define LOGIN_PROMPT	"\nLogin :\n"
#define PASSWD_PROMPT	"Password :\n"

//#undef CFG_ENABLE_LOGIN
#define CFG_ENABLE_LOGIN
#define CFG_DEFAULT_CMD_LEVEL		9999

//#define DEBUG_LV Kernel
#define K_Print(fmt,arg...) printf(fmt,##arg);
#define E_Print(fmt,arg...) if(Error >= DEBUG) printf(fmt,##arg);
#define W_Print(fmt,arg...) if(Warning >= DEBUG) printf(fmt,##arg);
#define I_Print(fmt,arg...) if(Info >= DEBUG) printf(fmt,##arg);

#define cliPASS  0
#define cliFAIL  1

struct cmd_table {
	char	*name;
	int		len_name;
	int 	(*Func)(int argc, char* argv[]);
	char 	*usage;
	int		cmd_lv;
	struct	cmd_table 	*next_lv;
	struct	cmd_table 	*prev_lv;
};

int32_t CLI_cmd_quit(void);
int32_t CLI_cmd_help(int argc, char* argv[]);
int32_t CLI_cmd_back(int argc, char* argv[]);
int32_t CLI_cmd_logout(int argc, char* argv[]);
int32_t CLI_cmd_showDevId(int argc, char* argv[]);
int32_t CLI_cmd_setloglv(int argc, char* argv[]);
/* Common command */
#define CMD_TBL_LOGOUT    CMD_TBL_ENTRY(          \
	"logout",		6,	CLI_cmd_logout,       \
	"logout		- Logout the system",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_QUIT    CMD_TBL_ENTRY(          \
	"quit",		4,	CLI_cmd_quit,       \
	"quit		- Quit the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_HELP	CMD_TBL_ENTRY(		\
	"help",		4,	CLI_cmd_help,				\
	"help		- Show usage message",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_BACK    CMD_TBL_ENTRY(          \
	"back",		4,	CLI_cmd_back,       \
	"back            - Back to prev level",  CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL                    \
),

#define CMD_TBL_MAIN    CMD_TBL_ENTRY(          \
	"main",		4,	NULL,       \
	"main		- Main the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SHOW_DEV_ID    CMD_TBL_ENTRY(          \
	"showid",		6,	CLI_cmd_showDevId,       \
	"showid		- Show Device ID",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SET_DEBUG_LV    CMD_TBL_ENTRY(          \
	"setlvl",		6,	CLI_cmd_setloglv,       \
	"setlvl		- Set System Debug Level(0:Off 1:Critical 2:Error 3:Info 4~6:Debug1-Debug3)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

void CLI_init_rtos_cli(void);
int32_t CLI_parse_cmd(char *cmd, int isTAB);
void CLI_show_cli_prompt(void);
void CLI_show_login_prompt(void);
void CLI_rtoscli_recv(char ch);
int32_t CLI_ispasswd(void);
int32_t CLI_islogin(void);
void CLI_recvInit(void);
void CLI_Init(void);

#endif
