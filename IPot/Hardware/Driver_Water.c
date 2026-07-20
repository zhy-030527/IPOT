/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			喷水装置开关的驱动代码
 *			
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.29      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#include "Driver_Water.h"
#include "Driver_ESP8266.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "semphr.h"
#include "gpio.h"
#include <stdio.h>
#include <stdbool.h>

#define Water_On	HAL_GPIO_WritePin(GPIOB, Water_Pin, GPIO_PIN_SET);
#define Water_Off	HAL_GPIO_WritePin(GPIOB, Water_Pin, GPIO_PIN_RESET);


extern QueueHandle_t xPumpQueue;
bool g_Water_status;

/**********************************************************************
 * 函数名称： Water_Init
 * 功能描述： 初始化喷洒装置
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void Water_Init(void)
{
	Water_Off;
}

/**********************************************************************
 * 函数名称： Light_Test
 * 功能描述： 当土壤湿度传感器发送一个开灯信号量时，控制灯亮
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void Water_Test(void *params)
{
	DeviceCmd_t recv_cmd;
	while(1)
	{
		if(xQueueReceive(xPumpQueue,&recv_cmd,portMAX_DELAY) == pdTRUE)
		{
			if (recv_cmd == CMD_ON) 
			{
                Water_On;
				g_Water_status = true;
            } 
			else 
			{
                Water_Off;
				g_Water_status = false;
            }
		}
	}
	
}
