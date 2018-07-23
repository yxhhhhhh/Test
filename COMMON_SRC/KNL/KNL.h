/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		KNL.h
	\brief		Kernel Control header file
	\author		Justin Chen
	\version	1.9
	\date		2018/06/29
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __KNL_h
#define __KNL_h

#include <stdio.h>
#include "_510PF.h"
#include "ADO_API.h"
#include "BB_API.h"
#include "H264_API.h"
#include "TWC_API.h"

#define KNL_AUX_CRC_FUNC				0		//!< Auxiliary CRC Function
#define KNL_MIN_COMPRESS_RATIO			7		//!< Minimum Compression Ratio
#define KNL_BB_RX_DON_Q_SIZE			10		//!< RX Queue Size
#define KNL_PROC_QUEUE_NUM				120		//!< KNL_Process Queue Size
#define KNL_AVG_PLY_QUEUE_NUM			30		//!< KNL_AvgPlyProcess Queue Size
#define KNL_JPEG_QUEUE_NUM				64		//!< JPEG_Monitor Queue Size
#define KNL_MAX_NODE_NUM				16		//!< Maximum Node Number
#define KNL_SRC_NUM						20		//!< Source Number

//#define KNL_ADO_SUB_PKT_LEN				48		//!< Audio Sub-Packet Length
#define KNL_ADO_SUB_PKT_LEN				36		//!< Audio Sub-Packet Length

#define KNL_AVG_PLY_CNT_TH				4
#define KNL_ADO_DEBUG_EN				0

#if defined(VBM_BU) || defined(BUC_CAM)
#define KNL_LCD_FUNC_ENABLE				0
#else
#define KNL_LCD_FUNC_ENABLE				1
#endif

#define KNL_SD_FUNC_ENABLE				1
#define KNL_REC_FUNC_ENABLE				0

#define KNL_USBH_FUNC_ENABLE            0

#define KNL_RFPWR_CTRL_ENABLE			0

typedef enum
{	
	KNL_FPS_OUT		= 0,			//!< Output FPS
	KNL_FPS_IN,						//!< Input FPS
	KNL_BB_FRM_OK
}KNL_FPS_TYPE;

typedef enum
{	
	KNL_OPMODE_VBM_1T		= 0,	//!< VBM->1T Operation Mode
	KNL_OPMODE_VBM_2T		= 1,	//!< VBM->2T Operation Mode
	KNL_OPMODE_VBM_4T		= 2,	//!< VBM->4T Operation Mode	
	KNL_OPMODE_BUC_H 		= 3,	//!< BUC->H Operation Mode	
	KNL_OPMODE_BUC_3T_1T2B 	= 4,	//!< BUC->3T_1T2B Operation Mode
	KNL_OPMODE_BUC_3T_2T1B 	= 5,	//!< BUC->3T_2T1B Operation Mode
	KNL_OPMODE_BUC_3T_2L1R 	= 6,	//!< BUC->3T_2L1R Operation Mode
	KNL_OPMODE_BUC_3T_1L2R 	= 7,	//!< BUC->3T_1L2R Operation Mode
	KNL_OPMODE_BUC_2T_1T1B 	= 8,	//!< BUC->2T_1T1B Operation Mode
	KNL_OPMODE_BUC_1T 		= 9,	//!< BUC->1T Operation Mode
}KNL_OPMODE;

typedef enum
{	
	KNL_NORMAL_PLY = 0,
	KNL_AVG_PLY,
}KNL_PLY_MODE;

typedef enum
{	
	KNL_NODE_START = 0,					//!< Start State
	KNL_NODE_TRANS = 1,					//!< Transition State
	KNL_NODE_STOP  = 0xFF,				//!< Stop State
}KNL_NODE_STATE;

typedef enum
{	
	KNL_DATA_TYPE_VDO = 0,					
	KNL_DATA_TYPE_ADO = 1,			
}KNL_DATA_TYPE;

typedef enum
{
	KNL_DISP_ROTATE_0,					//!< Without Rotate
	KNL_DISP_ROTATE_90					//!< With Rotate (90 Degree)	
}KNL_DISP_ROTATE;

typedef enum
{	
	KNL_SEN_MAIN_PATH = 0,				//!< MAIN Stream Output
	KNL_SEN_AUX_PATH,					//!< AUX Stream Output
	KNL_SEN_SUB_PATH,					//!< SUB Stream Output
}KNL_SEN_PATH;

typedef enum
{	
	KNL_DISP_SINGLE		= 0,			//!< Single
	KNL_DISP_DUAL_C		= 1,			//!< 1 Top,1 Bottom
	KNL_DISP_DUAL_U		= 2,			//!< 1 Left,1 Right
	KNL_DISP_QUAD 		= 3,			//!< Quad
	KNL_DISP_H			= 4,			//!< H Type
	KNL_DISP_3T_1T2B	= 5,			//!< 1 Top,2 Bottom
	KNL_DISP_3T_2T1B  	= 6,			//!< 2 Top,1 Bottom
	KNL_DISP_3T_2L1R  	= 7,			//!< 2 Left,1 Right
	KNL_DISP_3T_1L2R  	= 8,   			//!< 1 Left,2 Right
	KNL_DISP_NONSUP		= 0xFF,
}KNL_DISP_TYPE;

typedef enum
{	
	KNL_DISP_LOCATION1	= 0,			//!< Display @ Location1
	KNL_DISP_LOCATION2	= 1,			//!< Display @ Location2
	KNL_DISP_LOCATION3 	= 2,			//!< Display @ Location3
	KNL_DISP_LOCATION4 	= 3,			//!< Display @ Location4
	KNL_DISP_LOCATION_ERR = 0xFF,		//!< Error
}KNL_DISP_LOCATION;

typedef enum
{	
	KNL_VDO_CODEC_H264 = 0,				//!< H264 Codec
	KNL_VDO_CODEC_JPEG = 1				//!< JPEG Codec
}KNL_VDO_CODEC;

