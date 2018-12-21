#include "cmd.h"
//------------------------------------------------------------------------------
#ifdef CONFIG_CLI_CMD_UVC
#include "sonix_config.h"

#if defined (CONFIG_USBH_FREE_RTOS)
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "USBH_UVC.h"
#include "cmd_uvc.h"

#ifdef	CONFIG_CLI_SUPPORT_USBD_PREV
#include "usb_device.h"
#include "usbd_uvc.h"
#endif

#if(_SUPPORT_USBD_WIFI_PREVIEW)
#include "apps/wifi_stream.h"
#endif

#if defined (CONFIG_RECORD)
#include "rec_common.h"
#include "rec_schedule.h"
#include "record.h"
#include "automount.h"
#endif

#define assert(x)

USBH_UVC_MW_STRUCTURE		usbh_uvc_mw;
USBH_UVC_APP_STRUCTURE 	usbh_uvc_app;
uint8_t	usbd_inited = 0;

#if defined (CONFIG_RECORD)
recordinfo_t *uvc_record_info = NULL;
automount_info_t *uvc_automount_info = NULL;
recordinfo_t UVC_RECORD_INFO[max_stream_count+1];

static void uvc_record_task(void *pvParameters);
void uvc_record_video_task( void *pvParameters );
#endif

static void bits_handler_init(bits_handler *bs, char *buffer, unsigned buffer_length)
{
	bs->ptr = bs->base = buffer;
	bs->length = buffer_length;
	bs->index = 0;
}

static unsigned long read_bits(bits_handler *bs, int bits)
{
	int valid_bits = 8 - bs->index;
	int remain_bits = bits - valid_bits;
	unsigned long v = 0;
	int bytealign, i;

	assert(bits < 33);

	if (!(bits > valid_bits)) {
		v = ((*bs->ptr >> (valid_bits - bits)) & ((1 << bits) -1));
		bs->index += bits;
		return v;
	}

	v = (*bs->ptr++ & ((1 << valid_bits) - 1)) << 8;
	bs->index = 0;
	bytealign = ((remain_bits | 0x07) & ~(unsigned long)0x07) >> 3;
	for (i=0; i<bytealign; ++i) {
		//v = (v | *bs->ptr++) << 8;
		if (remain_bits > 8) {
			v = (v | *bs->ptr++) << 8;
			remain_bits -= 8;
		} else {
			v = (v | *bs->ptr) >> (8 - remain_bits);
			bs->index = remain_bits;
		}
	}

	return v;
}

static unsigned long ue_exp_golomb(bits_handler *bs)
{
	int leading_zero_bits = -1;
	int b;

	for (b = 0; !b; ++leading_zero_bits) {
		b = read_bits(bs, 1);
	}

	return (1 << leading_zero_bits) - 1 + read_bits(bs, leading_zero_bits);
}


static int get_slice_type(char *ptr, unsigned remain_sz)
{
	char *p = NULL;
	unsigned short nal_unit_type;
	int slice_type = NOT_SLICE;

	// 4 bytes start code and 4 bytes content should be parsed
	if (remain_sz < 8) {
#if _DEBUG_VERBOSE
		print_msg_queue("%s: remain data size not enough\n", __func__);
#endif
		return TYPE_ERROR;
	}

	p = ptr;
	p += 4;
	nal_unit_type = (*p++) & 0x1F;
	switch(nal_unit_type) {
		case SLICE_PIC:
		{
			bits_handler bs;
			//unsigned first_mb_in_slice;
			//unsigned pic_parameter_set_id;

			bits_handler_init(&bs, p, 32);
			//first_mb_in_slice = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			slice_type = ue_exp_golomb(&bs);
			//pic_parameter_set_id = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			break;
		}
		case SLICE_PA:
		case SLICE_PB:
		case SLICE_PC:
#if _DEBUG_VERBOSE
			print_msg_queue("*** find coded slice partition, "
				"nal_unit_type %u ***\n", nal_unit_type);
#endif
			break;
		case SLICE_IDR_PIC:
		{
			bits_handler bs;
			//unsigned first_mb_in_slice;
			//unsigned pic_parameter_set_id;

			bits_handler_init(&bs, p, 32);
			//first_mb_in_slice = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			slice_type = ue_exp_golomb(&bs);
			//pic_parameter_set_id = ue_exp_golomb(&bs);
			ue_exp_golomb(&bs);
			break;
		}
		case SEI:
#if _DEBUG_VERBOSE
			print_msg_queue("*** find SEI ***\n");
#endif
			break;
		case SPS:
		case PPS:
			break;
		default:
#if _DEBUF_VREBOSE
			print_msg_queue("*** H.264 nal_unit_type %u ***\n",
					nal_unit_type);
#endif
			break;
	}

	return slice_type;
}

