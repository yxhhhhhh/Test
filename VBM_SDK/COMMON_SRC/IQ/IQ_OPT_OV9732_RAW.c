/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IQ_OPT_OV9732_RAW.c
	\brief		OV9732 image quality relation function
	\author		
	\version	0.7
	\date		2018-05-11
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
//==========================================================
//                                                     	   |
//                       INCLUDE FILES                     |
//                                                         |
//==========================================================
#include <string.h>
#include "IQ_API.h"
#if (SEN_USE == SEN_OV9732)
#include "IQ_OPT_OV9732_RAW.H"

#if(IQ_DN_EN == 1 && AE_EN == 1 && AWB_EN == 1 && AF_EN == 1)
//==========================================================
//                                                         |
//                      GLOBAL VARIABLE                    |
//                                                         |
//==========================================================	
int32_t slIQ_LscLinear[11][0x40];				//(C0)LSC Dynamic IQ  AWB B GAIN
int32_t slIQ_SuppressLSC[11][0x40];
int32_t slIQ_AbsyLinear[11][0x40];				//(C2)Absy Mode Dynamic IQ
int32_t slIQ_CcmLinear[11][0x40];				//CCM Dynamic IQ  AWB B GAIN
int32_t slIQ_CcmLowLightLinear[11][0x40];		//CCM Low Light Color Suppress IQ  AE Cur Gain
//Day NR Dynamic IQ  AWB B GAIN
int32_t slIQ_DayNrRes1Linear[18][0xFF];
int32_t slIQ_DayNrRes2Linear[18][0xFF];
int32_t slIQ_DayNrRes3Linear[18][0xFF];			
//Night NR Dynamic IQ  AWB B GAIN
int32_t slIQ_NightNrRes1Linear[18][0xFF];
int32_t slIQ_NightNrRes2Linear[18][0xFF];			
int32_t slIQ_NightNrRes3Linear[18][0xFF];
//Res IQ Path1
int32_t slIQ_Path1Res1[11][0x40];
int32_t slIQ_Path1Res2[11][0x40];
int32_t slIQ_Path1Res3[11][0x40];
//Res IQ Path2
int32_t slIQ_Path2Res1[11][0x40];				
int32_t slIQ_Path2Res2[11][0x40];
int32_t slIQ_Path2Res3[11][0x40];	
//Res IQ Path3
int32_t slIQ_Path3Res1[11][0x40];				
int32_t slIQ_Path3Res2[11][0x40];
int32_t slIQ_Path3Res3[11][0x40];	

