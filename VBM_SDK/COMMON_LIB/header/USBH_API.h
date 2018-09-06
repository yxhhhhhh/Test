#ifndef _USBH_API_H_
#define _USBH_API_H_

#include "_510PF.h"

#define USBH_FuncEnable									\
						{								\
							USB_PHY1->PHY_PWR = 1;		\
							USB_PHY2->PHY_PWR = 1;		\
							GLB->UH1_FUNC_DIS = 0;		\
							GLB->UH2_FUNC_DIS = 0;		\
						}
#define USBH_FuncDisable								\
						{								\
							USB_PHY1->PHY_PWR = 0;		\
							USB_PHY2->PHY_PWR = 0;		\
							GLB->UH1_FUNC_DIS = 1;		\
							GLB->UH2_FUNC_DIS = 1;		\
						}
typedef	enum
{
	USBH_UVC_NONE = 0,
	USBH_UVC_YUV,
	USBH_UVC_MJPEG,
	USBH_UVC_H264
}USBH_UVC_FRAME_FMT;

typedef	enum
{
	USBH_UVC_1920x1080 = 0,
	USBH_UVC_1280x720
}USBH_UVC_FRAME_RESOLUTION;

typedef	enum
{
	USBH_UVC_FPS30 = 0
}USBH_UVC_FRAME_FPS;

typedef enum _USBH_PLUG_STATUS
{
    USBH_PLUG_OUT,
    USBH_PLUG_IN
}USBH_PLUG_STATUS;

typedef struct _USBH_UVC_FRAME_INFO
{
    USBH_UVC_FRAME_FMT fmt[8];
    uint16_t ulWidth[8];
    uint16_t ulHeight[8];
}USBH_UVC_FRAME_INFO;

typedef struct _USBH_UVC_QUEUE_DATA_INFO
{
    uint32_t ulStartAddr;
    uint32_t ulSize;
}USBH_UVC_QUEUE_DATA;

extern osMessageQId USBH_Q_Ext;

uint32_t ulUSBH_GetBufferSize(uint8_t bIsEnableWIFI, uint8_t bIsEnableDualHost, uint8_t stream_num, USBH_UVC_FRAME_INFO uvc_frame_info);
void USBH_Init(uint32_t ulBufferStrAddr);
uint8_t ubUSBH_UVC_GetInfo(void);
USBH_PLUG_STATUS USB_GetPlugStatus(void);

uint8_t ubUSBH_UVC_Start(uint8_t DeviceId, USBH_UVC_FRAME_FMT Fmt, USBH_UVC_FRAME_RESOLUTION Res, USBH_UVC_FRAME_FPS Fps);

void ulUSBH_MscInit(uint8_t DeviceId);
uint32_t ulUSBH_GetMscCapacity(void);
uint8_t ubUSBH_Msc_Write(uint32_t ulSrcAddr, uint32_t ulLba, uint32_t ulSize);
uint8_t ubUSBH_Msc_Read(uint32_t ulDesAddr, uint32_t ulLba, uint32_t ulSize);
#endif
