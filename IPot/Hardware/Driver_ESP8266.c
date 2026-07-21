/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			ESP8266的驱动代码
 *			
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.14      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/


#include "stm32f1xx_hal.h"
#include "Driver_ESP8266.h"
#include "Driver_Delay.h"
#include "Driver_UART.h"
#include "Driver_LCD.h"





// 串口接收缓冲区
char ESP8266_rx_buffer[RX_BUF_MAX_LEN];		//缓冲区大小1024
uint16_t ESP8266_rx_len = 0;				//缓冲区已使用字节
uint16_t g_Auto_Light_Mode;
uint16_t g_Auto_Water_Mode;
//传感器数据
uint16_t json_len;
char json_buf[350];

static SendData ReceiveData;
static Collector MyCollector;
//队列初始化
QueueHandle_t xSensorQueue;
QueueHandle_t xLEDQueue;
QueueHandle_t xPumpQueue;
//定时器初始化
TimerHandle_t DatadogHandle_t;

extern SemaphoreHandle_t xRxSemaphore;
extern bool g_Led_status;
extern bool g_Water_status;

/**********************************************************************
 * 函数名称： ESP8266_Clear_Buffer
 * 功能描述： 清空接收缓冲区
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/
// 清空接收缓冲区
void ESP8266_Clear_Buffer(void) {
    memset(ESP8266_rx_buffer, 0, RX_BUF_MAX_LEN);
    ESP8266_rx_len = 0;
}

/**********************************************************************
 * 函数名称： ESP8266_Send_Cmd
 * 功能描述： 向ESP8266发送指令，收到ok指令，继续运行如果没有收到ok，则直接报错
 * 输入参数： cmd: 指令字符串, res: 期望的响应字符串, timeout: 超时时间
 * 输出参数： 无
 * 返 回 值： 0：成功，1：失败
 ***********************************************************************/
// 
uint8_t ESP8266_Send_Cmd(char *cmd, char *res, uint16_t timeout) 
{ 
    ESP8266_Clear_Buffer(); // 1. 清空串口接收缓冲区
    printf("%s\r\n", cmd); // 2. 发送指令
    // 3. 循环等待直到超时或收到正确响应
    while(timeout--) 
	{
        if(strstr(ESP8266_rx_buffer, res)) 
		{
            return 0; // 成功
        }
        mdelay(1);
    }
    return 1; // 失败
}

/**********************************************************************
 * 函数名称： ESP8266_Init
 * 功能描述： ESP8266初始化，包括RESET，模式设置，网络连接，以及创建队列和开启任务
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0：成功，1：失败
 ***********************************************************************/
