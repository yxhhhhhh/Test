/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ADO_API.h
	\brief		Audio header file
	\author		Chinwei Hsu/Brouce Hsu
	\version	2.19
	\date		2018/05/17
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _AUDIO_API_H_
#define _AUDIO_API_H_

#include "_510PF.h"

#define ADO_AUDIO32 	1
#define ADO_AUDIO32_MAX_NUM 	7

extern osMessageQId tADO_EncodeQueue;       //!< Audio event queue for external
extern osMessageQId tADO_EncodeQueueHandle;
extern osMessageQId tADO_DecodeQueueHandle;

// Audio function switch
typedef enum 
{
	ADO_OFF = 0,    //!< Audio register switch : off
	ADO_ON          //!< Audio register switch : on
}ADO_FUN_SWITCH;
//------------------------------------------------------------------------------
typedef enum 
{
	NONE,
	AUDIO32,
	AAC
}ADO_ENCODE_TYPE;
//------------------------------------------------------------------------------
typedef struct ADO_Queue_Info
{
	ADO_ENCODE_TYPE EncType;
	uint32_t PcmAddr;
	uint32_t PcmSize;
	uint32_t Audio32Addr;
	uint32_t Audio32Size;
	uint32_t AACAddr;
	uint32_t AACSize;
	uint8_t HighPriority;
}ADO_Queue_INFO;
//------------------------------------------------------------------------------
typedef struct ADO_Aud32_Enc_Info
{
	uint32_t ulOutputSize;
	uint32_t ulRemainStartAddr;
	uint32_t ulRemainSize;
}ADO_AUD32_ENC_INFO;
//------------------------------------------------------------------------------
typedef struct ADO_Aud32_Dec_Info
{
	uint32_t ulResultSize;
}ADO_AUD32_DEC_INFO;
//------------------------------------------------------------------------------
typedef enum 
{
    DAC_RDY,
	PLAY_BUF_EMP	
}ADO_DEC_EVENT;
//------------------------------------------------------------------------------
typedef enum 
{
	NOISE_DISABLE,
	NOISE_AEC,
	NOISE_NR,
	NOISE_ALL
}ADO_NOISE_PROCESS_TYPE;
//------------------------------------------------------------------------------
typedef enum 
{
	ADO_SUCCESS,
	ADO_FAIL
}ADO_RETURN_FLAG;
//------------------------------------------------------------------------------
// System speed mode
typedef enum
{
	HIGH_SPEED = 0, //!< ST53510 system speed : high(DDR mode)
	LOW_POWER       //!< ST53510 system speed : low(SRAM mode)
}ADO_SYS_SPEED_MODE_t;
//------------------------------------------------------------------------------
// Audio ADC device
typedef enum
{
	SIG_DEL_ADC = 0,    //!< Sigma-delta ADC
	I2S_ADC,            //!< I2S ADC
}ADO_ADC_DEV_t;
//------------------------------------------------------------------------------
// Audio DAC device
typedef enum
{
	R2R_DAC = 0,    //!< R2R DAC
	I2S_DAC         //!< I2S DAC
}ADO_DAC_DEV_t;
//------------------------------------------------------------------------------
// Audio unsigned/signed signal
typedef enum
{
	UNSIGNED = 0,   //!< Unsigned signal
	SIGNED          //!< Signed signal
}ADO_SIGNAL_MODE;
//------------------------------------------------------------------------------
// Audio channel : Stereo/Mono
typedef enum
{
	STEREO = 0, //!< Stereo channel
	MONO        //!< Mono channel
}ADO_CHANNEL_MODE;
//------------------------------------------------------------------------------
// Audio sample size : 16/8-bit
typedef enum
{
	SAMPLESIZE_16_BIT = 0,  //!< 16-bit sample size
	SAMPLESIZE_8_BIT        //!< 8-bit sample size
}ADO_SAMPLESIZE;
//------------------------------------------------------------------------------
// Audio sample rate
typedef enum
{
	SAMPLERATE_8kHZ = 0,    //!< 8kHz sample rate
	SAMPLERATE_16kHZ,       //!< 16kHz sample rate
	SAMPLERATE_32kHZ,       //!< 32kHz sample rate
	SAMPLERATE_48kHZ,       //!< 48kHz sample rate
	SAMPLERATE_11_025kHZ,   //!< 11.025kHz sample rate
	SAMPLERATE_44_1kHZ      //!< 44.1kHz sample rate
}ADO_SAMPLERATE;
//------------------------------------------------------------------------------
// Audio format
typedef struct AUDIO_FORMAT
{
	ADO_SIGNAL_MODE	  sign_flag;        //!< Unsigned/Signed signal
	ADO_CHANNEL_MODE	channel;        //!< Stereo/Mono channel
	ADO_SAMPLESIZE  	sample_size;    //!< 16/8-bit sample size
	ADO_SAMPLERATE  	sample_rate;    //!< Sample rate
}ADO_FORMAT_t;
//------------------------------------------------------------------------------
// Audio compress function
typedef enum
{
	COMPRESS_NONE = 0,  //!< No compress audio
	COMPRESS_ALAW,      //!< A-law compression
	COMPRESS_MSADPCM,   //!< ADPCM compression
}ADO_COMPRESS_MODE;
//------------------------------------------------------------------------------
// Audio adpcm step size
typedef enum
{
	STEP_16 = 0,    //!< ADPCM minimum step size : 16
	STEP_1          //!< ADPCM minimum step size : 1
}ADO_ADPCM_STEP_SIZE;
//------------------------------------------------------------------------------
// Audio adpcm function
typedef struct AUDIO_ADPCM_FUNCTION
{
	ADO_FUN_SWITCH      extra_header;   //!< ADPCM extra header switch
	ADO_ADPCM_STEP_SIZE step_size;      //!< ADPCM step size
	uint16_t            enc_smpl_num;   //!< ADPCM encode sample number
	uint16_t            dec_byte_num;   //!< ADPCM decode byte number
}ADO_ADPCM_t;
//------------------------------------------------------------------------------
// Audio buffer size
typedef enum
{
	BUF_SIZE_512B = 0,  //!< 512Bytes buffer size
	BUF_SIZE_1KB,       //!< 1KB buffer size
	BUF_SIZE_2KB,       //!< 2KB buffer size
	BUF_SIZE_4KB,       //!< 4KB buffer size
	BUF_SIZE_8KB,       //!< 8KB buffer size
	BUF_SIZE_16KB,      //!< 16KB buffer size
	BUF_SIZE_32KB,      //!< 32KB buffer size
	BUF_SIZE_64KB,      //!< 64KB buffer size
}ADO_BUFFERSIZE;
//------------------------------------------------------------------------------
// Audio buffer threshold
typedef enum
{
//	BUF_TH_256B = 0,    //!< 256Bytes buffer threshold
//	BUF_TH_512B = 1,    //!< 512Bytes buffer threshold
	BUF_TH_1KB = 2,     //!< 1KB buffer threshold
	BUF_TH_2KB,         //!< 2KB buffer threshold
	BUF_TH_4KB,         //!< 4KB buffer threshold
	BUF_TH_8KB,         //!< 8KB buffer threshold
	BUF_TH_16KB,        //!< 16KB buffer threshold
	BUF_TH_32KB         //!< 32KB buffer threshold
}ADO_BUFFERTH;
//------------------------------------------------------------------------------// Audio ADC Gain
typedef enum
{
	ADC_GAIN_0DB = 0,   //!< ADC gain : 0dB
	ADC_GAIN_0p5DB,     //!< ADC gain : 0.5dB
	ADC_GAIN_1DB,       //!< ADC gain : 1dB
	ADC_GAIN_1p5DB,     //!< ADC gain : 1.5dB
	ADC_GAIN_2DB,       //!< ADC gain : 2dB
	ADC_GAIN_2p5DB,     //!< ADC gain : 2.5dB
	ADC_GAIN_3DB,       //!< ADC gain : 3dB
	ADC_GAIN_3p5DB,     //!< ADC gain : 3.5dB
	ADC_GAIN_4DB,       //!< ADC gain : 4dB
	ADC_GAIN_4p5DB,     //!< ADC gain : 4.5dB
	ADC_GAIN_5DB,       //!< ADC gain : 5dB
	ADC_GAIN_5p5DB,     //!< ADC gain : 5.5dB
	ADC_GAIN_6DB,       //!< ADC gain : 6dB
	ADC_GAIN_6p5DB,     //!< ADC gain : 6.5dB
	ADC_GAIN_7DB,       //!< ADC gain : 7dB
	ADC_GAIN_7p5DB,     //!< ADC gain : 7.5dB
	ADC_GAIN_8DB,       //!< ADC gain : 8dB
	ADC_GAIN_8p5DB,     //!< ADC gain : 8.5dB
	ADC_GAIN_9DB,       //!< ADC gain : 9dB
	ADC_GAIN_9p5DB,     //!< ADC gain : 9.5dB
	ADC_GAIN_10DB       //!< ADC gain : 10dB
}ADO_ADCGAIN;
//------------------------------------------------------------------------------
// Audio ADC mute speed
typedef enum
{
	ADC_MS_3DB_4SAMPLE = 0, //!< ADC mute speed : amplitude -3dB every 4 sample
	ADC_MS_3DB_8SAMPLE,     //!< ADC mute speed : amplitude -3dB every 8 sample
	ADC_MS_3DB_16SAMPLE,    //!< ADC mute speed : amplitude -3dB every 16 sample
	ADC_MS_3DB_32SAMPLE     //!< ADC mute speed : amplitude -3dB every 32 sample
}ADO_ADCMUTESPEED;
//------------------------------------------------------------------------------
// Audio DAC rmp rate
typedef enum
{
	DAC_MR_0p5DB_1SAMPLE = 0,   //!< DAC mute speed : 0.5 dB per 1 sample
	DAC_MR_0p5DB_2SAMPLE,       //!< DAC mute speed : 0.5 dB per 2 sample
	DAC_MR_0p5DB_4SAMPLE,       //!< DAC mute speed : 0.5 dB per 4 sample
	DAC_MR_0p5DB_8SAMPLE        //!< DAC mute speed : 0.5 dB per 8 sample
}AUDIO_DACMUTERMP;
//------------------------------------------------------------------------------
// Audio R2R volume
typedef enum 
{
	R2R_VOL_n45DB = 0,  //!< R2R volume : -45dB
	R2R_VOL_n42DB,      //!< R2R volume : -42dB
	R2R_VOL_n39p1DB,    //!< R2R volume : -39.1dB
	R2R_VOL_n36DB,      //!< R2R volume : -36dB
	R2R_VOL_n32p4DB,    //!< R2R volume : -32.4dB
	R2R_VOL_n29p8DB,    //!< R2R volume : -29.8dB
	R2R_VOL_n26p2DB,    //!< R2R volume : -26.2dB
	R2R_VOL_n23p5DB,    //!< R2R volume : -23.5dB
	R2R_VOL_n21p4DB,    //!< R2R volume : -21.4dB
	R2R_VOL_n18p2DB,    //!< R2R volume : -18.2dB
	R2R_VOL_n14p6DB,    //!< R2R volume : -14.6dB
	R2R_VOL_n11p9DB,    //!< R2R volume : -11.9dB
	R2R_VOL_n8p2DB,     //!< R2R volume : -8.2dB
	R2R_VOL_n5p6DB,     //!< R2R volume : -5.6dB
	R2R_VOL_n3DB,       //!< R2R volume : -3dB
	R2R_VOL_n0DB        //!< R2R volume : -0dB
}ADO_R2R_VOL;
//------------------------------------------------------------------------------
// Audio DAC Gain
typedef enum 
{
	DAC_GAIN_n0DB = 0,  //!< DAC gain : -0dB
	DAC_GAIN_n2p5DB,    //!< DAC gain : -2.5dB
	DAC_GAIN_n6DB,      //!< DAC gain : -6dB
	DAC_GAIN_n8p5DB,    //!< DAC gain : -8.5dB
	DAC_GAIN_n12DB,     //!< DAC gain : -12dB
	DAC_GAIN_n14DB,     //!< DAC gain : -14dB
	DAC_GAIN_n18DB,     //!< DAC gain : -18dB
	DAC_GAIN_n20DB,     //!< DAC gain : -20dB
	DAC_GAIN_n24DB,     //!< DAC gain : -24dB
	DAC_GAIN_n26p6DB,   //!< DAC gain : -26.6dB
	DAC_GAIN_n30p1DB,   //!< DAC gain : -30.1dB
	DAC_GAIN_n32p6DB,   //!< DAC gain : -32.6dB
	DAC_GAIN_n36p1DB,   //!< DAC gain : -36.1dB
	DAC_GAIN_n38p6DB,   //!< DAC gain : -38.6dB
	DAC_GAIN_n42p1DB,   //!< DAC gain : -42.1dB
	DAC_GAIN_n44p6DB    //!< DAC gain : -44.6dB
}ADO_DACGAIN;
//------------------------------------------------------------------------------
// Audio upsampling rate
typedef enum 
{
	UPSAMPLING_2x_out = 0,  //!< Upsample 2 times output
	UPSAMPLING_3x_out,      //!< Upsample 3 times output
	UPSAMPLING_4x_out,      //!< Upsample 4 times output
	UPSAMPLING_6x_out       //!< Upsample 6 times output
}ADO_UPSAMPLING;
//------------------------------------------------------------------------------
// Audio MCLK master/slave select
typedef enum
{
	MCLK_MASTER = 0,    //!< MCLK master mode
	MCLK_SLAVE          //!< MCLK slave mode
}ADO_MCLK_SEL_EXT;
//------------------------------------------------------------------------------
// Audio I2S master/slave select
typedef enum
{
	I2S_MASTER = 0, //!< I2S master mode
	I2S_SLAVE       //!< I2S slave mode
}ADO_I2S_SEL_SLAVE;
//------------------------------------------------------------------------------
// Audio gpio pin
typedef enum 
{
	ADO_I2S_ADC_DATA = 0x01,    //!< Audio gpio pin : I2S ADC DATA
	ADO_I2S_ADC_BCLK = 0x02,    //!< Audio gpio pin : I2S ADC BCLK
	ADO_I2S_DAC_DATA = 0x04,    //!< Audio gpio pin : I2S DAC DATA
	ADO_I2S_ADC_LRCLK = 0x08,   //!< Audio gpio pin : I2S ADC LRCLK
	ADO_I2S_ADC_MCLK = 0x10     //!< Audio gpio pin : I2S ADC MCLK
}ADO_GPIO_PIN;
//------------------------------------------------------------------------------
// Audio gpio type
typedef enum 
{
	ADO_GPIO_INPUT = 0, //!< Audio gpio type : input
	ADO_GPIO_OUTPUT     //!< Audio gpio type : output
}ADO_GPIO_TYPE;
//------------------------------------------------------------------------------
// Audio gpio value
typedef enum 
{
	ADO_GPIO_LOW = 0,   //!< Audio gpio value : low
	ADO_GPIO_HIGH       //!< Audio gpio value : high
}ADO_GPIO_VALUE;
//------------------------------------------------------------------------------
typedef enum SNX_AUD32_FORMAT {
	SNX_AUD32_FMT16_8K_8KBPS = 8,
	SNX_AUD32_FMT16_8K_16KBPS  = 13,
	SNX_AUD32_FMT16_16K_16KBPS = 29,
}ADO_SNX_AUD32_FORMAT;
//------------------------------------------------------------------------------
// AEC/NR return flag
typedef enum
{
	ERROR = 0,  //!< AEC/NR return error
	OK          //!< AEC/NR return ok
}AEC_NR_RETURN_FLAG_t;
//------------------------------------------------------------------------------
// AEC/NR sample rate mode
typedef enum
{
	AEC_NR_8kHZ = 8000,         //!< AEC/NR sample rate mode : 8kHz
	AEC_NR_16kHZ = 16000,       //!< AEC/NR sample rate mode : 16kHz
	AEC_NR_11_025kHZ = 11025    //!< AEC/NR sample rate mode : 11.025kHz
}AEC_NR_SAMPLERATE_MODE_t;
//------------------------------------------------------------------------------
// Audio IP ready status
typedef enum
{
    ADO_IP_NONREADY = 0,
    ADO_IP_READY
}ADO_IP_READY_t;
//------------------------------------------------------------------------------
// Sigma-delta ADC mode
typedef enum
{
    ADO_SIG_DIFFERENTIAL = 0,
    ADO_SIG_SINGLE_END = 1
}ADO_SIG_DEL_ADC_MODE;
//------------------------------------------------------------------------------
// Sigma-delta ADC BOOST gain
typedef enum
{
    ADO_SIG_BOOST_0DB = 0,  //!< Sigma-delta ADC BOOST gain: +0dB
    ADO_SIG_BOOST_20DB,     //!< Sigma-delta ADC BOOST gain: +20dB
    ADO_SIG_BOOST_30DB,     //!< Sigma-delta ADC BOOST gain: +30dB
    ADO_SIG_BOOST_37DB      //!< Sigma-delta ADC BOOST gain: +37dB
}ADO_SIG_DEL_ADC_BOOST_GAIN;
//------------------------------------------------------------------------------
// Sigma-delta ADC PGA gain
typedef enum
{
    ADO_SIG_PGA_MUTE = 0,   //!< Sigma-delta ADC PGA gain: mute
    ADO_SIG_PGA_n12DB,      //!< Sigma-delta ADC PGA gain: -12dB
    ADO_SIG_PGA_n10p5DB,    //!< Sigma-delta ADC PGA gain: -10.5dB
    ADO_SIG_PGA_n9DB,       //!< Sigma-delta ADC PGA gain: -9dB
    ADO_SIG_PGA_n7p5DB,     //!< Sigma-delta ADC PGA gain: -7.5dB
    ADO_SIG_PGA_n6DB,       //!< Sigma-delta ADC PGA gain: -6dB
    ADO_SIG_PGA_n4p5DB,     //!< Sigma-delta ADC PGA gain: -4.5dB
    ADO_SIG_PGA_n3DB,       //!< Sigma-delta ADC PGA gain: -3dB
    ADO_SIG_PGA_n1p5DB,     //!< Sigma-delta ADC PGA gain: -1.5dB
    ADO_SIG_PGA_0DB,        //!< Sigma-delta ADC PGA gain: +0dB
    ADO_SIG_PGA_1p5DB,      //!< Sigma-delta ADC PGA gain: +1.5dB
    ADO_SIG_PGA_3DB,        //!< Sigma-delta ADC PGA gain: +3dB
    ADO_SIG_PGA_4p5DB,      //!< Sigma-delta ADC PGA gain: +4.5dB
    ADO_SIG_PGA_6DB,        //!< Sigma-delta ADC PGA gain: +6dB
    ADO_SIG_PGA_7p5DB,      //!< Sigma-delta ADC PGA gain: +7.5dB
    ADO_SIG_PGA_9DB,        //!< Sigma-delta ADC PGA gain: +9dB
    ADO_SIG_PGA_10p5DB,     //!< Sigma-delta ADC PGA gain: +10.5dB
    ADO_SIG_PGA_12DB,       //!< Sigma-delta ADC PGA gain: +12dB
    ADO_SIG_PGA_13p5DB,     //!< Sigma-delta ADC PGA gain: +13.5dB
    ADO_SIG_PGA_15DB,       //!< Sigma-delta ADC PGA gain: +15dB
    ADO_SIG_PGA_16p5DB,     //!< Sigma-delta ADC PGA gain: +16.5dB
    ADO_SIG_PGA_18DB,       //!< Sigma-delta ADC PGA gain: +18dB
    ADO_SIG_PGA_19p5DB,     //!< Sigma-delta ADC PGA gain: +19.5dB
    ADO_SIG_PGA_21DB,       //!< Sigma-delta ADC PGA gain: +21dB
    ADO_SIG_PGA_22p5DB,     //!< Sigma-delta ADC PGA gain: +22.5dB
    ADO_SIG_PGA_24DB,       //!< Sigma-delta ADC PGA gain: +24dB
    ADO_SIG_PGA_25p5DB,     //!< Sigma-delta ADC PGA gain: +25.5dB
    ADO_SIG_PGA_27DB,       //!< Sigma-delta ADC PGA gain: +27dB
    ADO_SIG_PGA_28p5DB,     //!< Sigma-delta ADC PGA gain: +28.5dB
    ADO_SIG_PGA_30DB,       //!< Sigma-delta ADC PGA gain: +30dB
    ADO_SIG_PGA_31p5DB,     //!< Sigma-delta ADC PGA gain: +31.5dB
    ADO_SIG_PGA_33DB        //!< Sigma-delta ADC PGA gain: +33dB
}ADO_SIG_DEL_ADC_PGA_GAIN;
//------------------------------------------------------------------------------
//! DAC State
typedef enum
{
	ADO_NO_AUDIO_OUT_STATE,
	ADO_AUDIO_OUT_STATE
}ADO_DAC_STATE_TYPE;
//------------------------------------------------------------------------------
//! DAC State
typedef enum
{
	ADO_PLY_BUF_STARTUP,
	ADO_PLY_BUF_AFTER_STARTUP
}ADO_PLY_BUF_INIT_TYPE;
//------------------------------------------------------------------------------
// Audio test mode
typedef enum
{
	NORMAL_PLAY = 0,					//!< Dac normal play type
	WAV_PLAY							//!< Dac wav play type
}ADO_PLAY_TYPE;
//------------------------------------------------------------------------------
// Audio clock
typedef struct AUDIO_CLOCK
{
	ADO_FUN_SWITCH    	mclk_en;        //!< MCLK switch
	ADO_MCLK_SEL_EXT	mclk_sel_ext;   //!< MCLK master/slave select
	ADO_I2S_SEL_SLAVE	i2s_sel_sla;    //!< I2S master/slave select
}ADO_CLOCK_t;
//------------------------------------------------------------------------------
// Audio buffer control
typedef struct AUDIO_BUF
{
	ADO_BUFFERSIZE buffer_size; //!< Buffer size
	ADO_BUFFERTH   buffer_th;   //!< Buffer threshold
}ADO_BUF_t;
//------------------------------------------------------------------------------
// Audio buffer monitor
typedef struct AUDIO_BUF_MONITOR
{
	uint32_t buf_addr_start;        //!< Buffer start address
	uint32_t buf_addr_end;          //!< Buffer end address
	uint32_t buf_addr_index_in;     //!< Buffer index for in
	uint32_t buf_addr_index_out;    //!< Buffer index for out
	uint32_t buf_remain_cnt;		//!< Buffer remaining count
}ADO_BUF_MONIT_t;
//------------------------------------------------------------------------------
// Audio parameter for kernal setting
typedef struct KNL_ADO_PARAMETER
{	
	uint8_t ubQ_InitFlg;                //!< Audio queue initial, 0:No init; 1:Init
	
	ADO_SYS_SPEED_MODE_t Sys_speed;     //!< System speed mode
	
	ADO_ADC_DEV_t Rec_device;           //!< ADC device
	ADO_DAC_DEV_t Ply_device;           //!< DAC device	
	
    ADO_SIG_DEL_ADC_MODE ADO_SigDelAdcMode; //!< Sigma-delta ADC mode(if ADC device is Sigma-delta ADC)
    
	ADO_FORMAT_t Rec_fmt;               //!< ADC format
	ADO_FORMAT_t Ply_fmt;               //!< DAC format
	
	ADO_COMPRESS_MODE Compress_method;  //!< compress method(hardware level)
	
	ADO_ADPCM_t Adpcm_func;             //!< ADPCM parameter function(if compress mothod is ADPCM)
	
	ADO_BUFFERSIZE Rec_buf_size;        //!< ADC buffer size
	ADO_BUFFERSIZE Ply_buf_size;        //!< DAC buffer size
	ADO_BUFFERSIZE Audio32_En_buf_size;
	ADO_BUFFERSIZE Audio32_De_buf_size;
	ADO_BUFFERSIZE AAC_En_buf_size;
	ADO_BUFFERSIZE AAC_De_buf_size;
	ADO_BUFFERSIZE Alarm_buf_size;
		
	ADO_BUFFERTH Rec_buf_th;            //!< ADC buffer threshold
	ADO_BUFFERTH Ply_buf_th;            //!< DAC buffer threshold	
	ADO_BUFFERTH Audio32_En_buf_th;
	ADO_BUFFERTH Audio32_De_buf_th;	
	ADO_BUFFERTH AAC_En_buf_th;
	ADO_BUFFERTH AAC_De_buf_th;
	
	uint32_t ulADO_BufStartAddr;        //!< Audio buffer start address(4-byte alingment!!)
	
	uint32_t ulADO_DelayRestoreTiming;		//!< unit: ms => ex: ulADO_DelayRestoreTiming = 250;
}ADO_KNL_PARA_t;
//------------------------------------------------------------------------------
typedef struct WavPlayInfo{
	uint8_t ubSongIndex;
	uint32_t ubOffsetIndex;
}WavPlayInfo;
//------------------------------------------------------------------------------
// Audio meta data
typedef struct AUDIO_METADATA
{
	ADO_SYS_SPEED_MODE_t Sys_speed;         //!< System speed mode
	
	ADO_ADC_DEV_t Rec_device;               //!< ADC device
	ADO_DAC_DEV_t Ply_device;               //!< DAC device
	
    ADO_SIG_DEL_ADC_MODE ADO_SigDelAdcMode; //!< Sigma-delta ADC mode(if ADC device is Sigma-delta ADC)
    
	ADO_FORMAT_t Rec_fmt;                   //!< ADC format
	ADO_FORMAT_t Ply_fmt;                   //!< DAC format
	
	ADO_COMPRESS_MODE	Compress_method;    //!< Audio compress method
	
	ADO_ADPCM_t Adpcm_func;                 //!< ADPCM parameter function(if compress mothod is ADPCM)
	
	ADO_CLOCK_t ADO_clk;                    //!< Audio clock
	
	ADO_BUFFERSIZE Rec_buf_size;            //!< ADC buffer size
	ADO_BUFFERSIZE Ply_buf_size_startup;	//!< DAC startup buffer size
	ADO_BUFFERSIZE Ply_buf_size;            //!< DAC practical buffer size
	ADO_BUFFERSIZE TempProcess_buf_size;	
	ADO_BUFFERSIZE Audio32_En_buf_size;
	ADO_BUFFERSIZE Audio32_De_buf_size;
	ADO_BUFFERSIZE AAC_En_buf_size;
	ADO_BUFFERSIZE AAC_De_buf_size;
	ADO_BUFFERSIZE Alarm_buf_size;
	
	ADO_BUFFERTH Rec_buf_th;                //!< ADC buffer threshold
	ADO_BUFFERTH Ply_buf_th;                //!< DAC buffer threshold	
	ADO_BUFFERTH Audio32_En_buf_th;
	ADO_BUFFERTH Audio32_De_buf_th;
	ADO_BUFFERTH AAC_En_buf_th;
	ADO_BUFFERTH AAC_De_buf_th;
	
	ADO_BUF_MONIT_t Rec_BufMonit;           //!< ADC buffer index monitor
	ADO_BUF_MONIT_t Ply_BufMonit;           //!< DAC buffer index monitor
	ADO_BUF_MONIT_t Audio32Remain_BufMonit; //!< This biffer must locate on address before TempProcess
	ADO_BUF_MONIT_t TempProcess_BufMonit;	
	ADO_BUF_MONIT_t DeHowling_BufMonit;
	
	ADO_BUF_MONIT_t Audio32_En_BufMonit;
	ADO_BUF_MONIT_t Audio32_De_BufMonit;
	
	ADO_BUF_MONIT_t AAC_En_BufMonit;
	ADO_BUF_MONIT_t AAC_De_BufMonit;
	ADO_BUF_MONIT_t Alarm_BufMonit;
	
	ADO_FUN_SWITCH Audio32_Switch;
	ADO_FUN_SWITCH AAC_Switch;
	
	ADO_FUN_SWITCH AEC_Switch;
	ADO_FUN_SWITCH NR_Switch;
	ADO_FUN_SWITCH DeHowling_Switch;
	
	uint32_t ulADO_BufStartAddr;            //!< Audio buffer start address(4-byte alingment!!)
	uint32_t ulADO_TotalBufSize;            //!< Audio total buffer size(unit:bytes)
    
	ADO_IP_READY_t ADO_IpReadyStatus;
	
	ADO_PLAY_TYPE ADO_PlayType;
	
	WavPlayInfo WavInfo;
	uint8_t ubPlayStop;
	uint8_t ubReap;
	
	ADO_SNX_AUD32_FORMAT ADO_Aud32EncFmt[ADO_AUDIO32_MAX_NUM];
	ADO_SNX_AUD32_FORMAT ADO_Aud32DecFmt[ADO_AUDIO32_MAX_NUM];
	
	uint32_t ulADO_Encode_Timestamp;		//!< unit: 10ms => ex: ulADO_Encode_Timestamp=2 means 20ms;
//	uint32_t ulADO_Decode_Timestamp;		//!< unit: 10ms => ex: ulADO_Decode_Timestamp=2 means 20ms;
	uint32_t ulADO_DelayRestoreTiming;		//!< unit: 1ms => ex: ulADO_DelayRestoreTiming = 250;
	uint8_t ubADO_ResetDelayParameter;
}ADO_METADATA_t;

