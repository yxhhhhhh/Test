/*
**********************************************************************************************************
*                                                  AWB H File
*
*									Copyright 2003-2005, SONiX Technology Co.,LTD.
*					9F, No. 8, Lane32, Hsien Cheng 5th St, Chupei City, Hsinchu, Taiwan R.O.C.
*
* FILE     : 
* VERSION  : 
* ENGINEER : 
**********************************************************************************************************
*/

#ifndef _AE_ALG_API_H_
#define _AE_ALG_API_H_

//----------------------------------------------------------
//                                                         |
//                       MACRO FUNCTION                    |
//                                                         |
//----------------------------------------------------------
//Structure
#define xtAE_Info(ptAlgInst)							(ptAlgInst->xtAEInfo)									
#define xtAE_Ctrl(ptAlgInst)							(ptAlgInst->xtAE_Ctrl)

//pt
#define ptAE_SeqTbl(ptFsmInst)							(ptFsmInst->xtAE_Seq.ptAE_Seq)
#define ptAE_YTarTbl(ptAlgInst)							(ptAlgInst->xtAE_YTar.ptAE_YTarTab)
#define ptAE_IdxGainTbl_50Hz(ptAlgInst)					(ptAlgInst->xtAE_ExpGainTbl_50Hz.ptAE_ExpTbl)
#define ptAE_IdxGainTbl_60Hz(ptAlgInst)					(ptAlgInst->xtAE_ExpGainTbl_60Hz.ptAE_ExpTbl)
#define ptAE_ExpLineTbl(ptAlgInst)						(ptAlgInst->xtAE_ExpLineTbl.ptAE_ExpLineTbl)
#define ptAE_SenGainUnitTbl(ptAlgInst)					(ptAlgInst->xtAE_GainUnitTbl.ptAE_GainUnitTbl)
#define ptAE_HistBinTbl(ptAlgRptInst)					(ptAlgRptInst->xtAE_HistBinTbl.pbWeight)
#define ptAE_HistWeightTbl(ptAlgRptInst)				(ptAlgRptInst->xtAE_HistWeightTbl.pbWeight)
#define ptAE_RespWeightTbl(ptAlgRptInst)				(ptAlgRptInst->xtAE_RespWeightTbl.pbWeight)

//Get
#define ubAE_GetStableStatus(ptAlgInst)					(ptAlgInst->xtAEInfo.ubStable)
#define uwAE_GetCurrentY(ptAlgInst)						(ptAlgInst->xtAEInfo.uwCurrY)
#define ubAE_GetDrCmp(ptAlgInst)						(ptAlgInst->xtAEInfo.ubDrCmp)
#define uwAE_GetTarget(ptAlgInst)						(ptAlgInst->xtAEInfo.uwYTar)
#define ulAE_GetAbsY(ptAlgInst)							(ptAlgInst->xtAEInfo.ulAbsY)
#define ulAE_GetCurrIdxGain(ptAlgInst)					(ptAlgInst->xtAEInfo.ulIdxGain[0])
#define ubAE_GetFineScl(ptAlgInst)						(ptAlgInst->xtAEInfo.ubFineScl)
#define uwAE_GetGain(ptAlgInst)							(ptAlgInst->xtAEInfo.uwGain)
#define ubAE_GetExpIdx(ptAlgInst)						(ptAlgInst->xtAEInfo.ubExpIdx)
#define ulAE_GetExpTime(ptAlgInst)						(ptAlgInst->xtAEInfo.ulExpTime)
#define uwAE_GetExpLine(ptAlgInst)						(ptAlgInst->xtAEInfo.uwExpLine)

	// System Parameter
#define ubAE_GetRptDelay(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubRptDelay)
#define ubAE_GetPowerFreq(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.ubPwrFreq)	
#define ubAE_GetLowLightMode(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.ubLowLightMode)
#define ubAE_GetOutdoorMode(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.ubOutdoorMode)	
	                                                                                                 
	// Histrogram Report   
