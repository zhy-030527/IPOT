/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			DHT11温湿度传感器初始化设置
 *			DHT11采用单总线传输协议，只通过一根数据线进行传输
 *			工作模式为：主机先下拉电压，从机判断低电压后，接管总线，一次传输40位数据
 *			数据格式：8bit湿度整数+8bit湿度小数+8bit温度整数+8bit温度小数+8bit湿度校验位
 *			控制GPIO读取DHT11的数据：
 * 				1. 主机发出至少18MS的低脉冲: start信号
 * 				2. start信号变为高, 20-40us之后, dht11会拉低总线维持80us，然后拉高80us: 回应信号
 * 				3. 之后就是数据, 逐位发送
 *    				0 : 50us低脉冲, 26-28us高脉冲
 *    				1 : 50us低脉冲, 70us高脉冲
 *			注意这里的所有的时间单位都是us级
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.07      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#include "stm32f1xx_hal.h"
#include "Driver_DHT11.h"
#include "Driver_Delay.h"
#include "Driver_UART.h"
#include "Driver_LCD.h"
#include "Driver_ESP8266.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "queue.h"
#include "gpio.h"

#define DHT11_GPIO				GPIOA
#define DHT11_PIN				GPIO_PIN_1
#define DHT11_PinSetHigh()		HAL_GPIO_WritePin(DHT11_GPIO, DHT11_PIN, GPIO_PIN_SET)
#define DHT11_PinSetLow()		HAL_GPIO_WritePin(DHT11_GPIO, DHT11_PIN, GPIO_PIN_RESET)

extern void GetI2C(void);
extern void PutI2C(void);
extern QueueHandle_t xSensorQueue;

/**********************************************************************
 * 函数名称： DHT11_PinCfgAsInput
 * 功能描述： 把DHT11的数据引脚配置为输入模式
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void DHT11_PinCfgAsInput(void)
{
    /* 对于STM32F103, 已经把DHT11的引脚配置为"open drain, pull-up" 
	* 让它输出1就不会驱动这个引脚, 并且可以读入引脚状态
     */
	DHT11_PinSetHigh();
    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

/**********************************************************************
 * 函数名称： DHT11_PinRead
 * 功能描述： 读取DHT11的数据引脚
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 1-高电平, 0-低电平
 ***********************************************************************/
static int DHT11_PinRead(void)
{
    if (GPIO_PIN_SET == HAL_GPIO_ReadPin(DHT11_GPIO, DHT11_PIN))
		return 1;
	else
		return 0;
}

/**********************************************************************
 * 函数名称： DHT11_Start
 * 功能描述： 主机发送开始指令给从机
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void DHT11_Start(void)
{
	DHT11_PinSetLow();
	//低电平保持时间不能低于18ms
	mdelay(20);
	DHT11_PinCfgAsInput();
}

/**********************************************************************
 * 函数名称： DHT11_Wait_Ack
 * 功能描述： 等待DHT11的回应信号
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 1-无响应, 0-有响应
 ***********************************************************************/
static int DHT11_Wait_Ack(void)
{
	udelay(60);
	return DHT11_PinRead();
}

/**********************************************************************
 * 函数名称： DHT11_WaitFor_Val
 * 功能描述： 在指定时间内等待数据引脚变为某个值
 * 输入参数： val - 期待数据引脚变为这个值
 *            timeout_us - 超时时间(单位us)
 * 输出参数： 无
 * 返 回 值： 0-成功, (-1) - 失败
 ***********************************************************************/
static int DHT11_WaitFor_Val(int val, int timeout_us)
{
	while (timeout_us--)
	{
		if (DHT11_PinRead() == val)
			return 0; /* ok */
		udelay(1);
	}
	return -1; /* err */
}

/**********************************************************************
 * 函数名称： DHT11_ReadByte
 * 功能描述： 读取DH11 1byte数据
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 数据
 ***********************************************************************/
