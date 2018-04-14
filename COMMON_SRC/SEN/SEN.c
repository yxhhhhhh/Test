/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SEN.c
	\brief		Sensor relation function
	\author		BoCun
	\version	1.0
	\date		2018-01-19
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "SEN.h"
#include "INTC.h"
#include "TIMER.h"
#include "IQ_API.h"
#include "IQ_PARSER_API.h"
#include "ISP_API.h"
#include "AE_API.h"
#include "AF_API.h"
#include "AWB_API.h"
#include "DIS_API.h"
#include "MD_API.h"
#include "BUF.h"
#include "USBD_API.h"

//------------------------------------------------------------------------------
#define SEN_MAJORVER    1        //!< Major version = 1
#define SEN_MINORVER    0        //!< Minor version = 0
//------------------------------------------------------------------------------
osSemaphoreId 	SEM_SEN_VSyncRdy;
osMessageQId tSEN_ExtEventQueue;
static void SEN_DoVsyncThread(void const *argument);
static bool bSEN_InitFlg = FALSE;
uint8_t ubSEN_VIDEO[3]={0,0,0};

uint8_t ubSEN_ActiveFlg[3] = {0,0,0};//PATH1,PATH2,PATH3
uint8_t ubSEN_FirstOutFlg = 1;
uint8_t ubSEN_FreeRunFlg = 1;
uint8_t ubSEN_StateChangeFlg = 0;
uint8_t ubSEN_AlgReportFg = 0;
uint8_t ubSEN_FrameRate = 30;
uint8_t ubSEN_EventNode;
uint8_t ubSEN_UvcPathFlag = 0;
uint8_t ubSEN_ResChgFlg[3] = {0,0,0};
uint8_t ubSEN_IrMode = 0;

//------------------------------------------------------------------------------
void SEN_SetOutResolution(uint8_t ubPath, uint16_t uwH, uint16_t uwV)
{
	if(ubPath == SENSOR_PATH1)
	{
		// path1 image horizontal/vertical size
		sensor_cfg.ulHSize1 = uwH;
		sensor_cfg.ulVSize1 = uwV;
		// Path1 Image frame size
		sensor_cfg.ulFrameSize1 = uwH * uwV * 3 / 2;
	}
	else if(ubPath == SENSOR_PATH2)
	{
        // path2 image horizontal/vertical size
		sensor_cfg.ulHSize2 = uwH;
		sensor_cfg.ulVSize2 = uwV;
		// Path2 Image frame size
		sensor_cfg.ulFrameSize2 = uwH * uwV * 3 / 2;
	}
	else if(ubPath == SENSOR_PATH3)
	{
        // path3 image horizontal/vertical size
		sensor_cfg.ulHSize3 = uwH;
		sensor_cfg.ulVSize3 = uwV;
		// Path3 Image frame size 
		sensor_cfg.ulFrameSize3 = uwH * uwV * 3 / 2;
	}
	// Set scaler
	printd(DBG_InfoLvl,"P=%d,h=%d,v=%d,H=%d,V=%d\r\n",ubPath, uwH, uwV, sensor_cfg.ulHSize, sensor_cfg.ulVSize);
	ISP_SetScaler(ubPath, uwH, uwV, sensor_cfg.ulHSize, sensor_cfg.ulVSize);
}

//------------------------------------------------------------------------------
void SEN_RegisterEventQueue(osMessageQId tQueueId)
{
	tSEN_ExtEventQueue = tQueueId;
}

//------------------------------------------------------------------------------
void SEN_RegisterEventNode(uint8_t ubEventNode)
{
	ubSEN_EventNode = ubEventNode;
}

//------------------------------------------------------------------------------
void SEN_SetPathAddr(uint8_t ubPath,uint32_t ulBufAddr)
{
	if(ubPath == SENSOR_PATH1)
	{
		SEN->STR1_STA = ulBufAddr >> 8;       
        sensor_cfg.ulPath1_Addr = ulBufAddr;
	}
	else if(ubPath == SENSOR_PATH2)
	{
		SEN->STR2_STA = ulBufAddr >> 8;        
        sensor_cfg.ulPath2_Addr = ulBufAddr;
	}
	else if(ubPath == SENSOR_PATH3)
	{
		SEN->STR3_STA = ulBufAddr >> 8;        
        sensor_cfg.ulPath3_Addr = ulBufAddr;
	}
	else if(ubPath == ISP_3DNR)
	{
		SEN->NR3D_STA = ulBufAddr >> 8;
        SEN->NR3D_EDA = (ulBufAddr + ISP_WIDTH*ISP_HEIGHT*10/8) >> 8;
        
        sensor_cfg.ulPath4_Addr = ulBufAddr;
        sensor_cfg.ulPath5_Addr = (ulBufAddr + ISP_WIDTH*ISP_HEIGHT*10/8);
	}    
	else if(ubPath == ISP_MD_W0)
	{
		SEN->MD0_STA = ulBufAddr >> 8;        
        sensor_cfg.ulPath6_Addr = ulBufAddr;
	} 
	else if(ubPath == ISP_MD_W1)
	{
		SEN->MD1_STA = ulBufAddr >> 8;        
        sensor_cfg.ulPath7_Addr = ulBufAddr;
	} 
	else if(ubPath == ISP_MD_W2)
	{
		SEN->MD2_STA = ulBufAddr >> 8;       
        sensor_cfg.ulPath8_Addr = ulBufAddr;
	}  
	else if(ubPath == IQ_BIN_FILE)
	{      
        //bin file address
        sensor_cfg.ulPath9_Addr = ulBufAddr;
	}     
}

