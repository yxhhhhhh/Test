/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		DIS_API.h
	\brief		Digital image stabilization API functio header
	\author		BoCun
	\version	1.0
	\date		2018/05/10
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _DIS_API_H_
#define _DIS_API_H_

#include "_510PF.h"
//------------------------------------------------------------------------------
/*!	\file DIS_API.h
DIS FlowChart:
	\dot
	digraph DIS_flow {
		node [shape=record, fontname=Verdana, fontsize=10, fixedsize=true, width=5];
		"Set register value. \n DIS_Initial(1280, 720, 15);"->
		"Wait interrupt. \n SEN_DIS_ISR();"->
		"Get report. \n DIS_FrmEnd_ISR();"->
		"Clear DIS_TRANS_READY = 1. \n DIS_ClearReady();"->
		"Calculate motion vector. \n DIS_CalMotionVector();"->
		"Send value to the image controller. \n  swDIS_MotionVectorW/H();";	
	}
	\enddot
*/

//==============================================================================
// STRUCT
//==============================================================================
typedef struct strcDIS_CumMaxTrans {
	uint16_t uwCUM_MAX_TRANS_H_0;		    //!< Maximum of transition for path1(horizontal)
	uint16_t uwCUM_MAX_TRANS_V_0;		    //!< Maximum of transition for path1(vertical)
	uint16_t uwCUM_MAX_TRANS_H_1;		    //!< Maximum of transition for path2(horizontal)
	uint16_t uwCUM_MAX_TRANS_V_1;		    //!< Maximum of transition for path2(vertical)
	uint16_t uwCUM_MAX_TRANS_H_2;		    //!< Maximum of transition for path3(horizontal)
	uint16_t uwCUM_MAX_TRANS_V_2;		    //!< Maximum of transition for path3(vertical)    
}DIS_CumMaxTrans;

typedef struct strcDIS_CumTransModel {
	uint16_t uwCUM_TRANS_MODEL_H_0;		    //!< Transition for path1(horizontal)
	uint16_t uwCUM_TRANS_MODEL_V_0;		    //!< Transition for path1(vertical)
	uint16_t uwCUM_TRANS_MODEL_H_1;		    //!< Transition for path2(horizontal)
	uint16_t uwCUM_TRANS_MODEL_V_1;		    //!< Transition for path2(vertical)
	uint16_t uwCUM_TRANS_MODEL_H_2;		    //!< Transition for path3(horizontal)
	uint16_t uwCUM_TRANS_MODEL_V_2;		    //!< Transition for path3(vertical)    
}DIS_CumTransModel;

typedef struct strcDIS_TransModel {
	uint32_t ulTRANS_MODEL_H_0;		        //!< Horizontal global motion vector
	uint32_t ulTRANS_MODEL_V_0;		        //!< Vertical global motion vector
}DIS_TransModel;

typedef struct strcDIS_FE_Match {
	uint32_t ulCUR_FE_BUF_N;
	uint32_t ulMATCH_N;	
}DIS_FE_Match;

typedef struct strctISP_DIS_RPTObj {
    DIS_CumMaxTrans     xtDisCumMaxTrans;
    DIS_CumTransModel   xtDisCumTransModel;
    DIS_TransModel      xtDisTransModel;
    DIS_FE_Match        xtDisFEMatch;
}ISP_DIS_RPTObj;

typedef struct {
    int16_t     swDIS_Trans[2][2];	
	uint32_t 	ulImgW;
	uint32_t	ulImgH;
	uint32_t	ulUpImgH;	
	uint32_t	ulUpImgW;
}ISP_DIS_PARA;

//==============================================================================
// EXTERN
//==============================================================================
extern ISP_DIS_RPTObj xtISP_DIS_RPT;

//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!
\brief DIS initial.
\param ulHSize 		Horizontal size.
\param ulVSize 		Vertical size.
\return(no)
\par [Example]
\code
		DIS_Initial(1280, 720, 15);

		|----B----|
		||---A---||
		||       ||
		||-------||
		|---------|	
		set A,B	
		where A is OUT_IMGW/H (H264 encode)
				  B is A by the DIS up-scaler(i-th ISP output stream)
		note: B is bigger than A about 15%(FOV decrease 15%)	
\endcode
*/
void DIS_Initial(uint32_t ulHSize, uint32_t ulVSize);
//------------------------------------------------------------------------
/*!
\brief Pass cumulative motion vector(weight).
\return Cropping window weight.
\par [Example]
\code
		int16_t swVectorW;
		
		swVectorW = swDIS_MotionVectorW();
\endcode
*/
int16_t swDIS_MotionVectorW(void);
//------------------------------------------------------------------------
/*!
\brief Pass cumulative motion vector(height).
\return Cropping window height.
\par [Example]
\code
		int16_t swVectorH;

		swVectorH = swDIS_MotionVectorH();
\endcode
*/
int16_t swDIS_MotionVectorH(void);
#endif
