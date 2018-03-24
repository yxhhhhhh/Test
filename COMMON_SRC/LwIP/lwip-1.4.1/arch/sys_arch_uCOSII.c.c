/**
 * @file
 * lwIP Operating System abstraction
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
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
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"

#include "lwip/sys.h"

/* Most of the functions defined in sys.h must be implemented in the
 * architecture-dependent file sys_arch.c */

// Sonix modify - start, <Ricky>
#include "lwip/def.h"
#include "lwip/memp.h"
#include "lwip/timers.h"

static char pcQueueMemoryPool[MAX_QUEUES * sizeof(TQ_DESCR) + MEM_ALIGNMENT - 1];

OS_STK LWIP_TASK_STK[LWIP_TASK_MAX][LWIP_STK_SIZE];
u8_t    curr_prio_offset;

static OS_MEM * pQueueMem = NULL;

struct sys_timeout {
    struct sys_timeout* next;
    INT32U   time;
    sys_timeout_handler h;
    void * arg;
};

struct sys_timeouts {
    struct sys_timeout* next;
};

struct sys_timeouts null_timeouts;
struct sys_timeouts lwip_timeouts[LWIP_TASK_MAX];
// Sonix modify - end, <Ricky>


#if !NO_SYS

// Sonix modify - start, <Ricky>
void sys_init(void) {
    u8_t    i, ucErr;
    pQueueMem = OSMemCreate((void *)((u32_t)((u32_t)pcQueueMemoryPool + MEM_ALIGNMENT -1) & ~(MEM_ALIGNMENT - 1)), 
                                MAX_QUEUES, sizeof(TQ_DESCR), &ucErr);

    LWIP_ASSERT("OSMemCreate ", ucErr == OS_ERR_NONE);

    curr_prio_offset = 0;

    for (i=0; i<LWIP_TASK_MAX; i++) {
        lwip_timeouts[i].next = NULL;
    }
}