int snx_avc_get_slice_type(unsigned char *paddr, unsigned size)
{
	unsigned *ptr = (unsigned*)paddr;
	unsigned left_sz = size, value, status = 0;
	short last_status;
	int slice_type = TYPE_ERROR;

	if ((unsigned) ptr & 0x03L) {
#if _DEBUG_VERBOSE
		print_msg_queue("%s: bit-stream buffer not aligned\n");
#endif
	}
get_next_four_bytes:
	if (left_sz > 3) {
		value = *(unsigned*) ptr++;
		left_sz -= 4;
	} else if (left_sz > 0) {
		char *ptr_noaligned = (char*) ptr;
		value = 0x0L;
		while (left_sz) {
			value = value << 8;
			value |= *ptr_noaligned;
			ptr_noaligned++;
			left_sz--;
		}
		ptr = (unsigned*) ptr_noaligned;
	} else {
		goto parse_end;
	}
//parse_start:
	if (value == 0x01000000L) {
		last_status = 0;
		status = 5;
	}
state_start:
	switch (status) {
		case 0:
			if ((value & 0xFF000000L) == 0L) {
				status = 1;
			if ((value & 0xFFFF0000L) == 0L) {
				status = 2;
			if ((value & 0xFFFFFF00L) == 0L) {
				status = 3;
			if ((value & 0xFFFFFFFFL) == 0L)
				status = 4;
			}}}
			goto get_next_four_bytes;
		case 1:
			if ((value & 0x00FFFFFFL) == 0x00010000) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 2:
			if ((value & 0x0000FFFFL) == 0x00000100) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 3:
			if ((value & 0x000000FFL) == 0x00000001) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 4:
			// get next four bytes,
			// if value is one of case 1, 2, 3 or equal to
			// 0x01000000, then founded
			last_status = status;
			goto get_next_four_bytes;
		case 5:
		{
			char *start_code = NULL;
			if (last_status == 0) {
				start_code = (char*)(--ptr);
				left_sz += 4;
			} else if (last_status == 1) {
				start_code = (char*)(ptr - 2);
				start_code += 3;
				left_sz += 5;
			} else if (last_status == 2) {
				start_code = (char*)(ptr - 2);
				start_code += 2;
				left_sz += 6;
			} else if (last_status == 3) {
				start_code = (char*)(ptr - 2);
				start_code += 1;
				left_sz += 7;
			} else {
#if _DEBUG_VERBOSE
				print_msg_queue("%s: start code parsing "
						"failed\n", __func__);
#endif
			}


			// TODO: need to handle error case
			if ((slice_type = get_slice_type(start_code, left_sz)) < TYPE_P) {
				// integer pointer address must align 4 bytes
				status = 0;
				ptr = (unsigned *)(((unsigned)start_code + 4) & 0xFFFFFFFCL);
				left_sz -= 4 + ((unsigned)start_code & 0x03L);
				goto get_next_four_bytes;
			}
			break;
		}
	}
parse_end:
	return ((slice_type > 4) ? (slice_type - 5) : slice_type);
}

uint8_t streamid_to_streamidx(uint8_t device_id, uint8_t stream_id) {
	uint8_t i = 0;
	
	for(i = 0; i < max_stream_count; i++) {
		if(usbh_uvc_app.dev[device_id].stream[i].stream_id == stream_id)
			return i;
	}
	
	return max_stream_count;
}

#if defined (CONFIG_USBH_FREE_RTOS)
void uvc_stream_xfr_task(void *pvParameters)
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
void uvc_stream_xfr_task(void const *pvParameters)
#endif
{
	int currentFrameType = 0;
	uint8_t stream_idx = max_stream_count;
	uint32_t last_err_cnt[USBH_MAX_PORT*2][max_stream_count] = {0};
	uint32_t last_babble_cnt[USBH_MAX_PORT*2][max_stream_count] = {0};
	
  memset(&usbh_uvc_mw, 0x00, sizeof(usbh_uvc_mw));
	
	for(;;){
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueReceive(usbh_queue_uvc_mw, &usbh_uvc_mw, USBH_MAX);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessageGet(usbh_queue_uvc_mw, &usbh_uvc_mw, USBH_MAX);	
#endif
		
		stream_idx = streamid_to_streamidx(usbh_uvc_mw.dev_id, usbh_uvc_mw.stream_id);
		
		if(usbh_uvc_mw.err){
			usbh_uvc_mw.err = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errcnt ++;
		}
		if(usbh_uvc_mw.babble){
			usbh_uvc_mw.babble = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt ++;
			
		}
		if(usbh_uvc_mw.discard){
			usbh_uvc_mw.discard = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].discardcnt ++;
		}
		if(usbh_uvc_mw.underflow){
			usbh_uvc_mw.underflow = 0;
			usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].underflowcnt ++;
		}			

		if((last_err_cnt[usbh_uvc_mw.dev_id][stream_idx] != usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errcnt)
			|| (last_babble_cnt[usbh_uvc_mw.dev_id][stream_idx] != usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt)) {
			last_err_cnt[usbh_uvc_mw.dev_id][stream_idx] = usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errcnt;
			last_babble_cnt[usbh_uvc_mw.dev_id][stream_idx] = usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt;
				
			UVC_INFO("err=%d , babble=%d"
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errcnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt);
		}

#if defined(CONFIG_SNX_ISO_ACCELERATOR)
		//if((usbh_uvc_mw.size	> 12) && (((uint32_t)usbh_uvc_mw.ptr)+usbh_uvc_mw.size) <= usbh_uvc_mw.ring_buff_end ){
		if(usbh_uvc_mw.size	> 12) {
#else
			if(usbh_uvc_mw.size > 12){	
#endif 
				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt	++;
				UVC_INFO("%d, frame count = %d", usbh_uvc_mw.stream_id, usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt);
				
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].debug_msg){
					UVC_INFO(" =================================");
					UVC_INFO(" usbh_uvc_dev.id = %d",usbh_uvc_mw.dev_id);					
					UVC_INFO(" usbh_uvc_stream.id = %d",usbh_uvc_mw.stream_id);
					UVC_INFO(" Total FrameCNT = %d",usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt);					
					UVC_INFO(" usbh_uvc_mw.ptr = %08x",(uint32_t)usbh_uvc_mw.ptr);
					UVC_INFO(" usbh_uvc_mw.size = %d",usbh_uvc_mw.size);					
					UVC_INFO(" usbh_uvc_mw.ring_buff_end= %x",usbh_uvc_mw.ring_buff_end);
#if defined( CONFIG_SN_GCC_SDK )			
					UVC_INFO(" usbh_uvc_mw timeval= %x",tval.tv_sec*1000+(tval.tv_usec+500)/1000);						
					UVC_INFO(" usbh_uvc_mw iframe= %x",iframe);
#endif 					
					UVC_INFO(" usbh_uvc_mw.errcnt = %d , data_discard_cnt = %d , babble = %d , undeflow = %d"
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errcnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].underflowcnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].discardcnt);                                  
				}
