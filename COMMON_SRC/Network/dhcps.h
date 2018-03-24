/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		dhcps.h
	\brief		DHCP Server header file
	\author		Hann Chiu
	\version	1
	\date		2015/02/05
	\copyright	Copyright(C) 2014 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __DHCPS_H__
#define __DHCPS_H__

#include "SNX_Network.h"

#define IPV4_ADDR_LEN					4
#define MAC_HWADDR_LEN					6
#define DHCPS_PORT						67
#define DHCPC_PORT						68
#define NETIF_AP_DEF_IP1				192//5	//192
#define NETIF_AP_DEF_IP2				168//5	//168
#define NETIF_AP_DEF_IP3				0//1	//10
#define NETIF_AP_DEF_IP4				1//253	//1
#define NETIF_AP_NETMASK1				255
#define NETIF_AP_NETMASK2				255
#define NETIF_AP_NETMASK3				255
#define NETIF_AP_NETMASK4				0

#define NETIF_AP_IP_ADDR(netif, ipaddr)						\
{															\
	IP4_ADDR(ipaddr, NETIF_AP_DEF_IP1, NETIF_AP_DEF_IP2, NETIF_AP_DEF_IP3, NETIF_AP_DEF_IP4);	\
	netif_set_ipaddr(netif, ipaddr);						\
}
#define NETIF_AP_SUB_NETMASK(netif, ipaddr)					\
{															\
	ip4_addr_set_u32(ipaddr, ipaddr_addr("255.255.255.0"));	\
	netif_set_ipaddr(netif, ipaddr);						\
}
#define NETIF_AP_GW(netif, ipaddr)							\
{															\
	IP4_ADDR(ipaddr, NETIF_AP_DEF_IP1, NETIF_AP_DEF_IP2, NETIF_AP_DEF_IP3, NETIF_AP_DEF_IP4);	\
	netif_set_ipaddr(netif, ipaddr);						\
}

typedef enum
{
	DHCPS_IP_UNUSED,
	DHCPS_IP_USED
}DHCPS_IP_STS;

typedef enum
{
	DHCPDISCOVER = 1,
	DHCPOFFER    = 2,
	DHCPREQUEST  = 3,
	DHCPACK      = 5
}DHCPS_MSG_TYPE;

typedef struct
{
	DHCPS_IP_STS dhcps_sts;
    uint8_t mac[6];
    uint8_t ip[4];
}DHCPS_Entry_t;

typedef struct 
{
    uint8_t  dp_op;
    uint8_t  dp_htype;
    uint8_t  dp_hlen;
    uint8_t  dp_hops;
    uint32_t dp_xid;
    uint16_t dp_secs;
    uint16_t dp_flags;
    uint8_t  dp_ciaddr[IPV4_ADDR_LEN];
    uint8_t  dp_yiaddr[IPV4_ADDR_LEN];
    uint8_t  dp_siaddr[IPV4_ADDR_LEN];
    uint8_t  dp_giaddr[IPV4_ADDR_LEN];
    uint8_t  dp_chaddr[16];
    uint8_t  dp_legacy[192];
    uint8_t  dp_magic[4];
    uint8_t  dp_options[275];
}DHCPS_TYPE;

void dhcps_init(void);
void dhcps_Uninit(void);
static void dhcps_Listen(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, uint16_t port);
void dhcps_findRequestIP(uint8_t *opts, uint16_t len, uint8_t *ip);
void dhcps_SetOptions(uint8_t *opts, DHCPS_MSG_TYPE msg_type);
uint8_t dhcps_InventAddress(uint8_t *mac, uint8_t *ip);
uint8_t dhcps_renewReply(uint8_t *mac);
uint8_t *dhcps_get_mac_addr(struct ip_addr *addr);
void dhpcs_update_arp_entry(uint8_t *mac);
void dhcps_notify_event(uint8_t Event);

#endif

