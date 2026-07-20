/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.06      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#ifndef _DRIVER_MOISTURE_H
#define _DRIVER_MOISTURE_H

#include <stdint.h>

void MoistureSensor_Init(void);
int MoistureSensor_Read(uint32_t *pData);
void MoistureSensor_Test(void *params);

#endif