#if(_SUPPORT_USBD_WIFI_PREVIEW)
	if(usbh_uvc_app.socket_fd >0){
		if(wifi_stream_send(usbh_uvc_app.socket_fd, ((char *)usbh_uvc_mw.ptr)+12 ,usbh_uvc_mw.size - 12)<0)
    {
        printf("Wifi stream socket call failed");
    }
	}
#endif
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].usb_preview){
					if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isH264) {
						if (usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame == 0) {
								currentFrameType = snx_avc_get_slice_type((unsigned char*)usbh_uvc_mw.ptr, usbh_uvc_mw.size);
								if (currentFrameType == TYPE_I) {
									usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame = 1;
								}		
						}
						if (usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame) {
#if defined (CONFIG_CLI_SUPPORT_USBD_PREV) 							
							usbd_uvc_drv_send_image(usbh_uvc_mw.ptr ,usbh_uvc_mw.size - 12);
#endif
						}
					} else {
#if defined (CONFIG_CLI_SUPPORT_USBD_PREV)
						usbd_uvc_drv_send_image(usbh_uvc_mw.ptr ,usbh_uvc_mw.size - 12);
#endif 						
					}			
				}		
#if defined( CONFIG_RECORD )
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[usbh_uvc_mw.stream_id-1].sd_record){
					((currentFrameType = snx_avc_get_slice_type((unsigned char*)usbh_uvc_mw.ptr, usbh_uvc_mw.size)) == TYPE_I) ? (iframe = 1) : (iframe = 0);
					if((currentFrameType == TYPE_I) || (currentFrameType == TYPE_P)) {	
							record_video(uvc_record_info->pRecord_info, iframe, (unsigned char *)usbh_uvc_mw.ptr, usbh_uvc_mw.size);
					}
				}	
#endif
				
			}else{
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].debug_msg){
					UVC_INFO(" =================================");
					UVC_INFO(" usbh_uvc_dev.id = %d",usbh_uvc_mw.dev_id);						
					UVC_INFO(" usbh_uvc_mw.stream_id = %d",usbh_uvc_mw.stream_id);
					UVC_INFO(" Total FrameCNT = %d",usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt);					
					UVC_INFO(" usbh_uvc_mw.ptr = %08x",(uint32_t)usbh_uvc_mw.ptr);
					UVC_INFO(" usbh_uvc_mw.size = %d",usbh_uvc_mw.size);					
					UVC_INFO(" usbh_uvc_mw.ring_buff_end= %x",usbh_uvc_mw.ring_buff_end);
#if defined( CONFIG_SN_GCC_SDK )			
					UVC_INFO(" usbh_uvc_mw timeval= %x",tval.tv_sec*1000+(tval.tv_usec+500)/1000);						
					UVC_INFO(" usbh_uvc_mw iframe= %x",iframe);
#endif 					
					UVC_INFO(" usbh_uvc_mw.errcnt = %d , data_discard_cnt = %d , babble = %d , undeflow = %d"
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].errcnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].babblecnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].underflowcnt
					,usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].discardcnt);                                  
				}
				UVC_INFO("SKIP FRAME");
					
				usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].isIFrame = 0;
			}
#if defined(CONFIG_MODULE_USB_UVC_SUSPEND_TEST)
			if(usbh_uvc_mw.stream_id == 1) {
				if(test_frame_count < 50) {
#if defined(CONFIG_SNX_ISO_ACCELERATOR)
					if(usbh_uvc_mw.stream_xfr_type == USBH_ISO_IN_TYPE)
						uvc_stream_complete(&usbh_uvc_mw);
#endif
					++test_frame_count;
				}
			}
#else
#if defined(CONFIG_SNX_ISO_ACCELERATOR)
			if(usbh_uvc_mw.stream_xfr_type == USBH_ISO_IN_TYPE) {
				if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt == 12){
					usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[stream_idx].framecnt = 13;
				}
				uvc_stream_complete(&usbh_uvc_mw);
			}
#endif
#endif
			if(usbh_uvc_mw.stream_xfr_type == USBH_BK_IN_TYPE)
				uvc_clean_bk_buf();
	}
}

#if 0
void uvc_app_task(void *pvParameters){
	
	//USBH_UVC_INFO_Struct		*uvc_info = NULL;	
	USBH_Device_Structure			*uvc_dev 		= NULL;
	USBH_UVC_CLASS_Structure	*uvc_class 	= NULL;
	uint32_t			uvc_fmt = 0, uvc_res = 0, uvc_fps = 0;
	uint32_t			dev_id =0;
	uint32_t			i = 0;
	uint8_t				intf_start_idx = 0;
	
	while(1){
loop:
		xQueueReceive(usbh_queue_uvc_app, &dev_id, USBH_MAX);
		
		//uvc_info = (USBH_UVC_INFO_Struct*) uvc_get_info();
		//uvc_print_info_data(uvc_info);
		uvc_dev	= (USBH_Device_Structure*)usbh_uvc_init(dev_id);
		if(uvc_dev == NULL)
			continue;
		uvc_class = uvc_dev->CLASS_STRUCT_PTR;
		if(uvc_class->PROBE.dwStreamID != 0)
			continue;
		
		if (usbh_queue_uvc_mw == NULL) {
			usbh_queue_uvc_mw = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(USBH_UVC_MW_STRUCTURE));
		}		
		if(xTASK_HDL_UVC_STREAM_XFR == NULL){
			xTaskCreate(
				uvc_stream_xfr_task,
				( const	char * )"USBH_UVC_STERAM_XFR_TASK",
				16384,
				(void*)&usbh_uvc_app,
				250,
				&xTASK_HDL_UVC_STREAM_XFR
			);		
		}
		
		for(i=0;i<max_stream_count;i++){
			if (uvc_dev->CLASS_DRV == USBH_UVC_ISO_CLASS) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
				if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fmt, "h264") == 0){
					usbh_uvc_app.dev[dev_id].stream[i].size = H264FrameSize;
					uvc_fmt = USBH_UVC_STREAM_H264;
				}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fmt, "mjpeg") == 0){
					usbh_uvc_app.dev[dev_id].stream[i].size = MJPEGFrameSize;
					uvc_fmt = USBH_UVC_STREAM_MJPEG;
				}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fmt, "yuv") == 0){
					usbh_uvc_app.dev[dev_id].stream[i].size = YUVFrameSize;
					uvc_fmt = USBH_UVC_STREAM_YUV;
				} else {
					UVC_INFO("Format Not Support!");
					goto loop;
				}
