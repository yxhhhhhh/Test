/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file	    SEN.h
	\brief		Sensor funcations header
	\author     BoCun
	\version    1.1
	\date		2018-09-11
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _SENSOR_H_
#define _SENSOR_H_
#include <stdbool.h>
#include "_510PF.h"
#include "IQ_PARSER_API.h"
#include "USBD_API.h"
//------------------------------------------------------------------------------
/*!	\file SEN.h
SENSOR FlowChart:
	\dot
	digraph sensor_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=4];
		"Set sensor default value. \n SEN_SetSensorInitVal()"->
		"Setup sensor relation IRQ. \n SEN_ISRInitial()"->
		"Set sensor output size and ISP process size. \n SEN_SetWindowSize()"->
		"Set ISP output size. \n SEN_SetResolution()"->
		"Set and start sensor. \n ubSEN_Open(&sensor_cfg)";
		"Set video steam. \n SEN_SetPathState(SENSOR_PATH1, 1)"->
		"Enable video stram. \n SEN_EnableVideo()"->
		"Load IQ.bin to dram. \n IQ_ParserIQTable()"->
		"Strat ISP.(like as AE, AWB...etc) \n SEN_ISPInitial()"->
		"Output Image Source \n wait VSYNC, HSYNC, HW_END interrupt";	
		
		"Set and start sensor. \n ubSEN_Open(&sensor_cfg)"-> "Set video steam. \n SEN_SetPathState(SENSOR_PATH1, 1)"[label = "Pass"];		
		"Set and start sensor. \n ubSEN_Open(&sensor_cfg)"-> "Restart sensor initial" [label = "Fail"];	
	}
	\enddot
*/

//==============================================================================
// DEFINITION
//==============================================================================
enum _SENSOR_CLK_GEN
{
	SENSOR_96MHz = 0,	
	SENSOR_120MHz,
};

enum
{
	SEN_FPS05 = 5,
	SEN_FPS10 = 10,
    SEN_FPS15 = 15,
    SEN_FPS20 = 20,
    SEN_FPS25 = 25,
    SEN_FPS30 = 30,
};

enum _SENSOR_POWER_FREQ
{
    SENSOR_PWR_FREQ_AUTO = 0,
	SENSOR_PWR_FREQ_50HZ,	
	SENSOR_PWR_FREQ_60HZ,
};

enum
{
	SEN_SCALE_DOWN = 0,	
	SEN_SCALE_UP ,
};

enum _SENSOR_PATH
{
	SENSOR_PATH1 = 1,
	SENSOR_PATH2,
	SENSOR_PATH3,
    // ISP pipe 
    ISP_3DNR, 
    ISP_MD_W0, 
    ISP_MD_W1, 
    ISP_MD_W2,   
    IQ_BIN_FILE,      
};

//==============================================================================
// STRUCT
//==============================================================================
//--------------------------------
// Sensor setting declare
//--------------------------------
struct SENSOR_SETTING {
	uint32_t ulPathType;					//!< Bypass mode
	uint32_t ulSensorType;				    //!< Sensor type
	uint32_t ulPatternMode;				    //!< Currrent pattern mode 
	uint32_t ulSensorRes;					//!< Currrent sensor resolution	
	
	uint32_t ulFrameSize1;				    //!< Path1 frame size
	uint32_t ulFrameSize2;				    //!< Path2 frame size
	uint32_t ulFrameSize3;				    //!< Path3 frame size
	
	uint32_t ulHSize1;						//!< Path1 horizontal Size
	uint32_t ulVSize1;						//!< Path1 vertical Size
	uint32_t ulHSize2;						//!< Path2 horizontal Size
	uint32_t ulVSize2;						//!< Path2 vertical Size
	uint32_t ulHSize3;						//!< Path3 horizontal Size
	uint32_t ulVSize3;						//!< Path3 vertical Size
	
