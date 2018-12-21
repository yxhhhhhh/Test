
#include "sonix_config.h"

#if defined(CONFIG_CLI_MSC)	&& defined (CONFIG_MODULE_USB_MSC_CLASS)

#if defined (CONFIG_USBH_FREE_RTOS)
#include <FreeRTOS.h>
#endif

#include <stdio.h>
#include <string.h>
#include "USBH.h"
#include "USBH_MSC.h"
#include "cmd_msc.h"

#define			CMD_RW_TEST		1
#define			CMD_CR_TEST		2

#if defined (CONFIG_USBH_FREE_RTOS)
TaskHandle_t	xTASK_CLI_MSC_TEST = NULL;
TaskHandle_t	xTASK_CLI_CARD_IN = NULL;
TaskHandle_t	xTASK_CLI_MSC_RW_TEST[USBH_MAX_PORT*2] = {NULL};
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
osThreadId		xTASK_CLI_MSC_TEST = NULL;
osThreadId		xTASK_CLI_CARD_IN = NULL;
osThreadId		xTASK_CLI_MSC_RW_TEST[USBH_MAX_PORT*2] = {NULL};
#endif

uint8_t				usbh_inited = 0;
uint8_t				card_reader_device_id = 0;
uint8_t				msc_rw_test_host = 0;
uint32_t			msc_rw_test_times = 0;
uint32_t			msc_rw_test_sector_count = 0;


void msc_card_in_task(void *pvParameters)
{
	uint8_t						card_in_lun = 0;
	uint8_t 					device_id = 0;
	MSC_DEVICE_STRUCT *msc_dev = NULL;
	
	device_id = *((uint8_t*)pvParameters);
	
	for( ;; )
	{
		msc_dev = (MSC_DEVICE_STRUCT*) get_msc_device_info(device_id);
		
#if defined (CONFIG_USBH_FREE_RTOS)					
		if( xQueueReceive(msc_dev->QUEUE_CARD_IN, &card_in_lun, USBH_MAX) )
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
		osMessageGet(msc_dev->QUEUE_CARD_IN, &card_in_lun, USBH_MAX);
#endif		
		{
				MSC_DBG("msc_card_in_task : lun %d card in !!", card_in_lun);
				MSC_DBG("msc_card_in_task : lun %d capactiy  = %d !!", card_in_lun, msc_dev->msc_lun_info[card_in_lun].lun_capacity);
				MSC_DBG("msc_card_in_task : lun %d block len = %d !!", card_in_lun, msc_dev->msc_lun_info[card_in_lun].lun_block_len);
		}
				
#if defined (CONFIG_USBH_FREE_RTOS)		
		vTaskDelay(USBH_100ms); 
#endif
		
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_100ms);
#endif
		
	}
}


void msc_rw_test_task(void *pvParameters)
{	
	uint8_t device_id = 0;
	
	device_id = *((uint8_t*)pvParameters);
	//MSC_DBG("msc_rw_test_task : device_id = %d, %d, %d\n", device_id, msc_rw_test_times, msc_rw_test_sector_count);
	
	if( msc_random_test(device_id, msc_rw_test_times, msc_rw_test_sector_count, 1) == FAIL )
	{
		MSC_DBG("device %d R/W TEST FAIL !!", device_id);
	}
	else
	{
		MSC_DBG("device %d R/W TEST OK !!", device_id);
	}
	
	for(;;)
	{
#if defined (CONFIG_USBH_FREE_RTOS)		
		vTaskDelay(USBH_100ms); 
#endif
		
#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_100ms);
#endif		
	}
}


void msc_card_reader_test(uint8_t device_id)
{
#if defined (CONFIG_USBH_FREE_RTOS)	
	xTaskCreate(
		msc_card_in_task,
		(const char*)"MSC_CARD_IN_TASK",
		256,
		(void*)&device_id,
		200,
		&xTASK_CLI_CARD_IN
	);
#endif
		
#if defined (CONFIG_USBH_CMSIS_OS)
	osThreadDef(MSC_CARD_IN_TASK, msc_card_in_task, 200, 1, 256);	
	xTASK_CLI_CARD_IN = osThreadCreate(osThread(MSC_CARD_IN_TASK), &device_id);	
#endif
	
	if( xTASK_CLI_CARD_IN == NULL )
		printf("Create xTASK_CLI_CARD_IN Fail\n");	
}


void msc_rw_test(uint8_t device_id)
{
	char 		task_name[256] = {'\0'};
	
	sprintf((char *)&task_name, "%s%d", "MSC_RW_TEST_TASK", device_id);	
	
#if defined (CONFIG_USBH_FREE_RTOS)			
	xTaskCreate(
		msc_rw_test_task,
		(const char*)&task_name[0],
		256,
		(void*)&device_id,
		100,
		&xTASK_CLI_MSC_RW_TEST[device_id]
	);
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
	osThreadDef(task_name, msc_rw_test_task, 200, 1, 256);	
	xTASK_CLI_MSC_RW_TEST[device_id] = osThreadCreate(osThread(task_name), &device_id);	
#endif
	
	if( xTASK_CLI_MSC_RW_TEST[device_id] == NULL )
		printf("Create xTASK_CLI_MSC_RW_TEST Fail\n");
}