typedef enum
{	
	KNL_ADO_CODEC_PCM	= 0,			//!< PCM Codec
	KNL_ADO_CODEC_MSADPCM,				//!< MSADPCM Codec
	KNL_ADO_CODEC_ALAW,					//!< A-Law Codec
	KNL_ADO_CODEC_AAC,					//!< AAC Codec
	KNL_ADO_CODEC_ADO32,				//!< Audio32 Codec
}KNL_ADO_CODEC;

typedef enum
{	
	KNL_PATH_VDO	= 0,				//!< Video Path
	KNL_PATH_ADO 	= 1,				//!< Audio Path
}KNL_PATH_TYPE;

typedef enum
{	
	KNL_VDO_PKT		= 0,				//!< Video Packet
	KNL_ADO_PKT 	= 1,				//!< Audio Packet
}KNL_PACKET_TYPE;

typedef struct
{
	uint8_t		ubSrcNum;				//!< Source Number
	uint8_t 	ubCurNode;				//!< Current Node	
	uint8_t 	ubNextNode;				//!< Next Node	
	uint32_t	ulDramAddr1;			//!< RAW Data
	uint32_t	ulDramAddr2;			//!< Bit-Stream Data	
	uint32_t	ulSize;					//!< Size
	uint8_t 	ubPath;					//!< Video or Audio Path
	uint8_t 	ubCodecIdx;				//!< Codec Index
	uint32_t 	ulIdx;					//!< Index for Aux-Info
	uint8_t 	ubVdoGop;				//!< Video Group for Aux-Info
	uint32_t 	ulGop;					//!< GOP for Aux-Info
	uint8_t 	ubTargetRole;
	uint8_t 	ubTwcCmd;
}KNL_PROCESS;

typedef struct
{
	uint8_t		ubSrcNum;				//!< Source Number
	uint8_t 	ubCurNode;				//!< Current Node	
	uint8_t 	ubNextNode;				//!< Next Node	
	uint32_t	ulDramAddr1;			//!< RAW Data
	uint32_t	ulDramAddr2;			//!< Bit-Stream Data	
	uint32_t	ulSize;					//!< Size
	uint8_t 	ubPath;					//!< Video or Audio Path
	uint32_t 	ulIdx;					//!< Index for Aux-Info
	uint32_t 	ulGop;					//!< GOP for Aux-Info
}KNL_AVG_PLY_PROCESS;

typedef enum
{		
	KNL_NODE_SEN 			= 0x00,		//!< Sensor
	KNL_NODE_SEN_YUV_BUF	= 0x01,		//!< YUV Buffer for Sensor
	KNL_NODE_H264_ENC		= 0x02,		//!< H264 Encode	
	KNL_NODE_VDO_BS_BUF1	= 0x03,		//!< Bit-Stream Bufffer1
	KNL_NODE_VDO_BS_BUF2	= 0x04,		//!< Bit-Stream Bufffer2	
	KNL_NODE_COMM_TX_VDO	= 0x05,		//!< Communication TX For Video
	KNL_NODE_COMM_RX_VDO	= 0x06,		//!< Communication RX For Video
	KNL_NODE_H264_DEC		= 0x07,		//!< H264 Decode	
	KNL_NODE_LCD			= 0x08,		//!< LCD	
	KNL_NODE_JPG_ENC		= 0x09,		//!< JPEG Encode	
	KNL_NODE_JPG_DEC1		= 0x0A,		//!< JPEG Decode1
	KNL_NODE_JPG_DEC2		= 0x0B,		//!< JPEG Decode2
	KNL_NODE_ADC			= 0x0C,		//!< ADC
	KNL_NODE_DAC			= 0x0D,		//!< DAC
	KNL_NODE_ADC_BUF		= 0x0E,		//!< ADC Buffer
	KNL_NODE_DAC_BUF		= 0x0F,		//!< DAC Buffer
	KNL_NODE_RETRY_ADC_BUF	= 0x10,		//!< Retry for ADC_BUF Process
	KNL_NODE_COMM_TX_ADO	= 0x11,		//!< Communication TX For Audio
	KNL_NODE_COMM_RX_ADO	= 0x12,		//!< Communication RX For Audio	
	KNL_NODE_IMG_MERGE_BUF	= 0x13,		//!< Image Merge Buffer
	KNL_NODE_IMG_MERGE_H	= 0x14,		//!< Horizontal Image Merge	
	KNL_NODE_VDO_REC		= 0x15,		//!< Video Record
	KNL_NODE_ADO_REC		= 0x16,		//!< Audio Record
	KNL_NODE_UVC_MAIN		= 0x17,		//!< UVC Main Video
	KNL_NODE_UVC_SUB		= 0x18,		//!< UVC Sub Video	
	
	KNL_NODE_NONE 			= 0xE0,		//!< None Node
	KNL_NODE_END 			= 0xF0,		//!< End Node		
}KNL_NODE;