typedef enum 
{
	ADO_WAV_IDLE = 0,
	ADO_WAV_PLAYING
}ADO_WAV_STATE;

//------------------------------------------------------------------------------
void ADO_SetAudioOutState (ADO_DAC_STATE_TYPE tState);
ADO_DAC_STATE_TYPE tADO_GetAudioOutState (void);
//==============================================================================
// Audio External API
//==============================================================================
/*!
\brief ADO version
\return ((ADO_MAJORVER << 8) + ADO_MINORVER)
*/
uint16_t uwADO_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief ADO IP status
\return ADO_IP_READY_t
*/
ADO_IP_READY_t ADO_GetIpReadyStatus(void);
//------------------------------------------------------------------------------
/*!
\brief Audio Setup
\param ADO_KNL_PARA_t
\param pADO_EncodeQIdHandle		
\param pADO_DecodeQIdHandle
\par Note:
	1. You can set up the audio parameter and it will register ADC/DAC ISR and disable ADC/DAC related interrupt.
\return(no)
*/
void ADO_Setup(ADO_KNL_PARA_t *AdoPara ,osMessageQId* pADO_EncodeQIdHandle , osMessageQId* pADO_DecodeQIdHandle);
//------------------------------------------------------------------------------
/*!
\brief get audio total buffer size
\param ADO_KNL_PARA_t
\return total buffer size(unit:bytes)
*/
uint32_t ulADO_GetTotalBuffSize(ADO_KNL_PARA_t *AdoPara);
//------------------------------------------------------------------------------
/*!
\brief get audio AAC enable switch
\return AAC enable switch
*/
ADO_FUN_SWITCH tADO_GetAACEnable(void);
//------------------------------------------------------------------------------
/*!
\brief get audio Audio32 enable switch
\return Audio32 enable switch
*/
ADO_FUN_SWITCH tADO_GetAdo32Enable(void);
//------------------------------------------------------------------------------
/*!
\brief Audio record start
\par Note:
	1. It will enable ADC related interrupt.
\return(no)
*/
void ADO_RecStart(void);
//------------------------------------------------------------------------------
/*!
\brief Audio record stop
\par Note:
	1. It will disable ADC related interrupt.
\return(no)
*/
void ADO_RecStop(void);
//------------------------------------------------------------------------------
/*!
\brief Audio play start
\par Note:
	1. It will enable DAC related interrupt.
\return(no)
*/
void ADO_PlyStart(void);
//------------------------------------------------------------------------------
/*!
\brief Audio play stop
\par Note:
	1. It will disable DAC related interrupt.
\return(no)
*/
void ADO_PlyStop(void);
//------------------------------------------------------------------------------
/*!
\brief AEC NR Setting
\param Type             Process Type Switch
\param SampleRate       Data SampleRate
\return(no)
\par [Example]
\code 
    ADO_Noise_Process_Type(NOISE_DISABLE ,AEC_NR_16kHZ);
\endcode
*/
void ADO_Noise_Process_Type(ADO_NOISE_PROCESS_TYPE Type ,AEC_NR_SAMPLERATE_MODE_t SampleRate);
//------------------------------------------------------------------------------
/*!
\brief Sigma-delta ADC gain setting
\param Boost        BOOST gain
\param Pga          PGA gain
\return(no)
*/
void ADO_SetSigmaDeltaAdcGain(ADO_SIG_DEL_ADC_BOOST_GAIN Boost, ADO_SIG_DEL_ADC_PGA_GAIN Pga);
//------------------------------------------------------------------------------