//------------------------------------------------------------------------------
void SEN_UpdatePathAddr(void)
{
	SEN->COLOR_TRIG_5 = 1;
}

//------------------------------------------------------------------------------
void SEN_SetStateChangeFlg(uint8_t ubFlg)
{
	ubSEN_StateChangeFlg = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetStateChangeFlg(void)
{
	return ubSEN_StateChangeFlg;
}

//------------------------------------------------------------------------------
void SEN_SetActiveFlg(uint8_t ubPath,uint8_t ubFlg)
{
	if(ubPath == SENSOR_PATH1)
	{
		ubSEN_ActiveFlg[0] = ubFlg;
	}
	else if(ubPath == SENSOR_PATH2)
	{
		ubSEN_ActiveFlg[1] = ubFlg;
	}
	else if(ubPath == SENSOR_PATH3)
	{
		ubSEN_ActiveFlg[2] = ubFlg;
	}
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetActiveFlg(uint8_t ubPath)
{
	if(ubPath == SENSOR_PATH1)
	{
		return ubSEN_ActiveFlg[0];
	}
	else if(ubPath == SENSOR_PATH2)
	{
		return ubSEN_ActiveFlg[1];
	}
	else if(ubPath == SENSOR_PATH3)
	{
		return ubSEN_ActiveFlg[2];
	}
	printf("Err @ubSEN_GetActiveFlg\r\n");
	return 0;
}

//------------------------------------------------------------------------------
void SEN_SetFirstOutFlg(uint8_t ubFlg)
{
	ubSEN_FirstOutFlg = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetFirstOutFlg(void)
{
	return ubSEN_FirstOutFlg;
}

//------------------------------------------------------------------------------
void SEN_SetResChgFlg(uint8_t ubPath, uint8_t ubFlg)
{
	ubSEN_ResChgFlg[ubPath] = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetResChgFlg(uint8_t ubPath)
{
	return ubSEN_ResChgFlg[ubPath];
}

//------------------------------------------------------------------------------
void SEN_SetResChgState(uint8_t ubPath, uint16_t uwH, uint16_t uwV)
{
	SEN_SetResChgFlg(ubPath, TRUE);
	SEN_SetOutResolution(ubPath, uwH, uwV);
}

//------------------------------------------------------------------------------
void SEN_SetClkEn(uint8_t ubEnable)
{
	SEN->SEN_CLK_EN = ubEnable;
}

//------------------------------------------------------------------------------
void SEN_SetPathSrc(uint8_t ubPath1Src,uint8_t ubPath2Src,uint8_t ubPath3Src)
{
	sensor_cfg.ubPath1Src = ubPath1Src;
	sensor_cfg.ubPath2Src = ubPath2Src;
	sensor_cfg.ubPath3Src = ubPath3Src;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetPathSrc(uint8_t ubPath)
{
	if(ubPath == SENSOR_PATH1)
	{
		return sensor_cfg.ubPath1Src;
	}
	else if(ubPath == SENSOR_PATH2)
	{
		return sensor_cfg.ubPath2Src;
	}
	else if(ubPath == SENSOR_PATH3)
	{
		return sensor_cfg.ubPath3Src;
	}
	else
	{
		printf("Err @ubSEN_GetPathSrc\r\n");
		return 0xFF;
	}
}

//------------------------------------------------------------------------------
void SEN_Hsync_ISR(void)
{
	// Clear HSYNC flag.
	if( SEN->SEN_HSYNC_INT_FLAG )
	{
		SEN->SEN_HSYNC_INT_CLR = 1;	
	}   
	INTC_IrqClear(INTC_SEN_HSYNC_IRQ);
}

//------------------------------------------------------------------------------
void SEN_HwEnd_ISR(void)
{
	uint32_t ulSEN_NextYuv1Addr, ulSEN_NextYuv2Addr, ulSEN_NextYuv3Addr;
	SEN_EVENT_PROCESS tProcess;
	uint8_t ubSrc;

	// Clear HW_END flag.
	if( SEN->HW_END_INT_FLAG )
	{
		SEN->HW_END_INT_CLR = 1;
        //printf("Hw");
	}

    SEN_ChkISPState();

	SEN->HW_END_INT_CLR = 1;
	INTC_IrqClear(INTC_ISP_WIN_END_IRQ);
	SEN->IMG_TX_EN = 0;

	// frame end to get 3A report.
	SEN_FrmEnd_ISR();

    if(ubSEN_GetSensorFreeRunFlg() != 1)    
    {
        if(ubSEN_GetFirstOutFlg())
        {
			if(ubSEN_UvcPathFlag)
				SEN_SetFirstOutFlg(0);
            if(ubSEN_GetActiveFlg(SENSOR_PATH1))
            {
                ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
            }
            if(ubSEN_GetActiveFlg(SENSOR_PATH2))
            {
                ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
            }
            if(ubSEN_GetActiveFlg(SENSOR_PATH3))
            {
                ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
            }
        }
        else
        {
            //Path1-Q
            if(ubSEN_GetActiveFlg(SENSOR_PATH1))
            {
                if(!ubSEN_GetResChgFlg(SENSOR_PATH1))
                {
                    ulSEN_NextYuv1Addr = ulBUF_GetSen1YuvFreeBuf();
                    if(ulSEN_NextYuv1Addr != BUF_FAIL)
                    {
                        ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
                        tProcess.ubSrcNum		= ubSrc;
                        tProcess.ubCurNode	 	= 0;
                        tProcess.ubNextNode 	= ubSEN_EventNode;
                        tProcess.ulSize			= sensor_cfg.ulFrameSize1;
                        tProcess.ulDramAddr1	= SEN->STR1_STA << 8;	//YUV Address
                        sensor_cfg.ulPath1_Addr = tProcess.ulDramAddr1;
                        if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
                        {
                            ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                            printf("SEN_Q->Full !!!!\r\n");
                        }
                        else
                            SEN->STR1_STA = ulSEN_NextYuv1Addr >> 8;
                    }
                    else
                        ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                }
                else
                {
                    ubBUF_ReleaseSenYuvBuf(SEN->STR1_STA << 8);
                    SEN_SetResChgFlg(SENSOR_PATH1, FALSE);
                }
            }

            //Path2-Q
            if(ubSEN_GetActiveFlg(SENSOR_PATH2))
            {
                if(!ubSEN_GetResChgFlg(SENSOR_PATH2))
                {
                    ulSEN_NextYuv2Addr = ulBUF_GetSen2YuvFreeBuf();
                    if(ulSEN_NextYuv2Addr != BUF_FAIL)
                    {
                        ubSrc = ubSEN_GetPathSrc(SENSOR_PATH2);
                        tProcess.ubSrcNum		= ubSrc;
                        tProcess.ubCurNode	 	= 0;
                        tProcess.ubNextNode 	= ubSEN_EventNode;
                        tProcess.ulSize			= sensor_cfg.ulFrameSize2;
                        tProcess.ulDramAddr1	= SEN->STR2_STA << 8;	//YUV Address
                        sensor_cfg.ulPath2_Addr = tProcess.ulDramAddr1;
                        if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
                        {
                            ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
                            printf("SEN_Q->Full !!!!\r\n");
                        }
                        else
                            SEN->STR2_STA = ulSEN_NextYuv2Addr >> 8;
                    }
                    else
                        ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
                }
                else
                {
                    ubBUF_ReleaseSenYuvBuf(SEN->STR2_STA << 8);
                    SEN_SetResChgFlg(SENSOR_PATH2, FALSE);
                }
            }

            //Path3-Q
            if(ubSEN_GetActiveFlg(SENSOR_PATH3))
            {
                if(!ubSEN_GetResChgFlg(SENSOR_PATH3))
                {
                    ulSEN_NextYuv3Addr = ulBUF_GetSen3YuvFreeBuf();
                    if(ulSEN_NextYuv3Addr != BUF_FAIL)
                    {
                        ubSrc = ubSEN_GetPathSrc(SENSOR_PATH3);
                        tProcess.ubSrcNum		= ubSrc;
                        tProcess.ubCurNode	 	= 0;
                        tProcess.ubNextNode 	= ubSEN_EventNode;
                        tProcess.ulSize			= sensor_cfg.ulFrameSize3;
                        tProcess.ulDramAddr1	= SEN->STR3_STA << 8;	//YUV Address
                        sensor_cfg.ulPath3_Addr = tProcess.ulDramAddr1;
                        if(osMessagePut(tSEN_ExtEventQueue, &tProcess, 0) != osOK)
                        {
                            ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
                            printf("SEN_Q->Full !!!!\r\n");
                        }
                        else
                            SEN->STR3_STA = ulSEN_NextYuv3Addr >> 8;
                    }
                    else
                        ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
                }
                else
                {
                    ubBUF_ReleaseSenYuvBuf(SEN->STR3_STA << 8);
                    SEN_SetResChgFlg(SENSOR_PATH3, FALSE);
                }
            }
        }

        SEN->COLOR_TRIG_5 = 1;

        if(ubSEN_GetStateChangeFlg())
        {		
            SEN_SetStateChangeFlg(0);

            if(ubSEN_GetActiveFlg(SENSOR_PATH1))
            {
                SEN->VIDEO_STR_EN_1 = 1;
            }
            else
            {
                SEN->VIDEO_STR_EN_1 = 0;
            }

            if(ubSEN_GetActiveFlg(SENSOR_PATH2))
            {
                SEN->VIDEO_STR_EN_2 = 1;
            }
            else
            {
                SEN->VIDEO_STR_EN_2 = 0;
            }

            if(ubSEN_GetActiveFlg(SENSOR_PATH3))
            {
                SEN->VIDEO_STR_EN_3 = 1;
            }
            else
            {
                SEN->VIDEO_STR_EN_3 = 0;
            }
        }        
    }    
	SEN->IMG_TX_EN = 1;	
	//================================================================================

	if (ubSEN_UvcPathFlag) {
        SEN_SetISPOutputFormat();
	}
}

//------------------------------------------------------------------------------
void SEN_Vsync_ISR(void)
{
    // Clear VSYNC flag.
	if( SEN->SEN_VSYNC_INT_FLAG )
	{
		SEN->SEN_VSYNC_INT_CLR = 1;
	}
	INTC_IrqClear(INTC_SEN_VSYNC_IRQ);
        
	osSemaphoreRelease(SEM_SEN_VSyncRdy);		
}

//------------------------------------------------------------------------------
void SEN_InitProcess(void)
{
	uint8_t ubSrc;
	
	if(bSEN_InitFlg == FALSE)
	{
		GLB->PADIO45 = 0;
		GPIO->GPIO_OE3 = 1;
		GPIO->GPIO_O3 = 1 ;
		
		//I2C
		GLB->PADIO13 = 4;
		GLB->PADIO14 = 4;
	
		//REST
		GLB->PADIO15 = 0;
		GPIO->GPIO_OE1 = 1;
		GPIO->GPIO_O1 = 1 ;
		TIMER_Delay_ms(100);		
		GPIO->GPIO_O1 = 0 ;
		TIMER_Delay_ms(100);
		GPIO->GPIO_O1 = 1 ;
		
		// Initial sensor value.
		SEN_SetSensorInitVal();
        // Set AXI rate
        if(ubSEN_GetSensorType() == SEN_AR0330)
        {
            GLB->ISP_AXI_RATE = 0x03;        
        }
		// Setup sensor relation IRQ
		SEN_ISRInitial();					
		// Set sensor output size and ISP process size
		SEN_SetWindowSize();
	}
	
	//Path1
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH1);
	if(ubSrc != SEN_SRC_NONE)
	{
		SEN_SetResolution(SENSOR_PATH1, sensor_cfg.ulHSize1, sensor_cfg.ulVSize1);
	}	
	//Path2
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH2);
	if(ubSrc != SEN_SRC_NONE)
	{
		SEN_SetResolution(SENSOR_PATH2, sensor_cfg.ulHSize2, sensor_cfg.ulVSize2);
	}	
	//Path3
	ubSrc = ubSEN_GetPathSrc(SENSOR_PATH3);
	if(ubSrc != SEN_SRC_NONE)
	{
		SEN_SetResolution(SENSOR_PATH3, sensor_cfg.ulHSize3, sensor_cfg.ulVSize3);
	}		
	
	if(bSEN_InitFlg == FALSE)
	{
		// set&start sensor
		if(ubSEN_Open(&sensor_cfg,ubSEN_FrameRate) != 1)
		{
			printf("Sensor startup failed! \r\n");
		}
	}	

	// set video stream
	//Path1
	if(ubSEN_GetActiveFlg(SENSOR_PATH1))
	{
		SEN_SetPathState(SENSOR_PATH1, 1);
	}
	else
	{
		SEN_SetPathState(SENSOR_PATH1, 0);
	}

	//Path2
	if(ubSEN_GetActiveFlg(SENSOR_PATH2))
	{
		SEN_SetPathState(SENSOR_PATH2, 1);
	}
	else
	{
		SEN_SetPathState(SENSOR_PATH2, 0);
	}

	//Path3
	if(ubSEN_GetActiveFlg(SENSOR_PATH3))
	{
		SEN_SetPathState(SENSOR_PATH3, 1);
	}
	else
	{
		SEN_SetPathState(SENSOR_PATH3, 0);
	}

	SEN->VIDEO_STR_EN_1 = 0;
	SEN->VIDEO_STR_EN_2 = 0;
	SEN->VIDEO_STR_EN_3 = 0;

    SEN->IMG_TX_EN = 1;
	SEN->SEN_CLK_EN = 1;    
        
	if(bSEN_InitFlg == FALSE)
		SEN_ISPInitial();

	if(bSEN_InitFlg == FALSE)
	{
		bSEN_InitFlg = TRUE;
	}
}

//------------------------------------------------------------------------------
void SEN_ISRInitial(void)
{
	SEM_SEN_VSyncRdy = NULL;

	SEN_VsyncInit();
	// Sensor HSYNC/VSYNC/HWEND IRQ	
	INTC_IrqSetup(INTC_SEN_HSYNC_IRQ, INTC_LEVEL_TRIG, SEN_Hsync_ISR);
	INTC_IrqEnable(INTC_SEN_HSYNC_IRQ);		
	INTC_IrqSetup(INTC_ISP_WIN_END_IRQ, INTC_LEVEL_TRIG, SEN_HwEnd_ISR);
	INTC_IrqEnable(INTC_ISP_WIN_END_IRQ);		
	INTC_IrqSetup(INTC_SEN_VSYNC_IRQ, INTC_LEVEL_TRIG, SEN_Vsync_ISR);
	INTC_IrqEnable(INTC_SEN_VSYNC_IRQ);	
	
	// disable interrupt first
	SEN->HW_END_INT_EN = 0;
	SEN->SEN_VSYNC_INT_EN = 0;
	SEN->SEN_HSYNC_INT_EN = 0;	
}

//------------------------------------------------------------------------------
void SEN_ISPInitial(void)
{
	// AE and AWB report flag init
	ISP_Init();
	//Init AE,AWB,AF
    #if(AE_EN == 1)	
        AE_Init();
    #endif
    #if(AWB_EN == 1)
        AWB_Init();
    #endif
    #if(AF_EN == 1)
        AF_Init();
    #endif

    #if(IQ_DN_EN == 1 && AE_EN == 1 && AWB_EN == 1 && AF_EN == 1)
        IQ_DynamicInit();
    #endif

	IQ_Init();
	IQ_ReadIQTable();
	IQ_SetDynFrameRate(sensor_cfg.ulSensorFrameRate);

    // open 3DNR frame buffer compression
    ISP_Set3DNR_FBC();
    //NR Day mode
    SEN_SetIrMode(0);
}

//------------------------------------------------------------------------------
void SEN_SetResolution(uint8_t ubPath, uint32_t ulWidth, uint32_t ulHeight)
{
	// Set sensor struct value.
	SEN_SetOutResolution(ubPath, ulWidth, ulHeight);
    // Set scaler FIR
    IQ_SetResolution_SDK(ulWidth, ulHeight, ubIQ_GetDynFrameRate(), ubPath);
    // Set current preview ulWidth size
	IQ_SetISPRes();
}

//------------------------------------------------------------------------------
void SEN_SetWindowSize(void)
{
	SEN_SetSensorImageSize();
	
	// Image for ISP
	SEN->H_START = sensor_cfg.ulHOSize - sensor_cfg.ulHSize;
    if(ubSEN_GetSensorType() == SEN_AR0330)
    {
        // avoid 3DNR bug
        SEN->V_START = 2;
    }else if(ubSEN_GetSensorType() == SEN_IMX323){
        SEN->V_START = 0x10;          
    }else{
        SEN->V_START = 0;  
    }
	SEN->H_SIZE = sensor_cfg.ulHSize >> 1;
	SEN->V_SIZE = sensor_cfg.ulVSize >> 1;
	// Output size from sensor.
	SEN->HO_SIZE = sensor_cfg.ulHOSize >> 5;										//Unit:32pixel
	SEN->VO_SIZE = sensor_cfg.ulVOSize >> 5;										//Unit:32line	
}

//------------------------------------------------------------------------------
void SEN_SetISPRate(uint32_t ulISP_div)
{
	uint32_t ulSysClk;
	
	if(GLB->SYS_RATE == 0)
	{
		ulSysClk = 800000000; 		// system clock / 1
	}else if(GLB->SYS_RATE == 1){
		ulSysClk = 400000000;  		// system clock / 2		
	}else if(GLB->SYS_RATE == 2){
		ulSysClk = 200000000;  		// system clock / 4			
	}else{
		ulSysClk = 100000000;  		// system clock / 8			
	}
	printd(DBG_Debug3Lvl, "System clock = %d MHz, ",(ulSysClk/1000000));
	///////////////////////////
	//F(ISP_CLK)= DDR_PLL /(2^SYS_RATE) /ISP_RATE.
	//set isp rate to 15, ISP Clock = 8 MHz
	GLB->ISP_RATE = ulISP_div;
	printd(DBG_Debug3Lvl, "ISP clock = %d MHz \n",(ulSysClk/1000000)/ulISP_div);	
}

//------------------------------------------------------------------------------
void SEN_SetSensorRate(uint8_t ubSelBaseClk, uint8_t ubSensorDiv)
{
	uint32_t ulBaskClk;
	
	SEN->SEN_BASE_CLK_SEL = ubSelBaseClk;
	if(SEN->SEN_BASE_CLK_SEL == 0)
	{
		ulBaskClk = 96000000;		//96MHz
	}else if(SEN->SEN_BASE_CLK_SEL == 1){
		ulBaskClk = 120000000;	//120MHz
	}
	printd(DBG_Debug3Lvl, "Bask clock generator = %d MHz, ",(ulBaskClk/1000000));	
	
	SEN->SEN_RATE = ubSensorDiv;
	printd(DBG_Debug3Lvl, "Sensor clock = %d MHz \n",(ulBaskClk/1000000)/ubSensorDiv);	
}

//------------------------------------------------------------------------------
void SEN_VsyncInit(void)
{
	// Semaphore Create
	osSemaphoreDef(SEM_SEN_VSyncRdy);
	SEM_SEN_VSyncRdy    = osSemaphoreCreate(osSemaphore(SEM_SEN_VSyncRdy), 1);
	// Task Create
	osThreadDef(SEN_DoVsync, SEN_DoVsyncThread, osPriorityNormal, 1, 1024);
	osThreadCreate(osThread(SEN_DoVsync), NULL);
}

//------------------------------------------------------------------------------
uint8_t ubDropCnt = 0;
uint8_t ub3ACtrlTableCnt = 0;
uint8_t ub3ACtrlTableFg = FALSE;
static void SEN_DoVsyncThread(void const *argument)
{
	// update ISP after Vsync	
	while(1) {
		osSemaphoreWait(SEM_SEN_VSyncRdy ,osWaitForever);

        // drop "ISP_ALG_REPORT_CNT" frame then process AE, AWB and dynamic IQ alg.
        if(ubDropCnt >= ISP_ALG_REPORT_CNT)
        {
            SEN_SetAlgReportFg(1);
        }else{
            ubDropCnt++;
        }
        
        if(ubSEN_GetAlgReportFg() == 1)
        {
            #if (AWB_EN == 1)
                AWB_VSync();
            #endif	

            #if(AE_EN == 1)
                AE_VSync();
            #endif
            
            #if(IQ_DN_EN == 1 && AE_EN == 1 && AWB_EN == 1 && AF_EN == 1)
                IQ_DynamicVSync();
            #endif            
        }

		#if(AF_EN == 1)
            AF_VSync();
		#endif

        // using initial AE value when AE unstable.(accelerate AE stable)
        if(ub3ACtrlTableFg == FALSE)
        {
            if(ub3ACtrlTableCnt > 10)
            {
                AE_SetCtrlTable();
                AWB_SetCtrlTable();                
                ub3ACtrlTableFg = TRUE;
            }
            ub3ACtrlTableCnt++;                 
        }   
	}
}

//------------------------------------------------------------------------------
void SEN_SetPathState(uint8_t ubPath, uint8_t ubFlag)
{
	switch(ubPath)
	{
		case SENSOR_PATH1:
			ubSEN_VIDEO[0] = ubFlag;		
			break;
		case SENSOR_PATH2:
			ubSEN_VIDEO[1] = ubFlag;
			break;
		case SENSOR_PATH3:
			ubSEN_VIDEO[2] = ubFlag;
			break;
		default:
			break;			
	}
}

//------------------------------------------------------------------------------
void SEN_EnableVideo(void)
{
	// Enable sensor write image data to dram
	SEN->IMG_TX_EN = 1;
	SEN->SEN_CLK_EN = 1;
	SEN->VIDEO_STR_EN_1 = ubSEN_VIDEO[0];
	SEN->VIDEO_STR_EN_2 = ubSEN_VIDEO[1];
	SEN->VIDEO_STR_EN_3 = ubSEN_VIDEO[2];	
}

//------------------------------------------------------------------------------
void SEN_DisableVideo(void)
{
	// Disable sensor write image data to dram
	SEN->IMG_TX_EN = 0;
	SEN->SEN_CLK_EN = 0;
	SEN->VIDEO_STR_EN_1 = 0;
	SEN->VIDEO_STR_EN_2 = 0;
	SEN->VIDEO_STR_EN_3 = 0;
}

//------------------------------------------------------------------------------
void SEN_SetSensorInitVal(void)
{
//	// Clear struct of relational sensor value.
//	memset (&sensor_cfg, 0, sizeof (struct SENSOR_SETTING));	
//	memset (&xtSENInst, 0, sizeof (struct tagSENObj));	
	
	// some parameter are useless.
	xtSENInst.ubState			= 0;
	xtSENInst.ubPrevState		= 0;
	xtSENInst.ubLastInitIdx	    = 0xFF;
	xtSENInst.ubLastFRIdx		= 0xFF;
	xtSENInst.ubImgMode			= 0;
	xtSENInst.uwMaxExpLine 	    = SEN_MAX_EXPLINE;

	xtSENInst.xtSENCtl.ulExpTime	= 0;
	xtSENInst.xtSENCtl.uwExpLine	= 0;
	xtSENInst.xtSENCtl.uwGain		= 0;
	xtSENInst.xtSENCtl.uwTotalGain	= 0;
	xtSENInst.xtSENCtl.uwRedGain	= 0;
	xtSENInst.xtSENCtl.uwGreenGain	= 0;
	xtSENInst.xtSENCtl.uwBlueGain	= 0;
    
    SEN_SetSensorType();
}

//------------------------------------------------------------------------------
void SEN_ChkISPState(void)
{
	uint16_t uwCheckValue;    
	// for debug	
	uwCheckValue = SEN->REG_0x1300;
	if(uwCheckValue & 0x1ff)
	{
		if(uwCheckValue & 0x1)
			printf("!video1 buffer full!\n");
		if(uwCheckValue & 0x2)
			printf("!video2 buffer full!\n");
		if(uwCheckValue & 0x4)
			printf("!video3 buffer full!\n");
		if(uwCheckValue & 0x8)
			printf("!3DNR write full!\n");	
		if(uwCheckValue & 0x10)
			printf("!3DNR read empty!\n");	  
		if(uwCheckValue & 0x20)
			printf("!MD W0 write full!\n");
		if(uwCheckValue & 0x40)
			printf("!MD W0 read empty!\n");
		if(uwCheckValue & 0x80)
			printf("!MD W1 write full!\n");
		if(uwCheckValue & 0x100)
			printf("!MD W2 write full!\n");
      
		SEN->REG_0x1300 = 0x1ff;
	}
	uwCheckValue = SEN->REG_0x1304;
	if(uwCheckValue & 0x07)
	{
		if(uwCheckValue & 0x01)
			printf(" path1 FIFO full!!\n");
		if(uwCheckValue & 0x02)
			printf(" path2 FIFO full!!\n");
		if(uwCheckValue & 0x04)
			printf(" path3 FIFO full!!\n");		        
    }
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetSensorType(void)
{
    return sensor_cfg.ulSensorType;
}

//------------------------------------------------------------------------------
uint32_t ulSEN_GetSenHSize(uint8_t ubType)
{
	if(ubType == SENSOR_PATH1)
	{
		return sensor_cfg.ulHSize1;
	}else if(ubType == SENSOR_PATH2){
		return sensor_cfg.ulHSize2;
	}else if(ubType == SENSOR_PATH3){
		return sensor_cfg.ulHSize3;	
	}
	return 0;
}

//------------------------------------------------------------------------------
uint32_t ulSEN_GetSenVSize(uint8_t ubType)
{
	if(ubType == SENSOR_PATH1)
	{
		return sensor_cfg.ulVSize1;
	}else if(ubType == SENSOR_PATH2){
		return sensor_cfg.ulVSize2;
	}else if(ubType == SENSOR_PATH3){
		return sensor_cfg.ulVSize3;	
	}
	return 0;
}

//------------------------------------------------------------------------------
void SEN_SetFrameRate(uint8_t ubFPS)
{
	ubSEN_FrameRate = ubFPS;	
}

//------------------------------------------------------------------------------
void MIPI_PowerDownTimingSelect(uint8_t ubPclkIdx)
{	
	printf("-------------------------------\n");
	switch (ubPclkIdx)
	{
		case SEN_FPS15:
			//MIPI->DLAN_PDD_LSCALE_SEL0 = 16;
			MIPI->DLAN_PDD_LSCALE_SEL0 = 18;	// by oscilloscope
			break;       
		case SEN_FPS30:
			//MIPI->DLAN_PDD_LSCALE_SEL0 = 6;
			MIPI->DLAN_PDD_LSCALE_SEL0 = 10;	// by oscilloscope
			break;
		default:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 4;
			break;
	}
	printf("[FPS=%d]PDD select:%d\n",ubPclkIdx, MIPI->DLAN_PDD_LSCALE_SEL0);
	printf("-------------------------------\n");
}

//------------------------------------------------------------------------------
void MIPI_AutoPhaseDetect(void)
{
	uint32_t StartPos;
	uint32_t EndPos;
	uint8_t Bit;
	uint8_t ContiNum;
    uint32_t PhaseReport0;
    uint32_t PhaseReport1;
    uint8_t i;
    uint8_t ubClkSel;
    uint32_t count;
	
	count = 0;
	while(1)
	{
		// initial
		MIPI->DATA_LANE0_SEL0 = 0;
		MIPI->DATA_LANE0_SEL1 = 0;
		MIPI->DATA_LANE0_SEL2 = 0;
		MIPI->DATA_LANE0_SEL3 = 0;
		MIPI->DATA_LANE0_SEL4 = 0;
		
		MIPI->DATA_LANE1_SEL0 = 0;
		MIPI->DATA_LANE1_SEL1 = 0;
		MIPI->DATA_LANE1_SEL2 = 0;
		MIPI->DATA_LANE1_SEL3 = 0;
		MIPI->DATA_LANE1_SEL4 = 0;	
		
		MIPI->CLK_SEL0 = 0;
		MIPI->CLK_SEL1 = 0;
		MIPI->CLK_SEL2 = 0;
		MIPI->CLK_SEL3 = 0;
		MIPI->CLK_SEL4 = 0;		
		
		// auto phase scan
		MIPI->CLR_PHASE_DET_RDY = 1;
		
		MIPI->PHASE_FAIL_CONDITION = 0;
		
		MIPI->SHRINK_1BYTE_DATA_SIZE = 0;
		
		MIPI->AUTO_FW_DET_TRI = 1;
		
		MIPI->TRI_MODE = 1;
		
		//MIPI->PACKET_CNT_SIZE = 1;
		MIPI->PACKET_CNT_SIZE = 31;
		
		MIPI->PHA_DET_EN = 1;
		
		while(!MIPI->PHASE_DET_RDY);
		
		PhaseReport0 = MIPI->PHASE_DET_REPORT0;
		PhaseReport1 = MIPI->PHASE_DET_REPORT1;
		
		printf("PhaseReport0=0x%X, count=%d\n",PhaseReport0, count);
		printf("PhaseReport1=0x%X, count=%d\n",PhaseReport1, count);
		
		count++;
		
		if( PhaseReport0!=0 )
			break;
	}
	printf("-------------------------------\n");
	
	//Select middle phase
    StartPos = 0xFFFFFFFF;
    EndPos = 0xFFFFFFFF;
    if( PhaseReport0==0 )
	{
		ubClkSel = 15;
		printf("1. ubClkSel=0x%X\n",ubClkSel);
	}
	else
	{
		for(i=0; i<32; i++)
		{
			Bit = (PhaseReport0&(1<<i))>>i;
			if( Bit==0 )
			{
				if( StartPos==0xFFFFFFFF )
				{
					StartPos = i;
				}
			}
			else if( Bit==1 )
			{
				if( StartPos!=0xFFFFFFFF )
				{
					EndPos = i-1;
					break;
				}
			}
		}
		ContiNum = EndPos - StartPos + 1;
		if( (ContiNum%2)==1 )
		{
			ubClkSel = (EndPos+StartPos)/2;
			printf("2. ubClkSel=0x%X\n",ubClkSel);
		}
		else
		{
			ubClkSel = (EndPos+StartPos-1)/2;
			printf("3. ubClkSel=0x%X\n",ubClkSel);
		}
	}
	printf("-------------------------------\n");
	
	MIPI->CLK_SEL0 = (ubClkSel & 0x01)>>0;
	MIPI->CLK_SEL1 = (ubClkSel & 0x02)>>1;
	MIPI->CLK_SEL2 = (ubClkSel & 0x04)>>2;
	MIPI->CLK_SEL3 = (ubClkSel & 0x08)>>3;
	MIPI->CLK_SEL4 = (ubClkSel & 0x10)>>4;
}

//------------------------------------------------------------------------------
void SEN_SetISPOutputFormat(void)
{
	if(UVC_GetVdoFormat() == USB_UVC_VS_FORMAT_UNCOMPRESSED)
	{
		ISP_SenosrOutputType(ISP_RAW_DATA);
        sensor_cfg.ulPathType = ISP_RAW_DATA;
	}else{
		ISP_SenosrOutputType(ISP_NORMAL);
        sensor_cfg.ulPathType = ISP_NORMAL;
	}
}

//------------------------------------------------------------------------------
void SEN_SetIrMode(uint8_t ubMode)
{
	ubSEN_IrMode = ubMode;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetIrMode(void)
{
	return ubSEN_IrMode;
}

//------------------------------------------------------------------------------
void SEN_SetUvcPathFlag(uint8_t ubFlag)
{
	ubSEN_UvcPathFlag = ubFlag;
}

//------------------------------------------------------------------------------
void SEN_SetSensorFreeRun(uint8_t ubFlg)
{
	ubSEN_FreeRunFlg = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetSensorFreeRunFlg(void)
{
	return ubSEN_FreeRunFlg;
}

//------------------------------------------------------------------------------
void SEN_SetAlgReportFg(uint8_t ubFlg)
{
    ubSEN_AlgReportFg = ubFlg;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetAlgReportFg(void)
{
	return ubSEN_AlgReportFg;
}

//------------------------------------------------------------------------------
uint16_t uwSEN_GetVersion(void)
{
    return ((SEN_MAJORVER << 8) + SEN_MINORVER);
}
