/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SNX_udp.h
	\brief		UDP header
	\author		Hann Chiu
	\version	2
	\date		2015/01/22
	\copyright	Copyright(C) 2014 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __SNX_UDP_H
#define __SNX_UDP_H

#include <stdint.h>
#include "SNX_network.h"

#define SNX_UDP_SEND_QUEUE_SZ	60
#define MAX_UDP_PAYLOAD_SIZE    ( WIFI_PAYLOAD_MTU - UDP_HLEN - IP_HLEN - 20 - WIFI_PHYSICAL_HEADER )

typedef void (*pvUDP_recv_cb)(void *);

typedef enum
{
	udp_task_close,
	udp_task_open,
}udp_status_t;

typedef enum 
{
	SNX_udp_server_mode,
	SNX_udp_client_mode
}udp_mode;

typedef struct
{
	snx_ntk_tcpudp_connsts_t state;
	struct udp_pcb *pcb;
	struct pbuf *p_tx;
	pvUDP_recv_cb recv_cb;
	void *cbarg;
	struct ip_addr ip;
	uint16_t port;
	void *payload;
	uint32_t ulRemind_len;
	uint32_t ulRemind_pos;
	uint8_t ubRTY_Num;
	uint8_t ubRTY_Cnt;
	int32_t iWaitSignals;
	osThreadId xUdpId;
}udp_struct_arg;

void SNX_udp_init(void);
struct udp_pcb *SNX_udp_bind(uint16_t port, udp_recv_fn recv_fn);
struct udp_pcb *SNX_udp_connect(ip_addr_t *ipaddr, uint16_t port, pvUDP_recv_cb recv_cb);	
void SNX_udp_server_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
							struct ip_addr *addr, u16_t port);
void SNX_udp_client_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
							struct ip_addr *addr, u16_t port);
void SNX_udp_connect_close(struct udp_pcb *upcb);
osMessageQId SNX_udpGet_SendQHandle(void);
void SNX_udp_ClearSendQueue(void);
void SNX_udp_enable(void);
void SNX_udp_disable(void);

#endif
