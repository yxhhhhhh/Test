;/*!
;	The information contained herein is the exclusive property of SONiX and
;	shall not be distributed, or disclosed in whole or in part without prior
;	permission of SONiX.
;	SONiX reserves the right to make changes without further notice to the
;	product to improve reliability, function or design. SONiX does not assume
;	any liability arising out of the application or use of any product or
;	circuits described herein. All application information is advisor and does
;	not from part of the specification.
;
;	\file		startup_510PF.s
;	\brief		startup file for 510PF
;	\author		Nick Huang
;	\version	0.3
;	\date		2017/07/18
;	\copyright	Copyright (C) 2017 SONiX Technology Co., Ltd. All rights reserved.
;*/

;/* <<< Use Configuration Wizard in Context Menu >>>                          */ 

; Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

Mode_USR		EQU		0x10
Mode_FIQ		EQU		0x11
Mode_IRQ		EQU		0x12
Mode_SVC		EQU		0x13
Mode_ABT		EQU		0x17
Mode_UND		EQU		0x1B
Mode_SYS		EQU		0x1F

I_Bit			EQU		0x80								; when I bit is set, IRQ is disabled
F_Bit			EQU		0x40								; when F bit is set, FIQ is disabled

;// <h> Stack Configuration (Stack Sizes in Bytes)
;//   <o0> Undefined Mode      <0x0-0xFFFFFFFF:8>
;//   <o1> Supervisor Mode     <0x0-0xFFFFFFFF:8>
;//   <o2> Abort Mode          <0x0-0xFFFFFFFF:8>
;//   <o3> Fast Interrupt Mode <0x0-0xFFFFFFFF:8>
;//   <o4> Interrupt Mode      <0x0-0xFFFFFFFF:8>
;//   <o5> User/System Mode    <0x0-0xFFFFFFFF:8>
;// </h>

UND_Stack_Size	EQU		0x00000100
SVC_Stack_Size	EQU		0x00000400
ABT_Stack_Size	EQU		0x00000100
FIQ_Stack_Size	EQU		0x00000100
IRQ_Stack_Size	EQU		0x00000200
USR_Stack_Size	EQU		0x00000400

ISR_Stack_Size	EQU		(UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
						FIQ_Stack_Size + IRQ_Stack_Size)

				AREA	STACK, NOINIT, READWRITE, ALIGN=3

Stack_Mem		SPACE	USR_Stack_Size
__initial_sp	SPACE	ISR_Stack_Size
Stack_Top

;// <h> Heap Configuration
;//   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF>
;// </h>

Heap_Size		EQU		0x00004000

				AREA	HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem		SPACE	Heap_Size
__heap_limit