#else	
				usbh_uvc_app.dev[dev_id].stream[i].size			=	((Standard_iTD_EP_Max_Count*3072*8)+(Max_STD_ISO_FrameSize*2)+4096);
				//usbh_uvc_app.dev[dev_id].stream[i].size		=	((Standard_iTD_EP_Max_Count*1024*8)+4096);
#endif
			
#if defined( CONFIG_SN_GCC_SDK )
				do{
					usbh_uvc_app.dev[dev_id].stream[i].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[dev_id].size,GFP_DMA,MODULE_DRI_USBH);
				}while (usbh_uvc_app.dev[dev_id].stream[i].ptr == NULL);
#endif // end of if defined( CONFIG_SN_GCC_SDK )
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				do{
					usbh_uvc_app.dev[dev_id].stream[i].ptr 		= 	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[dev_id].stream[i].size);
					UVC_INFO(" usbh_uvc_app.dev[%d].stream[%d].ptr = %x", dev_id, i, (uint32_t)usbh_uvc_app.dev[dev_id].stream[i].ptr); 				
				}while(usbh_uvc_app.dev[dev_id].stream[i].ptr == NULL);	
#endif	// end of if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
			} else if (uvc_dev->CLASS_DRV == USBH_UVC_BULK_CLASS) {
#if defined( CONFIG_SN_GCC_SDK )
				usbh_uvc_app.dev[dev_id].stream[i].size = USBH_UVC_BK_STREAM_BUF_SIZE * 2;
				do {
					usbh_uvc_app.dev[dev_id].stream[i].ptr = (uint32_t*) pvPortMalloc(usbh_uvc_app.dev[dev_id].stream[i].size, GFP_DMA, MODULE_DRI_USBH);
				} while (usbh_uvc_app.dev[dev_id].stream[i].ptr == NULL);
#endif	
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
				usbh_uvc_app.dev[dev_id].stream[i].size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;
				do{
					usbh_uvc_app.dev[dev_id].stream[i].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[dev_id].stream[i].size);
				} while(usbh_uvc_app.dev[dev_id].stream[i].ptr == NULL);
#endif
			} else if (uvc_dev->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
				if(i == 0) {
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
					usbh_uvc_app.dev[dev_id].stream[0].size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;
					do{
						usbh_uvc_app.dev[dev_id].stream[0].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[dev_id].stream[0].size);
					} while(usbh_uvc_app.dev[dev_id].stream[0].ptr == NULL);
#endif
				} else if (i == 1) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
					if(strcmp(usbh_uvc_app.dev[dev_id].stream[1].fmt, "h264") == 0){
						usbh_uvc_app.dev[dev_id].stream[1].size = H264FrameSize;
					}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[1].fmt, "mjpeg") == 0){
						usbh_uvc_app.dev[dev_id].stream[1].size = MJPEGFrameSize;				
					}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[1].fmt, "yuv") == 0){
						usbh_uvc_app.dev[dev_id].stream[1].size = YUVFrameSize;				
					}
#if defined( CONFIG_SN_GCC_SDK )
					do{
						usbh_uvc_app.dev[dev_id].stream[i].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[dev_id].size,GFP_DMA,MODULE_DRI_USBH);
					}while (usbh_uvc_app.dev[dev_id].stream[i].ptr == NULL);
#endif 
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
					do{
						usbh_uvc_app.dev[dev_id].stream[i].ptr 		= 	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[dev_id].stream[i].size);
						UVC_INFO(" usbh_uvc_app.dev[%d].stream[%d].ptr = %x", dev_id, i, (uint32_t)usbh_uvc_app.dev[dev_id].stream[i].ptr); 				
			
					}while(usbh_uvc_app.dev[dev_id].stream[i].ptr == NULL);	
