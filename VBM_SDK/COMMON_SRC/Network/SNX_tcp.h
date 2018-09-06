/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SNX_tcp.h
	\brief		TCP header
	\author		Hann Chiu
	\version	10
	\date		2015/12/16
	\copyright	Copyright(C) 2015 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __SNX_TCP_H__
#define __SNX_TCP_H__

#include "SNX_network.h"

#define	SNX_TCP_SEND_QUEUE_SZ	100
#define SNX_TCP_SEM_POLL_SZ		5

typedef void (*pvTCP_recv_cb)(void *, uint32_t, void *);
typedef void (*pvTCP_poll_cb)(struct tcp_pcb *);

typedef enum
{
	SNX_tcp_server_mode,
	SNX_tcp_client_mode
}tcp_mode;

typedef enum
{
	TCP_OUT_TASK,
	TCP_OUT_IMM,
}TCP_OUTPUT_FLAG;

typedef enum
{
	add_act,
	del_act
}snx_tcp_poll_act;

typedef enum
{
	tcp_free_without_reset = 0,
	tcp_free_with_reset,
}snx_tcp_free_type;

typedef struct
{
	struct tcp_pcb *tpcb;
	pvTCP_poll_cb cb;
}snx_tcp_poll;

typedef struct
{
	osThreadId xTcpPollId;
	int32_t iSignals;
}snx_tcp_sem_poll;

typedef struct
{
	snx_ntk_tcpudp_connsts_t state;
	struct tcp_pcb *pcb;
	struct pbuf *p_tx;
	pvTCP_recv_cb recv_cb;
	void *cbarg;
	uint16_t uwSnd_len;
	uint16_t uwSnd_Quelen;
	void *payload;
	uint32_t ulRemind_len;
	uint32_t ulRemind_pos;
	uint8_t ubRTY_Num;
	uint8_t ubRTY_Cnt;
	int32_t iWaitSignals;
	osThreadId xTcpId;
}tcp_struct_arg;

void SNX_tcp_init(void);
struct tcp_pcb *SNX_tcp_bind(uint16_t port, pvTCP_recv_cb recv_cb);
struct tcp_pcb * SNX_tcp_connect(ip_addr_t *ipaddr, uint16_t port, pvTCP_recv_cb recv_cb);
static err_t SNX_tcp_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t SNX_tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t SNX_tcp_send(struct tcp_pcb *tpcb, const void* ptr, uint16_t length, uint8_t apiflags, TCP_OUTPUT_FLAG tcpflag);
err_t SNX_tcp_server_sent_cb(void *arg, struct tcp_pcb *pcb, uint16_t len);
err_t SNX_tcp_client_sent_cb(void *arg, struct tcp_pcb *pcb, uint16_t len);
static err_t SNX_tcp_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t SNX_tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void SNX_tcp_errf(void *arg, err_t err);
void SNX_tcp_connection_close(struct tcp_pcb *tpcb);
uint8_t SNX_tcp_check_data_status(tcp_struct_arg *tcparg);
osMessageQId SNX_tcpGet_SendQHandle(void);
void SNX_tcp_ClearSendQueue(void);
pvTCP_poll_cb SNX_tcp_find_match_poll_cb(struct tcp_pcb *pcb);
void SNX_tcp_connection_free(struct tcp_pcb *tpcb, uint8_t reset);
void SNX_tcp_free_all_connected(void);
void SNX_tcp_connection_poll(snx_tcp_poll_act act, struct tcp_pcb *pcb, pvTCP_poll_cb cb);
void snx_tcp_add_sem_poll(void *handle, int32_t signals);
uint8_t snx_tcp_debug_print_pcbs(void);

#endif	//!__SNX_TCP_CLIENT_H