;// <h> System Clock Configuration
;//   <o2.0..1>   SYS_RATE, System clock source frequency
;//                 <0=> 480MHz (120MHz in FPGA)
;//                 <1=> 240MHz (60MHz in FPGA)
;//                 <2=> 120MHz (30MHz in FPGA)
;//                 <3=> 60MHz (15MHz in FPGA)
;//   <o2.8..11>  CPU_RATE, CPU clock frequency control <2-15>
;//                 <i> CPU_CLK = DDR_PLL / 2^SYS_RATE / CPU_RATE, if CPU_PLL_SEL = 0
;//                 <i> CPU_CLK = USB_HOST_PLL / 2^SYS_RATE / CPU_RATE, if CPU_PLL_SEL = 1
;//                 <i> Note: CPU_CLK = 25MHz in FPGA
;//   <o3.0>      CPU_PLL_SEL, CPU PLL selection
;//                 <0=> DDR PLL  <1=> USB Host PLL
;//   <o2.4..7>   DDR_RATE, DDR clock frequency <2-15>
;//                 <i> DDR_CLK = DDR_PLL / 2^SYS_RATE / DDR_RATE
;//                 <i> Note: DDR_PLL is 200MHz in FPGA & DDR_CLK is fixed to 25MHz
;//   <o3.8..14>  DDR_DQS0_IDLY, DDR DQS 0 strobe delay <0-127>
;//                 <i> Note: Only active on FPGA
;//   <o3.16..22> DDR_DQS1_IDLY, DDR DQS 1 strobe delay <0-127>
;//                 <i> Note: Only active on FPGA
;//   <o3.24..27> DDR_DQS0_ODLY, DDR DQS 0 output delay <0-15>
;//                 <i> Note: Only active on FPGA
;//   <o3.28..31> DDR_DQS1_ODLY, DDR DQS 1 output delay <0-15>
;//                 <i> Note: Only active on FPGA
;//   <o2.12..15> H246_RATE, H.264 clock frequency <2-15>
;//                 <i> H246_CLK = H264_PLL / 2^SYS_RATE / H264_RATE
;//   <o2.16..19> IMG_RATE, Image Controller clock frequency <2-15>
;//                 <i> IMG_CLK = DDR_PLL / 2^SYS_RATE / IMG_RATE
;//   <o2.20..21> EROM_CK_DIV, External Flash ROM clock frequency
;//                 <0=> CPU_CLK / 2  <1=> CPU_CLK / 4
;//                 <2=> CPU_CLK / 8  <3=> CPU_CLK / 16
;//   <o2.22>     UART_RATE, UART clock frequency
;//                 <0=> 10MHz  <1=> 96MHz
;//   <o2.24..31> SAR_RATE, SAR ADC clock frequency <1-255>
;//                 <i> SAR clock = 6MHz / (2 * SAR_RATE)
;//   <o3.23..31> PWM_RATE, PWM clock frequency <2-511>
;//                 <i> PWM clock = 96MHz / PWM_RATE
;//   <o1.0..3>   JPG_RATE, JPEG Codec clock frquency <2-15>
;//                 <i> F(JPG_CLK) = DDR_PLL / 2^SYS_RATE / JPG_RATE
;//   <o1.4..7>   ISP_RATE, ISP Codec clock freqyency <2-15>
;//                 <i> F(ISP_CLK) = DDR_PLL / 2^SYS_RATE / ISP_RATE
;//   <o3.3..6>   ISP_AXI_RATE, ISP AXI bus clock frequency control <2-15>
;//                 <i> F(ISP_AXI_CLK) = DDR_PLL / 2^SYS_RATE / ISP_AXI_RATE
;//   <o1.8..11>  LCD_RATE, LCD control clock frequency <2-15>
;//                 <i> F(LCD_CLK) = H264_PLL / 2^SYS_RATE / LCD_RATE
;//   <o1.12..15> LCD_PIX_RATE, LCD pixel clock frequency <2-15>
;//                 <i> F(TG_MST_CLK) = LCD_PLL / LCD_PIX_RATE
;//   <o1.16..19> AHB1_RATE, AHB bus 1 clock frequency <2-15>
;//                 <i> F(AHB1_CLK) = USB_HOST_PLL / 2^SYS_RATE / AHB1_RATE
;//   <o1.20..23> AHB2_RATE, AHB bus 2 clock frequency <2-15>
;//                 <i> F(AHB2_CLK) = USB_HOST_PLL / 2^SYS_RATE / AHB2_RATE
;//   <o1.24..27> AHB3_RATE, AHB bus 3 clock frequency <2-15>
;//                 <i> F(AHB3_CLK) = DDR_PLL / 2^SYS_RATE / AHB3_RATE
;//   <o1.28..31> APBC_RATE, APBC clock frequency <2-15>
;//                 <i> F(APBC_CLK) = USB_HOST_PLL / 2^SYS_RATE / APBC_RATE
;// </h>
R9000_0000H		EQU		0x00000008
R9000_0004H		EQU		0xF468FFAA
R9000_0008H		EQU		0x011FF220
R9000_000CH		EQU		0x002E2EA8


