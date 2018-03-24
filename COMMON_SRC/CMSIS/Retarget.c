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
	\file		Retarget.c
	\brief		for target-dependent low level functions
	\author		Nick Huang
	\version	1
	\date		2016/11/08
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <rt_misc.h>
#include <time.h>
#include "_510PF.h"

#pragma import(__use_no_semihosting_swi)

#define scanfBufSz				64
//------------------------------------------------------------------------------
extern int  sendchar(int ch);  /* in Serial.c */
extern int  getkey(void);      /* in Serial.c */
extern long timeval;           /* in Time.c   */

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;


int fputc(int ch, FILE *f) {
  return (sendchar(ch));
}

int fgetc(FILE *f) {
  return (sendchar(getkey()));
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  sendchar (ch);
}


void _sys_exit(int return_code) {
  while (1);    /* endless loop */
}
//------------------------------------------------------------------------------
int sendchar(int ch)
{
	if(ch == '\n')
	{
		while(!UART2->TX_RDY);
		UART2->RS_DATA = '\r';
	}
	else if(ch == '\r')
	{
		while(!UART2->TX_RDY);
		UART2->RS_DATA = '\n';
	}
	while(!UART2->TX_RDY);
	UART2->RS_DATA = ch;
	return ch;
}
//------------------------------------------------------------------------------
int getkey(void)
{
	int iRet;
	
	while(!UART2->RX_RDY);
	iRet = UART2->RS_DATA & 0x0FF;
	
	return iRet;
}
//------------------------------------------------------------------------------
EscKey_t EscParser(char* str, int size)
{
	switch(size) {
		case 1:
			if(str[size-1] != 0x1B)
				return EK_Other;
			break;
		case 2:
			if(str[size-1] != 0x5B)
				return EK_Other;
			break;
		case 3:
			str[3] = 0;
			if(strcmp(str, VT100_CursorRight) == 0)
				return EK_CursorRight;
			if(strcmp(str, VT100_CursorLeft) == 0)
				return EK_CursorLeft;
			return EK_Other;
	}
	return EK_Unfinished;
}
//------------------------------------------------------------------------------
int __scanf(const char* fmt, ...)
{
	va_list ap;
	char buf[scanfBufSz];
	char c;
	int i, j, cnt;
	char EscKey[6];
	uint8_t isEsc = FALSE;

	cnt = EOF;
	memset(buf, 0, scanfBufSz);
	for(i=0;i<scanfBufSz-1;i++)
	{
		c = getkey();

		//! Esc Key
		if(c == 0x1B) {
			isEsc = TRUE;
			j = 0;
		}
		if(isEsc) {
			EscKey[j++] = c;
			switch(EscParser(EscKey, j)) {
				case EK_CursorLeft:
					i -= i?2:1;
					printf(VT100_CursorLeft);
					isEsc = FALSE;
					continue;
				case EK_CursorRight:
					printf(VT100_CursorRight);
					isEsc = FALSE;
					if(!buf[i])
						buf[i] = ' ';
					continue;
				case EK_Other:
					for(j=0;j<100;j++)
						isEsc = UART2->RS_DATA & 0x0FF;
					isEsc = FALSE;
					i -= 1;
					continue;
				case EK_Unfinished:
					i -= 1;
					continue;
			}
		}
		
		//! Backspace key
		if(c == 0x7F) {
			if(i--) {
				buf[i] = 0;
				i --;
				sendchar(c);
			}
			continue;
		}

		//! Shift + Backspace key
		if(c == '\b') {
			if(i--) {
				i --;
				sendchar(c);
			}
			continue;
		}

		sendchar(c);
		if((c == '\r') || (c == '\n')) {
			va_start(ap, fmt);
			cnt = vsscanf(buf, fmt, ap);
			va_end(ap);
			break;
		}
		else
			buf[i] = c;
	}

	return cnt;
}
//------------------------------------------------------------------------------
uint16_t uwScanf (void)
{
	uint8_t ubCnt=0;
	char cBuf[6];
	
	for (ubCnt=0; ubCnt<6; ++ubCnt)
	{
		sendchar(cBuf[ubCnt] = getkey());
		if (cBuf[ubCnt] == '\r')
		{
			cBuf[ubCnt] = '\0';
			break;
		}
		if (cBuf[ubCnt] == '\b')
			ubCnt -= 2;
	}
	return (uint16_t)atoi(cBuf);
}	
//------------------------------------------------------------------------------
