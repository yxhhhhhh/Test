/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMX323.h
	\brief		Sensor IMX323 header
	\author		BoCun
	\version	1.2
	\date		2018-07-06
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _IMX323_H_
#define _IMX323_H_
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
// Sensor IMX323
// SLAVE_ID is 0x1A [1:7] + [0] r/w
#define SEN_SLAVE_ADDR              (0x1A)
// sensor per frame max total line
#define SEN_MAX_EXPLINE             (1125)
// system control registers
#define IMX323_CHIP_MODELID_ADDR    (0x301F)
#define IMX323_CHIP_MODELID         (0x31)
#define IMX322_FRAME_LENGTH_H		(0x0340)
#define IMX322_FRAME_LENGTH_L		(0x0341)
#define IMX322_DUMMY_LINE_H			(0x0202)
#define IMX322_DUMMY_LINE_L			(0x0203)
#define IMX322_GAIN					(0x301E)

#define IMX322_MIRROR		        (0x1 << 0)
#define IMX322_FLIP                 (0x1 << 1)
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
\brief Read data from sensor register.
\param 	uwAddress 	Sensor address.
\param 	pValue 		Point of sensor register.
\retval Value		Sensor register data.
\retval False		0->read sensor fail.
\par [Example]
\code 
		ulSEN_I2C_Read(0x04, &ubValue);
\endcode
*/
uint32_t ulSEN_I2C_Read(uint16_t uwAddress, uint8_t *pValue);
//------------------------------------------------------------------------
/*!
\brief Write data to sensor register.
\param ubAddress1 	Sensor address(MSB).
\param ubAddress2 	Sensor address(LSB).
\param ubValue 		data.
\retval True		1->I2C write sensor ok.
\retval False		0->I2C write sensor fail.
\par [Example]
\code 
			ulSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
\endcode
*/
uint32_t ulSEN_I2C_Write(uint8_t ubAddress1, uint8_t ubAddress2, uint8_t ubValue);
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
		
		sensor(IMX323) output size is 1984x1152,
			sensor_cfg.HOSize = 1984, sensor_cfg.VOSize = 1152
		Image for 510PF ISP is FHD(1920x1080),
			sensor_cfg.HOSize = 1920, sensor_cfg.VOSize = 1080
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