;// <h> Watch Dog Timer 1 Configuration
;//   <o0>   WDOG_COUNTER, The Watch Dog Counter Value
;//            <i> The watchdog counter starts to decrease once WDOG_EN = 1
;//            <i> The watchdog counter holds the value when WDOG_EN = 0
;//   <o1>   WDOG_LOAD, The Watch Dog Counter reload value
;//   <o2.0> WDOG_EN, Enable watchdog timer
;//            <0=> Disable  <1=> Enable
;//   <o2.1> WDOG_RST_EN, Enable watchdog timer reset system
;//            <0=> Disable  <1=> Enable
;//   <o2.2> WDOG_INTR_EN, Enable watchdog timer system interrupt
;//            <0=> Disable  <1=> Enable
;//   <o2.3> WDOG_EXT_EN, Enable watchdog timer external signal
;//            <0=> Disable  <1=> Enable
;//   <o2.4> WDOG_CLOCK, watchdog timer clock source selection
;//            <0=> APBC_CLK  <1=> EXTCLK = 10MHz
;// </h>
R9290_0000H		EQU		0x00078000
R9290_0004H		EQU		0x00078000
R9290_000CH		EQU		0x00000010

;// <h> Watch Dog Timer 2 Configuration
;//   <o0>   WDOG_COUNTER, The Watch Dog Counter Value
;//            <i> The watchdog counter starts to decrease once WDOG_EN = 1
;//            <i> The watchdog counter holds the value when WDOG_EN = 0
;//   <o1>   WDOG_LOAD, The Watch Dog Counter reload value
;//   <o2.0> WDOG_EN, Enable watchdog timer
;//            <0=> Disable  <1=> Enable
;//   <o2.1> WDOG_RST_EN, Enable watchdog timer reset system
;//            <0=> Disable  <1=> Enable
;//   <o2.2> WDOG_INTR_EN, Enable watchdog timer system interrupt
;//            <0=> Disable  <1=> Enable
;//   <o2.3> WDOG_EXT_EN, Enable watchdog timer external signal
;//            <0=> Disable  <1=> Enable
;//   <o2.4> WDOG_CLOCK, watchdog timer clock source selection
;//            <0=> APBC_CLK  <1=> EXTCLK = 10MHz
;// </h>
R92A0_0000H		EQU		0x03EF1480
R92A0_0004H		EQU		0x03EF1480
R92A0_000CH		EQU		0x00000010

				IMPORT	INTC_IRQ_Handler

				IF		:DEF:FIQ_ENABLE
				IMPORT	INTC_FIQ_Handler
				ENDIF

				IF		:DEF:RTOS
				INCLUDE osMacro.s
				IMPORT	osContextSwitch
				ENDIF

				PRESERVE8

; Area Definition and Entry Point
; Startup Code must be linked first at Address at which it expects to run.

		AREA	RESET, CODE, READONLY
				ARM

				EXPORT	StartCode
StartCode

Vectors			LDR		PC, Reset_Addr
				LDR		PC, Undef_Addr
				LDR		PC, SWI_Addr
				LDR		PC, PAbt_Addr
				LDR		PC, DAbt_Addr
				NOP											; Reserved Vector
				LDR		PC, IRQ_Addr
FIQ_Handler
				IF		:DEF:FIQ_ENABLE
				IF		:DEF:RTOS
				osSaveContext
				ENDIF

				;IMPORT	INTC_FIQ_Handler
				;LDR		R0, =INTC_FIQ_Handler
				;MOV		LR, PC
				;BX		R0

				IF		:DEF:RTOS
				osRestoreContext
				ENDIF
				ENDIF
				B		FIQ_Handler

Reset_Addr		DCD		Reset_Handler
Undef_Addr		DCD		Undef_Handler
				IF		:DEF:RTOS
SWI_Addr		DCD		osContextSwitch
				ELSE
SWI_Addr		DCD		SWI_Handler
				ENDIF
PAbt_Addr		DCD		PAbt_Handler
DAbt_Addr		DCD		DAbt_Handler
				DCD		2									; Reserved Address
IRQ_Addr		DCD		IRQ_Handler
FIQ_Addr		DCD		FIQ_Handler