sys_thread_t sys_thread_new(const char * name, lwip_thread_fn function, void * arg, int stackSize, int prio) {
    u8_t ucErr=1;
    
    if (prio > 0 && prio <= LWIP_TASK_MAX) {
#if 1
        ucErr=OSTaskCreate(function, (void *)arg/*0x1111*/, &LWIP_TASK_STK[prio-1][LWIP_STK_SIZE-1], LWIP_START_PRIO+prio - 1);
#else
        OSTaskCreateExt((void (*)(void *)) function, 
                                     (void *) arg, 
                                     (OS_STK   * )&LWIP_TASK_STK[prio -1][LWIP_STK_SIZE - 1], 
                                     (INT8U         )LWIP_START_PRIO+prio - 1,
                                     (INT16U        )LWIP_START_PRIO+prio - 1,
                                     (OS_STK    * )&LWIP_TASK_STK[prio -1][0],
                                     (INT32U        ) LWIP_STK_SIZE,
                                     (void *          ) 0,
                                     (INT16U         )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
#endif
        OSTaskNameSet(LWIP_START_PRIO+prio - 1, (u8_t*)name, &ucErr);
        return 1;
    }
    else {
        //PRINT("lwip task prio out of range! error!");
    }

    return 0;
}

#if 0           // SONiX modify - <Ricky>
struct sys_timeouts * sys_arch_timeouts(void) {
    INT8U curr_prio;
    INT16S err, offset;
    OS_TCB curr_task_pcb;
    null_timeouts.next = NULL;

    err = OSTaskQuery(OS_PRIO_SELF, &curr_task_pcb);
    curr_prio = curr_task_pcb.OSTCBPrio;
    offset = curr_prio - LWIP_START_PRIO;

    if (offset < 0 || offset >= LWIP_TASK_MAX) {
        return &null_timeouts;
    }

    return &lwip_timeouts[offset];
}

void sys_timeout_debug(INT32U msecs, sys_timeout_handler h, void * arg, const char * name) {
    struct sys_timeouts * timeouts;
    struct sys_timeout * timeout, *t;

    timeout = (struct sys_timeout *)memp_malloc(MEMP_SYS_TIMEOUT);

    if (timeout == NULL) {
        return;
    }
    timeout->next = NULL;
    timeout->h = h;
    timeout->arg = arg;
    timeout->time = msecs;
    timeouts = sys_arch_timeouts();
    
    if (timeouts->next == NULL) {
        timeouts->next = timeout;
        return;
    }

    if (timeouts->next->time > msecs) {
        timeouts->next->time -= msecs;
        timeout->next = timeouts->next;
        timeouts->next = timeout;
    }
    else {
        for (t = timeouts->next; t!= NULL; t=t->next) {
            timeout->time -= t->time;
            if (t->next == NULL || t->next->time > timeout->time) {
                if (t->next != NULL) {
                    t->next->time -= timeout->time;
                }
                timeout->next = t->next;
                t->next = timeout;
                break;
            }  
        }
    }
}


void sys_untimeout(sys_timeout_handler h, void * arg) {
    struct sys_timeouts *timeouts;
    struct sys_timeout *prev_t, *t;

    timeouts = sys_arch_timeouts();

    if (timeouts->next == NULL) {
        return;
    }

    for (t=timeouts->next, prev_t = NULL; t != NULL; prev_t = t, t=t->next) {
        if (t->h == h && (t->arg == arg)) {
            if (prev_t == NULL) {
                timeouts->next = t->next;
            }
            else {
                prev_t->next = t->next;
            }

            if (t->next != NULL) {
                t->next->time += t->time;
            }
            memp_free(MEMP_SYS_TIMEOUT, t);
            return;
        }
    }
}
#endif

err_t sys_mbox_new(sys_mbox_t * mBox, int size) {
    INT8U ucErr;
    PQ_DESCR pQDesc;

    pQDesc = OSMemGet(pQueueMem, &ucErr);
//    LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky> sys_mbox_new(), pQDesc: %x\r\n", pQDesc));
    LWIP_ASSERT("OSMemGet ", ucErr == OS_ERR_NONE );

    if (ucErr == OS_ERR_NONE) {
        if (size > MAX_QUEUE_ENTRIES) {
            size = MAX_QUEUE_ENTRIES;
        }
        pQDesc->pQ = OSQCreate(&(pQDesc->pvQEntries[0]), size);
        //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky> sys_mbox_new() -> OSQCreate(), pQDesc->pQ: %x, size: %d\r\n", pQDesc->pQ, size));
        LWIP_ASSERT("OSQCreate ", pQDesc->pQ != NULL );
        if (pQDesc->pQ != NULL) {
            *mBox = pQDesc;
        }
        else {
            //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky> sys_mbox_new() -> OSMemPut(), pQDesc->pQ: %x, size: %d\r\n", pQDesc->pQ, size));
            ucErr = OSMemPut(pQueueMem, pQDesc);
            *mBox = SYS_MBOX_NULL;
        }
    }

    return ucErr;
}

void sys_mbox_free(sys_mbox_t* mbox) {
        u8_t ucErr;

        LWIP_ASSERT( "sys_mbox_free ", *mbox != SYS_MBOX_NULL );

        /* clear OSQ EVENT */
        OSQFlush( (*mbox)->pQ );

        /* del OSQ EVENT */
        (void)OSQDel((*mbox)->pQ, OS_DEL_NO_PEND, &ucErr);
        LWIP_ASSERT( "OSQDel ", ucErr == OS_ERR_NONE );

        /* put mem back to mem queue */
        ucErr = OSMemPut(pQueueMem, *mbox);
        LWIP_ASSERT( "OSMemPut ", ucErr == OS_ERR_NONE );
}

const void * const pvNullPointer = (mem_ptr_t *)0xffffffff;
void sys_mbox_post(sys_mbox_t* mbox, void * msg) {
    u8_t err, i = 0;
   // LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\nEnter sys_mbox_post()\r\n"));
    
    LWIP_ASSERT("sys_mbox_post: invalid mbox", sys_mbox_valid(mbox));

    if (! msg) {
        msg = (void *)&pvNullPointer;
        //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky>  msg == pvNullPointer\r\n", i));
    }
#if 0
    err = OSQPost(mbox->pQ, data);
#else
    do {
        err = OSQPost((*mbox)->pQ, msg);
       // LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky> %d time to OSQPost(), err: %d\r\n", i, err));
        i++;
        OSTimeDly(5);
    } while ((i<10) && err != OS_ERR_NONE) ;
#if 0
    if (i< 10) {
        apimsg = (struct api_msg *)tcpipMsg->msg.apimsg;
        //apimsg->msg.err = ERR_OK;
    }
#endif    
#endif
}

err_t sys_mbox_trypost(sys_mbox_t* mbox, void * msg) {
    if (msg == NULL) {
        msg = (void *)&pvNullPointer;
    }
    if ( (OSQPost((*mbox)->pQ, msg)) != OS_ERR_NONE ) {
        return ERR_MEM;
    }

    return ERR_OK;
}

#if 0
void sys_mbox_fetch(sys_mbox_t * mbox, void **msg) {
    INT32U time;
    struct sys_timeouts *timeouts;
    struct sys_timeout *tmptimeout;
    sys_timeout_handler h;
    void * arg;

    LWIP_ASSERT("sys_mbox_fetch: invalid mbox", sys_mbox_valid(mbox));

  again:
    timeouts = sys_arch_timeouts();
    if (!timeouts || !timeouts->next) {
        sys_arch_mbox_fetch(mbox, msg, 0);
    }
    else {
        if (timeouts->next->time > 0) {
            time = sys_arch_mbox_fetch(mbox, msg, timeouts->next->time);
        }
        else {
            time = SYS_ARCH_TIMEOUT;
        }

        if (time == SYS_ARCH_TIMEOUT) {
            tmptimeout = timeouts->next;
            timeouts->next = tmptimeout->next;
            h = tmptimeout->h;
            arg = tmptimeout->arg;
            memp_free(MEMP_SYS_TIMEOUT, tmptimeout);

            if (h != NULL) {
                h(arg);
            }

            goto again;
        }
        else {
            if (time <= timeouts->next->time) {
                timeouts->next->time -= time;
            }
            else {
                timeouts->next->time = 0;
            }
        }
    }

}
#endif

u32_t sys_arch_mbox_fetch(sys_mbox_t * mbox, void ** msg, u32_t timeout) {
    u8_t ucErr;
    u16_t ucos_timeout, timeout_new;

    //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("<Ricky> sys_arch_mbox_fetch(), timeout: %d", timeout));

    LWIP_ASSERT("sys_arch_mbox_fetch: invalid mbox", sys_mbox_valid(mbox));

    ucos_timeout = 0;

    if (timeout != 0) {
        ucos_timeout = (timeout * OS_TICKS_PER_SEC) / 1000;
        //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("<Ricky>rslt: %d\n", (timeout*OS_TICKS_PER_SEC)/1000));
        //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("<Ricky> ucos_timeout: %d\n", ucos_timeout));

        if (ucos_timeout < 1) {
            ucos_timeout = 1;
        }
        else if (ucos_timeout > 65535) {
            ucos_timeout = 65535;
        }
    }
    else {
        ucos_timeout = 0;
    }

    timeout = OSTimeGet();
   // LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("timeout: "U32_F"\n", timeout));

#if 0
    if (msg != NULL) {
        *msg = OSQPend(mbox->pQ, (u16_t)ucos_timeout, &ucErr);
    }
    else {
        OSQPend(mbox->pQ, (u16_t )ucos_timeout, &ucErr);
    }
#else
    //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("<Ricky> sys_arch_mbox_fetch, OSQPend() start, ucos_timeout: %d...\n", ucos_timeout));
    *msg = OSQPend((*mbox)->pQ, (u16_t)ucos_timeout, &ucErr);
    //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("<Ricky> sys_arch_mbox_fetch, OSQPend() end...\n"));
#endif

    if (ucErr == OS_ERR_TIMEOUT) {
        timeout = SYS_ARCH_TIMEOUT;
        //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky> sys_arch_mbox_fetch, ucErr == OS_ERR_TIMEOUT...\r\n"));
    }
    else {
        //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\n<Ricky> sys_arch_mbox_fetch() successfully, ucErr: %d\r\n", ucErr));
        LWIP_ASSERT( "OSQPend ", ucErr == OS_ERR_NONE );	
        timeout_new = OSTimeGet();
        if (timeout_new > timeout) {
            timeout_new = timeout_new - timeout;
        }
        else {
            timeout_new = 0xffffffff - timeout + timeout_new;
        }
        timeout = (timeout_new)*(1000/OS_TICKS_PER_SEC) +1;
        
        if (*msg == (void*)&pvNullPointer) {
            *msg = NULL;
        }
    }

    return timeout;
}



err_t sys_sem_new(sys_sem_t * sem, u8_t count) {
    *sem = OSSemCreate((u16_t)count);
    //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\nEnd sys_sem_new() -> OSSemCreate()\r\n"));
    if (*sem != NULL) {
        return ERR_OK;
    }

    return ERR_MEM;
}

void sys_sem_signal(sys_sem_t * sem) {
    u8_t err = OSSemPost((OS_EVENT *)*sem);

    LWIP_ASSERT("sys_sem_signal: invalid semaphore", sys_sem_valid(sem));

    switch (err) {
        case OS_ERR_NONE:
            /* Semaphore signaled */
            break;
        case OS_ERR_SEM_OVF:
            /* Semaphore has overflowed */
            break;
    }
}

#if 0
void sys_sem_wait(sys_sem_t * sem) {
    sys_arch_sem_wait(sem, 0);
}
#endif

u32_t sys_arch_sem_wait(sys_sem_t * sem, u32_t timeout) {
    u32_t ucErr;
    u16_t ucos_timeout = 0;

    LWIP_ASSERT("sys_arch_sem_wait: invalid semaphore", sys_sem_valid(sem));

    if (timeout != 0) {
        ucos_timeout = (timeout)*(OS_TICKS_PER_SEC/1000);

        if (ucos_timeout < 1) {
            ucos_timeout = 1;
        }
        else if (ucos_timeout > 65535) {
            ucos_timeout = 65535;
        }
    }
    timeout = OSTimeGet();
    
    OSSemPend((OS_EVENT *)*sem, (u16_t)ucos_timeout, (u8_t *)&ucErr);

    if (ucErr == OS_ERR_TIMEOUT) {
        timeout = SYS_ARCH_TIMEOUT;
    }
    else {
        timeout = (ucos_timeout - ucErr)*(1000/OS_TICKS_PER_SEC);
    }
    
    return timeout;
}

void sys_sem_free(sys_sem_t * sem) {
    u8_t err;

    LWIP_ASSERT("sys_sem_free: invalid semaphore", sys_sem_valid(sem));
    
    *sem = OSSemDel(*sem, OS_DEL_ALWAYS, &err);
    /*if (sem ==(OS_EVENT *)0) {
        
        }*/
}

void sys_sem_set_invalid(sys_sem_t * sem) {
    if (sem != NULL) {
        *sem = NULL;
    }
}

void sys_mbox_set_invalid(sys_mbox_t *mbox) {
    //LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\nstart sys_mbox_set_invalid()\r\n"));
    if (*mbox != NULL) {
        *mbox = NULL;
    }
   // LWIP_DEBUGF(LWIP_FLOW_DEBUG | LWIP_DBG_TRACE, ("\r\nend sys_mbox_set_invalid()\r\n"));
}

int sys_sem_valid(sys_sem_t *sem) {
    if (*sem != NULL)   return 1;

    return 0;
}

int sys_mbox_valid(sys_mbox_t *mbox) {
    if (*mbox != NULL)   return 1;

    return 0;
}

#endif /* !NO_SYS */