#define ubAE_GetPctLL(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPctLL)  
#define ubAE_GetPctLH(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPctLH) 
#define ubAE_GetPctHL(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPctHL) 
#define ubAE_GetPctHH(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPctHH)                                                                          
#define sbAE_GetTarYOffset(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.sbTargetOffset)
	                                                                                                 
	// Stable Zone Control
#define ubAE_GetOverCntThd(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.ubOverCntThd )
#define ubAE_GetUnderCntThd(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.ubUnderCntThd)
#define ubAE_GetStbCntThd(ptAlgInst)					(ptAlgInst->xtAE_Ctrl.ubStbCntThd)	
#define ubAE_GetStbHB(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubStableHB)
#define ubAE_GetStbLB(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubStableLB)
#define ubAE_GetCtrlHB(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubControlHB)
#define ubAE_GetCtrlLB(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubControlLB)
	
	// Stepping Control		
#define ubAE_GetSpeedH(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubSpeedH)
#define ubAE_GetSpeedL(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubSpeedL)			
#define ubAE_GetPID_P(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPID[0])
#define ubAE_GetPID_I(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPID[1])
#define ubAE_GetPID_D(ptAlgInst)						(ptAlgInst->xtAE_Ctrl.ubPID[2]) 
#define ubAE_GetMAXY(ptAlgInst)							(ptAlgInst->xtAE_Ctrl.ubMaxY)

	// Table
#define ubAE_GetSeqTblRowSize(ptAlgInst)				(ptAlgInst->xtAE_Seq.ubRowSize)
#define ubAE_GetSeqTblColSize(ptAlgInst)				(ptAlgInst->xtAE_Seq.ubColSize)
#define ubAE_GetYTarTblRowSize(ptAlgInst)				(ptAlgInst->xtAE_YTar.ubRowSize)
#define ubAE_GetYTarTblColSize(ptAlgInst)				(ptAlgInst->xtAE_YTar.ubColSize)
#define ubAE_GetIdxGainTbl_50HzRowSize(ptAlgInst)		(ptAlgInst->xtAE_ExpGainTbl_50Hz.ubRowSize)
#define ubAE_GetIdxGainTbl_50HzColSize(ptAlgInst)		(ptAlgInst->xtAE_ExpGainTbl_50Hz.ubColSize)
#define ubAE_GetIdxGainTbl_60HzRowSize(ptAlgInst)		(ptAlgInst->xtAE_ExpGainTbl_60Hz.ubRowSize)
#define ubAE_GetIdxGainTbl_60HzColSize(ptAlgInst)		(ptAlgInst->xtAE_ExpGainTbl_60Hz.ubColSize)
#define ubAE_GetExpLineTblRowSize(ptAlgInst)			(ptAlgInst->xtAE_ExpLineTbl.ubRowSize)
#define ubAE_GetExpLineTblColSize(ptAlgInst)			(ptAlgInst->xtAE_ExpLineTbl.ubColSize)
#define ubAE_GetGainUnitTbl_RowSize(ptAlgInst)			(ptAlgInst->xtAE_GainUnitTbl.ubRowSize)
#define ubAE_GetGainUnitTbl_ColSize(ptAlgInst)			(ptAlgInst->xtAE_GainUnitTbl.ubColSize)
#define ubAE_GetHistBinTblSize(ptAlgRptInst)			(ptAlgRptInst->xtAE_HistBinTbl.ubSize)
#define ubAE_GetHistWeightTblSize(ptAlgRptInst)			(ptAlgRptInst->xtAE_HistWeightTbl.ubSize)
#define ubAE_GetRespWeightTblSize(ptAlgRptInst)			(ptAlgRptInst->xtAE_RespWeightTbl.ubSize)

