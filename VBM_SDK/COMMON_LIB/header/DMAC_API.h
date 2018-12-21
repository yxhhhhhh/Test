/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		DMAC_API.h
	\brief		DMA Controller API Header
	\author		Pierce
	\version	0.6
	\date		2017/09/04
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _DMA_API_H_
#define _DMA_API_H_
//------------------------------------------------------------------------------
#include "stdint.h"
#include "cmsis_os.h"
//------------------------------------------------------------------------------
/*!	\file DMA_API.h
DMAC FlowChart:
	\dot
digraph DMAC_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		"Not Busy" [color=blue, fontcolor=blue];
		"Busy" [color=red, fontcolor=red];
		DMAC_Init -> ulDMAC_CalDmaDespBufSize -> DMAC_SetDmaDespBufAddr ->"Not Busy";
		"Not Busy" -> tDMAC_MemCopy;
		"Not Busy" -> tDMAC_MemSet;
		edge [headport=sw, tailport=corner, fontname=Verdana, fontcolor=blue, fontsize=8];
		tDMAC_MemCopy -> "Not Busy" [label="ok"];
		edge [headport=se, tailport=corner];
		tDMAC_MemSet -> "Not Busy" [label="ok"];
		edge [headport=nw, tailport=nw, fontcolor=red];
		tDMAC_MemCopy -> "Not Busy" [label="fail"];
		edge [headport=ne, tailport=ne];
		tDMAC_MemSet -> "Not Busy" [label="fail"];
		edge [color=red, headport=nw, tailport =corner];
		tDMAC_MemCopy -> Busy [label="Busy"];
		edge [headport=ne];
		tDMAC_MemSet -> Busy [label="Busy"];
		edge [dir=back, color=blue, headport=corner,fontcolor=blue, fontsize=10];
		"Not Busy" -> "Busy" [headlabel= "Delay Time"];
	}
	\enddot
*/
//------------------------------------------------------------------------------
/*!
	\brief DMA Controller Result Type 
*/
typedef enum
{
	DMAC_UNSTA,	//!< DMAC un-start
	DMAC_OK,	//!< DMAC ok
	DMAC_BUSY,	//!< DMAC four channels busy
	DMAC_FAIL	//!< DMAC setting fail
}DMAC_RESULT;
//------------------------------------------------------------------------------
/*!
	\brief DMA Controller Thread Notify Type
*/
typedef struct
{
	osThreadId	ThreadId;	//!< Thread id
	int32_t		slSignal;	//!< Notify signal
}DMAC_THREAD_NOTIFY_TYP;
//------------------------------------------------------------------------------
/*!
    \brief      DMA Controller Initial Function    
    \par        [Example]
    \code    
                DMAC_Init();
    \endcode
*/
void DMAC_Init (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Calculate DMAC Desp Buffer size Function
	\return DMAC Desp buffer size
	\par [Example]
	\code    
		 uint32_t ulBufSz;

		 ulBufSz = ulDMAC_CalDmaDespBufSize();
	\endcode
*/
uint32_t ulDMAC_CalDmaDespBufSize (void);
//------------------------------------------------------------------------------
/*!
	\brief 		Set DMAC Desp Buffer Address Function
	\param[in]	ulAddr DMAC Desp buffer address
	\note		DMAC Desp buffer address must be four alignment
	\par [Example]
	\code    
		 DMAC_SetDmaDespBufAddr(0x200000);
	\endcode
*/
void  DMAC_SetDmaDespBufAddr(uint32_t ulAddr);
//------------------------------------------------------------------------------
/*!
	\brief 		DMA Controller Memory Copy Function	
	\param[in]	ulReadAddr  DMAC memory copy read (source) address
	\param[in]	ulWriteAddr DMAC memory copy write (destination) address
	\param[in]	ulLen	    DMAC memory copy length	(ulLen bytes) 
	\param[in]	ptNotify    User thread id & signal after DMAC memory copy done
	\return		DMAC_OK	    DMAC ok!	
	\return		DMAC_FAIL   DMAC setting fail!
	\note		ulReadAddr  must be four aligment\n
				ulWriteAddr must be four aligment\n 
	\par [Example]
	\code    
		 tDMAC_MemCopy(0x200000, 0x300000,0x4000, NULL);
	\endcode
*/
DMAC_RESULT tDMAC_MemCopy (uint32_t ulReadAddr, uint32_t ulWriteAddr, uint32_t ulLen, DMAC_THREAD_NOTIFY_TYP *ptNotify);
//------------------------------------------------------------------------------
/*!
	\brief 		DMA Controller Memory set Function	
	\param[in]	ubData	    DMAC memory set data
	\param[in]	ulWriteAddr DMAC memory set write (destination) address
	\param[in]	ulLen	    DMAC memory set length (ulLen bytes)	
	\param[in]	ptNotify    User thread id & signal after DMAC memory set done
	\return		DMAC_OK	    DMAC ok!	
	\return 	DMAC_FAIL   DMAC setting fail!	
	\par [Example]
	\code    
		 tDMAC_MemSet(0x5A, 0x300000,0x4000, NULL);
	\endcode
*/
DMAC_RESULT tDMAC_MemSet (uint8_t ubData, uint32_t ulWriteAddr, uint32_t ulLen, DMAC_THREAD_NOTIFY_TYP *ptNotify);
//------------------------------------------------------------------------------
/*!
	\brief 	Get DMAC Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwDMAC_GetVersion();
		 printf("DMAC Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwDMAC_GetVersion (void);
#endif