// 
uint8_t ESP8266_Init(void)
{
	LCD_PrintString(0,0,"ESP_Init_Start");
	mdelay(500);
	
	//重置ESP8266
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"1 ESP_RST");
	if(ESP8266_Send_Cmd("AT+RST", "OK", 2000)){return 1;}
	mdelay(500);
	
	//测试通讯
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"2 ESP_TEST");
	if(ESP8266_Send_Cmd("AT", "OK", 500)){return 1;}
	mdelay(500);
	
	//设置ESP8288为STA模式并关闭回显
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"3 ESP_SET_STA");
	if(ESP8266_Send_Cmd("AT+CWMODE=1", "OK", 500)){return 1;}
	mdelay(500);
	
	if(ESP8266_Send_Cmd("ATE0", "OK", 500)){return 1;}
	mdelay(500);
	
	//连接WIFI
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"4 ESP_WIFI");
	char wifi_cmd[128];
	sprintf(wifi_cmd, "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASSWORD);
	if(ESP8266_Send_Cmd(wifi_cmd, "OK", 1000))
	{
		mdelay(500);
		if(ESP8266_Send_Cmd("AT+CWJAP=\"zong\",\"13306569330\"", "OK", 1000)){return 1;}
	}
	mdelay(500);
	
	//配置MQTT用户信息
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"5 ESP_CONFIG");
	char mqttcfg_cmd[512];
    sprintf(mqttcfg_cmd, 
        "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"",
        DEVICE_ID,  				
        PRODUCT_ID,                	
        AUTH_INFO               	
    );
	if(ESP8266_Send_Cmd(mqttcfg_cmd, "OK", 500)){return 1;}
	mdelay(500);
	
	//连接OneNet MQTT服务器
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"6 ESP_ONENET");
	if(ESP8266_Send_Cmd("AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,0", "OK", 1000)){return 1;}
	mdelay(500);
	
	//再连接上OneNet服务器之后，需要订阅数据下发指令
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"7 ONENET_TOPIC");
	if(ESP8266_Send_Cmd("AT+MQTTSUB=0,\"$sys/Fv2Kuy99hL/ESP8266/thing/property/set\",0", "OK", 1000)){return 1;}
	mdelay(500);
	LCD_ClearLine(0,0);
	LCD_PrintString(0,0,"ESP_SET_OK");
	
		
	//创建三个队列
	xSensorQueue = xQueueCreate( 30, sizeof(SendData));
	xLEDQueue = xQueueCreate( 5, sizeof(DeviceCmd_t));
	xPumpQueue = xQueueCreate( 5, sizeof(DeviceCmd_t));
	
	//开启三个任务
	extern void DHT11_Test(void *params);
	xTaskCreate(DHT11_Test, "DHT11_Test", 128, NULL, osPriorityNormal, NULL);
	  
	extern void BH1750_Test(void *params);
	xTaskCreate(BH1750_Test, "BH1750_Test", 128, NULL, osPriorityNormal, NULL);
	  
	extern void MoistureSensor_Test(void *params);
	xTaskCreate(MoistureSensor_Test, "MoistureSensor_Test", 128, NULL, osPriorityNormal, NULL);
	
	//然后还需开启两个执行任务
	extern void Light_Test(void *params);
    xTaskCreate(Light_Test, "Light_Test", 32, NULL, osPriorityNormal2, NULL);
  
    extern void Water_Test(void *params);
    xTaskCreate(Water_Test, "Water_Test", 32, NULL, osPriorityNormal2, NULL);
	
	//创建软件定时器（数据看门狗）
	DatadogHandle_t = xTimerCreate("DatadogHandle", pdMS_TO_TICKS(5000), pdFALSE, (void *)0, vDataWatchdogCallback);

	return 0;
	
}

/**********************************************************************
 * 函数名称： ESP8266_Connect_Test
 * 功能描述： 判断是否能够成功初始化ESP8266
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0：成功，1：失败
 ***********************************************************************/
// 
void ESP8266_Connect_Test(void)
{
	if(ESP8266_Init())
	{
		LCD_ClearLine(0,0);
		LCD_PrintString(0,0,"ESP_Init_Fail");
		LCD_PrintString(0,2,"Press RST");
	}
	else
	{
		LCD_ClearLine(0,0);
		LCD_PrintString(0,0,"Succeed");
		mdelay(500);
		LCD_Clear();
	}
}

/**********************************************************************
 * 函数名称： ESP8266_PackData
 * 功能描述： ESP8266数据json处理
 * 输入参数： SendData *data,char *json_buf, size_t buf_size
 * 输出参数： 无
 * 返 回 值： json_buf文件长度
 ***********************************************************************/
// 	
uint16_t  ESP8266_PackData(SendData *data,char *json_buf, size_t buf_size)
{
//    int len = snprintf(json_buf, buf_size,
//        "{\\\"id\\\":\\\"123\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,"
//        "\\\"params\\\":{"
//        "\\\"Temperature\\\":{\\\"value\\\":%d}\\,"
//        "\\\"humidity\\\":{\\\"value\\\":%d}\\,"
//        "\\\"light\\\":{\\\"value\\\":%d}\\,"
//        "\\\"moisture\\\":{\\\"value\\\":%d}\\,"
//		"\\\"Lstatus\\\":{\\\"value\\\":%d}\\,"
//		"\\\"Pstatus\\\":{\\\"value\\\":%d}"
//        "}}",
//        data -> Temperature, data -> Humidity,
//        data -> Light, data -> Soil_moisture,
//		data -> Led_status, data -> Pump_status);
//    return len;
	int len = snprintf(json_buf, buf_size,
        "{\"id\":\"123\",\"version\":\"1.0\","
        "\"params\":{"
        "\"Temperature\":{\"value\":%d},"
        "\"humidity\":{\"value\":%d},"
        "\"light\":{\"value\":%d},"
        "\"moisture\":{\"value\":%d},"
        "\"Lstatus\":{\"value\":%d},"
        "\"Pstatus\":{\"value\":%d}"
        "}}",
        data->Temperature, data->Humidity,
        data->Light, data->Soil_moisture,
        data->Led_status, data->Pump_status);
        
    return len;
}