Undef_Handler	B		Undef_Handler
SWI_Handler		B		SWI_Handler
PAbt_Handler	B		PAbt_Handler
DAbt_Handler	B		DAbt_Handler
IRQ_Handler
				IF		:DEF:RTOS
				;DISABLE_FIQ
				osSaveContext
				ELSE
				;STMDB	SP!, {LR}
				SUB     LR, LR, #4
				STMFD   SP!, {R0-R12, LR}
				ENDIF

				;LDR		R0, =INTC_IRQ_Handler
				;MOV		LR, PC
				;BX		R0
				BL		INTC_IRQ_Handler

				IF		:DEF:RTOS
				osRestoreContext
				;ENABLE_FIQ
				ELSE
				;LDMIA	SP!, {LR}
				;SUBS	PC, LR, #4
				LDMFD   SP!, {R0-R12,PC}^
				ENDIF

; Reset_Handler
				EXPORT	Reset_Handler
Reset_Handler

;System Clock Initialize
				LDR		R0, =0x9000000C
				LDR		R1, [R0]
				BIC 	R1, R1, #0x1
				STR		R1, [R0]
				LDR		R0, =0x90000008
				LDR		R1, [R0]
				BIC 	R1, R1, #0x00000F00
				ORR		R1, R1, #0x00000200
				STR		R1, [R0]
				;LDR		R0, =0x90000004
				;LDR		R1, =R9000_0004H
				;LDR		R2, =R9000_0008H
				;LDR		R3, =R9000_000CH
				;STMIA	R0!, {R1-R3}

;; Watch Dog Timer 1 Initialize
				;LDR		R0, =0x92900000
				;LDR		R1, =R9290_0000H
				;LDR		R2, =R9290_0004H
				;LDR		R3, =0
				;LDR		R4, =R9290_000CH
				;STMIA	R0!, {R1-R4}

;; Watch Dog Timer 2 Initialize
				;LDR		R0, =0x92A00000
				;LDR		R1, =R92A0_0000H
				;LDR		R2, =R92A0_0004H
				;LDR		R3, =0
				;LDR		R4, =R92A0_000CH
				;STMIA	R0!, {R1-R4}

; Setup Stack for each mode ----------------------------------------------------

				LDR		R0, =Stack_Top

; Enter Undefined Instruction Mode and set its Stack Pointer
				MSR		CPSR_c, #Mode_UND:OR:I_Bit:OR:F_Bit
				MOV		SP, R0
				SUB		R0, R0, #UND_Stack_Size

; Enter Abort Mode and set its Stack Pointer
				MSR		CPSR_c, #Mode_ABT:OR:I_Bit:OR:F_Bit
				MOV		SP, R0
				SUB		R0, R0, #ABT_Stack_Size

; Enter FIQ Mode and set its Stack Pointer
				MSR		CPSR_c, #Mode_FIQ:OR:I_Bit:OR:F_Bit
				MOV		SP, R0
				SUB		R0, R0, #FIQ_Stack_Size

; Enter IRQ Mode and set its Stack Pointer
				MSR		CPSR_c, #Mode_IRQ:OR:I_Bit:OR:F_Bit
				MOV		SP, R0
				SUB		R0, R0, #IRQ_Stack_Size

; Enter Supervisor Mode and set its Stack Pointer
				MSR		CPSR_c, #Mode_SVC:OR:I_Bit:OR:F_Bit
				MOV		SP, R0
				SUB		R0, R0, #SVC_Stack_Size

; Enter the C code -------------------------------------------------------------
				IMPORT	__main
				LDR		R0, =__main
				BX		R0


				IF		:DEF:__MICROLIB

				EXPORT	__heap_base
				EXPORT	__heap_limit

				ELSE
; User Initial Stack & Heap
				AREA	|.text|, CODE, READONLY

				IMPORT	__use_two_region_memory
				EXPORT	__user_initial_stackheap
__user_initial_stackheap

				LDR		R0, =  Heap_Mem						; Heap Base
				LDR		R1, =(Stack_Mem + USR_Stack_Size)	; Stack Base
				LDR		R2, = (Heap_Mem +      Heap_Size)	; Heap Limit
				LDR		R3, = Stack_Mem						; Stack Limit
				BX		LR
				ENDIF

				END
