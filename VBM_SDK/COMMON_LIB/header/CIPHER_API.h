/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		CIPHER_API.h
	\brief		Cipher API Header File
	\author		Hanyi Chiu
	\version	0.2
	\date		2017/03/08
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _CIPHER_API_H_
#define _CIPHER_API_H_

#include <stdint.h>

typedef enum
{
	AES_DES_3DES_MODE = 0,			//!< AES/DES mode
	RC4_MODE,						//!< RC4 mode
}CIPHER_MODE_t;

typedef enum
{
	AES_ENC = 0,    				//!< AES encryption
	AES_DEC,						//!< AES descryption
	DES_ENC = 4,    				//!< DES encryption
	DES_DEC,						//!< DES descryption
	DES3_ENC = 0xC,    				//!< 3DES encryption
	DES3_DEC,						//!< 3DES descryption
}AES_DES_FUNC_t;

typedef enum
{
	ECB = 0,
	CBC,
	CFB,
	OFB
}AES_DES_MODE_t;

typedef enum
{
	KEY_128BITS = 0,
	KEY_192BITS,
	KEY_256BITS
}AES_DES_KEYLEN_t;

typedef enum
{
	RC4_SBOX_SIZE2 = 0,
	RC4_SBOX_SIZE4,
	RC4_SBOX_SIZE8,
	RC4_SBOX_SIZE16,
}RC4_SBOX_SIZE;

//------------------------------------------------------------------------
/*!
\brief Initial Cipher
\return(no)
*/
void CIPHER_Init(void);
//------------------------------------------------------------------------------
/*!
\brief AES or DES crypto function
\param tAES_DES_Mode 		AES or DES mode.
\param tFunc 				Crypto function
\param tKeyLen				Key length
\param pKeyIn				Key
\param pIVIn				Initialization vector
\param ulReadAddr 			Source address
\param ulWriteAddr 			Destination address
\param ulData_Len			Data length
\return(no)
*/
void CIPHER_Crypto(AES_DES_MODE_t tAES_DES_Mode, AES_DES_FUNC_t tFunc, AES_DES_KEYLEN_t tKeyLen, uint8_t *pKeyIn, uint8_t *pIVIn, uint32_t ulReadAddr, uint32_t ulWriteAddr, uint32_t ulData_Len);
//------------------------------------------------------------------------
/*!
\brief RC4 crypto function
\param pSBox 				SBOX
\param SBoxSize 			SBOX size
\param ulReadAddr 			Source address
\param ulWriteAddr 			Destination address
\param ulData_Len			Data length
\return(no)
*/
void RC4_Crypto(uint8_t *pSBox, RC4_SBOX_SIZE SBoxSize, uint32_t ulReadAddr, uint32_t ulWriteAddr, uint32_t ulData_Len);
//------------------------------------------------------------------------
/*!
\brief 	Get Cipher Version	
\return	Version
*/
uint16_t uwCIPHER_GetVersion(void);

#endif
