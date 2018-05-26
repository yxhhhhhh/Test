/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		BB_API.h
	\brief		Baseband API header
	\author		Bing
	\version	1.9
	\date		2018/05/03
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef __ST53510_BB_API_H
#define __ST53510_BB_API_H
#include "_510PF.h"

#define BB_1R_MODE					0
#define BB_2R_MODE					1

#define BB_RF_DETECT_GPIO6_USE		0x6
#define BB_RF_DETECT_GPIO8_USE		0x8
#if OP_AP		
//#define BB_RF_DETECT				BB_RF_DETECT_GPIO6_USE	
#define BB_RF_DETECT				BB_RF_DETECT_GPIO8_USE
#endif
#if OP_STA		
#define BB_RF_DETECT				BB_RF_DETECT_GPIO8_USE	
#endif


typedef enum
{
	SET_STA1 = 0,
	SET_STA2 = 1,
	SET_STA3 = 2,
	SET_STA4 = 3,
	SET_SLAVE_AP = 4,
	SET_MASTER_AP = 0xF,
}EN_SET_ROLE;
typedef enum
{
	EN_IDLE = 0,
	EN_TX_POWER,
	EN_TX_0101,
	EN_TX_PRBS,
	EN_RX_ONLY,
	EN_FIX_HOPPING,
	EN_AFH_HOPPING,
}EN_STATUS;
typedef enum
{
	SET_SUCCESS = 0,
	SET_FAIL,
}EN_RETURN_STATUS;
typedef struct 
{
	uint8_t ubEnalbe;
	uint8_t ubPoint;
	uint8_t ubUsFlg;
	uint8_t ubNowPoint;
	uint8_t ubLayer1;
	uint8_t ubLayer2;
	uint8_t ubLayer3;
	EN_STATUS tStatus;
	EN_SET_ROLE tRole;
	uint8_t ubData1;
	uint8_t ubData2;
}EN_MODE_UI;

typedef enum
{
	BB_TX_ADO_STA1 = 0,
	BB_TX_ADO_STA2,
	BB_TX_ADO_STA3,
	BB_TX_ADO_STA4,
	BB_TX_ADO_SLAVE_AP,
	BB_TX_ADO_NONE,
	BB_TX_ADO_MASTER_AP = 0xF,
}TXADO;

typedef enum
{
	BB_RX_ADO_STA1 = 0,
	BB_RX_ADO_STA2,
	BB_RX_ADO_STA3,
	BB_RX_ADO_STA4,
	BB_RX_ADO_SLAVE_AP,
	BB_RX_ADO_ALL_STA,
	BB_RX_ADO_NONE,
	BB_RX_ADO_MASTER_AP = 0xF,
}RXADO;

typedef enum
{
	BB_PAYLOAD_NONE = 0,
	BB_OPEN_STA1_PAYLOAD = 1,
	BB_OPEN_STA2_PAYLOAD = 2,
	BB_OPEN_STA3_PAYLOAD = 4,
	BB_OPEN_STA4_PAYLOAD = 8,
	BB_OPEN_ALL_PAYLOAD  = 0xF,
}PAYLOAD_PATH;

typedef enum
{
	BB_ADO = 0,
	BB_VDO = 1,
}SET_RETRY_TYP;

typedef enum
{
	BB_TX_MASTER = 0,
	BB_TX_SLAVE  = 1,
}SET_TX_PATH;

typedef enum
{
	BB_SLOT_1 = 1,
	BB_SLOT_2 = 2,
	BB_SLOT_4 = 4,
}SET_SLOT_MODE;

typedef enum
{
	BB_TIME_0 = 0,
	BB_TIME_1,
	BB_TIME_2,
	BB_TIME_3,
	BB_TIME_4,
	BB_TIME_5,
	BB_TIME_6,
	BB_TIME_7,
	BB_TIME_8,
	BB_TIME_9,
	BB_TIME_10,
	BB_TIME_11,
	BB_TIME_12,
	BB_TIME_13,
	BB_TIME_14,
	BB_TIME_15,
	BB_TIME_16,
	BB_TIME_17,
	BB_TIME_18,
	BB_TIME_19,
	BB_TIME_20,
	BB_TIME_21,
	BB_TIME_22,
	BB_TIME_23,
	BB_TIME_24,
	BB_TIME_25,
	BB_TIME_26,
	BB_TIME_27,
	BB_TIME_28,
	BB_TIME_29,
	BB_TIME_30,
	BB_TIME_31,
	BB_TIME_32,
	BB_TIME_STOP 	= 0xFE,
	BB_TIME_ALWAYS 	= 0xFF,
}RETRY_TIME;

