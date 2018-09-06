/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OV9732.h
	\brief		Sensor OV9732 header
	\author		BoCun
	\version	1.2
	\date		2018/07/25
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _OV9732_H_
#define _OV9732_H_
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
// Sensor OV9732
// SLAVE_ID is 0x10 [1:7] + [0] r/w
#define SEN_SLAVE_ADDR              (0x10)
//sensor per frame max total line
#define SEN_MAX_EXPLINE             (802)
// system control registers
/* ID */
#define OV9732_CHIP_ID_HIGH_ADDR    (0x300A)
#define OV9732_CHIP_ID_LOW_ADDR     (0x300B)
#define OV9732_CHIP_ID              (0x9732)
#define OV9732_VTS_H                (0x380E)
#define OV9732_VTS_L                (0x380F)
#define OV9732_AEC_H                (0x3500)
#define OV9732_AEC_M                (0x3501)
#define OV9732_AEC_L                (0x3502)
#define OV9732_GAIN_H               (0x350A)
#define OV9732_GAIN_L               (0x350B)
#define OV9732_RGAIN_H              (0x5180)
#define OV9732_RGAIN_L              (0x5181)
#define OV9732_GGAIN_H              (0x5182)
#define OV9732_GGAIN_L              (0x5183)
#define OV9732_BGAIN_H              (0x5184)
#define OV9732_BGAIN_L              (0x5185)

#define OV9732_MIRROR		        (0x08)
#define OV9732_FLIP		            (0x14)
//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define SEN_SetGain(g)				{ xtSENInst.xtSENCtl.uwGain = g; SEN_WrGain(xtSENInst.xtSENCtl.uwGain); }
#define SEN_SetExpLine()			{ SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);  }
#define SEN_SetDummyLine()			{ SEN_WrDummyLine(xtSENInst.xtSENCtl.uwDmyLine); }
#define uwSEN_CalExpLine(t)			( ulSEN_GetPixClk() / 1000000L * (uint32_t)t / (uint32_t)ulSEN_GetPckPerLine() / 10)

//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!
\brief Set frame rate and pixel clock.
\param ubPclkIdx		Set frame rate number.
\return(no)
\par [Example]
\code 
		SEN_PclkSetting(SEN_FPS30);
\endcode
*/
void SEN_PclkSetting(uint8_t ubPclkIdx);
//------------------------------------------------------------------------
/*!
\brief Load sensor initial table from SF.
\return(no)
\par [Example]
\code 
		SEN_InitBySF();
\endcode
*/
void SEN_InitBySF(void);
//------------------------------------------------------------------------
/*!
\brief Setup sensor initial setting and check sensor chip ID.
\param setting 	    Parameter of sensor setting struct.
\param ubFPS 	    FPS.
\retval True		1->sensor chip initial success.
\retval False		0->read sensor chip fail.
\par [Example]
\code 
		ubSEN_Start(&sensor_cfg, ubFPS);
\endcode
*/
uint8_t ubSEN_Start (struct SENSOR_SETTING *setting, uint8_t ubFPS);
//------------------------------------------------------------------------
/*!
\brief Setup sensor initial and ISP/sensor rate.
\param setting 	    Parameter of sensor setting struct.
\param ubFPS 	    FPS.
\retval True	    1->Setup sensor setting success.
\retval False	    0->Setup sensor setting fail.
\par [Example] 
\code 
		ubSEN_Open(&sensor_cfg, ubSEN_FrameRate);
\endcode
*/
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting, uint8_t ubFPS);
//------------------------------------------------------------------------
/*!
\brief Transfer the exposure line of algorithm to exposure line and dummmy line.
\param ulAlgExpTime 	Exposure time.
\return(no)
\par [Example]
\code 
		SEN_CalExpLDmyL(ulExpTime);
\endcode
*/
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime);
//------------------------------------------------------------------------
/*!
\brief Get pixel clock from sensor struct.
\return Pixel clock value.
\par [Example]
\code 
		ulSEN_GetPixClk();
\endcode
*/
uint32_t ulSEN_GetPixClk(void);
//------------------------------------------------------------------------
/*!
\brief Get pixel clock per line from sensor struct.
\return Pixel clock per line value.
\par [Example]
\code 
		ulSEN_GetPckPerLine();
\endcode
*/
uint32_t ulSEN_GetPckPerLine(void);
//------------------------------------------------------------------------
/*!
\brief Write total line to sensor.
\return (no)
\par [Example]
\code 
		SEN_WriteTotalLine(void);
\endcode
*/
void SEN_WriteTotalLine(void);
//------------------------------------------------------------------------
/*!
\brief Write dummy line to sensor.
\param uwDL 	Number of dummy line.
\return (no)
\par [Example]
\code 
		SEN_WrDummyLine(xtSENInst.xtSENCtl.uwDmyLine);
\endcode
*/
void SEN_WrDummyLine(uint16_t uwDL);
//------------------------------------------------------------------------
/*!
\brief Write exposure line to sensor.
\param uwExpLine 	The number of exposure line.
\return (no)
\par [Example]
\code 
		SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
\endcode
*/
void SEN_WrExpLine(uint16_t uwExpLine);
//------------------------------------------------------------------------
/*!
\brief Write AGC(auto gain control) gain to sensor.
\param ulGainX1024 	Gain value.
\return (no)
\par [Example]
\code 
		SEN_WrGain(xtSENInst.xtSENCtl.uwGain);
\endcode
*/
void SEN_WrGain(uint32_t ulGainX1024);
//------------------------------------------------------------------------
/*!
\brief Turn the group hold ON after vsync.
\return (no)
\par [Example]
\code 
		SEN_GroupHoldOnVSync();
\endcode
*/
void SEN_GroupHoldOnVSync(void);
//------------------------------------------------------------------------
/*!
\brief Turn the group hold OFF after vsync.
\return (no)
\par [Example]
\code 
		SEN_GroupHoldOffVSync();
\endcode
*/
void SEN_GroupHoldOffVSync(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor output size and image size.
\return (no)
\par [Example]
\code 
		SEN_SetSensorImageSize(void);
		
		sensor(OV9732) output size is 1280x720,
			sensor_cfg.HOSize = 1280, sensor_cfg.VOSize = 720
		Image for 510PF ISP is HD(1280x720),
			sensor_cfg.HOSize = 1280, sensor_cfg.VOSize = 720
\endcode
*/
void SEN_SetSensorImageSize(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor type.
\return (no)
\par [Example]
\code 
		SEN_SetSensorType();
\endcode
*/
void SEN_SetSensorType(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor mirror/flip.
\param ubMirrorEn 	mirror.
\param ubFlipEn 	flip.
\return (no)
*/
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn);
#endif
