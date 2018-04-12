/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BSP_Board.c
	\brief		VBM PU/BU Demo/EV Board
	\author		Hanyi Chiu
	\version	0.1
	\date		2017/05/03
	\copyright	Copyright(C) 2017 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#if defined VBM_PU || defined VBM_BU

#include "BSP.h"
#include "_510PF.h"

//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMPU_DEMO
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! LCD
	//! GPIO SPI
	GLB->PADIO47 = 1;
	GLB->PADIO48 = 1;
	GLB->PADIO49 = 1;
	GLB->PADIO50 = 1;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;
	
	//! Speaker
	GLB->PADIO54 = 0;
	
	GLB->PADIO55 = 0;

	//! LED
	GLB->PADIO16 = 0;
	GLB->PADIO17 = 0;
	GLB->PADIO55 = 0;
	GLB->PADIO56 = 0;
	GLB->PADIO57 = 0;
	
	//! BL
	GLB->PADIO51 = 7;

	// LCD POWER
	GLB->PADIO52 = 0;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMPU_EV
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! SSP
	GLB->PADIO58 = 4;
	GLB->PADIO59 = 4;
	GLB->PADIO60 = 4;
	GLB->PADIO61 = 4;
	//! LCD Pin
	GLB->PADIO26 = 5;
	GLB->PADIO27 = 5;
	GLB->PADIO28 = 5;
	GLB->PADIO29 = 5;
	GLB->PADIO30 = 5;
	GLB->PADIO31 = 5;
	GLB->PADIO32 = 5;
	GLB->PADIO33 = 5;
	GLB->PADIO34 = 5;
	GLB->PADIO35 = 5;
	GLB->PADIO36 = 5;

	//! SDIO Wi-Fi
	GLB->PADIO52 = 4;
	GLB->PADIO53 = 4;
	GLB->PADIO54 = 4;
	GLB->PADIO55 = 4;
	GLB->PADIO56 = 4;
	GLB->PADIO57 = 4;
	
	//! RF SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO15 = 0;
	GLB->PADIO21 = 0;

	//! SD
	GLB->PADIO0  = 4;
	GLB->PADIO1  = 4;
	GLB->PADIO2  = 4;
	GLB->PADIO3  = 4;
	GLB->PADIO4  = 4;
	GLB->PADIO5  = 4;
	GLB->PADIO6  = 4;
	GLB->PADIO7  = 4;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMBU_DEMO
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF_SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO28 = 0;
	GLB->PADIO27 = 0;

	//! SD
	GLB->PADIO0 = 4;
	GLB->PADIO1 = 4;
	GLB->PADIO2 = 4;
	GLB->PADIO3 = 4;
	GLB->PADIO4 = 4;
	GLB->PADIO5 = 4;
	GLB->PADIO6 = 4;
	GLB->PADIO7 = 4;

	//! I2C
	GLB->PADIO13 = 4;
	GLB->PADIO14 = 4;

		
	//! Speaker
	GLB->PADIO16 = 0;
	
	//! IR LED
	GLB->PADIO32 = 0;

	//! LED
	GLB->PADIO29 = 0;
	GLB->PADIO30 = 0;
	GLB->PADIO31 = 0;

	//! Temp I2c
	GLB->PADIO37 = 2;
	GLB->PADIO38 = 2;
}
#endif
//------------------------------------------------------------------------------
#ifdef BSP_BOARD_VBMBU_EV
void BSP_BoardInit(void)
{
	//! UART
	GLB->PADIO22 = 2;
	GLB->PADIO23 = 2;

	//! RF_SPI
	GLB->PADIO18 = 1;
	GLB->PADIO19 = 1;
	GLB->PADIO20 = 1;
	GLB->PADIO28 = 0;
	GLB->PADIO27 = 0;

	//! SD
	GLB->PADIO0 = 4;
	GLB->PADIO1 = 4;
	GLB->PADIO2 = 4;
	GLB->PADIO3 = 4;
	GLB->PADIO4 = 4;
	GLB->PADIO5 = 4;
	GLB->PADIO6 = 4;
	GLB->PADIO7 = 4;

	//! SDIO Wi-Fi
	GLB->PADIO52 = 4;
	GLB->PADIO53 = 4;
	GLB->PADIO54 = 4;
	GLB->PADIO55 = 4;
	GLB->PADIO56 = 4;
	GLB->PADIO57 = 4;
}
#endif
//------------------------------------------------------------------------------

#endif //! End #if defined VBM_PU || defined VBM_BU
