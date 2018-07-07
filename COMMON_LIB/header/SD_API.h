/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SD_API.h
	\brief		SD API access header file
	\author		Justin Chen
	\version	0.3
	\date		2017/03/16
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __SD_API_h
#define __SD_API_h

#include "_510PF.h"

#define SD_FunctionDisable								\
						{								\
							GLB->SDIO1_FUNC_DIS = 1;	\
							GLB->SDIO2_FUNC_DIS = 1;	\
						}

typedef enum
{
	SD_1 = 0,
	SD_2,
}SD_DEVICE_t;

typedef enum
{
	SD_ACC_TRGANDFORGOT = 0,	//Trigger and forgot 	(Interrupt Mode)
	SD_ACC_WAIT					//Trigger and wait		(Normal Mode)
}SD_ACCESS_MODE;

//------------------------------------------------------------------------
/*!
\brief Initial SD
\return(no)
*/
void SD_Init(void);

//------------------------------------------------------------------------
/*!
\brief SD Interface setup
\return(no)
*/
uint8_t SD_SetupIF(SD_DEVICE_t tSD_Dev);

//------------------------------------------------------------------------
/*!
\brief Get SD interface number
\return SD_1 or SD_2
*/
SD_DEVICE_t tSD_GetDevIF(void);

//------------------------------------------------------------------------
/*!
\brief Check SD interface setup result
\return 0->Not ready
        1->Ready
*/
uint8_t ubSD_ChkIFSetup(void);

//------------------------------------------------------------------------
/*!
\brief Identify SD
\return 0->Fail\n
		1->Pass	
*/
uint8_t ubSD_Identify(SD_DEVICE_t tSD_Dev);

//------------------------------------------------------------------------------
/*!
\brief 	Get SD Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwSD_GetVersion();
	 printf("SD Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwSD_GetVersion(void);

//------------------------------------------------------------------------
/*!
\brief Check SD is card-in or card-out status
\return 0->Card-Out\n
		1->Card-In
*/
uint8_t ubSD_ChkCardIn(SD_DEVICE_t tSD_Dev);

//------------------------------------------------------------------------
/*!
\brief Check SD is ready or busy to access
\return 0->Busy\n
		1->Ready
*/
uint8_t ubSD_ChkRdy(SD_DEVICE_t tSD_Dev);

//------------------------------------------------------------------------
/*!
\brief Write data to SD
\param ulSrcAddr	Source address
\param ulSdLBA 	Destination SD LBA
\param ulSize 	Access size
\param ubMode 	0->Without wait access done,1->Wait access done
\return 0->Fail\n
		1->Pass
*/
uint8_t ubSD_Write(SD_DEVICE_t tSD_Dev,uint32_t ulSrcAddr, uint32_t ulSdLBA, uint32_t ulSize, uint8_t ubMode);

//------------------------------------------------------------------------
/*!
\brief Read data from SD
\param ulDestAddr	Destination address
\param ulSdLBA 	Source SD LBA
\param ulSize 	Access size
\param ubMode 	0->Without wait access done,1->Wait access done
\return 0->Fail\n
		1->Pass
*/
uint8_t ubSD_Read(SD_DEVICE_t tSD_Dev,uint32_t ulDestAddr, uint32_t ulSdLBA, uint32_t ulSize, uint8_t ubMode);

//------------------------------------------------------------------------
/*!
\brief Get SD total sectors
\return Total sectors		
*/
uint32_t ulSD_GetTotalSector(SD_DEVICE_t tSD_Dev);

#endif