//------------------------------------------------------------------------------
void IQ_DynamicInit_SDK(void) {
	xtIQDnInst.xtIQJudgeInst.uwAWB_BGainX128	= 128;	
	xtIQDnInst.xtIQJudgeInst.uwAWB_GGainX128	= 128;	
	xtIQDnInst.xtIQJudgeInst.uwAWB_RGainX128	= 128;
	xtIQDnInst.xtIQJudgeInst.ubAE_Expidx		= 4;
	xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain		= 64;
	xtIQDnInst.xtIQJudgeInst.ulAE_AbsY			= 0;
	xtIQDnInst.xtIQJudgeInst.ulAF_W0Sum			= 0;
	xtIQDnInst.xtIQJudgeInst.ulAF_W1Sum			= 0;	

	IQ_SetLscLinearInf((void *)slIQ_LscLinear, (sizeof(slIQ_LscLinear) / sizeof(slIQ_LscLinear[0])), sizeof(slIQ_LscLinear[0]) / sizeof(int32_t));
	IQ_SetAbsyLinearInf((void *)slIQ_AbsyLinear, (sizeof(slIQ_AbsyLinear) / sizeof(slIQ_AbsyLinear[0])), sizeof(slIQ_AbsyLinear[0]) / sizeof(int32_t));
	IQ_SetDayNrRes1Inf((void *)slIQ_DayNrRes1Linear, (sizeof(slIQ_DayNrRes1Linear) / sizeof (slIQ_DayNrRes1Linear[0])), sizeof(slIQ_DayNrRes1Linear[0]) / sizeof(int32_t));
	IQ_SetDayNrRes2Inf((void *)slIQ_DayNrRes2Linear, (sizeof(slIQ_DayNrRes2Linear) / sizeof (slIQ_DayNrRes2Linear[0])), sizeof(slIQ_DayNrRes2Linear[0]) / sizeof(int32_t));
	IQ_SetDayNrRes3Inf((void *)slIQ_DayNrRes3Linear, (sizeof(slIQ_DayNrRes3Linear) / sizeof (slIQ_DayNrRes3Linear[0])), sizeof(slIQ_DayNrRes3Linear[0]) / sizeof(int32_t));
	IQ_SetNightNrRes1Inf((void *)slIQ_NightNrRes1Linear, (sizeof(slIQ_NightNrRes1Linear) / sizeof(slIQ_NightNrRes1Linear[0])), sizeof(slIQ_NightNrRes1Linear[0]) / sizeof(int32_t));
	IQ_SetNightNrRes2Inf((void *)slIQ_NightNrRes2Linear, (sizeof(slIQ_NightNrRes2Linear) / sizeof(slIQ_NightNrRes2Linear[0])), sizeof(slIQ_NightNrRes2Linear[0]) / sizeof(int32_t));
	IQ_SetNightNrRes3Inf((void *)slIQ_NightNrRes3Linear, (sizeof(slIQ_NightNrRes3Linear) / sizeof(slIQ_NightNrRes3Linear[0])), sizeof(slIQ_NightNrRes3Linear[0]) / sizeof(int32_t));
	IQ_SetCcmLinearInf((void *)slIQ_CcmLinear, (sizeof(slIQ_CcmLinear) / sizeof(slIQ_CcmLinear[0])), sizeof(slIQ_CcmLinear[0]) / sizeof(int32_t));
	IQ_SetCcmLowLightLinearInf((void *)slIQ_CcmLowLightLinear, (sizeof(slIQ_CcmLowLightLinear) / sizeof(slIQ_CcmLowLightLinear[0])), sizeof(slIQ_CcmLowLightLinear[0]) / sizeof(int32_t));   
    // Path1
	IQ_SetPath1Res1Inf((void *)slIQ_Path1Res1, (sizeof(slIQ_Path1Res1) / sizeof (slIQ_Path1Res1[0])), sizeof(slIQ_Path1Res1[0]) / sizeof(int32_t));
	IQ_SetPath1Res2Inf((void *)slIQ_Path1Res2, (sizeof(slIQ_Path1Res2) / sizeof (slIQ_Path1Res2[0])), sizeof(slIQ_Path1Res2[0]) / sizeof(int32_t));
	IQ_SetPath1Res3Inf((void *)slIQ_Path1Res3, (sizeof(slIQ_Path1Res3) / sizeof (slIQ_Path1Res3[0])), sizeof(slIQ_Path1Res3[0]) / sizeof(int32_t));
    // Path2
    IQ_SetPath2Res1Inf((void *)slIQ_Path2Res1, (sizeof(slIQ_Path2Res1) / sizeof (slIQ_Path2Res1[0])), sizeof(slIQ_Path2Res1[0]) / sizeof(int32_t));
	IQ_SetPath2Res2Inf((void *)slIQ_Path2Res2, (sizeof(slIQ_Path2Res2) / sizeof (slIQ_Path2Res2[0])), sizeof(slIQ_Path2Res2[0]) / sizeof(int32_t));
	IQ_SetPath2Res3Inf((void *)slIQ_Path2Res3, (sizeof(slIQ_Path2Res3) / sizeof (slIQ_Path2Res3[0])), sizeof(slIQ_Path2Res3[0]) / sizeof(int32_t));
    // Path3
    IQ_SetPath3Res1Inf((void *)slIQ_Path3Res1, (sizeof(slIQ_Path3Res1) / sizeof (slIQ_Path3Res1[0])), sizeof(slIQ_Path3Res1[0]) / sizeof(int32_t));
	IQ_SetPath3Res2Inf((void *)slIQ_Path3Res2, (sizeof(slIQ_Path3Res2) / sizeof (slIQ_Path3Res2[0])), sizeof(slIQ_Path3Res2[0]) / sizeof(int32_t));
	IQ_SetPath3Res3Inf((void *)slIQ_Path3Res3, (sizeof(slIQ_Path3Res3) / sizeof (slIQ_Path3Res3[0])), sizeof(slIQ_Path3Res3[0]) / sizeof(int32_t));
}

