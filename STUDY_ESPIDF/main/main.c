#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include "driver/gpio.h"


#define DnPin GPIO_NUM_34
#define ESP_INTR_FLAG_DEFAULT 0

TaskHandle_t myTask1Handler=NULL;
TaskHandle_t myTask2Handler=NULL;
TaskHandle_t myIntTaskHandle =NULL;
unsigned int count;
bool ButtonFlag=false;

void myTask1(void * p)
{
	int myTask1Cnt=0,PassParameters=(int)(int *)p;
	if(PassParameters<1)PassParameters=1;
	while(1)
	{
		vTaskDelay(PassParameters*configTICK_RATE_HZ/1000);

		TaskHandle_t t=xTaskGetCurrentTaskHandle();
		printf("%s executed %d Times\r\n",pcTaskGetName(t),myTask1Cnt++);
		if(myTask1Cnt==10)
			{
				vTaskDelete(myTask2Handler);
			vTaskDelete(t);

			}
	}
}
void myTask2(void * p)
{

	while(1)
	{
		vTaskDelay(5000*configTICK_RATE_HZ/1000);
		vTaskSuspend(myTask1Handler);
		vTaskDelay(5000*configTICK_RATE_HZ/1000);
		vTaskResume(myTask1Handler);
	}
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	BaseType_t checkIfYeildRequired;
	checkIfYeildRequired=xTaskResumeFromISR(myIntTaskHandle);
	portYIELD_FROM_ISR(checkIfYeildRequired);
}
void MyInterrupTask(void * p)
{
	while(1)
			{
				vTaskSuspend(NULL);			//vTaskDelay(1000);
				count++;
				ButtonFlag =true;
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

void app_main(void)
{
	int val=10;
	xTaskCreate(myTask1,"myTask1Handle",1<<12,(void*)1000,tskIDLE_PRIORITY,&myTask1Handler);
	xTaskCreate(myTask2,"myTask2Handle",1<<9,(void*)0,tskIDLE_PRIORITY,&myTask2Handler);
	xTaskCreate(MyInterrupTask,"myInterruptTaskHandle",1<<9,(void*)val,tskIDLE_PRIORITY,&myIntTaskHandle);
	ConfigInterrupt(DnPin);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(DnPin, gpio_isr_handler, (void*) DnPin);

	while(1)
		{
		vTaskDelay(configTICK_RATE_HZ);
		if(ButtonFlag)
			{
				ButtonFlag=false;
				printf("You pressed Down Button %d Times\r\n",count);
			}
		}
}