typedef enum
{	
	//For Video Path
	KNL_SRC_1_MAIN 	= 0,				//!< Source Number (MAIN1)	
	KNL_SRC_2_MAIN 	= 1,				//!< Source Number (MAIN2)
	KNL_SRC_3_MAIN 	= 2,				//!< Source Number (MAIN3)
	KNL_SRC_4_MAIN 	= 3,				//!< Source Number (MAIN4)	
	KNL_SRC_1_SUB	= 4,				//!< Source Number (SUB1)
	KNL_SRC_2_SUB	= 5,				//!< Source Number (SUB2)
	KNL_SRC_3_SUB	= 6,				//!< Source Number (SUB3)
	KNL_SRC_4_SUB	= 7,				//!< Source Number (SUB4)
	KNL_SRC_1_AUX	= 8,				//!< Source Number (AUX1)
	KNL_SRC_2_AUX	= 9,				//!< Source Number (AUX2)
	KNL_SRC_3_AUX	= 10,				//!< Source Number (AUX3)
	KNL_SRC_4_AUX	= 11,				//!< Source Number (AUX4)
	
	//For Audio Path
	KNL_SRC_1_OTHER_A	= 12,			//!< Source Number (OTHER_A_1)
	KNL_SRC_2_OTHER_A	= 13,			//!< Source Number (OTHER_A_1)
	KNL_SRC_3_OTHER_A	= 14,			//!< Source Number (OTHER_A_1)
	KNL_SRC_4_OTHER_A	= 15,			//!< Source Number (OTHER_A_1)	
	KNL_SRC_1_OTHER_B	= 16,			//!< Source Number (OTHER_B_1)
	KNL_SRC_2_OTHER_B	= 17,			//!< Source Number (OTHER_B_1)
	KNL_SRC_3_OTHER_B	= 18,			//!< Source Number (OTHER_B_1)
	KNL_SRC_4_OTHER_B	= 19,			//!< Source Number (OTHER_B_1)

	KNL_SRC_MASTER_AP   = 0xFE,

	KNL_SRC_NONE		= 0xFF,			//!< Source Number (NONE)
}KNL_SRC;

typedef enum
{
	KNL_STA1 = 0,						//!< Role (STA1)
	KNL_STA2,							//!< Role (STA2)
	KNL_STA3,							//!< Role (STA3)
	KNL_STA4,							//!< Role (STA4)
	KNL_SLAVE_AP,						//!< Role (SLAVE_AP)
	KNL_MASTER_AP,						//!< Role (MASTER_AP)
	KNL_NONE,							//!< Role (NONE)
	KNL_MAX_ROLE,
}KNL_ROLE;

typedef enum
{
	KNL_LOST_LINK = 0,
	KNL_LINK,
}KNL_LINK_STATUS;

enum
{
	KNL_IMG_MERGE_H = 0,	//!< Horizontal Image Merge
};

typedef enum
{
	KNL_SCALE_X1 	= 1,	//!< Without Scaling
	KNL_SCALE_X0P5	= 2,	//!< (1/2) Scaling
	KNL_SCALE_X0P25	= 3		//!< (1/4) Scaling
}KNL_SCALE;

typedef struct
{
	uint8_t 	ubPreNode;			//!< Previous Node
	uint8_t 	ubCurNode;			//!< Current Node
	uint8_t 	ubNextNode;			//!< Next Node
	uint16_t 	uwVdoH;				//!< Horizontal Resolution
	uint16_t 	uwVdoV;				//!< Vertical Resolution
	uint8_t 	ubHMirror;			//!< Horizontal Mirror
	uint8_t 	ubVMirror;			//!< Vertical Mirror
	uint8_t 	ubRotate;			//!< Rotation
	uint8_t 	ubHScale;			//!< Horizontal Scaling
	uint8_t 	ubVScale;			//!< Vertical Scaling
	uint8_t 	ubMergeSrc1;		//!< Source 1 for Image Merge
	uint8_t 	ubMergeSrc2;		//!< Source 2 for Image Merge
	uint8_t 	ubMergeDest;		//!< Destination for Image Merge	
	uint16_t 	uwMergeH;			//!< Horizontal Resolution for Image Merge
	uint16_t 	uwMergeV;			//!< Vertical Resolution for Image Merge
	uint8_t 	ubCodecIdx;			//!< Codec Index
}KNL_NODE_INFO;

typedef enum
{
	KNL_I_FRAME = 0,	//!< H264 I Frame
	KNL_P_FRAME = 1,	//!< H264 P Frame
}KNL_FRAME_TYPE;

typedef enum
{	
	KNL_COMM_STATE_STOP	 = 0,	//!< Communication State -> Stop
	KNL_COMM_STATE_START = 1	//!< Communication State -> Start
}KNL_COMM_STATE;

typedef enum
{
	IMG_MERGE_LCD	= 0,		//!< Source for Image Merge
}KNL_IMG_MERGE_SOURCE;

typedef struct					//Kernel Information
{		
	//System
	uint8_t	ubRole;						//!< Role
	uint8_t ubOpMode;									//!< Operation Mode
	uint8_t	ubAuxInfoFlg;				//!< With or Without Aux Information
	
	//Video
	uint8_t ubVdoCodec;					//!< Video Codec	
	uint16_t uwVdoH[KNL_SRC_NUM];		//!< Video Horizontal Resolution	
	uint16_t uwVdoV[KNL_SRC_NUM];		//!< Video Vertical Resolution	
	uint8_t ubJpegCodecQp;				//!< JPEG Codec QP
	H264_ENCODE_INDEX tEncIdx;			//!< H264 Encoder Index
	uint32_t ulGop;						//!< H264 GOP
	
	//Audio
	ADO_COMPRESS_MODE 	tAdoCodec;		//!< Audio Codec
	ADO_SAMPLERATE 	tAdoSamplingRate;	//!< Audio Sampling Rate	
	uint32_t ulAdcBufSz;				//!< Audio ADC Buffer Size
	uint32_t ulAdcRptSz;				//!< Audio ADC Report Size	
	uint32_t ulDacBufSz;				//!< Audio DAC Buffer Size
	uint32_t ulDacRptSz;				//!< Audio DAC Report Size		
	ADO_ADC_DEV_t	tAdcDevice;			//!< ADC Device(Type)
	ADO_DAC_DEV_t	tDacDevice;			//!< DAC Device(Type)
	
	//Display
	KNL_DISP_TYPE	tDispType;			//!< Display Type
	KNL_DISP_ROTATE tDispRotate;		//!< Display Rotate
	uint8_t ubDisp1SrcNum;				//!< Source Number @Disp1 Location
	uint8_t ubDisp2SrcNum;				//!< Source Number @Disp2 Location
	uint8_t ubDisp3SrcNum;				//!< Source Number @Disp3 Location
	uint8_t ubDisp4SrcNum;				//!< Source Number @Disp4 Location	
	uint16_t uwLcdDmyImgH;				//!< Horizontal Resolution for {H Type,Rotate 0}	
	uint16_t uwDispH;					//!< Display Horizontal Resolution
	uint16_t uwDispV;					//!< Display Vertical Resolution
	
	//Sen
	uint8_t ubSenPath1Src;				//!< Source Number for Sensor path1 output									
	uint8_t ubSenPath2Src;				//!< Source Number for Sensor path2 output
	uint8_t ubSenPath3Src;				//!< Source Number for Sensor path3 output
	
	//Multi-Output
	uint8_t ubMultiOutFlg[0x100L];		//!< Multiple-Output Flag
	uint8_t ubMultiInSrc[0x100L];		//!< Input Source for Multi-Output Node
	uint8_t ubMultiOutSrc1[0x100L];		//!< Output Source 1 for Multi-Output Node
	uint8_t ubMultiOutSrc2[0x100L];		//!< Output Source 2 for Multi-Output Node	
	
	//Play mode
	KNL_PLY_MODE tPlyMode;				//!< Play mode
	uint8_t ubVdoFps;					//!< Video fps
}KNL_INFO;

