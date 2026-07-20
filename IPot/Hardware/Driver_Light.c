/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			照明开关的驱动代码
 *			
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.29      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#include "Driver_Light.h"
#include "Driver_BH1750.h"
#include "Driver_ESP8266.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "semphr.h"
#include "gpio.h"
#include <stdio.h>
#include <stdbool.h>

#define Light_On	HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_SET)
#define Light_Off	HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET)

extern QueueHandle_t xLEDQueue;
bool g_Led_status;


/**********************************************************************
 * 函数名称： Light_Init
 * 功能描述： 初始化照明
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void Light_Init(void)
{
	Light_Off;
}

/**********************************************************************
 * 函数名称： Light_Test
 * 功能描述： 当BH1750发送一个开灯信号量时，控制灯亮
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void Light_Test(void *params)
{
	DeviceCmd_t recv_cmd;
//	UBaseType_t freeNum;
//	TaskHandle_t xTaskHandle;
	while(1)
	{
//		xTaskHandle = xTaskGetCurrentTaskHandle();
//		freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);
//		printf("%ld",freeNum);
		if(xQueueReceive(xLEDQueue,&recv_cmd,portMAX_DELAY) == pdTRUE)
		{
			if (recv_cmd == CMD_ON) 
			{
                Light_On;
				g_Led_status = true;
            } else 
			{
                Light_Off;
				g_Led_status = false;
            }
		}
	}
	
}