static int DHT11_ReadByte(void)
{
	int i;
	int data = 0;
	
	for (i = 0; i < 8; i++)
	{
		//等待高电位，超时则返回-1
		if (DHT11_WaitFor_Val(1, 1000) == 1 )
		{
			return -1;
		}
		//DHT11 每 bit 开始时：先拉低50µs,再拉高.所以这里需要延迟一段时间，再去检测低电平
		udelay(40);
		//数据向前挪一位，准备接收下一位数据
		data <<= 1;
		//如果延迟后接收到的数据为高电平，则代表这位数据是1，反之则为0
		if (DHT11_PinRead() == 1)
			data |= 1;
		
		//等待低电平结束，如果等不到直接报错
		if (DHT11_WaitFor_Val(0, 1000))
		{
			return -1;
		}
	}
	
	return data;
}

/**********************************************************************
 * 函数名称： DHT11_Init
 * 功能描述： DHT11的初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
void DHT11_Init(void)
{
	DHT11_PinSetHigh();
}

/**********************************************************************
 * 函数名称： DHT11_Read
 * 功能描述： 读取DHT11的温度/湿度
 * 输入参数： 无
 * 输出参数： hum  - 用于保存湿度值
 *            temp - 用于保存温度值
 * 返 回 值： 0 - 成功, (-1) - 失败
 * DHT11 的温度和湿度的小数部分，在实际使用中可以直接忽略，DHT11 分辨率只有 1℃ / 1%
 ***********************************************************************/
int DHT11_Read(int *hum, int *temp)
{
	unsigned char hum_m, hum_n;
	unsigned char temp_m, temp_n;
	unsigned char check;	
	
	DHT11_Start();
	
	//DHT11在开始的时候会发送一个低电平表示准备发送数据，没有的话代表接受失败
	if (DHT11_Wait_Ack() != 0)
	{
		return -1;
	}
	//接受数据之前的操作，是否先拉高电位，然后是否能开始正式发送数据（低点位）
	if (DHT11_WaitFor_Val(1, 1000) != 0)  /* 等待ACK变为高电平, 超时时间是1000us */
	{
		return -1;
	}
	if (DHT11_WaitFor_Val(0, 1000)!= 0)  /* 数据阶段: 等待低电平, 超时时间是1000us */
	{
		return -1;
	}

	hum_m  = DHT11_ReadByte();
	hum_n  = DHT11_ReadByte();
	temp_m = DHT11_ReadByte();
	temp_n = DHT11_ReadByte();
	check  = DHT11_ReadByte();

	//拉高管教电位，代表一组数据采集完毕
	DHT11_PinSetHigh();

	if (hum_m + hum_n + temp_m + temp_n == check)
	{
		*hum  = hum_m;
		*temp = temp_m;
		return 0;
	}
	else
	{
		return -1;
	}

}

/**********************************************************************
 * 函数名称： DHT11_Test
 * 功能描述： DHT11测试程序
 * 输入参数： 无
 * 输出参数： 无
 *            无
 * 返 回 值： 无
 ***********************************************************************/
void DHT11_Test(void *params)
{
	int hum, temp,err;
	SendData Mydata;
    Mydata.type = SENSOR_DHT11;
//	UBaseType_t freeNum;
//	TaskHandle_t xTaskHandle;
	while(1)
	{
//		xTaskHandle = xTaskGetCurrentTaskHandle();
//		freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);
//		printf("%ld",freeNum);
		vTaskSuspendAll();
		err = DHT11_Read(&hum, &temp);
		xTaskResumeAll();
		if(err != 0)
		{
			GetI2C();	//等待I2C，获得互斥锁
			LCD_PrintString(0,0,"error");
			//printf("error\r\n");
			PutI2C();	//释放互斥锁
			DHT11_Init();
		}
		else
		{
			Mydata.Humidity = hum ;
			Mydata.Temperature = temp ;
			//将数据写入队列
			xQueueSend(xSensorQueue,&Mydata,0);
			GetI2C();
			LCD_PrintString(0,0,"Humidity = ");
			LCD_PrintSignedVal(12,0,hum);
			LCD_PrintString(15,0,"%");
			//printf("humidity = %2d", hum);
			//printf("%%   	");
			LCD_PrintString(0,2,"Tempture = ");
			LCD_PrintSignedVal(12,2,temp);
			LCD_PrintString(15,2,"C");
			//printf("Tempture = %2d", temp);
			//printf(" C\r\n");
			PutI2C();
		}
		vTaskDelay(2000);
	}
}