typedef struct
{
	uint8_t			  ubSetupFlag;
	KNL_SRC			  tSrcNum[4];
	KNL_DISP_LOCATION tSrcLocate[4];
}KNL_SrcLocateMap_t;

typedef enum
{
	KNL_TUNINGMODE_OFF = 0,
	KNL_TUNINGMODE_ON,
}KNL_TuningMode_t;

typedef KNL_SRC (*pvRoleSrcMap)(KNL_ROLE tRole);

//------------------------------------------------------------------------------
/*!
\brief 	Get KNL Function Version	
\return	Unsigned short value, high byte is the major version and low byte is the minor version
\par [Example]
\code		 
	 uint16_t uwVer;
	 
	 uwVer = uwKNL_GetVersion();
	 printf("KNL Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
\endcode
*/
uint16_t uwKNL_GetVersion (void);

uint8_t ubKNL_GetQp(uint8_t ubCodecIdx);

//------------------------------------------------------------------------
/*!
\brief Get BB IP buffer size
\return Buffer size
*/
uint32_t ulKNL_GetBbIpBufSz(void);

//------------------------------------------------------------------------
/*!
\brief Set Audio Information
\param tAdoInfo Audio Information
\return (no)
*/
void KNL_SetAdoInfo(ADO_KNL_PARA_t tAdoInfo);	

//------------------------------------------------------------------------
/*!
\brief Initial Kernel
\return(no)
*/
void KNL_Init(void);

//------------------------------------------------------------------------
/*!
\brief Set Play Mode
\param tPlyMode 0->Normal Play,1->Smooth Play
\return (no)
*/
void KNL_SetPlyMode(KNL_PLY_MODE tPlyMode);

//------------------------------------------------------------------------
/*!
\brief Set Video FPS
\param ubFps Video FPS
\return (no)
*/
void KNL_SetVdoFps(uint8_t ubFps);

//------------------------------------------------------------------------
/*!
\brief Get Play Mode
\return Play Mode 0->Normal Play,1->Smooth Play
*/
KNL_PLY_MODE tKNL_GetPlyMode(void);

//------------------------------------------------------------------------
/*!
\brief Get Video FPS
\return Video FPS
*/
uint8_t ubKNL_GetVdoFps(void);

//------------------------------------------------------------------------
/*!
\brief Set H264 GOP
\param ulGop H264 GOP
\return (no)
*/
void KNL_SetVdoGop(uint32_t ulGop);

//------------------------------------------------------------------------
/*!
\brief Get H264 GOP
\return GOP
*/
uint32_t ulKNL_GetVdoGop(void);

//------------------------------------------------------------------------
/*!
\brief Get H264 Frame Index
\return Frame Index
*/
uint32_t ulKNL_GetVdoFrmIdx(uint8_t ubCh);

//------------------------------------------------------------------------
/*!
\brief Get Data Bit-Rate
\param ubDataType Data type
\return Bit-Rate
*/
uint32_t ulKNL_GetDataBitRate(uint8_t ubDataType,uint8_t ubCodecIdx);

