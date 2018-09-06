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

#ifndef _AWB_ALG_API_H_
#define _AWB_ALG_API_H_
//----------------------------------------------------------
//                                                         |
//                     VARIABLES EXTERN                    |
//                                                         |
//----------------------------------------------------------

//----------------------------------------------------------
//                                                         |
//                       MACRO FUNCTION                    |
//                                                         |
//----------------------------------------------------------
//Structure
#define xtAWB_Info(ptAlgInst)								(ptAlgInst->xtAWBInfo)									
#define xtAWB_Ctrl(ptAlgInst)								(ptAlgInst->xtAWBCtrl)

//pt
#define ptAWB_InitCXCYTbl(ptAlgInst)						(ptAlgInst->xtAWBInitCXCY.ptAWB_InitCXCYTab)
#define ptAWB_CXCYTbl(ptAlgInst)							(ptAlgInst->xtAWBCXCY.ptAWB_CXCYTab)
#define ptAWB_GARpt(ptAlgInst)							(ptAlgInst->ptAWB_GARPT)
#define ptAWB_GACenter(ptAlgInst)						(ptAlgInst->ptAWB_GACenter)
#define AWB_GA_INIT()										{ AWB_GA_FpInit(pbAWB_GetGAFpP()); }

//cal
#define swAWB_GA_CalCenterX(uwSX_0, uwSY_0, uwSY_1, uw_Slp, uw_Offset)						(uwSX_0 + (AWB_SXSYSLP_UNIT*(uwSY_1-uwSY_0)/uw_Slp + uw_Offset)/2)
#define swAWB_GA_CalCenterY(uwSY_0, uwSY_1)			((uwSY_0 + uwSY_1)/2)

//Get
	//Info
#define ulAWB_GetCurrY(ptAlgInst)							(ptAlgInst->xtAWBInfo.ulCurrY)
#define uwAWB_GetCX(ptAlgInst)								(ptAlgInst->xtAWBInfo.swCX_Curr)
#define uwAWB_GetCY(ptAlgInst)								(ptAlgInst->xtAWBInfo.swCY_Curr)
#define ubAWB_GetStable(ptAlgInst)							(ptAlgInst->xtAWBInfo.ubStable)
#define uwAWB_GetCurrGain_B(ptAlgInst)						(ptAlgInst->xtAWBInfo.uwCurrGain_B)
#define uwAWB_GetCurrGain_G(ptAlgInst)						(ptAlgInst->xtAWBInfo.uwCurrGain_G)
#define uwAWB_GetCurrGain_R(ptAlgInst)						(ptAlgInst->xtAWBInfo.uwCurrGain_R)
	//Ctrl
#define uwAWB_GetMinGain_B(ptAlgInst)						(ptAlgInst->xtAWBCtrl.uwMinGain_B) 
#define uwAWB_GetMaxGain_B(ptAlgInst)						(ptAlgInst->xtAWBCtrl.uwMaxGain_B) 
#define uwAWB_GetMinGain_G(ptAlgInst)						(ptAlgInst->xtAWBCtrl.uwMinGain_G) 
#define uwAWB_GetMaxGain_G(ptAlgInst)						(ptAlgInst->xtAWBCtrl.uwMaxGain_G) 
#define uwAWB_GetMinGain_R(ptAlgInst)						(ptAlgInst->xtAWBCtrl.uwMinGain_R) 
#define uwAWB_GetMaxGain_R(ptAlgInst)						(ptAlgInst->xtAWBCtrl.uwMaxGain_R) 
	// Table
#define ubAWB_GetInitCXCYTblRowSize(ptAlgInst)				(ptAlgInst->xtAWBInitCXCY.ubRowSize)
#define ubAWB_GetInitCXCYTblColSize(ptAlgInst)				(ptAlgInst->xtAWBInitCXCY.ubColSize)
#define ubAWB_GetCXCYTblRowSize(ptAlgInst)					(ptAlgInst->xtAWBCXCY.ubRowSize)
#define ubAWB_GetCXCYTblColSize(ptAlgInst)					(ptAlgInst->xtAWBCXCY.ubColSize)

//Set
	//Info
