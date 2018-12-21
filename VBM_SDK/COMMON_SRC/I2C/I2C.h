/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		I2C.h
	\brief		I2C Funcations Header
	\author		Pierce
	\version	0.2
	\date		2017/03/10
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _I2C_H_
#define _I2C_H_
//------------------------------------------------------------------------------
#include <stdbool.h>
#include "_510PF.h"
/*!	\file I2C.h
DMAC FlowChart:
	\dot
digraph I2C_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		I2C1 [shape=polygon, sides=11, color=blue, fontcolor=blue];
		"I2C2~5" [shape=polygon, sides=11, color=blue, fontcolor=blue];
		"Master IdIe" [shape=polygon, sides=11, color=blue, fontcolor=blue];
		"Slave IdIe" [shape=polygon, sides=11, color=blue, fontcolor=blue];
		"Tranmist Done" [shape=polygon, sides=11, color=blue, fontcolor=blue];		
		I2C1 -> pI2C_MasterInit;
		I2C1 -> pI2C_SlaveInit;
		"I2C2~5" -> pI2C_MasterInit;
		"I2C2~5" -> pI2C_SlaveInit;
		pI2C_MasterInit -> "Master IdIe" [color=blue];
		pI2C_SlaveInit -> "Slave IdIe" [color=blue];
		"Master IdIe" -> bI2C_MasterProcess;
		"Master IdIe" -> bI2C_MasterInt;
		"Slave IdIe" -> bI2C_SlaveWrite;
		"Slave IdIe" -> bI2C_SlaveRead;
		"Slave IdIe" -> bI2C_SlaveIntWrite;
		"Slave IdIe" -> bI2C_SlaveIntRead;
		bI2C_MasterProcess ->"Master IdIe" [color=blue, headport=e, tailport=ne];
		bI2C_MasterInt ->"Master IdIe" [color=blue, headport=w, tailport=n];
		bI2C_SlaveWrite ->"Slave IdIe" [color=blue, headport=w, tailport=n];
		bI2C_SlaveRead ->"Slave IdIe" [color=blue, headport=sw];
		bI2C_SlaveIntWrite ->"Slave IdIe" [color=blue, tailport=nw];
		bI2C_SlaveIntRead ->"Slave IdIe" [color=blue, headport=e, tailport=n];
		bI2C_MasterProcess ->"Tranmist Done" [color=blue];
		bI2C_MasterInt ->"Tranmist Done" [color=blue, headport=w];
		bI2C_SlaveWrite ->"Tranmist Done" [color=blue];
		bI2C_SlaveRead ->"Tranmist Done" [color=blue];	
		bI2C_SlaveIntWrite ->"Tranmist Done" [color=blue];
		bI2C_SlaveIntRead ->"Tranmist Done" [color=blue, headport=e];		
	}
	\enddot
*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define I2C_TIME_OUT				(1000000)
//------------------------------------------------------------------------------
#define I2C_SLAVE_ADDR8				(0x51)
#define I2C_SLAVE_ADDR10			(0x351)
#define I2C_SLAVE_ADDR10_MASK		(0x7800)
#define I2C_SLAVE_ADDR10_HMASK		(I2C_SLAVE_ADDR10_MASK >> 7)
#define I2C_RW_F					(1 << 0)
#define I2C_NACK_F					(1 << 1)
#define I2C_TX_DONE					(1 << 4)
#define I2C_RX_DONE					(1 << 5)
#define I2C_BERR_F					(1 << 6)
#define I2C_STOP_F					(1 << 7)
#define I2C_SAM_F					(1 << 8)
#define I2C_GC_F					(1 << 9)
#define I2C_START_F					(1 << 11)
//------------------------------------------------------------------------------
/*!
	\brief I2C Nth Type
*/
typedef enum
{
	I2C_1,	
	I2C_2,	
	I2C_3,	
	I2C_4,
	I2C_5,
}I2C_TYP;
//------------------------------------------------------------------------------
/*!
	\brief I2C SCL Speed Type
*/
typedef enum
{
	I2C_SCL_100K = 1,	//!< SCL = 100KHz
	I2C_SCL_400K = 4,	//!< SCL = 400KHz
	I2C_SCL_1M = 10		//!< SCL = 1MHz
}I2C_SCL_SPEED_TYP;
//------------------------------------------------------------------------------
/*!
	\brief I2C Slave Mode
*/
typedef enum
{
	I2C_SLAVE,	//!< No support gerneral call
	I2C_GCE		//!< Support gerneral call
}I2C_SLAVE_TYP;
//------------------------------------------------------------------------------
/*!
	\brief I2C Slave Address Mode
*/
typedef enum
{
	I2C_SADDR8,	//!< Slave Address 8-bit
	I2C_SADDR10	//!< Slave Address 10-bit
}I2C_SLAVE_ADDR_TYP;
//------------------------------------------------------------------------------
/*!
	\brief I2C Master Mode
*/
enum
{
	I2C_POLLING,	//!< Polling mode
	I2C_INT,		//!< Interrupt mdoe, but start interrupt disable
	I2C_START_INT	//!< Interrupt mdoe, and start interrupt enable
};
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
enum
{
	I2C_DO_ACK,
	I2C_DO_NACK
};
enum
{
	I2C_DATA_NEXT,
	I2C_DATA_STOP
};
typedef enum
{
	I2C_SLAVE_RD,
	I2C_SLAVE_WR
}I2C_SLAVE_RW_TYP;
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Master Initial Function
	\param[in]	tNth 	  Selet Nth I2C (I2C1~5)
	\param[in]	tSclSpeed Set SCL speed (100KHz, 400KHz, 1MHz)
	\return     I2C register address point
	\par [Example]
	\code
		 I2C1_Type *pI2C;

		 pI2C = pI2C_MasterInit (I2C_1, I2C_SCL_400K);
	\endcode
