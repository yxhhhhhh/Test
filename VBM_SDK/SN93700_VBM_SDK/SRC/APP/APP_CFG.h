/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_CFG.h
	\brief		APP Configuration header file
	\author		Hanyi Chiu
	\version	0.8
	\date		2017/11/06
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#include <stdint.h>

//Thread Priority Setting
//-----------------------------------------------------
#define THREAD_PRIO_UI_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_UIEVENT_HANDLER		osPriorityAboveNormal
#define THREAD_PRIO_KEY_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_BB_HANDLER			osPriorityRealtime
#define THREAD_PRIO_KNL_PROC			osPriorityHigh
#define THREAD_PRIO_KNL_AVG_PLY			osPriorityAboveNormal
#define THREAD_PRIO_KNL_TWC_MONIT		osPriorityNormal
#define THREAD_PRIO_KNL_VDOIN_PROC		osPriorityAboveNormal
#define THREAD_PRIO_ADO_ENC_PROC		osPriorityAboveNormal
#define THREAD_PRIO_ADO_DEC_PROC		osPriorityHigh
#define THREAD_PRIO_COMM_TX_VDO			osPriorityHigh
#define THREAD_PRIO_COMM_RX_VDO			osPriorityNormal
#define THREAD_PRIO_COMM_RX_ADO			osPriorityNormal
#define THREAD_PRIO_IMG_MONIT			osPriorityNormal
#define THREAD_PRIO_APP_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_PAIRING_HANDLER		osPriorityNormal
#define THREAD_PRIO_EN_HANDLER			osPriorityBelowNormal
#define THREAD_PRIO_KNLRECORD_HANDLER	osPriorityBelowNormal
#define THREAD_PRIO_JPEG_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_LINK_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_RC_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_RC_SYS_MONIT		osPriorityBelowNormal
#define THREAD_PRIO_LINK_UPDATE			osPriorityBelowNormal
#define THREAD_PRIO_SEC_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_SYS_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_SD_HANDLER          osPriorityNormal
#define THREAD_PRIO_FS_HANDLER          osPriorityNormal
#define THREAD_PRIO_LOOPREC_HANDLER     osPriorityNormal
#define THREAD_PRIO_WDT_HANDLER			osPriorityLow				// other thread can not set osPriorityLow!!!

//Thread Stack Size Setting
//-----------------------------------------------------
#define THREAD_STACK_UI_HANDLER			3072
#define THREAD_STACK_UIEVENT_HANDLER	2048
#define THREAD_STACK_KEY_HANDLER		1024
#define THREAD_STACK_BB_HANDLER			8192
#define THREAD_STACK_EN_HANDLER			8192
#define THREAD_STACK_KNL_PROC			8192
#define THREAD_STACK_KNL_AVG_PLY		2048
#define THREAD_STACK_KNL_TWC_MONIT		2048
#define THREAD_STACK_KNL_VDOIN_PROC		2048
#define THREAD_STACK_ADO_PROC			8192
#define THREAD_STACK_COMM_TX_VDO	    2048
#define THREAD_STACK_COMM_RX_VDO	    1536
#define THREAD_STACK_COMM_RX_ADO	    1536
#define THREAD_STACK_IMG_MONIT		    1024
#define THREAD_STACK_APP_HANDLER		8192
#define THREAD_STACK_PAIRING_HANDLER	512
#define THREAD_STACK_KNLRECORD_HANDLER	1024
#define THREAD_STACK_JPEG_MONIT			2048
#define THREAD_STACK_LINK_MONIT			1024
#define THREAD_STACK_RC_MONIT			2048
#define THREAD_STACK_RC_SYS_MONIT		2048
#define THREAD_STACK_LINK_UPDATE		1024
#define THREAD_STACK_SEC_MONIT			1024
#define THREAD_STACK_SYS_MONIT			1024
#define THREAD_STACK_SD_HANDLER         4096
#define THREAD_STACK_FS_HANDLER         4096
#define THREAD_STACK_LOOPREC_HANDLER    2048

//! APP Event Definition
#define APP_REFRESH_EVENT				0
#define APP_PWRCTRL_EVENT				1
#define APP_LINK_EVENT					2
#define APP_LOSTLINK_EVENT				3
#define APP_PAIRING_START_EVENT			4
#define APP_PAIRING_STOP_EVENT			5
#define APP_PAIRING_SUCCESS_EVENT		6
#define APP_PAIRING_FAIL_EVENT			7
#define APP_LINKSTATUS_REPORT_EVENT		8
#define APP_UNBIND_BU_EVENT				9
#define APP_VIEWTYPECHG_EVENT			10
#define APP_ADOSRCSEL_EVENT				11
#define APP_PTT_EVENT					12
#define APP_POWERSAVE_EVENT				13

//! APP Serial Flash Start Sector
#define PAIR_SF_START_SECTOR			2
#define UI_SF_START_SECTOR				6		//! Greater than to 2
#define KNL_SF_START_SECTOR				7		//! Greater than to 2

#define	SF_AP_UI_SECTOR_TAG 			"SN93701_UI"
#define	SF_STA_UI_SECTOR_TAG 			"SN93700_UI"

#define	SF_AP_KNL_SECTOR_TAG 			"SN93701_KNL"
#define	SF_STA_KNL_SECTOR_TAG 			"SN93700_KNL"

//! APP FWU Volume label
#ifdef VBM_BU
#define SN937XX_VOLUME_LABLE			"SN93700BU"
#endif
#ifdef VBM_PU
#define SN937XX_VOLUME_LABLE			"SN93701PU"
#endif

//! APP Queue message type
typedef struct
{
	uint8_t ubAPP_Event;
	uint8_t ubAPP_Message[7];
}APP_EventMsg_t;

//! APP Display mode
#define	DISPLAY_1T1R  					1
#define	DISPLAY_2T1R  					2
#define	DISPLAY_4T1R  					4
#define DISPLAY_MODE					DISPLAY_4T1R

//! APP Display Resolution
typedef enum
{
	FHD_WIDTH 	= 1920,
	FHD_HEIGHT	= 1088,
	HD_WIDTH 	= 1280,
	HD_HEIGHT	= 720,
	WVGA_WIDTH  = 800,
	WVGA_HEIGHT = 480,
	VGA_WIDTH   = 640,
	VGA_HEIGHT  = 480,
}APP_DISPLAY_RESOLUTION;

typedef enum
{
	LCD_PM_SUSPEND,
	LCD_PWR_OFF
}LCD_PM_OPT;

#define LCD_PM							LCD_PWR_OFF

#define RTC_RECORD_PWRSTS_ADDR			0
#define RTC_PWRSTS_KEEP_TAG				0x03
#define RTC_WATCHDOG_CHK_TAG			0x0C
#define RTC_PS_WOR_TAG					0x30

#define RTC_RECORD_VIEW_MODE_ADDR		1
#define RTC_RECORD_VIEW_CAM_ADDR		2

#define WDT_TIMEOUT_CNT					8	// unit: 1 second / 3

//! SF Write protect use GPIO
#define SF_WP_GPIN						14		//!< 0~13, >=14 is no wp pin

#endif
