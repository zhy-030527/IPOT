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

#include "Driver_Uart.h"
#include "usart.h"

#include <stdio.h>

#define DEBUG_UART_TIMEOUT 500

extern UART_HandleTypeDef huart1;
static UART_HandleTypeDef * g_HDebugUART = &huart1;

/**********************************************************************
 * 函数名称： UART_Init
 * 功能描述： UART初始化函数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 ***********************************************************************/
void UART_Init(void)
{
}


int fputc(int c, FILE *f) 
{
    (void)f;
    HAL_UART_Transmit(g_HDebugUART, (const uint8_t *)&c, 1, DEBUG_UART_TIMEOUT);
    return c;
}

int fgetc(FILE *f)
{
    uint8_t ch = 0;
    (void)f;

    /* Clear the Overrun flag just before receiving the first character */
    __HAL_UART_CLEAR_OREFLAG(g_HDebugUART);

    /* Wait for reception of a character on the USART RX line and echo this
    * character on console */
    HAL_UART_Receive(g_HDebugUART, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    HAL_UART_Transmit(g_HDebugUART, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;    
}