typedef enum
{
	BB_DATA_VIDEO = 1,
	BB_DATA_AUDIO = 2
}SET_TYPE;

typedef enum
{
	BB_SET_BUF_SUCCESS ,
	BB_SET_BUF_BUSY	,
	BB_SET_BUF_FAIL	,
}SET_BUF;

typedef enum
{
	BB_NOT_RECEIVED_ACK ,
	BB_RECEIVE_ACK ,
}GET_STATUS;

typedef enum
{
	BB_TX_MASTER_VDO_SUCCESS,
	BB_TX_SLAVE_VDO_SUCCESS,
	BB_TX_MASTER_VDO_FAIL,
	BB_TX_SLAVE_VDO_FAIL,
	BB_TX_ADO_SUCCESS,
	BB_TX_ADO_FAIL,
}TX_END_STATUS;

typedef enum
{
	BB_SEND_SUCCESS		 	,
	BB_SEND_FAIL			,
	BB_SEND_RETRY_TIMEOUT	,
}SEND_INF;

typedef enum
{
	BB_GET_STA1 = 0,
	BB_GET_STA2 = 1,
	BB_GET_STA3 = 2,
	BB_GET_STA4 = 3,
	BB_GET_AP   = 4,
}GET_STA;

typedef enum
{
	BB_STA1 = 0,
	BB_STA2 = 1,
	BB_STA3 = 2,
	BB_STA4 = 3,
	BB_SLAVE_AP = 4,
	BB_MASTER_AP = 0xF,
}LINK_ROLE;

typedef enum
{
	BB_GET_STA1_PER 	= 0,
	BB_GET_STA2_PER 	= 1,
	BB_GET_STA3_PER 	= 2,
	BB_GET_STA4_PER 	= 3,
	BB_GET_SLAVE_AP_PER = 4,
	BB_GET_MASTER_AP_PER= 5,
}GET_PER_ROLE;

typedef enum
{
	BB_GET_STA1_RSSI 	= 0,
	BB_GET_STA2_RSSI	= 1,
	BB_GET_STA3_RSSI 	= 2,
	BB_GET_STA4_RSSI 	= 3,
	BB_GET_SLAVE_AP_RSSI = 4,
	BB_GET_MASTER_AP_RSSI= 5,
}GET_RSSI_ROLE;

typedef enum
{
	BB_LOST_LINK = 0,
	BB_LINK,
}LINK_STATUS;


typedef enum
{
	BB_USE_START = 0,
	BB_USE_END,
}ADDR_STATUS;

typedef enum
{
	BB_HEAD_PER = 0,
	BB_VDO_PER,
	BB_ADO_PER,
}PER_TYPE;

typedef enum
{
	BB_DEBUG_NONE	= 0,
	BB_DEBUG_HOP	= 1,
	BB_DEBUG_HECOK	= 2,
	BB_DEBUG_ADOOK	= 4,
	BB_DEBUG_VDOOK	= 8,
}BB_DEBUG_MD;

enum
{
	BB_TASK_IDLE = 0,
	BB_TASK_START,
	BB_TASK_STOP,
};

