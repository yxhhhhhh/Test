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
;   \file       osMacro.s
;   \brief      macro definition for RTOS
;   \author     Nick Huang
;   \version    0.7
;   \date       2017/07/18
;   \copyright  Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
;*/
;-------------------------------------------------------------------------------
	IMPORT  osCritical
	IMPORT	osCurrentTCB
;-------------------------------------------------------------------------------
	MACRO
	osRestoreContext

	LDR		R0, =osCurrentTCB
	LDR		R0, [R0]
	LDR		LR, [R0]

	LDR		R0, =osCritical
	LDMFD	LR!, {R1}
	STR		R1, [R0]

	LDMFD	LR!, {R0}
	MSR		SPSR_cxsf, R0

	LDMFD	LR, {R0-R14}^
	NOP

	LDR		LR, [LR, #+60]

	SUBS	PC, LR, #4

	MEND
;-------------------------------------------------------------------------------
	MACRO
	osSaveContext

	STMDB 	SP!, {R0}

	STMDB	SP,{SP}^
	NOP
	SUB		SP, SP, #4
	LDMIA	SP!,{R0}

	STMDB	R0!, {LR}
	MOV		LR, R0
	LDMIA	SP!, {R0}

	STMDB	LR,{R0-LR}^
	NOP
	SUB		LR, LR, #60

	MRS		R0, SPSR
	STMDB	LR!, {R0}

	LDR		R0, =osCritical
	LDR		R0, [R0]
	STMDB	LR!, {R0}

	LDR		R0, =osCurrentTCB
	LDR		R1, [R0]
	STR		LR, [R1]
	
	MEND
;-------------------------------------------------------------------------------
	MACRO
	DISABLE_FIQ

	MRS		R1, CPSR
	ORR		R1, R1, #0x40			; Disable FIQ
	MSR		CPSR_c, R1

	MEND
;-------------------------------------------------------------------------------
	MACRO
	ENABLE_FIQ

	MRS		R1, CPSR
	BIC		R1, R1, #0x40			; Enable FIQ
	MSR		CPSR_c, R1
	
	MEND
;-------------------------------------------------------------------------------
	MACRO
	DISABLE_IRQ

	MRS		R1, CPSR
	ORR		R1, R1, #0x80			; Disable IRQ
	MSR		CPSR_c, R1

	MEND
;-------------------------------------------------------------------------------
	MACRO
	ENABLE_IRQ

	MRS		R1, CPSR
	BIC		R1, R1, #0x80			; Enable IRQ
	MSR		CPSR_c, R1
	
	MEND
;-------------------------------------------------------------------------------
	MACRO
	DISABLE_INT

	MRS		R1, CPSR
	ORR		R1, R1, #0xC0			; Disable INT (FIQ + IRQ)
	MSR		CPSR_c, R1

	MEND
;-------------------------------------------------------------------------------
	MACRO
	ENABLE_INT

	MRS		R1, CPSR
	BIC		R1, R1, #0xC0			; Enable INT (FIQ + IRQ)
	MSR		CPSR_c, R1
	
	MEND
;-------------------------------------------------------------------------------
	END
