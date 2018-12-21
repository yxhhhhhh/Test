#ifndef __PING_H__
#define __PING_H__

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    LWIP_SOCKET
#endif

#ifndef SNX_WECHAT_SUPPORT
#include "lwipopts.h"
#include "lwip/debug.h"

#define IPADDR_PING  ( (u32_t)0x01015FA8UL )

typedef void (*pvPing_Result)(void);

void ping_init(pvPing_Result PingResult_cb);
void ping_uninit(void);
#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */
u32_t ping_get_target_ip(void);
#else
#include "SNX_PING_API.h"
#define IPADDR_PING  ( (u32_t)0x01015FA8UL )
#endif

#endif /* __PING_H__ */