typedef enum 
{
#if OP_AP		
	BB_GET_RXSTA1_VDO_FLOW = 0,
	BB_GET_RXSTA2_VDO_FLOW,
	BB_GET_RXSTA3_VDO_FLOW,
	BB_GET_RXSTA4_VDO_FLOW,
	
	BB_GET_RXSTA1_ADO_FLOW, 
	BB_GET_RXSTA2_ADO_FLOW,
	BB_GET_RXSTA3_ADO_FLOW,
	BB_GET_RXSTA4_ADO_FLOW,
	BB_GET_RXAP_ADO_FLOW,
	
	BB_GET_TX_ADO_FLOW,
	
	BB_GET_DEF_1T1R_AP_ADO_FLOW,
	BB_GET_DEF_1T1R_STA1_ADO_FLOW,
	BB_GET_DEF_1T1R_STA1_VDO_FLOW,
	
	BB_GET_DEF_2T1R_AP_ADO_FLOW,
	BB_GET_DEF_2T1R_STA1_ADO_FLOW,
	BB_GET_DEF_2T1R_STA2_ADO_FLOW,
	BB_GET_DEF_2T1R_STA1_VDO_FLOW,
	BB_GET_DEF_2T1R_STA2_VDO_FLOW,
	
	BB_GET_DEF_3T1R_AP_ADO_FLOW,
	BB_GET_DEF_3T1R_STA1_ADO_FLOW,
	BB_GET_DEF_3T1R_STA2_ADO_FLOW,
	BB_GET_DEF_3T1R_STA3_ADO_FLOW,
	BB_GET_DEF_3T1R_STA1_VDO_FLOW,
	BB_GET_DEF_3T1R_STA2_VDO_FLOW,
	BB_GET_DEF_3T1R_STA3_VDO_FLOW,
	
	BB_GET_DEF_4T1R_AP_ADO_FLOW,
	BB_GET_DEF_4T1R_STA1_ADO_FLOW,
	BB_GET_DEF_4T1R_STA2_ADO_FLOW,
	BB_GET_DEF_4T1R_STA3_ADO_FLOW,
	BB_GET_DEF_4T1R_STA4_ADO_FLOW,
	BB_GET_DEF_4T1R_STA1_VDO_FLOW,
	BB_GET_DEF_4T1R_STA2_VDO_FLOW,
	BB_GET_DEF_4T1R_STA3_VDO_FLOW,
	BB_GET_DEF_4T1R_STA4_VDO_FLOW,
#endif	
#if OP_STA		
	BB_GET_RXMAP_ADO_FLOW = 0,
	BB_GET_RXSAP_ADO_FLOW,
	
	BB_GET_TXMAP_VOD_FLOW,
	BB_GET_TXSAP_VOD_FLOW,
	
	BB_GET_TXAP_ADO_FLOW,
	
	BB_GET_DEF_1T1R_AP_ADO_FLOW,
	BB_GET_DEF_1T1R_STA1_ADO_FLOW,
	BB_GET_DEF_1T1R_STA1_VDO_FLOW,
	
	BB_GET_DEF_2T1R_AP_ADO_FLOW,
	BB_GET_DEF_2T1R_STA1_ADO_FLOW,
	BB_GET_DEF_2T1R_STA2_ADO_FLOW,
	BB_GET_DEF_2T1R_STA1_VDO_FLOW,
	BB_GET_DEF_2T1R_STA2_VDO_FLOW,
	
	BB_GET_DEF_3T1R_AP_ADO_FLOW,
	BB_GET_DEF_3T1R_STA1_ADO_FLOW,
	BB_GET_DEF_3T1R_STA2_ADO_FLOW,
	BB_GET_DEF_3T1R_STA3_ADO_FLOW,
	BB_GET_DEF_3T1R_STA1_VDO_FLOW,
	BB_GET_DEF_3T1R_STA2_VDO_FLOW,
	BB_GET_DEF_3T1R_STA3_VDO_FLOW,
	
	BB_GET_DEF_4T1R_AP_ADO_FLOW,
	BB_GET_DEF_4T1R_STA1_ADO_FLOW,
	BB_GET_DEF_4T1R_STA2_ADO_FLOW,
	BB_GET_DEF_4T1R_STA3_ADO_FLOW,
	BB_GET_DEF_4T1R_STA4_ADO_FLOW,
	BB_GET_DEF_4T1R_STA1_VDO_FLOW,
	BB_GET_DEF_4T1R_STA2_VDO_FLOW,
	BB_GET_DEF_4T1R_STA3_VDO_FLOW,
	BB_GET_DEF_4T1R_STA4_VDO_FLOW,
#endif		
}BB_GET_DATA_FLOW;

typedef struct 
{
	SET_TYPE	Type;
	GET_STA 	tSTA;
	uint32_t 	ulSize;
	uint8_t 	ubGetCrc;
	uint32_t 	ulCrcLen;
	uint32_t 	ulAddr;
}RX_DON;

