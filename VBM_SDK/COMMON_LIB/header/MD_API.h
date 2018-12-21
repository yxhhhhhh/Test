/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		MD_API.c
	\brief		Motion detection API function header
	\author		BoCun
	\version	0.5
	\date		2018-05-21
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _MD_API_H_
#define _MD_API_H_

#include "_510PF.h"
//------------------------------------------------------------------------------
/*!	\file MD_API.h
MD FlowChart:
	\dot
	digraph MD_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=2];
		MD_Init->
		"MD_SetROIindex \n MD_SetROIweight \n MD_SetROImask "->
		"Wait MD interrput"->
		uwMD_GetCnt;	
		
		"MD_SetROIindex \n MD_SetROIweight \n MD_SetROImask "->
		"Set or modify, \n must colse MD_Switch(OFF)"->
		"Modify OK,\n then open MD_Switch(ON)";
	}
	\enddot
*/

/*
	Motion detection flow
    1.Disable MD.
    2.Set ROI index/weight/mask and some register.
    3.Enable MD.
    4.Wait MD interrupt.
    5.Get MD report.
    ***If user want to set or modify ROI value, must disable MD***
*/

//==============================================================================
// STRUCT
//==============================================================================
enum
{
	MD_OFF = 0,
	MD_ON,
};

enum
{
	MD_UNSTABLE = 0,
	MD_STABLE,
};

enum
{
	MD_REG1_CNT_00 = 0,
	MD_REG1_CNT_01,
	MD_REG1_CNT_02,
	MD_REG1_CNT_03,
	MD_REG1_CNT_04,
	MD_REG1_CNT_05,
	MD_REG1_CNT_06,
	MD_REG1_CNT_07,
	MD_REG1_CNT_08,
	MD_REG1_CNT_09,
	MD_REG1_CNT_10,
	MD_REG1_CNT_11,
	MD_REG1_CNT_12,
	MD_REG1_CNT_13,
	MD_REG1_CNT_14,
	MD_REG1_CNT_15,	
};

typedef void (*pvMD_CbFunc)(uint8_t ubReport);
//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!
\brief Set motion detection initialization.
\return(no)
\par [Example]
\code 
		MD_Init(void);
