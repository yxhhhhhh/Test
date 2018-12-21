/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APBC.c
	\brief		APB Controller function
	\author		Nick Huang
	\version	0.2
	\date		2017/10/19
	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "APBC.h"
#include "INTC.h"
//------------------------------------------------------------------------------
#define APBC_MAJOR_VER    0         // Major version = 0
#define APBC_MINOR_VER    2         // Minor version = 2

#define APBC_SSP1_DATA      0x92000018
#define APBC_UART1_DATA     0x92100010
#define APBC_UART2_DATA     0x92200010

enum
{
    APBC_APB_SRC,
    APBC_AHB_SRC
};

enum
{
    APBC_INC_0,
    APBC_INC_1,
    APBC_INC_2,
    APBC_INC_4
};

const uint32_t ulAPBC_DataPortAddr[] =
{
    0,
    APBC_SSP1_DATA,
    APBC_SSP1_DATA,
    APBC_UART1_DATA,
    APBC_UART1_DATA,
    APBC_UART2_DATA,
    APBC_UART2_DATA
};

APBC_IsrHandler pAPBC_ChAIsrFunc, pAPBC_ChBIsrFunc, pAPBC_ChCIsrFunc, pAPBC_ChDIsrFunc = NULL;
void APBC_ISR(void);
//------------------------------------------------------------------------------
void APBC_Init(void)
{
    INTC_IrqSetup(INTC_APB_IRQ, INTC_EDGE_TRIG, APBC_ISR);
    INTC_IrqEnable(INTC_APB_IRQ);
}
//------------------------------------------------------------------------------
uint16_t uwAPBC_GetVersion(void)
{
    return ((APBC_MAJOR_VER << 8) + APBC_MAJOR_VER);
}
//------------------------------------------------------------------------------
void APBC_ChA_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR)
{
    // Source Setup
    if(ulSrcAdr < 10)
    {
        APBC->CHA_SRC_A     = ulAPBC_DataPortAddr[ulSrcAdr];
        APBC->CHA_SRC_SEL   = APBC_APB_SRC;
        APBC->CHA_SRC_INC   = APBC_INC_0;
        APBC->CHA_SRC_REQ   = ulSrcAdr;
    }
    else
    {
        APBC->CHA_SRC_A     = ulSrcAdr;
        APBC->CHA_SRC_SEL   = APBC_AHB_SRC;
        APBC->CHA_SRC_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHA_SRC_REQ   = 0;
    }
    
    // Destination Setup
    if(ulDestAdr < 10)
    {
        APBC->CHA_DST_A     = ulAPBC_DataPortAddr[ulDestAdr];
        APBC->CHA_DST_SEL   = APBC_APB_SRC;
        APBC->CHA_DST_INC   = APBC_INC_0;
        APBC->CHA_DST_REQ   = ulDestAdr;
    }
    else
    {
        APBC->CHA_DST_A     = ulDestAdr;
        APBC->CHA_DST_SEL   = APBC_AHB_SRC;
        APBC->CHA_DST_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHA_DST_REQ   = 0;
    }
    
    APBC->CHA_BURST_MODE    = tBurst;
    APBC->CHA_SIZE          = ulSize;
    APBC->CHA_DATA_WIDTH    = tWidth;
    
    if(pfISR != NULL)
    {
        APBC->CHA_FIN_INTR_EN   = 1;
        APBC->CHA_FIN_FLAG      = 0;
        APBC->CHA_ERR_INTR_EN   = 1;
        APBC->CHA_ERR_FLAG      = 0;
        pAPBC_ChAIsrFunc        = pfISR;
    }
    else
    {
        APBC->CHA_FIN_INTR_EN   = 0;
        APBC->CHA_ERR_INTR_EN   = 0;
    }
}
//------------------------------------------------------------------------------
void APBC_ChB_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR)
{
    // Source Setup
    if(ulSrcAdr < 10)
    {
        APBC->CHB_SRC_A     = ulAPBC_DataPortAddr[ulSrcAdr];
        APBC->CHB_SRC_SEL   = APBC_APB_SRC;
        APBC->CHB_SRC_INC   = APBC_INC_0;
        APBC->CHB_SRC_REQ   = ulSrcAdr;
    }
    else
    {
        APBC->CHB_SRC_A     = ulSrcAdr;
        APBC->CHB_SRC_SEL   = APBC_AHB_SRC;
        APBC->CHB_SRC_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHB_SRC_REQ   = 0;
    }
    
    // Destination Setup
    if(ulDestAdr < 10)
    {
        APBC->CHB_DST_A     = ulAPBC_DataPortAddr[ulDestAdr];
        APBC->CHB_DST_SEL   = APBC_APB_SRC;
        APBC->CHB_DST_INC   = APBC_INC_0;
        APBC->CHB_DST_REQ   = ulDestAdr;
    }
    else
    {
        APBC->CHB_DST_A     = ulDestAdr;
        APBC->CHB_DST_SEL   = APBC_AHB_SRC;
        APBC->CHB_DST_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHB_DST_REQ   = 0;
    }
    
    APBC->CHB_BURST_MODE    = tBurst;
    APBC->CHB_SIZE          = ulSize;
    APBC->CHB_DATA_WIDTH    = tWidth;
    
    if(pfISR != NULL)
    {
        APBC->CHB_FIN_INTR_EN   = 1;
        APBC->CHB_FIN_FLAG      = 0;
        APBC->CHB_ERR_INTR_EN   = 1;
        APBC->CHB_ERR_FLAG      = 0;
        pAPBC_ChBIsrFunc        = pfISR;
    }
    else
    {
        APBC->CHB_FIN_INTR_EN   = 0;
        APBC->CHB_ERR_INTR_EN   = 0;
    }
}
//------------------------------------------------------------------------------
void APBC_ChC_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR)
{
    // Source Setup
    if(ulSrcAdr < 10)
    {
        APBC->CHC_SRC_A     = ulAPBC_DataPortAddr[ulSrcAdr];
        APBC->CHC_SRC_SEL   = APBC_APB_SRC;
        APBC->CHC_SRC_INC   = APBC_INC_0;
        APBC->CHC_SRC_REQ   = ulSrcAdr;
    }
    else
    {
        APBC->CHC_SRC_A     = ulSrcAdr;
        APBC->CHC_SRC_SEL   = APBC_AHB_SRC;
        APBC->CHC_SRC_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHC_SRC_REQ   = 0;
    }
    
    // Destination Setup
    if(ulDestAdr < 10)
    {
        APBC->CHC_DST_A     = ulAPBC_DataPortAddr[ulDestAdr];
        APBC->CHC_DST_SEL   = APBC_APB_SRC;
        APBC->CHC_DST_INC   = APBC_INC_0;
        APBC->CHC_DST_REQ   = ulDestAdr;
    }
    else
    {
        APBC->CHC_DST_A     = ulDestAdr;
        APBC->CHC_DST_SEL   = APBC_AHB_SRC;
        APBC->CHC_DST_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHC_DST_REQ   = 0;
    }
    
    APBC->CHC_BURST_MODE    = tBurst;
    APBC->CHC_SIZE          = ulSize;
    APBC->CHC_DATA_WIDTH    = tWidth;
    
    if(pfISR != NULL)
    {
        APBC->CHC_FIN_INTR_EN   = 1;
        APBC->CHC_FIN_FLAG      = 0;
        APBC->CHC_ERR_INTR_EN   = 1;
        APBC->CHC_ERR_FLAG      = 0;
        pAPBC_ChCIsrFunc        = pfISR;
    }
    else
    {
        APBC->CHC_FIN_INTR_EN   = 0;
        APBC->CHC_ERR_INTR_EN   = 0;
    }
}
//------------------------------------------------------------------------------
void APBC_ChD_Setup(uint32_t ulSrcAdr, uint32_t ulDestAdr, uint32_t ulSize, APBC_DMA_WIDTH_t tWidth, APBC_BURST_t tBurst, APBC_IsrHandler pfISR)
{
    // Source Setup
    if(ulSrcAdr < 10)
    {
        APBC->CHD_SRC_A     = ulAPBC_DataPortAddr[ulSrcAdr];
        APBC->CHD_SRC_SEL   = APBC_APB_SRC;
        APBC->CHD_SRC_INC   = APBC_INC_0;
        APBC->CHD_SRC_REQ   = ulSrcAdr;
    }
    else
    {
        APBC->CHD_SRC_A     = ulSrcAdr;
        APBC->CHD_SRC_SEL   = APBC_AHB_SRC;
        APBC->CHD_SRC_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHD_SRC_REQ   = 0;
    }
    
    // Destination Setup
    if(ulDestAdr < 10)
    {
        APBC->CHD_DST_A     = ulAPBC_DataPortAddr[ulDestAdr];
        APBC->CHD_DST_SEL   = APBC_APB_SRC;
        APBC->CHD_DST_INC   = APBC_INC_0;
        APBC->CHD_DST_REQ   = ulDestAdr;
    }
    else
    {
        APBC->CHD_DST_A     = ulDestAdr;
        APBC->CHD_DST_SEL   = APBC_AHB_SRC;
        APBC->CHD_DST_INC   = (tWidth == APBC_DMA_32BIT) ? APBC_INC_4 : (tWidth == APBC_DMA_16BIT) ? APBC_INC_2 : APBC_INC_1;
        APBC->CHD_DST_REQ   = 0;
    }
    
    APBC->CHD_BURST_MODE    = tBurst;
    APBC->CHD_SIZE          = ulSize;
    APBC->CHD_DATA_WIDTH    = tWidth;
    
    if(pfISR != NULL)
    {
        APBC->CHD_FIN_INTR_EN   = 1;
        APBC->CHD_FIN_FLAG      = 0;
        APBC->CHD_ERR_INTR_EN   = 1;
        APBC->CHD_ERR_FLAG      = 0;
        pAPBC_ChDIsrFunc        = pfISR;
    }
    else
    {
        APBC->CHD_FIN_INTR_EN   = 0;
        APBC->CHD_ERR_INTR_EN   = 0;
    }
}
//------------------------------------------------------------------------------
void APBC_ISR(void)
{
    INTC_IrqClear(INTC_APB_IRQ);

    if(APBC->CHA_FIN_FLAG)
    {
        APBC->CHA_FIN_FLAG = 0;
        if(pAPBC_ChAIsrFunc != NULL)
            pAPBC_ChAIsrFunc(APBC_FINISH);
    }

    if(APBC->CHB_FIN_FLAG)
    {
        APBC->CHB_FIN_FLAG = 0;
        if(pAPBC_ChBIsrFunc != NULL)
            pAPBC_ChBIsrFunc(APBC_FINISH);
    }

    if(APBC->CHC_FIN_FLAG)
    {
        APBC->CHC_FIN_FLAG = 0;
        if(pAPBC_ChCIsrFunc != NULL)
            pAPBC_ChCIsrFunc(APBC_FINISH);
    }

    if(APBC->CHD_FIN_FLAG)
    {
        APBC->CHD_FIN_FLAG = 0;
        if(pAPBC_ChDIsrFunc != NULL)
            pAPBC_ChDIsrFunc(APBC_FINISH);
    }

    if(APBC->CHA_ERR_FLAG)
    {
        APBC->CHA_ERR_FLAG = 0;
        if(pAPBC_ChAIsrFunc != NULL)
            pAPBC_ChAIsrFunc(APBC_ERROR);
    }

    if(APBC->CHB_ERR_FLAG)
    {
        APBC->CHB_ERR_FLAG = 0;
        if(pAPBC_ChBIsrFunc != NULL)
            pAPBC_ChBIsrFunc(APBC_ERROR);
    }

    if(APBC->CHC_ERR_FLAG)
    {
        APBC->CHC_ERR_FLAG = 0;
        if(pAPBC_ChCIsrFunc != NULL)
            pAPBC_ChCIsrFunc(APBC_ERROR);
    }

    if(APBC->CHD_ERR_FLAG)
    {
        APBC->CHD_ERR_FLAG = 0;
        if(pAPBC_ChDIsrFunc != NULL)
            pAPBC_ChDIsrFunc(APBC_ERROR);
    }
}
