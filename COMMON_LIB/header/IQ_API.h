/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IQ_API.h
	\brief		Image quality API function header
	\author		BoCun
	\version	0.1
	\date		2017/03/15
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _IQ_API_H_
#define _IQ_API_H_

//==============================================================================
// FILE INCLUSION
//==============================================================================
#include "SEN.h"

//==============================================================================
// DEFINITION
//==============================================================================
enum {
	CNL_OFFSET_B = 1,
	CNL_OFFSET_GB,
	CNL_OFFSET_GR,
	CNL_OFFSET_R,
	CNL_GAIN_B,
	CNL_GAIN_GB,
	CNL_GAIN_GR,
	CNL_GAIN_R,
	LSC_RSLOPE1,
	LSC_GSLOPE1,
	LSC_BSLOPE1,
	AWB_POS_RGAIN,
	AWB_POS_GGAIN,
	AWB_POS_BGAIN,
	DT_MATRIX_YR,
	DT_MATRIX_YG,
	DT_MATRIX_YB,
	DT_MATRIX_UR,
	DT_MATRIX_UG,
	DT_MATRIX_UB,
	DT_MATRIX_VR,
	DT_MATRIX_VG,
	DT_MATRIX_VB,
	RGB_GAMMA0,
	RGB_GAMMA1,
	RGB_GAMMA2,
	RGB_GAMMA3,
	RGB_GAMMA4,
	RGB_GAMMA5,
	RGB_GAMMA6,
	RGB_GAMMA7,
	RGB_GAMMA8,
	RGB_GAMMA9,
	RGB_GAMMA10,
	RGB_GAMMA11,
	RGB_GAMMA12,
	RGB_GAMMA13,
	RGB_GAMMA14,
	RGB_GAMMA15,
	RGB_GAMMA16,
	RGB_GAMMA17,
	RGB_GAMMA18,
	RGB_GAMMA19,
	RGB_GAMMA20,
	RGB_GAMMA21,
	RGB_GAMMA22,
	RGB_GAMMA23,
	RGB_GAMMA24,
	RGB_GAMMA25,
	RGB_GAMMA26,
	RGB_GAMMA27,
	RGB_GAMMA28,
	RGB_GAMMA29,	
	CI_EDGE_GAIN_THD,
	CI_EDGE_HGAIN,
	CI_EDGE_VGAIN,
	CI_EDGE_GRADLB,
	CI_EDGE_THD,
	DARK_AMP_LMT,
	INTENSITY_VAR,
	gL2E_PC0,
	GL2E_PC1,
	GL2E_PC2,
	GL2E_PC3,
	gL2E_PC4,
	gL2H2_PC0,
	GL2H2_PC1,
	GL2H2_PC2,
	GL2H2_PC3,
	GL2E_O0,
	GL2E_O1,
	gL2E_o2,
	GL2E_O3,
	GL2E_O4,
	GL2H2_O0,
	GL2H2_O1,
	gL2H2_o2,
	GL2H2_O3,
	GL2H2_O4,
	gE2H2_PC0,
	GE2H2_PC1,
	GE2H2_PC2,
	gE2H2_PC3,
	gL2H2_PC4,
	GE2H2_O0,
	GE2H2_O1,
	GE2H2_O2,
	NS_SLP_B,
	NS_SLP_G,
	NS_SLP_R,
	NS_OFF_B,
	NS_OFF_G,
	NS_OFF_R,
	W_PC1,
	W_PC2,
	W_PC3,
	W_O1,
	W_O2,
	FLAT_SLP_B,
	FLAT_SLP_G,
	FLAT_SLP_R,
	IP_LOW,
	IP_HIGH,
	EDGE_OFF_B,
	EDGE_OFF_G,
	EDGE_OFF_R,
	FLAT_AC_RB,
	FLAT_AC_G,
	CNLM_EN,
	EDM_EN,
	EDM_MODE,
	DPC_CLS_EN,
	DPC_CLS3_EN,
	DPC_BLK_THD,
	DPC_WHT_THD,
	DPC_BLK_SLP,
	DPC_WHT_SLP,
	DPC_EDGE_THD,
	DPC_EDGE_GAIN,
	NR3D_RSLOP0,
	NR3D_GSLOP0,
	NR3D_BSLOP0,
	NR3D_RSLOP1,
	NR3D_GSLOP1,
	NR3D_BSLOP1,
	NR3D_RP,
	NR3D_GP,
	NR3D_BP,
	NR3D_ROFFSET,
	NR3D_GOFFSET,
	NR3D_BOFFSET,
	NR3D_MOTION_LB,
	NR3D_MOTION_HB,
	NR3D_W0,
	NR3D_W1,
	NR3D_W2,
	NR3D_W3,
	NR3D_W4,
	AYF_YEEM_THD,
	UI_Y_OFFSET,
	UI_Y_GAIN,
};

