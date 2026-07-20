/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			BH1750光照传感器初始化配置
 *			本实验将ADDR管脚悬空，此BH1750的地址为01000011
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.07      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#include "stm32f1xx_hal.h"
#include "i2c.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "queue.h"
#include "semphr.h"

#include "Driver_BH1750.h"
#include "Driver_Delay.h"
#include "Driver_UART.h"
#include "Driver_LCD.h"
#include "Driver_ESP8266.h"

extern I2C_HandleTypeDef hi2c1;
extern uint16_t g_Auto_Light_Mode;
static I2C_HandleTypeDef *g_pHI2C_MPU6050 = &hi2c1;

extern void GetI2C(void);
extern void PutI2C(void);
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xLEDQueue;

//光照参数
uint16_t LightNum , err;

/**********************************************************************
 * 函数名称： BH1750_WriteRegister
 * 功能描述： 写BH1750寄存器
 * 输入参数： reg-寄存器地址, data-要写入的数据
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
static int BH1750_WriteRegister(uint8_t data)
{
    uint8_t tmpbuf;
    tmpbuf = data;
    
    return HAL_I2C_Master_Transmit(g_pHI2C_MPU6050, BH1750_I2C_ADDR, &tmpbuf, 1, BH1750_TIMEOUT);
}

/**********************************************************************
 * 函数名称： BH1750_ReadRegister
 * 功能描述： 读BH1750寄存器
 * 输出参数： pdata-用来保存读出的数据
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int BH1750_ReadRegister(uint8_t *pdata)
{
	return HAL_I2C_Master_Receive(g_pHI2C_MPU6050, BH1750_I2C_ADDR, pdata, 2, BH1750_TIMEOUT);
}

/**********************************************************************
 * 函数名称： BH1750_Init
 * 功能描述： BH1750初始化函数,
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int BH1750_Init(void)
{
	BH1750_WriteRegister(BH1750_Power_ON);				//通电唤醒
	BH1750_WriteRegister(BH1750_Reset);				//复位清零
	BH1750_WriteRegister(BH1750_H_Resolution_Mode);	//选择连续测量、高分辨率
	
	// 给传感器一点时间进行第一次测量 (高分辨率模式大约需要120ms)
    HAL_Delay(150); 
	
	return 0;
}

/**********************************************************************
 * 函数名称： BH1750_ReadData
 * 功能描述： 读取BH1750数据
 * 输入参数： 无
 * 输出参数： LightNum
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
int BH1750_ReadData(uint16_t *LightNum)
{
	uint8_t data[2];
	
	if(BH1750_ReadRegister(data) == HAL_OK)
	{
		// 传感器返回的两个字节：buf[0] 是高八位，buf[1] 是低八位
		*LightNum = (data[0]<<8)|data[1];
		return 1;
	}
	else
	{
		return 0;
	}
}

void BH1750_Test(void *params)
{
	DeviceCmd_t cmd;
	SendData Mydata;
    Mydata.type = SENSOR_BH1750;
//	UBaseType_t freeNum;
//	TaskHandle_t xTaskHandle;
	while(1)
	{	
		GetI2C();
		err = BH1750_ReadData(&LightNum);
		PutI2C();
//		xTaskHandle = xTaskGetCurrentTaskHandle();
//		freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);
		if(err == 0)
		{
			GetI2C();
			LCD_PrintString(0,4,"error");
			PutI2C();
	//		printf("error\r\n");
			BH1750_Init();
		}
		else
		{
			//如果光照强度低于20 lux时，发送一个队列信号去控制灯开关
			if(g_Auto_Light_Mode == 1)
			{
				if(LightNum < 25 )
				{
					cmd = CMD_ON ;
					xQueueSend(xLEDQueue, &cmd, 0);
				}
				else
				{
					cmd = CMD_OFF ;
					xQueueSend(xLEDQueue, &cmd, 0);
				}
			}
//			printf("%ld",freeNum);
			Mydata.Light = LightNum ;
			//将数据写入队列
			xQueueSend(xSensorQueue,&Mydata,0);
			GetI2C();
			LCD_PrintString(0,4,"Light    = ");
			LCD_PrintString(12,4,"				");
			LCD_PrintSignedVal(12,4,(int32_t)LightNum);
			PutI2C();
			//printf("LightNum = %5d", LightNum);
			//printf("  lx\r\n");
		}
		vTaskDelay(2000);
	}
}
