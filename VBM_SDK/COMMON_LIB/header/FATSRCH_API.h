/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		FATSRCH_API.h
	\brief		FAT-Search API header file
	\author		Justin Chen
	\version	0.2
	\date		2017/03/13
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __FATSRCH_API_h
#define __FATSRCH_API_h

#include "_510PF.h"

typedef enum
{
	FATSRCH_16_BIT = 0,
	FATSRCH_32_BIT
}FATSRCH_MODE;

typedef enum
{
	FATSRCH_FAIL = 0,
	FATSRCH_OK
}FATSRCH_RESULT;

//------------------------------------------------------------------------------
/*!
\brief 	Get FATSRCH Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwFATSRCH_GetVersion();
	 printf("FATSRCH Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwFATSRCH_GetVersion (void);

//------------------------------------------------------------------------
/*!
\brief Initial FAT-Search
\return(no)
*/
void FATSRCH_Init(void);

//------------------------------------------------------------------------
/*!
\brief FAT-Search
\param ubMode FATSRCH_16_BIT->16 Bits Format\n
			FATSRCH_32_BIT->32 Bits Format	
\param ulTarData 	Target pattern, 0x00000000 ~ 0xFFFFFFFF
\param ulSrchStartAddr 	Search start address
\param ulSrchLen 	Search length (Unit: Byte)
\param ulRptAddr 	Report Address
\return FATSRCH_FAIL->Fat Search Fail\n
		FATSRCH_OK->Fat Search Ok
*/
FATSRCH_RESULT tFATSRCH_Srch(uint8_t ubMode,uint32_t ulTarData,uint32_t ulSrchStartAddr,uint32_t ulSrchLen,uint32_t ulRptAddr);

//------------------------------------------------------------------------
/*!
\brief Check FatSrch IP is ready or busy to access
\return 0->Busy\n
		1->Ready
*/
uint8_t ubFATSRCH_ChkRdy(void);

//------------------------------------------------------------------------
/*!
\brief Get valid length of search report
\return Valid length of search report
*/
uint32_t ulFATSRCH_GetRptLen(void);

#endif