//------------------------------------------------------------------------------
void IQ_SuppressLscGain(void)
{
	uint8_t i,j;
	int32_t slLSC_TempValue;
    static uint32_t ulSuppressGainOld = 128;
    
    if(ulIQ_GetSuppressGain() == ulSuppressGainOld)
        return;
    
    //
    memcpy(&slIQ_LscLinear, &slIQ_SuppressLSC, sizeof(slIQ_LscLinear));
    //   
    for (i = 0; i < slIQ_LscLinear[0][0]; i ++)
    {
        if((slIQ_LscLinear[1][2+i] == 0x1644) || (slIQ_LscLinear[1][2+i] == 0x1648))
        {
            for(j=0; j<slIQ_LscLinear[0][1]; j++)
            {
                if(slIQ_LscLinear[2][2+i] == 0x000007FF)
                {
                    slLSC_TempValue = (slIQ_LscLinear[3+j][2+i] * ulIQ_GetSuppressGain()) / 128;
                    slIQ_LscLinear[3+j][2+i] = slLSC_TempValue;                
                }
                else if(slIQ_LscLinear[2][2+i] == 0x07FF0000)
                {
                    slLSC_TempValue = (((slIQ_LscLinear[3+j][2+i])>>16)  * ulIQ_GetSuppressGain()) / 128;
                    slIQ_LscLinear[3+j][2+i] = slLSC_TempValue <<16;
                }
            }
        }    
    }
    //
    ulSuppressGainOld = ulIQ_GetSuppressGain();    
}

//------------------------------------------------------------------------------
void IQ_DynamicLSC_4G5(void) {
    static uint8_t ubFirstFg = 0;
	uint8_t i;
	int32_t slLSC_Temp[0x40];
	
    if(ubFirstFg == 0)
    {
        //protect LSC linear data
        memcpy(&slIQ_SuppressLSC, &slIQ_LscLinear, sizeof(slIQ_LscLinear));
        ubFirstFg = 1;
    }
    IQ_SuppressLscGain();
    
	IQ_DynamicLinearInterpolation(xtIQDnInst.xtIQJudgeInst.uwAWB_BGainX128, slIQ_LscLinear[0][1], sizeof(slIQ_LscLinear[0]) / sizeof(int32_t), &slIQ_LscLinear[3][0], slLSC_Temp);
   
	for (i = 0; i < slIQ_LscLinear[0][0]; i ++) {
		IQ_SetBinVal(slIQ_LscLinear[0][0 + i + 2], slIQ_LscLinear[1][0 + i + 2], slIQ_LscLinear[2][0 + i + 2], slLSC_Temp[i]); 
	}
}

