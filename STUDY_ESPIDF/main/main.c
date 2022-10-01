#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
TaskHandle_t myTask1Handler=NULL;

void myTask1(void * p)
{
	int myTask1Cnt=0,PassParameters=(int)(int *)p;
	while(1)
	{
		vTaskDelay(PassParameters*configTICK_RATE_HZ/1000);
		printf("MyTask1 executed %d Times\r\n",myTask1Cnt++);
	}
}


void app_main(void)
{
	unsigned int App_mainCnt=0;
	xTaskCreate(myTask1, "MyTask1", 1<<13, (void *)1000,tskIDLE_PRIORITY , &myTask1Handler);
	while(1)
	{
		vTaskDelay( configTICK_RATE_HZ );
		printf("App_main executed %d Times\r\n",App_mainCnt++);
	}

}
