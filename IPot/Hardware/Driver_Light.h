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

#ifndef _DRIVER_LIGHT_H
#define _DRIVER_LIGHT_H

#include <stdint.h>
#include <stdio.h>

void Light_Init(void);
void Light_Test(void *params);

#endif

