/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SNX_Netwrok.c
	\brief		SONiX Network function
	\author		Hanyi Chiu
	\version	1
	\date		2017/01/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "SNX_Network.h"
#include "WIFI_API.h"
#include "SNX_tcp.h"
#include "SNX_udp.h"
#include "dhcps.h"

static void SNX_Network_Thread(void const *argument);
osMessageQId xSnxNtkQueue;
osThreadId xSnxNtkThread;
struct netif snx_if;
struct dhcp snxif_dhcp;
ip_addr_t xAP_IPAddr, xAP_NetMask, xAP_Gateway;
//------------------------------------------------------------------------------
void SNX_Network_Init(void)
{
	WiFi_SetRecv_cbFunc(SNX_Network_RecvFunc);
	WiFi_SetConnected_cbFunc(SNX_Network_Start);
	WiFi_SetDisConnect_cbFunc(SNX_Network_Stop);
	SNX_Network_WiFiConfig();
	SNX_Network_LwipConfig();
	SNX_tcp_init();
	SNX_udp_init();
	osMessageQDef(NtkQ, SNX_NTK_QUEUE_SZ, uint16_t);
	xSnxNtkQueue = osMessageCreate(osMessageQ(NtkQ), NULL);
	osThreadDef(NtkThread, SNX_Network_Thread, osPriorityNormal, 1, 1024);
	xSnxNtkThread = osThreadCreate(osThread(NtkThread), NULL);
}
//------------------------------------------------------------------------------
void SNX_Network_Start(void)
{
	/* Set MAC hardware address */
	WiFi_Get_MACAddr(snx_if.hwaddr);
	SNX_Network_SetState(WIFI_LINKED);
}
//------------------------------------------------------------------------------
void SNX_Network_Stop(void)
{
	SNX_Network_SetState(WIFI_DISCONNECT);
}
//------------------------------------------------------------------------------
void SNX_Network_WiFiConfig(void)
{
#define CONFIG_AP_SSID       "510PF_AP"
#define CONFIG_AP_SSID_LEN	 8
#define CONFIG_AP_CHANNEL    8
#define CONFIG_AP_SECURITY   WIFI_SECURITY_WPA2_AES_PSK
#define CONFIG_AP_PASSPHRASE "bbbbbbbb"
#define CLIENT_AP_SSID       "TOTOLINK_6CB4E1"
#define CLIENT_AP_SSID_LEN	 15
#define CLIENT_AP_PASSPHRASE "bbbbbbbb"
#define CLIENT_AP_SECURITY   WIFI_SECURITY_OPEN
	WIFI_ConfigDct_t tWiFi_Config;
	WIFI_Interface_t tWiFi_Interface = WiFi_GetInterface();

	tWiFi_Config.SSID.len = (tWiFi_Interface == WIFI_AP_INTERFACE)?CONFIG_AP_SSID_LEN:CLIENT_AP_SSID_LEN;
	memcpy((char *)tWiFi_Config.SSID.val, (tWiFi_Interface == WIFI_AP_INTERFACE)?CONFIG_AP_SSID:CLIENT_AP_SSID, tWiFi_Config.SSID.len);
	tWiFi_Config.security_key_length = 8;
	memcpy((char *)tWiFi_Config.security_key, (tWiFi_Interface == WIFI_AP_INTERFACE)?CONFIG_AP_PASSPHRASE:CLIENT_AP_PASSPHRASE, tWiFi_Config.security_key_length);
	tWiFi_Config.security = (tWiFi_Interface == WIFI_AP_INTERFACE)?CONFIG_AP_SECURITY:CLIENT_AP_SECURITY;
	WiFi_ConfigDct(tWiFi_Config);
}
//------------------------------------------------------------------------------
void SNX_Network_LwipConfig(void)
{
	uint8_t ubInterface[] = {NETIF_STA_INTERFACE, NETIF_AP_INTERFACE};
	ip_addr_t xIPAddr, xNetMask, xGateway;

	ip_addr_set_zero(&xGateway);
	ip_addr_set_zero(&xIPAddr);
	ip_addr_set_zero(&xNetMask);
	tcpip_init(NULL, NULL);
	/* Set a network interface as the default network interface */
	netif_set_default(netif_add( &snx_if, &xIPAddr, &xNetMask, &xGateway, NULL, ethernetif_init, tcpip_input ), ubInterface[WiFi_GetInterface()]);
	switch(snx_if.interface)
	{
		case NETIF_AP_INTERFACE:
			dhcps_init();
			break;
		case NETIF_STA_INTERFACE:
			dhcp_set_struct(&snx_if, &snxif_dhcp);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void SNX_Network_netif_up(void)
{
	if(NETIF_AP_INTERFACE == snx_if.interface)
	{
		NETIF_AP_IP_ADDR(&snx_if, &xAP_IPAddr);
		NETIF_AP_SUB_NETMASK(&snx_if, &xAP_NetMask);
		NETIF_AP_GW(&snx_if, &xAP_Gateway);
	}
	netif_set_up(&snx_if);
	SNX_udp_enable();
}
//------------------------------------------------------------------------------
void SNX_Network_netif_down(void)
{
	if(NETIF_STA_INTERFACE == snx_if.interface)
		dhcp_stop(&snx_if);
	/* bring the interface down */
	netif_set_down(&snx_if);
	/* remove IP address from interface */
	ip_addr_set(&(snx_if.ip_addr), IP_ADDR_ANY);
	netif_set_gw(&snx_if, IP_ADDR_ANY);
	netif_set_netmask(&snx_if, IP_ADDR_ANY);
	SNX_udp_disable();
	SNX_tcp_ClearSendQueue();
	SNX_udp_ClearSendQueue();
}
//------------------------------------------------------------------------------
static void SNX_Network_Thread(void const *argument)
{
	uint32_t ulNtkWaitTime = osWaitForever;
	uint16_t uwSNX_NTK_State = SNX_NTK_NONE;
	uint16_t uwTimeOut = 0;
	err_t err;

	while(1)
	{
		osMessageGet(xSnxNtkQueue, &uwSNX_NTK_State, ulNtkWaitTime);
		switch(uwSNX_NTK_State)
		{
			case WIFI_LINKED:
				SNX_Network_netif_up();
				if(NETIF_STA_INTERFACE == snx_if.interface)
					SNX_Network_SetState(SNX_NTK_DHCP_START);
				break;
			case WIFI_DISCONNECT:
				SNX_Network_netif_down();
				break;
			case SNX_NTK_DHCP_START:
				err = dhcp_start(&snx_if);
				PRT_DEG("DHCP Start[%d]...", err);
				if(err == ERR_OK)
				{
					ulNtkWaitTime = (SNX_NTK_CYCLE_TIME / 10);
					SNX_Network_SetState(SNX_NTK_CHK_DHCP_STS);					
				}
				break;
			case SNX_NTK_CHK_DHCP_STS:
				if(snx_if.dhcp->state == DHCP_BOUND)
				{
					ulNtkWaitTime = osWaitForever;
					uwSNX_NTK_State = SNX_NTK_NONE;
					break;
				}
				if(++uwTimeOut >= (80000/SNX_NTK_CYCLE_TIME))
				{
					printf("\nDHCP Timeout!!!\n");					
					ulNtkWaitTime = osWaitForever;
					dhcp_stop(&snx_if);
					uwTimeOut = 0;
					SNX_Network_SetState(SNX_NTK_DHCP_START);
				}
				break;
			default:
				break;
		}
	}
}
//------------------------------------------------------------------------------
void SNX_Network_SetState(uint16_t uwState)
{
	osStatus xStatus;

	if((uwState & 0xFF) == WIFI_DISCONNECT)
		osMessageReset(xSnxNtkQueue);
	xStatus = osMessagePut(xSnxNtkQueue, &uwState, osWaitForever);
	if(xStatus != osOK)
		PRT_ERR("SNX NTK Q Full");
}
//------------------------------------------------------------------------------
void SNX_Network_UpdateDHCPS_ARPTable(uint8_t *pmac)
{
	dhpcs_update_arp_entry(pmac);
}
//------------------------------------------------------------------------------
#if (!LWIP_NETCONN && LWIP_DNS)
err_t SNX_Network_GetHostByName(const char *name, ip_addr_t *addr)
{
	struct dns_api_msg msg;
	err_t err;
	sys_sem_t sem;

	if(name == NULL)
	{
		PRT_ERR("invalid name")
		return ERR_ARG;
	}
	if(addr == NULL)
	{
		PRT_ERR("invalid addr")
		return ERR_ARG;
	}

	osSemaphoreDef(NtkDnsSem);
	err = sys_sem_new(&sem, 0, osSemaphore(NtkDnsSem));
	if(err != ERR_OK)
		return err;

	msg.name = name;
	msg.addr = addr;
	msg.err = &err;
	msg.sem = &sem;

	tcpip_callback(do_gethostbyname, &msg);
	sys_sem_wait(&sem);
	sys_sem_free(&sem);

	return err;
}
//------------------------------------------------------------------------------
void do_gethostbyname(void *arg)
{
	struct dns_api_msg *msg = (struct dns_api_msg*)arg;

	*msg->err = dns_gethostbyname(msg->name, msg->addr, do_dns_found, msg);
	if(*msg->err != ERR_INPROGRESS)
		sys_sem_signal(msg->sem);
}
//------------------------------------------------------------------------------
static void do_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct dns_api_msg *msg = (struct dns_api_msg*)arg;

	if(strcmp(msg->name, name))
		printf("DNS response for wrong host name");
	LWIP_UNUSED_ARG(name);
	if(ipaddr == NULL)
	{
		*msg->err = ERR_VAL;
	}
	else
	{
		*msg->err = ERR_OK;
		*msg->addr = *ipaddr;
	}
	sys_sem_signal(msg->sem);
}
#endif		//!<End #if !LWIP_NETCONN
//------------------------------------------------------------------------------
void SNX_Network_RecvFunc(void *pvBuF, uint32_t len, void *pvMemType)
{
	struct netif *netif = NULL;
	struct eth_hdr *ethhdr;
	struct pbuf *p = NULL;
	uint8_t *pMemType = (uint8_t *)(pvMemType);

	netif = &snx_if;
	if(netif == NULL || netif->input == NULL)
		return;

	if(*pMemType == RECV_MEMREALLOC)
	{
		p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
		if(p == NULL)
		{
			PRT_DEG("OOPS ... @@");
			return;
		}
		memcpy(p->payload, (uint8_t *)pvBuF, len);
	}
	else
		p = (struct pbuf *)pvBuF;
	ethhdr = p->payload;
	switch(htons(ethhdr->type))
	{
		case ETH_P_EAPOL:
			pbuf_free(p);
			break;
		/* IP or ARP packet? */
		case ETHTYPE_IP:
		case ETHTYPE_ARP:
	#if PPPOE_SUPPORT
		/* PPPoE packet? */
		case ETHTYPE_PPPOEDISC:
		case ETHTYPE_PPPOE:
	#endif /* PPPOE_SUPPORT */
			/* full packet send to tcpip_thread to process */
			if(netif->input(p, netif) != ERR_OK)
			{
				PRT_DEG("ethernetif_input: IP input error");
				pbuf_free(p);
			}
			break;
		default:
			pbuf_free(p);
			break;
	}
}