/**********************************************************************
 * 函数名称： ESP8266_SendToOneNet
 * 功能描述： ESP8266数据json处理
 * 输入参数： char *json_buf, uint16_t json_len
 * 输出参数： 无
 * 返 回 值： json_buf文件长度
 ***********************************************************************/
// 	
void ESP8266_SendToOneNet(char *json_buf, uint16_t json_len)
{
//    char at_cmd[350];
//    int at_len = snprintf(at_cmd, sizeof(at_cmd),
//        "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n",
//        ESP8266_Send, json_buf);
//    // 发送 AT 指令
//    printf("%s", at_cmd);
	char at_cmd[128];
    
    // 🌟 步骤一：先发指令头，明确告知 ESP8266 接下来要发 json_len 长度的裸数据
    snprintf(at_cmd, sizeof(at_cmd),
        "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n",
        ESP8266_Send, json_len);
        
    // 通过串口发送给 ESP8266
    printf("%s", at_cmd);
    
    // 🌟 步骤二：必须延时 50ms 左右，等 ESP8266 睁开眼睛并吐出 ">" 提示符
    // 如果你在 FreeRTOS 任务里，建议用 vTaskDelay(pdMS_TO_TICKS(50));
    HAL_Delay(50); 
    
    // 🌟 步骤三：把纯净的 JSON 裸数据一次性推过去，彻底无视 256 字节限制！
    printf("%s", json_buf);
}


/**********************************************************************
 * 函数名称： ESP8266_SendTask
 * 功能描述： ESP8266数据处理并发送数据
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0：成功，1：失败
 ***********************************************************************/
// 
void ESP8266_SendTask(void *params)
{
	ESP8266_Connect_Test();
//	UBaseType_t freeNum;
//	TaskHandle_t xTaskHandle;
	while(1)
	{
//		xTaskHandle = xTaskGetCurrentTaskHandle();
//		freeNum = uxTaskGetStackHighWaterMark(xTaskHandle);
//		printf("%ld",freeNum);
		if(xQueueReceive( xSensorQueue, &ReceiveData, portMAX_DELAY) == pdPASS)
		{
			//如果数据是新的一组里的数据，开启新的一轮定时器
			if (MyCollector.Rflags == 0) 
			{
				xTimerReset(DatadogHandle_t, 0); // Start/Reset 定时器
			}
			//获取三个传感器的数据收集并进行整合
			switch(ReceiveData.type)
			{
				//接收到DHT11的数据
				case SENSOR_DHT11:
					MyCollector.Data.Temperature = ReceiveData.Temperature;
					MyCollector.Data.Humidity = ReceiveData.Humidity;
					MyCollector.Rflags |= 0x01;  // 设置DHT11标志位
				break;
				//接收到BH1750的数据
				case SENSOR_BH1750:
					MyCollector.Data.Light = ReceiveData.Light;
					MyCollector.Rflags |= 0x02;  // 设置BH1750标志位
				break;
				//接收到土壤湿度的数据
				case SENSOR_SOIL:
					MyCollector.Data.Soil_moisture = ReceiveData.Soil_moisture;
					MyCollector.Rflags |= 0x04;  // 设置土壤湿度的标志位
				break;
				//接收到强制发送命令
				case SENSOR_TIMEOUT_FORCE_SEND:
					Pack_And_Send_Data();
				break;
				
				default:
					
				break;
			}
		}
		//当收集到的标志位为0111时，代表四个数据都已获取，进行打包并发送
		if(MyCollector.Rflags == 0x07)
		{
			Pack_And_Send_Data();
			xTimerStop(DatadogHandle_t, 0); 
			//memset(&MyCollector.Rflags, -1, sizeof(SendData));
		}
	}
}

