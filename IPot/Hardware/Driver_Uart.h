/*  Copyright (C) 2026 粽
 *  All rights reserved	
 *  
 * 摘要：
 *			串口的配置（printf重定向）
 *
 *
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2026.05.06      v01        	  粽      	 创建文件
 *-----------------------------------------------------
*/

#ifndef _DRIVER_UART_H
#define _DRIVER_UART_H

#include <stdint.h>
#include <stdio.h>

void UART_Init(void);
int fputc(int c, FILE *f);
int fgetc(FILE *f);

#endif
