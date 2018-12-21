/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WiFi_API.h
	\brief		Wi-Fi header
	\author		Hann Chiu
	\version	2
	\date		2015/08/03
	\copyright	Copyright(C) 2015 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __WIFI_API_H__
#define __WIFI_API_H__

#include "_510PF.h"

#define SUCCESS 		0
#define UNSUCCESS   	1
#define UNASSIGNED		2

#ifdef WIFI_TYPE_SDIO
#define WIFI_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX 	( 8 + 12 + 4 + 2 )
#else
#define WIFI_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX 	( 8 + 12 + 4 + 2 + 2 )
#endif
#define WIFI_LINK_TAIL_AFTER_ETHERNET_FRAME     		( 0 )
#define WIFI_ETHERNET_SIZE         						(14)
#define WIFI_PHYSICAL_HEADER       						(WIFI_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX + WIFI_ETHERNET_SIZE)
#define WIFI_PHYSICAL_TRAILER      						(WIFI_LINK_TAIL_AFTER_ETHERNET_FRAME)
#define WIFI_PAYLOAD_MTU           						(1500)
#define WIFI_LINK_MTU              						(WIFI_PAYLOAD_MTU + WIFI_PHYSICAL_HEADER + WIFI_PHYSICAL_TRAILER)

typedef enum
{
	WIFI_POWER_OFF,
	WIFI_POWER_ON
}WiFi_PowerCtrl_t;

typedef enum
{
	WIFI_MT7601,
	WIFI_AP6181
}WiFi_SupportType_t;

typedef enum
{
	WIFI_STA_INTERFACE     = 0, /**< STA Interface  */
    WIFI_AP_INTERFACE      = 1, /**< SoftAP Interface         */
}WIFI_Interface_t;

typedef enum
{
	RECV_DIRECT,
	RECV_MEMREALLOC,
}WIFI_RecvMemType_t;

typedef enum
{
    WIFI_SECURITY_OPEN           = 0,                 /**< Open security                           */
    WIFI_SECURITY_WPA_TKIP_PSK   = 3,                 /**< WPA Security with TKIP                  */
    WIFI_SECURITY_WPA_AES_PSK    = 4,                 /**< WPA Security with AES                   */
    WIFI_SECURITY_WPA2_AES_PSK   = 5,                 /**< WPA2 Security with AES                  */
    WIFI_SECURITY_WPA2_TKIP_PSK  = 6,                 /**< WPA2 Security with TKIP                 */
    WIFI_SECURITY_WPA2_MIXED_PSK = 7,  				  /**< WPA2 Security with AES & TKIP           */
}WIFI_Security_t;

typedef struct
{
	struct
	{
		uint8_t len;
		uint8_t val[32];
	}SSID;
	WIFI_Security_t security;
	uint8_t security_key_length;
    char    security_key[64];
	uint8_t channel;
}WIFI_ConfigDct_t;

typedef void(*pvWiFi_PwrCtrl)(WiFi_PowerCtrl_t);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi Function initialize
\param tWiFi_IF 	Wi-Fi interface
\param PwrCtrl_cb 	Wi-Fi dongle power control callback function
\par [Example]
\code    
	 WiFi_Init(WIFI_STA_INTERFACE, APP_WiFiPowerControl);
\return(no)
*/
void WiFi_Init(WIFI_Interface_t tWiFi_IF, pvWiFi_PwrCtrl PwrCtrl_cb);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi dongle initialize
\return(no)
*/
void WiFi_Start(void);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi Buffer Setup
\param ulBUF_StartAddr 	Avaliable DDR address
\param PwrCtrl_cb 	Wi-Fi dongle power control callback function
\par [Example]
\code
	ulWiFi_BufSetup(0xA0000);
\return Wi-Fi Buffer Size
*/
uint32_t ulWiFi_BufSetup(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi Config
\param tDct_Info 	Router information
\return(no)
*/
void WiFi_ConfigDct(WIFI_ConfigDct_t tDct_Info);
//------------------------------------------------------------------------
/*!
\brief Get interface of Wi-Fi dongle
\return Interface
*/
WIFI_Interface_t WiFi_GetInterface(void);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi TX Function
\param data 		data
\param data_len		data length
\return result
*/
int WiFi_SendFunc(void *data, uint32_t data_len);
//------------------------------------------------------------------------
/*!
\brief Set callback function of Wi-Fi RX
\param recv_cb 		Callback function of RX
\return Function pointer
*/
typedef void(*pvWiFi_RX)(void *data, uint32_t data_len, void *type);
void WiFi_SetRecv_cbFunc(pvWiFi_RX recv_cb);
//------------------------------------------------------------------------
/*!
\brief Set callback function of Wi-Fi connected.(Link to Router)
\param connected_cb 		Callback function of connected
\return Function pointer
*/
typedef void(*pvWiFi_Connected)(void);
void WiFi_SetConnected_cbFunc(pvWiFi_Connected connected_cb);
//------------------------------------------------------------------------
/*!
\brief Set callback function of Wi-Fi disconnect
\param disconnect_cb 		Callback function of disconnect
\return Function pointer
*/
typedef void(*pvWiFi_DisConnect)(void);
void WiFi_SetDisConnect_cbFunc(pvWiFi_DisConnect disconnect_cb);
//------------------------------------------------------------------------
/*!
\brief Obtain the status of Wi-Fi dongle initialize
\return status
*/
uint32_t WiFi_GetInitStatus(void);
//------------------------------------------------------------------------
/*!
\brief Obtain the MAC address of Wi-Fi dongle
\param mac 		MAC address
\return result
*/
int WiFi_Get_MACAddr(void *mac);
//------------------------------------------------------------------------
/*!
\brief Obtain the Wi-Fi dongle type
\param mac 		MAC address
\return result
*/
uint8_t WiFi_GetWiFiDongleType(void);
//------------------------------------------------------------------------------
/*!
\brief 	Obtain Wi-Fi Version	
\return	Version
*/
uint16_t uwWiFi_GetVersion(void);
#endif