//数据打包及重置函数
void Pack_And_Send_Data(void)
{
	MyCollector.Data.Pump_status = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);		//直接改为获取管脚高低电平
	MyCollector.Data.Led_status = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
	json_len = ESP8266_PackData(&MyCollector.Data,json_buf, sizeof(json_buf));
	ESP8266_SendToOneNet(json_buf,json_len);
	// 重置批次,所有数据初始值为0
	MyCollector.Rflags = 0;
	//重置结构体为 -1
	MyCollector.Data.Temperature = -1;
	MyCollector.Data.Humidity = -1;
	MyCollector.Data.Light = -1;
	MyCollector.Data.Soil_moisture = -1;
}


//定时器超时回调函数（禁止在回调函数中进行阻塞操作）
void vDataWatchdogCallback(TimerHandle_t xTimer) 
{
    SendData timeout_cmd;
    timeout_cmd.type = SENSOR_TIMEOUT_FORCE_SEND; // 宏定义一个特殊类型
    
    // 注意：在定时器回调中，等同于中断，建议使用不阻塞的发送方式
    xQueueSend(xSensorQueue, &timeout_cmd, 0); 
}


/**********************************************************************
 * 函数名称： Extract_JSON_Bool
 * 功能描述： 轻量级 JSON 布尔值解析工具,自动忽略冒号前后的空格，精准提取 true/false
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0：flase，1：true，-1：没找到这个键值对
 ***********************************************************************/
// 
int8_t Extract_JSON_Bool(const char *json_str, const char *key) 
{
    // 1. 先找到 key (比如 "Light_status") 的位置
    char *ptr = strstr(json_str, key);
    if (ptr == NULL) return -1; // 没找到该字段，直接返回
    
    // 2. 找到该 key 后面的第一个冒号 ':'
    ptr = strchr(ptr, ':');
    if (ptr == NULL) return -1; 
    
    // 3. 跨过冒号，并且跳过所有的空格（这是最关键的容错设计！）
    while(*(++ptr) == ' ' || *ptr == '\t'); 
    
    // 4. 判断提取到的值
    if (strncmp(ptr, "true", 4) == 0) return 1;
    if (strncmp(ptr, "false", 5) == 0) return 0;
    
    return -1; // 既不是true也不是false，可能是个格式错误
}

/**********************************************************************
 * 函数名称： ESP8266_ReceiveTask
 * 功能描述： ESP8266接收数据并且处理数据
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0：成功，1：失败
 ***********************************************************************/
// 
void ESP8266_ReceiveTask(void *params)
{	
	while(1)
	{
		if (xSemaphoreTake(xRxSemaphore, portMAX_DELAY) == pdTRUE)
        {
			DeviceCmd_t cmd;
			int8_t status;
			//解析灯光控制
			status = Extract_JSON_Bool(ESP8266_rx_buffer, "\"Light_status\"");
			if (status != -1) 
			{
				cmd = (status == 1) ? CMD_ON : CMD_OFF;
				xQueueSend(xLEDQueue, &cmd, 0);
			}
			
			//解析水泵控制
			status = Extract_JSON_Bool(ESP8266_rx_buffer, "\"Water_status\"");
			if (status != -1) 
			{
				cmd = (status == 1) ? CMD_ON : CMD_OFF;
				xQueueSend(xPumpQueue, &cmd, 0);
			}  
			
			//解析自动模式控制
			status = Extract_JSON_Bool(ESP8266_rx_buffer, "\"Auto_light_mode\"");
			if (status != -1) 
			{
				g_Auto_Light_Mode = status; // 直接更新全局变量
				cmd = CMD_OFF ;
				xQueueSend(xLEDQueue, &cmd, 0);
			}
			
			status = Extract_JSON_Bool(ESP8266_rx_buffer, "\"Auto_water_mode\"");
			if (status != -1)
			{
				g_Auto_Water_Mode = status; // 直接更新全局变量
				cmd = CMD_OFF ;
				xQueueSend(xPumpQueue, &cmd, 0);
			}
			//清空缓存
			ESP8266_Clear_Buffer();
		}
	}
}