typedef struct 
{
	LINK_ROLE 		tRole;
	LINK_STATUS		tStatus;
}LINK_REPORT;
//------------------------------------------------------------------------
/*!
\brief BB Initial
\param tBB_SlotMode (input)	Slot mode setup
\param pData 	(input) 	Timing By Flash 
\param ubStatus (input) 	0:Disable RF initial, 1:Enable RF initial
\param ubRFGIO1 (input) 	Control RF GIO1 setting
\param ubRFGIO2 (input) 	Control RF GIO2 setting
\param ubMode   (input) 	0:Run FCC/1:Run CE
\param ubDetectRssi (input) Detect TX channel RSSI.RSSI >= ubDetectRssi.Do not TX.CE only
\return(No)
*/
void BB_Init(SET_SLOT_MODE tBB_SlotMode, uint8_t *pData, uint8_t ubStauts,uint8_t ubGIO1,uint8_t ubGIO2,uint8_t ubMode,uint8_t ubDetectRssi);
//------------------------------------------------------------------------
/*!
    \brief      Baseband Start
    \param[in]  ulStackSize Thread Stack Size 	
    \param[in]  priority    Thread Priority
    \return     none
    \note       Please always keep the Thread Priority at highest priority
*/
void BB_Start(uint32_t ulStackSize, osPriority priority);
//------------------------------------------------------------------------
/*!
\brief Baseband Stop
\param (No) 	
\par Note
	 Baseband Stop Tx/Rx
\return (No)
*/
void BB_Stop(void);
//------------------------------------------------------------------------
/*!
\brief Setting Video/Audio Data Path 
\param tTxAdo 	(input)		Tx Audio Data Path 	
\param tRxAdo 	(input) 	Rx Audio Data Path
\param tRxVdo 	(input) 	Open At The Same Time,Sta Video/Data Payload
\return (No)
*/
void BB_SetDataPath(TXADO tTxAdo,RXADO tRxAdo,PAYLOAD_PATH tPath);
//------------------------------------------------------------------------
/*!
\brief Setting Video/Audio Packet Max Retry Time
\param Typ 		(input)		Video Or Audio 	
\param Time 	(input) 	Retry Time
\return (No)
*/
void BB_SetPacketRetryTime(SET_RETRY_TYP Typ,RETRY_TIME Time);
//------------------------------------------------------------------------
/*!
\brief Send Vidoe/Audio Data Port
\param Queue 	(input)	Reqistered Queue.Send Video/Audio After Finish,Return Status 	
\param Typ		(input) Vido/Audio
\param Addr 	(input) Video/Audio Data Start Address
\param ulLen 	(input) Video/Audio Size
\param tTxPath 	(input) Set Tx Video Path.AUDIO Set NULL
\par Note
	1. If Queue Is Set NULL Finish No Return Status \n
	2. If Queue Is Not NULL Return Status \n
		(BB_SEND_SUCCESS)		Send Video/Audio Is Success \n
		(BB_SEND_FAIL)			Send Video/Audio Is Fail \n
		(BB_SEND_RETRY_TIMEOUT)	Send Video/Audio Is Timeout \n
\return SET_BUF \n
		(BB_SET_BUF_SUCCESS)	Video/Audio Copy To Basebnad Buffer Success \n	
		(BB_SET_BUF_BUSY)		Basebnad Buffer Full \n
		(BB_SET_BUF_FAIL)		Video/Audio More than the Basebnad Buffer \n	
*/
SET_BUF tBB_SendData(osMessageQId *Queue,SET_TYPE Typ,uint8_t *Addr,uint32_t ulLen,SET_TX_PATH tTxPath);
//------------------------------------------------------------------------
/*!
\brief Rx Vidoe/Audio After Finish,Return Status 
\param Queue 	(input)	Reqistered Queue.Get Video/Audio After Finish,Return Status 	
\param Typ		(input) Vido/Audio
\par Note
	1. If Queue Is Set NULL Finish No Return Status \n
	2. If Queue Is Not NULL Return Status \n
		(Type)		Get Data Is Video or Audio \n 
		(tSTA)		Get Data Is That Sta Number \n
		(ulSize)	Data Size \n
		(ulAddr)	Data Start Address \n
\return (No)	
*/
void BB_RxDataReqisteredQueue(osMessageQId *Queue,SET_TYPE Typ);
//------------------------------------------------------------------------
/*!
\brief AP/Sta TRX Frmae Finish 
\param Queue (input) Reqistered Queue.TRX After Finish,Return Status 	
\par Note
	1. If Queue Is Set NULL Finish No Return Status \n
	2. If Queue Is Not NULL Return Status \n
	3. Queue type uint8_t \n
	4. AP side(Return 0 = Get Sta1,1 = Get Sta2,2 = Get Sta3,3 = Get Sta4) \n
	5. Sta side(Return 0x0F) \n
\return (No)	
*/
void BB_FrameOkReqisteredQueue(osMessageQId *Queue);
//------------------------------------------------------------------------
/*!
\brief Report Link/Lose Link Status
\param Queue 	(input)	Reqistered Queue. 	
\par Note
		It will only be reported once, unless the state changes \n
		1. If Queue Is Set NULL Finish No Return Status \n
		2. If Queue Is Not NULL Return Status \n
		(tRole)		Link Or Lose Link Sta \n 
		(tStatus)	Link Or Lose Link \n
\return (No)	
*/
void BB_LinkStatusReqisteredQueue(osMessageQId *Queue);
//------------------------------------------------------------------------
/*!
\brief Get Baseband Use Start/End Address.
\param tUse 	(input)	Get Start/End Address	
\par Note
		Baseband Address Usage Status
\return Address	
*/
uint32_t ulBB_GetBasebandUseAddr(ADDR_STATUS tUse);
//------------------------------------------------------------------------
/*!
\brief Get Baseband Use Payload Start/End Address.
\param tUse 	(input)	Get Start/End Address	
\par Note
		Baseband Address Usage Status
\return Address	
*/
uint32_t ulBB_GetPayloadFifoAddr(ADDR_STATUS tUse);
//------------------------------------------------------------------------
/*!
\brief Release Baseband RX Buffer	
\param Typ		(input) Vido/Audio
\param STA 		(input)	Release STAx RX Buffer
\par Note
		End Processing Rx Buffer,Call The Function 
\return (No)	
*/
void BB_RxBufRelease(SET_TYPE Typ,GET_STA tSTA);
//------------------------------------------------------------------------
/*!
\brief Set RF In WakeUp On Rx Mode	
\param uwSleepTime		(input) [9:0],Scal = 7.8ms
\param ubRxTime 		(input)	[5:0],Scal = 244us
\par Note
		RF In WakeUp On Rx Mode,RF will fall asleep and wake up
\return (No)	
*/
void BB_EnableWOR(uint16_t uwSleepTime,uint8_t ubRxTime);
//------------------------------------------------------------------------
/*!
\brief Disband Sync STA	
\param (No)
\par Note
		AP Only
\return (No)	
*/
void BB_DisbandSync(void);
//------------------------------------------------------------------------
/*!
\brief Send WakeUp Packet To WakeUp The Other Side 
\param uwTime	(input)Send Packet Number of Times	
\param tRole	(input)Wake Up Sta
\par Note
		Interval Unit = 1ms
\return GET_STATUS \n
		Get ACK
*/
GET_STATUS tBB_SendWakeUpPacket(uint16_t uwTime,LINK_ROLE tRole);
//------------------------------------------------------------------------
/*!
\brief Get Header/Video/Audio Packet Error Rate
\param tPerType (input)	Get Header/Video/Audio 	
\param tRole    (input)	Get Packet Error Rate Role
\par Note
		1. AP Get Header/Video/Audio \n
		1. Sta Get Header/Audio \n	
\return  uint8_t \n
		Baseband Count 100 Times.(0 is Best)(100 the Worst) 	
*/
uint8_t ubBB_GetPer(PER_TYPE tPerType,GET_PER_ROLE tRole);
//------------------------------------------------------------------------
/*!
\brief Get RSSI Value
\param tGetRole (input) Received Role
\return  uint8_t \n
		RSSI Value	
*/
uint8_t ubBB_GetRssiValue(GET_RSSI_ROLE tGetRole);
//------------------------------------------------------------------------
/*!
\brief Tx Power Setting 
\param ubMde 	  (input) 0 -> Disable, 1 -> Enable Setting
\param ubTxPwrReg (input) Mapping RF Tx Power Register, 
\par Note
		Sta Only
\return (No) 
			
*/
void BB_SetTxPwr(uint8_t ubMde,uint8_t ubTxPwrReg);
//------------------------------------------------------------------------
/*!
\brief Enable Baseband Debug GPIO 
\param tMde 	  	 (input) BB_DEBUG_NONE or (BB_DEBUG_HOP|BB_DEBUG_HECOK|BB_DEBUG_ADOOK|BB_DEBUG_VDOOK)
\param uwHopGPIO 	 (input) HOP Start Define GPIO0 ~ 13 
\param uwHecOkGPIO   (input) HEC OK Define GPIO0 ~ 13
\param uwAdoOkGPIO   (input) ADO OK Define GPIO0 ~ 13
\param uwVdoOkGPIO   (input) VDO OK Define GPIO0 ~ 13
\par Note
		(No)
\return (No) 
			
*/
void BB_DebugPinEnable(BB_DEBUG_MD tMde,uint16_t uwHopGPIO,uint16_t uwHecOkGPIO,uint16_t uwAdoOkGPIO,uint16_t uwVdoOkGPIO);
//------------------------------------------------------------------------
/*!
\brief Initail Baseband Variable 
\param ulBaseAddrs 	  (input) Basebnad Variable Start Address
\par Note
		(No)
\return (No) 
			
*/
void BB_VariableInit(uint32_t ulBaseAddrs);
//------------------------------------------------------------------------
/*!
\brief Get Video and Audio Flow
\param tFlow 	  	 (input) Get Path
\par Note
		
\return Bytes Per Second
			
*/
uint32_t ulBB_GetBBFlow(BB_GET_DATA_FLOW tFlow);
//------------------------------------------------------------------------
/*!	\file BB_API.h
nT1R_MainApFlow:
	\dot
digraph nT1R_MainApFlow
{
"Start"[shape=hexagon];

"BB Initial"[shape=box];
"BB Start"[shape=box];
"Segmentation Audio And Video"[shape=box];
"Send Beacon"[shape=box];
"Wait Ack"[shape=box];
"Analysis Ack Packet"[shape=box];
"Process Ack Packet"[shape=box];
"Select Next Channel"[shape=box];


"Get Ack ?"[shape=diamond];
"Other Sta ?"[shape=diamond];

"Start" 			-> 		"BB Initial";
"BB Initial"			->		"BB Start";
"BB Start"			->		"Segmentation Audio And Video"	
"Segmentation Audio And Video"		->		"Send Beacon"
"Send Beacon"			->		"Wait Ack"
"Wait Ack"			->		"Get Ack ?"
"Get Ack ?"			->		"Analysis Ack Packet"[label = "Yes"]; 	
"Get Ack ?"			->		"Select Next Channel"[label = "No"]; 
"Select Next Channel"	->	"Send Beacon"


"Analysis Ack Packet"		->		"Process Ack Packet"	
"Process Ack Packet"		->		"Other Sta ?"
"Other Sta ?"			->		"Select Next Channel"[label = "No"];
"Other Sta ?"			->		"Wait Ack"[label = "Yes"];


}
	\enddot
nT1R_MainStaFlow:
	\dot
digraph nT1R_MainStaFlow
{
"Start"[shape=hexagon];

"BB Initial"[shape=box];
"BB Start"[shape=box];
"Segmentation Audio And Video"[shape=box];
"Send Ack"[shape=box];
"Wait Beacon"[shape=box];
"Process Beacon Packet"[shape=box];
"Analysis Beacon Packet"[shape=box];
"Ready To Send Ack"[shape=box];
"Select Next Channel"[shape=box];


"Get Beacon ?"[shape=diamond];
"Time Out ?"[shape=diamond];

"Start" 			-> 		"BB Initial";
"BB Initial"			->		"BB Start";
"BB Start"			->		"Wait Beacon"
"Wait Beacon"			->		"Get Beacon ?"
"Get Beacon ?"			->		"Analysis Beacon Packet"[label = "Yes"];
"Get Beacon ?"			->		"Time Out ?"[label = "No"];
"Time Out ?"			->		"Wait Beacon"[label = "No"];
"Time Out ?"			->		"Select Next Channel"[label = "Yes"];
"Select Next Channel"		->		"Wait Beacon"	


"Analysis Beacon Packet"		->		"Process Beacon Packet"	
"Process Beacon Packet"		->		"Ready To Send Ack"
"Ready To Send Ack"		->		"Segmentation Audio And Video"
"Segmentation Audio And Video"		->		"Send Ack"
"Send Ack"			->		"Select Next Channel"
}
	\enddot
*/

