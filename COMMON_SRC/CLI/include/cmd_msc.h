#ifndef _CMD_MSC_H_
#define _CMD_MSC_H_

#include "sonix_config.h"

#if defined (CONFIG_USBH_FREE_RTOS)
#include <FreeRTOS.h>
#endif

#if defined(CONFIG_CLI_MSC)	&& defined (CONFIG_MODULE_USB_MSC_CLASS)
int32_t cmd_msc_rw_test(int argc, char* argv[]);
int32_t cmd_msc_card_reader_test(int argc, char* argv[]);

#define CMD_TBL_MSC CMD_TBL_ENTRY(		\
	"+msc",		4,      NULL,			\
	"+msc		- MSC Command Table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_tbl_msc,		cmd_main_tbl			\
),

#define CMD_TBL_RW_TEST	CMD_TBL_ENTRY(		\
	"rw_test",		7,      cmd_msc_rw_test,	\
	"rw_test		- MSC Read/Write test <host> <test times> <sector count>",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_CR_TEST	CMD_TBL_ENTRY(		\
	"cr_test",		7,      cmd_msc_card_reader_test,	\
	"cr_test		- Card Reader Test <host>",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif	//end of #if defined(CONFIG_CLI_MSC)	&& defined (CONFIG_MODULE_USB_MSC_CLASS)

#endif