#endif
#endif
				}
			}
			
			// parsing resolution
			if( (strcmp(usbh_uvc_app.dev[dev_id].stream[i].width, "1920") == 0) || (strcmp(usbh_uvc_app.dev[dev_id].stream[i].height, "1080")==0) ){
				uvc_res = USBH_UVC_STREAM_1920X1080;
			}else if( (strcmp(usbh_uvc_app.dev[dev_id].stream[i].width, "1280") == 0) || (strcmp(usbh_uvc_app.dev[dev_id].stream[i].height, "720")==0) ){
				uvc_res = USBH_UVC_STREAM_1280X720;			
			}else if( (strcmp(usbh_uvc_app.dev[dev_id].stream[i].width, "640") == 0) || (strcmp(usbh_uvc_app.dev[dev_id].stream[i].height, "480")==0) ){
				uvc_res = USBH_UVC_STREAM_640X480;			
			} else if( (strcmp(usbh_uvc_app.dev[dev_id].stream[i].width, "320") == 0) || (strcmp(usbh_uvc_app.dev[dev_id].stream[i].height, "240")==0) ) {
				uvc_res	= USBH_UVC_STREAM_320X240;
			} else if( (strcmp(usbh_uvc_app.dev[dev_id].stream[i].width, "160") == 0) || (strcmp(usbh_uvc_app.dev[dev_id].stream[i].height, "120")==0) ) {
				uvc_res	= USBH_UVC_STREAM_160X120;
			}else{
				UVC_INFO("Resolution Not Support!"); 
				goto loop;
			}

			// parsing fps
			if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fps, "30") == 0){
				uvc_fps = USBH_UVC_STREAM_30_FPS;
			}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fps, "15") == 0){
				uvc_fps = USBH_UVC_STREAM_15_FPS;
			}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fps, "10") == 0){
				uvc_fps = USBH_UVC_STREAM_10_FPS;
			}else if(strcmp(usbh_uvc_app.dev[dev_id].stream[i].fps, "5") == 0){
				uvc_fps = USBH_UVC_STREAM_5_FPS;				
			}else{
				UVC_INFO("FPS Not Support!"); 
				goto loop;
			}
			
			usbh_uvc_app.dev[dev_id].stream[i].stream_id   =	uvc_start(uvc_dev, uvc_fmt, uvc_res, uvc_fps,usbh_uvc_app.dev[dev_id].stream[i].ptr, usbh_uvc_app.dev[dev_id].stream[i].size, intf_start_idx);			
			UVC_INFO("The Stream ID is	: 0x%x",usbh_uvc_app.dev[dev_id].stream[i].stream_id);
		}
	}	
}
#endif

int32_t cmd_uvc_init(int argc, char* argv[])
{
	memset((void *)&usbh_uvc_app, 0, sizeof(USBH_UVC_APP_STRUCTURE));
	
	usbh_uvc_app.usb_preview_enable = 1;
	//usbh_uvc_app.sd_record_enable 	= 1;
	usbh_uvc_app.debug_enable				= 1;
	
#if defined (CONFIG_USBH_FREE_RTOS)
	if(usbh_queue_uvc_app == NULL){		
		usbh_queue_uvc_app = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE,4);	
	}
	
	if(usbh_queue_uvc_mw == NULL) {
		usbh_queue_uvc_mw = xQueueCreate(USBH_UVC_DATA_QUEUE_SIZE, sizeof(USBH_UVC_MW_STRUCTURE));
	}
	
	return pdPASS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if(usbh_queue_uvc_app == NULL){
		osMessageQDef(usbh_queue_uvc_app, USBH_UVC_DATA_QUEUE_SIZE, uint32_t);
		usbh_queue_uvc_app = osMessageCreate(osMessageQ(usbh_queue_uvc_app), NULL);
	}

	if(usbh_queue_uvc_mw == NULL) {
		osMessageQDef(usbh_queue_uvc_mw, USBH_UVC_DATA_QUEUE_SIZE, USBH_UVC_MW_STRUCTURE);
		usbh_queue_uvc_mw = osMessageCreate(osMessageQ(usbh_queue_uvc_mw), NULL);
	}
	
	return osOK;
#endif
}

int32_t cmd_uvc_get_info(int argc, char* argv[])
{
	USBH_UVC_INFO_Struct			*uvc_info = NULL;
		
	uvc_info	=	(USBH_UVC_INFO_Struct*) uvc_get_info();
	uvc_print_info_data(uvc_info);

#if defined (CONFIG_USBH_FREE_RTOS)
	return pdPASS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return osOK;
#endif
}
	
uint8_t uvc_open_stream(USBH_Device_Structure *uvc_dev, uint8_t device_id, uint8_t stream_idx) {
			
	uint32_t			uvc_fmt = 0, uvc_res = 0, uvc_fps = 0;
	uint8_t				intf_start_idx = 0;
	uint32_t			temp_iso_size = 0, temp_bk_size = 0;
	
	if (uvc_dev->CLASS_DRV == USBH_UVC_ISO_CLASS) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "h264") == 0){
			usbh_uvc_app.dev[device_id].stream[stream_idx].size = H264FrameSize;
		}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "mjpeg") == 0){
			usbh_uvc_app.dev[device_id].stream[stream_idx].size = MJPEGFrameSize;
		}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "yuv") == 0){
			usbh_uvc_app.dev[device_id].stream[stream_idx].size = YUVFrameSize;
		} else {
			UVC_INFO("Format Not Support!");
			return FAIL;
		}
#else
		usbh_uvc_app.dev[device_id].stream[stream_idx].size			=	((Standard_iTD_EP_Max_Count*3072*8)+(Max_STD_ISO_FrameSize*2)+4096);
		//usbh_uvc_app.dev[dev_id].stream[i].size		=	((Standard_iTD_EP_Max_Count*1024*8)+4096);
#endif
	
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr 		= 	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size);
			UVC_INFO("usbh_uvc_app.dev[%d].stream[%d].ptr = %x", device_id, stream_idx, (uint32_t)usbh_uvc_app.dev[device_id].stream[stream_idx].ptr); 				
		}while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);	
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = (uint32_t *)RF_USBH_Buf_PTR;
		while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL) {
			osDelay(USBH_100ms);
		}
		RF_USBH_Buf_PTR += usbh_uvc_app.dev[device_id].stream[stream_idx].size;
#endif
#endif
	} else if (uvc_dev->CLASS_DRV == USBH_UVC_BULK_CLASS) {
		usbh_uvc_app.dev[device_id].stream[stream_idx].size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
		do {
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = (uint32_t*) pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size, GFP_DMA, MODULE_DRI_USBH);
		} while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size);
		} while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = (uint32_t *)RF_USBH_Buf_PTR;
		while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL) {
			osDelay(USBH_100ms);
		}
		RF_USBH_Buf_PTR += usbh_uvc_app.dev[device_id].stream[stream_idx].size;