*/
I2C1_Type *pI2C_MasterInit (I2C_TYP tNth, I2C_SCL_SPEED_TYP tSclSpeed);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Master Process (RW for CPU polling) Function
	\param[in]	pI2C 	  I2C register address point
	\param[in]	ubSlvAddr Slave device address
	\param[in]	pWrBuf	  Write Data (NULL when ulWrSz = 0)
	\param[in]	ulWrSz	  Write Data length
	\param[out]	pRdBuf	  Read Data (NULL when ulRdSz = 0)
	\param[in]	ulRdSz	  Read Data length
	\return		true  	  I2C RW ok!
	\return		false  	  I2C write data fail or read data fail!
	\par [Example]
	\code
		 I2C1_Type *pI2C;
		 uint8_t   ubData;
		 uint8_t   pBuf[2] = {0, 1};

		 pI2C = pI2C_MasterInit (I2C_1, I2C_SCL_100K);
		 bI2C_MasterProcess (pI2C, 0x1A, pBuf, 2, &ubData, 1);
	\endcode
*/
bool bI2C_MasterProcess (I2C1_Type *pI2C, uint8_t ubSlvAddr, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Master Interrupt (RW for interrupt) Function
	\param[in]	pI2C 	  I2C register address point
	\param[in]	ubSlvAddr Slave device address
	\param[in]	ubMode	  Interrupt mode, is start interrupt enable?
	\param[in]	pWrBuf	  Write Data (NULL when ulWrSz = 0)
	\param[in]	ulWrSz	  Write Data length
	\param[out]	pRdBuf	  Read Data (NULL when ulRdSz = 0)
	\param[in]	ulRdSz	  Read Data length
	\return		true  	  I2C RW ok!
	\return		false  	  I2C write data fail or read data fail!
	\par [Example]
	\code
		 I2C1_Type *pI2C;
		 uint8_t   ubData;
		 uint8_t   pBuf[2] = {0, 1};

		 pI2C = pI2C_MasterInit (I2C_1, I2C_SCL_400K);
		 bI2C_MasterInt (pI2C, I2C_INT, 0x1A, pBuf, 2, &ubData, 1);
	\endcode
*/
bool bI2C_MasterInt (I2C1_Type *pI2C, uint8_t ubMode, uint8_t ubSlvAddr, uint8_t *pWrBuf, uint32_t ulWrSz, uint8_t *pRdBuf, uint32_t ulRdSz);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Slave Initial Function
	\param[in]	tNth 	  Selet Nth I2C (I2C1~5)
	\param[in]	tGc		  Support general call? (No / Yes)
	\param[in]	tAddrTyp  Slave address type (8-bit / 10-bit)
	\param[in]	uwAddr	  Slave address
	\return     I2C register address point
	\par [Example]
	\code
		 I2C1_Type *pI2C;

		 pI2C = pI2C_SlaveInit (I2C_2, I2C_SLAVE, I2C_SADDR8, 0x51);
	\endcode
*/
I2C1_Type *pI2C_SlaveInit (I2C_TYP tNth, I2C_SLAVE_TYP tGc, I2C_SLAVE_ADDR_TYP tAddrTyp, uint16_t uwAddr);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Slave Write (CPU Polling) Function
	\param[in]	pI2C 	  I2C register address point
	\param[in]	pBuf	  Write data
	\param[in]	ulSz	  Write data length
	\return		true  	  I2C Write ok!
	\return		false  	  I2C write data fail!
	\par [Example]
	\code
		 I2C1_Type *pI2C;
		 uint8_t   pBuf[2] = {0, 1};

		 pI2C = pI2C_SlaveInit (I2C_2, I2C_SLAVE, I2C_SADDR8, 0x51);
		 bI2C_SlaveWrite (pI2C, pBuf, 2):
	\endcode
*/
bool bI2C_SlaveWrite (I2C1_Type *pI2C, uint8_t *pBuf, uint32_t ulSz);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Slave Read (CPU Polling) Function
	\param[in]	pI2C 	  I2C register address point
	\param[out]	pBuf	  Read data
	\param[in]	ulSz	  Read data length
	\return		true  	  I2C Read ok!
	\return		false  	  I2C Read data fail!
	\par [Example]
	\code
		 I2C1_Type *pI2C;
		 uint8_t   pBuf[2];

		 pI2C = pI2C_SlaveInit (I2C_2, I2C_SLAVE, I2C_SADDR10, 0x351);
		 bI2C_SlaveRead (pI2C, pBuf, 2):
	\endcode
*/
bool bI2C_SlaveRead (I2C1_Type *pI2C, uint8_t *pBuf, uint32_t ulSz);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Slave Interrupt Write Function
	\param[in]	pI2C 	  I2C register address point
	\param[in]	ubMode	  Interrupt mode, is start interrupt enable?
	\param[in]	pBuf	  Write data
	\param[in]	ulSz	  Write data length
	\return		true  	  I2C Write ok!
	\return		false  	  I2C Write data fail!
	\par [Example]
	\code
		 I2C1_Type *pI2C;
		 uint8_t   pBuf[2]={0, 1};

		 pI2C = pI2C_SlaveInit (I2C_2, I2C_SLAVE, I2C_SADDR8, 0x51);
		 bI2C_SlaveIntWrite (pI2C, I2C_INT, pBuf, 2):
	\endcode
*/
bool bI2C_SlaveIntWrite (I2C1_Type *pI2C, uint8_t ubMode, uint8_t *pBuf, uint32_t ulSz);
//------------------------------------------------------------------------------
/*!
	\brief 		I2C Slave Interrupt Read Function
	\param[in]	pI2C 	  I2C register address point
	\param[in]	ubMode	  Interrupt mode, is start interrupt enable?
	\param[out]	pBuf	  Read data
	\param[in]	ulSz	  Read data length
	\return		true  	  I2C Read ok!
	\return		false  	  I2C Read data fail!
	\par [Example]
	\code
		 I2C1_Type *pI2C;
		 uint8_t   pBuf[2];

		 pI2C = pI2C_SlaveInit (I2C_2, I2C_SLAVE, I2C_SADDR10, 0x351);
		 bI2C_SlaveIntRead (pI2C, I2C_START_INT, pBuf, 2):
	\endcode
*/
bool bI2C_SlaveIntRead (I2C1_Type *pI2C, uint8_t ubMode, uint8_t *pBuf, uint32_t ulSz);
//------------------------------------------------------------------------------
/*!
	\brief 	Get I2C Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwI2C_GetVersion();
		 printf("I2C Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwI2C_GetVersion (void);
#endif