enum {
	ISP_PATH1 = 1,
	ISP_PATH2,
	ISP_PATH3,    
};

#define IQ_EVENT_SUSPEND			(0)
#define IQ_EVENT_RESUME				(1)
#define IQ_EVENT_VSYNC				(2)
#define IQ_EVENT_START				(3)
#define IQ_EVENT_STOP				(4)

#define IQ_STATE_SUSPEND			(0)
#define IQ_STATE_STANDBY			(1)
#define IQ_STATE_DO					(2)
#define IQ_STATE_WAIT				(3)
// ------------ TUNING --------------
// Library Parameter
#define IQ_WAIT_FRAMES				(0)

//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define IQ_SetSkip(ubSp)								{ xtIQDnInst.ubSkip = ubSp; }
#define ubIQ_GetSkip()									( xtIQDnInst.ubSkip)

#define IQ_DynamicSuspend()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_SUSPEND); }
#define IQ_DynamicResume()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_RESUME); }
#define IQ_DynamicVSync()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_VSYNC); }
#define IQ_DynamicStart()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_START); }
#define IQ_DynamicStop()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_STOP); }

//==============================================================================
// STRUCT
//==============================================================================
typedef struct tagIQJudgeObj {
	// Dynamic IQ Judge Value
	uint16_t uwAWB_BGainX128;
	uint16_t uwAWB_GGainX128;
	uint16_t uwAWB_RGainX128;
	uint8_t ubAE_Expidx;
	uint16_t uwAE_CurrGain;
	uint32_t ulAE_AbsY;
	uint32_t ulAF_W0Sum;
	uint32_t ulAF_W1Sum;
}IQJudgeObj;

typedef struct tagIQDNObj {
	// Dynamic IQ System and State Control
	uint8_t ubState;
	uint8_t ubWaitCnt;
	uint8_t ubSkip;
	uint32_t uwCurrPrevHSz;
	// Dynamic IQ Judge Value
	IQJudgeObj xtIQJudgeInst;	
}IQDNObj;

//==============================================================================
// IQ extern item
//==============================================================================
#if(IQ_DN_EN == 1 && AE_EN == 1 && AWB_EN == 1 && AF_EN == 1)
	extern IQDNObj xtIQDnInst;
#endif

