/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CLI.c
	\brief		Command line system
	\author		Bruce Hsu
	\version	0.1
	\date		2017/07/25
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CLI.h"
#include "cmd.h"
#include "schema.h"
//------------------------------------------------------------------------------
#define MAX_CMD_LENGTH			48
#define RECV_QUEUE_SIZE			MAX_CMD_LENGTH
#define RECV_QUEUE_STACK_SIZE	1024
//------------------------------------------------------------------------------
#ifdef CFG_ENABLE_LOGIN
static int32_t login = 0;
static uint32_t login_uid = 0xffffffff;
#else
static int32_t login = 1;
static uint32_t login_uid = 0;
#endif
static int32_t user_flag = 0;
static int32_t passwd_flag = 0;

/* A queue for received characters, not processed yet */
osMessageQId tCLI_RecvQueue;
//------------------------------------------------------------------------------
int32_t CLI_ispasswd()
{
	return passwd_flag;
}
//------------------------------------------------------------------------------
int32_t CLI_islogin()
{
	return login;
}
//------------------------------------------------------------------------------
int32_t CLI_cmd_quit(void)
{
	return 0;
}
//------------------------------------------------------------------------------
int32_t CLI_cmd_setloglv(int argc, char* argv[])
{
	SYS_PrintLevel_t tCLI_SysLvl = (SYS_PrintLevel_t)atoi(argv[1]);

	if((argc > 2) || (tCLI_SysLvl > DBG_Debug3Lvl))
	{
		printf(" Please Check Uasge!\n");
		printf("###################################\n");
		printf(" Usage: setlvl <Level>\n");
		printf(" Level: \n");
		printf("	 0: OFF (Turn off logging)\n");
		printf("	 1: Critical Level\n");
		printf("	 2: Error Level\n");
		printf("	 3: Info Level\n");
		printf("	 4: Debug Level\n");
		printf(" Example: setlvl 1\n");
		printf("###################################\n");
		return cliFAIL;
	}
	SYS_SetPrintLevel(tCLI_SysLvl);
	return cliPASS;
}
//------------------------------------------------------------------------------
int32_t CLI_cmd_logout(int argc, char* argv[])
{
	login = 0;
	user_flag = 0;
	passwd_flag = 0;
	login_uid = 0xffffffff;
	curr_lv = cmd_main_tbl;

	return 0;
}
//------------------------------------------------------------------------------
int32_t CLI_cmd_help(int argc, char* argv[])
{
	int32_t i = 1;

	while (curr_lv[i].usage) {
		if (login_uid <= curr_lv[i].cmd_lv) {
			printf("%s\n", curr_lv[i].usage);
		}
		i++;
	}

	return 0;
}
//------------------------------------------------------------------------------
int32_t CLI_cmd_back(int argc, char* argv[])
{
	if (curr_lv[0].prev_lv) {
		curr_lv = curr_lv[0].prev_lv;
	}

	CLI_cmd_help(0, NULL);

	/* cmd_back return a special value */
	return 99;
}
//------------------------------------------------------------------------------
int32_t CLI_tok_cmd(char *argument, char *array_arg[16])
{
	int32_t num_arg = 0;
	char *delim=" ";

	array_arg[0] = strtok(argument,delim);
	for (num_arg = 1 ; num_arg < 16 ; num_arg++) {
		
		array_arg[num_arg] = strtok(NULL,delim);
		if(!array_arg[num_arg])
		break;

	}

	return num_arg;
}
//------------------------------------------------------------------------------
int32_t CLI_rtoscli_login(char *user_pw)
{
	int32_t ret = 0;

	if (passwd_flag == 0) {
		if (!strncmp("admin", user_pw, 5)) {
			// Check
			user_flag = 1;
		} else {
			user_flag = 0;
		}
		passwd_flag = 1;
	} else {
		// Password check
		if (!strncmp("1234", user_pw, 4) && user_flag) {
			login = 1;
			login_uid = 0;	// Test for admin
			CLI_cmd_help(0, NULL);
		} else {
			ret = -1;
			login = 0;
		}
		user_flag = 0;
		passwd_flag = 0;
	}

	return ret;
}
//------------------------------------------------------------------------------
int32_t CLI_parse_cmd(char *cmd, int isTAB)
{
	int32_t i = 1;
	int32_t ret = 0;
	int32_t found = 0;
	//int only = 0, tmp_ind = 0;
	//int x = 0;
	char *pargv[16];
	int32_t num_argc = 0;

	if (strlen(cmd) == 0)
		return 0;

	//memset(search_tmp, 0, 128);

#ifdef CFG_ENABLE_LOGIN
	if (!CLI_islogin()) {
		CLI_rtoscli_login(cmd);
		return 0;
	}
#endif

	num_argc = CLI_tok_cmd(cmd, pargv);

	while (curr_lv[i].name != NULL) {
		if ((login_uid <= curr_lv[i].cmd_lv) && (curr_lv[i].len_name == strlen(pargv[0])))
			found = strncmp(curr_lv[i].name, pargv[0], curr_lv[i].len_name);
		else
			found = 1;	/* not found */

		if (!found) {
			if (curr_lv[i].Func != NULL) {
				//num_argc = tok_cmd(cmd, pargv);
				ret = curr_lv[i].Func(num_argc, pargv);
			}

			if (curr_lv[i].next_lv && ret != 99) {
				curr_lv = curr_lv[i].next_lv;
				CLI_cmd_help(0, NULL);
			}

			break;
		}
		i++;
	}

	if (found != 0) {
		ret = 0;
		printf("Command not found\n");
		CLI_cmd_help(0, NULL);
	} else {
		ret = 1;
	}

#if 0
	/* isTAB > 0 mean hit TAB key */
	if (isTAB) {
		while (curr_lv[i].name != NULL) {
			found = strncasecmp(curr_lv[i].name, cmd, isTAB);
			if (!found) {
				sprintf(search_tmp, "%s %s", search_tmp, curr_lv[i].name);
				only++;
				tmp_ind = i;
			}
			i++;
		}
		if (only == 1) {
			printf("                                                                                ");
			x = x - isTAB;
			printf("%s", curr_lv[tmp_ind].name);
			strncpy(cmd, curr_lv[tmp_ind].name, curr_lv[tmp_ind].len_name);
			ret = curr_lv[tmp_ind].len_name - isTAB;
		} else if (only > 1) {
			printf("%s", search_tmp);
		} else {
			printf("                                                                                ");
			ret = 0;
		}

		return ret;
	} else { /* isTAB == 0 */
		while (curr_lv[i].name != NULL) {
			found = strncmp(curr_lv[i].name, cmd, curr_lv[i].len_name);
			if (!found) {
				if (curr_lv[i].Func != NULL) {
					num_argc = tok_cmd(cmd, pargv);
					//printf("argc = %d\n", num_argc);
					ret = curr_lv[i].Func(num_argc, pargv);
				}

				if (curr_lv[i].next_lv && ret != 99) {
					curr_lv = curr_lv[i].next_lv;
					CLI_cmd_help();
				}

				break;
			}
			i++;
		}

		if (found != 0) {
			ret = 0;
			printf("Command not found\n");
			CLI_cmd_help();
		} else {
			ret = 1;
		}
	}
#endif
	return ret;
}
//------------------------------------------------------------------------------
void CLI_init_rtos_cli()
{
	curr_lv = cmd_main_tbl;
#ifdef CFG_ENABLE_LOGIN
	printf(LOGIN_PROMPT);
#else
	CLI_cmd_help(0, NULL);
	printf(PROMPT, curr_lv[0].name);
#endif
}
//------------------------------------------------------------------------------
void CLI_show_cli_prompt()
{
	if (CLI_islogin()) {
		printf(PROMPT, curr_lv[0].name);
		return;
	}
	if (CLI_ispasswd()){
		printf(PASSWD_PROMPT);
	} else {
		printf(LOGIN_PROMPT);
	}

	return;
}
//------------------------------------------------------------------------------
void CLI_recvInit()
{
    /* Create and assert a queue for received characters */
	osMessageQDef(tCLI_RevQueue, RECV_QUEUE_SIZE, sizeof(char));
	tCLI_RecvQueue = osMessageCreate(osMessageQ(tCLI_RevQueue), NULL);
}
//------------------------------------------------------------------------------
void CLI_rtoscli_recv(char ch)
{
	if((osMessagePut(tCLI_RecvQueue, (void*) &ch, 0)) != osOK) {
		printf("CLI Q full\r\n");
	}
}
//------------------------------------------------------------------------------
void CLI_RecvThread(void const *argument)
{
	char ch;
	char cmdbuf[MAX_CMD_LENGTH];
	int32_t i = 0;
	CLI_init_rtos_cli();
	
	for ( ; ; ) {
		/* The task is blocked until something appears in the queue */
		if (osMessageGet(tCLI_RecvQueue, (void*) &ch, osWaitForever)) {
			if (!CLI_ispasswd()) {
				printf("%c",ch);
				//print_char(ch);
			} else {
				//print_char('*');
			}

    		if (ch == 0xd) { /* enter key */
    			if (i > 0) {
    				printf("\n");
    				cmdbuf[i] = 0x0;

    				CLI_parse_cmd(cmdbuf, 0);
						memset(cmdbuf, 0, sizeof(cmdbuf));
    				i = 0;
    			}

    			CLI_show_cli_prompt();
    			//printf(PROMPT, curr_lv[0].name);
    		} else if (ch == 0x7f) { // Del key
    			if (i > 0) {
    				i--;
    				cmdbuf[i] = 0x0;
    			}
    		} else {
    				//printf("%c", c);
    				cmdbuf[i] = ch;
    				i++;
    				if (i >= MAX_CMD_LENGTH - 1) { // Over Max command length, Force excute command to avoid buffer overflow
    					printf("\n");
							
							//Bruce 2016/09/30
    					//cmdbuf[MAX_CMD_LENGTH] = 0x0;

        				CLI_parse_cmd(cmdbuf, 0);
        				memset(cmdbuf, 0, sizeof(cmdbuf));
        				i = 0;

        				CLI_show_cli_prompt();
    				}
    		}
		}
	}

	//(void) params;
}
//------------------------------------------------------------------------------
void CLI_Init()
{
	//RBK the stack size was 5k, increate to 5k*3 due to stack overflow
	CLI_recvInit();
	osThreadDef(cliThread, CLI_RecvThread, osPriorityLow, 1, 1280);
	osThreadCreate(osThread(cliThread), NULL);
}
//------------------------------------------------------------------------------
