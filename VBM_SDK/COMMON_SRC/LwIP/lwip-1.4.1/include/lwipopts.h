/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include "WiFi_API.h"

#define LWIP_NETCONN 					(0)
#define LWIP_SOCKET 					(0)

/* Minimal changes to opt.h required */
#define MEM_SIZE                        (68*1024L)
#define TCPIP_MBOX_SIZE                 (128)
#define DEFAULT_TCP_RECVMBOX_SIZE       (23360)
#define DEFAULT_UDP_RECVMBOX_SIZE       (23360)
#define DEFAULT_RAW_RECVMBOX_SIZE 		(23360)
#define DEFAULT_ACCEPTMBOX_SIZE         (23360)

#define LWIP_TCP						(1)
#define LWIP_UDP						(1)
#define LWIP_DHCP 						(1)
#define LWIP_DNS						(1)
#define PING_IP_ADDRRESS(ipaddr)		(ip4_addr_set_u32(ipaddr, ipaddr_addr("202.108.22.5")))
#define DNS_SERVER_ADDRESS(ipaddr)      (ip4_addr_set_u32(ipaddr, ipaddr_addr("168.95.1.1")))

#define PING_SYS_TIMEOUT				(1000)
#define PING_DEBUG						(LWIP_DBG_ON)

#define LWIP_PROVIDE_ERRNO				(1)

#ifndef PBUF_POOL_TX_SIZE
#define PBUF_POOL_TX_SIZE               (7)
#endif

#ifndef PBUF_POOL_RX_SIZE
#define PBUF_POOL_RX_SIZE               (7)
#endif

#if PBUF_POOL_TX_SIZE > 2
#ifndef IP_REASS_MAX_PBUFS
#define IP_REASS_MAX_PBUFS              (PBUF_POOL_TX_SIZE - 2)
#endif
#else
#define IP_REASS_MAX_PBUFS              (0)
#define IP_REASSEMBLY                   (0)
#endif

#define PBUF_POOL_BUFSIZE              (LWIP_MEM_ALIGN_SIZE(WIFI_LINK_MTU) + LWIP_MEM_ALIGN_SIZE(sizeof(struct pbuf)) + 1)

#define PBUF_LINK_HLEN                 (WIFI_PHYSICAL_HEADER)

#define ETH_PAD_SIZE                   (0)

/**
 * The maximum size of the wwd_buffer_header_t structure (i.e. largest bus implementation)
 */
#define MAX_BUS_HEADER_LENGTH (12)

/**
 * The maximum size of the SDPCM + BDC header, including offsets and reserved space
 * 12 bytes - SDPCM header
 * 2 bytes  - Extra offset for SDPCM headers that come as 14 bytes
 * 4 bytes  - BDC header
 */
#define MAX_SDPCM_HEADER_LENGTH (18)

/**
 * The maximum space in bytes required for headers in front of the Ethernet header.
 * This definition allows WICED to use a pre-built bus-generic network stack library regardless of choice of bus.
 * Note: adjust accordingly if a new bus is added.
 */
#define WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX ( MAX_BUS_HEADER_LENGTH + MAX_SDPCM_HEADER_LENGTH )

#define SUB_ETHERNET_HEADER_SPACE      (WICED_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX)

/* ------------------------ Memory options -------------------------------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
		lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
		byte alignment -> define MEM_ALIGNMENT to 2. 
*/
#define MEM_ALIGNMENT				   (4)

#define LWIP_DEBUG

#define MEM_DEBUG                      (LWIP_DBG_OFF)
#define MEMP_DEBUG                     (LWIP_DBG_OFF)
#define PBUF_DEBUG                     (LWIP_DBG_OFF)
#define TCPIP_DEBUG                    (LWIP_DBG_OFF)
#define NETIF_DEBUG                    (LWIP_DBG_OFF)
#define IP_DEBUG                       (LWIP_DBG_OFF)
#define IP_REASS_DEBUG                 (LWIP_DBG_OFF)
#define RAW_DEBUG                      (LWIP_DBG_OFF)
#define UDP_DEBUG                      (LWIP_DBG_OFF)
#define TCP_DEBUG                      (LWIP_DBG_ON)
#define TCP_INPUT_DEBUG                (LWIP_DBG_OFF)
#define TCP_OUTPUT_DEBUG               (LWIP_DBG_OFF)
#define TCP_RTO_DEBUG                  (LWIP_DBG_OFF)
#define TCP_CWND_DEBUG                 (LWIP_DBG_OFF)
#define TCP_WND_DEBUG                  (LWIP_DBG_OFF)
#define TCP_FR_DEBUG                   (LWIP_DBG_OFF)
#define TCP_QLEN_DEBUG                 (LWIP_DBG_OFF)
#define TCP_RST_DEBUG                  (LWIP_DBG_OFF)
#define SYS_DEBUG                      (LWIP_DBG_OFF)
#define TIMERS_DEBUG                   (LWIP_DBG_OFF)
#define SLIP_DEBUG                     (LWIP_DBG_OFF)
#define DHCP_DEBUG                     (LWIP_DBG_OFF)
#define LWIP_DBG_TYPES_ON              (LWIP_DBG_OFF)   /* (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT) */

#endif /* __LWIPOPTS_H__ */