	uint32_t ulHSize;						//!< Image input horizontal size
	uint32_t ulVSize;						//!< Image input vertical Size
	uint32_t ulHOSize;						//!< Sensor output horizontal size
	uint32_t ulVOSize;						//!< Sensor output vertical Size

	uint32_t ulPath1_Addr;				    //!< Video stream1 start address
	uint32_t ulPath2_Addr;				    //!< Video stream2 start address
	uint32_t ulPath3_Addr;				    //!< Video stream3 start address
	uint32_t ulPath4_Addr;				    //!< 3DNR ring buffer start address
	uint32_t ulPath5_Addr;				    //!< 3DNR ring buffer end address
	uint32_t ulPath6_Addr;				    //!< MDv2 M0 address
	uint32_t ulPath7_Addr;				    //!< MDv2 M1 address
	uint32_t ulPath8_Addr;				    //!< MDv2 M2 address	
	uint32_t ulPath9_Addr;				    //!< IQ bin file address

	uint32_t ulSensorPclk;				    //!< Sensor Pixel clock
	uint32_t ulSensorPclkPerLine;	        //!< Sensor Pixel clock per line
	uint32_t ulSensorFrameRate;             //!< Sensor frame rate
	uint32_t ulMaximumSensorFrameRate;      //!< Sensor maximun frame rate
    
	uint8_t	ubPath1Src;					    //!< Path1 source type
	uint8_t	ubPath2Src;					    //!< Path2 source type
	uint8_t	ubPath3Src;					    //!< Path3 source type
};

typedef struct tagSENCtl {
	uint16_t uwExpLine;						//!< Exposure line
	uint16_t uwDmyLine;						//!< Dummy line
	uint32_t ulExpTime;						//!< Exposure time
	uint16_t uwGain;						//!< Gain control
    uint16_t uwTotalGain;					//!< 
    uint16_t uwBlueGain;					//!< Blue channel offset for AWB
    uint16_t uwRedGain;						//!< Red channel offset for AWB
    uint16_t uwGreenGain;					//!< Green channel offset for AWB
} tfSENCtl;

typedef struct tagSENObj {
	uint8_t 	ubState;					//!< Current state
	uint8_t 	ubPrevState;				//!< Previous state
	uint8_t 	ubLastInitIdx;
	uint8_t 	ubLastFRIdx;
    tfSENCtl 	xtSENCtl;
	uint8_t 	ubBuf[4];
	uint8_t 	ubImgMode;					//!< Image mirror/ flip control
	uint16_t 	uwMaxExpLine;				//!< Maximum exposure line
} tfSENObj;

typedef struct {
	uint8_t		ubSrcNum;
	uint8_t 	ubCurNode;	
	uint8_t 	ubNextNode;
	uint32_t	ulDramAddr1;
	uint32_t	ulDramAddr2;	
	uint32_t	ulSize;
	uint8_t 	ubPath;
}SEN_EVENT_PROCESS;

typedef void (*pvSEN_CbFunc)(void);