//------------------------------------------------------------------------------
void IQ_DynamicCCM_4G5(void) {
	int32_t 	slCCM_Temp[0x40], slCCM_Temp2[0x40];
	uint32_t 	ulData[6];	
	uint16_t 	ulAeIdxGainTmp;    
	int16_t 	slTemp;	
	uint8_t 	i,shift[6];
	uint8_t 	ccmspr = 0x40;
    	
	ulAeIdxGainTmp = (uint16_t)(xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain * (uint16_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx);
    
	IQ_DynamicLinearInterpolation(xtIQDnInst.xtIQJudgeInst.uwAWB_BGainX128, slIQ_CcmLinear[0][1], sizeof(slIQ_CcmLinear[0]) / sizeof(int32_t), &slIQ_CcmLinear[3][0], slCCM_Temp);
	
	IQ_DynamicLinearInterpolation(ulAeIdxGainTmp, slIQ_CcmLowLightLinear[0][1], sizeof(slIQ_CcmLowLightLinear[0]) / sizeof(int32_t), &slIQ_CcmLowLightLinear[3][0], slCCM_Temp2);
	
	for (i = 0; i < 6; i ++) {       
		ulData[i] = slCCM_Temp[i];
        
		shift[i] = (slIQ_CcmLinear[2][i + 2] & 0x000001FF) ? (0) : (16);
		ulData[i] >>= shift[i];
		slTemp  = (ulData[i] & (0x1 << 8)) ? (ulData[i] - 512) : (ulData[i]);
		slCCM_Temp[i] = ((int32_t)slTemp * ccmspr) >> 6;	
	}

	slCCM_Temp[2] = -(slCCM_Temp[0] + slCCM_Temp[1]);
	slCCM_Temp[3] = -(slCCM_Temp[4] + slCCM_Temp[5]);

	for (i = 0; i < 6; i ++) {
		slCCM_Temp[i] = slCCM_Temp[i] << shift[i];
	}

	for (i = 0; i < (slIQ_CcmLinear[0][0]); i ++) {
		IQ_SetBinVal(slIQ_CcmLinear[0][0 + i + 2], slIQ_CcmLinear[1][0 + i + 2], slIQ_CcmLinear[2][0 + i + 2], slCCM_Temp[i]); 
	}
	
	for (i = 0; i < (slIQ_CcmLowLightLinear[0][0]); i ++) {
		IQ_SetBinVal(slIQ_CcmLowLightLinear[0][0 + i + 2], slIQ_CcmLowLightLinear[1][0 + i + 2], slIQ_CcmLowLightLinear[2][0 + i + 2], slCCM_Temp2[i]); 
	}	
}

//------------------------------------------------------------------------------
void IQ_DynamicDenoise_4G5(void) {
	uint8_t i;
	int32_t slNrLinearTemp[0x100];

    //switch day or night mode by IR cut module    
	if (ubSEN_GetIrMode() == 0)
    {
        //Day mode        
        if(slIQ_DayNrRes1Linear[1][0] == xtIQDnInst.uwCurrPrevHSz)
        {
            IQ_DynamicLinearInterpolation((uint32_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx * xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain, slIQ_DayNrRes1Linear[0][1], sizeof(slIQ_DayNrRes1Linear[0]) / sizeof(int32_t), &slIQ_DayNrRes1Linear[3][0], slNrLinearTemp);

            for (i = 0; i < (slIQ_DayNrRes1Linear[0][0]); i ++) {
                IQ_SetBinVal(slIQ_DayNrRes1Linear[0][0 + i + 2], slIQ_DayNrRes1Linear[1][0 + i + 2], slIQ_DayNrRes1Linear[2][0 + i + 2], slNrLinearTemp[i]); 
            }
        }
        if(slIQ_DayNrRes2Linear[1][0] == xtIQDnInst.uwCurrPrevHSz)
        {
            IQ_DynamicLinearInterpolation((uint32_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx * xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain, slIQ_DayNrRes2Linear[0][1], sizeof(slIQ_DayNrRes2Linear[0]) / sizeof(int32_t), &slIQ_DayNrRes2Linear[3][0], slNrLinearTemp);

            for (i = 0; i < (slIQ_DayNrRes2Linear[0][0]); i ++) {
                IQ_SetBinVal(slIQ_DayNrRes2Linear[0][0 + i + 2], slIQ_DayNrRes2Linear[1][0 + i + 2], slIQ_DayNrRes2Linear[2][0 + i + 2], slNrLinearTemp[i]); 
            }
        }
        if(slIQ_DayNrRes3Linear[1][0] == xtIQDnInst.uwCurrPrevHSz)
        {
            IQ_DynamicLinearInterpolation((uint32_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx * xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain, slIQ_DayNrRes3Linear[0][1], sizeof(slIQ_DayNrRes3Linear[0]) / sizeof(int32_t), &slIQ_DayNrRes3Linear[3][0], slNrLinearTemp);

            for (i = 0; i < (slIQ_DayNrRes3Linear[0][0]); i ++) {
                IQ_SetBinVal(slIQ_DayNrRes3Linear[0][0 + i + 2], slIQ_DayNrRes3Linear[1][0 + i + 2], slIQ_DayNrRes3Linear[2][0 + i + 2], slNrLinearTemp[i]); 
            }
        }
    }else if (ubSEN_GetIrMode() == 1){
        //Night mode
        if(slIQ_NightNrRes1Linear[1][0] == xtIQDnInst.uwCurrPrevHSz)
        {
            IQ_DynamicLinearInterpolation((uint32_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx * xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain, slIQ_NightNrRes1Linear[0][1], sizeof(slIQ_NightNrRes1Linear[0]) / sizeof(int32_t), &slIQ_NightNrRes1Linear[3][0], slNrLinearTemp);

            for (i = 0; i < (slIQ_NightNrRes1Linear[0][0]); i ++) {
                IQ_SetBinVal(slIQ_NightNrRes1Linear[0][0 + i + 2], slIQ_NightNrRes1Linear[1][0 + i + 2], slIQ_NightNrRes1Linear[2][0 + i + 2], slNrLinearTemp[i]); 
            }
        }
        if(slIQ_NightNrRes2Linear[1][0] == xtIQDnInst.uwCurrPrevHSz)
        {
            IQ_DynamicLinearInterpolation((uint32_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx * xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain, slIQ_NightNrRes2Linear[0][1], sizeof(slIQ_NightNrRes2Linear[0]) / sizeof(int32_t), &slIQ_NightNrRes2Linear[3][0], slNrLinearTemp);

            for (i = 0; i < (slIQ_NightNrRes2Linear[0][0]); i ++) {
                IQ_SetBinVal(slIQ_NightNrRes2Linear[0][0 + i + 2], slIQ_NightNrRes2Linear[1][0 + i + 2], slIQ_NightNrRes2Linear[2][0 + i + 2], slNrLinearTemp[i]); 
            }
        }
        if(slIQ_NightNrRes3Linear[1][0] == xtIQDnInst.uwCurrPrevHSz)
        {
            IQ_DynamicLinearInterpolation((uint32_t)xtIQDnInst.xtIQJudgeInst.ubAE_Expidx * xtIQDnInst.xtIQJudgeInst.uwAE_CurrGain, slIQ_NightNrRes3Linear[0][1], sizeof(slIQ_NightNrRes3Linear[0]) / sizeof(int32_t), &slIQ_NightNrRes3Linear[3][0], slNrLinearTemp);

            for (i = 0; i < (slIQ_NightNrRes3Linear[0][0]); i ++) {
                IQ_SetBinVal(slIQ_NightNrRes3Linear[0][0 + i + 2], slIQ_NightNrRes3Linear[1][0 + i + 2], slIQ_NightNrRes3Linear[2][0 + i + 2], slNrLinearTemp[i]); 
            }
        }
    }
}
#endif

//------------------------------------------------------------------------------
void IQ_SetResolution_SDK(uint16_t uwWidth, uint16_t uwHeight, uint8_t ubFps, uint8_t ubPath)
{
	int32_t slResTemp[0x40];
	uint8_t i;
	if (ubPath == ISP_PATH1) {
        
        if ((uwWidth == slIQ_Path1Res1[1][0]) && (uwHeight == slIQ_Path1Res1[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path1Res1[0][1], sizeof(slIQ_Path1Res1[0]) / sizeof(int32_t), &slIQ_Path1Res1[3][0], slResTemp);
			
			for (i = 0; i < (slIQ_Path1Res1[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path1Res1[0][0 + i + 2], slIQ_Path1Res1[1][0 + i + 2], slIQ_Path1Res1[2][0 + i + 2], slResTemp[i]); 
			}			
		}
        else if ((uwWidth == slIQ_Path1Res2[1][0]) && (uwHeight == slIQ_Path1Res2[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path1Res2[0][1], sizeof(slIQ_Path1Res2[0]) / sizeof(int32_t), &slIQ_Path1Res2[3][0], slResTemp);
            
			for (i = 0; i < (slIQ_Path1Res2[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path1Res2[0][0 + i + 2], slIQ_Path1Res2[1][0 + i + 2], slIQ_Path1Res2[2][0 + i + 2], slResTemp[i]); 
			}			
		}        
        else if ((uwWidth == slIQ_Path1Res3[1][0]) && (uwHeight == slIQ_Path1Res3[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path1Res3[0][1], sizeof(slIQ_Path1Res3[0]) / sizeof(int32_t), &slIQ_Path1Res3[3][0], slResTemp);

			for (i = 0; i < (slIQ_Path1Res3[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path1Res3[0][0 + i + 2], slIQ_Path1Res3[1][0 + i + 2], slIQ_Path1Res3[2][0 + i + 2], slResTemp[i]); 
			}
		}
	} else if (ubPath == ISP_PATH2) {
        if ((uwWidth == slIQ_Path2Res1[1][0]) && (uwHeight == slIQ_Path2Res1[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path2Res1[0][1], sizeof(slIQ_Path2Res1[0]) / sizeof(int32_t), &slIQ_Path2Res1[3][0], slResTemp);
			
			for (i = 0; i < (slIQ_Path2Res1[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path2Res1[0][0 + i + 2], slIQ_Path2Res1[1][0 + i + 2], slIQ_Path2Res1[2][0 + i + 2], slResTemp[i]); 
			}
		}	               
		else if ((uwWidth == slIQ_Path2Res2[1][0]) && (uwHeight == slIQ_Path2Res2[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path2Res2[0][1], sizeof(slIQ_Path2Res2[0]) / sizeof(int32_t), &slIQ_Path2Res2[3][0], slResTemp);
            
			for (i = 0; i < (slIQ_Path2Res2[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path2Res2[0][0 + i + 2], slIQ_Path2Res2[1][0 + i + 2], slIQ_Path2Res2[2][0 + i + 2], slResTemp[i]); 
			}
		}
        else if ((uwWidth == slIQ_Path2Res3[1][0]) && (uwHeight == slIQ_Path2Res3[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path2Res3[0][1], sizeof(slIQ_Path2Res3[0]) / sizeof(int32_t), &slIQ_Path2Res3[3][0], slResTemp);

			for (i = 0; i < (slIQ_Path2Res3[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path2Res3[0][0 + i + 2], slIQ_Path2Res3[1][0 + i + 2], slIQ_Path2Res3[2][0 + i + 2], slResTemp[i]); 
			}
		}         
	} else if (ubPath == ISP_PATH3) {
        if ((uwWidth == slIQ_Path3Res1[1][0]) && (uwHeight == slIQ_Path3Res1[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path3Res1[0][1], sizeof(slIQ_Path3Res1[0]) / sizeof(int32_t), &slIQ_Path3Res1[3][0], slResTemp);
			
			for (i = 0; i < (slIQ_Path3Res1[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path3Res1[0][0 + i + 2], slIQ_Path3Res1[1][0 + i + 2], slIQ_Path3Res1[2][0 + i + 2], slResTemp[i]); 
			}
		}	               
		else if ((uwWidth == slIQ_Path3Res2[1][0]) && (uwHeight == slIQ_Path3Res2[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path3Res2[0][1], sizeof(slIQ_Path3Res2[0]) / sizeof(int32_t), &slIQ_Path3Res2[3][0], slResTemp);
            
			for (i = 0; i < (slIQ_Path3Res2[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path3Res2[0][0 + i + 2], slIQ_Path3Res2[1][0 + i + 2], slIQ_Path3Res2[2][0 + i + 2], slResTemp[i]); 
			}
		}
        else if ((uwWidth == slIQ_Path3Res3[1][0]) && (uwHeight == slIQ_Path3Res3[1][1]))
        {
			IQ_DynamicLinearInterpolation(ubFps, slIQ_Path3Res3[0][1], sizeof(slIQ_Path3Res3[0]) / sizeof(int32_t), &slIQ_Path3Res3[3][0], slResTemp);

			for (i = 0; i < (slIQ_Path3Res3[0][0]); i ++) {
				IQ_SetBinVal(slIQ_Path3Res3[0][0 + i + 2], slIQ_Path3Res3[1][0 + i + 2], slIQ_Path3Res3[2][0 + i + 2], slResTemp[i]); 
			}
		}         
	}
}

//------------------------------------------------------------------------------
void IQ_DYNAMIC_SDK(void) { 	
	IQ_DynamicLSC_4G5();
	IQ_DynamicCCM_4G5();
	IQ_DynamicDenoise_4G5();
}

#endif