//------------------------------------------------------------------------
/*!
\brief Get FPS
\param tFpsType	FPS Type
\param ubSrcNum	Source number
\return FPS
*/
uint32_t ulKNL_GetFps(KNL_FPS_TYPE tFpsType,uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Initial kernel buffer
\return(no)
*/
void KNL_BufInit(void);

//------------------------------------------------------------------------
/*!
\brief Initial kernel block
\return(no)
*/
void KNL_BlockInit(void);

//------------------------------------------------------------------------
/*!
\brief Set kernel role
\param ubRole	Role
\return(no)
*/
void KNL_SetRole(uint8_t ubRole);

//------------------------------------------------------------------------
/*!
\brief Get kernel role
\return Kernel role
*/
uint8_t ubKNL_GetRole(void);


//------------------------------------------------------------------------
/*!
\brief Set operation mode
\param ubOpMode	Operation mode
\return(no)
*/
void KNL_SetOpMode(uint8_t ubOpMode);

//------------------------------------------------------------------------
/*!
\brief Get opration mode
\return Operation mode
*/
uint8_t ubKNL_GetOpMode(void);

//------------------------------------------------------------------------
/*!
\brief Set auxiliary information function
\param ubEnable	0->Disable auxiliary information function,1->Enable auxiliary information function
\return(no)
*/
void KNL_SetAuxInfoFunc(uint8_t ubEnable);

//------------------------------------------------------------------------
/*!
\brief Get auxiliary informaton function status
\return Status : 0->Disable,1->Enable
*/
uint8_t ubKNL_GetAuxInfoFunc(void);

//------------------------------------------------------------------------
/*!
\brief Add auxiliary information
\param tPktType	Packet type
\param ubSrcNum	Source number
\param ulAddr 	Target address
\param ulSize 	Target size
\param ulFrmIdx	Frame index
\param ulGop	Video codec GOP
\param ubVdoGroupIdx Video group
\return 0->Fail\n
		1->Pass
*/
uint32_t ulKNL_AddAuxInfo(KNL_PACKET_TYPE tPktType,uint8_t ubSrcNum,uint32_t ulAddr,uint32_t ulSize,uint32_t ulFrmIdx,uint32_t ulGop,uint8_t ubVdoGroupIdx);

//------------------------------------------------------------------------
/*!
\brief Set video codec
\param ubVdoCodec	Video codec : 0->H264 Codec,1->JPEG Codec
\return(no)
*/
void KNL_SetVdoCodec(uint8_t ubVdoCodec);

//------------------------------------------------------------------------
/*!
\brief Get video codec
\return Video codec : 0->H264 Codec,1->JPEG Codec
*/
uint8_t ubKNL_GetVdoCodec(void);

//------------------------------------------------------------------------
/*!
\brief Set video horizontal resolution
\param ubSrcNum Source number
\param uwVdoH video horizontal resolution
\return (no)
*/
void KNL_SetVdoH(uint8_t ubSrcNum,uint16_t uwVdoH);

//------------------------------------------------------------------------
/*!
\brief Get video horizontal resolution
\param ubSrcNum Source number
\return Video horizontal resolution
*/
uint16_t uwKNL_GetVdoH(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set video vertical resolution
\param ubSrcNum Source number
\param uwVdoV video vertical resolution
\return (no)
*/
void KNL_SetVdoV(uint8_t ubSrcNum,uint16_t uwVdoV);

//------------------------------------------------------------------------
/*!
\brief Get video vertical resolution
\param ubSrcNum Source number
\return Video vertical resolution
*/
uint16_t uwKNL_GetVdoV(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Reset video flow
\return(no)
*/
void KNL_VdoReset(void);

//------------------------------------------------------------------------
/*!
\brief Suspend video path
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoSuspend(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Resume video path
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoResume(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Start video flow
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Stop video flow
\param ubSrcNum Source number
\return(no)
*/
void KNL_VdoStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Check video flow activation
\param ubSrcNum Source number
\return 0->Without active\n
		1->Active
*/
uint8_t ubKNL_ChkVdoFlowAct(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Check IMG block ready or not
\return 0->Not ready\n
		1->Ready
*/
uint8_t ubKNL_ChkImgRdy(void);

//------------------------------------------------------------------------
/*!
\brief Get frame type
\param ulAddr Target address
\return 0->I frame\n
		1->P frame
*/
KNL_FRAME_TYPE tKNL_GetFrameType(uint32_t ulAddr);

//------------------------------------------------------------------------
/*!
\brief Set JPEG QP
\param ubQp : 0~255
\return(no)
*/
void KNL_SetJpegQp(uint8_t ubQp);

//------------------------------------------------------------------------
/*!
\brief Get JPEG QP
\return JPEG QP
*/
uint8_t ubKNL_GetJpegQp(void);

//------------------------------------------------------------------------
/*!
\brief Get communication link status
\param ubRole :
\return 0->Un-Link\n
		1->Link
*/
uint8_t ubKNL_GetCommLinkStatus(uint8_t ubRole);

uint8_t ubKNL_GetRtCommLinkStatus(uint8_t ubRole);
//------------------------------------------------------------------------
/*!
\brief Hook function for TWC
\param GetSta Target role : 0->TWC_STA1,1->TWC_STA2,2->TWC_STA3,3->TWC_STA4,4->TWC_AP_SLAVE,5->TWC_AP_MASTER
\param ubStatus Status : 0->TWC_SUCCESS,1->TWC_FAIL,2->TWC_BUSY
\return(no)
*/
void KNL_TwcResult(TWC_TAG GetSta,TWC_STATUS ubStatus);

//------------------------------------------------------------------------
/*!
\brief Send two way command
\param ubRole Destination device : 0->KNL_STA1,1->KNL_STA2,2->KNL_STA3,3->KNL_STA4,4->KNL_AP_SLAVE,5->KNL_AP_MASTER
\param Opc Operation command
\param *Data Pointer for TWC data
\param ubLen Length for TWC data
\param ubRetry Retry times for TWC
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_TwcSend(uint8_t ubRole,TWC_OPC Opc,uint8_t *Data,uint8_t ubLen,uint8_t ubRetry);

//------------------------------------------------------------------------
/*!
\brief Reset node state
\return(no)
*/
void KNL_NodeStateReset(void);

//------------------------------------------------------------------------
/*!
\brief Set node state
\param ubSrcNum Source number
\param ubNode Node : 0~255
\param ubState State : 0->KNL_NODE_START,1->KNL_NODE_TRANS,0xFF->KNL_NODE_STOP
\return(no)
*/
void KNL_SetNodeState(uint8_t ubSrcNum,uint8_t ubNode,uint8_t ubState);

//------------------------------------------------------------------------
/*!
\brief Check node is finish or not
\param ubSrcNum Source number
\return 0->Not finish\n
		1->Finish
*/
uint8_t ubKNL_ChkNodeFinish(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Wait node to finish state
\param ubSrcNum Source number
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_WaitNodeFinish(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set Multiple output node
\param ubNode Node : 0~255
\param ubEnable 0->Disable,1->Enable
\param ubInSrc Input source
\param ubOutSrc1 Output source1
\param ubOutSrc2 Output Source2
\return(no)
*/
void KNL_SetMultiOutNode(uint8_t ubNode,uint8_t ubEnable,uint8_t ubInSrc,uint8_t ubOutSrc1,uint8_t ubOutSrc2);

//------------------------------------------------------------------------
/*!
\brief Check node is multiple output node or not
\param ubNode Node : 0~255
\return 0->Not multiple output node\n
		1->Multiple output node
*/
uint8_t ubKNL_ChkMultiOutNode(uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get output source
\param ubNode Node : 0~255
\param ubOutSrc Output Source : 0/1
\return Output source
*/
uint8_t ubKNL_GetMultiOutSrc(uint8_t ubNode,uint8_t ubOutSrc);

//------------------------------------------------------------------------
/*!
\brief Get input source
\param ubNode Node : 0~255
\return Input source
*/
uint8_t ubKNL_GetMultiInSrc(uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get node index
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Index : 0~255
*/
uint8_t ubKNL_GetNodeIdx(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get node information
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Node information
*/
KNL_NODE_INFO tKNL_GetNodeInfo(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get next node
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Next node : 0~255
*/
uint8_t ubKNL_GetNextNode(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Get previous node
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return Previous node : 0~255
*/
uint8_t ubKNL_GetPreNode(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Check node is exist or not
\param ubSrcNum Source number
\param ubNode Node : 0~255
\return 0->Not exist\n
		1->Exist
*/
uint8_t ubKNL_ExistNode(uint8_t ubSrcNum,uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Check node is exist or not
\param ubNode Node : 0~255
\return 0->Not exist\n
		1->Exist
*/
uint8_t ubKNL_ChkExistNode(uint8_t ubNode);

//------------------------------------------------------------------------
/*!
\brief Reset video path
\return(no)
*/
void KNL_VdoPathReset(void);

//------------------------------------------------------------------------
/*!
\brief Set video path
\param ubSrcNum Source number
\param ubNodeIndex Node index : 0~255
\param tNodeInfo Node information
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_SetVdoPathNode(uint8_t ubSrcNum,uint8_t ubNodeIndex,KNL_NODE_INFO tNodeInfo);

//------------------------------------------------------------------------
/*!
\brief Reset node information of vidio path.
\param ubSrcNum Source number
\return (no)
*/
void KNL_VdoPathNodeReset(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Show video path
\param ubSrcNum Source number
\return (no)
*/
void KNL_ShowVdoPathNode(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get bit-stream buffer address
\param ubSrcNum Source number
\return Address
*/
uint32_t ulKNL_GetBsBufAddr(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Release bit-stream buffer
\param ubCurNode Current node
\param ubSrcNum Source number
\param ulBufAddr Buffer address
\return 0xA5(Fail)\n
			1(Success)
*/
uint8_t ubKNL_ReleaseBsBufAddr(uint8_t ubCurNode,uint8_t ubSrcNum,uint32_t ulBufAddr);

//------------------------------------------------------------------------
/*!
\brief Get LCD buffer size
\return LCD buffer size
*/
uint32_t ulKNL_CalLcdBufSz(void);

//------------------------------------------------------------------------
/*!
\brief Set Cropping and Scale Parameter
\return 0(Fail)\n
			1(Success)
*/
uint8_t ubKNL_SetDispCropScaleParam(void);

//------------------------------------------------------------------------
/*!
\brief Check Lcd Display location status
\return 0(Fail)\n
		1(Ok)
*/
uint8_t KNL_ChkLcdDispLocation(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief LCD Display Setting
\return no
*/
void KNL_LcdDisplaySetting(void);

//------------------------------------------------------------------------
/*!
\brief Reset LCD Channel
\return (no)
*/
void KNL_ResetLcdChannel(void);

//------------------------------------------------------------------------
/*!
\brief Get LCD display address
\param ubSrcNum Source number
\return LCD display address
*/
uint32_t ulKNL_GetLcdDispAddr(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Acitve LCD display buffer
\param ubSrcNum Source number
\return (no)
*/
void KNL_ActiveLcdDispBuf(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set LCD dummy image horizontal resolution
\param uwH Horizontal resolution
\return (no)
*/
void KNL_SetLcdDmyImgH(uint16_t uwH);

//------------------------------------------------------------------------
/*!
\brief Get LCD dummy image horizontal resolution
\return Dummy image horizontal resolution
*/
uint16_t uwKNL_GetLcdDmyImgH(void);

//------------------------------------------------------------------------
/*!
\brief Set display horizontal/vertical resolution
\param uwDispH Display horizontal resolution
\param uwDispV Display vertical resolution
\return (no)
*/
void KNL_SetDispHV(uint16_t uwDispH,uint16_t uwDispV);

//------------------------------------------------------------------------
/*!
\brief Set display type
\param tDispType Display type : 
\return (no)
*/
void KNL_SetDispType(KNL_DISP_TYPE tDispType);

//------------------------------------------------------------------------
/*!
\brief Get display type
\return Display type
*/
KNL_DISP_TYPE tKNL_GetDispType(void);

//------------------------------------------------------------------------
/*!
\brief Set display source
\param ubDispLocation Display location
\param ubDispSrcNum Display source number
\return (no)
*/
void KNL_SetDispSrc(KNL_DISP_LOCATION tDispLocation,uint8_t ubDispSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get display source
\param tDispLocation Display location
\return Display source
*/
uint8_t ubKNL_GetDispSrc(KNL_DISP_LOCATION tDispLocation);

//------------------------------------------------------------------------
/*!
\brief Get display location
\param ubSrcNum Display source
\return Display location : 0->KNL_DISP_LOCATION1,1->KNL_DISP_LOCATION2,2->KNL_DISP_LOCATION3,3->KNL_DISP_LOCATION4
*/
KNL_DISP_LOCATION tKNL_GetDispLocation(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Modify display type
\param tDispType 			Display type
\param ubDispSrcNum  		Display source number, use for display is single.
\param KNL_SrcLocateMap_t  	Display source number and location, use for display is single and dual(if config 4T/1R mode).
\code
	1. KNL_SwDispInfo.tSrcNum[0] = KNL_SRC_1_MAIN;
	   KNL_ModifyDispType(KNL_DISP_SINGLE, KNL_SwDispInfo);
	2. KNL_ModifyDispType(KNL_DISP_QUAD, KNL_SwDispInfo);
	3. If config 4T/1R mode.
	   KNL_SwDispInfo.ubSetupFlag   = TRUE;
	   KNL_SwDispInfo.tSrcNum[0] 	= KNL_SRC_1_MAIN;
	   KNL_SwDispInfo.tSrcNum[1] 	= KNL_SRC_2_MAIN;
	   KNL_SwDispInfo.tSrcLocate[0] = KNL_DISP_LOCATION1;
	   KNL_SwDispInfo.tSrcLocate[1] = KNL_DISP_LOCATION2;
	   KNL_ModifyDispType(KNL_DISP_DUAL_C, KNL_SwDispInfo);
	4. If config 2T/1R mode
	   KNL_SwDispInfo.ubSetupFlag = FALSE;
	   KNL_ModifyDispType(KNL_DISP_DUAL_C, KNL_SwDispInfo);
\endcode
\return (no)
*/
void KNL_ModifyDispType(KNL_DISP_TYPE tDispType, KNL_SrcLocateMap_t tSrcLocate);

//------------------------------------------------------------------------
/*!
\brief Set display rotate
\param tRotateType Rotate type
\return (no)
*/
void KNL_SetDispRotate(KNL_DISP_ROTATE tRotateType);

//------------------------------------------------------------------------
/*!
\brief Get display rotate
\return Display rotate
*/
KNL_DISP_ROTATE tKNL_GetDispRotate(void);

//------------------------------------------------------------------------
/*!
\brief Sensor yuv buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_SenYuvBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief H264 Encode process task
\param tProc Information for process
\return (no)
*/
void KNL_H264EncProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief H264 Decode process task
\param tProc Information for process
\return (no)
*/
void KNL_H264DecProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief JPEG Decode1 process task
\param tProc Information for process
\return (no)
*/
void KNL_JpegDec1Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief JPEG Decode2 process task
\param tProc Information for process
\return (no)
*/
void KNL_JpegDec2Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief JPEG Encode process task
\param tProc Information for process
\return (no)
*/
void KNL_JpegEncProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Video bit-stream buffer1 process task
\param tProc Information for process
\return (no)
*/
void KNL_VdoBsBuf1Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Video bit-stream buffer2 process task
\param tProc Information for process
\return (no)
*/
void KNL_VdoBsBuf2Process(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Image merge buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_ImgMergeBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Image horizontal merge process task
\param tProc Information for process
\return (no)
*/
void KNL_ImgMergeHProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Video transmit process task
\param tProc Information for process
\return (no)
*/
void KNL_BbTxVdoProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief ADC buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_AdcBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief ADC buffer retry process task
\param tProc Information for process
\return (no)
*/
void KNL_RetryAdcBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief DAC buffer process task
\param tProc Information for process
\return (no)
*/
void KNL_DacBufProcess(KNL_PROCESS tProc);

//------------------------------------------------------------------------
/*!
\brief Sensor start
\param ubSrcNum Source number
\return (no)
*/
void KNL_SenStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Sensor stop
\param ubSrcNum Source number
\return (no)
*/
void KNL_SenStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Image stability
\return (no)
*/
void KNL_ImgStabNotifyFunc(void);
//------------------------------------------------------------------------
/*!
\brief Check sensor state change is done or not
\return 0->not yet\n
			1->done
*/
uint8_t ubKNL_ChkSenStateChangeDone(void);

//------------------------------------------------------------------------
/*!
\brief Image encode initial
\param CodecIdx encode codec index
\param uwVdoH Video horizontal resolution
\param uwVdoV Video vertical resolution
\return (no)
*/
void KNL_ImgEncInit(H264_ENCODE_INDEX CodecIdx,uint16_t uwVdoH,uint16_t uwVdoV);

//------------------------------------------------------------------------
/*!
\brief Image decode initial
\param CodecIdx decode codec index
\param uwVdoH Video horizontal resolution
\param uwVdoV Video vertical resolution
\return (no)
*/
void KNL_ImgDecInit(H264_DECODE_INDEX CodecIdx,uint16_t uwVdoH,uint16_t uwVdoV);

//------------------------------------------------------------------------
/*!
\brief Image decoder setup
\param ubSrcNum Source number
\return (no)
*/
void KNL_ImageDecodeSetup(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Image encoder setup
\param ubSrcNum Source number
\return (no)
*/
void KNL_ImageEncodeSetup(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Image encode
\param CodecIdx encode codec index
\param ulYuvAddr YUV buffer address
\param ulBsAddr Bit-stream buffer address
\return 0->Fail\n
			1->Success
*/
uint8_t ubKNL_ImgEnc(H264_ENCODE_INDEX CodecIdx,uint32_t ulYuvAddr,uint32_t ulBsAddr);

//------------------------------------------------------------------------
/*!
\brief Image decode
\param CodecIdx decode codec index
\param ulYuvAddr YUV buffer address
\param ulBsAddr Bit-stream buffer address
\return 0->Fail\n
			1->Success
*/
uint8_t ubKNL_ImgDec(H264_DECODE_INDEX CodecIdx,uint32_t ulYuvAddr,uint32_t ulBsAddr);

//------------------------------------------------------------------------
/*!
\brief Get image merge buffer address
\param ubSrcNum Source number
\return Buffer address
*/
uint32_t ulKNL_GetImgMergeBufAddr(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Get image merge buffer size
\return Buffer size
*/
uint32_t ulKNL_GetImgMergeBufSz(void);

//------------------------------------------------------------------------
/*!
\brief Image Monitor function
\param ReceiveResult H264 Codec Report
\return(no)
*/
void KNL_ImgMonitorFunc(struct IMG_RESULT ReceiveResult);

//------------------------------------------------------------------------
/*!
\brief Check audio flow activation
\param ubSrcNum Source number
\return 0->Without active\n
		1->Active
*/
uint8_t ubKNL_ChkAdoFlowAct(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido start
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido stop
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido resume
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoResume(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido suspend
\param ubSrcNum Source number
\return (no)
*/
void KNL_AdoSuspend(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Reset audio path
\return(no)
*/
void KNL_AdoPathReset(void);

//------------------------------------------------------------------------
/*!
\brief Alignment audio packet size
\param ulInputSz Input size
\return Output size
*/
uint32_t ulKNL_AlignAdoPktSz(uint32_t ulInputSz);

//------------------------------------------------------------------------
/*!
\brief Auido ADC start
\return (no)
*/
void KNL_AdcStart(void);

//------------------------------------------------------------------------
/*!
\brief Auido ADC stop
\return (no)
*/
void KNL_AdcStop(void);

//------------------------------------------------------------------------
/*!
\brief Auido DAC start
\param ubSrcNum Source number
\return (no)
*/
void KNL_DacStart(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Auido DAC stop
\param ubSrcNum Source number
\return (no)
*/
void KNL_DacStop(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Set audio path
\param ubSrcNum Source number
\param ubNodeIndex Node index : 0~255
\param tNodeInfo Node information
\return 0->Fail\n
		1->Success
*/
uint8_t ubKNL_SetAdoPathNode(uint8_t ubSrcNum,uint8_t ubNodeIndex,KNL_NODE_INFO tNodeInfo);
//------------------------------------------------------------------------
/*!
\brief Show audio path
\param ubSrcNum Source number
\return (no)
*/
void KNL_ShowAdoPathNode(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief Check DAC flow activation
\param ubSrcNum Source number
\return 0->Without active\n
		1->Active
*/
uint8_t ubKNL_ChkDacFlowAct(uint8_t ubSrcNum);

//------------------------------------------------------------------------
/*!
\brief DAC stop case
\return (no)
*/
void KNL_DacStopCase(void);
//------------------------------------------------------------------------
/*!
\brief JPEG Encode process
\param uwH 			Set image size of horizontal.
\param uwV 			Set image size of vertical.
\param ulVdoAddr 	Start address of Image.
\param ulJpgAddr 	Start address of JPEG.
\return(no)
*/
uint8_t ubKNL_JPEGEncode(uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr);
//------------------------------------------------------------------------
/*!
\brief JPEG Decode process
\param pKNL_NodeInfo	Image information
\param uwH 				Set image size of horizontal.
\param uwV 				Set image size of vertical.
\param ulVdoAddr 		Start address of Image.
\param ulJpgAddr 		Start address of JPEG.
\return(no)
*/
uint8_t ubKNL_JPEGDecode(KNL_NODE_INFO *pKNL_NodeInfo, uint16_t uwH, uint16_t uwV, uint32_t ulVdoAddr, uint32_t ulJpgAddr);

//------------------------------------------------------------------------
/*!
\brief Source number mapping to role number for video
\param pvRoleSrcMap		Mapping callback function
\return(no)
*/
void KNL_SetVdoRoleInfoCbFunc(pvRoleSrcMap VdoRoleMap_cb);

//------------------------------------------------------------------------
/*!
\brief Source number mapping to role number for audio
\param pvRoleSrcMap		Mapping callback function
\return(no)
*/
void KNL_SetAdoRoleInfoCbFunc(pvRoleSrcMap AdoRoleMap_cb);

//------------------------------------------------------------------------
/*!
\brief Get RSSI value
\param tKNL_Role		Kernel Role Number
\return RSSI Value
*/
uint8_t KNL_GetRssiValue(KNL_ROLE tKNL_Role);
//------------------------------------------------------------------------
/*!
\brief Get PER value
\param tKNL_Role		Kernel Role Number
\return (100 - PER) Value
*/
uint8_t KNL_GetPerValue(KNL_ROLE tKNL_Role);
//------------------------------------------------------------------------
/*!
\brief Get H264 Encoder Index
\return Encoder Index
*/
H264_ENCODE_INDEX tKNL_GetEncIdx(void);
	
//------------------------------------------------------------------------
/*!
\brief Enable WOR function
\return no
*/
void KNL_EnableWORFunc(void);
//------------------------------------------------------------------------
/*!
\brief Disable WOR function
\return no
*/
void KNL_DisableWORFunc(void);
//------------------------------------------------------------------------
/*!
\brief Wake-up device on WOR mode
\param tKNL_Role		Kernel Role Number
\param ubMode			Start/Stop wake-up function, 1:start, 0:stop
\return no
*/
uint8_t KNL_WakeupDevice(KNL_ROLE tKNL_Role, uint8_t ubMode);
//------------------------------------------------------------------------
/*!
\brief Turn on tuning tool
\return no
*/
void KNL_TurnOnTuningTool(void);
//------------------------------------------------------------------------
/*!
\brief Turn off tuning tool
\return no
*/
void KNL_TurnOffTuningTool(void);
//------------------------------------------------------------------------
/*!
\brief Get mode of tuning tool
\return Tuning tool mode
*/
KNL_TuningMode_t KNL_GetTuningToolMode(void);
//------------------------------------------------------------------------
/*!
\brief Firmware upgrade function use SD card
\return no
*/
void KNL_SDUpgradeFwFunc(void);

//Extern
extern osMessageQId KNL_ProcessQueue;
extern osMessageQId tKNL_EncEventQue;		
extern osMessageQId tKNL_DecEventQue;

//------------------------------------------------------------------------
uint32_t APP_TIMER_Get1ms(void);
uint32_t ulAPP_ADO_GetMemInitValueAdr(void);
uint32_t ulAPP_ADO_GetWifiPacketSize(void);
void APP_RecordOnceEnd_SDK(void);
void APP_1MSTrigger(void);
void APP_1MSCounter(void);
void KNL_TestRecInit(void);

typedef void(*pvKNL_BbFrmOkCbFunc)(uint8_t ubBbFrmStatus);
void KNL_SetBbFrmMonitCbFunc(pvKNL_BbFrmOkCbFunc BbFrmOkCbFunc);

//------------------------------------------------------------------------
#endif
