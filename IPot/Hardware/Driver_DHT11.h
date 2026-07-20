/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			DHT11温湿度传感器初始化设置
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.07      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#ifndef _DRIVER_DHT11_H
#define _DRIVER_DHT11_H

#include <stdint.h>
#include <stdio.h>

static void DHT11_PinCfgAsInput(void);
void DHT11_Init(void);
void DHT11_Test(void *params);

#endif

