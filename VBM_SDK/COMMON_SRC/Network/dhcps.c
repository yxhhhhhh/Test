/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		dhcps.c
	\brief		DHCP Server
	\author		Hann Chiu
	\version	1
	\date		2015/02/05
	\copyright	Copyright(C) 2014 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "dhcps.h"
#include "SNX_udp.h"
#include "netif/etharp.h"
#include "APP_HS.h"

static DHCPS_TYPE dhcps;
DHCPS_Entry_t dhcpsTable[ARP_TABLE_SIZE];
struct udp_pcb *dhcps_upcb;
uint8_t router_ip[IPV4_ADDR_LEN] = {NETIF_AP_DEF_IP1, NETIF_AP_DEF_IP2, NETIF_AP_DEF_IP3, NETIF_AP_DEF_IP4};
char magic_cookie[] = {0x63, 0x82, 0x53, 0x63};
static uint8_t ubisbroadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//------------------------------------------------------------------------------
void dhcps_init(void)
{
	uint8_t ubIdx;

	memset(&dhcps, 0, sizeof(DHCPS_TYPE));
	for(ubIdx = 0; ubIdx < ARP_TABLE_SIZE; ubIdx++)
	{
		memset(&dhcpsTable[ubIdx], 0, sizeof(DHCPS_Entry_t));
		dhcpsTable[ubIdx].dhcps_sts = DHCPS_IP_UNUSED;
	}
	dhcps_upcb = SNX_udp_bind(DHCPS_PORT, dhcps_Listen);
	if(dhcps_upcb == NULL)
		PRT_DEG("dhcps_upcb is NULL");
	etharp_set_dhcps_dest_cbFunc(dhcps_get_mac_addr);
}
//------------------------------------------------------------------------------
void dhcps_Uninit(void)
{
	SNX_udp_connect_close(dhcps_upcb);
	dhcps_upcb = NULL;
}
//------------------------------------------------------------------------------
static void dhcps_Listen(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, uint16_t port)
{
#define osDHCPS_SIGNALS		0x7651
	DHCPS_MSG_TYPE dhcps_msg_type;
	osEvent tDhcps_Event;
	osStatus xStatus;
	udp_struct_arg *udp_arg;
//	struct pbuf *q;
//	err_t rc;

	udp_arg = (udp_struct_arg *)dhcps_upcb->recv_arg;
	memset(&dhcps, 0, sizeof(DHCPS_TYPE));
	memcpy(&dhcps, p->payload, sizeof(DHCPS_TYPE));
	pbuf_free(p);
	if(dhcps.dp_op != 0x1)
		return;

	switch(dhcps.dp_options[2])
	{
		case DHCPDISCOVER:
		    if(!dhcps_InventAddress(dhcps.dp_chaddr, dhcps.dp_yiaddr))
			{
				dhcps_msg_type 	= DHCPOFFER;
				dhcps.dp_op 	= 0x2;
				dhcps.dp_secs 	= 0;
				dhcps.dp_flags  = 0;				
				memcpy(dhcps.dp_magic, magic_cookie, 4);
				memset(&dhcps.dp_options, 0, sizeof(dhcps.dp_options));
				dhcps_SetOptions(dhcps.dp_options, dhcps_msg_type);
				IP4_ADDR(&udp_arg->ip, dhcps.dp_yiaddr[0], dhcps.dp_yiaddr[1], dhcps.dp_yiaddr[2], dhcps.dp_yiaddr[3]);
			}
			else
			{
				PRT_ERR("DHCPS: rejected discover\n");
				return;
			}
			break;
		case DHCPREQUEST:
			dhcps_msg_type 	= DHCPACK;
			dhcps.dp_op 	= 0x2;
			dhcps.dp_secs 	= 0;
			dhcps.dp_flags 	= 0;
			if((dhcps.dp_ciaddr[0] == NETIF_AP_DEF_IP1) && (dhcps.dp_ciaddr[1] == NETIF_AP_DEF_IP2) && (dhcps.dp_ciaddr[2] == NETIF_AP_DEF_IP3))
			{
				if(dhcps_renewReply(dhcps.dp_chaddr))
					return;
				memcpy(dhcps.dp_yiaddr, dhcps.dp_ciaddr, IPV4_ADDR_LEN);
			}
			else
			{
				dhcps_findRequestIP(dhcps.dp_options, sizeof(dhcps.dp_options), dhcps.dp_yiaddr);
			}
			IP4_ADDR(&udp_arg->ip, dhcps.dp_yiaddr[0], dhcps.dp_yiaddr[1], dhcps.dp_yiaddr[2], dhcps.dp_yiaddr[3]);			
			if(!memcmp(dhcps_get_mac_addr(&udp_arg->ip), ubisbroadcast, MAC_HWADDR_LEN))	//if(dhcps_get_mac_addr(&udp_arg->ip) == NULL)
				return;
			memcpy(dhcps.dp_magic, magic_cookie, 4);
			memset(&dhcps.dp_options, 0, sizeof(dhcps.dp_options));
			dhcps_SetOptions(dhcps.dp_options, dhcps_msg_type);
			break;
	}
	udp_arg->port 			= DHCPC_PORT;
	udp_arg->payload 		= (DHCPS_TYPE *)&dhcps;
	udp_arg->ulRemind_len 	= sizeof(DHCPS_TYPE);
	udp_arg->ulRemind_pos 	= 0;
	udp_arg->ubRTY_Num 		= 0;
	udp_arg->ubRTY_Cnt 		= 0;
	udp_arg->iWaitSignals 	= osDHCPS_SIGNALS;
	udp_arg->xUdpId 		= osThreadGetId();

	xStatus = osMessagePut(SNX_udpGet_SendQHandle(), udp_arg, osWaitForever);
	if(xStatus != osOK)
	{
		PRT_ERR("DHCPS Q Full");
		return;
	}
	else
	{
		tDhcps_Event = osSignalWait(osDHCPS_SIGNALS, 10000);		
		if(tDhcps_Event.status != osEventSignal)
		{
			PRT_ERR("dhcps send timeout...");
			dhcps_notify_event(0);
		}
	}

//	if(dhcps_msg_type == DHCPOFFER)
//		dhcps_notify_event(APP_DEV_SETTING_EVENT);
	if(dhcps_msg_type == DHCPACK)
		dhcps_notify_event(0);

	PRT_DEG("DHCPS %s: %02X:%02X:%02X:%02X:%02X:%02X -> %d.%d.%d.%d\n", 
	      (dhcps_msg_type == DHCPOFFER)?"Offer":"ACK",
		   dhcps.dp_chaddr[0],dhcps.dp_chaddr[1],dhcps.dp_chaddr[2],
		   dhcps.dp_chaddr[3],dhcps.dp_chaddr[4],dhcps.dp_chaddr[5],
		   dhcps.dp_yiaddr[0],dhcps.dp_yiaddr[1],dhcps.dp_yiaddr[2],dhcps.dp_yiaddr[3]);
}
//------------------------------------------------------------------------------
void dhcps_findRequestIP(uint8_t *opts, uint16_t len, uint8_t *ip)
{
	uint16_t uwIdx;
	uint8_t OptionEnd = 0xFF;
	uint8_t ReqIpAddr = 50;

	for(uwIdx = 0; uwIdx < len; uwIdx++)
	{
		if(opts[uwIdx] == OptionEnd)
			break;
		if(opts[uwIdx] == ReqIpAddr)
		{
			memcpy(ip, &opts[uwIdx + 2], IPV4_ADDR_LEN);
			break;
		}
	}
}
//------------------------------------------------------------------------------
void dhcps_SetOptions(uint8_t *opts, DHCPS_MSG_TYPE msg_type)
{
	char OptionIpLeaseTime[6] = {51, 4, 0, 0x0, 0x0, 0x78};;//{51, 4, 0, 0x1, 0x51, 0x80};
	char OptionSubnet[6]      = {1, 4, NETIF_AP_NETMASK1, NETIF_AP_NETMASK2, NETIF_AP_NETMASK3, NETIF_AP_NETMASK4};
	uint8_t OptionEnd = 0xFF;
	char *opts_ptr;

	opts_ptr = (char *)opts;
	opts_ptr[0] = 53;
	opts_ptr[1] = 1;
	opts_ptr[2] = msg_type;												//Message type
	opts_ptr += 3;

	opts_ptr[0] = 54;
	opts_ptr[1] = 4;
	memcpy(&opts_ptr[2], router_ip, IPV4_ADDR_LEN);						//Server Identifier
	opts_ptr += 6;

	memcpy(opts_ptr, OptionIpLeaseTime, sizeof(OptionIpLeaseTime));		//IP Address Lease Time
	opts_ptr += sizeof(OptionIpLeaseTime);
	
	memcpy(opts_ptr, OptionSubnet, sizeof(OptionSubnet));				//Subnet Mask
	opts_ptr += sizeof(OptionSubnet);

//	opts_ptr[0] = 3;
//	opts_ptr[1] = 4;
//	memcpy(&opts_ptr[2], router_ip, IPV4_ADDR_LEN);						//Routers
//	opts_ptr += 6;

//	opts_ptr[0] = 6;
//	opts_ptr[1] = 4;
//	memcpy(&opts_ptr[2], router_ip, IPV4_ADDR_LEN);						//DNS Servers
//	opts_ptr += 6;

	*opts_ptr = OptionEnd;
}
//------------------------------------------------------------------------------
uint8_t dhcps_InventAddress(uint8_t *mac, uint8_t *ip) 
{
	uint8_t ubIdx;

	for(ubIdx = 0; ubIdx < ARP_TABLE_SIZE; ubIdx++)
	{
		if(!memcmp(mac, dhcpsTable[ubIdx].mac, MAC_HWADDR_LEN)) 
		{
			memcpy(ip, dhcpsTable[ubIdx].ip, IPV4_ADDR_LEN);
			return 0;
		}
	}
	for(ubIdx = 0; ubIdx < ARP_TABLE_SIZE; ubIdx++)
	{
		if(dhcpsTable[ubIdx].dhcps_sts == DHCPS_IP_UNUSED)
		{
			router_ip[3] = (router_ip[3] > (250 - ARP_TABLE_SIZE))?(250 - ubIdx):(ubIdx + 3);
			dhcpsTable[ubIdx].dhcps_sts = DHCPS_IP_USED;
			memcpy(dhcpsTable[ubIdx].mac, mac, MAC_HWADDR_LEN);
			memcpy(dhcpsTable[ubIdx].ip, router_ip, IPV4_ADDR_LEN);
			memcpy(ip, dhcpsTable[ubIdx].ip, IPV4_ADDR_LEN);
			router_ip[3] = NETIF_AP_DEF_IP4;
			return 0;
		}
	}

    return 1;
}
//------------------------------------------------------------------------------
uint8_t dhcps_renewReply(uint8_t *mac)
{
	uint8_t ubIdx;
	for(ubIdx = 0; ubIdx < ARP_TABLE_SIZE; ubIdx++)
	{
		if(!memcmp(mac, dhcpsTable[ubIdx].mac, MAC_HWADDR_LEN))
			return 0;
	}
	return 1;
}
//------------------------------------------------------------------------------
uint8_t *dhcps_get_mac_addr(struct ip_addr *addr)
{
	uint8_t ubIdx;

	for(ubIdx = 0; ubIdx < ARP_TABLE_SIZE; ubIdx++)
	{
		if(!memcmp(addr, dhcpsTable[ubIdx].ip, IPV4_ADDR_LEN))
			return dhcpsTable[ubIdx].mac;
	}
	return ubisbroadcast;
}
//------------------------------------------------------------------------------
void dhpcs_update_arp_entry(uint8_t *mac)
{
	uint8_t ubIdx;

	for(ubIdx = 0; ubIdx < ARP_TABLE_SIZE; ubIdx++)
	{
		if(!memcmp(mac, dhcpsTable[ubIdx].mac, MAC_HWADDR_LEN))
		{
			memset(&dhcpsTable[ubIdx], 0, sizeof(DHCPS_Entry_t));
			dhcpsTable[ubIdx].dhcps_sts = DHCPS_IP_UNUSED;
		}
	}
}
//------------------------------------------------------------------------------
void dhcps_notify_event(uint8_t Event)
{

}
//------------------------------------------------------------------------------
