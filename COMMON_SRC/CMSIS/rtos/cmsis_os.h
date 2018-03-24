/* ----------------------------------------------------------------------
 * $Date:        5. February 2013
 * $Revision:    V1.02
 *
 * Project:      CMSIS-RTOS API
 * Title:        cmsis_os.h header file
 *
 * Version 0.02
 *    Initial Proposal Phase
 * Version 0.03
 *    osKernelStart added, optional feature: main started as thread
 *    osSemaphores have standard behavior
 *    osTimerCreate does not start the timer, added osTimerStart
 *    osThreadPass is renamed to osThreadYield
 * Version 1.01
 *    Support for C++ interface
 *     - const attribute removed from the osXxxxDef_t typedef's
 *     - const attribute added to the osXxxxDef macros
 *    Added: osTimerDelete, osMutexDelete, osSemaphoreDelete
 *    Added: osKernelInitialize
 * Version 1.02
 *    Control functions for short timeouts in microsecond resolution:
 *    Added: osKernelSysTick, osKernelSysTickFrequency, osKernelSysTickMicroSec
 *    Removed: osSignalGet 
 *----------------------------------------------------------------------------
 *
 * Copyright (c) 2013 ARM LIMITED
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*!
    \file       cmsis_os.h
    \brief      CMSIS-RTOS API header file
    \author     Nick Huang
    \version    1.1
    \date       2018/02/05
*/
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stddef.h>
	
#ifndef _CMSIS_OS_H
#define _CMSIS_OS_H
//------------------------------------------------------------------------------
//! \note MUST REMAIN UNCHANGED: \b osCMSIS identifies the CMSIS-RTOS API version.
#define osCMSIS				0x10002												//!< API version (main [31:16] .sub [15:0])

#define OS_MAJOR_VER        1                                                   // Major version
#define OS_MINOR_VER        1                                                   // Minor version

#define OS_VERSION_NUM      "V1.1"

//! \note CAN BE CHANGED: \b osCMSIS_KERNEL identifies the underlying RTOS kernel and version number.
#define osCMSIS_KERNEL	((OS_MAJOR_VER<<8) + OS_MINOR_VER                       //!< RTOS identification and version (major [15:8] .minor [7:0])

//! \note MUST REMAIN UNCHANGED: \b osKernelSystemId shall be consistent in every CMSIS-RTOS.
#define osKernelSystemId	"CMSIS-RTOS "##OS_VERSION_NUM                       //!< RTOS identification string

//! \note MUST REMAIN UNCHANGED: \b osFeature_xxx shall be consistent in every CMSIS-RTOS.
#define osFeature_MainThread	1												//!< main thread      1=main can be thread, 0=not available
#define osFeature_Pool			1												//!< Memory Pools:    1=available, 0=not available
#define osFeature_MailQ			1												//!< Mail Queues:     1=available, 0=not available
#define osFeature_MessageQ		1												//!< Message Queues:  1=available, 0=not available
#define osFeature_Signals		32												//!< maximum number of Signal Flags available per thread
#define osFeature_Semaphore		30												//!< maximum count for \ref osSemaphoreCreate function
#define osFeature_Wait			0												//!< osWait function: 1=available, 0=not available
#define osFeature_SysTick		1												//!< osKernelSysTick functions: 1=available, 0=not available