#endif
#endif
	} else if (uvc_dev->CLASS_DRV == (USBH_UVC_BULK_CLASS | USBH_UVC_ISO_CLASS)) {
#if defined( CONFIG_SNX_ISO_ACCELERATOR )
		if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "h264") == 0){
			temp_iso_size = H264FrameSize;
		}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "mjpeg") == 0){
			temp_iso_size = MJPEGFrameSize;
		}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "yuv") == 0){
			temp_iso_size = YUVFrameSize;
		} else {
			UVC_INFO("Format Not Support!");
			return FAIL;
		}
#else
		temp_iso_size		=	((Standard_iTD_EP_Max_Count*3072*8)+(Max_STD_ISO_FrameSize*2)+4096);
#endif
		
		temp_bk_size = USBH_UVC_BK_STREAM_BUF_SIZE * 2 + 40*1024*3 + 0x1000;
		
		(temp_iso_size > temp_bk_size) ? (usbh_uvc_app.dev[device_id].stream[stream_idx].size = temp_iso_size) : 
		(usbh_uvc_app.dev[device_id].stream[stream_idx].size = temp_bk_size);
		
#if defined (CONFIG_USBH_FREE_RTOS)
#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) || defined( CONFIG_XILINX_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr 		= 	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].stream[stream_idx].size);
			UVC_INFO("usbh_uvc_app.dev[%d].stream[%d].ptr = %x", device_id, stream_idx, (uint32_t)usbh_uvc_app.dev[device_id].stream[stream_idx].ptr); 				
		}while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);	
#endif
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_GCC_SDK )
		do{
			usbh_uvc_app.dev[device_id].stream[stream_idx].ptr		=	(uint32_t*)pvPortMalloc(usbh_uvc_app.dev[device_id].size, GFP_DMA, MODULE_DRI_USBH);
		}while (usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = (uint32_t *)RF_USBH_Buf_PTR;
		while(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr == NULL) {
			osDelay(USBH_100ms);
		}
		RF_USBH_Buf_PTR += usbh_uvc_app.dev[device_id].stream[stream_idx].size;
#endif
#endif
	}
	
	if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "h264") == 0){
		uvc_fmt = USBH_UVC_STREAM_H264;
	}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "mjpeg") == 0){
		uvc_fmt = USBH_UVC_STREAM_MJPEG;
	}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fmt, "yuv") == 0){	
		uvc_fmt = USBH_UVC_STREAM_YUV;
	} else {
		UVC_INFO("Format Not Support!");
		goto ERR;
	}
	
	if( (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "1920") == 0) && (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "1080") == 0) ){
		uvc_res = USBH_UVC_STREAM_1920X1080;
	}else if( (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "1280") == 0) && (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "720") == 0) ){
		uvc_res = USBH_UVC_STREAM_1280X720;			
	}else if( (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "640") == 0) && (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "480") == 0) ){
		uvc_res = USBH_UVC_STREAM_640X480;			
	} else if( (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "320") == 0) && (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "240") == 0) ) {
		uvc_res	= USBH_UVC_STREAM_320X240;
	} else if( (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].width, "160") == 0) && (strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].height, "120") == 0) ) {
		uvc_res	= USBH_UVC_STREAM_160X120;
	}else{
		UVC_INFO("Resolution Not Support!"); 
		goto ERR;
	}

	// parsing fps
	if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "30") == 0){
		uvc_fps = USBH_UVC_STREAM_30_FPS;
	}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "15") == 0){
		uvc_fps = USBH_UVC_STREAM_15_FPS;
	}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "10") == 0){
		uvc_fps = USBH_UVC_STREAM_10_FPS;
	}else if(strcmp(usbh_uvc_app.dev[device_id].stream[stream_idx].fps, "5") == 0){
		uvc_fps = USBH_UVC_STREAM_5_FPS;				
	}else{
		UVC_INFO("FPS Not Support!"); 
		goto ERR;
	}			
	
	usbh_uvc_app.dev[device_id].stream[stream_idx].stream_id = uvc_start(uvc_dev, uvc_fmt, uvc_res, uvc_fps,usbh_uvc_app.dev[device_id].stream[stream_idx].ptr, usbh_uvc_app.dev[device_id].stream[stream_idx].size, intf_start_idx);			
	UVC_INFO("The Stream ID is	: 0x%x",usbh_uvc_app.dev[device_id].stream[stream_idx].stream_id);
	
	if(usbh_uvc_app.dev[device_id].stream[stream_idx].stream_id != 0)
		return SUCCESS;
	else
		return FAIL;
	
	ERR:
	if(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr != NULL){
#if defined (CONFIG_USBH_FREE_RTOS)
		vPortFree(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr);
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_GCC_SDK )
		vPortFree(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr);
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		RF_USBH_Buf_PTR -= usbh_uvc_app.dev[device_id].stream[stream_idx].size;
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
#endif
#endif
	}
	
	return FAIL;
}