/*!
\brief ADC adjust function(DC compensation)
\param ubTH1			ADC DC compensation STEP Threshold 1.
\param ubTH2 			ADC DC compensation STEP Threshold 2.
\param ubTH3		 	ADC DC compensation STEP Threshold 3.
\param Switch		 	ADO_ON/ADO_OFF
\par Note:
	1. Threshold 3 > Threshold 2 > Threshold 1.
\return(no)
*/
void ADO_SetAdcDcComp(uint8_t ubTH1, uint8_t ubTH2, uint8_t ubTH3, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief ADC adjust function(Gain)
\param Gain                 Digital gain for ADC (Range: 0-20).
\param ubGainRmpRate        Amplitude +0.5dB every (GAINRMP_RATE*512) sample.
\param Switch               ADO_ON/ADO_OFF
\par Note:
	1. Gain:\n
					00000:0dB,   00111:3.5dB, 01110: 7dB,\n
					00001:0.5dB, 01000:4dB,   01111: 7.5dB,\n
					00010:1dB,   01001:4.5dB, 10000: 8dB,\n
					00011:1.5dB, 01010:5dB,   10001: 8.5dB,\n
					00100:2dB,   01011:5.5dB, 10010: 9dB,\n
					00101:2.5dB, 01100:6dB,   10011: 9.5dB,\n
					00110:3dB,   01101:6.5dB, 10100: 10dB.
\return(no)
*/
void ADO_SetAdcGain(ADO_ADCGAIN Gain, uint8_t ubGainRmpRate, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief ADC adjust function(Mute)
\param Mode         Mute speed.
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Mute speed:\n
                    00:Amplitude -3dB every 4 sample\n
                    01:Amplitude -3dB every 8 sample\n
                    10:Amplitude -3dB every 16 sample\n
                    11:Amplitude -3dB every 32 sample.
\return(no)
*/
void ADO_SetAdcMute(ADO_ADCMUTESPEED Mode, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief ADC adjust function(Report)
\param uwLowLvPeak      ADC low level peak detection for 16-bit pcm.
\param uwHighLvPeak     ADC high level peak detection for 16-bit pcm.
\param Switch           ADO_ON/ADO_OFF
\par Note:
	1. Must enable ADC gain if you want to use ADC report.
\return(no)
*/
void ADO_SetAdcRpt(uint16_t uwLowLvPeak, uint16_t uwHighLvPeak, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Get ADC report(Sum_L)
\return Adc_Sum_L(summation of 16-bit PCM for ADC when the value>Arch-band ADC_TH_L)
*/
uint32_t ulADO_GetAdcSumLow(void);
//------------------------------------------------------------------------------
/*!
\brief Get ADC report(Sum_H)
\return Adc_Sum_L(summation of 16-bit PCM for ADC when the value>Arch-band ADC_TH_H)
*/
uint32_t ulADO_GetAdcSumHigh(void);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Mute)
\param Mode         Attenuation ramp rate for DAC.
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Ramp rate:\n
            00:0.5 dB per 1 sample.  10:0.5 dB per 4 samples.\n
            01:0.5 dB per 2 samples. 11:0.5 dB per 8 samples.
\return(no)
*/
void ADO_SetDacMute(AUDIO_DACMUTERMP Mode, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Automute)
\param Switch       ADO_ON/ADO_OFF
\return(no)
*/
void ADO_SetDacAutoMute(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(R2R volume)
\param Volume       Volume control.
\par Note:
	1. Volume control:\n
                    15: 0dB,    7:-23.5dB\n
                    14:-3dB,    6:-26.2dB\n
                    13:-5.6dB,  5:-29.8dB\n
                    12:-8.2dB,  4:-32.4dB\n
                    11:-11.9dB, 3:-36dB\n
                    10:-14.6dB, 2:-39.1dB\n
                    9:-18.2dB, 1:-42dB\n
                    8:-21.4dB, 0:-45dB
\return(no)
*/
void ADO_SetDacR2RVol(ADO_R2R_VOL Volume);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Gain)
\param Gain         Digital gain for DAC (Range: 0-15).
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Gain:\n
					0:0db,     8:-24db\n
					1:-2.5db,  9:-26.6db\n
					2:-6db,   10:-30.1db\n
					3:-8.5db, 11:-32.6db\n
					4:-12db,  12:-36.1db\n
					5:-14db,  13:-38.6db\n
					6:-18db,  14:42.1db\n
					7:-20db,  15:-44.6db
\return(no)
*/
void ADO_SetDacGain(ADO_DACGAIN Gain, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Upsample)
\param Mode         Upsample mode.
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. Upsample mode:\n
                    00:2x output samples\n
                    01:3x output samples\n
                    10:4x output samples\n
                    11:6x output samples
\return(no)
*/
void ADO_SetDacUpsample(ADO_UPSAMPLING Mode, ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(M2SO)
\param Switch       ADO_ON/ADO_OFF
\par Note:
	1. The left channel audio data will be copied to right channel in I2S DAC MONO mode\n
		 when this bit is enabled. (Mono to Stereo out)
\return(no)
*/
void ADO_SetDacM2so(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief DAC adjust function(Report)
\param Switch       ADO_ON/ADO_OFF
\return(no)
*/
void ADO_SetDacRpt(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Audio gpio initial
\param Mask     ADO_I2S_ADC_DATA\n
                ADO_I2S_ADC_BCLK\n
                ADO_I2S_DAC_DATA\n
                ADO_I2S_ADC_LRCLK\n
                ADO_I2S_ADC_MCLK
\param Type     ADO_GPIO_INPUT/ADO_GPIO_OUTPUT
\return(no)
*/
void ADO_GpioInit(ADO_GPIO_PIN Mask, ADO_GPIO_TYPE Type);
//------------------------------------------------------------------------------
/*!
\brief Audio gpio write
\param Mask     ADO_I2S_ADC_DATA\n
                ADO_I2S_ADC_BCLK\n
                ADO_I2S_DAC_DATA\n
                ADO_I2S_ADC_LRCLK\n
                ADO_I2S_ADC_MCLK
\param Value    ADO_GPIO_LOW/ADO_GPIO_HIGH
\return(no)
*/
void ADO_GpioWrt(ADO_GPIO_PIN Mask, ADO_GPIO_VALUE Value);
//------------------------------------------------------------------------------
/*!
\brief Audio gpio read
\param Mask     ADO_I2S_ADC_DATA\n
                ADO_I2S_ADC_BCLK\n
                ADO_I2S_DAC_DATA\n
                ADO_I2S_ADC_LRCLK\n
                ADO_I2S_ADC_MCLK
\return ADO_GPIO_VALUE      ADO_GPIO_LOW/ADO_GPIO_HIGH
*/
ADO_GPIO_VALUE ubADO_GpioRd(ADO_GPIO_PIN Mask);
//------------------------------------------------------------------------------
/*!
\brief Get audio sample rate
\return SampleRateValue
*/
uint16_t uwADO_GetSampleRate(void);
//------------------------------------------------------------------------------
/*!
\brief Write audio data to decode buffer.
\param AM		    Audio Metadata
\param Input        Input audid data(encode data).
\return(no)
*/
void ADO_DecodeBufferWrtIn(ADO_METADATA_t *AM, ADO_Queue_INFO *Input);
//------------------------------------------------------------------------------
#if ADO_AUDIO32
/*!
\brief Audio32 Enable Setting
\param Switch       ADO_ON/ADO_OFF
\return(no)
\par [Example]
\code 
			ADO_Set_Audio32_Enable(ADO_ON);
\endcode
*/
void ADO_Set_Audio32_Enable(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Encoder Initial
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) encoder initial
\param Format   	Audio32 Format Enum 
\return(no)
\par [Example]
\code 
        ADO_Audio32_Encoder_Init(0,SNX_AUD32_FMT8_8KBPS);
\endcode
*/
void ADO_Audio32_Encoder_Init(uint8_t ubUseIdx, ADO_SNX_AUD32_FORMAT Format);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Decoder Initial
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) decoder initial
\param Format       Audio32 Format Enum 
\return(no)
\par [Example]
\code 
        ADO_Audio32_Decoder_Init(0,SNX_AUD32_FMT8_8KBPS);
\endcode
*/
void ADO_Audio32_Decoder_Init(uint8_t ubUseIdx, ADO_SNX_AUD32_FORMAT Format);
//------------------------------------------------------------------------------
/*!
\brief Get Audio32 encoder format
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support to get (ADO_AUDIO32_MAX_NUM) encoder format
\return	ADO_SNX_AUD32_FORMAT
\par [Example]
\code 
		ADO_SNX_AUD32_FORMAT fmt
        fmt = ADO_GetAudio32EncFormat(0);
\endcode
*/
ADO_SNX_AUD32_FORMAT ADO_GetAudio32EncFormat(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief Get Audio32 decoder format
\param ubUseIdx		range:0~(ADO_AUDIO32_MAX_NUM-1); support to get (ADO_AUDIO32_MAX_NUM) encoder format
\return	ADO_SNX_AUD32_FORMAT
\par [Example]
\code 
		ADO_SNX_AUD32_FORMAT fmt
        fmt = ADO_GetAudio32DecFormat(0);
\endcode
*/
ADO_SNX_AUD32_FORMAT ADO_GetAudio32DecFormat(uint8_t ubUseIdx);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Encode Function
\param ubUseIdx			range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) encoder
\param ulSrc            Address of Source
\param ulDes            Address of Destition
\param ulTotalSize      Total Size of Source Data
\return ADO_AUD32_ENC_INFO
\par [Example]
\code
		ADO_AUD32_ENC_INFO Aud32EncInfo;
        Aud32EncInfo = ADO_Audio32_Encode(0,SrcAddr,DesAddr,100);
\endcode
*/
ADO_AUD32_ENC_INFO ADO_Audio32_Encode(uint8_t ubUseIdx, uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulSize);
//------------------------------------------------------------------------------
/*!
\brief Audio32 Decode Function
\param ubUseIdx			range:0~(ADO_AUDIO32_MAX_NUM-1); support (ADO_AUDIO32_MAX_NUM) decoder
\param ulSrc            Address of Source
\param ulDes            Address of Destition
\param ulTotalSize      Total Size of Source Data
\return ADO_AUD32_DEC_INFO
\par [Example]
\code
		ADO_AUD32_DEC_INFO Aud32DecInfo;
        Aud32DecInfo = ADO_Audio32_Decode(0,SrcAddr,DesAddr,100);
\endcode
*/
ADO_AUD32_DEC_INFO ADO_Audio32_Decode(uint8_t ubUseIdx, uint32_t ulSrcAddr, uint32_t ulDestAddr, uint32_t ulSize);
//------------------------------------------------------------------------
/*!
\brief Audio32 encode buffer write in function
\param AM			Audio Metadata
\param SourAddr		source address
\param Size			write in size
\param Copy			Copy or not:ADO_ON/ADO_OFF.
\return(no)
\endcode
*/
void ADO_Audio32_EncBufWrtIn(ADO_METADATA_t *AM, uint32_t SourAddr, uint32_t Size, ADO_FUN_SWITCH Copy);
//------------------------------------------------------------------------
/*!
\brief Audio32 encode buffer read out function
\param AM			Audio Metadata
\param DestAddr		destination address
\param Size			read out size
\param Copy			Copy or not:ADO_ON/ADO_OFF.
\return(no)
\endcode
*/
void ADO_Audio32_EncBufRdOut(ADO_METADATA_t *AM, uint32_t DestAddr, uint32_t Size, ADO_FUN_SWITCH Copy);
//------------------------------------------------------------------------
/*!
\brief Audio32 decode buffer write in function
\param AM			Audio Metadata
\param SourAddr		source address
\param Size			write in size
\param Copy			Copy or not:ADO_ON/ADO_OFF.
\return(no)
\endcode
*/
void ADO_Audio32_DecBufWrtIn(ADO_METADATA_t *AM, uint32_t SourAddr, uint32_t Size, ADO_FUN_SWITCH Copy);
//------------------------------------------------------------------------
/*!
\brief Audio32 deocde buffer read out function
\param AM			Audio Metadata
\param DestAddr		destination address
\param Size			read out size
\param Copy			Copy or not:ADO_ON/ADO_OFF.
\return(no)
\endcode
*/
void ADO_Audio32_DecBufRdOut(ADO_METADATA_t *AM, uint32_t DestAddr, uint32_t Size, ADO_FUN_SWITCH Copy);
//------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------
/*!
\brief 	Get Audio Version	
\return	Version
*/
uint16_t uwADO_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief DeHowling Enable
\param Switch   On/OFF
\return(no)
\par [Example]
\code 
        ADO_Set_DeHowling_Enable(ADO_ON);
\endcode
*/
void ADO_Set_DeHowling_Enable(ADO_FUN_SWITCH Switch);
//------------------------------------------------------------------------------
/*!
\brief Play Wav Audio
\param ubIndex Song Index
\return(no)
\par [Example]
\code 
        ADO_WavPlay(0);
\endcode
*/
void ADO_WavPlay(uint8_t ubIndex);
//------------------------------------------------------------------------------
/*!
\brief Repeat Wav Audio
\param ubIndex Song Index
\return(no)
\par [Example]
\code 
        ADO_WavRepeat(0);
\endcode
*/
void ADO_WavRepeat(uint8_t ubIndex);
//------------------------------------------------------------------------------
/*!
\brief Stop Wav Audio
\return(no)
\par [Example]
\code 
        ADO_WavStop();
\endcode
*/
void ADO_WavStop(void);
//------------------------------------------------------------------------------
/*!
\brief Resume Wav Audio. RePlay at last ADO_WavStop time 
\return(no)
\par [Example]
\code 
        ADO_WavResume();
\endcode
*/
void ADO_WavResume(void);
//------------------------------------------------------------------------------
/*!
\brief Get Wav Play state 
\return 0: wav idle
		1: wav playing
\code 
        tADO_GetWavState();
\endcode
*/
ADO_WAV_STATE tADO_GetWavState(void);
//------------------------------------------------------------------------------
/*!
\brief Compensate input gain
\param ulGainValue		gain value: 1/2/3/4...
\param ulStartAddr		buffer start address
\param ulSize			total size of data
\return ADO_SUCCESS/ADO_FAIL
\par [Example]
\code 
        ADO_NrCompensationGain(4, 0x1D00000, 4096);
\endcode
*/
ADO_RETURN_FLAG ADO_NrCompensationGain(uint32_t ulGainValue, uint32_t ulStartAddr, uint32_t ulSize);
//------------------------------------------------------------------------------
/*!
\brief reset delay parameter when Tx/Rx startup
\return(no)
\par [Example]
\code 
        ADO_ResetDelayPara();
\endcode
*/
void ADO_ResetDelayPara(void);
//------------------------------------------------------------------------------
extern const uint32_t ulADO_BufTh[];
extern ADO_METADATA_t GlobalAudioMeta;
//------------------------------------------------------------------------------
#endif
