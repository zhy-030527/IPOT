/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			土壤湿度传感器的配置（ADC模式）
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.06      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#include "Driver_Moisture.h"
#include "Driver_Uart.h"
#include "Driver_LCD.h"
#include "Driver_ESP8266.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "queue.h"
#include "semphr.h"
#include "gpio.h"
#include "adc.h"

#include <math.h>


#define Moisture_SENSOR_ADC_TIMEOUT 500

extern ADC_HandleTypeDef hadc1;
static ADC_HandleTypeDef *g_HADC_MoistureSensor = &hadc1;

extern void GetI2C(void);
extern void PutI2C(void);
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xPumpQueue;
extern uint16_t g_Auto_Water_Mode;

uint32_t Airvalue = 3340 ;
uint32_t Watervalue = 1120 ;

/**********************************************************************
 * 函数名称： MoistureSensor_Init
 * 功能描述： 土壤湿度传感器的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void MoistureSensor_Init(void)
{
    /* PA3用作ADC(ADC1_IN3) */
    HAL_ADCEx_Calibration_Start(g_HADC_MoistureSensor);

}


/**********************************************************************
 * 函数名称： MoistureSensor_Read
 * 功能描述： 读取土壤湿度传感器的ADC值
 * 输入参数： 无
 * 输出参数： pData - 用来保存ADC值
 * 返 回 值： 0 - 成功, (-1) - 失败
 ***********************************************************************/
int MoistureSensor_Read(uint32_t *pData)
{
     HAL_ADC_Start(g_HADC_MoistureSensor);
     if (HAL_OK == HAL_ADC_PollForConversion(g_HADC_MoistureSensor, Moisture_SENSOR_ADC_TIMEOUT))
     {
        *pData = HAL_ADC_GetValue(g_HADC_MoistureSensor);
        return 0;
     }
     else
     {
        return -1;
     }
}

/**********************************************************************
 * 函数名称： map
 * 功能描述： 读取土壤湿度传感器的ADC值进行线性转换
 * 输入参数： // 当前电阻，干燥，湿润，湿度 0%，湿度 100%
 * 输出参数： 湿度百分比
 ***********************************************************************/
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**********************************************************************
 * 函数名称： MoistureSensor_Test
 * 功能描述： 测试土壤湿度传感器
 * 输入参数： 无
 ***********************************************************************/
void MoistureSensor_Test(void *params)
{
	uint32_t Moisture_num;
	SendData Mydata;
    Mydata.type = SENSOR_SOIL;
	DeviceCmd_t cmd;
//	UBaseType_t freeNum;
//	TaskHandle_t xTaskHandle;
	while(1)
	{
//		xTaskHandle = xTaskGetCurrentTaskHandle();
//		freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);
		if(MoistureSensor_Read(&Moisture_num) == 0)
		{
			if (Moisture_num > Airvalue) Moisture_num = Airvalue;
			if (Moisture_num < Watervalue) Moisture_num = Watervalue;
			int moisturePercent = map(Moisture_num, Airvalue, Watervalue, 0, 100);
			
			if(g_Auto_Water_Mode == 1)
			{
				if(moisturePercent < 25 )
				{
					cmd = CMD_ON ;
					xQueueSend(xPumpQueue, &cmd, 0);
				}
				else
				{
					cmd = CMD_OFF ;
					xQueueSend(xPumpQueue, &cmd, 0);
				}
			}
			
//			printf("%ld",freeNum);
			//printf("Moisture: %2d %%\r\n",moisturePercent);
			Mydata.Soil_moisture = moisturePercent ;
			//将数据写入队列
			xQueueSend(xSensorQueue,&Mydata,0);
			GetI2C();	//等待I2C，获得互斥锁
			LCD_PrintString(0,6,"Moisture = ");
			LCD_PrintString(12,6,"     ");
			LCD_PrintSignedVal(12,6,moisturePercent);
			PutI2C();	//释放互斥锁
			
		}
		vTaskDelay(2000);
	}
	
}