//Set
#define AE_SetStable(ptAlgInst, ubSt)					{ ptAlgInst->xtAEInfo.ubStable = ubSt; }
#define AE_SetYL(ptAlgInst, uwAEYL)						{ ptAlgInst->xtAEInfo.uwYL = uwAEYL; }
#define AE_SetCurrentY(ptAlgInst, uwAECurY)				{ ptAlgInst->xtAEInfo.uwCurrY = uwAECurY; }
#define AE_SetYH(ptAlgInst, uwAEYH)						{ ptAlgInst->xtAEInfo.uwYH = uwAEYH; }
#define AE_SetBoundCtrlY(ptAlgInst, uwBcY)				{ ptAlgInst->xtAEInfo.uwBoundCtrlY = uwBcY; }
#define AE_CalAbsY(ptAlgInst, uwAEPureY)				{ ptAlgInst->xtAEInfo.ulAbsY = (ptAlgInst->xtAEInfo.ubFineScl?(uwAEPureY * (ptAlgInst->xtAEInfo.uwLinePerFrm * AE_RAW_UNIT * AE_FINESCL_UNIT) / ptAlgInst->xtAEInfo.ulIdxGain[ptAlgInst->xtAE_Ctrl.ubRptDelay]):(uwAEPureY * (AE_GAIN_UNIT * AE_RAW_UNIT) / ptAlgInst->xtAEInfo.ulIdxGain[ptAlgInst->xtAE_Ctrl.ubRptDelay])); }
#define AE_SetIdxGain(ptAlgInst, ulAEIdGain)			{ ptAlgInst->xtAEInfo.ulIdxGain[0] = ulAEIdGain;}
#define AE_SetGain(ptAlgInst, uwAEGain)					{ ptAlgInst->xtAEInfo.uwGain = uwAEGain; }
#define AE_SetExpIdx(ptAlgInst, ubAEExpIdx)				{ ptAlgInst->xtAEInfo.ubExpIdx = ubAEExpIdx; }
#define AE_SetExpTime(ptAlgInst, ulAEExpTime)			{ ptAlgInst->xtAEInfo.ulExpTime = ulAEExpTime; }
#define AE_SetExpLine(ptAlgInst, uwAEExpLine)			{ ptAlgInst->xtAEInfo.uwExpLine = uwAEExpLine; }
#define AE_SetLinePF(ptAlgInst, uwLpf)					{ ptAlgInst->xtAEInfo.uwLinePerFrm = uwLpf; }
#define AE_SetLowLightIdx(ptAlgInst, ubAELowLightIdx)	{ ptAlgInst->xtAEInfo.ubLowLightIdx = ubAELowLightIdx; }
#define AE_SetFrmIntvl(ptAlgInst, AEFrmIntvl)			{ ptAlgInst->xtAEInfo.uwFrmIntvl = AEFrmIntvl; }
   
	// System Parameter
#define AE_SetRptDelay(ptAlgInst, ubAERptDelay)			{ ptAlgInst->xtAE_Ctrl.ubRptDelay = ubAERptDelay; }	
#define AE_SetPowerFreq(ptAlgInst, ubAEPwrFreq)			{ ptAlgInst->xtAE_Ctrl.ubPwrFreq = ubAEPwrFreq; }	
#define AE_SetLowLightMode(ptAlgInst, ubAELowLightMode)	{ ptAlgInst->xtAE_Ctrl.ubLowLightMode = ubAELowLightMode; }
#define AE_SetOutdoorMode(ptAlgInst, ubAEOutdoorMode)	{ ptAlgInst->xtAE_Ctrl.ubOutdoorMode = ubAEOutdoorMode; }	
#define AE_SetExpMode(ptAlgInst, ubMode)				{ ptAlgInst->xtAE_Ctrl.ubExpMode = ubMode; }
#define AE_SetInsMode(ptAlgInst, ubMode)				{ ptAlgInst->xtAE_Ctrl.ubIvsMode = ubMode; }
	                                                                                                 
	// Histrogram Report 