#define AWB_SetUpdate(ptAlgInst)							{ ptAlgInst->ubUpdate = 1; }
#define AWB_SetInitialFlag(ptAlgInst)						{ ptAlgInst->ubInitialFlag = 1; }
#define AWB_SetStable(ptAlgInst, ubSt)						{ ptAlgInst->xtAWBInfo.ubStable = ubSt; }
#define AWB_SetUnStable(ptAlgInst)							{ AWB_SetStable(ptAlgInst, AWB_UNSTABLE);ptAlgInst->ubUnStbCounter = 0; }
#define AWB_SetGainStatus_B(ptAlgInst, uwRptStatusB) 		{ ptAlgInst->xtAWBInfo.uwRptStatus_B = uwRptStatusB; }
#define AWB_SetGainStatus_G(ptAlgInst, uwRptStatusG) 		{ ptAlgInst->xtAWBInfo.uwRptStatus_G = uwRptStatusG; }
#define AWB_SetGainStatus_R(ptAlgInst, uwRptStatusR) 		{ ptAlgInst->xtAWBInfo.uwRptStatus_R = uwRptStatusR; }
#define AWB_SetCurrGain_B(ptAlgInst, uwCurrGainB)			{ ptAlgInst->xtAWBInfo.uwCurrGain_B = uwCurrGainB; }
#define AWB_SetCurrGain_G(ptAlgInst, uwCurrGainG)			{ ptAlgInst->xtAWBInfo.uwCurrGain_G = uwCurrGainG; }
#define AWB_SetCurrGain_R(ptAlgInst, uwCurrGainR)			{ ptAlgInst->xtAWBInfo.uwCurrGain_R = uwCurrGainR; }
#define AWB_SetCurrY(ptAlgInst, ulCurY)						{ ptAlgInst->xtAWBInfo.ulCurrY = ulCurY; }
#define AWB_SetAbsY(ptAlgInst, ulCurY, ulIdxGain)			{ ptAlgInst->xtAWBInfo.ulAbsY = ulCurY/ulIdxGain; }
#define AWB_SetOpenLoopRPT_B(ptAlgInst, ulSumB)				{ ptAlgInst->xtAWBInfo.xtOpenLoopRPT.ulBSum = ulSumB; }						
#define AWB_SetOpenLoopRPT_G(ptAlgInst, ulSumG)				{ ptAlgInst->xtAWBInfo.xtOpenLoopRPT.ulGSum = ulSumG; }						
#define AWB_SetOpenLoopRPT_R(ptAlgInst, ulSumR)				{ ptAlgInst->xtAWBInfo.xtOpenLoopRPT.ulRSum = ulSumR; }
#define AWB_SetOpenLoopRPT_Cnt(ptAlgInst, ulOpenLoopCnt)	{ ptAlgInst->xtAWBInfo.xtOpenLoopRPT.ulCount = ulOpenLoopCnt; }
#define AWB_SetCloseLoopRPT_B(ptAlgInst, ulSumB)			{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT.ulBSum = ulSumB; }						
#define AWB_SetCloseLoopRPT_G(ptAlgInst, ulSumG)			{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT.ulGSum = ulSumG; }						
#define AWB_SetCloseLoopRPT_R(ptAlgInst, ulSumR)			{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT.ulRSum = ulSumR; }						
#define AWB_SetCloseLoopRPT_Cnt(ptAlgInst, ulCloseLoopCnt)	{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT.ulCount = ulCloseLoopCnt; }						
#define AWB_SetCloseLoopRPT_CTW_B(ptAlgInst, ulSumB)		{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT_CTW.ulBSum = ulSumB; }						
#define AWB_SetCloseLoopRPT_CTW_G(ptAlgInst, ulSumG)		{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT_CTW.ulGSum = ulSumG; }						
#define AWB_SetCloseLoopRPT_CTW_R(ptAlgInst, ulSumR)		{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT_CTW.ulRSum = ulSumR; }
#define AWB_SetCloseLoopRPT_CTW_Cnt(ptAlgInst, ulCTWCnt)	{ ptAlgInst->xtAWBInfo.xtCloseLoopRPT_CTW.ulCount = ulCTWCnt; }
	//Ctrl	