int32_t cmd_uvc_start(int argc, char* argv[]) {
	uint8_t status = FAIL;
	USBH_Device_Structure 			*uvc_dev = NULL;
	uint8_t i = 0, device_id = 0, stream_idx = 0;
		
	if( argc < 6 )
		goto hint;
	
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
	if(RF_USBH_Buf_PTR == 0)
		goto err_hint;
#endif	
#endif
	
	device_id = atoi(argv[1]);
	if(device_id < 5){
	}
#if defined( CONFIG_DUAL_HOST )
	else if(device_id < 10) {
	}
#endif
	else
		goto dev_hint;
	
	uvc_dev = (USBH_Device_Structure*)usbh_uvc_init(device_id);
	if(uvc_dev == NULL) {
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueReceive(usbh_queue_uvc_app, NULL, USBH_3000ms);
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
		osMessageGet(usbh_queue_uvc_app, NULL, USBH_3000ms);
#endif
		
		uvc_dev = (USBH_Device_Structure*)usbh_uvc_init(device_id);
		if(uvc_dev == NULL)
			goto dev_hint;
	}
	
	for(i = 0; i < max_stream_count; i++) {
		if(usbh_uvc_app.dev[device_id].stream[i].isOpen == 0) {
			stream_idx = i;
			break;
		}
	}
	
	if(i == max_stream_count)
		goto err_hint;

#if defined (CONFIG_CLI_SUPPORT_USBD_PREV)
	if((strcmp(argv[6], "PREVIEW") == 0) || (strcmp(argv[6], "preview") == 0)){		
		if(device_id < 5){
			for(i = 0; i < max_stream_count; i++) {
				if(usbh_uvc_app.dev[device_id].stream[i].usb_preview)
					break;
			}
		
			if(i == max_stream_count) {
				usbh_uvc_app.dev[device_id].stream[stream_idx].usb_preview = 1;
			
				if(!usbd_inited) {
					//USBD_VariableInit(0x600000L);
					//usbd_drv_task_init();
					usbd_inited = 1;
				}
			} else {
				UVC_INFO("Another stream previewing!");
			}
		}
	}
#endif

#if defined (CONFIG_RECORD)	
	if((strcmp(argv[6], "RECORD") == 0) || (strcmp(argv[6], "record") == 0)){
		rec_filemanage_init();
		init_uvc_record_task();
		usbh_uvc_app.dev[device_id].stream[stream_idx].sd_record = 1;
	}
#endif	
	
	if(argc == 8)
		if((strcmp(argv[7], "DEBUG") == 0) || (strcmp(argv[7], "debug") == 0)){
			usbh_uvc_app.dev[device_id].stream[stream_idx].debug_msg = 1;
		}
	
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].fmt,argv[2],8);
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].width,argv[3],8);
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].height,argv[4],8);
	strncpy((char *)usbh_uvc_app.dev[device_id].stream[stream_idx].fps,argv[5],8);
		
	if((strcmp(argv[2], "H264") == 0) || (strcmp(argv[2], "h264") == 0)){
		usbh_uvc_app.dev[device_id].stream[stream_idx].isH264 = 1;
	}

	status = uvc_open_stream(uvc_dev, device_id, stream_idx);
	
	if(status == FAIL)
		goto err_hint;
	
	usbh_uvc_app.dev[device_id].stream[stream_idx].isOpen = 1;
	
#if(_SUPPORT_USBD_WIFI_PREVIEW)
	usbh_uvc_app.socket_fd = wifi_stream_init(WIFI_STREAM_IPADDR,WIFI_STREAM_PORT);
	if ( usbh_uvc_app.socket_fd  < 0 )
  {
        printf("Wifi stream socket call failed");
  }
#endif

#if defined (CONFIG_USBH_FREE_RTOS)
	if(xTASK_HDL_UVC_STREAM_XFR == NULL){
		xTaskCreate(
			uvc_stream_xfr_task,
			( const	char * )"USBH_UVC_STERAM_XFR_TASK",
			16384,
			(void*)&usbh_uvc_app,
			250,
			&xTASK_HDL_UVC_STREAM_XFR
		);
	}
	
	return pdPASS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	if(xTASK_HDL_UVC_STREAM_XFR == NULL){
			osThreadDef(USBH_UVC_STERAM_XFR_TASK, uvc_stream_xfr_task, THREAD_PRIO_USBH_PROCESS, 1, 16384);
			xTASK_HDL_UVC_STREAM_XFR = osThreadCreate(osThread(USBH_UVC_STERAM_XFR_TASK), &usbh_uvc_app);
			if(xTASK_HDL_UVC_STREAM_XFR == NULL )
				printf("Create xTASK_HDL_UVC_STREAM_XFR fail\n");	
	}
	
	return osOK;
#endif
	
hint:
	UVC_INFO("\r\n usage : uvc_start devid fmt resX resY fps (preview)");
	UVC_INFO("  ex : uvc_start 0 h264 1280 720 30 preview");
#if defined (CONFIG_USBH_FREE_RTOS)
	return pdFAIL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return (!osOK);
#endif
	
dev_hint:
	UVC_INFO("\r\rn ERROR : Device Not Exist!");
	memset(&usbh_uvc_app.dev[device_id].stream[stream_idx], 0, sizeof(USBH_UVC_APP_STREAM_STRUCTURE));
#if defined (CONFIG_USBH_FREE_RTOS)
	return pdFAIL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return (!osOK);
#endif
	
err_hint:
	UVC_INFO("\r\n ERROR : Cannot Open UVC Stream!");
	memset(&usbh_uvc_app.dev[device_id].stream[stream_idx], 0, sizeof(USBH_UVC_APP_STREAM_STRUCTURE));
#if defined (CONFIG_USBH_FREE_RTOS)
	return pdFAIL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return (!osOK);
#endif
}