#define AE_SetPctLL(ptAlgInst, ubAEPctLL)				{ ptAlgInst->xtAE_Ctrl.ubPctLL = ubAEPctLL; }  
#define AE_SetPctLH(ptAlgInst, ubAEPctLH)				{ ptAlgInst->xtAE_Ctrl.ubPctLH = ubAEPctLH; } 
#define AE_SetPctHL(ptAlgInst, ubAEPctHL)				{ ptAlgInst->xtAE_Ctrl.ubPctHL = ubAEPctHL; } 
#define AE_SetPctHH(ptAlgInst, ubAEPctHH)				{ ptAlgInst->xtAE_Ctrl.ubPctHH = ubAEPctHH; } 
#define AE_SetPctRHB(ptAlgInst, ubAEPctRHB)				{ ptAlgInst->xtAE_Ctrl.ubPctRHB = ubAEPctRHB; }
#define AE_SetPctRLB(ptAlgInst, ubAEPctRLB)				{ ptAlgInst->xtAE_Ctrl.ubPctRLB = ubAEPctRLB; }                                                                        
#define AE_SetTarYOffset(ptAlgInst, sbAETarYOffset)		{ ptAlgInst->xtAE_Ctrl.sbTargetOffset = sbAETarYOffset; }
	                                                                                                 
	// Stable Zone Control
#define AE_SetOverCntThd(ptAlgInst, ubAEOverCntThd)		{ ptAlgInst->xtAE_Ctrl.ubOverCntThd = ubAEOverCntThd; }
#define AE_SetUnderCntThd(ptAlgInst, ubAEUnderCntThd)	{ ptAlgInst->xtAE_Ctrl.ubUnderCntThd = ubAEUnderCntThd; }
#define AE_SetStbCntThd(ptAlgInst, ubAEStbCntThd)		{ ptAlgInst->xtAE_Ctrl.ubStbCntThd	= ubAEStbCntThd; }	
#define AE_SetStbHB(ptAlgInst, ubAESTBHB)				{ ptAlgInst->xtAE_Ctrl.ubStableHB = ubAESTBHB; }
#define AE_SetStbLB(ptAlgInst, ubAESTBLB)				{ ptAlgInst->xtAE_Ctrl.ubStableLB = ubAESTBLB; }
#define AE_SetCtrlHB(ptAlgInst, ubAECTRLHB)				{ ptAlgInst->xtAE_Ctrl.ubControlHB = ubAECTRLHB; }
#define AE_SetCtrlLB(ptAlgInst, ubAECTRLLB)				{ ptAlgInst->xtAE_Ctrl.ubControlLB = ubAECTRLLB; }
	
	// Stepping Control		
#define AE_SetSpeedH(ptAlgInst, ubAESpeedH)				{ ptAlgInst->xtAE_Ctrl.ubSpeedH = ubAESpeedH; }
#define AE_SetSpeedL(ptAlgInst, ubAESpeedL)				{ ptAlgInst->xtAE_Ctrl.ubSpeedL = ubAESpeedL; }			
#define AE_SetPID_P(ptAlgInst, ubAEPID_P)				{ ptAlgInst->xtAE_Ctrl.ubPID[0] = ubAEPID_P; }
#define AE_SetPID_I(ptAlgInst, ubAEPID_I)				{ ptAlgInst->xtAE_Ctrl.ubPID[1] = ubAEPID_I; }
#define AE_SetPID_D(ptAlgInst, ubAEPID_D)				{ ptAlgInst->xtAE_Ctrl.ubPID[2] = ubAEPID_D; } 
#define AE_SetMaxY(ptAlgInst, ubMax_Y)					{ ptAlgInst->xtAE_Ctrl.ubMaxY = ubMax_Y; }

