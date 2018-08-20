/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SPI.c
	\brief		SPI (Serial Peripheral Interface) function
	\author		Nick Huang
	\version	0.3
	\date		2017/10/26
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "SPI.h"
#include "APBC.h"
#include "INTC.h"
//------------------------------------------------------------------------------
#define SPI_MAJOR_VER    0           // Major version
#define SPI_MINOR_VER    3           // Minor version

#define SPI_SLAVE_GOT_DATA  0x00000002
#define SPI_TX_DMA_END      0x00000004

uint8_t SPI_TxEndFlag;
uint8_t SPI_RxEndFlag;
uint8_t* pbSPI_RxData;
uint8_t ubSPI_RxDataLen;
uint8_t ubSPI_RxByteWidth;
SPI_SlaveHook pfSPI_SlaveHook = NULL;
SPI_DmaEndHook pfSPI_DmaEndHook = NULL;
uint8_t ubSPI_WaitEvent = 0;

void SPI_ISR(void);
void SPI_Thread(void const *argument);
void SPI_DmaEnd(void);
osThreadId SPI_ThreadId = NULL;
osThreadDef(SPI_SlavePolling, SPI_Thread, osPriorityNormal, 1, 512);
//------------------------------------------------------------------------------
uint16_t uwSPI_GetVersion(void)
{
    return ((SPI_MAJOR_VER << 8) + SPI_MAJOR_VER);
}
//------------------------------------------------------------------------------
void SPI_Init(SPI_Setup_t* setup)
{
    SSP->SSP_SCLKPH     = setup->ubSPI_CPHA;
    SSP->SSP_SCLKPO     = setup->ubSPI_CPOL;
    SSP->SSP_OPM        = setup->tSPI_Mode;
    SSP->SSP_SCLKDIV    = setup->uwClkDiv;
    SSP->SSP_FFMT       = 1;            // Motorola SPI
    pfSPI_DmaEndHook    = setup->pfDmaEndHook;

    if(SSP->SSP_OPM == SPI_SLAVE)
        pfSPI_SlaveHook = setup->pfSlaveHook;
    
    //! clear rx/tx fifo
    SSP->SSP_RXF_CLR = 1;
    SSP->SSP_TXF_CLR = 1;
    SSP->SSP_RXF_TH = 1;
    SSP->SSP_TXF_TH = 15;
    SSP->SSP_TXDOE = 1;

    SSP->SSP_EN = 0;

    if(SPI_ThreadId == NULL)
    {
        SPI_ThreadId = osThreadCreate(osThread(SPI_SlavePolling), NULL);
        if(SPI_ThreadId == NULL)
        {
            printd(DBG_CriticalLvl, "SPI: Create SPI_Thread fail !!!!\r\n");
            while(1);
        }
    }
    INTC_IrqSetup(INTC_SSP_IRQ, INTC_EDGE_TRIG, SPI_ISR);
	SSP->SSP_GPIO_MODE = 0;
}
//------------------------------------------------------------------------------
#if 0
void SPI_CpuRW(uint8_t* pbTxData, uint8_t* pbRxData, uint8_t ubDataLen, uint8_t ubDataWidth)
{
    int i;
    uint8_t ubByteWidth;

    if(SSP->SSP_OPM == SPI_MASTER)
        if((ubDataLen == 0) || (ubDataLen > 16)) return;
    if((ubDataWidth == 0) || (ubDataWidth > 32)) return;

    SSP->SSP_SDL = ubDataWidth - 1;
    ubByteWidth = (ubDataWidth + 7) >> 3;
    
    //! clear rx/tx fifo
    SSP->SSP_RXF_CLR = 1;
    SSP->SSP_TXF_CLR = 1;

    if(SSP->SSP_OPM == SPI_MASTER)
    {
        for(i=0;i<ubDataLen;i++)
        {
            switch(ubByteWidth)
            {
                case 1:
                    SSP->SSP_DATA = pbTxData[i];
                    break;
                case 2:
                    SSP->SSP_DATA = ((uint16_t*)pbTxData)[i];
                    break;
                case 3:
                case 4:
                    SSP->SSP_DATA = ((uint32_t*)pbTxData)[i];
                    break;
                default:
                    printd(DBG_ErrorLvl, "SPI: CPU RW...wrong byte width\n");
                    break;
            }
        }
        
        SSP->SSP_EN = 1;
        while(SSP->SSP_BUSY);
        SSP->SSP_EN = 0;

        i = 0;
        while(SSP->SSP_RXF_VE)
        {
            switch(ubByteWidth)
            {
                case 1:
                    pbRxData[i++] = SSP->SSP_DATA;
                    break;
                case 2:
                    ((uint16_t*)pbRxData)[i++] = SSP->SSP_DATA;
                    break;
                case 3:
                case 4:
                    ((uint32_t*)pbRxData)[i++] = SSP->SSP_DATA;
                    break;
                default:
                    printd(DBG_ErrorLvl, "SPI: CPU RW...wrong byte width\n");
                    break;
            }
        }
        if(i != ubDataLen)
            printd(DBG_ErrorLvl, "SPI: Rx data length not match: %d\n", i);
    }
    else
    {
        for(i=0;i<16;i++)
        {
            switch(ubByteWidth)
            {
                case 1:
                    SSP->SSP_DATA = pbTxData[0];
                    break;
                case 2:
                    SSP->SSP_DATA = ((uint16_t*)pbTxData)[0];
                    break;
                case 3:
                case 4:
                    SSP->SSP_DATA = ((uint32_t*)pbTxData)[0];
                    break;
                default:
                    printd(DBG_ErrorLvl, "SPI: CPU RW...wrong byte width\n");
                    break;
            }
        }
        pbSPI_RxData = pbRxData;
        ubSPI_RxByteWidth = ubByteWidth;
        ubSPI_RxDataLen = 0;

        INTC_IrqEnable(INTC_SSP_IRQ);
        SSP->SSP_RXF_TH_INTR_EN = 1;
        SSP->SSP_EN = 1;
    }
}
#endif
//------------------------------------------------------------------------------
void SPI_TxDmaIsr(APBC_INT_t tEvent)
{
    if(tEvent == APBC_FINISH)
    {
        SPI_TxEndFlag = 1;
        if(ubSPI_WaitEvent == 1)
            osSignalSet(SPI_ThreadId, SPI_TX_DMA_END);
    }
    if(tEvent == APBC_ERROR)
        printd(DBG_ErrorLvl, "SPI: Tx DMA Error!\n");
}
//------------------------------------------------------------------------------
void SPI_RxDmaIsr(APBC_INT_t tEvent)
{
    if(tEvent == APBC_FINISH)
        SPI_RxEndFlag = 1;
    if(tEvent == APBC_ERROR)
        printd(DBG_ErrorLvl, "SPI: Rx DMA Error!\n");
}
//------------------------------------------------------------------------------
void SPI_DmaRW(uint8_t* pbTxData, uint8_t* pbRxData, uint32_t ulDataLen, uint8_t ubDataWidth, SPI_WaitMode_t tWaitMode)
{
    APBC_DMA_WIDTH_t tDmaWidth;
    if(ulDataLen == 0) return;
    if((ubDataWidth == 0) || (ubDataWidth > 32)) return;
    
    switch(ubDataWidth)
    {
        case 32:
            tDmaWidth = APBC_DMA_32BIT;
            break;
        case 16:
            tDmaWidth = APBC_DMA_16BIT;
            break;
        case 8:
            tDmaWidth = APBC_DMA_8BIT;
            break;
        default:
            printd(DBG_ErrorLvl, "SPI: DMA Data Width Error %d!\n", ubDataWidth);
            return;
    }

    SSP->SSP_SDL = ubDataWidth - 1;
    
    //! clear rx/tx fifo
    SSP->SSP_RXF_CLR = 1;
    SSP->SSP_TXF_CLR = 1;
    
    APBC_ChA_Setup((uint32_t)pbTxData, APBC_SSP1_TX, ulDataLen, tDmaWidth, APBC_NONBURST, SPI_TxDmaIsr);
    APBC_ChB_Setup(APBC_SSP1_RX, (uint32_t)pbRxData, ulDataLen, tDmaWidth, APBC_NONBURST, SPI_RxDmaIsr);
    APBC_CHA_ENABLE;
    APBC_CHB_ENABLE;
    
    SSP->SSP_TXF_DMA_EN = 1;
    SSP->SSP_RXF_DMA_EN = 1;
    
    SPI_TxEndFlag = 0;
    SPI_RxEndFlag = 0;
    
    SSP->SSP_EN = 1;
    
    if((tWaitMode == SPI_DontWait) && (pfSPI_DmaEndHook != NULL))
    {
        ubSPI_WaitEvent = 1;
        return;
    }

    SPI_DmaEnd();
}
//------------------------------------------------------------------------------
void SPI_Thread(void const *argument)
{
    osEvent evt;
    while(1)
    {
        evt = osSignalWait(0, osWaitForever);
        
        if(evt.status == osEventSignal)
        {
            switch(evt.value.signals)
            {
                case SPI_SLAVE_GOT_DATA:
                    while(SSP->SSP_BUSY);
                    SSP->SSP_EN = 0;
                    while(SSP->SSP_RXF_VE)
                    {
                        switch(ubSPI_RxByteWidth)
                        {
                            case 1:
                                pbSPI_RxData[ubSPI_RxDataLen++] = SSP->SSP_DATA;
                                break;
                            case 2:
                                ((uint16_t*)pbSPI_RxData)[ubSPI_RxDataLen++] = SSP->SSP_DATA;
                                break;
                            case 3:
                            case 4:
                                ((uint32_t*)pbSPI_RxData)[ubSPI_RxDataLen++] = SSP->SSP_DATA;
                                break;
                            default:
                                printd(DBG_ErrorLvl, "SPI: CPU RW...wrong byte width\n");
                                break;
                        }
                    }
                    INTC_IrqDisable(INTC_SSP_IRQ);
                    SSP->SSP_RXF_TH_INTR_EN = 0;
                    if(pfSPI_SlaveHook != NULL)
                        pfSPI_SlaveHook(ubSPI_RxDataLen);
                    break;
                case SPI_TX_DMA_END:
                    SPI_DmaEnd();
                    break;
                default:
                    printd(DBG_ErrorLvl, "SPI: SPI_Thread got a invalid signal\n");
                    break;
            }
        }
        else
        {
            printd(DBG_ErrorLvl, "SPI: SPI_Thread got error event, not a signal event\n");
        }
    }
}
//------------------------------------------------------------------------------
void SPI_ISR(void)
{
    INTC_IrqClear(INTC_SSP_IRQ);
    if(SSP->SSP_RXF_TH_FLAG)
        osSignalSet(SPI_ThreadId, SPI_SLAVE_GOT_DATA);
}
//------------------------------------------------------------------------------
void SPI_DmaEnd(void)
{
    while(SSP->SSP_BUSY);
    while(!SPI_TxEndFlag);
    while(!SPI_RxEndFlag);
    SSP->SSP_EN = 0;

    SSP->SSP_TXF_DMA_EN = 0;
    SSP->SSP_RXF_DMA_EN = 0;

    APBC_CHA_DISABLE;
    APBC_CHB_DISABLE;
    
    if(ubSPI_WaitEvent == 1)
    {
        ubSPI_WaitEvent = 0;
        pfSPI_DmaEndHook();
    }
}
//------------------------------------------------------------------------------
