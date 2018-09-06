/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SNX_tcp.c
	\brief		TCP
	\author		Hann Chiu
	\version	10
	\date		2015/12/16
	\copyright	Copyright(C) 2015 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdlib.h>
#include "SNX_tcp.h"
#include "lwip/memp.h"
#include "lwip/tcp_impl.h"

#define MAX_CONN_NUM		8

static void SNX_tcpSend_Thread(void const *argument);
const char * const snx_tcp_state_str[] = 
{
  "CLOSED",
  "LISTEN",
  "SYN_SENT",
  "SYN_RCVD",
  "ESTABLISHED",
  "FIN_WAIT_1",
  "FIN_WAIT_2",
  "CLOSE_WAIT",
  "CLOSING",
  "LAST_ACK",
  "TIME_WAIT"
};
snx_tcp_poll tpcb_poll[MAX_CONN_NUM];
osMessageQId xTcpSendQueue;
snx_tcp_sem_poll tpcb_sem_poll[SNX_TCP_SEM_POLL_SZ];
uint8_t ubSnx_tcp_sem_index;
//------------------------------------------------------------------------------
void SNX_tcp_init(void)
{
	uint8_t ubIdx;

	ubSnx_tcp_sem_index = 0;
	for(ubIdx = 0; ubIdx < SNX_TCP_SEM_POLL_SZ; ubIdx++)
	{
		tpcb_sem_poll[ubIdx].xTcpPollId = (void *)NULL;
		tpcb_sem_poll[ubIdx].iSignals	= 0;
	}
	for(ubIdx = 0; ubIdx < MAX_CONN_NUM; ubIdx++)
	{
		tpcb_poll[ubIdx].tpcb = NULL;
		tpcb_poll[ubIdx].cb = NULL;
	}
	osMessageQDef(SnxTcpQ, SNX_TCP_SEND_QUEUE_SZ, sizeof(tcp_struct_arg));
	xTcpSendQueue = osMessageCreate(osMessageQ(SnxTcpQ), NULL);
	osThreadDef(SnxTcpThread, SNX_tcpSend_Thread, osPriorityNormal, 1, 2048);
	osThreadCreate(osThread(SnxTcpThread), NULL);
}
//------------------------------------------------------------------------------
struct tcp_pcb *SNX_tcp_bind(uint16_t port, pvTCP_recv_cb recv_cb)
{
	struct tcp_pcb *server_tpcb;
	tcp_struct_arg *server_targ;

	if((server_tpcb = tcp_new()) == NULL)
		return NULL;

	if(tcp_bind(server_tpcb, IP_ADDR_ANY, port) != ERR_OK)
	{
		tcp_close(server_tpcb);
		return NULL;
	}

	server_tpcb = tcp_listen(server_tpcb);
	tcp_accept(server_tpcb, SNX_tcp_accept);
	server_targ = (tcp_struct_arg *)mem_malloc(sizeof(tcp_struct_arg));
	if(server_targ == NULL)
	{
		tcp_close(server_tpcb);
		return NULL;	
	}
	memset(server_targ, 0, sizeof(tcp_struct_arg));
	server_targ->pcb = server_tpcb;
	server_targ->recv_cb = recv_cb;
	tcp_arg(server_tpcb, server_targ);	

	return server_tpcb;
}
//------------------------------------------------------------------------------
struct tcp_pcb *SNX_tcp_connect(ip_addr_t *ipaddr, uint16_t port, pvTCP_recv_cb recv_cb)
{
	err_t err;
	struct tcp_pcb *client_tpcb;
	tcp_struct_arg *client_targ;

	if((client_tpcb = tcp_new()) == NULL)
	{
		printf("(1): ");
		return NULL;
	}

	if((err = tcp_connect(client_tpcb, ipaddr, port, SNX_tcp_connected)) != ERR_OK)
	{
		tcp_close(client_tpcb);
		printf("(%d): ", err);
		return NULL;
	}