//==============================================================================
// DEFINITION
//==============================================================================
	//---------------------------------------
	// Sensor Format
	//---------------------------------------
	#define SEN_OV9715				0		
	#define SEN_OV9732				1			
	#define SEN_AR0330				2		
	#define SEN_H62				    3		
	#define SEN_H62_MIPI		    4
	#define SEN_IMX323  		    5
	#define SEN_SC2235  		    6

	#define SEN_USE				    SEN_H62

	//---------------------------------------
	// Image for ISP
	//---------------------------------------
    #define ISP_HD				0	
    #define ISP_FHD				1	
    #define ISP_1296P			2

    #if (SEN_USE == SEN_OV9715)
        #include "OV9715.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_OV9732)
        #include "OV9732.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_AR0330)
        #include "AR0330.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_H62)
        #include "H62.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_H62_MIPI)
        #include "H62_MIPI.h"
        #define ISP_RES				ISP_HD
    #elif (SEN_USE == SEN_IMX323)
        #include "IMX323.h"
        #define ISP_RES				ISP_FHD
    #elif (SEN_USE == SEN_SC2235)
        #include "SC2235.h"
        #define ISP_RES				ISP_FHD
    #endif

    #if (ISP_RES == ISP_HD)  
        #define ISP_WIDTH	1280
        #define ISP_HEIGHT	720

        #define YUY2_WIDTH	1280
        #define YUY2_HEIGHT	720
    #elif (ISP_RES == ISP_FHD)
        #define ISP_WIDTH	1920
        #define ISP_HEIGHT	1088 
        
        #define YUY2_WIDTH	1920
        #define YUY2_HEIGHT	1080
    #elif (ISP_RES == ISP_1296P)
        #define ISP_WIDTH	2304
        #define ISP_HEIGHT	1296 
        
        #define YUY2_WIDTH	1920
        #define YUY2_HEIGHT	1080
    #endif

////----------------------------------------------------------
	#define AE_EN									1
	#define AWB_EN									1
	#define AF_EN									1
	#define IQ_DN_EN								1
////---------------------------------------------------------- 

    #if((AE_EN == 1) && (AWB_EN == 1) && (AF_EN == 1))
        #define SEN_FrmEnd_ISR()				{ AE_FrmEndIsr_Handler(); AWB_FrmEndIsr_Handler(); AF_FrmEndIsr_Handler(); }		// AE AF AWB
    #elif((AE_EN == 1) && (AWB_EN == 1) && (AF_EN != 1))
        #define SEN_FrmEnd_ISR()				{ AE_FrmEndIsr_Handler(); AWB_FrmEndIsr_Handler(); }								// AE AWB
    #elif((AE_EN == 1) && (AWB_EN != 1) && (AF_EN == 1))
        #define SEN_FrmEnd_ISR()				{ AE_FrmEndIsr_Handler(); AF_FrmEndIsr_Handler();  }								// AE AF
    #elif((AE_EN == 1) && (AWB_EN != 1) && (AF_EN != 1))
        #define SEN_FrmEnd_ISR()				{ AE_FrmEndIsr_Handler(); }															// AE
    #elif((AE_EN != 1) && (AWB_EN == 1) && (AF_EN == 1))
        #define SEN_FrmEnd_ISR()				{ AWB_FrmEndIsr_Handler(); AF_FrmEndIsr_Handler();  } 								// AWB AF
    #elif((AE_EN != 1) && (AWB_EN == 1) && (AF_EN != 1))
        #define SEN_FrmEnd_ISR()				{ AWB_FrmEndIsr_Handler(); } 														// AWB
    #elif((AE_EN != 1) && (AWB_EN != 1) && (AF_EN == 1))																				 
        #define SEN_FrmEnd_ISR()				{ AF_FrmEndIsr_Handler();  }														// AF
    #else
        #define SEN_FrmEnd_ISR()				{  }																				// Null
    #endif

    #define ISP_GEN				ISP_3G5_292
    #define SEN_SRC_NONE	    0xFF				//!< Source Number (NONE) 
    #define ISP_ALG_REPORT_CNT	1    
