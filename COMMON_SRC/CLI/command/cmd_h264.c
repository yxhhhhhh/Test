#include <stdio.h>
#include <stdlib.h>
#include "H264_API.h"
#include "cmd_h264.h"
#include "CLI.h"

void H264StatusUsage(void)
{
	printf(" Usage: h264_status [Channel]\n");
	printf(" Channel:0~3 \n");
	printf(" Example: \n");
	printf(" h264_status 1 \n");
}

void H264InitUsage(void)
{
	printf(" Usage: h264_init [Channel] [QP] [GOP]\n");
	printf(" Channel:0~3 \n");
	printf(" QP:1~51 \n");
	printf(" GOP:0~65535 \n");
	printf(" Example: \n");
	printf(" h264_init 1 30 30 \n");
}

int32_t cmd_h264_status(int argc, char* argv[])
{
	int32_t channel;
	channel  = strtoul(argv[1],NULL,10);
	
	if(argc<2)
	{
		H264StatusUsage();
		return cliFAIL;	
	}
	
	if(channel<0 || channel >3)
	{
		printf("Wrong Channel Number\n");
		return cliFAIL;	
	}
	printf("H264 Encode Channel %d\n",channel);
	printf("H264 GOP = %d\n",H264_GetGOP((H264_ENCODE_INDEX)channel));
	printf("H264 QP = %d\n",H264_GetCurrentQP());
	return cliFAIL;   
}

int32_t cmd_h264_init(int argc, char* argv[])
{
	int32_t channel;
	uint32_t QP,GOP;
	
	if(argc<4)
	{
		H264InitUsage();
		return cliFAIL;	
	}
	
	channel  = strtoul(argv[1],NULL,10);
	
	if(channel<0 || channel >3)
	{
		printf("Wrong Channel Number\n");
		return cliFAIL;	
	}
	
	QP  = strtoul(argv[2],NULL,10);
	if(QP<1 || QP >51)
	{
		printf("Wrong QP Value\n");
		return cliFAIL;	
	}
	
	GOP  = strtoul(argv[3],NULL,10);
	if(GOP>65535)
	{
		printf("Wrong GOP Value\n");
		return cliFAIL;	
	}
	
	H264_SetQp((H264_ENCODE_INDEX)channel,QP,QP);
	H264_SetGOP((H264_ENCODE_INDEX)channel,GOP);
	
	return cliPASS; 
}