	client_targ = (tcp_struct_arg *)mem_malloc(sizeof(tcp_struct_arg));
	if(client_targ == NULL)
	{
		tcp_close(client_tpcb);
		printf("(3): ");
		return NULL;
	}
	memset(client_targ, 0, sizeof(tcp_struct_arg));
	client_targ->recv_cb = recv_cb;

	client_targ->state = SNX_NOT_CONNECTED;
	tcp_arg(client_tpcb, client_targ);
	tcp_recv(client_tpcb, SNX_tcp_client_recv);
	tcp_err(client_tpcb, SNX_tcp_errf);

	return client_tpcb;
}
//------------------------------------------------------------------------------
static err_t SNX_tcp_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	tcp_struct_arg *client;

	client = (tcp_struct_arg *)arg;
	switch(err)
	{
		case ERR_OK:
			if(client != NULL)
			{
				client->pcb   = tpcb;
				client->state = SNX_CONNECTED;
				client->p_tx  = NULL;
				tcp_sent(tpcb, SNX_tcp_client_sent_cb);
				PRT_ERRDEG("TCP connect established...");
			}
			else
			{
				err = ERR_VAL;
			}
			break;
		case ERR_MEM:
			SNX_tcp_connection_close(tpcb);
			break;
		default:
			break;
	}

	return err;
}
//------------------------------------------------------------------------------
static err_t SNX_tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	tcp_struct_arg *client;
	err_t recv_err = ERR_OK;

	client = (tcp_struct_arg *)arg;
	if((client == NULL) && (p != NULL))
	{
		pbuf_free(p);
	}
	else if(err != ERR_OK)
	{
		if(p != NULL)
			pbuf_free(p);
		recv_err = err;
	}
	else if(p == NULL)
	{
		PRT_ERRDEG("!!!===>p is NULL\n");
		client->state = SNX_CLOSING;
		SNX_tcp_connection_close(tpcb);
	}
	else if(client->state == SNX_CONNECTED)
	{
		if(client->recv_cb)
		{
			struct pbuf *q;

			for(q = p; q != NULL; q = q->next)
			{
				if(q->payload)
					client->recv_cb((uint8_t *)q->payload, q->len, client->cbarg);
			}
		}
		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);
	}
	else
	{
		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);
	}

	return recv_err;
}
//------------------------------------------------------------------------------
err_t SNX_tcp_client_sent_cb(void *arg, struct tcp_pcb *pcb, uint16_t len)
{
	osStatus xStatus;
	tcp_struct_arg *tcparg;
	pvTCP_poll_cb poll_cb;
	uint16_t uwSnd_len, uwSndQue_len;
	uint8_t ubSendQ = 1;

	tcparg = (tcp_struct_arg *)arg;
	if(tcparg)
	{
		uwSnd_len = tcp_sndbuf(pcb);
		uwSndQue_len = tcp_sndqueuelen(pcb);
		PRT_DEG("C=> sent: %d, buf: %d", len, uwSnd_len);
		PRT_DEG("");
		poll_cb = SNX_tcp_find_match_poll_cb(pcb);
		if(poll_cb != NULL)
			poll_cb(pcb);
		if(uwSnd_len)
		{
			ubSendQ = SNX_tcp_check_data_status(tcparg);
			if((ubSendQ) && (uwSndQue_len < (uwSnd_len / TCP_MSS)))
			{
				xStatus = osMessagePut(xTcpSendQueue, tcparg, 0);
				if(xStatus != osOK)
					PRT_ERR("TCP Client Send Q Full");
			}
		}
	}

	return ERR_OK;
}
//------------------------------------------------------------------------------
err_t SNX_tcp_send(struct tcp_pcb *tpcb, const void *ptr, uint16_t length, uint8_t apiflags, TCP_OUTPUT_FLAG tcpflag)
{
	err_t wr_err = ERR_OK;

	wr_err = tcp_write(tpcb, ptr, length, apiflags);
	if(wr_err == ERR_OK)
	{
		if(tcpflag == TCP_OUT_IMM)
			tcp_output(tpcb);
	}

	return wr_err;
}
//------------------------------------------------------------------------------
err_t SNX_tcp_server_sent_cb(void *arg, struct tcp_pcb *pcb, uint16_t len)
{
	osStatus xStatus;
	tcp_struct_arg *tcparg;
	uint16_t uwSnd_len, uwSndQue_len;
	uint8_t ubSendQ = 1;

	tcparg = (tcp_struct_arg *)arg;
	if(tcparg)
	{
		uwSnd_len = tcp_sndbuf(pcb);
		uwSndQue_len = tcp_sndqueuelen(pcb);
		PRT_DEG("S=> sent: %d, buf: %d", len, uwSnd_len);
		PRT_DEG("");
		if(uwSnd_len)
		{
			ubSendQ = SNX_tcp_check_data_status(tcparg);
			if((ubSendQ) && (uwSndQue_len < (uwSnd_len / TCP_MSS)))
			{
				xStatus = osMessagePut(xTcpSendQueue, tcparg, 0);
				if(xStatus != osOK)
					PRT_ERR("TCP Server Send Q Full");
			}
		}
	}

	return ERR_OK;
}
//------------------------------------------------------------------------------
static err_t SNX_tcp_accept(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	tcp_struct_arg *server, *lis_arg;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	lis_arg = (tcp_struct_arg *)arg;
	tcp_setprio(tpcb, TCP_PRIO_MIN);
	switch(err)
	{
		case ERR_OK:
			server = (tcp_struct_arg *)mem_malloc(sizeof(tcp_struct_arg));
			if(server != NULL)
			{
				memset(server, 0, sizeof(tcp_struct_arg));
				server->state = SNX_ACCEPTED;
				server->pcb = tpcb;
				server->p_tx = NULL;
				server->cbarg = server;
				lis_arg->pcb = tpcb;
				lis_arg->cbarg = server;
				server->recv_cb = lis_arg->recv_cb;
				tcp_sent(tpcb, SNX_tcp_server_sent_cb);
				tcp_recv(tpcb, SNX_tcp_server_recv);
				tcp_err(tpcb, SNX_tcp_errf);
				tcp_arg(tpcb, server);
				break;
			}
			else
			{
				err = ERR_VAL;
			}			
		case ERR_MEM:
			SNX_tcp_connection_close(tpcb);
			break;
		default:
			break;
	}

	return err;  
}
//------------------------------------------------------------------------------
static err_t SNX_tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	tcp_struct_arg *server;
	err_t recv_err = ERR_OK;

	server = (tcp_struct_arg *)arg;
	if((server == NULL) && (p != NULL))
	{
		pbuf_free(p);
	}
	else if(err != ERR_OK)
	{
		if(p != NULL)
			pbuf_free(p);
		recv_err = err;
	}
	else if(p == NULL)
	{
		server->state = SNX_CLOSING;
		SNX_tcp_connection_close(tpcb);
	}
	else if(err != ERR_OK)
	{
		server->p_tx = NULL;
		pbuf_free(p);
	}
	else if((server->state == SNX_ACCEPTED) || (server->state == SNX_RECEIVED))
	{
		uint8_t *pRecv;	

		pRecv = (uint8_t *)p->payload;
		tcp_recved(tpcb, p->tot_len);	
		if(server->recv_cb)
			server->recv_cb(pRecv, p->tot_len, server->cbarg);
		pbuf_free(p);		
	}
	else  
	{  
		tcp_recved(tpcb, p->tot_len);
		pbuf_free(p);
	}

	return recv_err;
}
//------------------------------------------------------------------------------
static void SNX_tcp_errf(void *arg, err_t err)
{
	tcp_struct_arg *targ;

	targ = (tcp_struct_arg *)arg;
	if((!targ) || (err == ERR_OK))
		return;

	PRT_ERR("tcp_errf be called...\n");
	switch(err)
	{
		case ERR_MEM:                                            /* Out of memory error.     */  
			PRT_ERR("\r\n ERR_MEM\r\n");
			break;    
		case ERR_BUF:                                            /* Buffer error.            */  
			PRT_ERR("\r\n ERR_BUF\r\n");
			break;  
		case  ERR_TIMEOUT:                                       /* Timeout.                 */  
			PRT_ERR("\r\n ERR_TIMEOUT\r\n");
			break;  
		case ERR_RTE:                                            /* Routing problem.         */        
			PRT_ERR("\r\n ERR_RTE\r\n");
			break;  
	   case ERR_ISCONN:                                          /* Already connected.       */  
			PRT_ERR("\r\n ERR_ISCONN\r\n");
			break;  
		case ERR_ABRT:                                           /* Connection aborted.      */
			PRT_ERR("\r\n ERR_ABRT\r\n");
			break;  
		case ERR_RST:                                            /* Connection reset.        */       
			PRT_ERR("\r\n ERR_RST\r\n");
			break;  
		case ERR_CONN:                                           /* Not connected.           */  
			PRT_ERR("\r\n ERR_CONN\r\n");
			break;  
		case ERR_CLSD:                                           /* Connection closed.       */  
			PRT_ERR("\r\n ERR_CLSD\r\n");
			break;  
		case ERR_VAL:                                            /* Illegal value.           */  
			PRT_ERR("\r\n ERR_VAL\r\n");
			return;  
		case ERR_ARG:                                            /* Illegal argument.        */  
			PRT_ERR("\r\n ERR_ARG\r\n");
			return;  
		case ERR_USE:                                            /* Address in use.          */  
			PRT_ERR("\r\n ERR_USE\r\n");
			return;   
		case ERR_IF:                                             /* Low-level netif error    */  
			PRT_ERR("\r\n ERR_IF\r\n");
			break;  
		case ERR_INPROGRESS:                                     /* Operation in progress    */  
			PRT_ERR("\r\n ERR_INPROGRESS\r\n");
			break;
	}
	if(targ)
		targ->state = SNX_CLOSING;
}
//------------------------------------------------------------------------------
void SNX_tcp_connection_close(struct tcp_pcb *tpcb)
{
	err_t err = ERR_OK;
	tcp_struct_arg *targ;

	targ = (tcp_struct_arg *)(tpcb->callback_arg);
	printf("   tcp_close(%d_0x%p_0x%p)->  ", tpcb->state, tpcb, targ);
	if(targ != NULL)
		mem_free(targ);
	if(tpcb != NULL)
	{
//		if(tpcb->state != LISTEN)
//			tcp_recv(tpcb, NULL);
//		tcp_arg(tpcb, NULL);
		if((tpcb->recv != NULL) && (tpcb->state != LISTEN))
			tpcb->recv = NULL;
		if(tpcb->callback_arg != NULL)
			tpcb->callback_arg = NULL;
		err = tcp_close(tpcb);
		tpcb = NULL;
		printf((err == ERR_OK)?"   <-\n":"  ...err\n");
	}
}
//------------------------------------------------------------------------------
void SNX_tcp_connection_free(struct tcp_pcb *tpcb, uint8_t reset)
{
	tcp_struct_arg *targ;

	targ = (tcp_struct_arg *)(tpcb->callback_arg);
	if(targ != NULL)
		mem_free(targ);
	if(tpcb != NULL)
	{
//		if(tpcb->state != LISTEN)
//			tcp_recv(tpcb, NULL);
//		tcp_arg(tpcb, NULL);
		if((tpcb->recv != NULL) && (tpcb->state != LISTEN))
			tpcb->recv = NULL;
		if(tpcb->callback_arg != NULL)
			tpcb->callback_arg = NULL;
		printf("   tcp_close(%d)->", tpcb->state);
		tcp_abandon(tpcb, reset);
		tpcb = NULL;
		printf(" <-\r\n");
	}
}
//------------------------------------------------------------------------------
void SNX_tcp_free_all_connected(void)
{
	struct tcp_pcb ** const tcp_pcb_rmv_lists[] = {&tcp_active_pcbs, &tcp_tw_pcbs};
	struct tcp_pcb *pcb, *pcb_1st;
	uint8_t i;

	for(i = 0; i < 2; i++)
	{
		pcb_1st = *tcp_pcb_rmv_lists[i];
		for(pcb = *tcp_pcb_rmv_lists[i]; pcb != NULL; pcb = pcb->next)
		{
			SNX_tcp_connection_free(pcb, tcp_free_without_reset);
			if(pcb->next == pcb_1st)
				break;
		}
	}
	*(&tcp_tmp_pcb) = (void *)NULL;
	tcp_port_init();
	tcp_set_tmr_flag(tmr_normal);
	memp_reinit_tcp_pcb();
	mem_init();
	PRT_DEG("tcp remove...");
}
//------------------------------------------------------------------------------
static void SNX_tcpSend_Thread(void const *argument)
{
	osStatus xStatus;
	tcp_struct_arg tcpQueue, *tcp_info;

	while(1)
	{
		xStatus = osMessageGet(xTcpSendQueue, &tcpQueue, osWaitForever);
		if(xStatus != osEventMessage)
		{
			PRT_ERR("TCP Send, Q empty");
		}
		else
		{
			if((tcpQueue.pcb->state != ESTABLISHED) || ((tcpQueue.state != SNX_CONNECTED) && (tcpQueue.state != SNX_ACCEPTED)))
			{
				PRT_DEG("tcp state: %d", tcpQueue.pcb->state);
				continue;
			}
			tcp_info = tcpQueue.pcb->callback_arg;
			if(tcp_info == NULL)
			{
				PRT_DEG("tcp info NULL:0x%p", tcp_info);
				continue;
			}
			if(((tcp_info->state == SNX_CONNECTED) || ((tcp_info->state == SNX_ACCEPTED))))
			{
				uint16_t uwSend_len = 0;

				tcp_info->uwSnd_len = tcp_sndbuf(tcp_info->pcb);
				tcp_info->uwSnd_Quelen = tcp_sndqueuelen(tcp_info->pcb);
				uwSend_len = (tcp_info->ulRemind_len > tcp_info->uwSnd_len)?tcp_info->uwSnd_len:tcp_info->ulRemind_len;
				if((tcp_info->payload != NULL) && (tcp_info->ulRemind_len < (200 * 1024)) &&
				   (tcp_info->uwSnd_len >= uwSend_len) && ((tcp_info->uwSnd_Quelen < (tcp_info->uwSnd_len / TCP_MSS))))
				{
					err_t rc = ERR_OK;

					rc = SNX_tcp_send(tcp_info->pcb, (uint8_t *)tcp_info->payload + tcp_info->ulRemind_pos, uwSend_len, TCP_WRITE_FLAG_COPY, TCP_OUT_TASK);
					if(rc == ERR_OK)
					{
						tcp_info->ulRemind_pos += uwSend_len;
						tcp_info->ulRemind_len -= uwSend_len;
						SNX_tcp_check_data_status(tcp_info);
					}
					else
					{
						PRT_DEG("tcp send fail: %d_%d_%d", rc, uwSend_len, tcp_info->uwSnd_len);
					}
				}
				else
				{
					PRT_DEG("tcp stop send: 0x%p_%d_%d_%d", tcp_info->payload, tcp_info->ulRemind_len, tcp_info->uwSnd_len, tcp_info->uwSnd_Quelen);
					SNX_tcp_ClearSendQueue();
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
uint8_t SNX_tcp_check_data_status(tcp_struct_arg *tcparg)
{
	uint8_t ubStop = 1;
	uint8_t ubIdx  = 0;

	if(!tcparg->ulRemind_len)
	{
		if(tcparg->ubRTY_Num)
		{
			++tcparg->ubRTY_Cnt;
			tcparg->ulRemind_len = (tcparg->ubRTY_Num == tcparg->ubRTY_Cnt)?0:tcparg->ulRemind_pos;
			tcparg->ulRemind_pos = 0;
		}
		if((!tcparg->ulRemind_len) && (tcparg->xTcpId != NULL))
		{
			ubStop 				= 0;
			tcparg->ubRTY_Cnt 	= 0;
			tcparg->ubRTY_Num 	= 0;
			for(ubIdx = 0; ubIdx < ubSnx_tcp_sem_index; ubIdx++)
			{
				if(tcparg->xTcpId == tpcb_sem_poll[ubIdx].xTcpPollId)
					break;
			}
			if(ubIdx == ubSnx_tcp_sem_index)
				return ubStop;
			osSignalSet(tcparg->xTcpId, tcparg->iWaitSignals);
		}
	}
	return ubStop;
}
//------------------------------------------------------------------------------
osMessageQId SNX_tcpGet_SendQHandle(void)
{
	return xTcpSendQueue;
}
//------------------------------------------------------------------------------
void SNX_tcp_ClearSendQueue(void)
{
	if(xTcpSendQueue != NULL)
		osMessageReset(xTcpSendQueue);
}
//------------------------------------------------------------------------------
pvTCP_poll_cb SNX_tcp_find_match_poll_cb(struct tcp_pcb *pcb)
{
	uint8_t ubIdx;

	for(ubIdx = 0; ubIdx < MAX_CONN_NUM; ubIdx++)
	{
		if(tpcb_poll[ubIdx].tpcb == pcb)
			return tpcb_poll[ubIdx].cb;
	}
	return NULL;
}
//------------------------------------------------------------------------------
void SNX_tcp_connection_poll(snx_tcp_poll_act act, struct tcp_pcb *pcb, pvTCP_poll_cb cb)
{
	uint8_t ubIdx;

	for(ubIdx = 0; ubIdx < MAX_CONN_NUM; ubIdx++)
	{
		if((act == add_act) && (tpcb_poll[ubIdx].tpcb == NULL) && (tpcb_poll[ubIdx].cb == NULL))
		{
			tpcb_poll[ubIdx].tpcb = pcb;
			tpcb_poll[ubIdx].cb = cb;
			break;
		}
		else if((act == del_act) && (tpcb_poll[ubIdx].tpcb == pcb))
		{
			tpcb_poll[ubIdx].tpcb = NULL;
			tpcb_poll[ubIdx].cb = NULL;
			break;
		}
	}
}
//------------------------------------------------------------------------------
void snx_tcp_add_sem_poll(void *handle, int32_t signals)
{
	if(ubSnx_tcp_sem_index > SNX_TCP_SEM_POLL_SZ)
		return;

	tpcb_sem_poll[ubSnx_tcp_sem_index++].xTcpPollId = (osThreadId)handle;
	tpcb_sem_poll[ubSnx_tcp_sem_index++].iSignals 	= signals;
}
//------------------------------------------------------------------------------
uint8_t snx_tcp_debug_print_pcbs(void)
{
	struct tcp_pcb *pcb;
	uint8_t pcb_loop = 0;

	printf("Active PCB states: \n");
	for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
	{
		printf("[0x%p]Local port %"U16_F", Remote port %"U16_F" State: %s\n", pcb,
					pcb->local_port, pcb->remote_port, snx_tcp_state_str[pcb->state]);
		if(++pcb_loop > 64)
			return 0;
	}
	pcb_loop = 0;
	printf("Listen PCB states: \n");
	for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next)
	{
		printf("Local port %"U16_F", Remote port %"U16_F" State: %s\n",
					 pcb->local_port, pcb->remote_port, snx_tcp_state_str[pcb->state]);
		if(++pcb_loop > 64)
			return 0;
	}
	return 1;
}
