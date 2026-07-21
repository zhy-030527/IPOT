/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			ESP8266的驱动代码
 *			
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.07      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#ifndef _DRIVER_ESP8266_H
#define _DRIVER_ESP8266_H

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "cmsis_os2.h"
#include "queue.h"
#include "string.h"
#include "stdio.h"
#include "semphr.h"
#include "timers.h"

#define RX_BUF_MAX_LEN  512                  // 接收缓冲区大小
#define WIFI_SSID			"zong"
#define WIFI_PASSWORD		"13306569330"
#define DEVICE_ID			"ESP8266"
#define PRODUCT_ID			"Fv2Kuy99hL"
#define AUTH_INFO         	"version=2018-10-31&res=products%2FFv2Kuy99hL%2Fdevices%2FESP8266&et=1810224000&method=md5&sign=8sZ2vjBOEPqvO2aNbYTLnQ%3D%3D"
#define ESP8266_Send		"$sys/Fv2Kuy99hL/ESP8266/thing/property/post"



// 数据结构
typedef enum {
    SENSOR_DHT11,   // 温湿度传感器
    SENSOR_BH1750,      // 光照传感器
    SENSOR_SOIL,        // 土壤湿度传感器
	SENSOR_TIMEOUT_FORCE_SEND,
} SensorType_t;

// 传感器数据包
typedef struct{
	SensorType_t type; 		//数据类型
	int Temperature;		//DHT11温度
	int Humidity;			//DHT11湿度
	int Light;				//BH1750光照强度
	int Soil_moisture;		//土壤湿度
	bool Pump_status;
	bool Led_status;
}SendData;

typedef enum {
    CMD_OFF = 0,      // 关
    CMD_ON  = 1       // 开
} DeviceCmd_t;

// 批次收集器
typedef struct {
    SendData Data;    // 合并后的数据
    uint8_t Rflags;     // 收到数据的标志位
} Collector;

uint8_t ESP8266_Init(void);
void ESP8266_Connect_Test(void);
void Pack_And_Send_Data(void);
void vDataWatchdogCallback(TimerHandle_t xTimer);

#endif

