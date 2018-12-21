#ifndef _CMD_UVC_H_
#define _CMD_UVC_H_

#include <stdio.h>
#include <string.h>
#include "USBH_UVC.h"

enum {
	SLICE_PIC	= 1,
	SLICE_PA,
	SLICE_PB,
	SLICE_PC,
	SLICE_IDR_PIC,
	SEI,
	SPS,
	PPS,
};

enum {
	TYPE_ERROR = -2,
	NOT_SLICE,
	TYPE_P,
	TYPE_B,
	TYPE_I,
	TYPE_SP,
	TYPE_SI
};

typedef struct bits_handler {
	const char *ptr, *base;
	unsigned length;
	int index;
} bits_handler;

int32_t cmd_uvc_init(int argc, char* argv[]);
int32_t cmd_uvc_get_info(int argc, char* argv[]);		
int32_t cmd_uvc_start(int argc, char* argv[]);
int32_t cmd_uvc_stop(int argc, char* argv[]);

#define CMD_TBL_UVC		CMD_TBL_ENTRY(		\
	"+uvc",		4,      cmd_uvc_init,			\
	"+uvc		- UVC Command Table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_uvc_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_UVC_GET_INFO	CMD_TBL_ENTRY(		\
	"uvc_get_info",	12,      cmd_uvc_get_info,	\
	"uvc_get_info	- UVC GET Stream Infomation",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UVC_START	CMD_TBL_ENTRY(		\
	"uvc_start",	9,      cmd_uvc_start,	\
	"uvc_start	- UVC START Stream",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UVC_STOP	CMD_TBL_ENTRY(		\
	"uvc_stop",	8,      	cmd_uvc_stop,	\
	"uvc_stop	- UVC STOP Stream",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

typedef struct
{
	uint32_t	stream_id;
	uint32_t 	*ptr;
	uint32_t 	size;
	uint32_t 	framecnt;
	uint32_t	errcnt;
	uint32_t	babblecnt;
	uint32_t	underflowcnt;
	uint32_t	discardcnt;
	char		fmt[8];
	char		width[8];
	char		height[8];	
	char		fps[8];
	uint32_t	debug_msg:1;
	uint32_t	sd_record:1;
	uint32_t	usb_preview:1;
	uint32_t	lcm_preview:1;
	uint32_t	isH264:1;
	uint32_t	isIFrame:1;
	uint32_t	isOpen:1;
	uint32_t	reserve:25;
	
}USBH_UVC_APP_STREAM_STRUCTURE;

typedef struct
{  
	USBH_UVC_APP_STREAM_STRUCTURE	stream[max_stream_count];
}USBH_UVC_APP_DEV_STRUCTURE;

typedef struct
{
	uint32_t 			sd_record_enable;	
	uint32_t 			usb_preview_enable;		
	uint32_t 			lcm_preview_enable;	
	uint32_t 			debug_enable;
	//uint32_t 			err_debug_enable;
#if(_SUPPORT_USBD_WIFI_PREVIEW)
	int 					socket_fd;
#endif	
	USBH_UVC_APP_DEV_STRUCTURE	dev[USBH_MAX_PORT*2];
	
}USBH_UVC_APP_STRUCTURE;

extern USBH_UVC_APP_STRUCTURE usbh_uvc_app;

#endif