#define AE_SetBlCom2BlCntThd(ptAlgInst, ubCntThd)		{ ptAlgInst->xtAE_Ctrl.ubBL_Com2BlCntThd = ubCntThd; }
#define AE_SetBlStbCntThd(ptAlgInst, ubCntThd)			{ ptAlgInst->xtAE_Ctrl.ubBL_StbCntThd = ubCntThd; }
#define AE_SetBlRspsCntThd(ptAlgInst, ubCntThd)			{ ptAlgInst->xtAE_Ctrl.ubBL_RspsCntThd = ubCntThd; }
#define AE_SetBlCom2BlSpd(ptAlgInst, ubSpd)				{ ptAlgInst->xtAE_Ctrl.ubBL_Com2BlSpd = ubSpd; }
#define AE_SetBlBl2ComSpd(ptAlgInst, ubSpd)				{ ptAlgInst->xtAE_Ctrl.ubBL_Bl2ComSpd = ubSpd; }
#define AE_SetBlPctRLBOffset(ptAlgInst, ubOffset)		{ ptAlgInst->xtAE_Ctrl.ubBL_PctRLBOffset = ubOffset; }

#if (AE_CBAdr==AE_CBADR_ENABLE)
#define AE_Seq_Reset(ptAlgFsmInst)						{ AE_SeqInit(ptAlgFsmInst); }
#else
#define AE_Seq_Reset()									{ AE_SeqInit(); }										
#endif

	// Table
#define AE_SetSeqTblRowSize(ptAlgInst, ubRowS)			{ ptAlgInst->xtAE_Seq.ubRowSize = ubRowS; }
#define AE_SetSeqTblColSize(ptAlgInst, ubColS)			{ ptAlgInst->xtAE_Seq.ubColSize = ubColS; }
#define AE_SetYTarTblRowSize(ptAlgInst, ubRowS)			{ ptAlgInst->xtAE_YTar.ubRowSize = ubRowS; }
#define AE_SetYTarTblColSize(ptAlgInst, ubColS)			{ ptAlgInst->xtAE_YTar.ubColSize = ubColS; }
#define AE_SetIdxGainTbl_50HzRowSize(ptAlgInst, ubRowS)	{ ptAlgInst->xtAE_ExpGainTbl_50Hz.ubRowSize = ubRowS; }
#define AE_SetIdxGainTbl_50HzColSize(ptAlgInst, ubColS)	{ ptAlgInst->xtAE_ExpGainTbl_50Hz.ubColSize = ubColS; }
#define AE_SetIdxGainTbl_60HzRowSize(ptAlgInst, ubRowS)	{ ptAlgInst->xtAE_ExpGainTbl_60Hz.ubRowSize = ubRowS; }
#define AE_SetIdxGainTbl_60HzColSize(ptAlgInst, ubColS)	{ ptAlgInst->xtAE_ExpGainTbl_60Hz.ubColSize = ubColS; }
#define AE_SetExpLineTblRowSize(ptAlgInst, ubRowS)		{ ptAlgInst->xtAE_ExpLineTbl.ubRowSize = ubRowS; }
#define AE_SetExpLineTblColSize(ptAlgInst, ubColS)		{ ptAlgInst->xtAE_ExpLineTbl.ubColSize = ubColS; }

#define AE_SetGainUnitTbl_RowSize(ptAlgInst, ubRowS)	{ ptAlgInst->xtAE_GainUnitTbl.ubRowSize = ubRowS; }
#define AE_SetGainUnitTbl_ColSize(ptAlgInst, ubColS)	{ ptAlgInst->xtAE_GainUnitTbl.ubColSize = ubColS; }
#define AE_SetHistBinTblSize(ptAlgRptInst, ubS)			{ ptAlgRptInst->xtAE_HistBinTbl.ubSize = ubS; }
#define AE_SetHistWeightTblSize(ptAlgRptInst, ubS)		{ ptAlgRptInst->xtAE_HistWeightTbl.ubSize = ubS; }
#define AE_SetRespWeightTblSize(ptAlgRptInst, ubS)		{ ptAlgRptInst->xtAE_RespWeightTbl.ubSize = ubS; }

#endif