#define AWB_SetMethod(ptAlgInst, ubAWB_Method)				{ ptAlgInst->xtAWBCtrl.ubMethod = ubAWB_Method; }
#define AWB_SetRptDelay(ptAlgInst, ubRptDly)				{ ptAlgInst->xtAWBCtrl.ubRptDelay = ubRptDly; }	
#define AWB_SetGAEn(ptAlgInst, ubEn)				{ ptAlgInst->xtAWBCtrl.ubGA_En = ubEn; }
#define AWB_SetGAMinCntThd(ptAlgInst, ulMinCntThd)				{ ptAlgInst->xtAWBCtrl.ulGA_MinCntThd = ulMinCntThd; }
#define AWB_SetGAPLKTarget(ptAlgInst, ubTarget)				{ ptAlgInst->xtAWBCtrl.ubPLK_Target = ubTarget; }
#define AWB_SetOpenLoopYMax(ptAlgInst, ubOpenLoop_YMax)		{ ptAlgInst->xtAWBCtrl.ubOpenLoopYMax = ubOpenLoop_YMax; }
#define AWB_SetOpenLoopYMin(ptAlgInst, ubOpenLoop_YMin)		{ ptAlgInst->xtAWBCtrl.ubOpenLoopYMin = ubOpenLoop_YMin; }
#define AWB_SetCloseLoopYMax(ptAlgInst, ubCloseLoop_YMax)	{ ptAlgInst->xtAWBCtrl.ubCloseLoopYMax = ubCloseLoop_YMax; }
#define AWB_SetCloseLoopYMin(ptAlgInst, ubCloseLoop_YMin)	{ ptAlgInst->xtAWBCtrl.ubCloseLoopYMin = ubCloseLoop_YMin; }
#define AWB_SetIMax(ptAlgInst, ubI_Max)						{ ptAlgInst->xtAWBCtrl.ubIMax = ubI_Max; }
#define AWB_SetQMax(ptAlgInst, ubQ_Max)						{ ptAlgInst->xtAWBCtrl.ubQMax = ubQ_Max; }
#define AWB_SetMinGain_B(ptAlgInst, uwMinGainB)				{ ptAlgInst->xtAWBCtrl.uwMinGain_B = uwMinGainB; }
#define AWB_SetMaxGain_B(ptAlgInst, uwMaxGainB) 			{ ptAlgInst->xtAWBCtrl.uwMaxGain_B = uwMaxGainB; }
#define AWB_SetMinGain_G(ptAlgInst, uwMinGainG) 			{ ptAlgInst->xtAWBCtrl.uwMinGain_G = uwMinGainG; }
#define AWB_SetMaxGain_G(ptAlgInst, uwMaxGainG) 			{ ptAlgInst->xtAWBCtrl.uwMaxGain_G = uwMaxGainG; }
#define AWB_SetMinGain_R(ptAlgInst, uwMinGainR) 			{ ptAlgInst->xtAWBCtrl.uwMinGain_R = uwMinGainR; }
#define AWB_SetMaxGain_R(ptAlgInst, uwMaxGainR) 			{ ptAlgInst->xtAWBCtrl.uwMaxGain_R = uwMaxGainR; }
#define AWB_SetGlobal_Weighting(ptAlgInst, ubGlobalWeighting)	{ ptAlgInst->xtAWBCtrl.ubGlobal_Weighting = ubGlobalWeighting; }
#define AWB_SetCTW_Weighting(ptAlgInst, ubCTWWeighting) 	{ ptAlgInst->xtAWBCtrl.ubCTW_Weighting = ubCTWWeighting; }
#define AWB_SetStbRng(ptAlgInst, ubStRng) 					{ ptAlgInst->xtAWBCtrl.ubStbRng = ubStRng; }
#define AWB_SetUnStbRng(ptAlgInst, ubUnStRng) 				{ ptAlgInst->xtAWBCtrl.ubUnStbRng = ubUnStRng; }
#define AWB_SetStbRng_Edge(ptAlgInst, ubStRng) 				{ ptAlgInst->xtAWBCtrl.ubStbRng_Edge = ubStRng; }
#define AWB_SetUnStbRng_Edge(ptAlgInst, ubUnStRng) 			{ ptAlgInst->xtAWBCtrl.ubUnStbRng_Edge = ubUnStRng; }
#define AWB_SetStbCntThd(ptAlgInst, ubStCntThd)				{ ptAlgInst->xtAWBCtrl.ubStbCntThd = ubStCntThd; } 
#define AWB_SetUnStbCntThd(ptAlgInst, ubUnStCntThd)			{ ptAlgInst->xtAWBCtrl.ubUnStbCntThd = ubUnStCntThd; }
#define AWB_SetP(ptAlgInst, ubP)							{ ptAlgInst->xtAWBCtrl.ubPID[0] = ubP; }
#define AWB_SetI(ptAlgInst, ubI)							{ ptAlgInst->xtAWBCtrl.ubPID[1] = ubI; }
#define AWB_SetD(ptAlgInst, ubD)							{ ptAlgInst->xtAWBCtrl.ubPID[2] = ubD; }
#define AWB_SetBase(ptAlgInst, ubB)							{ ptAlgInst->xtAWBCtrl.ubBase = ubB; }
#define AWB_SetCxyRoffset(ptAlgInst, ubOffset)				{ ptAlgInst->xtAWBCtrl.ubCxyRoffset = ubOffset; }
#define AWB_SetCxMinStep(ptAlgInst, ubMinStep)				{ ptAlgInst->xtAWBCtrl.ubCxMinStep = ubMinStep; }
#define AWB_SetCyMinStep(ptAlgInst, ubMinStep)				{ ptAlgInst->xtAWBCtrl.ubCyMinStep = ubMinStep; }
	// Table