#ifdef  __cplusplus
extern "C"
{
#endif
//------------------------------------------------------------------------------
//! Priority used for thread control.
//! \note MUST REMAIN UNCHANGED: \b osPriority shall be consistent in every CMSIS-RTOS.
typedef enum {
	osPriorityIdle			= -3,												//!< priority: idle (lowest)
	osPriorityLow			= -2,												//!< priority: low
	osPriorityBelowNormal	= -1,												//!< priority: below normal
	osPriorityNormal		=  0,												//!< priority: normal (default)
	osPriorityAboveNormal	= +1,												//!< priority: above normal
	osPriorityHigh			= +2,												//!< priority: high
	osPriorityRealtime		= +3,												//!< priority: realtime (highest)
	osPriorityError			= 0x84												//!< system cannot determine priority or thread has illegal priority
} osPriority;

//! Timeout value.
//! \note MUST REMAIN UNCHANGED: \b osWaitForever shall be consistent in every CMSIS-RTOS.
#define osWaitForever     0xFFFFFFFF											//!< wait forever timeout value

//! Status code values returned by CMSIS-RTOS functions.
//! \note MUST REMAIN UNCHANGED: \b osStatus shall be consistent in every CMSIS-RTOS.
typedef enum {
	osOK                    =     0,											//!< function completed; no error or event occurred.
	osEventSignal           =  0x08,											//!< function completed; signal event occurred.
	osEventMessage          =  0x10,											//!< function completed; message event occurred.
	osEventMail             =  0x20,											//!< function completed; mail event occurred.
	osEventTimeout          =  0x40,											//!< function completed; timeout occurred.
	osErrorParameter        =  0x80,											//!< parameter error: a mandatory parameter was missing or specified an incorrect object.
	osErrorResource         =  0x81,											//!< resource not available: a specified resource was not available.
	osErrorTimeoutResource  =  0xC1,											//!< resource not available within given time: a specified resource was not available within the timeout period.
	osErrorISR              =  0x82,											//!< not allowed in ISR context: the function cannot be called from interrupt service routines.
	osErrorISRRecursive     =  0x83,											//!< function called multiple times from ISR with same object.
	osErrorPriority         =  0x84,											//!< system cannot determine priority or thread has illegal priority.
	osErrorNoMemory         =  0x85,											//!< system is out of memory: it was impossible to allocate or reserve memory for the operation.
	osErrorValue            =  0x86,											//!< value of a parameter is out of range.
	osErrorOS               =  0xFF,											//!< unspecified RTOS error: run-time error but no other error message fits.
	os_status_reserved      =  0x7FFFFFFF										//!< prevent from enum down-size compiler optimization.
} osStatus;

//! Thread State
/* Thread state returned by osThreadGetState */
typedef enum {
	osThreadRunning			= 0x0,												//!< A thread is querying the state of itself, so must be running.
	osThreadReady			= 0x1,												//!< The thread being queried is in a read or pending ready list.
	osThreadBlocked			= 0x2,												//!< The thread being queried is in the Blocked state.
	osThreadSuspended		= 0x3,												//!< The thread being queried is in the Suspended state, or is in the Blocked state with an infinite time out.
	osThreadTerminated      = 0x4,												//!< The thread being queried has been terminated, but its TCB has not yet been freed.
	osThreadError			= 0x7FFFFFFF										//!< Thread error
} osThreadState;

//! Timer type value for the timer definition.
//! \note MUST REMAIN UNCHANGED: \b os_timer_type shall be consistent in every CMSIS-RTOS.
typedef enum {
	osTimerOnce             = 0,												//!< one-shot timer
	osTimerPeriodic         = 1													//!< repeating timer
} os_timer_type;

//! Entry point of a thread.
//! \note MUST REMAIN UNCHANGED: \b os_pthread shall be consistent in every CMSIS-RTOS.
typedef void (*os_pthread) (void const *argument);

//! Entry point of a timer call back function.
//! \note MUST REMAIN UNCHANGED: \b os_ptimer shall be consistent in every CMSIS-RTOS.
typedef void (*os_ptimer) (void const *argument);

// >>> the following data type definitions may shall adapted towards a specific RTOS

//! Thread ID identifies the thread (pointer to a thread control block).
//! \note CAN BE CHANGED: \b os_thread_cb is implementation specific in every CMSIS-RTOS.
typedef void * osThreadId;

//! Timer ID identifies the timer (pointer to a timer control block).
//! \note CAN BE CHANGED: \b os_timer_cb is implementation specific in every CMSIS-RTOS.
typedef void * osTimerId;

//! Mutex ID identifies the mutex (pointer to a mutex control block).
//! \note CAN BE CHANGED: \b os_mutex_cb is implementation specific in every CMSIS-RTOS.
typedef void * osMutexId;

//! Semaphore ID identifies the semaphore (pointer to a semaphore control block).
//! \note CAN BE CHANGED: \b os_semaphore_cb is implementation specific in every CMSIS-RTOS.
typedef void * osSemaphoreId;

//! Pool ID identifies the memory pool (pointer to a memory pool control block).
//! \note CAN BE CHANGED: \b os_pool_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_cb *osPoolId;

//! Message ID identifies the message queue (pointer to a message queue control block).
//! \note CAN BE CHANGED: \b os_messageQ_cb is implementation specific in every CMSIS-RTOS.
typedef void * osMessageQId;

//! Mail ID identifies the mail queue (pointer to a mail queue control block).
//! \note CAN BE CHANGED: \b os_mailQ_cb is implementation specific in every CMSIS-RTOS.
typedef struct os_mailQ_cb *osMailQId;


//! Thread Definition structure contains startup information of a thread.
//! \note CAN BE CHANGED: \b os_thread_def is implementation specific in every CMSIS-RTOS.
typedef struct os_thread_def {
	char					*name;												//!< Thread name 
	os_pthread				pthread;											//!< start address of thread function
	osPriority				tpriority;											//!< initial thread priority
	uint32_t				instances;											//!< maximum number of instances of that thread function
	uint32_t				stacksize;											//!< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;

//! Timer Definition structure contains timer parameters.
//! \note CAN BE CHANGED: \b os_timer_def is implementation specific in every CMSIS-RTOS.
typedef struct os_timer_def {
	os_ptimer				ptimer;												//!< start address of a timer function
} osTimerDef_t;

//! Mutex Definition structure contains setup information for a mutex.
//! \note CAN BE CHANGED: \b os_mutex_def is implementation specific in every CMSIS-RTOS.
typedef struct os_mutex_def {
	uint32_t				dummy;												//!< dummy value.
} osMutexDef_t;

//! Semaphore Definition structure contains setup information for a semaphore.
//! \note CAN BE CHANGED: \b os_semaphore_def is implementation specific in every CMSIS-RTOS.
typedef struct os_semaphore_def {
	uint32_t				dummy;												//!< dummy value.
} osSemaphoreDef_t;

//! Definition structure for memory block allocation.
//! \note CAN BE CHANGED: \b os_pool_def is implementation specific in every CMSIS-RTOS.
typedef struct os_pool_def {
	uint32_t				pool_sz;											//!< number of items (elements) in the pool
	uint32_t				item_sz;											//!< size of an item
	void					*pool;												//!< pointer to memory for pool
} osPoolDef_t;

//! Definition structure for message queue.
//! \note CAN BE CHANGED: \b os_messageQ_def is implementation specific in every CMSIS-RTOS.
typedef struct os_messageQ_def {
	uint32_t				queue_sz;											//!< number of elements in the queue
	uint32_t				item_sz;											//!< size of an item
	//void					*pool;												//!< memory array for messages
} osMessageQDef_t;

//! Definition structure for mail queue.
//! \note CAN BE CHANGED: \b os_mailQ_def is implementation specific in every CMSIS-RTOS.
typedef struct os_mailQ_def {
	uint32_t				queue_sz;											//!< number of elements in the queue
	uint32_t				item_sz;											//!< size of an item
	struct os_mailQ_cb		**cb;
} osMailQDef_t;

//! Event structure contains detailed information about an event.
//! \note MUST REMAIN UNCHANGED: \b os_event shall be consistent in every CMSIS-RTOS.
//!       However the struct may be extended at the end.
typedef struct {
	osStatus				status;												//!< status code: event or error information
	union {
		uint32_t			v;													//!< message as 32-bit value
		void				*p;													//!< message or mail as void pointer
		int32_t				signals;											//!< signal flags
	} value;																	//!< event value
	union {
		osMailQId			mail_id;											//!< mail id obtained by \ref osMailCreate
		osMessageQId		message_id;											//!< message id obtained by \ref osMessageCreate
	} def;																		//!< event definition
} osEvent;

//! Timeout structure contains detailed information about timeout.
typedef struct {
    uint32_t initTick;                                                          //!< initial tick
    uint32_t timeoutTick;                                                       //!< timeout tick
    uint8_t  timeoutOV;                                                         //!< whether the timeout tick is overflow
} osTimeout;

uint16_t uwOS_GetVersion(void);
//==============================================================================
//							Kernel Control Functions
//==============================================================================
/*!
	\brief      Initialize the RTOS Kernel for creating objects.
    \param[in]  pHeap               pointer to a heap
    \param[in]  ulHeapSize          size of heap in bytes
    \param[in]  pUncachedHeap       pointer to a uncached heap (optional), NULL if absence
    \param[in]  ulUncachedHeapSize  size of uncached heap in bytes
	\return status code that indicates the execution status of the function.
*/
osStatus osKernelInitialize(uint8_t* pHeap, uint32_t ulHeapSize, uint8_t* pUncachedHeap, uint32_t ulUncachedHeapSize);
//------------------------------------------------------------------------------
/*!
	\brief  Start the RTOS Kernel.
	\return status code that indicates the execution status of the function.
	\note   MUST REMAIN UNCHANGED: \b osKernelStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osKernelStart(void);
//------------------------------------------------------------------------------
/*!
	\brief  Check if the RTOS kernel is already started.
	\return 0   RTOS is not started\n
	        1   RTOS is started\n
	\note   MUST REMAIN UNCHANGED: \b osKernelRunning shall be consistent in every CMSIS-RTOS.
*/
int32_t osKernelRunning(void);
//------------------------------------------------------------------------------
#if (defined (osFeature_SysTick)  &&  (osFeature_SysTick != 0))                 // System Timer available
/*!
    \brief  Get the RTOS Kernel SysTick counter 
    \return RTOS kernel system timer as 32-bit value
    \note   MUST REMAIN UNCHANGED: \b osKernelSysTick shall be consistent in every CMSIS-RTOS.
*/
uint32_t osKernelSysTick(void);
//------------------------------------------------------------------------------
//! The RTOS kernel system timer frequency in Hz
//! \note Reflects the system timer setting and is typically defined in a configuration file.
#define osKernelSysTickFrequency    100
//------------------------------------------------------------------------------
/*!
    \brief  Convert a microseconds value to a RTOS kernel system timer value.
    \param  microsec    time value in microseconds.
    \return time value normalized to the \ref osKernelSysTickFrequency
*/
#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec * (osKernelSysTickFrequency)) / 1000000)
#endif                                                                          // System Timer available
//------------------------------------------------------------------------------
/*!
    \brief  Handles the tick increment
    \return none
*/
void osSystickHandler(void);
//------------------------------------------------------------------------------


//==============================================================================
//                              Thread Management
//==============================================================================
/*!
    \brief  Create a Thread Definition with function, priority, and stack requirements.
    \param  name        name of the thread function.
    \param  thread      start address of thread function
    \param  priority    initial priority of the thread function.
    \param  instances   number of possible thread instances.
    \param  stacksz     stack size (in bytes) requirements for the thread function.
    \note   CAN BE CHANGED: The parameters to \b osThreadDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)         // object is external
#define osThreadDef(name, thread, priority, instances, stacksz)  \
extern const osThreadDef_t os_thread_def_##name
#else                                   // define the object
#define osThreadDef(name, thread, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ #name, (thread), (priority), (instances), (stacksz)  }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Thread definition.
    \param  name    name of the thread definition object.
    \note   CAN BE CHANGED: The parameter to \b osThread shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osThread(name)  \
&os_thread_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create a thread and add it to Active Threads and set it to state READY.
    \param[in]  thread_def  thread definition referenced with \ref osThread.
    \param[in]  argument    pointer that is passed to the thread function as start argument.
    \return     thread ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osThreadCreate shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument);
//------------------------------------------------------------------------------
/*!
    \brief  Return the thread ID of the current running thread.
    \return thread ID for reference by other functions or NULL in case of error.
    \note   MUST REMAIN UNCHANGED: \b osThreadGetId shall be consistent in every CMSIS-RTOS.
*/
osThreadId osThreadGetId (void);
//------------------------------------------------------------------------------
/*!
    \brief      Terminate execution of a thread and remove it from Active Threads.
    \param[in]  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osThreadTerminate shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadTerminate (osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief  Pass control to next thread that is in state \b READY.
    \return status code that indicates the execution status of the function.
    \note   MUST REMAIN UNCHANGED: \b osThreadYield shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadYield (void);
//------------------------------------------------------------------------------
/*!
    \brief      Change priority of an active thread.
    \param[in]  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \param[in]  priority    new priority value for the thread function.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osThreadSetPriority shall be consistent in every CMSIS-RTOS.
*/
osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority);
//------------------------------------------------------------------------------
/*!
    \brief      Get current priority of an active thread.
    \param[in]  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \return     current priority value of the thread function.
    \note       MUST REMAIN UNCHANGED: \b osThreadGetPriority shall be consistent in every CMSIS-RTOS.
*/
osPriority osThreadGetPriority (osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief  Obtain the state of any thread.
    \param  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \return the stae of the thread, states are encoded by the osThreadState enumerated type.
*/
osThreadState osThreadGetState(osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief  Check if a thread is already suspended or not.
    \param  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadIsSuspended(osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief  Suspend execution of a thread.
    \param  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
                        A thread may suspend it self by passing NULL in place of a
                        valid thread handle.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadSuspend(osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief  Resume execution of a suspended thread.
    \param  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadResume(osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief  Suspend execution of a all active threads.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadSuspendAll(void);
//------------------------------------------------------------------------------
/*!
    \brief  Resume execution of a all suspended threads.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadResumeAll(void);
//------------------------------------------------------------------------------
/*!
    \brief  Lists all the current threads, along with their current state 
            and stack usage high water mark.
    \param  buffer  A buffer into which the above mentioned details
            will be written
    \param  length  The length of buffer in byte.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadList(uint8_t *buffer, int length);
//------------------------------------------------------------------------------
/*!
    \brief  Lists CPU time of all the current threads.
    \param  buffer  A buffer into which the above mentioned details
            will be written
    \param  length  The length of buffer in byte.
    \return status code that indicates the execution status of the function.
*/
osStatus osThreadCpuTime(uint8_t *buffer, int length);
//------------------------------------------------------------------------------
/*!
    \brief  Initial a timeout
    \param  pToStruct   A pointer to a osTimeout structure.
            ulTimeoutMs Timeout value (in ms)
    \return none
*/
void osThreadTimeoutInit(osTimeout* pToStruct, uint32_t ulTimeoutMs);
//------------------------------------------------------------------------------
/*!
    \brief  Check timeout status
    \param  pToStruct   A pointer to a osTimeout structure.
    \return osOK            timeout has not occurred
            osEventTimeout  timeout occurred
*/
osStatus osThreadCheckTimeout(osTimeout* pToStruct);
//------------------------------------------------------------------------------
/*!
    \brief  Enable thread monitor
    \param  min_stack_sz    Minimum stack size, osThreadStackWarningHook() will
                            be called while a thread whose stack size is smaller
                            than min_stack_sz.
    \return none
*/
void osThreadMonitorEnable(int min_stack_sz);
//------------------------------------------------------------------------------
/*!
    \brief  Disable thread monitor
    \return none
*/
void osThreadMonitorDisable(void);

//==============================================================================
//                           Generic Wait Functions
//==============================================================================
/*!
    \brief      Wait for Timeout (Time Delay).
    \param[in]  millisec    time delay value
    \return     status code that indicates the execution status of the function.
*/
osStatus osDelay(uint32_t millisec);
//------------------------------------------------------------------------------
#if (defined(osFeature_Wait) && (osFeature_Wait != 0))                          // Generic Wait available
/*!
    \brief      Wait for Signal, Message, Mail, or Timeout.
    \param[in]  millisec    timeout value or 0 in case of no time-out
    \return     event that contains signal, message, or mail information or error code.
    \note       MUST REMAIN UNCHANGED: \b osWait shall be consistent in every CMSIS-RTOS.
*/
osEvent osWait(uint32_t millisec);
#endif  // Generic Wait available
//------------------------------------------------------------------------------
/*!
    \brief  Delay a task until a specified time
    \param  PreviousWakeTime    Pointer to a variable that holds the time at which the 
            task was last unblocked. PreviousWakeTime must be initialised with the current time
            prior to its first use (PreviousWakeTime = osKernelSysTick() )
    \param  millisec            time delay value
    \return status code that indicates the execution status of the function.
*/
osStatus osDelayUntil(uint32_t *PreviousWakeTime, uint32_t millisec);

//==============================================================================
//                          Timer Management Functions
//==============================================================================
/*!
    \brief  Define a Timer object.
    \param  name        name of the timer object.
    \param  function    name of the timer call back function.
    \note   CAN BE CHANGED: The parameter to \b osTimerDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)  // object is external
#define osTimerDef(name, function)  \
extern const osTimerDef_t os_timer_def_##name
#else                            // define the object
#define osTimerDef(name, function)  \
const osTimerDef_t os_timer_def_##name = \
{ (function) }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Timer definition.
    \param  name    name of the timer object.
    \note   CAN BE CHANGED: The parameter to \b osTimer shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osTimer(name) \
&os_timer_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create a timer.
    \param[in]  timer_def   timer object referenced with \ref osTimer.
    \param[in]  type        osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
    \param[in]  argument    argument to the timer call back function.
    \return     timer ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osTimerCreate shall be consistent in every CMSIS-RTOS.
*/
osTimerId osTimerCreate(const osTimerDef_t *timer_def, os_timer_type type, void *argument);
//------------------------------------------------------------------------------
/*!
    \brief      Start or restart a timer.
    \param[in]  timer_id    timer ID obtained by \ref osTimerCreate.
    \param[in]  millisec    time delay value of the timer.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osTimerStart shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStart(osTimerId timer_id, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Stop the timer.
    \param[in]  timer_id    timer ID obtained by \ref osTimerCreate.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osTimerStop shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerStop(osTimerId timer_id);
//------------------------------------------------------------------------------
/*!
    \brief      Delete a timer that was created by \ref osTimerCreate.
    \param[in]  timer_id    timer ID obtained by \ref osTimerCreate.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osTimerDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osTimerDelete(osTimerId timer_id);

//==============================================================================
//                              Signal Management
//==============================================================================
/*!
    \brief      Set the specified Signal Flags of an active thread.
    \param[in]  thread_id   thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
    \param[in]  signals     specifies the signal flags of the thread that should be set.
    \return     osOK if successful, osErrorOS if failed.
    \note       MUST REMAIN UNCHANGED: \b osSignalSet shall be consistent in every CMSIS-RTOS.
*/
int32_t osSignalSet(osThreadId thread_id, int32_t signals);
//------------------------------------------------------------------------------
/*!
    \brief      Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
    \param[in]  signals     wait until all specified signal flags set or 0 for any single signal flag.
    \param[in]  millisec    timeout value or 0 in case of no time-out.
    \return     event flag information or error code.
    \note       MUST REMAIN UNCHANGED: \b osSignalWait shall be consistent in every CMSIS-RTOS.
*/
osEvent osSignalWait (int32_t signals, uint32_t millisec);

//==============================================================================
//                              Mutex Management
//==============================================================================
/*!
    \brief  Define a Mutex.
    \param  name    name of the mutex object.
    \note   CAN BE CHANGED: The parameter to \b osMutexDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)  // object is external
#define osMutexDef(name)  \
extern const osMutexDef_t os_mutex_def_##name
#else                            // define the object
#define osMutexDef(name)  \
const osMutexDef_t os_mutex_def_##name = { 0 }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Mutex definition.
    \param  name    name of the mutex object.
    \note   CAN BE CHANGED: The parameter to \b osMutex shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osMutex(name)  \
&os_mutex_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create and Initialize a Mutex object.
    \param[in]  mutex_def   mutex definition referenced with \ref osMutex.
    \return     mutex ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osMutexCreate shall be consistent in every CMSIS-RTOS.
*/
osMutexId osMutexCreate(const osMutexDef_t *mutex_def);
//------------------------------------------------------------------------------
/*!
    \brief      Wait until a Mutex becomes available.
    \param[in]  mutex_id    mutex ID obtained by \ref osMutexCreate.
    \param[in]  millisec    timeout value or 0 in case of no time-out.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osMutexWait shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Release a Mutex that was obtained by \ref osMutexWait.
    \param[in]  mutex_id    mutex ID obtained by \ref osMutexCreate.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osMutexRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexRelease (osMutexId mutex_id);
//------------------------------------------------------------------------------
/*!
    \brief      Delete a Mutex that was created by \ref osMutexCreate.
    \param[in]  mutex_id    mutex ID obtained by \ref osMutexCreate.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osMutexDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osMutexDelete (osMutexId mutex_id);
//------------------------------------------------------------------------------
/*!
    \brief  Create and Initialize a Recursive Mutex
    \param  mutex_def   mutex definition referenced with \ref osMutex.
    \return mutex ID for reference by other functions or NULL in case of error..
*/
osMutexId osRecursiveMutexCreate(const osMutexDef_t *mutex_def);
//------------------------------------------------------------------------------
/*!
    \brief  Release a Recursive Mutex
    \param  mutex_id    mutex ID obtained by \ref osRecursiveMutexCreate.
    \return status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexRelease(osMutexId mutex_id);
//------------------------------------------------------------------------------
/*!
    \brief  Release a Recursive Mutex
    \param  mutex_id    mutex ID obtained by \ref osRecursiveMutexCreate.
    \param  millisec    timeout value or 0 in case of no time-out.
    \return status code that indicates the execution status of the function.
*/
osStatus osRecursiveMutexWait(osMutexId mutex_id, uint32_t millisec);

//==============================================================================
//                        Semaphore Management Functions
//==============================================================================
#if (defined(osFeature_Semaphore) && (osFeature_Semaphore != 0))                // Semaphore available
/*!
    \brief  Define a Semaphore object.
    \param  name    name of the semaphore object.
    \note   CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)  // object is external
#define osSemaphoreDef(name)  \
extern const osSemaphoreDef_t os_semaphore_def_##name
#else                            // define the object
#define osSemaphoreDef(name)  \
const osSemaphoreDef_t os_semaphore_def_##name = { 0 }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Semaphore definition.
    \param  name    name of the semaphore object.
    \note   CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osSemaphore(name)  \
&os_semaphore_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create and Initialize a Semaphore object used for managing resources.
    \param[in]  semaphore_def semaphore definition referenced with \ref osSemaphore.
    \param[in]  count   number of available resources.
    \return     semaphore ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osSemaphoreCreate shall be consistent in every CMSIS-RTOS.
*/
osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count);
//------------------------------------------------------------------------------
/*!
    \brief      Wait until a Semaphore token becomes available.
    \param[in]  semaphore_id    semaphore object referenced with \ref osSemaphoreCreate.
    \param[in]  millisec        timeout value or 0 in case of no time-out.
    \return     number of available tokens, or -1 in case of incorrect parameters.
    \note       MUST REMAIN UNCHANGED: \b osSemaphoreWait shall be consistent in every CMSIS-RTOS.
*/
int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Release a Semaphore token.
    \param[in]  semaphore_id    semaphore object referenced with \ref osSemaphoreCreate.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osSemaphoreRelease shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreRelease(osSemaphoreId semaphore_id);
//------------------------------------------------------------------------------
/*!
    \brief      Delete a Semaphore that was created by \ref osSemaphoreCreate.
    \param[in]  semaphore_id    semaphore object referenced with \ref osSemaphoreCreate.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osSemaphoreDelete shall be consistent in every CMSIS-RTOS.
*/
osStatus osSemaphoreDelete(osSemaphoreId semaphore_id);
#endif     // Semaphore available

//==============================================================================
//                   Dynamic Memory Management Functions
//==============================================================================
/*!
    \brief      Allocates a block \a size bytes of memory, returning a pointer
                to the beginning of the block. The content of the newly allocated
                block of memory is not initialized, remaining with indeterminate
                values.
    \param[in]  size    Size of the memory block, in bytes.
    \return     On success, a pointer to the memory block allocated by the function.
                If the function falied to allocat the requested block of memory,
                a null pointer is returned.
*/
void* osMalloc(size_t size);
//------------------------------------------------------------------------------
/*!
    \brief      Allocates a block of memory for an array of \a num elements, each
                of them \a size bytes long.
    \param[in]  num     Number of elements to allocate.
    \param[in]  size    Size of each element.
    \return     On success, a pointer to the memory block allocated by the function.
                If the function falied to allocat the requested block of memory,
                a null pointer is returned.
*/
void* osCalloc(size_t num, size_t size);
//------------------------------------------------------------------------------
/*!
    \brief      Allocates a block \a size bytes of memory within \b Uncached memory,
                returning a pointer to the beginning of the block. The content
                of the newly allocated block of memory is not initialized, 
                remaining with indeterminate values.
    \param[in]  size    Size of the memory block, in bytes.
    \return     On success, a pointer to the memory block allocated by the function.
                If the function falied to allocat the requested block of memory,
                a null pointer is returned.
*/
void* osUncachedMalloc(size_t size);
//------------------------------------------------------------------------------
/*!
    \brief      Allocates a block of memory for an array of \a num elements within
                \b Uncached memory, each of them \a size bytes long.
    \param[in]  num     Number of elements to allocate.
    \param[in]  size    Size of each element.
    \return     On success, a pointer to the memory block allocated by the function.
                If the function falied to allocat the requested block of memory,
                a null pointer is returned.
*/
void* osUncachedCalloc(size_t num, size_t size);
//------------------------------------------------------------------------------
/*!
    \brief      A block of memory previously allocated by a call to \b osMalloc,
                \b osCalloc, \b osUncachedMalloc or \b osUncachedCalloc is
                deallocated, making it available again for further allocations.\n
                If \a ptr does not point to a block of memory allocated with the
                above functions, it causes undefined behavior.\n
                If \a ptr is a null pointer, the function does nothing.
    \param[in]  ptr     Pointer to a memory block previously allocated with \b osMalloc
                        , \b osCalloc, \b osUncachedMalloc or \b osUncachedCalloc.
*/
void osFree(void* ptr);

//==============================================================================
//                      Memory Pool Management Functions
//==============================================================================
#if (defined(osFeature_Pool) && (osFeature_Pool != 0))                          // Memory Pool Management available
/*!
    \brief  Define a Memory Pool.
    \param  name    name of the memory pool.
    \param  no      maximum number of blocks (objects) in the memory pool.
    \param  type    data type of a single block (object).
    \note   CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)  // object is external
#define osPoolDef(name, no, type)   \
extern const osPoolDef_t os_pool_def_##name
#else                            // define the object
#define osPoolDef(name, no, type)   \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type), NULL }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Memory Pool definition.
    \param  name    name of the memory pool
    \note   CAN BE CHANGED: The parameter to \b osPool shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osPool(name) \
&os_pool_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create and Initialize a memory pool.
    \param[in]  pool_def    memory pool definition referenced with \ref osPool.
    \return     memory pool ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osPoolCreate shall be consistent in every CMSIS-RTOS.
*/
osPoolId osPoolCreate(const osPoolDef_t *pool_def);
//------------------------------------------------------------------------------
/*!
    \brief      Allocate a memory block from a memory pool.
    \param[in]  pool_id     memory pool ID obtain referenced with \ref osPoolCreate.
    \return     address of the allocated memory block or NULL in case of no memory available.
    \note       MUST REMAIN UNCHANGED: \b osPoolAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osPoolAlloc(osPoolId pool_id);
//------------------------------------------------------------------------------
/*!
    \brief      Allocate a memory block from a memory pool and set memory block to zero.
    \param[in]  pool_id     memory pool ID obtain referenced with \ref osPoolCreate.
    \return     address of the allocated memory block or NULL in case of no memory available.
    \note       MUST REMAIN UNCHANGED: \b osPoolCAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osPoolCAlloc(osPoolId pool_id);
//------------------------------------------------------------------------------
/*!
    \brief      Return an allocated memory block back to a specific memory pool.
    \param[in]  pool_id     memory pool ID obtain referenced with \ref osPoolCreate.
    \param[in]  block       address of the allocated memory block that is returned to the memory pool.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osPoolFree shall be consistent in every CMSIS-RTOS.
*/
osStatus osPoolFree(osPoolId pool_id, void *block);
#endif   // Memory Pool Management available

//==============================================================================
//                    Message Queue Management Functions
//==============================================================================
#if (defined(osFeature_MessageQ) && (osFeature_MessageQ != 0))                  // Message Queues available
/*!
    \brief  Create a Message Queue Definition.
    \param  name        name of the queue.
    \param  queue_sz    maximum number of messages in the queue.
    \param  type        data type of a single message element (for debugger).
    \note   CAN BE CHANGED: The parameter to \b osMessageQDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)  // object is external
#define osMessageQDef(name, queue_sz, type)   \
extern const osMessageQDef_t os_messageQ_def_##name
#else                            // define the object
#define osMessageQDef(name, queue_sz, type)   \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), sizeof (type)  }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Message Queue Definition.
    \param  name    name of the queue
    \note   CAN BE CHANGED: The parameter to \b osMessageQ shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osMessageQ(name) \
&os_messageQ_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create and Initialize a Message Queue.
    \param[in]  queue_def   queue definition referenced with \ref osMessageQ.
    \param[in]  thread_id   thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
    \return     message queue ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osMessageCreate shall be consistent in every CMSIS-RTOS.
*/
osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief      Put a Message to a Queue.
    \param[in]  queue_id    message queue ID obtained with \ref osMessageCreate.
    \param[in]  msg_ptr     pointer to buffer with message to put into a queue.
    \param[in]  millisec    timeout value or 0 in case of no time-out.
    \return     status code that indicates the execution status of the function.
    \note       This API was modified version and wasn't compatible to original
                CMSIS-RTOS. Original CMSIS-RTOS adopted "Queue by reference"
                method. "Queue by copy" method was adopted in this modified
                version and it's considered to be simultaneously more powerful
                and simpler.
*/
osStatus osMessagePut(osMessageQId queue_id, const void *msg_ptr, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Put a Message to the front of a Queue.
    \param[in]  queue_id    message queue ID obtained with \ref osMessageCreate.
    \param[in]  msg_ptr     pointer to buffer with message to put into a queue.
    \param[in]  millisec    timeout value or 0 in case of no time-out.
    \return     status code that indicates the execution status of the function.
*/
osStatus osMessagePutToFront(osMessageQId queue_id, const void *msg_ptr, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Get a Message or Wait for a Message from a Queue.
    \param[in]  queue_id    message queue ID obtained with \ref osMessageCreate.
    \param[out] msg_ptr     pointer to buffer for message to get from a queue.
    \param[in]  millisec    timeout value or 0 in case of no time-out.
    \return     status code that indicates the execution status of the function.
    \note       This API was modified version and wasn't compatible to original
                CMSIS-RTOS. Original CMSIS-RTOS adopted "Queue by reference"
                method. "Queue by copy" method was adopted in this modified
                version and it's considered to be simultaneously more powerful
                and simpler.
*/
osStatus osMessageGet(osMessageQId queue_id, void *msg_ptr, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Receive an item from a queue without removing the item from the queue.
    \param[in]  queue_id    message queue ID obtained with \ref osMessageCreate.
    \param[out] msg_ptr     pointer to buffer for message to peek from a queue.
    \param[in]  millisec    timeout value or 0 in case of no time-out.
    \return     status code that indicates the execution status of the function.
*/
osStatus osMessagePeek(osMessageQId queue_id, void *msg_ptr, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief  Resets a queue to its original empty state. Any data contained in the
            queue at the time it is reset is discarded.
    \param  queue_id    message queue ID obtained with \ref osMessageCreate.
    \return none
*/
void osMessageReset(osMessageQId queue_id);
//------------------------------------------------------------------------------
/*!
    \brief  Get the number of messages stored in a queue.
    \param  queue_id    message queue ID obtained with \ref osMessageCreate.
    \return The number of messages available in the queue.
*/
uint32_t osMessages(osMessageQId queue_id);
#endif     // Message Queues available

//==============================================================================
//                      Mail Queue Management Functions
//==============================================================================
#if (defined(osFeature_MailQ) && (osFeature_MailQ != 0))                        // Mail Queues available
/*!
    \brief  Create a Mail Queue Definition.
    \param  name        name of the queue
    \param  queue_sz    maximum number of messages in queue
    \param  type        data type of a single message element
    \note   CAN BE CHANGED: The parameter to \b osMailQDef shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#if defined (osObjectsExternal)  // object is external
#define osMailQDef(name, queue_sz, type) \
extern struct os_mailQ_cb *os_mailQ_cb_##name \
extern osMailQDef_t os_mailQ_def_##name
#else                            // define the object
#define osMailQDef(name, queue_sz, type) \
struct os_mailQ_cb *os_mailQ_cb_##name; \
const osMailQDef_t os_mailQ_def_##name =  \
{ (queue_sz), sizeof (type), (&os_mailQ_cb_##name) }
#endif
//------------------------------------------------------------------------------
/*!
    \brief  Access a Mail Queue Definition.
    \param  name    name of the queue
    \note   CAN BE CHANGED: The parameter to \b osMailQ shall be consistent but the
            macro body is implementation specific in every CMSIS-RTOS.
*/
#define osMailQ(name)  \
&os_mailQ_def_##name
//------------------------------------------------------------------------------
/*!
    \brief      Create and Initialize mail queue.
    \param[in]  queue_def   reference to the mail queue definition obtain with \ref osMailQ
    \param[in]  thread_id   thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
    \return     mail queue ID for reference by other functions or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osMailCreate shall be consistent in every CMSIS-RTOS.
*/
osMailQId osMailCreate(const osMailQDef_t *queue_def, osThreadId thread_id);
//------------------------------------------------------------------------------
/*!
    \brief      Allocate a memory block from a mail.
    \param[in]  queue_id    mail queue ID obtained with \ref osMailCreate.
    \param[in]  millisec    timeout value or 0 in case of no time-out
    \return     pointer to memory block that can be filled with mail or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osMailAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osMailAlloc(osMailQId queue_id, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Allocate a memory block from a mail and set memory block to zero.
    \param[in]  queue_id    mail queue ID obtained with \ref osMailCreate.
    \param[in]  millisec    timeout value or 0 in case of no time-out
    \return     pointer to memory block that can be filled with mail or NULL in case of error.
    \note       MUST REMAIN UNCHANGED: \b osMailCAlloc shall be consistent in every CMSIS-RTOS.
*/
void *osMailCAlloc(osMailQId queue_id, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Put a mail to a queue.
    \param[in]  queue_id    mail queue ID obtained with \ref osMailCreate.
    \param[in]  mail        memory block previously allocated with \ref osMailAlloc or \ref osMailCAlloc.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osMailPut shall be consistent in every CMSIS-RTOS.
*/
osStatus osMailPut(osMailQId queue_id, void *mail);
//------------------------------------------------------------------------------
/*!
    \brief      Get a mail from a queue.
    \param[in]  queue_id    mail queue ID obtained with \ref osMailCreate.
    \param[in]  millisec    timeout value or 0 in case of no time-out
    \return     event that contains mail information or error code.
    \note       MUST REMAIN UNCHANGED: \b osMailGet shall be consistent in every CMSIS-RTOS.
*/
osEvent osMailGet(osMailQId queue_id, uint32_t millisec);
//------------------------------------------------------------------------------
/*!
    \brief      Free a memory block from a mail.
    \param[in]  queue_id    mail queue ID obtained with \ref osMailCreate.
    \param[in]  mail        pointer to the memory block that was obtained with \ref osMailGet.
    \return     status code that indicates the execution status of the function.
    \note       MUST REMAIN UNCHANGED: \b osMailFree shall be consistent in every CMSIS-RTOS.
*/
osStatus osMailFree(osMailQId queue_id, void *mail);
#endif  // Mail Queues available

#ifdef  __cplusplus
}
#endif

#endif  // _CMSIS_OS_H
