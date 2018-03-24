/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/
/*!
	\file		Retarget.h
	\brief		low level retarget header file
	\author		Nick Huang
	\version	1
	\date		2016/01/18
*/
//------------------------------------------------------------------------------
#include <stdint.h>
//------------------------------------------------------------------------------
int __scanf(const char * fmt, ...);
uint16_t uwScanf (void);

//! VT100 Escaoe Sequence Commands
//! Reference
//! http://ascii-table.com/ansi-escape-sequences-vt-100.php
#define VT100_ClearScreen	"\x1B\x5B\x32\x4A"
#define VT100_CursorHome	"\x1B\x5B\x3B\x48"
#define VT100_ResetTerm		"\x1B\x63"
#define VT100_CursorRight	"\x1B\x5B\x43"
#define VT100_CursorLeft	"\x1B\x5B\x44"

//! Terminal control macro definitions
#define tClearScreen			printf(VT100_ClearScreen);
#define tCursorHome				printf(VT100_CursorHome);
#define tMoveCursorUp(L)		printf("\x1B\x5B%dA", L)
#define tMoveCursorDown(L)		printf("\x1B\x5B%dB", L)
#define tMoveCursorRight(L)		printf("\x1B\x5B%dC", L)
#define tMoveCursorLeft(L)		printf("\x1B\x5B%dD", L)
#define tMoveCursor(X, Y)		printf("\x1B\x5B%d;%dH", Y, X)
#define tResetTerm				printf(VT100_ResetTerm)

//! EscKey definition
typedef enum
{
	EK_CursorLeft,
	EK_CursorRight,
	EK_Other,
	EK_Unfinished
} EscKey_t;