//------------------------------------------------------------------------
/*!
\brief Set IQ default state.
\return(no)
\par [Example]
\code 
		IQ_Init();	
\endcode	
*/
void IQ_Init(void);
//------------------------------------------------------------------------
/*!
\brief Linear Interpolation	Function.
\param uwJudgeVal 	-
\param ubRowMaxNum 	-
\param ubLinearNum 	-
\param plLinear 		-
\param plReturnVal 	-
\return(no)
*/
void IQ_DynamicLinearInterpolation(uint32_t ulJudgeVal, uint8_t ubRowMaxNum, uint8_t ubLinearNum, int32_t *plLinear, int32_t *plReturnVal);
//------------------------------------------------------------------------
/*!
\brief Set brightness/contrast.
\param uwGamma 	gamma value.
\param swBrit 	bright value.
\param uwCntrs 	constrast value.
\param uwGain 	gain.
\return(no)
*/
void IQ_SetGammaBrightnessContrast(uint16_t uwGamma, int16_t swBrit, uint16_t uwCntrs, uint16_t uwGain);
//------------------------------------------------------------------------
/*!
\brief Set hue/saturation.
\param swHue 		Hue value.
\param uwSatu 	Saturation value.
\return(no)
*/
void IQ_SetClrMtx(int16_t swHue, uint16_t uwSatu);
//------------------------------------------------------------------------
/*!
\brief Set edge enhance.
\param ubHGain 	horizontal Gain.
\param ubVGain 	vertical Gain.
\param ubThd 		threshold value.
\return(no)
*/
void IQ_SetEdgeEnhnce(uint8_t ubHGain, uint8_t ubVGain, uint8_t ubThd);
//------------------------------------------------------------------------
/*!
\brief Set resolution of IQ struct.
\return(no)
*/
void IQ_SetISPRes(void);
//------------------------------------------------------------------------
/*!
\brief Set mirror/flip.
\param ubMirrorEn 	mirror switch.
\param ubFlipEn 		flip  switch.
\return(no)
*/
void IQ_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn);
//------------------------------------------------------------------------
/*!
\brief Dynamic IQ SDK.
\return(no)
*/
void IQ_DYNAMIC_SDK(void);
//------------------------------------------------------------------------
/*!
\brief Dynamic IQ inital SDK.
\return(no)
*/
void IQ_DynamicInit_SDK(void);
//------------------------------------------------------------------------
/*!
\brief Set resolution.
\param uwWidth 			Resolution width.
\param uwHeight 		Resolution height.
\param ubFps 				Frame rate.
\param ubPath 			Path1/2.
\return(no)
*/
void IQ_SetResolution_SDK(uint16_t uwWidth, uint16_t uwHeight, uint8_t ubFps, uint8_t ubPath) ;
//------------------------------------------------------------------------
/*!
\brief Set IQ command value.
\param ubId 	command item.
\param swVal 	value.
\return(no)
*/
void IQ_SetCmdVal(uint8_t ubId, int16_t swVal);
//------------------------------------------------------------------------
/*!
\brief Setting bin file value.
\param ulCmd 		command item.
\param ulAddr 	address.
\param ulMask 	data mask .
\param ulVal 		value.
\return(no)
*/
void IQ_SetBinVal(uint32_t ulCmd, uint32_t ulAddr, uint32_t ulMask, uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Switch tunning tool.
\param ubFuncEn 	on/off.
\return(no)
*/
void IQ_SetTunningSwitch(uint8_t ubFuncEn);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ default state.
\return(no)
*/
void IQ_DynamicInit(void);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ value(AWB,AE).
\return(no)
*/
void IQ_DynamicJudgeVal(void);
//------------------------------------------------------------------------
/*!
\brief Dynamic 	IQ operating process.
\param ubEvent 	Dynamic IQ current state.
\return(no)
*/
void IQ_DynamicStatemachine3G5(uint8_t ubEvent);
//------------------------------------------------------------------------
/*!
\brief Set noise reduce(NR) value.
\param ulId 	NR index.
\param ulVal 	NR value.
\retval true	0->set nr value success.
\retval false	1->set nr value fail.
*/
uint8_t ubIQ_SetNrTurningVal(uint32_t ulId, uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get noise reduce(NR) value.
\param ulId 	NR index.
\param ulMask 	NR value.
\retval value	Get value.
*/
uint32_t ulIQ_GetNrTurningVal(uint32_t ulId, uint32_t ulMask);
//------------------------------------------------------------------------
/*!
\brief 	Get IQ Version	
\return	Version
*/
uint16_t uwIQ_GetVersion(void);
//------------------------------------------------------------------------
/*!
\brief Parser Dynamic IQ (black level).
\param ubTemp 	Dynamic IQ value.
\return(no)
*/
void IQ_SetAeCtrlPct(uint8_t *ubTemp);
#endif