//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!	
\brief Get acitve flag
\param ubPath ISP Path
\return Flag
*/
uint8_t ubSEN_GetActiveFlg(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief 	Get first out flag
\return Flag
*/
uint8_t ubSEN_GetFirstOutFlg(void);

//------------------------------------------------------------------------
/*!
\brief 	Set change resoltuion state
\return Flag
*/
void SEN_SetResChgState(uint8_t ubPath, uint16_t uwH, uint16_t uwV);

//------------------------------------------------------------------------
/*!
\brief 	Check ISP state.
\return (no)
*/
void SEN_ChkISPState(void);    
//------------------------------------------------------------------------
/*!
\brief ISP(AE, AF, AWB, MD, DIS...etc) initialize.
\return(no)
\par [Example]
\code 
		SEN_ISPInitial();	
\endcode	
*/
void SEN_ISPInitial(void);
//------------------------------------------------------------------------
/*!
\brief ISR(HW_END, VSYNC...etc) and task initialize
\return(no)
\par [Example]
\code 
		SEN_ISRInitial();	
\endcode	
*/
void SEN_ISRInitial(void);
//------------------------------------------------------------------------
/*!
\brief Sensor HSync interrupt
\return(no)
\par [Example]
\code 
		INTC_IrqSetup(INTC_SEN_HSYNC_IRQ, INTC_LEVEL_TRIG, SEN_Hsync_ISR);
\endcode	
*/
void SEN_Hsync_ISR(void);
//------------------------------------------------------------------------
/*!
\brief Sensor Hw_end interrupt
\return(no)
\par [Example]
\code 
		INTC_IrqSetup(INTC_ISP_WIN_END_IRQ, INTC_LEVEL_TRIG, SEN_HwEnd_ISR);
\endcode	
*/
void SEN_HwEnd_ISR(void);
//------------------------------------------------------------------------
/*!
\brief Sensor VSync interrupt.
\return(no)
\par [Example]
\code 
		INTC_IrqSetup(INTC_SEN_VSYNC_IRQ, INTC_LEVEL_TRIG, SEN_Vsync_ISR);
\endcode	
*/
void SEN_Vsync_ISR(void);
//------------------------------------------------------------------------
/*!
\brief Initial Vsync task.
\return(no)
\par [Example]
\code 
		SEN_VsyncInit();
\endcode
*/
void SEN_VsyncInit(void);
//------------------------------------------------------------------------
/*!
\brief Setup Sensor resolution.
\param ubPath 	Select video path.
\param ulWidth 	Horizontal size.
\param ulHeight Vertical size.
\return(no)
\par [Example]
\code 
		SEN_SetResolution(SENSOR_PATH1, 1280, 720);
\endcode	
*/
void SEN_SetResolution(uint8_t ubPath, uint32_t ulWidth,uint32_t ulHeight);
//------------------------------------------------------------------------
/*!
\brief Set sensor output and ISP process size.
\return(no)
\par [Example]
\code 
		SEN_SetWindowSize();

			Ho								(H_s, V_s)		 H	
	-----------------								-----------------
	|				|								|				|
	|	Sensor	    |	Vo	-------------->			|	    ISP		| V
	|				|								|				|
	-----------------								-----------------
\endcode	
*/
void SEN_SetWindowSize(void);		
//------------------------------------------------------------------------
/*!
\brief Set video stream path switch.
\param ubPath 		Select video path.
\param ubFlag 		Video on/off.
\return(no)
\par [Example]
\code 
		SEN_SetPathState(SENSOR_PATH1,1);
\endcode
*/
void SEN_SetPathState(uint8_t ubPath, uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Enable sensor write data to dram.
\return(no)
\par [Example]
\code 
		SEN_EnableVideo();
\endcode
*/
void SEN_EnableVideo(void);
//------------------------------------------------------------------------
/*!
\brief Disable sensor write data to dram.
\return(no)
\par [Example]
\code 
		SEN_DisableVideo();
\endcode
*/
void SEN_DisableVideo(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor default value.
\return(no)
\par [Example]
\code 
		SEN_SetSensorInitVal();
\endcode
*/
void SEN_SetSensorInitVal(void);
//------------------------------------------------------------------------
/*!
\brief Get sensor type.
\return(no)
\par [Example]
\code 
		ubSEN_GetSensorType();
\endcode
*/
uint8_t ubSEN_GetSensorType(void);
//------------------------------------------------------------------------
/*!
\brief Get path horizontal size from sensor struct.
\param ubType 		select path1/2/3.
\return horizontal size of sensor struct.
\par [Example]
\code 
		ulSEN_GetSenHSize(ubPath);
\endcode
*/
uint32_t ulSEN_GetSenHSize(uint8_t ubType);
//------------------------------------------------------------------------
/*!
\brief Get path vertical size from sensor struct.
\param ubType 		select path1/2/3.
\return vertical size of sensor struct.
\par [Example]
\code 
		ulSEN_GetSenVSize(ubPath);
\endcode
*/
uint32_t ulSEN_GetSenVSize(uint8_t ubType);
//------------------------------------------------------------------------
/*!
\brief 	Sensor initialize
\return	(no)
\par [Example]
\code 
		SEN_InitProcess();	
\endcode		
*/
#ifdef OP_STA
void SEN_InitProcess (void);
#else
#define SEN_InitProcess()			((void)0)
#endif
//------------------------------------------------------------------------
/*!
\brief Set ISP rate.
\param ulISP_div 		Number of divison.
\return(no)
\par [Example]
\code 
		SEN_SetISPRate(8);
		
		F(ISP_CLK)= DDR_PLL/ (2^SYS_RATE) /ISP_RATE.
		where SYS_RATE is 0x9000_0008[1:0]
\endcode
*/
void SEN_SetISPRate(uint32_t ulISP_div);
//------------------------------------------------------------------------
/*!
\brief Set sensor rate.
\param ubSelBaseClk 	Select sensor clock generator.
\param ubSensorDiv 		Number of divison.
\return(no)
\par [Example]
\code 
		SEN_SetSensorRate(SENSOR_96MHz, 12);
		
		sensor clock = 96MHz(120MHz) / SEN_RATE
\endcode
*/
void SEN_SetSensorRate(uint8_t ubSelBaseClk, uint8_t ubSensorDiv);
//------------------------------------------------------------------------
/*!
\brief 	Set sensor output source
\param 	ubPath1Src	Source number for PATH1
\param 	ubPath2Src	Source number for PATH2
\param 	ubPath3Src	Source number for PATH3
\return (no)
\par [Example]
\code 
		SEN_SetPathSrc(KNL_SRC_NONE,KNL_SRC_NONE,KNL_SRC_1_SUB);
\endcode
*/
void SEN_SetPathSrc(uint8_t ubPath1Src,uint8_t ubPath2Src,uint8_t ubPath3Src);
//------------------------------------------------------------------------
/*!
\brief 	Get sensor output source
\param 	ubPath Sensor path
\return Source number
\par [Example]
\code 
		ubSEN_GetPathSrc(SENSOR_PATH1);
\endcode
*/
uint8_t ubSEN_GetPathSrc(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief Set path buffer address
\param ubPath	Output path
\param ulBufAddr	Buffer address
\return (no)
\par [Example]
\code 
		SEN_SetPathAddr(SENSOR_PATH1, ulTemp1);
\endcode
*/
void SEN_SetPathAddr(uint8_t ubPath,uint32_t ulBufAddr);
//------------------------------------------------------------------------
/*!
\brief 	Set sensor output resolution
\param 	ubPath	Output path
\param 	uwH	Horizontal resolution
\param 	uwV	Vertical resolution
\return (no)
\par [Example]
\code 
		SEN_SetOutResolution(ubPath, ulWidth, ulHeight);
\endcode
*/
void SEN_SetOutResolution(uint8_t ubPath,uint16_t uwH,uint16_t uwV);
//------------------------------------------------------------------------
/*!
\brief 	Set acitve flag
\param 	ubFlg	Flag
\param ubPath ISP Path
\return (no)
\par [Example]
\code 
		SEN_SetActiveFlg(1);
\endcode
*/
void SEN_SetActiveFlg(uint8_t ubPath,uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Set first out flag
\param ubFlg	Flag
\return (no)
\par [Example]
\code 
		SEN_SetFirstOutFlg(1);
\endcode
*/
void SEN_SetFirstOutFlg(uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Set state change flag
\param ubFlg	Flag
\return (no)
\par [Example]
\code 
		SEN_SetStateChangeFlg(1);
\endcode
*/
void SEN_SetStateChangeFlg(uint8_t ubFlg);
//------------------------------------------------------------------------
/*!
\brief Get state change flag
\return Flag
\par [Example]
\code 
		ubSEN_GetStateChangeFlg(1);
\endcode
*/
uint8_t ubSEN_GetStateChangeFlg(void);
//------------------------------------------------------------------------
/*!
\brief Update path buffer address
\return (no)
\par [Example]
\code 
		SEN_UpdatePathAddr();
\endcode
*/
void SEN_UpdatePathAddr(void);
//------------------------------------------------------------------------
/*!
\brief Set clock enable
\param ubEnable	0->disable,1->enable
\return (no)
\par [Example]
\code 
		SEN_SetClkEn(1);
\endcode
*/
void SEN_SetClkEn(uint8_t ubEnable);
//------------------------------------------------------------------------
/*!
\brief Register event queue
\param tQueueId	Queue
\return (no)
\par [Example]
\code 
		SEN_RegisterEventQueue(KNL_ProcessQueue);
\endcode
*/
#ifdef OP_STA
    void SEN_RegisterEventQueue(osMessageQId tQueueId);
#else
    #define SEN_RegisterEventQueue(tQueueId)	((void)0)
#endif
//------------------------------------------------------------------------
/*!
\brief Register event node
\param ubEventNode	Event node
\return (no)
\par [Example]
\code 
		SEN_RegisterEventNode(KNL_NODE_SEN_YUV_BUF);
\endcode
*/
#ifdef OP_STA
    void SEN_RegisterEventNode(uint8_t ubEventNode);
#else
    #define SEN_RegisterEventNode(ubEventNode)	((void)0)
#endif
//------------------------------------------------------------------------------
/*!
\brief Set data lane0 digital power down large-scale timing
\param ubPclkIdx		Set frame rate number.
\return(no)
\par [Example]
\code 
		MIPI_PowerDownTimingSelect(SEN_FPS30);
\endcode
*/
void MIPI_PowerDownTimingSelect(uint8_t ubPclkIdx);
//------------------------------------------------------------------------------
/*!
\brief 	MIPI auto phase dectection and setting
\return	none.
\par none
\code		 
	 MIPI_AutoPhaseDetect();
\endcode
*/
void MIPI_AutoPhaseDetect(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set ISP output according to USB format.	
\par [Example]
\code		 
	 SEN_SetISPOutputFormat();
\endcode
*/
void SEN_SetISPOutputFormat (void);
//------------------------------------------------------------------------------
/*!
\brief 	Set flag of UVC path, 1:UVC path exist.	
\par [Example]
\code		 
	 SEN_SetUvcPathFlag(1);
\endcode
*/
void SEN_SetUvcPathFlag(uint8_t ubFlag);
//------------------------------------------------------------------------------
/*!
\brief 	Get IR cut mode state.	
*/
uint8_t ubSEN_GetIrMode(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set IR cut mode state.	
*/
void SEN_SetIrMode(uint8_t ubMode);
//------------------------------------------------------------------------------
/*!
\brief 	Get SEN function version	
\return	sensor version.
\par [Example]
\code		 
	 uwSEN_GetVersion();
\endcode
*/
uint16_t uwSEN_GetVersion (void);
//------------------------------------------------------------------------------
/*!
\brief 	Set sensor free run flag.	
\par [Example]
\code		 
	 SEN_SetSensorFreeRun(1);
\endcode
*/
void SEN_SetSensorFreeRun(uint8_t ubFlg);
//------------------------------------------------------------------------------
/*!
\brief 	Get sensor free run flag.	
\return	Free run falg.
\par [Example]
\code		 
	 ubSEN_GetSensorFreeRunFlg();
\endcode
*/
uint8_t ubSEN_GetSensorFreeRunFlg(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set AE and AWB report ready flag.	
\par [Example]
\code		 
	 SEN_SetAlgReportFg(1);
\endcode
*/
void SEN_SetAlgReportFg(uint8_t ubFlg);
//------------------------------------------------------------------------------
/*!
\brief 	Get AE and AWB report ready flag.	
\return	report ready falg.
\par [Example]
\code		 
	 ubSEN_GetAlgReportFg();
\endcode
*/
uint8_t ubSEN_GetAlgReportFg(void);
//------------------------------------------------------------------------
/*!
\brief Sensor image stable call back function
\param pvCB     Callback function when image stable is ready
\return (no)
*/
void SEN_SetIspFinishCbFunc(pvSEN_CbFunc pvCB);
//------------------------------------------------------------------------
/*!
\brief 	Load IQ bin file.
\return	(no)	
*/
#ifdef OP_STA
    void SEN_LoadIQData(void);
#else
    #define SEN_LoadIQData()	((void)0)
#endif
//------------------------------------------------------------------------------
/*!
\brief 	Set frame rate.
\param ubPath		Frame drop path.
\param ubFPS		Frame rate.
\par [Example]
\code		 
    uint8_t ubFPS = 10;
    SEN_SetFrameRate(SENSOR_PATH1, ubFPS)

    where ubFPS maximum is 30.
\endcode
*/
#ifdef OP_STA
    void SEN_SetFrameRate(uint8_t ubPath, uint8_t ubFPS);
#else
    #define SEN_SetFrameRate(ubPath, ubFPS)	((void)0)
#endif
//------------------------------------------------------------------------------
/*!
\brief 	Set frame drop.
\param ubPath		Frame drop path.
\param ubEnable		Frame drop switch.
\param ubN		    Set N.
\param ubM		    Set M.
\par [Example]
\code		 
	 Path1_fps = sensor_fps * (N1/M1).
     Path2_fps = sensor_fps * (N1/M1) * (N2/M2).
     Path3_fps = sensor_fps * (N1/M1) * (N3/M3).
     M > N when ubEnable = 1.
\endcode
*/
void SEN_SetFrameDrop(uint8_t ubPath, uint8_t ubEnable, uint8_t ubN, uint8_t ubM);
//------------------------------------------------------------------------------
/*!
\brief 	Get frame drop state.	
\param ubPath		Frame drop path.
\return drop state. 
\par [Example]
\code		 
     
     If sensor_fps = 30, set path1 to 10fps.
        SEN_SetFrameDrop(SENSOR_PATH1, 1, 1, 3);
     
	 bSEN_GetFrameDropState(SENSOR_PATH1);
        1 0 0 1 0 0 1 0 0 1
        0 0 1 0 0 1 0 0 1 0
        0 1 0 0 1 0 0 1 0 0
     1: frame isn't drop
     0: frame is drop. 
\endcode
*/
bool bSEN_GetFrameDropState(uint8_t ubPath);
//------------------------------------------------------------------------
/*!
\brief Set raw reorder.
\param ubMirrorEn   mirror on/off.
\param ubFlipEn     flip  on/off.
\return(no)
\par [Example]
\code		 
     Normal
        B  Gb B  Gb ...
        Gr R  Gr R  ...
     Mirror
        Gb B  Gb B  ...
        R  Gr R  Gr ...
     Flip   
        Gr R  Gr R  ...
        B  Gr B  Gr ...
     Mirror & Flip
        R  Gr R  Gr ...
        Gr B  Gr B  ...
\endcode
*/
void SEN_SetRawReorder(uint8_t ubMirrorEn, uint8_t ubFlipEn);
//==============================================================================
// SENSOR extern item
//==============================================================================
extern struct SENSOR_SETTING sensor_cfg;
extern tfSENObj xtSENInst;
#endif
