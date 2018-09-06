/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SNX_udp.c
	\brief		UDP
	\author		Hann Chiu
	\version	2
	\date		2015/01/22
	\copyright	Copyright(C) 2014 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include "SNX_udp.h"

static void SNX_udpSend_Thread(void const *argument);
osThreadId xUdpThread;
osMessageQId xUdpSendQueue;
udp_status_t Snx_Udp_Sts;
//------------------------------------------------------------------------------
void SNX_udp_init(void)
{
	Snx_Udp_Sts = udp_task_open;
	
	osMessageQDef(SnxUdpQ, SNX_UDP_SEND_QUEUE_SZ, sizeof(udp_struct_arg));
	xUdpSendQueue = osMessageCreate(osMessageQ(SnxUdpQ), NULL);
	osThreadDef(SnxUdpThread, SNX_udpSend_Thread, osPriorityNormal, 1, 2048);
	xUdpThread = osThreadCreate(osThread(SnxUdpThread), NULL);
}
//------------------------------------------------------------------------------
struct udp_pcb *SNX_udp_bind(uint16_t port, udp_recv_fn recv_fn)
{
	struct udp_pcb *server_upcb;
	udp_struct_arg *server_uarg;

	server_upcb = udp_new();
	if(server_upcb == NULL)
		return NULL;

	if(udp_bind(server_upcb, IP_ADDR_ANY, port) != ERR_OK)
		return NULL;

	server_uarg = (udp_struct_arg *)mem_malloc(sizeof(udp_struct_arg));
	if(server_uarg == NULL)
	{
		SNX_udp_connect_close(server_upcb);
		return NULL;
	}
	memset(server_uarg, 0, sizeof(udp_struct_arg));
	server_uarg->pcb = server_upcb;
	server_uarg->cbarg = server_uarg;
	server_uarg->state = SNX_CONNECTED;
	udp_recv(server_upcb, (recv_fn == NULL)?SNX_udp_server_recv:recv_fn, server_uarg);	

	return server_upcb;
}
//------------------------------------------------------------------------------
struct udp_pcb *SNX_udp_connect(ip_addr_t *ipaddr, uint16_t port, pvUDP_recv_cb recv_cb)
{
	struct udp_pcb *client_upcb;
	udp_struct_arg *client_uarg;

	if((client_upcb = udp_new()) == NULL)
		return NULL;

	if(udp_connect(client_upcb, ipaddr, port) != ERR_OK)
		return NULL;

	client_uarg = (udp_struct_arg *)mem_malloc(sizeof(udp_struct_arg));
	if(client_uarg == NULL)
	{
		SNX_udp_connect_close(client_upcb);
		return NULL;	
	}
	memset(client_uarg, 0, sizeof(udp_struct_arg));
	client_uarg->pcb = client_upcb;
	client_uarg->cbarg = client_uarg;
	client_uarg->recv_cb = recv_cb;
	client_uarg->state = SNX_CONNECTED;
	udp_recv(client_upcb, SNX_udp_client_recv, client_uarg);
		
	return client_upcb;
}
//------------------------------------------------------------------------------
void SNX_udp_server_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
							struct ip_addr *addr, u16_t port)
{
	udp_struct_arg *server_uarg;

	server_uarg = (udp_struct_arg *)arg;	
	if(p == NULL)
	{
		server_uarg->state = SNX_CLOSING;
		SNX_udp_connect_close(upcb);
	}	
	else if(server_uarg->state == SNX_CONNECTED)
	{
		uint8_t *pRecv;	
		
		pRecv = (uint8_t *)p->payload;
		if(server_uarg->recv_cb)
			server_uarg->recv_cb(pRecv);
		pbuf_free(p);
	}
}
//------------------------------------------------------------------------------
void SNX_udp_client_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
							struct ip_addr *addr, u16_t port)
{
	udp_struct_arg *client_uarg;

	client_uarg = (udp_struct_arg *)arg;
	if(p == NULL)
	{
		client_uarg->state = SNX_CLOSING;
		SNX_udp_connect_close(upcb);
	}	
	else if(client_uarg->state == SNX_CONNECTED)
	{		
		uint8_t *pRecv;

		pRecv = (uint8_t *)p->payload;
		if(client_uarg->recv_cb)
			client_uarg->recv_cb(pRecv);
		pbuf_free(p);		
	}
}
//------------------------------------------------------------------------------
static void SNX_udpSend_Thread(void const *argument)
{
	osStatus xStatus;
	udp_struct_arg udpQueue;

	memset(&udpQueue, 0, sizeof(udp_struct_arg));
	while(1)
	{
		xStatus = osMessageGet(xUdpSendQueue, &udpQueue, osWaitForever);
		if(xStatus != osEventMessage)
		{
			PRT_ERR("UDP Send, Q empty");
		}
		else 
		{
			if((udpQueue.state == SNX_CONNECTED) && (udpQueue.pcb != NULL) && 
			   (udpQueue.payload != NULL))
			{
				err_t rc;
				struct pbuf *p;
				uint32_t ulSend_len;

				udpQueue.ulRemind_pos = 0;
				while((udpQueue.ulRemind_len) && (Snx_Udp_Sts == udp_task_open))
				{
					ulSend_len = (udpQueue.ulRemind_len > MAX_UDP_PAYLOAD_SIZE)?MAX_UDP_PAYLOAD_SIZE:udpQueue.ulRemind_len;
					p = pbuf_alloc(PBUF_TRANSPORT, ulSend_len, PBUF_POOL);
					if(p == NULL)
					{
						printf("  x\n");
						memset(&udpQueue, 0, sizeof(udp_struct_arg));
						break;
					}
					#ifdef WIFI_TYPE_SDIO
					p->ref = 0;
					#endif
					memcpy(p->payload, (uint8_t *)udpQueue.payload + udpQueue.ulRemind_pos, p->tot_len);
					rc = udp_sendto(udpQueue.pcb, p, &udpQueue.ip, udpQueue.port);
					if(rc != ERR_OK)
					{
						pbuf_free(p);
						memset(&udpQueue, 0, sizeof(udp_struct_arg));
						break;
					}
					#ifdef WIFI_TYPE_USB
					pbuf_free(p);
					#endif
					udpQueue.ulRemind_pos += ulSend_len;
					udpQueue.ulRemind_len -= ulSend_len;
					if((!udpQueue.ulRemind_len) && (udpQueue.xUdpId))
						osSignalSet(udpQueue.xUdpId, udpQueue.iWaitSignals);
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
void SNX_udp_connect_close(struct udp_pcb *upcb)
{
	if(upcb != NULL)
	{
		udp_struct_arg *uarg;

		uarg = (udp_struct_arg *)upcb->recv_arg;
		if(uarg != NULL)
		{
			uarg->state = SNX_CLOSING;
			uarg->payload = NULL;
			uarg->p_tx = NULL;
			uarg->recv_cb = NULL;
			uarg->pcb = NULL;
			mem_free(uarg);
		}
		udp_remove(upcb);
	}
}
//------------------------------------------------------------------------------
osMessageQId SNX_udpGet_SendQHandle(void)
{
	return xUdpSendQueue;
}
//------------------------------------------------------------------------------
void SNX_udp_ClearSendQueue(void)
{
	osMessageReset(xUdpSendQueue);
}
//------------------------------------------------------------------------------
void SNX_udp_enable(void)
{
	Snx_Udp_Sts = udp_task_open;
}
//------------------------------------------------------------------------------
void SNX_udp_disable(void)
{
	Snx_Udp_Sts = udp_task_close;
}