int32_t cmd_uvc_stop(int argc, char* argv[]) {
	uint8_t status = FAIL;
	USBH_Device_Structure 			*uvc_dev = NULL;
	uint8_t device_id = 0, stream_id = 0, stream_idx = 0;
	
	if(argc != 2)
		goto hint;
	
	stream_id = atoi(argv[1]);
	
	device_id = uvc_streamid_to_devid(stream_id);
	if(device_id == 0xFF)
		goto err_hint;
	
	uvc_dev	= (USBH_Device_Structure*)usbh_uvc_init(device_id);
	if((uvc_dev != NULL) && (stream_id > 0)){
		status = uvc_stop(uvc_dev, stream_id);
		if(status == FAIL)
			goto err_hint;
	} else {
		goto err_hint;
	}
	
#if defined( CONFIG_RECORD )
	if(usbh_uvc_app.dev[device_id].stream[stream_idx].sd_record){
		record_set_stop(uvc_record_info->pRecord_info, 1);
		record_uninit(uvc_record_info->pRecord_info);
	}
#endif
	
	uvc_unregister_streamid(device_id, stream_id);
	
	stream_idx = streamid_to_streamidx(device_id, stream_id);

	if(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr != NULL){
#if defined (CONFIG_USBH_FREE_RTOS)
		vPortFree(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr);
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
#if defined( CONFIG_SN_GCC_SDK )
		vPortFree(usbh_uvc_app.dev[device_id].stream[stream_idx].ptr);
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined( CONFIG_PLATFORM_ST53510 )
		RF_USBH_Buf_PTR -= usbh_uvc_app.dev[device_id].stream[stream_idx].size;
		usbh_uvc_app.dev[device_id].stream[stream_idx].ptr = NULL;
#endif
#endif
	}
	
#if(_SUPPORT_USBD_WIFI_PREVIEW)
	if(usbh_uvc_app.socket_fd >0){
		if(wifi_stream_destory(usbh_uvc_app.socket_fd)<0)
    {
        printf("Wifi stream socket close failed");
    }else{
				usbh_uvc_app.socket_fd = 0;
		}
	}
#endif

	UVC_INFO("\r\nTotal Frame count = %d", usbh_uvc_app.dev[device_id].stream[stream_idx].framecnt);

	memset(&usbh_uvc_app.dev[device_id].stream[stream_idx], 0, sizeof(USBH_UVC_APP_STREAM_STRUCTURE));
	
#if defined (CONFIG_USBH_FREE_RTOS)
	return pdPASS;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return osOK;
#endif
	
hint:
	UVC_INFO("\r\n usage : uvc_stop StreamID");
	UVC_INFO("  ex : uvc_stop 1");
#if defined (CONFIG_USBH_FREE_RTOS)
	return pdFAIL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return (!osOK);
#endif
	
err_hint:
	UVC_INFO("\r\n Error : Cannot Stop UVC Stream !");
#if defined (CONFIG_USBH_FREE_RTOS)
	return pdFAIL;
#endif
#if defined (CONFIG_USBH_CMSIS_OS)
	return (!osOK);
#endif
}

#if defined( CONFIG_RECORD )
int init_uvc_record_task(void)
{
	if (pdPASS != xTaskCreate(uvc_record_task, "TASK_UVC_REC", 1536, NULL, 35, NULL)) {
		UVC_DBG("init_record_task : could not create uvc_record_task !!\n");
		return (-1);
	}
	
	return 0;
}

static void uvc_record_task(void *pvParameters)
{
	int ret;
	
	uvc_record_info = rec_info_init();
	if( uvc_record_info == NULL )
		goto end;
		
	rec_video_init();
	
	/*****init rec*****/
	ret = rec_record_init(uvc_record_info);
	if (ret == pdFAIL) {
		UVC_DBG("uvc_record_flow : rec_record_init fail !!\n");
		goto finally;
	}

	rec_filenode_update(uvc_record_info->type);
	if (pdPASS != xTaskCreate(uvc_record_video_task, "TASK_UVC_REC_VDO", 512, NULL, 40, NULL))
	{
		UVC_DBG("Could not create TASK_UVC_REC_VDO !! \n");
		goto finally;
	}	
	
	uvc_automount_info = get_automount_info();
	
	for (;;) {
		if (uvc_record_info->recordclose == 1) {
			goto finally;
		}
				
		uvc_record_info->readfilelistok = 0;
		uvc_record_info->sdcardseed = 0;
		if (xSemaphoreTake(uvc_automount_info->SdRecordMutex, portMAX_DELAY) == pdTRUE) {
			add_rec_status(uvc_record_info, RECORD_START_RUNNING);
			rec_filenode_update(uvc_record_info->type);
			
			record_writebufreset(uvc_record_info->pRecord_info);
			uvc_record_info->sdcardisfull = 0;

			record_set_start(uvc_record_info->pRecord_info, 1);
		}
	}

finally:
		
	if (uvc_record_info->pRecord_info != NULL) {
		record_uninit(uvc_record_info->pRecord_info);
		uvc_record_info->pRecord_info = NULL;
	}
	snx_fm_release_filelist(uvc_record_info->type, NULL);
	if (uvc_record_info != NULL) {
		if (uvc_record_info->rm_file_task != NULL) {
			vTaskDelete(uvc_record_info->rm_file_task);
			uvc_record_info->rm_file_task = NULL;
		}
		if (uvc_record_info->rm_queue != NULL) {
			vQueueDelete(uvc_record_info->rm_queue);
			uvc_record_info->rm_queue = NULL;
		}
		rec_info_uninit(&uvc_record_info);
	}

end:
//	record_is_running(0);
	vTaskDelete(NULL);
}

void uvc_record_video_task( void *pvParameters )
{
	unsigned char *pFrame;
	unsigned int uiFrameSize;
	unsigned char IFrame = 1;
	int frame_num = 0;
	int FrameType = 0;
	
	while(1)
	{
//		if(video_info.ucStreamMode|REC_FMT_H264)
//		{
			
			if(usbh_uvc_app.dev[usbh_uvc_mw.dev_id].stream[usbh_uvc_mw.stream_id-1].sd_record){
					((FrameType = snx_avc_get_slice_type((unsigned char*)usbh_uvc_mw.ptr, usbh_uvc_mw.size)) == TYPE_I) ? (IFrame = 1) : (IFrame = 0);
					if((FrameType == TYPE_I) || (FrameType == TYPE_P)) {	
							record_video(uvc_record_info->pRecord_info, IFrame, (unsigned char *)usbh_uvc_mw.ptr, usbh_uvc_mw.size);
					}
			}	
			vTaskDelay(100 / portTICK_RATE_MS);
//		}
//		else
//			vTaskDelay(10 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

#endif	//end of
#endif