void msc_get_device_task(void *pvParameters)
{
	uint8_t	cmd_type = 0;
	uint8_t device_id = 0;
	
	cmd_type = *((uint8_t*)pvParameters);	
	
	if( USBH_QUEUE_MSC_TEST != NULL )
	{
#if defined (CONFIG_USBH_FREE_RTOS)
		xQueueReceive(USBH_QUEUE_MSC_TEST, &device_id, portMAX_DELAY);
#endif		
		
#if defined (CONFIG_USBH_CMSIS_OS)
	osMessageGet(USBH_QUEUE_MSC_TEST, &device_id, USBH_MAX);
#endif
		
		if( cmd_type == CMD_RW_TEST )
			msc_rw_test(device_id);
		
		if( cmd_type == CMD_CR_TEST )
			msc_card_reader_test(device_id);
	}
	
	for(;;)
	{
#if defined (CONFIG_USBH_FREE_RTOS)		
		vTaskDelay(USBH_100ms); 
#endif		

#if defined (CONFIG_USBH_CMSIS_OS)
		osDelay(USBH_100ms);
#endif		
	}	
}


void	msc_test_init(uint8_t cmd_type)
{		
#if defined (CONFIG_USBH_FREE_RTOS)		
	if( USBH_QUEUE_MSC_TEST == NULL )
	{
		USBH_QUEUE_MSC_TEST	=	xQueueCreate(USBH_MSC_DATA_QUEUE_SIZE, 4);
	}
#endif	

#if defined (CONFIG_USBH_CMSIS_OS)
	if(USBH_QUEUE_MSC_TEST == NULL)
	{
		osMessageQDef(USBH_QUEUE_MSC_TEST, USBH_MSC_DATA_QUEUE_SIZE, uint32_t);
		USBH_QUEUE_MSC_TEST = osMessageCreate(osMessageQ(USBH_QUEUE_MSC_TEST), NULL);
	}
#endif
	
#if defined (CONFIG_USBH_FREE_RTOS)	
	xTaskCreate(
		msc_get_device_task,
		(const char*)"MSC_GET_DEVICE_TASK",
		256,
		(void*)&cmd_type,
		200,
		&xTASK_CLI_MSC_TEST
	);
#endif		

#if defined (CONFIG_USBH_CMSIS_OS)
	osThreadDef(MSC_GET_DEVICE_TASK, msc_get_device_task, 200, 1, 256);	
	xTASK_CLI_MSC_TEST = osThreadCreate(osThread(MSC_GET_DEVICE_TASK), &cmd_type);	
#endif
	
	if( xTASK_CLI_MSC_TEST == NULL )
	{
		printf("Create xTASK_CLI_MSC_TEST Fail !!\n");
	}
}


int32_t cmd_msc_rw_test(int argc, char* argv[])
{	
	uint8_t 	host = 0;
	
	if( argc < 3 )
	{
		printf("Argument Error !!");
#if defined (CONFIG_USBH_FREE_RTOS)			
		return pdFAIL;
#endif
		
#if defined (CONFIG_USBH_CMSIS_OS)		
		return osErrorOS;
#endif
	}
	
	host = atoi(argv[1]);
#if defined( CONFIG_DUAL_HOST )			
	if( (host != 1) && (host != 2) )
		host = 1;
#else
	if( host != 1 )
		host = 1;
#endif
	
	msc_rw_test_host = host;
	msc_rw_test_times = atoi(argv[2]);
	msc_rw_test_sector_count = atoi(argv[3]);

	msc_test_init(CMD_RW_TEST);
	
//	if( !usbh_inited )
//	{
//		if( msc_rw_test_host == 1 )
//		{
//			usbh_freertos_init();		
//		}
//		else
//		{
//#if defined( CONFIG_DUAL_HOST )						
//			if( msc_rw_test_host == 2 )
//				usbh_freertos_init_2();			
//			else
//				usbh_freertos_init();	
//#endif					
//		}
//		usbh_inited = 1;
//	}	

#if defined (CONFIG_USBH_FREE_RTOS)		
		return pdPASS;
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
		return osOK;
#endif	
}


int32_t cmd_msc_card_reader_test(int argc, char* argv[])
{
	uint8_t host = 0;
	
	if( argc < 1 )
	{
		printf("Argument Error !!");
#if defined (CONFIG_USBH_FREE_RTOS)			
		return pdFAIL;
#endif
		
#if defined (CONFIG_USBH_CMSIS_OS)		
		return osErrorValue;
#endif		
	}
	
	host = atoi(argv[1]);
#if defined( CONFIG_DUAL_HOST )			
	if( (host != 1) && (host != 2) )
		host = 1;
#else
	if( host != 1 )
		host = 1;
#endif

	msc_test_init(CMD_CR_TEST);
			
#if defined (CONFIG_USBH_FREE_RTOS)	
	return pdPASS;
#endif

#if defined (CONFIG_USBH_CMSIS_OS)
	return osOK;
#endif	
}

#endif
