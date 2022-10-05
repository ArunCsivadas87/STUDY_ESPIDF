#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include "driver/gpio.h"
#include "freertos/queue.h"
#include <esp_system.h>
#include <esp_spi_flash.h>

#define _256_Byte	1<<8
#define _512_Byte	1<<9
#define _1K_Byte	1<<10
#define _2K_Byte	1<<11
#define _4K_Byte	1<<12

#define DnPin GPIO_NUM_34
#define UpPin GPIO_NUM_35
#define OkPin GPIO_NUM_27
#define FhPin GPIO_NUM_14

#define ESP_INTR_FLAG_DEFAULT 0

//TaskHandle_t myTask1Handler=NULL;
TaskHandle_t myTask2Handler=NULL;
TaskHandle_t myIntTaskHandle =NULL;

static  QueueHandle_t GpioISRQueue =NULL;
uint32_t GlobalVaribale=0;
//unsigned int count;
bool ButtonFlag=false;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	uint32_t gpioNum=(int)(int *)arg;
	gpio_isr_handler_remove(gpioNum);
	xQueueSendFromISR(GpioISRQueue,&gpioNum,NULL);
}
void myTask2(void * p)
{
	uint32_t GpioPin=(int)(int *)p;
	vTaskDelay(200/portTICK_RATE_MS);
	gpio_isr_handler_add(GpioPin, gpio_isr_handler, (void*) GpioPin);
	if( myTask2Handler != NULL )vTaskDelete( myTask2Handler );
	//vTaskSuspend(NULL);
}
void MyInterrupTask(void * p)
{
	uint32_t gpioNum;
	while(1)
	{
		if(xQueueReceive(GpioISRQueue, &gpioNum,portMAX_DELAY))
		{
			printf("GPIO[%d] intr,val:%d GlobalVaribale=%d\r\n",gpioNum,gpio_get_level(gpioNum),GlobalVaribale++);
			xTaskCreate(myTask2,"myTask2Handle",_4K_Byte,(void*)gpioNum,tskIDLE_PRIORITY,&myTask2Handler);
		}
	}
}



void ConfigInterrupt(uint64_t GpioPin)
{

	gpio_config_t io_conf = {};
	io_conf.intr_type 		= GPIO_INTR_POSEDGE;//interrupt of rising edge
	io_conf.pin_bit_mask 	= io_conf.pin_bit_mask |(1ULL<<GpioPin);//bit mask of the pins, use GPIO34/35 here
	io_conf.mode 			= GPIO_MODE_INPUT;//set as input mode
	io_conf.pull_up_en 		= 1;//enable pull-up mode
	gpio_config(&io_conf);
}
void FetchChipInfo(void)
{

	esp_chip_info_t chip_data;
	esp_chip_info(&chip_data);
	printf("Model				: %s\r\n",		(chip_data.model==1)?"CHIP_ESP32":
												(chip_data.model==2)?"CHIP_ESP32S2":
												(chip_data.model==4)?"CHIP_ESP32S3":
												(chip_data.model==5)?"CHIP_ESP32C3":"Unknown");
	printf("Silicon Revision 		: Ver %d\r\n",			chip_data.revision);
	printf("No of CPU core			: %d\r\n",	chip_data.cores);
	printf("Flash size			: %dMB %s\r\n",	spi_flash_get_chip_size()/(1024*1024),
												(chip_data.features&CHIP_FEATURE_EMB_FLASH)?"EMBBEDED FLASH":"EXTERNAL");
	if(chip_data.features&(1<<1|1<<4|1<<5))
	printf("Available features 		: %s,%s,%s \r\n",
												(chip_data.features&CHIP_FEATURE_WIFI_BGN)?"WIFI_BGN":"",
												(chip_data.features&CHIP_FEATURE_BLE)?"LowEnergy":"",
												(chip_data.features&CHIP_FEATURE_BT)?"Classic BlueTooth": "");

	uint8_t MacId[6];
	esp_efuse_mac_get_default(MacId);
	printf("MAC ID 				: %02x:%02x:%02x:%02x:%02x:%02x  \r\n",MacId[0],MacId[1],MacId[2],MacId[3],MacId[4],MacId[5]);
	printf("EspIDF Ver 			: %s \r\n",esp_get_idf_version());

}

void app_main(void)
{
	int val=10;
	FetchChipInfo();
//	xTaskCreate(myTask1,"myTask1Handle",1<<12,(void*)1000,tskIDLE_PRIORITY,&myTask1Handler);
//	xTaskCreate(myTask2,"myTask2Handle",1<<9,(void*)0,tskIDLE_PRIORITY,&myTask2Handler);
	xTaskCreate(MyInterrupTask,"myInterruptTaskHandle",_2K_Byte,(void*)val,tskIDLE_PRIORITY,&myIntTaskHandle);
	GpioISRQueue = xQueueCreate(10, sizeof(uint32_t));
	ConfigInterrupt(DnPin);
	ConfigInterrupt(UpPin);
	ConfigInterrupt(OkPin);
	ConfigInterrupt(FhPin);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(DnPin, gpio_isr_handler, (void*) DnPin);
	gpio_isr_handler_add(UpPin, gpio_isr_handler, (void*) UpPin);
	gpio_isr_handler_add(OkPin, gpio_isr_handler, (void*) OkPin);
	gpio_isr_handler_add(FhPin, gpio_isr_handler, (void*) FhPin);

	while(1)
		{
		vTaskDelay(configTICK_RATE_HZ);
//		if(ButtonFlag)
//			{
//				ButtonFlag=false;
//				printf("You pressed Down Button %d Times\r\n",count);
//			}
		}
}
