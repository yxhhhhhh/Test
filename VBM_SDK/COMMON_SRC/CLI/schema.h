#ifndef _SCHEMA_H_
#define _SCHEMA_H_

#include "CLI.h"
#include "stdio.h"
#include "cmd.h"

struct cmd_table *curr_lv;
struct cmd_table cmd_main_tbl[];

#ifdef CONFIG_CLI_CMD_SYSTEM
struct cmd_table cmd_system_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_I2C
struct cmd_table cmd_i2c_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_ISP
struct cmd_table cmd_isp_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_SF
struct cmd_table cmd_sf_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_BB
struct cmd_table cmd_bb_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_LCD
struct cmd_table cmd_lcd_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_MC
struct cmd_table cmd_mc_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_ADO
struct cmd_table cmd_ado_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_DDR
struct cmd_table cmd_ddr_tbl[];
#endif

// =======================================================
#ifdef CONFIG_CLI_CMD_PRINTF
struct cmd_table cmd_printf_tbl[] = {
	CMD_TBL_PRINTF
	CMD_TBL_PRINTF_HELLO
	CMD_TBL_PRINTF_ARGU
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_H264
struct cmd_table cmd_h264_tbl[] = {
	CMD_TBL_H264
	CMD_TBL_H264_INIT
	CMD_TBL_H264_STATUS
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_UVC
struct cmd_table cmd_uvc_tbl[] = {
	CMD_TBL_UVC
	CMD_TBL_UVC_GET_INFO
	CMD_TBL_UVC_START
	CMD_TBL_UVC_STOP
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_SYSTEM
struct cmd_table cmd_system_tbl[] = {
	CMD_TBL_SYSTEM
	CMD_TBL_SYS_DATE
	CMD_TBL_SYS_PHYMEM_RW
	CMD_TBL_SYS_FPS
	CMD_TBL_SYS_PADIO
	CMD_TBL_SYS_WDT
	CMD_TBL_SYS_USBD
	CMD_TBL_SYS_FWU
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_I2C
struct cmd_table cmd_i2c_tbl[] = {
	CMD_TBL_I2C
	CMD_TBL_I2C_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_I2C


#ifdef CONFIG_CLI_CMD_ISP
struct cmd_table cmd_isp_tbl[] = {
	CMD_TBL_ISP
	CMD_TBL_ISP_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_SF
struct cmd_table cmd_sf_tbl[] = {
	CMD_TBL_SF
	CMD_TBL_SF_INFO
	CMD_TBL_SF_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_BB
struct cmd_table cmd_bb_tbl[] = {
	CMD_TBL_BB
	CMD_TBL_BB_INFO
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_LCD
struct cmd_table cmd_lcd_tbl[] = {
	CMD_TBL_LCD
	CMD_TBL_LCD_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_MC
struct cmd_table cmd_mc_tbl[] = {
	CMD_TBL_MOTOR
	CMD_TBL_MOTOR_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_ADO
struct cmd_table cmd_ado_tbl[] = {
	CMD_TBL_ADO
	CMD_TBL_ADO_WAV
	CMD_TBL_ADO_BUZZER
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_DDR
struct cmd_table cmd_ddr_tbl[] = {
	CMD_TBL_DDR
	CMD_TBL_DDR_GCONF
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

struct cmd_table cmd_main_tbl[] = {
	CMD_TBL_MAIN

	CMD_TBL_LOGOUT

#ifdef CONFIG_CLI_CMD_H264
	CMD_TBL_H264
#endif

#ifdef CONFIG_CLI_CMD_PRINTF
	CMD_TBL_PRINTF
#endif
	
#ifdef CONFIG_CLI_CMD_VCS
	CMD_TBL_VCS
#endif	

#ifdef CONFIG_CLI_CMD_UVC
	CMD_TBL_UVC
#endif

#ifdef CONFIG_CLI_CMD_SYSTEM
	CMD_TBL_SYSTEM
#endif

#ifdef CONFIG_CLI_CMD_I2C
	CMD_TBL_I2C
#endif

#ifdef CONFIG_CLI_CMD_ISP
	CMD_TBL_ISP
#endif

#ifdef CONFIG_CLI_CMD_SF
	CMD_TBL_SF
#endif

#ifdef CONFIG_CLI_CMD_BB
	CMD_TBL_BB
#endif

#ifdef CONFIG_CLI_CMD_LCD
	CMD_TBL_LCD
#endif

#ifdef CONFIG_CLI_CMD_MC
	CMD_TBL_MOTOR
#endif

#ifdef CONFIG_CLI_CMD_ADO
	CMD_TBL_ADO
#endif

#ifdef CONFIG_CLI_CMD_DDR
	CMD_TBL_DDR
#endif

	CMD_TBL_SHOW_DEV_ID
	CMD_TBL_SET_DEBUG_LV
	CMD_TBL_HELP
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif
