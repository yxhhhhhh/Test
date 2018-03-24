/************************************************************************/
/**	\file	sys_arch.h
 *	\brief	LwIP system functions for SYSBIOS on C674x
 *
 *	\date	2011/09/28
 *	\author	Stephane Lesage & Pierre Audenard for LwIP
 *
 */
/************************************************************************/
#ifndef _SYS_ARCH_H_
#define _SYS_ARCH_H_


/* Includes */
#include "Ucos_ii.h"


#define LWIP_COMPAT_MUTEX 1

/* Macros */
#define SYS_MBOX_NULL   (PQ_DESCR)0
#define SYS_SEM_NULL    (OS_EVENT *)0

#define MAX_QUEUES  10
#define MAX_QUEUE_ENTRIES	20  // Ricky test

#define LWIP_STK_SIZE   300
#define LWIP_TASK_START_PRIO	6
#define LWIP_TASK_END_PRIO		8
#define LWIP_TASK_MAX     (LWIP_TASK_END_PRIO - LWIP_TASK_START_PRIO + 1)
#define LWIP_START_PRIO LWIP_TASK_START_PRIO

/* Macros */
#define sys_arch_mbox_tryfetch(mbox,msg) \
        sys_arch_mbox_fetch(mbox,msg,1)

/* Types */
 typedef struct {
      OS_EVENT* pQ;
      void * pvQEntries[MAX_QUEUE_ENTRIES];
  }TQ_DESCR, *PQ_DESCR;
 
  typedef PQ_DESCR sys_mbox_t;
 
  typedef OS_EVENT* sys_sem_t;
  //typedef OS_EVENT sys_mbox_t;
  typedef OS_EVENT* sys_mutex_t;
  typedef INT8U sys_thread_t;
  // SONiX modify - end, <Ricky>

/* Variables */

/* Functions */
#if 0
#define sys_sem_valid(s) 			((s != NULL) && (*s != NULL))
#define sys_sem_set_invalid(s)		*s = NULL

#define sys_mutex_valid(m)			((m != NULL) && (m->Handle != NULL))
#define sys_mutex_set_invalid(m)	*m = NULL

#define sys_mbox_valid(m)			((m != NULL) && (*m != NULL))
#define sys_mbox_set_invalid(m)		*m = NULL
#endif

#endif // _SYS_ARCH_H_