//------------------------------------------------------------------------
/*!
\brief Get RX Audio Path
\return Path
*/
RXADO tBB_GetRxAdoPath(void);

//------------------------------------------------------------------------
/*!
\brief Get TX Audio Path
\return Path
*/
TXADO tBB_GetTxAdoPath(void);

//------------------------------------------------------------------------
/*!
\brief External Set Role
\param ubRole Role
\return(no)
*/
void BB_ExtSetRole(uint8_t ubRole);

//------------------------------------------------------------------------
/*!
\brief Get Start Flag
\return Flag
*/
uint8_t ubBB_GetStartFlg(void);

//------------------------------------------------------------------------
/*!
\brief Get TX Total Buffer Number
\param tTxPath TX Path
\return Buffer Number
*/
uint8_t ubBB_GetTxTotalBufNum(SET_TYPE tType,SET_TX_PATH tTxPath);

//------------------------------------------------------------------------
/*!
\brief Get TX Used Buffer Number
\param tTxPath TX Path
\return Buffer Number
*/
uint8_t ubBB_GetTxUsedBufNum(SET_TYPE tType,SET_TX_PATH tTxPath);
//------------------------------------------------------------------------
void BB_HoppingPairingStart(void);
void BB_HoppingPairingEnd(void);
//------------------------------------------------------------------------
typedef enum
{
	AFH_FIX_CH = 0,
    AFH_SEQENCE_CH,
    AFH_ADA_CH,
}AFH_HOPPING_MODE;
void AFH_Start(uint8_t ubMode,uint8_t ubCe,uint8_t ubChannel);
//------------------------------------------------------------------------
/*!
\brief Clear TX Buffer
\param tTxPath TX Path
\param tTyp Data Type
\return (no)
*/
void BB_ClearTxBuf(SET_TX_PATH tTxPath,SET_TYPE tTyp);