\endcode
*/
void MD_Init(void);
//------------------------------------------------------------------------
/*!
\brief Set motion detection ROI group of report.
\param ulIndexData 	Point of index data array.
\return(no)
\par [Example]
\code
    [Note]
        If image window size is 1280x720, 
            division of 30x23 blocks in image and each block size is {1280/30, 720/23}.
        Each block can be assigned one index. 
        One index is 4 bits data, and there are 8 index data in one word.
    
    [Initial]
        Write 0 for all ROI index data. 
        (word data of ROI index of report x (Data) is 32'h0000_0000)
        if user does not give any ROI index.
    
    [Illustration]
                        30
 				---------------------
 				|  0|  1|... ...| 29| 
 				| 30| 31|... ...| 59|
              23|				    |
 			 	|...   ...  |688|689|		
				---------------------	

Ex: Set block0 to block59 is index[1] and other is index[0].
    ulMD_Weight[87] = {  0x11111111,
                        ...,
                        0x00001111,                  
                        ...,
                        0x00000000,};
    MD_SetROIindex(&ulMD_Index[0]);
    
                    MSB                   LSB
    block number:   | 7| 6| 5| 4| 3| 2| 1| 0|  
    ulMD_Weight[0]: 0x1  1  1  1  1  1  1  1     
\endcode
*/
void MD_SetROIindex(uint32_t *ulIndexData);
//------------------------------------------------------------------------
/*!
\brief Set motion detection ROI weight of report.
\param ulGroupData 	Point of group data array.
\return(no)
\par [Example]
\code 
    [Note]
        If image window size is 1280x720, 
            division of 30x23 blocks in image and each block size is {1280/30, 720/23}.
        Each block can be assigned one weight. 
        One weight is 4 bits data, and there are 8 weight data in one word.
        Larger value lets input data be decided to Move Object difficultly.
        Smaller value lets input data be decided to Move Object easily.
    
    [Initial]
        Write 4 for all ROI weight data. 
        (word data of ROI weight of report x (Data) is 32'h4444_4444)
        if user does not give any ROI weight.
    
    [Illustration]
                        30
 				---------------------
 				|  0|  1|... ...| 29| 
 				| 30| 31|... ...| 59|
              23|				    |
 			 	|...   ...  |688|689|		
				---------------------	

Ex: Set block0 to block59 is weight[1] and other is weight[4].
    ulMD_Weight[87] = {  0x11111111,
                        ...,
                        0x44441111,                  
                        ...,
                        0x44444444,};
    MD_SetROIindex(&ulMD_Index[0]);
    
                    MSB                   LSB
    block number:   | 7| 6| 5| 4| 3| 2| 1| 0|  
    ulMD_Weight[0]: 0x1  1  1  1  1  1  1  1 
\endcode
*/
void MD_SetROIweight(uint32_t *ulGroupData);
//------------------------------------------------------------------------
/*!
\brief Set motion detection threshold.
\param ubThd 	threshold.
\return(no)
\par [Example]
\code
    [Note]
        Threshold value associated with dark current noise.
        Larger value lets input data be decided to Move Object difficultly.
        Smaller value lets input data be decided to Move Object easily.
    
    uint8_t ubMDthreshold = 80;
    
    MD_SetSensitivity(ubMDthreshold);
        where ubMDthreshold is 0-255.
\endcode
*/
void MD_SetSensitivity(uint8_t ubThd);
//------------------------------------------------------------------------
/*!
\brief  Set motion detection threshold from user.
\param  uwValue 	threshold value.
\return (no)
\par [Example]
\code 
    uint16_t uwValue = 50;
    
    MD_SetUserThreshold (uwValue);
        where uwValue limit is 0-10800.
\endcode
*/
void MD_SetUserThreshold (uint16_t uwValue);
//------------------------------------------------------------------------
/*!
\brief Motion detection switch.
\param ubSwitch 	Motion detection on/off.
\return(no)
\par [Example]
\code 
    MD switch can be set in anytime, and is active in next VSYNC.
    
    MD_Switch(MD_ON);
\endcode
*/
void MD_Switch(uint8_t ubSwitch);
//------------------------------------------------------------------------
/*!
\brief Set motion detection stable state for system start.
\param ubMdState 	flag.
\return(no)
*/
void MD_SetMdState(uint8_t ubMdState);
//------------------------------------------------------------------------
/*!
\brief Get motion detection count.
\param ubIndex 	Select of Index.
\return motion count value.
\par [Example]
\code 
        [Note]
            Set MD_SetROIindex select interesting area finish, 
                then use uwMD_GetCnt get motion count report.
        
        uint16_t uwMDcount;
        
		uwMDcount = uwMD_GetCnt(ubIndex);
		where ubIndex equal 0~15, could get index-motion count report.      
\endcode
*/
uint16_t uwMD_GetCnt(uint8_t ubIndex);
//------------------------------------------------------------------------
/*!
\brief Get motion detection address.
\return address.
\par [Example]
\code 
        [Note]
            Set MD_SetROIindex select interesting area finish, 
                then use uwMD_GetCnt get motion count report.
        
        uint16_t uwMDcount;
        
		uwMDcount = uwMD_GetCnt(ubIndex);
		where ubIndex equal 0~15, could get index-motion count report.      
\endcode
*/
uint32_t ulMD_GetMdAddr(void);
//------------------------------------------------------------------------
/*!
\brief  Motion detection report ready call back function
\param  pvCB     Callback function when motion detection report is ready
\return (no)
*/
void MD_ReportReadyCbFunc(pvMD_CbFunc pvCB);
#endif