#define AWB_SetInitCXCYTblRowSize(ptAlgInst, ubRowS)		{ ptAlgInst->xtAWBInitCXCY.ubRowSize = ubRowS; }
#define AWB_SetInitCXCYTblColSize(ptAlgInst, ubColS)		{ ptAlgInst->xtAWBInitCXCY.ubColSize = ubColS; }
#define AWB_SetCXCYTblRowSize(ptAlgInst, ubRowS)			{ ptAlgInst->xtAWBCXCY.ubRowSize = ubRowS; }
#define AWB_SetCXCYTblColSize(ptAlgInst, ubColS)			{ ptAlgInst->xtAWBCXCY.ubColSize = ubColS;}

#define AWB_SetLimGain(ptAlgInst)							{ if(ptAlgInst->xtAWBInfo.uwCurrGain_B > uwAWB_GetMaxGain_B(ptAlgInst))ptAlgInst->xtAWBInfo.uwCurrGain_B = uwAWB_GetMaxGain_B(ptAlgInst);\
																								if(ptAlgInst->xtAWBInfo.uwCurrGain_G > uwAWB_GetMaxGain_G(ptAlgInst))ptAlgInst->xtAWBInfo.uwCurrGain_G = uwAWB_GetMaxGain_G(ptAlgInst);\
																								if(ptAlgInst->xtAWBInfo.uwCurrGain_R > uwAWB_GetMaxGain_R(ptAlgInst))ptAlgInst->xtAWBInfo.uwCurrGain_R = uwAWB_GetMaxGain_R(ptAlgInst);\
																								if(ptAlgInst->xtAWBInfo.uwCurrGain_B < uwAWB_GetMinGain_B(ptAlgInst))ptAlgInst->xtAWBInfo.uwCurrGain_B = uwAWB_GetMinGain_B(ptAlgInst);\
																								if(ptAlgInst->xtAWBInfo.uwCurrGain_G < uwAWB_GetMinGain_G(ptAlgInst))ptAlgInst->xtAWBInfo.uwCurrGain_G = uwAWB_GetMinGain_G(ptAlgInst);\
																								if(ptAlgInst->xtAWBInfo.uwCurrGain_R < uwAWB_GetMinGain_R(ptAlgInst))ptAlgInst->xtAWBInfo.uwCurrGain_R = uwAWB_GetMinGain_R(ptAlgInst); }

//----------------------------------------------------------
//                                                         |
//                    FUNCTION PROTOTYPES                  |
//                                                         |
//----------------------------------------------------------
//Input function of AWB library
	//AWB control  
AWB_AlgObj* ptAWB_GetAlgInst(void);
UBYTE* pbAWB_GetVerID(void);
UBYTE AWB_RptCal_4G5(UWORD* uwHisBin, ULONG* ulCurrY);
#endif