//------------------------------------------------------------------------
/*!
\brief Get Tx Audio Path
\return Tx Audio Path
*/
uint8_t ubBB_GetTxAdoDataPath(void);

//------------------------------------------------------------------------
/*!
\brief Get Rx Audio Path
\return Rx Audio Path
*/
uint8_t ubBB_GetRxAdoDataPath(void);	

//------------------------------------------------------------------------
/*!
\brief Get Rx Video Path
\return Rx Video Path
*/
uint8_t ubBB_GetRxVdoDataPath(void);

//------------------------------------------------------------------------
/*!
\brief Get CRC Report
\param  ulAddr    	Memory address
\param  ulSize    	Data length
\return CRC Report
*/
uint8_t ubBB_GetCrcReport(uint32_t ulAddr,uint32_t ulSize);

//------------------------------------------------------------------------------
/*!
\brief  Confirm WakeUp
\return	1:Yes 0:No
*/
uint8_t BB_ConfirmWakeUpInf(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set wakeUp	
\param  ubStatus    0:Disable WakeUp, 1:Enable WakeUp
\param  ubSta    	0,1,2,3,4
\return	(no)
*/
void BB_SetWakeUp(uint8_t ubStatus,uint8_t ubSta);
//------------------------------------------------------------------------------
/*!
\brief 	Get BB Version	
\return	Version
*/
uint16_t uwBB_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief  To clear TX audio buffer
\param  ubSta    AP = 0,1,2,3,  STA = 4
\return	(no)
*/
void BB_ToClearTxBuf(uint8_t ubSta);
//------------------------------------------------------------------------------
EN_RETURN_STATUS tEN_Start(EN_STATUS tEnStatus,EN_SET_ROLE tRole,uint8_t *pData1,uint8_t *pData2); 

extern uint8_t ubTestChTable[3];
extern uint8_t ubFixChTable[3];
#endif	/*__ST53510_BB_API_H*/
