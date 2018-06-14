/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		USBD_API.h
	\brief		USB Device Config header file
	\author		Hanyi Chiu
	\version	1.1
	\date		2018/06/12
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _USBD_API_H_
#define _USBD_API_H_

#include "_510PF.h"

#define USBD_ENABLE		TRUE

#define USBD_FuncEnable		(GLB->UD_FUNC_DIS = 0)
#define USBD_FuncDisable	(GLB->UD_FUNC_DIS = 1)

typedef enum
{
	USBD_UVC_MODE = 1,
	USBD_MSC_MODE,
	USBD_UNKNOWN_MODE,
}USBD_ClassMode_t;

typedef enum
{
    USB_UVC_VS_FORMAT_UNCOMPRESSED  = 0x04,
	USB_UVC_VS_FORMAT_MJPEG         = 0x06,
    USB_UVC_VS_FORMAT_FRAME_BASED   = 0x10,
}USBD_ClassType_t;

typedef enum
{
	USBD_SUCCESS	= 0,
	USBD_FAIL,
}USBD_STATUS;

typedef enum
{
    USB_UVC_FHD_WIDTH       = 1920,
	USB_UVC_HD_WIDTH        = 1280,
    USB_UVC_VGA_WIDTH       = 640,
}USBD_ResolutionType_t;

typedef void(*pvUsbdXuCbFunc)(uint8_t ubMode, uint32_t *pulBuf);

USBD_STATUS tUSBD_RegXuCbFunc(pvUsbdXuCbFunc pvCb);
//------------------------------------------------------------------------------
/*!
\brief USB Device initial
\param tClassMode 		USB device class mode
\return(no)
*/
void USBD_Init(USBD_ClassMode_t tClassMode);
//------------------------------------------------------------------------------
/*!
\brief USB Device Start
\return(no)
*/
void USBD_Start(void);
//------------------------------------------------------------------------------
/*!
\brief Get USB device config status
\return Config result
\par Note:
		USBD_SUCCESS: USB device enumeration success.
		USBD_FAIL   : USB device enumeration fail.
*/
USBD_STATUS tUSBD_GetConfigStatus(void);
//------------------------------------------------------------------------------
/*!
\brief Get USB Class Mode
\return Class mode
*/
USBD_ClassMode_t USBD_GetClassMode(void);
//------------------------------------------------------------------------------
/*!
\brief Buffer setup for USB Device
\param ulBUF_StartAddr 	Available memory address of DDR
\return USBD Buffer Size
*/
uint32_t USBD_BufSetup(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------------
/*!
\brief Get video format of UVC
\return Video format
*/
uint8_t UVC_GetVdoFormat(void);
//------------------------------------------------------------------------------
/*!
\brief Get video width of UVC
\return Video width
*/
uint16_t UVC_GetVdoWidth(void);
//------------------------------------------------------------------------------
/*!
\brief Update video image through UVC path
\param pImg_Buf 		Video Image buffer
\param ulImg_Size 		Video Image Size
\return Video Mode
*/
void uvc_update_image(uint32_t *pImg_Buf, uint32_t ulImg_Size);
//------------------------------------------------------------------------
/*!
\brief 	Get USBD Version	
\return	Version
*/
uint16_t uwUSBD_GetVersion(void);
int32_t slUSBD_GetIQFileSize(void);
void USBD_SetIQFileSize(int32_t slSize);
int16_t swUSBD_GetRemainTransferLength(void);
void USBD_SetRemainTransferLength(int16_t swLength);
#endif
