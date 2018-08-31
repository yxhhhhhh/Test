/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		INTC.c
	\brief		Interrupt Controller
	\author		Nick Huang
	\version	1
	\date		2016/09/06
	\copyright	Copyright (C) 2016 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "INTC.h"

__attribute__((section("D_SRAM"))) INTC_IrqHandler INTC_IrqFP[64];													//!< IRQ function pointer
#ifdef FIQ_ENABLE
INTC_FiqHandler INTC_FiqFP[32];													//!< FIQ function pointer
#endif
//------------------------------------------------------------------------------
void INTC_IrqSetup(INTC_IrqSrc_t irqSrc, INTC_TrigMode_t trigMode, INTC_IrqHandler pfISR)
{
	if(pfISR == NULL) return;													//!< Check whether ISR function valid or not
	INTC_IrqFP[irqSrc] = pfISR;

	if(irqSrc < 32)
	{
		if(trigMode)
			SET_BIT(INTC->IRQ0_TRG_MODE, irqSrc);
		else
			CLR_BIT(INTC->IRQ0_TRG_MODE, irqSrc);
	}
	else
	{
		irqSrc -= 32;
		if(trigMode)
			SET_BIT(INTC->IRQ1_TRG_MODE, irqSrc);
		else
			CLR_BIT(INTC->IRQ1_TRG_MODE, irqSrc);
	}
}
//------------------------------------------------------------------------------
void INTC_IrqEnable(INTC_IrqSrc_t irqSrc)
{
	if(irqSrc < 32)
	{
		SET_BIT(INTC->IRQ0_EN, irqSrc);
	}
	else
	{
		irqSrc -= 32;
		SET_BIT(INTC->IRQ1_EN, irqSrc);
	}
}
//------------------------------------------------------------------------------
void INTC_IrqDisable(INTC_IrqSrc_t irqSrc)
{
	if(irqSrc < 32)
	{
		CLR_BIT(INTC->IRQ0_EN, irqSrc);
	}
	else
	{
		irqSrc -= 32;
		CLR_BIT(INTC->IRQ1_EN, irqSrc);
	}
}
//------------------------------------------------------------------------------
void INTC_IrqClear(INTC_IrqSrc_t irqSrc)
{
	uint32_t ulIrq;
	
	if(irqSrc < 32)
	{
		ulIrq = ((uint32_t) 1) << irqSrc;
		INTC->CLR_IRQ0 = ulIrq;
	}
	else
	{
		irqSrc -= 32;
		ulIrq = ((uint32_t) 1) << irqSrc;
		INTC->CLR_IRQ1 = ulIrq;
	}
}
//------------------------------------------------------------------------------
__attribute__((section("PG_SRAM"))) void INTC_IrqChk(void)
{
   uint32_t ulEvent;
   uint8_t ubi;
   
   for (ubi=0; ubi<INTC_MAX_IRQ; ++ubi)
   {
	 ulEvent = INTC->IRQ0_FLAG;
	 if (ulEvent & (((uint32_t) 1) << INTC_RF1_IRQ))
	 {
		if(INTC_IrqFP[INTC_RF1_IRQ] != NULL)
				   INTC_IrqFP[INTC_RF1_IRQ]();
	 }
	 if (INTC_RF1_IRQ != ubi && NULL != INTC_IrqFP[ubi])
	 {
		if(ubi < 32)
		{
				   if(ulEvent & (((uint32_t) 1) << ubi))
							 INTC_IrqFP[ubi]();
		}
		else
		{
				   if(INTC->IRQ1_FLAG & (((uint32_t) 1) << (ubi - 32)))
							 INTC_IrqFP[ubi]();
		}
	 }
   }
}
//------------------------------------------------------------------------------
__attribute__((section("PG_SRAM"))) void INTC_IRQ_Handler(void)
{
    INTC_IrqChk();
}
#ifdef FIQ_ENABLE
//------------------------------------------------------------------------------
void INTC_FiqSetup(INTC_FiqSrc_t fiqSrc, INTC_TrigMode_t trigMode, INTC_FiqHandler pfISR)
{
	if(pfISR == NULL) return;													//!< Check whether ISR function valid or not
	INTC_FiqFP[fiqSrc] = pfISR;

	if(trigMode)
		SET_BIT(INTC->FIQ_TRG_MODE, fiqSrc);
	else
		CLR_BIT(INTC->FIQ_TRG_MODE, fiqSrc);
}
//------------------------------------------------------------------------------
void INTC_FiqEnable(INTC_FiqSrc_t fiqSrc)
{
	SET_BIT(INTC->FIQ_EN, fiqSrc);
}
//------------------------------------------------------------------------------
void INTC_FiqDisable(INTC_FiqSrc_t fiqSrc)
{
	CLR_BIT(INTC->FIQ_EN, fiqSrc);
}
//------------------------------------------------------------------------------
void INTC_FiqClear(INTC_FiqSrc_t fiqSrc)
{
	uint32_t ulFiq;
	
	ulFiq = ((uint32_t) 1) << fiqSrc;
	INTC->CLR_FIQ = ulFiq;
}
//------------------------------------------------------------------------------
void INTC_FiqChk(INTC_FiqSrc_t fiqSrc)
{
	if(INTC->FIQ_FLAG & (((uint32_t) 1) << fiqSrc))
		if(INTC_FiqFP[fiqSrc] != NULL)
			INTC_FiqFP[fiqSrc]();
}
//------------------------------------------------------------------------------
void INTC_FIQ_Handler(void)
{
	//! FIQ check order can be changed
	INTC_FiqChk(INTC_CT16B0_FIQ);
	INTC_FiqChk(INTC_CT16B1_FIQ);
	INTC_FiqChk(INTC_CT16B2_FIQ);
	INTC_FiqChk(INTC_CT16B3_FIQ);
	INTC_FiqChk(INTC_CT32B0_FIQ);
	INTC_FiqChk(INTC_CT32B1_FIQ);
	INTC_FiqChk(INTC_CT32B2_FIQ);
	INTC_FiqChk(INTC_CT32B3_FIQ);
	INTC_FiqChk(INTC_CT32B4_FIQ);
	INTC_FiqChk(INTC_RF1_FIQ);
	INTC_FiqChk(INTC_RF2_FIQ);
	INTC_FiqChk(INTC_GPIO_FIQ);
}
//------------------------------------------------------------------------------
#endif
