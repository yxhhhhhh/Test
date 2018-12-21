/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		INTC.h
	\brief		Interrupt Controller header file
	\author		Nick Huang
	\version	1
	\date		2016/11/08
	\copyright	Copyright(C) 2016 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _INTC_H_
#define _INTC_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
//------------------------------------------------------------------------------
typedef enum
{
	INTC_TIMER1_IRQ		= 0,
	INTC_TIMER2_IRQ		= 1,
	INTC_TIMER3_IRQ		= 2,
	INTC_USB2DEV_IRQ	= 3,
	INTC_USB2HOST2_IRQ	= 4,
	INTC_USB2HOST1_IRQ	= 5,
	INTC_LCD_IRQ		= 6,
	INTC_MIPI_IRQ		= 7,

	INTC_SEN_HSYNC_IRQ	= 8,
	INTC_ISP_WIN_END_IRQ= 9,
	INTC_SEN_VSYNC_IRQ	= 10,
	INTC_ISP_DIS_IRQ	= 11,
	INTC_IMG_TX_IRQ		= 12,
	INTC_H264_ENCODE_IRQ= 13,
	INTC_JPEG_IRQ		= 14,
	INTC_RF1_IRQ		= 15,

	INTC_RF2_IRQ		= 16,
	INTC_MAC_IRQ		= 17,
	INTC_UART1_IRQ		= 18,
	INTC_UART2_IRQ		= 19,
	INTC_SD1_DET_IRQ	= 20,
	INTC_SD1_IRQ		= 21,
	INTC_SD2_DET_IRQ	= 22,
	INTC_SD2_IRQ		= 23,

	INTC_SD3_IRQ		= 24,
	INTC_SF_IRQ			= 25,
	INTC_CRC_OK_IRQ		= 26,
	INTC_CIPHER_OK_IRQ	= 27,
	INTC_GPIO_IRQ		= 28,
	INTC_DMA_IRQ		= 29,
	INTC_ADO_R_IRQ		= 30,
	INTC_ADO_W_IRQ		= 31,

	INTC_AHB0_AHBC_IRQ	= 32,
	INTC_AHBC1_IRQ		= 33,
	INTC_AHBC2_IRQ		= 34,
	INTC_AHBC3_IRQ		= 35,
	INTC_I2C1_IRQ		= 36,
	INTC_I2C2_IRQ		= 37,
	INTC_I2C3_IRQ		= 38,
	INTC_I2C4_IRQ		= 39,

	INTC_I2C5_IRQ		= 40,
	INTC_SSP_IRQ		= 41,
	INTC_APB_IRQ		= 42,
	INTC_DDR_CTRL_IRQ	= 43,
	INTC_WDOG_IRQ		= 44,
	INTC_RTC_ALARM_IRQ	= 45,
	INTC_RTC_WAKEUP_IRQ	= 46,
	INTC_CMDQUE_IRQ		= 47,

	INTC_KEY_IRQ		= 48,
	INTC_RTC_GPIO0_IRQ	= 49,
	INTC_RTC_GPIO1_IRQ	= 50,
	INTC_REG_RW_ERR_IRQ	= 51,
	INTC_ISP_MD_IRQ		= 52,
	INTC_MAX_IRQ	
} INTC_IrqSrc_t;

#ifdef FIQ_ENABLE
typedef enum
{
	INTC_CT16B0_FIQ		= 0,
	INTC_CT16B1_FIQ		= 1,
	INTC_CT16B2_FIQ		= 2,
	INTC_CT16B3_FIQ		= 3,
	INTC_CT32B0_FIQ		= 4,
	INTC_CT32B1_FIQ		= 5,
	INTC_CT32B2_FIQ		= 6,
	INTC_CT32B3_FIQ		= 7,
	INTC_CT32B4_FIQ		= 8,
	INTC_RF1_FIQ		= 9,
	INTC_RF2_FIQ		= 10,
	INTC_GPIO_FIQ		= 11
} INTC_FiqSrc_t;
#endif

typedef enum
{
	INTC_LEVEL_TRIG = 0,
	INTC_EDGE_TRIG
} INTC_TrigMode_t;

typedef void (*INTC_IrqHandler) (void);
typedef void (*INTC_FiqHandler) (void);
//------------------------------------------------------------------------------
void INTC_IrqSetup(INTC_IrqSrc_t irqSrc, INTC_TrigMode_t trigMode, INTC_IrqHandler pfISR);
void INTC_IrqEnable(INTC_IrqSrc_t irqSrc);
void INTC_IrqDisable(INTC_IrqSrc_t irqSrc);
void INTC_IrqClear(INTC_IrqSrc_t irqSrc);
#ifdef FIQ_ENABLE
void INTC_FiqSetup(INTC_FiqSrc_t fiqSrc, INTC_TrigMode_t trigMode, INTC_FiqHandler pfISR);
void INTC_FiqEnable(INTC_FiqSrc_t fiqSrc);
void INTC_FiqDisable(INTC_FiqSrc_t fiqSrc);
void INTC_FiqClear(INTC_FiqSrc_t fiqSrc);
#endif
//------------------------------------------------------------------------------
#endif
