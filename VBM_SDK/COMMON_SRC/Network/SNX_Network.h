/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SNX_Netwrok.h
	\brief		SONiX Network header file
	\author		Hanyi Chiu
	\version	1
	\date		2017/01/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __SNX_NETWORK_H__
#define __SNX_NETWORK_H__

#include <stdint.h>
#include <string.h>
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/def.h"
#include "netif/etharp.h"

#define SNX_NTK_QUEUE_SZ				100
#define SNX_NTK_CYCLE_TIME				500
#define ETH_P_EAPOL 					0x888e

#define PRT_ENTER						printf("\n")
#define SNX_PRT_MSG_OFF					0x0
#define SNX_PRT_DEG_VAL					0x1
#define SNX_PRT_INFO_VAL				0x2
#define SNX_PRT_ERR_DEG_VAL				0x4
#define SNX_PRT_ERR_VAL					0x8
#define SNX_PRT_LEVEL					(SNX_PRT_DEG_VAL | SNX_PRT_ERR_DEG_VAL | SNX_PRT_ERR_VAL | SNX_PRT_INFO_VAL)
#define PRT_DEG(...)					if(SNX_PRT_LEVEL & SNX_PRT_DEG_VAL) { printf("%s(%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); PRT_ENTER; }
#define PRT_INFO(...)					if(SNX_PRT_LEVEL & SNX_PRT_INFO_VAL) { printf("%s(%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); PRT_ENTER; }
#define PRT_ERRDEG(...)					if(SNX_PRT_LEVEL & SNX_PRT_ERR_DEG_VAL) { printf("%s(%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); PRT_ENTER; }
#define PRT_ERR(...)					if(SNX_PRT_LEVEL & SNX_PRT_ERR_VAL) { printf("%s(%d): ", __FILE__, __LINE__); printf(__VA_ARGS__); PRT_ENTER; }

typedef enum
{
	WIFI_LINKED,		
	SNX_NTK_DHCP_START,
	SNX_NTK_CHK_DHCP_STS,
	WIFI_DISCONNECT = 0xFE,
	SNX_NTK_NONE = 0xFF,
}snx_ntk_state_t;

typedef enum 
{
	SNX_NOT_CONNECTED = 0,
	SNX_ACCEPTED,
	SNX_CONNECTED,
	SNX_RECEIVED,
	SNX_CLOSING,
}snx_ntk_tcpudp_connsts_t;

#if (!LWIP_NETCONN && LWIP_DNS)
struct dns_api_msg 
{
  const char *name;
  ip_addr_t *addr;
  sys_sem_t *sem;
  err_t *err;
};
#endif	//!<End #if !LWIP_NETCONN

typedef uint8_t(*pvWiFi_ScanCH)(void);
	
void SNX_Network_Init(void);
void SNX_Network_Start(void);
void SNX_Network_Stop(void);
void SNX_Network_WiFiConfig(void);
void SNX_Network_LwipConfig(void);
err_t ethernetif_init( struct netif *xNetIf );
void SNX_Network_SetState(uint16_t uwState);
void SNX_Network_UpdateDHCPS_ARPTable(u8_t *pmac);
#if (!LWIP_NETCONN && LWIP_DNS)
err_t SNX_Network_GetHostByName(const char *name, ip_addr_t *addr);
void do_gethostbyname(void *arg);
static void do_dns_found(const char *name, ip_addr_t *ipaddr, void *arg);
#endif	//!<End #if !LWIP_NETCONN
void SNX_Network_RecvFunc(void *pvBuF, uint32_t len, void *pvMemType);

#endif
